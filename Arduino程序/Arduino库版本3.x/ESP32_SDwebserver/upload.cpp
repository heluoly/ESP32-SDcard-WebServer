#include "upload.h"

extern WebServer esp32_server;
File fsUploadFile;

void listUploadFile() {
  String page = esp32_server.arg("page");  //获取页数
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
    esp32_server.send(404, "text/plain", "Not found");
  }
  if (!root.isDirectory()) {
    esp32_server.send(404, "text/plain", "Not found");
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
  esp32_server.send(200, "application/json", message);
}


void uploadFileRespond() {
  esp32_server.send(200, "text/plain", "Upload Complete");
}

void handleFileUpload() { 
  HTTPUpload &upload = esp32_server.upload();

  if (upload.status == UPLOAD_FILE_START) {
    String filename = upload.filename;
    if (!filename.startsWith("/")) filename = "/upload/" + filename;
    // Serial.println("File Name: " + filename);

    if (my_fs.exists((char *)upload.filename.c_str())) {
      my_fs.remove((char *)upload.filename.c_str());
      // Serial.println("delete exist file");
    }

    fsUploadFile = my_fs.open(filename, FILE_WRITE);

  } else if (upload.status == UPLOAD_FILE_WRITE) {

    if (fsUploadFile) {
      fsUploadFile.write(upload.buf, upload.currentSize);
      // Serial.println("writing");
    }
      
  } else if (upload.status == UPLOAD_FILE_END) {

    if (fsUploadFile) {
      fsUploadFile.close();
    } else {
      // Serial.println("File upload failed");
      esp32_server.send(500, "text/plain", "500: couldn't create file");
    }

  }
}

void deleteUploadFile() {
  String deletePath = esp32_server.arg("deletePath");
  char flag = 0;
  flag = deleteFile(my_fs, (char *)deletePath.c_str());
  if (flag) {
    esp32_server.send(200, "text/html", "删除成功");
  } else {
    esp32_server.send(200, "text/html", "删除失败");
    // Serial.println("Delete failed");
  }
}

void downloadUploadFile() {
  String attname = esp32_server.arg("attname");
  String downloadPath = esp32_server.arg("downloadPath");
  String attachment = "";
  attachment += "attachment; filename=" + attname;
  if (my_fs.exists(downloadPath)) {
    File file = my_fs.open(downloadPath, FILE_READ);
    esp32_server.sendHeader("Content-Disposition", (char *)attachment.c_str());
    esp32_server.streamFile(file, "application/octet-stream");
    file.close();
  } else {
    esp32_server.send(404, "text/plain", "404 Not Found");
  }
}
