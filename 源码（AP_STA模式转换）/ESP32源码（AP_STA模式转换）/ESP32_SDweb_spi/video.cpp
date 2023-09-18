#include"video.h"

extern WebServer esp32_server;
extern String IPAD;

//读取txt文件
String readFile(fs::FS &fs, const char * path){
  int i=0;
  char test1[200];
  String message="";

  File file = fs.open(path);
  if(!file){
      Serial.println("Failed to open file for reading");
      return message;
  }

  while(file.available()){
      if(i<192)
      {
          test1[i] = file.read();
          i++;
      }
      else{
        break;
      }
  }
  file.close();
  test1[i]='\0';
  message = test1;
  return message;
}

//列出内存卡中的视频
String listVideoDir(fs::FS &fs, const char * dirname, const char * page,const String tapeName)
{

  String message="";
  String namepath="";
  String picpath="";
  String videoName="";
  uint8_t i=1;
  char page1=((page[0]&0x0f)-1)*20;     //设定分页区间，现在是每20个视频一页
  char page2=(page[0]&0x0f)*20+1;
  File root = fs.open(dirname);
  if(!root){
      message += "Failed to open directory <br />";
      return message;
  }
  if(!root.isDirectory()){
      message += "Not a directory <br />";
      return message;
  }
  File file = root.openNextFile();
  while(file){
      if(file.isDirectory() && i>page1 && i<page2){

          namepath=String(file.path()) + "/0.txt";      //视频标题路径
          picpath=String(file.path()) + "/0.jpg";       //视频预览图路径
          videoName = readFile(SD,(char*)namepath.c_str());     //读取视频标题
          message += "<img src=\"" + picpath + "\" alt=\"" + videoName + "\" width=auto height=\"200px\"/>";   //配置预览图大小
          message += "<form action=\"/openvideo\">  ";
          message += videoName;
          message += "<input type=\"hidden\" name=\"videoPath\" value=\"http://" + IPAD + String(file.path()) + "\">";
          message += ": <input type=\"submit\" value=\"play\"><br />";
          message += "</form>";
          namepath="";
          picpath="";

      }
      file = root.openNextFile();
      i++;
  }

  page1 = (i+18)/20; 
  message += "<br/><br/><form action=\"/videolist\"><input type=\"hidden\" name=\"videoTape\" value=\"" + String(dirname) + "\">";
  message += "<input type=\"hidden\" name=\"tapeName\" value=\"" + tapeName + "\">page: ";
  
  for(i=1;i<=page1;i++){
      message += "<input type=\"submit\" name=\"page\" value=\"";
      message += i;
      message += "\">  ";  
  }
  message += "</form>page: ";
  message += page;
   
  return message;
}

void listvideo() {

  String videoTape = esp32_server.arg("videoTape"); //获取视频分区路径
  String page = esp32_server.arg("page");           //获取页数
  String tapeName = esp32_server.arg("tapeName");   //获取视频分区名称
  String message = "<html><head><meta charset=\"UTF-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, minimum-scale=0.5, maximum-scale=2.0, user-scalable=yes\" /></head><body><h2>" + tapeName + "</h2>";
  message += listVideoDir(SD,(char*)videoTape.c_str(),(char*)page.c_str(),tapeName);
  message += "</body></html>";
  esp32_server.send(200,"text/html",message); 
} 

//打开视频
void openVideo()
{
  String videoPath = esp32_server.arg("videoPath"); //获取视频路径
  uint8_t len = videoPath.length();
  uint8_t cout = 0;
  uint8_t i,j;

  //提取出视频路径
  for(i=0;i<=len;i++)
  {
      if(videoPath[i]=='/')
      {
          cout++;
          if(cout == 3)
          {
              break;
          } 
      }   
  }
  char namepath2[len-i+2];
  cout=len-i+1;
  for(j=0;j<cout;j++)
  {
      namepath2[j]=videoPath[i];
      i++;
  }
  String namepath = namepath2;
  namepath += "/0.txt";

  //得到回车字符串
  char test1[3];
  String test2 = "";
  test1[0]='\r';
  test1[1]='\n';
  test1[2]='\0';
  test2 = test1;

  //调用videoJS
  String message = "<head><meta charset=\"UTF-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, minimum-scale=0.5, maximum-scale=2.0, user-scalable=yes\" /><title>Theater</title><link href=\"http://" + IPAD + "/video/bin/video-js.min.css\" rel=\"stylesheet\"><script src=\"http://" + IPAD + "/video/bin/video.min.js\"></script><script src=\"http://" + IPAD + "/video/bin/videojs-contrib-hls.js\"></script></head><body><center><h2>";
  message += readFile(SD,(char*)namepath.c_str());
  message += "</h2><section id=\"videoPlayer\"></section></center><script type=\"text/javascript\"> function createvideo(url) {let str = `";
  message += test2;
  message += "<video id=\"video_demo\" autoplay width=\"\" class=\"video-js vjs-default-skin vjs-big-play-centered\" poster=\"\"><source src=\"\" type=\"application/x-mpegURL\" id=\"target\"></video>";
  message += test2;
  message += "` ";

  message += test2;
  message += "let wrap = document.getElementById('videoPlayer');wrap.innerHTML = str;let source = document.getElementById('target');source.src = url ? url : '";

  message += videoPath;
  
  message += "/index.m3u8';var player = videojs('video_demo', { \"poster\": \"\", \"controls\": \"true\" },function() {this.on('play', function() {console.log('正在播放');});this.on('pause', function() {console.log(\"暂停中\")});this.on('ended', function() {console.log('结束');})});player.play()}createvideo()</script></body></html>"; 

  esp32_server.send(200, "text/html", message); //发送网页
}
