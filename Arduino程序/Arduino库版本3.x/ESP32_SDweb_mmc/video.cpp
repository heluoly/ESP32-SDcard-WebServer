#include "video.h"

extern WebServer esp32_server;
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
void listvideo() {
  String videoTape = esp32_server.arg("videoTape");  //获取视频分区路径
  String page = esp32_server.arg("page");            //获取页数
  String tapeName = esp32_server.arg("tapeName");    //获取视频分区名称
  String namepath = "";
  String picpath = "";
  String videoName = "";
  uint8_t i = 1;
  char page0 = String2Char((char *)page.c_str());
  char page1 = (page0 - 1) * 20;  //设定分页区间，现在是每20个视频一页
  char page2 = page0 * 20 + 1;
  String message = htmlHeader + "<style>.container {width: 500px;margin: 0 auto;}</style></head><body><div class=\"container\"><h2>" + tapeName + "</h2>";

  File root = SD_MMC.open((char *)videoTape.c_str());
  if (!root) {
    message += "Failed to open directory <br />";
  } else if (!root.isDirectory()) {
    message += "Not a directory <br />";
  } else {
    File file = root.openNextFile();
    while (file) {
      if (file.isDirectory() && i > page1 && i < page2) {

        namepath = String(file.path()) + "/0.txt";               //视频标题路径
        picpath = String(file.path()) + "/0.jpg";                //视频预览图路径
        videoName = readFile(SD_MMC, (char *)namepath.c_str());  //读取视频标题
        message += "<a href=\"/openvideo?videoPath=" + String(file.path());
        message += "\"><img src=\"" + picpath + "\" alt=\"" + videoName + "\" width=auto height=\"200px\"/></a>";  //配置预览图大小
        message += "<form action=\"/openvideo\">  ";
        message += videoName;
        message += "<input type=\"hidden\" name=\"videoPath\" value=\"" + String(file.path()) + "\">";
        message += ": <input type=\"submit\" value=\"播放\">";
        message += "</form><br />";
        namepath = "";
        picpath = "";
      }
      file = root.openNextFile();
      i++;
    }

    page1 = (i + 18) / 20;
    message += "<form action=\"/videolist\"><input type=\"hidden\" name=\"videoTape\" value=\"" + videoTape + "\">";
    message += "<input type=\"hidden\" name=\"tapeName\" value=\"" + tapeName + "\">page: ";

    for (i = 1; i <= page1; i++) {
      message += "<input type=\"submit\" name=\"page\" value=\"";
      message += i;
      message += "\">  ";
    }
    message += "</form>page: ";
    message += page;
  }

  message += "</div></body></html>";
  esp32_server.send(200, "text/html", message);
}

//打开视频
void openVideo() {
  String videoPath = esp32_server.arg("videoPath");  //获取视频路径
  String namepath = videoPath;
  namepath += "/0.txt";

  //得到回车字符串
  char enter[3];
  String enter2 = "";
  enter[0] = '\r';
  enter[1] = '\n';
  enter[2] = '\0';
  enter2 = enter;

  //调用videoJS
  String message = htmlHeader + "<title>Theater</title><link href=\"" + "/video/bin/video-js.min.css\" rel=\"stylesheet\"><script src=\"" + "/video/bin/video.min.js\"></script><script src=\"" + "/video/bin/videojs-contrib-hls.js\"></script></head><body><center><h2>";
  message += readFile(SD_MMC, (char *)namepath.c_str());  //读取视频标题
  message += "</h2><section id=\"videoPlayer\"></section></center><script type=\"text/javascript\"> function createvideo(url) {let str = `";
  message += enter2;
  message += "<video id=\"video_demo\" autoplay width=\"\" class=\"video-js vjs-default-skin vjs-big-play-centered\" poster=\"\"><source src=\"";
  message += videoPath;
  message += "/index.m3u8\" type=\"application/x-mpegURL\" id=\"target\"></video>";
  message += enter2;
  message += "` ";
  message += enter2;
  message += "let wrap = document.getElementById('videoPlayer');wrap.innerHTML = str;let source = document.getElementById('target');";
  message += "var player = videojs('video_demo', { \"poster\": \"\", \"controls\": \"true\" });player.play()}createvideo()</script></body></html>";

  esp32_server.send(200, "text/html", message);  //发送网页
}
