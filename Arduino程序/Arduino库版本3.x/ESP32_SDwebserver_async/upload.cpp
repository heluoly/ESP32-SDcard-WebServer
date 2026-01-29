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

void handleFileUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
  if (!index) {
    request->_tempFile = my_fs.open("/upload/" + filename, FILE_WRITE);
    if (!request->_tempFile) {
      request->send(400, "text/plain", "File not available for writing");
    }
  }
  if (len) {
    request->_tempFile.write(data, len);
  }
  if (final) {
    request->_tempFile.close();
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
