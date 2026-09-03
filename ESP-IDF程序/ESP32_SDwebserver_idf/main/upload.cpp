#include "upload.h"

void listUploadFile(AsyncWebServerRequest *request) {
  String page = request->getParam("page")->value();  //获取页数
  const char *dirname = "/upload";
  uint8_t i = 1;
  const char pageBreak = 20;  //设定分页区间，每20个文件一页
  char page0 = String2Char((char *)page.c_str());
  char page1 = (page0 - 1) * pageBreak;
  char page2 = page0 * pageBreak + 1;
  int pageTotal = 1;
  bool first = true;
  String filePath = "";
  String fileName = "";
  String message = "";

  File root = my_fs.open(dirname);
  if (!root) {
    request->send(404, "text/plain", "Not found");
  }
  if (!root.isDirectory()) {
    request->send(404, "text/plain", "Not found");
  }

  message += "{\"files\": [ ";
  File file = root.openNextFile();
  while (file) {
    if (file.isDirectory()) {
      // 文件夹不处理
    } else if (i > page1 && i < page2) {
      filePath = String(file.path());
      fileName = String(file.name());

      if (!first) {
        message += ",";
      }
      first = false;

      message += "{ \"name\": \"";
      message += fileName;
      message += "\", \"size\": \"";
      message += file.size();
      message += "\", \"path\": \"";
      message += filePath;
      message += "\" }";
      i++;
    } else {
      i++;
      // 非分页范围忽略，最后统计总文件数量
    }
    file = root.openNextFile();
  }
  // message.remove(message.length() - 1);  //删除最后的","

  pageTotal = (i + pageBreak - 2) / pageBreak;
  message += " ], \"currentPage\": ";
  message += page;
  message += " , \"totalPages\": ";
  message += pageTotal;
  message += "}";
  request->send(200, "application/json", message);
}


void uploadFileRespond(AsyncWebServerRequest *request) {
  request->send(200);
}


#define UPLOAD_WRITER_BUF_COUNT 4           //数据缓冲个数
#define UPLOAD_WRITER_BUF_SIZE  (32 * 1024) //单块32KB（优先PSRAM）
#define UPLOAD_WRITER_PRIO      (12)        //高于async_tcp(10)，低于tcpip线程
#define UPLOAD_WRITER_STACK     (6144)
#define UPLOAD_QUEUE_WAIT_MS    (1000)      //背压等待上限（async_tcp带看门狗，不能久等）

//写队列元素：缓冲指针 + 本次实际要写的字节数（NULL指针=结束哨兵）
typedef struct {
  uint8_t *buf;
  size_t len;
} uploadWriteItem_t;

typedef struct {
  File file;                   //目标文件（由写卡任务打开/关闭）
  QueueHandle_t qEmpty;        //空闲缓冲队列
  QueueHandle_t qFull;         //待写缓冲队列（buf==NULL=结束哨兵）
  TaskHandle_t task;           //写卡任务句柄
  SemaphoreHandle_t doneSem;   //写卡任务结束信号
  uint8_t *pool;               //缓冲池（连续 bufCount*bufCap 字节）
  size_t bufCount;
  size_t bufCap;
  uint8_t *cur;                //当前正在填充的缓冲
  size_t curUsed;
  volatile bool ok;            //链路是否正常（出错后停止写入）
  volatile uint32_t recvBytes; //收包侧累计收到的文件字节（校验用）
  volatile uint32_t writeBytes;//写卡侧累计已写入字节（诊断用）
  uint32_t prevSize;           //断点续传：文件原有的字节数（新文件为0）
} uploadSession_t;

//取第i块缓冲指针
static inline uint8_t *uploadBufferAt(uploadSession_t *s, size_t i) {
  return s->pool + i * s->bufCap;
}

