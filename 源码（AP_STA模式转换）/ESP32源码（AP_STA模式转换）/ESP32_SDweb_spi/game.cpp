#include"game.h"

extern WebServer esp32_server;

//列出内存卡的游戏
String listGameDir(fs::FS &fs, const char * dirname, uint8_t levels)
{
  
  String filename = "";
  String filename2 = "";
  String message="";
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
      if(file.isDirectory()){
          message +="  DIR : ";
          message += String(file.path())+String("<br />");
          if(levels){
              message += listGameDir(fs, file.path(), levels -1);
          }
      } else {

          filename = String(file.path());
          //filename2 = indexOfFilename(filename);
          filename2 = String(file.name());
          
          message += "<form action=\"/game\">  FILE: " + filename2 + " <input type=\"hidden\" name=\"gamePath\" value=\"" + filename + "\"><input type=\"submit\" value=\"play\"><br />";
          message += "  SIZE: ";
          message += formatBytes(file.size()) + "<br /></form>";
      }
      file = root.openNextFile();
  }
  return message;
}

void listGame() {
  String folder = esp32_server.arg("folder");   //获取游戏文件夹路径
  String header = "<html><body>";
  String message= header + "<h2>" + folder + ":</h2>";
  message += listGameDir(SD,(char*)folder.c_str(),1);
  esp32_server.send(200,"text/html",message); 
} 

//打开游戏
void openGame() {
  String gamePath = esp32_server.arg("gamePath");   //获取游戏路径
  String message = "<!DOCTYPE html><html><head><title>game</title><body><center><br />";
  message += "<object class=\"ObjectyMe\" uri=\"";
  message += gamePath; 
  message += "\" width=\"800\" height=\"800\"></object></center><br />";  //设置游戏窗口大小
  message += "<script type=\"text/javascript\" src=\"/webgame/objecty/objecty.js\"></script></body></html>";

  esp32_server.send(200, "text/html", message); //发送网页
}
