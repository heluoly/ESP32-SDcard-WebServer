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
extern char ssid_hidden;
extern char autoconnect;
extern String pressid;
extern String prepassword;

String htmlHeader = "<!DOCTYPE html><html lang=\"zh-CN\"><head><meta charset=\"UTF-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, minimum-scale=0.5, maximum-scale=2.0, user-scalable=yes\">";

TaskHandle_t Task_Sntp;  //网络时间同步任务

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
void WiFiconfigRead()
{
  char buff[128];
  if(configRead(SD_MMC,"ssid","/config.txt",buff))
  {
    ssid=buff;
  }
  if(configRead(SD_MMC,"password","/config.txt",buff))
  {
    password=buff;
  }
  if(configRead(SD_MMC,"channel","/config.txt",buff))
  {
    channel=String2Char((char*)buff);
  }
  if(configRead(SD_MMC,"autoconnect","/config.txt",buff))
  {
    autoconnect=String2Char((char*)buff);
  }
    if(configRead(SD_MMC,"pressid","/config.txt",buff))
  {
    pressid=buff;
  }
  if(configRead(SD_MMC,"prepassword","/config.txt",buff))
  {
    prepassword=buff;
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
  WiFi.softAP((char*)ssid.c_str(), (char*)password.c_str(), channel, ssid_hidden, 4);  //启动AP模式
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
  // Serial.println("HTTP server started");

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
  server.on("/pageConfigAutoConnect", pageConfigAutoConnect);   //发送配置WiFi自动连接网页
  server.on("/configAutoConnect", configAutoConnect);   //保存iFi自动连接配置
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
  // Serial.println('\n');
  // Serial.print("Connected to ");
  // Serial.println(WiFi.SSID());              // 通过串口监视器输出连接的WiFi名称
  // Serial.print("IP address:\t");
  // Serial.println(WiFi.localIP());           // 通过串口监视器输出ESP32-NodeMCU的IP

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
  // Serial.println("HTTP server started");

  xTaskCreatePinnedToCore(task_sntp, "Task_Sntp", 2048, NULL, 5, &Task_Sntp, 0);   //创建网络时间同步任务

  while(mode_switch)    //监听用户请求，直到模式转换
  {
    esp32_server.handleClient();                    // 处理用户请求
    vTaskDelay(2/portTICK_PERIOD_MS);
  }
  
  mode_switch=1;
  esp32_server.close();   //关闭网站服务
  
}

void server_presta(){
  char flag_ok=0;
  mode_wifi = 4;
  WiFi.mode(WIFI_STA);

  //尝试连接上次成功连接WIFI
  WiFi.begin((char*)pressid.c_str(), (char*)prepassword.c_str());
  for (int i = 0; i < 20; i++)        //超时判断
  {
    if (WiFi.status() == WL_CONNECTED)    //如果检测到状态为成功连接WIFI
    {
      flag_ok=1;
      break;
    }
    else
    {
      vTaskDelay(500/portTICK_PERIOD_MS);
    }
  }

  if(flag_ok)
  {
    mode_wifi = 3;
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
    // Serial.println("HTTP server started");

    xTaskCreatePinnedToCore(task_sntp, "Task_Sntp", 2048, NULL, 5, &Task_Sntp, 0);   //创建网络时间同步任务

    while(mode_switch)    //监听用户请求，直到模式转换
    {
      esp32_server.handleClient();                    // 处理用户请求
      vTaskDelay(2/portTICK_PERIOD_MS);
    }
    
    mode_switch=1;
    esp32_server.close();   //关闭网站服务

  }
 
}





