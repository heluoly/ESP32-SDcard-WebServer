#include "game.h"

extern WebServer esp32_server;
extern String htmlHeader;

//列出内存卡的游戏
void listGame() {
  String folder = esp32_server.arg("folder");  //获取游戏文件夹路径
  String page = esp32_server.arg("page");      //获取页数
  String filename = "";
  String filename2 = "";
  uint8_t i = 1;
  char pageState = 0;
  const char pageBreak = 20;  //设定分页区间，现在是每20个视频一页
  char page0 = String2Char((char*)page.c_str());
  char page1 = (page0 - 1) * 20;  //设定分页区间，现在是每20个视频一页
  char page2 = page0 * 20 + 1;

  String message = htmlHeader + "<style>.container {width: 500px;margin: 0 auto;}</style><body><div class=\"container\"><h2>";
  message += folder + ":</h2>";

  File root = SD_MMC.open((char*)folder.c_str());
  if (!root) {
    message += "Failed to open directory <br />";
  } else if (!root.isDirectory()) {
    message += "Not a directory <br />";
  } else {
    File file = root.openNextFile();
    while (file) {
      if (!file.isDirectory() && i > page1 && i < page2) {
        filename = String(file.path());
        //filename2 = indexOfFilename(filename);
        filename2 = String(file.name());

        message += "<form action=\"/opengame\">  FILE: " + filename2 + " <input type=\"hidden\" name=\"gamePath\" value=\"" + filename + "\"><input type=\"submit\" value=\"play\"><br />";
        message += "  SIZE: ";
        message += formatBytes(file.size()) + "</form><br />";
      }
      file = root.openNextFile();
      i++;
    }

    page1 = (i + pageBreak - 2) / pageBreak;
    message += "<form action=\"/gamelist\"><input type=\"hidden\" name=\"folder\" value=\"" + folder + "\">";
    message += "页数: ";

    for (i = 1; i <= page1; i++) {
      message += "<input type=\"submit\" name=\"page\" value=\"";
      message += i;
      message += "\">  ";
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
          message += "</form>";
          break;
        }
      case 1:
        {
          message += " <button type=\"submit\" name=\"page\" value=\"";
          message += page0 + 1;
          message += "\">下一页</button></form>";
          break;
        }
      case 2:
        {
          message += " <button type=\"submit\" name=\"page\" value=\"";
          message += page0 - 1;
          message += "\">上一页</button></form>";
          break;
        }
      case 3:
        {
          message += " <button type=\"submit\" name=\"page\" value=\"";
          message += page0 - 1;
          message += "\">上一页</button> <button type=\"submit\" name=\"page\" value=\"";
          message += page0 + 1;
          message += "\">下一页</button></form>";
          break;
        }
      default:
        {
          message += "</form>";
        }
    }
  }

  message += "</div></body></html>";
  esp32_server.send(200, "text/html", message);
}

//打开游戏
void openGame() {
  String gamePath = esp32_server.arg("gamePath");  //获取游戏路径
  String message = "<!DOCTYPE html><html><head><title>game</title><body><center><br />";
  message += "<object class=\"ObjectyMe\" uri=\"";
  message += gamePath;
  message += "\" width=\"800\" height=\"600\"></object></center><br />";  //设置游戏窗口大小
  message += "<script type=\"text/javascript\" src=\"/webgame/objecty/objecty.js\"></script></body></html>";

  esp32_server.send(200, "text/html", message);  //发送网页
}