//写卡任务：从qFull取整块缓冲写盘；收到NULL哨兵后关闭文件并退出
static void uploadWriterTask(void *pv) {
  uploadSession_t *s = (uploadSession_t *)pv;
  for (;;) {
    uploadWriteItem_t item;
    if (xQueueReceive(s->qFull, &item, portMAX_DELAY) != pdTRUE) break;
    if (item.buf == NULL) break;  //结束哨兵
    if (s->ok && item.len > 0) {
      size_t remaining = item.len;
      uint8_t *p = item.buf;
      while (remaining > 0) {
        size_t w = s->file.write(p, remaining);
        if (w == 0 || w > remaining) {  //写盘失败（如SD卡满）
          ESP_LOGW("upload", "write fail at offset %llu (req %u)", (unsigned int)s->writeBytes, (unsigned)remaining);
          s->ok = false;
          break;
        }
        p += w;
        remaining -= w;
        s->writeBytes += w;
      }
    }
    uploadWriteItem_t back;
    back.buf = item.buf;
    back.len = 0;
    xQueueSend(s->qEmpty, &back, portMAX_DELAY);  //回收缓冲
  }
  s->file.close();
  xSemaphoreGive(s->doneSem);
  vTaskDelete(NULL);
}

//等待写卡任务退出（最多3s）
static void uploadSessionJoin(uploadSession_t *s) {
  if (s == NULL || s->task == NULL || s->doneSem == NULL) return;
  for (int i = 0; i < 60; i++) {
    if (xSemaphoreTake(s->doneSem, pdMS_TO_TICKS(50)) == pdTRUE) break;
  }
  s->task = NULL;
}

//释放会话资源（调用前需已结束写卡任务）
static void uploadSessionFree(uploadSession_t *s) {
  if (s == NULL) return;
  if (s->qFull) vQueueDelete(s->qFull);
  if (s->qEmpty) vQueueDelete(s->qEmpty);
  if (s->doneSem) vSemaphoreDelete(s->doneSem);
  if (s->pool) free(s->pool);
  free(s);
}

//异常中止：投递结束哨兵，让写卡任务把已入队数据写完、关闭文件并退出，然后回收
static void uploadSessionAbort(uploadSession_t *s) {
  if (s == NULL) return;
  uploadWriteItem_t end;
  end.buf = NULL;
  end.len = 0;
  for (int i = 0; i < 20; i++) {  //若队列满则等待腾位，最多约2s
    if (xQueueSend(s->qFull, &end, pdMS_TO_TICKS(100)) == pdTRUE) break;
  }
  uploadSessionJoin(s);
  uploadSessionFree(s);
}

