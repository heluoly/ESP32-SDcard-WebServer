#include "server.h"

// extern TaskHandle_t Task_Server;

extern WebServer esp32_server;    //网页服务
extern WebServer server;         //WIFI配网

extern bool mode_switch;
extern bool mode_switch2;
extern char mode_wifi;

extern String IPAD;
extern String ssid;
extern String password;
extern char channel;

String htmlHeader = "<!DOCTYPE html><html lang=\"zh-CN\"><head><meta charset=\"UTF-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, minimum-scale=0.5, maximum-scale=2.0, user-scalable=yes\">";


//模式转换
void changemode(){
  mode_switch=0;    //使下面的函数跳出while循环，从而在loop函数中进入下一个模式
}

// void changemode2(){
//   // WiFi.disconnect(true,true);   //断开WiFi
//   mode_switch=0;    //使下面的函数跳出while循环，从而在loop函数中进入下一个模式
// }

//返回AP模式
void backToAP(){
  // WiFi.disconnect(true,true);   //断开WiFi
  mode_switch2=0;   //跳过STA模式，直接进入AP模式
  mode_switch=0;    //使下面的函数跳出while循环，从而在loop函数中进入下一个模式
}


//读取SD卡中保存的热点名称和密码
void readFile3(fs::FS &fs, const char * path){
  char i=0,j=0;
  char flag_hotspot=1;
  char flag_OK=0;
  char hotspotName[64];
  char hotspotPassword[64];
  char hotspotChannel[5];

  File file = fs.open(path);
  if(!file){
      Serial.println("Failed to open file for reading");
      return;
  }
  while(file.available()){
    if(flag_hotspot == 1)
    {
      if(j<63){
        hotspotName[j] = file.read(); //读取热点名称
        if(hotspotName[j]=='\r'){
          i++;
          hotspotName[j]='\0';
        }
        else if(hotspotName[j]=='\n'){  //以回车键作为划分名称和密码的标志
          hotspotName[j]='\0';
          flag_hotspot=2;
          j=0;
        }
        else{
          i++;
          j++;
        }
      }
      else{
        break;  //如果超过63个字符，则名称过长，退出循环
      }
    }
    else if(flag_hotspot == 2){
      if(j<63){
        hotspotPassword[j] = file.read();  //读取热点密码
        if(hotspotPassword[j]=='\r'){
          i++;
          hotspotPassword[j]='\0';
        }
        else if(hotspotPassword[j]=='\n'){  //以回车键作为划分密码和信道的标志
          hotspotPassword[j]='\0';
          flag_hotspot=3;
          j=0;
          flag_OK=1;    //获取到合适的热点名称
        }
        else{
          i++;
          j++;
        }
      }
      else{
        break;  //如果超过63个字符，则密码过长，退出循环
      }
    }
    
    else{
      if(j<3){
        hotspotChannel[j] = file.read();  //读取热点信道
        if(hotspotChannel[j]=='\r'){
          break;
        }
        else{
          i++;
          j++;
        }
      }
      else{
        break;  //如果超过2个字符，则信道错误，退出循环
      }
    }
  }
  hotspotChannel[j]='\0';
  file.close();
  if(flag_OK){
    ssid=hotspotName;
    password=hotspotPassword;
    channel=String2Char((char*)hotspotChannel);
    if(channel>13 || channel<1){
      channel = 1;
    }
    // Serial.println("hotspotName:");
    // Serial.println(hotspotName);
    // Serial.println("hotspotPassword:");
    // Serial.println(hotspotPassword);
//    Serial.println("hotspotChannel:");
//    Serial.println(channel);
    
  }
  else{
    return;
  }
}

