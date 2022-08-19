/*
ESP32_SD卡服务器
作者：B站 狼尾巴的猫
项目实现文件上传下载、网页flash游戏、播放视频、模式转换（AP STA AP+STA互相转换）、剪切板功能
SD卡代码参考 https://youtu.be/QAbn-7Ai6UU
文件上传、网页响应代码参考 http://www.taichi-maker.com/homepage/esp8266-nodemcu-iot/iot-c/spiffs/spiffs-web-server/file-upload-server/
flash播放器使用objecty https://wiltgen.net/objecty/
视频播放器使用videoJS
网页配网代码参考 https://github.com/yuan910715/Esp8266_NTP_Clock_Weather中的网页配网部分
*/

#include <WiFi.h>
//#include <WiFiMulti.h>
#include <WebServer.h> 
#include "FS.h"
#include "SD_MMC.h"
#include"common.h"
#include"server.h"
#include"game.h"
#include"upload.h"
#include"video.h"
#include"web.h"
#include"copy.h"
#include"wifiConnect.h"

bool hasSD = false;  //是否有SD卡
bool ONE_BIT_MODE = true;  //设置SD卡模式 1bit：true 4bit：false

WebServer esp32_server(80);    // 建立网络服务器对象，该对象用于响应HTTP请求。监听端口（80）
WebServer server(80);         //WIFI配网
File fsUploadFile;              // 建立文件对象用于闪存文件上传

char mode_switch = 1;    //用于控制模式变换中的跳出while循环
char mode_switch2 = 1;    //用于跳过STA模式，转换到AP模式

String IPAD = "192.168.1.1";    //在AP和STA模式下存储ESP32的IP地址
String ssid = "ESP32_WebServer";   //wifi名称
String password = "123456789";     //wifi密码（注意WiFi密码位数不要小于8位）


void setup() {
  Serial.begin(115200);          // 启动串口通讯
  Serial.println("");
  
  if(!SD_MMC.begin("/sdcard", ONE_BIT_MODE))  //SD卡初始化
  {
    Serial.println("Card Mount Failed");
    return;
  }
  else
  {
    Serial.println("SD Card Ready!");
    hasSD=true;
  }
  uint8_t cardType = SD_MMC.cardType();
  if(cardType == CARD_NONE){
    Serial.println("No SD card attached");
    return;
  }

  Serial.print("SD Card Type: ");   //输出SD卡信息
  if(cardType == CARD_MMC){
    Serial.println("MMC");
  } else if(cardType == CARD_SD){
    Serial.println("SDSC");
  } else if(cardType == CARD_SDHC){
    Serial.println("SDHC");
  } else {
    Serial.println("UNKNOWN");
  }
  uint64_t cardSize = SD_MMC.cardSize() / (1024 * 1024);
  Serial.printf("SD Card Size: %lluMB\n", cardSize);
  listDir(SD_MMC, "/", 0);
  Serial.printf("Total space: %lluMB\n", SD_MMC.totalBytes() / (1024 * 1024));
  Serial.printf("Used space: %lluMB\n", SD_MMC.usedBytes() / (1024 * 1024));

  if(hasSD)
    readFile3(SD_MMC, "/password.txt");   //读取保存的AP名称和密码
  
}


void loop(void) {
  server_ap();
  server_ap_sta();
  if(mode_switch2)
    server_sta();
  mode_switch2=1;
}