//创建会话：分配缓冲池/队列/信号量并启动写卡任务；失败返回NULL（文件由调用者关闭）
static uploadSession_t *uploadSessionCreate(File &file, const char *path) {
  uploadSession_t *s = (uploadSession_t *)calloc(1, sizeof(uploadSession_t));
  if (s == NULL) return NULL;
  s->file = file;
  s->bufCount = UPLOAD_WRITER_BUF_COUNT;
  s->bufCap = UPLOAD_WRITER_BUF_SIZE;
  s->ok = true;
  // snprintf(s->path, sizeof(s->path), "%s", path);
  s->qEmpty = xQueueCreate(s->bufCount, sizeof(uploadWriteItem_t));
  s->qFull = xQueueCreate(s->bufCount + 2, sizeof(uploadWriteItem_t));
  s->doneSem = xSemaphoreCreateBinary();
  if (s->qEmpty == NULL || s->qFull == NULL || s->doneSem == NULL) goto fail;

  s->pool = (uint8_t *)heap_caps_malloc(s->bufCount * s->bufCap, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
  if (s->pool == NULL) {
    s->pool = (uint8_t *)malloc(s->bufCount * s->bufCap);  //PSRAM不可用时退回普通堆
  }
  if (s->pool == NULL) goto fail;

  for (size_t i = 0; i < s->bufCount; i++) {
    uploadWriteItem_t it;
    it.buf = uploadBufferAt(s, i);
    it.len = 0;
    if (xQueueSend(s->qEmpty, &it, 0) != pdTRUE) goto fail;
  }

  if (xTaskCreate(uploadWriterTask, "ul_writer", UPLOAD_WRITER_STACK, s, UPLOAD_WRITER_PRIO, &s->task) != pdPASS) {
    s->task = NULL;
    goto fail;
  }
  return s;

fail:
  uploadSessionFree(s);  //task为空，free不会等待
  return NULL;
}

void handleFileUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
  uploadSession_t *s = (uploadSession_t *)request->_tempObject;

  if (!index) {
    //防御：前一文件块异常未收尾
    if (s != NULL) {
      request->_tempObject = NULL;
      uploadSessionAbort(s);
    }

    String path = "/upload/" + filename;
    //断点续传：若文件已存在，记录原有大小并在其后追加
    uint32_t prevSize = 0;
    bool existed = my_fs.exists(path);
    if (existed) {
      File t = my_fs.open(path, FILE_READ);
      if (t) {
        prevSize = t.size();
        t.close();
      }
    }
    File f;
    if (existed) {
      f = my_fs.open(path, FILE_APPEND);
    } else {
      f = my_fs.open(path, FILE_WRITE);
    }
    if (!f) {
      request->send(400, "text/plain", "File not available for writing");
      return;  //打开失败：忽略后续数据
    }

    s = uploadSessionCreate(f, path.c_str());
    if (s == NULL) {
      f.close();
      request->send(500, "text/plain", "upload session fail");
      return;
    }
    s->prevSize = prevSize;
    request->_tempObject = s;

    //请求异常断开时收尾（正常结束后_tempObject已置NULL，此处为空操作）
    request->onDisconnect([request]() {
      uploadSession_t *sess = (uploadSession_t *)request->_tempObject;
      if (sess != NULL) {
        request->_tempObject = NULL;
        uploadSessionAbort(sess);
      }
    });
  }

  if (s == NULL) {
    return;  //会话创建失败：忽略后续数据
  }

  //生产端：数据拷入当前缓冲，攒满整块（指针+长度）交给写卡任务
  if (len > 0 && s->ok) {
    while (len > 0) {
      if (s->cur == NULL) {
        uploadWriteItem_t it;
        if (xQueueReceive(s->qEmpty, &it, pdMS_TO_TICKS(UPLOAD_QUEUE_WAIT_MS)) != pdTRUE) {
          s->ok = false;  //长时间等不到空闲缓冲：写卡严重落后，停止接收
          break;
        }
        s->cur = it.buf;
        s->curUsed = 0;
      }
      size_t space = s->bufCap - s->curUsed;
      size_t n = (len < space) ? len : space;
      memcpy(s->cur + s->curUsed, data, n);
      s->curUsed += n;
      s->recvBytes += n;
      data += n;
      len -= n;

      if (s->curUsed == s->bufCap) {
        uploadWriteItem_t full;
        full.buf = s->cur;
        full.len = s->bufCap;
        s->cur = NULL;
        s->curUsed = 0;
        if (xQueueSend(s->qFull, &full, pdMS_TO_TICKS(UPLOAD_QUEUE_WAIT_MS)) != pdTRUE) {
          s->ok = false;
          uploadWriteItem_t back;
          back.buf = full.buf;
          back.len = 0;
          xQueueSend(s->qEmpty, &back, 0);  //归还缓冲
          break;
        }
      }
    }
  }

  if (final) {
    request->_tempObject = NULL;  //收尾开始，断开回调不再触碰本会话
    //残块（<32KB）按真实长度交给写卡任务
    if (s->cur != NULL) {
      if (s->curUsed > 0 && s->ok) {
        uploadWriteItem_t tail;
        tail.buf = s->cur;
        tail.len = s->curUsed;
        if (xQueueSend(s->qFull, &tail, pdMS_TO_TICKS(UPLOAD_QUEUE_WAIT_MS)) != pdTRUE) {
          s->ok = false;
          uploadWriteItem_t back;
          back.buf = tail.buf;
          back.len = 0;
          xQueueSend(s->qEmpty, &back, 0);
        }
      } else {  //空缓冲/出错：直接归还
        uploadWriteItem_t back;
        back.buf = s->cur;
        back.len = 0;
        xQueueSend(s->qEmpty, &back, 0);
      }
      s->cur = NULL;
      s->curUsed = 0;
    }
    //结束哨兵，等待写卡任务把剩余数据写完并关闭文件
    uploadWriteItem_t end;
    end.buf = NULL;
    end.len = 0;
    xQueueSend(s->qFull, &end, pdMS_TO_TICKS(UPLOAD_QUEUE_WAIT_MS));
    uploadSessionJoin(s);
    uploadSessionFree(s);
  }
}

void handleUploadStatus(AsyncWebServerRequest *request) {
  if (!request->hasParam("filename")) {
    request->send(400, "application/json", "{\"exists\":false,\"size\":0}");
    return;
  }
  String filename = request->getParam("filename")->value();
  String path = "/upload/" + filename;

  if (my_fs.exists(path)) {
    File f = my_fs.open(path, FILE_READ);
    size_t sz = f.size();
    f.close();
    String json = "{\"exists\":true,\"size\":" + String(sz) + "}";
    request->send(200, "application/json", json);
  } else {
    request->send(200, "application/json", "{\"exists\":false,\"size\":0}");
  }
}

