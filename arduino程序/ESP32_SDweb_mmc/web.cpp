#include "web.h"

extern WebServer esp32_server;
extern bool hasSD;
extern bool ONE_BIT_MODE;

// 处理用户浏览器的HTTP访问
void handleUserRequet() {         
  String path = esp32_server.uri();   // 获取用户请求网址信息
  bool fileReadOK = false;
  String contentType = "";

  if (path.endsWith("/")) {                   // 如果访问地址以"/"为结尾
    path = "/index.html";                     // 则将访问地址修改为/index.html便于SPIFFS访问
    contentType = "text/html";
  }
  else if(path.endsWith(".html")) contentType =  "text/html";
  else if(path.endsWith(".ts")) contentType =  "video/MP2T";
  else if(path.endsWith(".css")) contentType =  "text/css";
  else if(path.endsWith(".js")) contentType =  "application/javascript";
  else if(path.endsWith(".png")) contentType =  "image/png";
  else if(path.endsWith(".gif")) contentType =  "image/gif";
  else if(path.endsWith(".jpg")) contentType =  "image/jpeg";
  else if(path.endsWith(".ico")) contentType =  "image/x-icon";
  else if(path.endsWith(".m3u8")) contentType =  "application/x-mpegURL";
  else contentType = "text/plain";

  if (SD_MMC.exists(path)) {                     // 如果访问的文件可以在SD卡中找到
    File file = SD_MMC.open(path, FILE_READ);          // 则尝试打开该文件
    esp32_server.streamFile(file, contentType);// 并且将该文件返回给浏览器
    file.close();                                // 并且关闭文件
    fileReadOK = true;
  }
  else{
    fileReadOK = false;
  }

  if (!fileReadOK){
    SD_MMC.end();
    if(SD_MMC.begin("/sdcard", ONE_BIT_MODE))  //SD卡初始化
    {
      if(!hasSD){
        esp32_server.send(404, "text/plain", "Card Mount Succeed");  // 如果在SD卡初始化成功，则回复Card Mount Succeed
        hasSD=true;
      }
      else{
        esp32_server.send(404, "text/plain", "404 Not Found");  // 如果在SD卡无法找到用户访问的资源，则回复404 Not Found
      }

    }
    else
    {
      hasSD=false;
      esp32_server.send(404, "text/plain", "Card Mount Failed");  // 如果无法读取SD卡，则回复Card Mount Failed
    }
  }

}

void respondOK(){
  esp32_server.send(200);
}
