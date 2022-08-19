#include"web.h"

extern WebServer esp32_server;
extern bool hasSD;
extern bool ONE_BIT_MODE;

// 获取文件类型
String getContentType(String filename){
  if(filename.endsWith(".htm")) return "text/html";
  else if(filename.endsWith(".html")) return "text/html";
  else if(filename.endsWith(".css")) return "text/css";
  else if(filename.endsWith(".js")) return "application/javascript";
  else if(filename.endsWith(".png")) return "image/png";
  else if(filename.endsWith(".gif")) return "image/gif";
  else if(filename.endsWith(".jpg")) return "image/jpeg";
  else if(filename.endsWith(".ico")) return "image/x-icon";
  else if(filename.endsWith(".ts")) return "video/MP2T";
  else if(filename.endsWith(".m3u8")) return "application/x-mpegURL";
  /*
  else if(filename.endsWith(".xml")) return "text/xml";
  else if(filename.endsWith(".swf")) return "application/x-shockwave-flash";
  else if(filename.endsWith(".acc")) return "audio/aac";
  else if(filename.endsWith(".mp3")) return "audio/mpeg";
  else if(filename.endsWith(".avi")) return "video/x-msvideo";
  else if(filename.endsWith(".mp4")) return "video/mp4";
  else if(filename.endsWith(".mpeg")) return "video/mpeg";
  else if(filename.endsWith(".m3u8")) return "application/x-mpegURL";
  else if(filename.endsWith(".m3u8")) return "video/MP2T";
  else if(filename.endsWith(".pdf")) return "application/x-pdf";
  else if(filename.endsWith(".zip")) return "application/x-zip";
  else if(filename.endsWith(".gz")) return "application/x-gzip";
  */
  return "text/plain";
}


bool handleFileRead(String path) {            //处理浏览器HTTP访问

  if (path.endsWith("/")) {                   // 如果访问地址以"/"为结尾
    path = "/index.html";                     // 则将访问地址修改为/index.html便于SPIFFS访问
  } 
  
  String contentType = getContentType(path);  // 获取文件类型
  
  if (SD_MMC.exists(path)) {                     // 如果访问的文件可以在SPIFFS中找到
    File file = SD_MMC.open(path, FILE_READ);          // 则尝试打开该文件
    esp32_server.streamFile(file, contentType);// 并且将该文件返回给浏览器
    file.close();                                // 并且关闭文件
    return true;                                 // 返回true
  }
  return false;                                  // 如果文件未找到，则返回false
}


// 处理用户浏览器的HTTP访问
void handleUserRequet() {         
  String webAddress = esp32_server.uri();   // 获取用户请求网址信息
  
  bool fileReadOK = handleFileRead(webAddress); // 通过handleFileRead函数处处理用户访问

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