void deleteUploadFile(AsyncWebServerRequest *request) {
  String deletePath = request->getParam("deletePath")->value();
  char flag = 0;
  flag = deleteFile(my_fs, (char *)deletePath.c_str());
  if (flag) {
    request->send(200, "text/html", "删除成功");
  } else {
    request->send(200, "text/html", "删除失败");
    // Serial.println("Delete failed");
  }
}

//https://forum.arduino.cc/t/esp32-espasyncwebserver-library-beginchunkedresponse-usage/1403445/15

void downloadUploadFile(AsyncWebServerRequest *request) {
  String attname = request->getParam("attname")->value();
  String downloadPath = request->getParam("downloadPath")->value();

  // Serial.println(downloadPath);
  if (!my_fs.exists(downloadPath)) {
    request->send(404, "text/plain", "Not found");
    return;
  }

  File file = my_fs.open(downloadPath, FILE_READ);
  if (!file) {
    request->send(500, "text/plain", "Failed to open file");
    return;
  }

  size_t fileSize = file.size();
  size_t startByte = 0;
  size_t endByte = fileSize - 1;
  bool isPartialContent = false;

  // 解析Range头部
  if (request->hasHeader("Range")) {
    String rangeHeader = request->getHeader("Range")->value();
    // Serial.println("Range Header: " + rangeHeader);

    if (rangeHeader.startsWith("bytes=")) {
      String rangeValue = rangeHeader.substring(6);
      int dashIndex = rangeValue.indexOf('-');

      if (dashIndex != -1) {
        String startStr = rangeValue.substring(0, dashIndex);
        String endStr = rangeValue.substring(dashIndex + 1);

        if (startStr.length() > 0) {
          startByte = (size_t)strtoul(startStr.c_str(), NULL, 10);
        }

        if (endStr.length() > 0) {
          endByte = (size_t)strtoul(endStr.c_str(), NULL, 10);
        } else {
          endByte = fileSize - 1;
        }

        // 边界检查
        if (startByte >= fileSize || endByte >= fileSize || startByte > endByte) {
          request->send(416, "text/plain", "The requested scope is invalid");
          file.close();
          return;
        }

        isPartialContent = true;
      }
    }
  }

  size_t contentLength = endByte - startByte + 1;

  // 跳转到起始位置
  if (startByte > 0) {
    if (!file.seek(startByte)) {
      request->send(500, "text/plain", "Failed to open file");
      file.close();
      return;
    }
  }

  // Serial.printf("传输范围: %s [%zu-%zu]/%zu, 长度: %zu\n", attname.c_str(), startByte, endByte, fileSize, contentLength);

  // 使用智能指针管理文件对象
  auto filePtr = std::make_shared<File>(std::move(file));

  // 创建分块响应
  AsyncWebServerResponse *response = request->beginChunkedResponse(
    "application/octet-stream",
    [filePtr, startByte, endByte, contentLength](uint8_t *buffer, size_t maxLen, size_t index) -> size_t {
      // 计算剩余字节数
      size_t bytesRemaining = contentLength - index;

      // 如果已经传输完毕
      if (bytesRemaining == 0) {
        filePtr->close();
        return 0;
      }

      // 计算本次读取的字节数
      size_t bytesToRead = min(maxLen, bytesRemaining);

      // 读取文件数据
      size_t bytesRead = filePtr->read(buffer, bytesToRead);

      // 检查是否到达了结束位置
      size_t currentPos = startByte + index + bytesRead;
      if (currentPos > endByte + 1) {
        bytesRead = endByte + 1 - (startByte + index);
      }

      return bytesRead;
    });

  // 设置HTTP头
  if (isPartialContent) {
    response->setCode(206);
    String contentRange = "bytes " + String(startByte) + "-" + String(endByte) + "/" + String(fileSize);
    response->addHeader("Content-Range", contentRange);
  }

  response->addHeader("Content-Length", String(contentLength));
  response->addHeader("Accept-Ranges", "bytes");
  response->addHeader("Content-Disposition", "attachment; filename=\"" + attname + "\"");
  response->addHeader("Connection", "close");

  request->send(response);
}