//AP模式
void server_ap(){
  // UBaseType_t istack;
  mode_wifi = 1;
  WiFi.disconnect(true,true);
  WiFi.mode(WIFI_AP);
  //WiFi.softAPdisconnect(true);
  IPAddress local_IP(192, 168, 1, 1);  //配置ESP32的IP地址
  IPAddress gateway(192, 168, 1, 1);    //配置ESP32的网关
  IPAddress subnet(255, 255, 255, 0);   //配置ESP32的子网
 
  WiFi.softAPConfig(local_IP,gateway,subnet);
//WiFi.softAP(ssid, passphrase, channel, ssid_hidden, max_connection)
  WiFi.softAP((char*)ssid.c_str(), (char*)password.c_str(), channel, 0, 4);  //启动AP模式
//  Serial.println(ssid);
//  Serial.println(password);

  // Serial.print("主机名:");
  // Serial.println(WiFi.softAPgetHostname());
  // Serial.print("主机IP:");
  // Serial.println(WiFi.softAPIP());
  // Serial.print("主机SSID:");
  // Serial.println(WiFi.SSID());
  // Serial.print("主机广播IP:");
  // Serial.println(WiFi.softAPBroadcastIP());
  // Serial.print("主机mac地址:");
  // Serial.println(WiFi.softAPmacAddress());
  // Serial.print("主机连接个数:");
  // Serial.println(WiFi.softAPgetStationNum());

  IPAD = WiFi.softAPIP().toString();  //将当前IP地址存储起来

  esp32_server.onNotFound(handleUserRequet);      // 告知系统如何处理用户请求
  esp32_server.on("/gamelist", HTTP_GET, listGame);   //列出游戏列表
  esp32_server.on("/opengame", HTTP_GET, openGame);   //打开游戏
  esp32_server.on("/upload",   // 如果客户端通过upload页面
          HTTP_POST,        // 向服务器发送文件(请求方法POST)
          respondOK,        // 则回复状态码 200 给客户端
          handleFileUpload);// 并且运行处理文件上传函数
  esp32_server.on("/filelist", HTTP_GET, listUploadFile);   //列出文件上传列表
  esp32_server.on("/deleteUploadFile", HTTP_GET, deleteUploadFile);   //删除文件
  esp32_server.on("/downloadUploadFile", HTTP_GET, downloadUploadFile);   //下载文件
  esp32_server.on("/videolist", listvideo);   //列出视频列表
  esp32_server.on("/openvideo",HTTP_GET,openVideo);   //打开视频
  esp32_server.on("/edittxt",HTTP_GET,editTxt);   //编辑txt文件
  esp32_server.on("/clipboard",HTTP_GET,clipBoard);   //剪切板
  esp32_server.on("/wificonnect",changemode);   //模式转换
  esp32_server.on("/setTime",setTime);   //设置时间

  esp32_server.begin();                           // 启动网站服务
  Serial.println("HTTP server started");

  while(mode_switch)    //监听用户请求，直到模式转换
  {
    esp32_server.handleClient();                    // 处理用户请求
    // istack = uxTaskGetStackHighWaterMark(Task_Server);
    // printf("Task_Server istack = %d\n", istack);
    vTaskDelay(2/portTICK_PERIOD_MS);
  }
  
  mode_switch=1;
  esp32_server.close();   //关闭网站服务
}

void server_ap_sta(){
  mode_wifi = 2;
  WiFi.mode(WIFI_AP_STA);
  /*
  IPAddress local_IP(192, 168, 1, 1);  
  IPAddress gateway(192, 168, 1, 1);
  IPAddress subnet(255, 255, 255, 0);
 
  WiFi.softAPConfig(local_IP,gateway,subnet);
  WiFi.softAP((char*)ssid.c_str(), (char*)password.c_str());
*/
  server.on("/wificonnect", handleRoot);    //发送配网页面
  server.on("/HandleWifi", HTTP_GET, HandleWifi);   //尝试连接网页发送的WIFI
  server.on("/HandleScanWifi", HandleScanWifi);   //扫描附近WIFI并返回
  server.on("/configAP", configAP);         //配置热点
  server.on("/pageConfigAP", pageConfigAP);   //发送配置热点网页
  server.on("/", backToAP);   //返回AP模式
  server.onNotFound(wifi_handleNotFound);//请求失败回调函数
  server.begin();                           // 启动网站服务

  while (mode_switch)
  {
    server.handleClient();
    vTaskDelay(2/portTICK_PERIOD_MS);
  }
  mode_switch=1;
  server.close();
  
}

void server_sta(){
  mode_wifi = 3;
  WiFi.softAPdisconnect(true);
  WiFi.mode(WIFI_STA);
  Serial.println('\n');
  Serial.print("Connected to ");
  Serial.println(WiFi.SSID());              // 通过串口监视器输出连接的WiFi名称
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP());           // 通过串口监视器输出ESP32-NodeMCU的IP

  IPAD = WiFi.localIP().toString();

  esp32_server.onNotFound(handleUserRequet);      // 告知系统如何处理用户请求
  esp32_server.on("/gamelist", HTTP_GET, listGame);   //列出游戏列表
  esp32_server.on("/opengame", HTTP_GET, openGame);    //打开游戏
  esp32_server.on("/upload.html",   // 如果客户端通过upload页面
          HTTP_POST,        // 向服务器发送文件(请求方法POST)
          respondOK,        // 则回复状态码 200 给客户端
          handleFileUpload);// 并且运行处理文件上传函数
  esp32_server.on("/filelist", HTTP_GET, listUploadFile);   //列出文件上传列表
  esp32_server.on("/deleteUploadFile", HTTP_GET, deleteUploadFile);   //删除文件
  esp32_server.on("/downloadUploadFile", HTTP_GET, downloadUploadFile);   //下载文件
  esp32_server.on("/videolist", listvideo);    //列出视频列表
  esp32_server.on("/openvideo",HTTP_GET,openVideo);   //打开视频
  esp32_server.on("/edittxt",HTTP_GET,editTxt);   //编辑txt文件
  esp32_server.on("/clipboard",HTTP_GET,clipBoard);   //剪切板
  esp32_server.on("/wificonnect",changemode);   //模式转换
  esp32_server.on("/setTime",setTime);   //设置时间

  esp32_server.begin();                           // 启动网站服务
  Serial.println("HTTP server started");

  while(mode_switch)    //监听用户请求，直到模式转换
  {
    esp32_server.handleClient();                    // 处理用户请求
    vTaskDelay(2/portTICK_PERIOD_MS);
  }
  
  mode_switch=1;
  esp32_server.close();   //关闭网站服务
  
}
