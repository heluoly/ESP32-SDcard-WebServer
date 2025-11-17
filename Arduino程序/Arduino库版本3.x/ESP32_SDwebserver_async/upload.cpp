#include "upload.h"

//列出上传的文件
String listUploadDir(fs::FS &fs, const char *dirname, uint8_t levels, String page) {
  uint8_t i = 1;
  char pageState = 0;
  const char pageBreak = 20;  //设定分页区间，现在是每20个视频一页
  char page0 = String2Char((char *)page.c_str());
  char page1 = (page0 - 1) * pageBreak;
  char page2 = page0 * pageBreak + 1;

  String filepath = "";
  String filename = "";

  String message = "";
  File root = fs.open(dirname);
  if (!root) {
    message += "Failed to open directory <br />";
    return message;
  }
  if (!root.isDirectory()) {
    message += "Not a directory <br />";
    return message;
  }
  message += "<table><tr><th align='left'>文件名</th><th align='left'>大小</th><th></th><th></th></tr>";
  File file = root.openNextFile();
  while (file) {
    if (file.isDirectory()) {
      // message +="  DIR : ";
      // message += String(file.path())+String("<br />");
      // if(levels){
      //     message += listUploadDir(fs, file.path(), levels -1);
      // }
    } else if (i > page1 && i < page2) {
      filepath = String(file.path());
      filename = String(file.name());

      message += "<tr align='left'><td>" + filename + "</td><td>" + formatBytes(file.size());
      message += "</td><td><button onclick=\"downloadButton(\'" + filepath + "\',\'" + filename + "\')\">下载</button></td>";
      message += "<td><button onclick=\"deleteButton(\'" + filepath + "\')\">删除</button></tr>";
    }
    file = root.openNextFile();
    i++;
  }
  message += "</table>";

  page1 = (i + pageBreak - 2) / pageBreak;
  message += "<br />页数: ";
  for (i = 1; i <= page1; i++) {
    message += "<button onclick=\"listFilesPage(\'";
    message += i;
    message += "\')\">";
    message += i;
    message += "</button>  ";
  }
  message += "<br />当前页: ";
  message += page;

  //判断当前页位置
  if (page1 == 1) {
    pageState = 0;  //不要上一页 不要下一页
  } else if (page0 == 1) {
    pageState = 1;  //不要上一页
  } else if (page0 == page1) {
    pageState = 2;  //不要下一页
  } else {
    pageState = 3;  //正常
  }

  switch (pageState) {
    case 0:
      {
        break;
      }
    case 1:
      {
        message += " <button onclick=\"listFilesPage(\'";
        message += page0 + 1;
        message += "\')\">下一页</button>";
        break;
      }
    case 2:
      {
        message += " <button onclick=\"listFilesPage(\'";
        message += page0 - 1;
        message += "\')\">上一页</button>";
        break;
      }
    case 3:
      {
        message += " <button onclick=\"listFilesPage(\'";
        message += page0 - 1;
        message += "\')\">上一页</button> <button onclick=\"listFilesPage(\'";
        message += page0 + 1;
        message += "\')\">下一页</button>";
        break;
      }
    default:
      {
        break;
      }
  }
  return message;
}

void listUploadFile(AsyncWebServerRequest *request) {
  String page = request->getParam("page")->value();  //获取页数
  String message = listUploadDir(my_fs, "/upload", 1, page);
  request->send(200, "text/plain", message);
}

void uploadFileRespond(AsyncWebServerRequest *request) {
  request->send(200);
}

void handleFileUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {

  if (my_fs.exists(filename.c_str())) {
    my_fs.remove(filename.c_str());
  }
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

// void downloadUploadFile(AsyncWebServerRequest *request) {
//   String attname = request->getParam("attname")->value();
//   String downloadPath = request->getParam("downloadPath")->value();
//   String attachment = "";
//   attachment += "attachment; filename=" + attname;
//   // Serial.println(downloadPath);
//   if (my_fs.exists(downloadPath)) {
//     AsyncWebServerResponse *response = request->beginResponse(my_fs, downloadPath, "application/octet-stream");
//     response->addHeader("Content-Disposition", attachment);
//     request->send(response);
//   } else {
//     request->send(404, "text/plain", "Not found");
//   }
// }


//--------------------------------------------------------------

// void downloadUploadFile(AsyncWebServerRequest *request) {
//   String attname = request->getParam("attname")->value();
//   String downloadPath = request->getParam("downloadPath")->value();

//   // Serial.println(downloadPath);
//   if (!my_fs.exists(downloadPath)) {
//     request->send(404, "text/plain", "Not found");
//     return;
//   }

//   File file = my_fs.open(downloadPath, FILE_READ);
//   if (!file) {
//     request->send(500, "text/plain", "Failed to open file");
//     return;
//   }

//   size_t fileSize = file.size();
//   size_t startByte = 0;
//   size_t endByte = fileSize - 1;

//   // 检查Range头部，支持断点续传
//   if (request->hasHeader("Range")) {
//     String rangeHeader = request->getHeader("Range")->value();
//     Serial.println("Range Header: " + rangeHeader);

//     // 解析Range头部，格式: bytes=start-end
//     if (rangeHeader.startsWith("bytes=")) {
//       String rangeValue = rangeHeader.substring(6);
//       int dashIndex = rangeValue.indexOf('-');

//       if (dashIndex != -1) {
//         String startStr = rangeValue.substring(0, dashIndex);
//         String endStr = rangeValue.substring(dashIndex + 1);

//         startByte = startStr.toInt();
//         if (endStr.length() > 0) {
//           endByte = endStr.toInt();
//         }

//         // 边界检查
//         if (endByte >= fileSize) {
//           endByte = fileSize - 1;
//         }

//         if (startByte > endByte || startByte >= fileSize) {
//           request->send(416, "text/plain", "The requested scope is invalid");
//           file.close();
//           return;
//         }
//       }
//     }
//   }

//   size_t contentLength = endByte - startByte + 1;

//   // 设置响应头
//   AsyncWebServerResponse *response = request->beginResponse(file, downloadPath, "application/octet-stream", false);

//   // 如果是部分内容请求，发送206状态码
//   if (startByte > 0 || endByte < fileSize - 1) {
//     response->setCode(206);
//     String contentRange = "bytes " + String(startByte) + "-" + String(endByte) + "/" + String(fileSize);
//     response->addHeader("Content-Range", contentRange);
//     file.seek(startByte);   // 如果请求指定了起始位置，跳转到该位置
//     // Serial.println("Partial content: " + contentRange);
//   }

//   response->addHeader("Content-Length", String(contentLength));
//   response->addHeader("Accept-Ranges", "bytes");
//   response->addHeader("Content-Disposition", "attachment; filename=\"" + attname + "\"");
//   response->addHeader("Connection", "close");   // 主动关闭当前连接

//   request->send(response);

// }

//--------------------------------------------------------------
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
