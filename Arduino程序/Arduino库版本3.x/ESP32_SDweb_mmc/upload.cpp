#include "upload.h"

extern WebServer esp32_server;
extern File fsUploadFile;

//列出上传的文件
String listUploadDir(fs::FS &fs, const char *dirname, uint8_t levels) {

  String page = esp32_server.arg("page");  //获取页数
  uint8_t i = 1;
  char pageState = 0;
  const char pageBreak = 20;  //设定分页区间，现在是每20个视频一页
  char page0 = String2Char((char *)page.c_str());
  char page1 = (page0 - 1) * pageBreak;
  char page2 = page0 * pageBreak + 1;

  String filename = "";
  String filename2 = "";

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
      filename = String(file.path());
      //filename2 = indexOfFilename(filename);
      filename2 = String(file.name());

      message += "<tr align='left'><td>" + filename2 + "</td><td>" + formatBytes(file.size());
      message += "</td><td><button onclick=\"downloadButton(\'" + filename + "\',\'" + filename2 + "\')\">下载</button></td>";
      message += "<td><button onclick=\"deleteButton(\'" + filename + "\')\">删除</button></tr>";
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

void listUploadFile() {
  String message = listUploadDir(SD_MMC, "/upload", 1);
  esp32_server.send(200, "text/html", message);
}


void handleFileUpload() {

  HTTPUpload &upload = esp32_server.upload();

  if (upload.status == UPLOAD_FILE_START) {  // 如果上传状态为UPLOAD_FILE_START

    if (SD_MMC.exists((char *)upload.filename.c_str())) {
      SD_MMC.remove((char *)upload.filename.c_str());
    }

    String filename = upload.filename;                                // 建立字符串变量用于存放上传文件名
    if (!filename.startsWith("/")) filename = "/upload/" + filename;  // 为上传文件名前加上"/"
    // Serial.println("File Name: " + filename);                 // 通过串口监视器输出上传文件的名称

    fsUploadFile = SD_MMC.open(filename, FILE_WRITE);  // 在SD卡中建立文件用于写入用户上传的文件数据

  } else if (upload.status == UPLOAD_FILE_WRITE) {  // 如果上传状态为UPLOAD_FILE_WRITE

    if (fsUploadFile)
      fsUploadFile.write(upload.buf, upload.currentSize);  // 向SD卡文件写入浏览器发来的文件数据

  } else if (upload.status == UPLOAD_FILE_END) {                          // 如果上传状态为UPLOAD_FILE_END
    if (fsUploadFile) {                                                   // 如果文件成功建立
      fsUploadFile.close();                                               // 将文件关闭
    } else {                                                              // 如果文件未能成功建立
                                                                          //   Serial.println("File upload failed");               // 通过串口监视器输出报错信息
      esp32_server.send(500, "text/plain", "500: couldn't create file");  // 向浏览器发送相应代码500（服务器错误）
    }
  }
}

void deleteUploadFile() {
  String deletePath = esp32_server.arg("deletePath");
  char flag = 0;
  flag = deleteFile(SD_MMC, (char *)deletePath.c_str());
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
  if (SD_MMC.exists(downloadPath)) {
    File file = SD_MMC.open(downloadPath, FILE_READ);
    esp32_server.sendHeader("Content-Disposition", (char *)attachment.c_str());
    esp32_server.streamFile(file, "application/octet-stream");
    file.close();
  } else {
    esp32_server.send(404, "text/plain", "404 Not Found");
  }
}
