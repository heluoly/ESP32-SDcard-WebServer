#include "upload.h"
#include "rangeFileResponse.h"
#include "sdFile64.h"

void listUploadFile(AsyncWebServerRequest *request) {
  String page = request->getParam("page")->value();  // 获取页数
  const char *dirname = "/upload";
  uint8_t i = 1;
  const char pageBreak = 20;  // 设定分页区间，每20个文件一页
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
    return;
  }
  if (!root.isDirectory()) {
    request->send(404, "text/plain", "Not found");
    return;
  }

  message += "{\"files\": [ ";
  File file = root.openNextFile();
  while (file) {
    if (file.isDirectory()) {
      // 文件夹不处理
    } else if (i > page1 && i < page2) {
      filePath = String(file.path());
      fileName = String(file.name());
      char sizeBuf[24];
      snprintf(sizeBuf, sizeof(sizeBuf), "%llu", (unsigned long long)sd64::fileSize(filePath));

      if (!first) {
        message += ",";
      }
      first = false;

      message += "{ \"name\": \"";
      message += fileName;
      message += "\", \"size\": \"";
      message += sizeBuf;
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
  // message.remove(message.length() - 1);  // 删除最后的","

  pageTotal = (i + pageBreak - 2) / pageBreak;
  message += " ], \"currentPage\": ";
  message += page;
  message += " , \"totalPages\": ";
  message += pageTotal;
  message += "}";
  request->send(200, "application/json", message);
}


struct UploadContext {
  FIL fil;
  bool open;
  bool failed;
  uint8_t *writeBuf;
  size_t writeLen;
};

static constexpr size_t kUploadBufferSize = 16 * 1024;

static bool parseOffset(const String &s, uint64_t &out) {
  if (s.length() == 0 || s[0] < '0' || s[0] > '9') {
    return false;
  }
  errno = 0;
  char *end = nullptr;
  unsigned long long v = strtoull(s.c_str(), &end, 10);
  if (errno == ERANGE || end == s.c_str() || *end != '\0') {
    return false;
  }
  out = (uint64_t)v;
  return true;
}

static bool uploadEnsureBuffer(UploadContext *ctx) {
  if (ctx->writeBuf != nullptr) {
    return true;
  }
  ctx->writeBuf = (uint8_t *)heap_caps_aligned_alloc(64, kUploadBufferSize, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
  if (ctx->writeBuf == nullptr) {
    ctx->writeBuf =
      (uint8_t *)heap_caps_aligned_alloc(64, kUploadBufferSize, MALLOC_CAP_INTERNAL | MALLOC_CAP_DMA | MALLOC_CAP_8BIT);
  }
  return ctx->writeBuf != nullptr;
}

static bool uploadFlush(UploadContext *ctx) {
  if (ctx->failed) {
    return false;
  }
  if (ctx->writeLen == 0) {
    return true;
  }

  size_t offset = 0;
  while (offset < ctx->writeLen) {
    UINT bytesWritten = 0;
    FRESULT res = f_write(&ctx->fil, ctx->writeBuf + offset, (UINT)(ctx->writeLen - offset), &bytesWritten);
    if (res != FR_OK || bytesWritten == 0) {
      ctx->failed = true;
      if (ctx->open) {
        f_close(&ctx->fil);
        ctx->open = false;
      }
      return false;
    }
    offset += bytesWritten;
  }
  ctx->writeLen = 0;
  return true;
}

static void releaseUploadContext(UploadContext *ctx) {
  if (ctx == nullptr) {
    return;
  }
  if (ctx->open) {
    /* 异常断开时把缓冲中的数据刷入文件，便于客户端按实际大小续传 */
    if (!ctx->failed && ctx->writeLen > 0) {
      uploadFlush(ctx);
    }
    if (ctx->open) {
      f_close(&ctx->fil);
      ctx->open = false;
    }
  }
  if (ctx->writeBuf != nullptr) {
    heap_caps_free(ctx->writeBuf);
    ctx->writeBuf = nullptr;
  }
  free(ctx);
}

void uploadFileRespond(AsyncWebServerRequest *request) {
  UploadContext *ctx = static_cast<UploadContext *>(request->_tempObject);
  if (ctx != nullptr) {
    bool failed = ctx->failed;
    request->_tempObject = nullptr;
    releaseUploadContext(ctx);
    if (failed) {
      request->send(500, "text/plain", "Upload write failed");
      return;
    }
  }
  request->send(200);
}

void handleFileUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
  if (!index) {
    /* 同一请求包含多个文件部件时，先释放上一个文件的句柄 */
    UploadContext *previous = static_cast<UploadContext *>(request->_tempObject);
    if (previous != nullptr) {
      request->_tempObject = nullptr;
      releaseUploadContext(previous);
    }

    String path = "/upload/" + filename;

    uint64_t offset = 0;
    if (request->hasHeader("X-Upload-Offset")) {
      const AsyncWebHeader *h = request->getHeader("X-Upload-Offset");
      if (h && h->value().length() > 0) {
        if (!parseOffset(h->value(), offset)) {
          request->send(400, "text/plain", "invalid X-Upload-Offset");
          return;
        }
      }
    }

    uint64_t prevSize = 0;
    bool isDir = false;
    bool existed = sd64::stat(path, prevSize, isDir);
    if (isDir) {
      request->send(400, "text/plain", "target is a directory");
      return;
    }

    UploadContext *ctx = static_cast<UploadContext *>(calloc(1, sizeof(UploadContext)));
    if (ctx == nullptr) {
      request->send(500, "text/plain", "out of memory");
      return;
    }

    FRESULT res;
    String fatPath = sd64::toFatPath(path);

    if (offset > 0) {
      if (!existed) {
        releaseUploadContext(ctx);
        request->send(404, "text/plain", "resume target missing");
        return;
      }
      if (prevSize != offset) {
        releaseUploadContext(ctx);
        request->send(409, "text/plain", "offset mismatch, re-query upload status");
        return;
      }
      res = f_open(&ctx->fil, fatPath.c_str(), FA_READ | FA_WRITE | FA_OPEN_EXISTING);
      if (res == FR_INVALID_DRIVE || res == FR_NOT_ENABLED) {
        sd64::init();
        fatPath = sd64::toFatPath(path);
        res = f_open(&ctx->fil, fatPath.c_str(), FA_READ | FA_WRITE | FA_OPEN_EXISTING);
      }
      if (res == FR_OK) {
        res = f_lseek(&ctx->fil, (FSIZE_t)offset);
        if (res != FR_OK) {
          f_close(&ctx->fil);
        }
      }
    } else {
      res = f_open(&ctx->fil, fatPath.c_str(), FA_READ | FA_WRITE | FA_CREATE_ALWAYS);
      if (res == FR_INVALID_DRIVE || res == FR_NOT_ENABLED) {
        sd64::init();
        fatPath = sd64::toFatPath(path);
        res = f_open(&ctx->fil, fatPath.c_str(), FA_READ | FA_WRITE | FA_CREATE_ALWAYS);
      }
    }

    if (res != FR_OK) {
      releaseUploadContext(ctx);
      request->send(400, "text/plain", "File not available for writing");
      return;
    }
    ctx->open = true;

    request->_tempObject = ctx;
    request->onDisconnect([request]() {
      UploadContext *c = static_cast<UploadContext *>(request->_tempObject);
      if (c != nullptr) {
        request->_tempObject = nullptr;
        releaseUploadContext(c);
      }
    });
  }

  UploadContext *ctx = static_cast<UploadContext *>(request->_tempObject);
  if (ctx == nullptr || ctx->failed) {
    return;
  }

  if (len) {
    if (!uploadEnsureBuffer(ctx)) {
      UINT bytesWritten = 0;
      FRESULT res = f_write(&ctx->fil, data, (UINT)len, &bytesWritten);
      if (res != FR_OK || bytesWritten != len) {
        ctx->failed = true;
        if (ctx->open) {
          f_close(&ctx->fil);
          ctx->open = false;
        }
        return;
      }
    } else {
      size_t offset = 0;
      while (offset < len) {
        size_t space = kUploadBufferSize - ctx->writeLen;
        size_t n = (len - offset < space) ? (len - offset) : space;
        memcpy(ctx->writeBuf + ctx->writeLen, data + offset, n);
        ctx->writeLen += n;
        offset += n;
        if (ctx->writeLen == kUploadBufferSize) {
          if (!uploadFlush(ctx)) {
            return;
          }
        }
      }
    }
  }

  if (final) {
    if (!ctx->failed) {
      uploadFlush(ctx);
    }
    if (!ctx->failed) {
      f_sync(&ctx->fil);
    }
    if (ctx->open) {
      f_close(&ctx->fil);
      ctx->open = false;
    }
  }
}

void handleUploadStatus(AsyncWebServerRequest *request) {
  if (!request->hasParam("filename")) {
    request->send(400, "application/json", "{\"exists\":false,\"size\":0}");
    return;
  }
  String filename = request->getParam("filename")->value();
  String path = "/upload/" + filename;

  uint64_t sz = 0;
  bool isDir = false;
  if (sd64::stat(path, sz, isDir) && !isDir) {
    char json[64];
    snprintf(json, sizeof(json), "{\"exists\":true,\"size\":%llu}", (unsigned long long)sz);
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

void downloadUploadFile(AsyncWebServerRequest *request) {
  if (!request->hasParam("downloadPath")) {
    request->send(400, "text/plain", "missing downloadPath");
    return;
  }
  String downloadPath = request->getParam("downloadPath")->value();
  String attname = "";
  if (request->hasParam("attname")) {
    attname = request->getParam("attname")->value();
  }
  if (attname.length() == 0) {
    int slash = downloadPath.lastIndexOf('/');
    attname = (slash >= 0) ? downloadPath.substring(slash + 1) : downloadPath;
  }

  rangeFileServeDownload(request, downloadPath, attname);
}
