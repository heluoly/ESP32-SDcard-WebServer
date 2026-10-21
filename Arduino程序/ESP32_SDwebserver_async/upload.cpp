#include "upload.h"
#include "rangeFileResponse.h"

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
  // message.remove(message.length() - 1);  // 删除最后的","

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
    String path = "/upload/" + filename;
    uint32_t offset = 0;
    if (request->hasHeader("X-Upload-Offset")) {
      const AsyncWebHeader *h = request->getHeader("X-Upload-Offset");
      if (h && h->value().length() > 0) {
        offset = (uint32_t)strtoul(h->value().c_str(), NULL, 10);
      }
    }
    uint32_t prevSize = 0;
    bool existed = my_fs.exists(path);
    if (existed) {
      File t = my_fs.open(path, FILE_READ);
      if (t) {
        prevSize = t.size();
        t.close();
      }
    }
    if (offset > 0) {
      if (!existed) {
        request->send(404, "text/plain", "resume target missing");
        return;
      }
      if (prevSize != offset) {
        request->send(409, "text/plain", "offset mismatch, re-query upload status");
        return;
      }
      request->_tempFile = my_fs.open(path, FILE_APPEND);
    } else {
      if (existed) {
        my_fs.remove(path);
      }
      request->_tempFile = my_fs.open(path, FILE_WRITE);
    }
    if (!request->_tempFile) {
      request->send(400, "text/plain", "File not available for writing");
      return;
    }
  }
  if (len) {
    request->_tempFile.write(data, len);
  }
  if (final) {
    request->_tempFile.close();
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
