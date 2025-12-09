#include "video.h"

extern String IPAD;
extern String htmlHeader;

//读取txt文件
String readFile(fs::FS &fs, const char *path) {
  int i = 0;
  const int maximumLength = 256;
  char readbuff[maximumLength];
  String message = "";

  File file = fs.open(path);
  if (!file) {
    // Serial.println("Failed to open file for reading");
    return message;
  }

  while (file.available()) {
    if (i < maximumLength - 2) {
      readbuff[i] = file.read();
      i++;
    } else {
      break;
    }
  }
  file.close();
  readbuff[i] = '\0';
  message = readbuff;
  return message;
}

//列出内存卡中的视频
void listvideo(AsyncWebServerRequest *request) {
  String videoTape = request->getParam("videoTape")->value();  //获取视频分区路径
  String page = request->getParam("page")->value();            //获取页数
  uint8_t i = 1;
  const char pageBreak = 20;  //设定分页区间，现在是每20个视频一页
  char page0 = String2Char((char *)page.c_str());
  char page1 = (page0 - 1) * pageBreak;
  char page2 = page0 * pageBreak + 1;
  int pageTotal = 1;
  bool first = true;
  String filePath = "";
  String namePath = "";
  String picPath = "";
  String videoName = "";
  String message = "";

  File root = my_fs.open((char *)videoTape.c_str());
  if (!root) {
    // message += "Failed to open directory <br>";
    request->send(404, "text/plain", "Not found");
    return;
  } else if (!root.isDirectory()) {
    // message += "Not a directory <br>";
    request->send(404, "text/plain", "Not found");
    return;
  } else {
    message += "{\"videos\": [ ";
    File file = root.openNextFile();
    while (file) {
      if (!file.isDirectory()) {
        // 文件夹不处理
      } else if (i > page1 && i < page2) {

        filePath = String(file.path());
        namePath = filePath + "/0.txt";                         //视频标题路径
        picPath = filePath + "/0.jpg";                          //视频预览图路径
        videoName = readFile(my_fs, (char *)namePath.c_str());  //读取视频标题

        if (!first) {
          message += ",";
        }
        first = false;

        message += "{ \"title\": \"";
        message += videoName;
        message += "\", \"cover\": \"";
        message += picPath;
        message += "\", \"path\": \"";
        message += filePath;
        message += "\" }";
        i++;
      } else {
        i++;
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
  }
  request->send(200, "application/json", message);
}

//打开视频
void openVideo(AsyncWebServerRequest *request) {
  String videoPath = request->getParam("videoPath")->value();  //获取视频路径
  String namePath = videoPath;
  namePath += "/0.txt";
  String videoName = readFile(my_fs, (char *)namePath.c_str());  //读取视频标题

  //得到回车字符串
  // char enter[3];
  // String enter2 = "";
  // enter[0] = '\r';
  // enter[1] = '\n';
  // enter[2] = '\0';
  // enter2 = enter;

  String message = "<!DOCTYPE html><html lang=\"zh-CN\"><head><meta charset=\"UTF-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\"><title>";
  message += videoName;
  message += "</title><link href=\"/bin/videojs/7.5.5/video-js.min.css\" rel=\"stylesheet\"><style>* { margin: 0; padding: 0; box-sizing: border-box; font-family: 'Segoe UI', 'Microsoft YaHei', sans-serif; } body { background-color: #f8f9fa; color: #333; line-height: 1.6; min-height: 100vh; display: flex; flex-direction: column; align-items: center; padding: 20px; } .container { max-width: 800px; width: 100%; display: flex; flex-direction: column; gap: 20px; } .header { text-align: center; margin-bottom: 10px; } h1 { font-size: 28px; color: #333; font-weight: 600; text-shadow: 0 1px 2px rgba(0,0,0,0.1); } .video-card { background: white; border-radius: 15px; overflow: hidden; box-shadow: 0 5px 20px rgba(0, 0, 0, 0.08); border: 1px solid #f0f0f0; } .video-player-container { position: relative; width: 100%; background: #000; border-radius: 15px 15px 0 0; height: 0; padding-bottom: 56.25%; } .video-js { position: absolute; top: 0; left: 0; width: 100%; height: 100%; border-radius: 15px 15px 0 0; } .video-js .vjs-tech { object-fit: contain; } .video-js.vjs-fullscreen .vjs-tech { object-fit: contain; } .video-info { padding: 20px; text-align: center; } .video-title { font-size: 18px; color: #333; font-weight: 500; } @media (max-width: 768px) { body { padding: 15px; } h1 { font-size: 24px; } .video-info { padding: 15px; } .video-title { font-size: 16px; } } @media (max-width: 480px) { .container { gap: 15px; } h1 { font-size: 22px; } }</style></head><body><div class=\"container\"><div class=\"header\"><h1>";
  message += videoName;
  message += "</h1></div><div class=\"video-card\"><div class=\"video-player-container\"><video id=\"video_demo\" class=\"video-js vjs-default-skin vjs-big-play-centered\" controls preload=\"auto\" poster=\"";
  message += videoPath;
  message += "/0.jpg\" data-setup=\"{}\"><source src=\"";
  message += videoPath;
  message += "/index.m3u8\" type=\"application/x-mpegURL\"></video></div></div></div><script src=\"/bin/videojs/7.5.5/video.min.js\"></script><script src=\"/bin/videojs/videojs-contrib-hls/videojs-contrib-hls.min.js\"></script><script>document.addEventListener('DOMContentLoaded', function() { var player = videojs('video_demo', { \"controls\": true, \"autoplay\": false, \"fluid\": true, \"responsive\": true }); player.ready(function() { console.log('视频播放器已初始化'); player.on('fullscreenchange', function() { if (player.isFullscreen()) { console.log('进入全屏模式'); } else { console.log('退出全屏模式'); } }); }); });</script></body></html>";

  request->send(200, "text/html", message);  //发送网页
}
