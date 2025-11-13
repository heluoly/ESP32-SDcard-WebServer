#include "upload.h"

//列出上传的文件
String listUploadDir(fs::FS &fs, const char *dirname, uint8_t levels, String page) {
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

void downloadUploadFile(AsyncWebServerRequest *request) {
  String attname = request->getParam("attname")->value();
  String downloadPath = request->getParam("downloadPath")->value();
  String attachment = "";
  attachment += "attachment; filename=" + attname;
  // Serial.println(downloadPath);
  if (my_fs.exists(downloadPath)) {
    AsyncWebServerResponse *response = request->beginResponse(my_fs, downloadPath, "application/octet-stream");
    response->addHeader("Content-Disposition", attachment);
    request->send(response);
  } else {
    request->send(404, "text/plain", "Not found");
  }
}
