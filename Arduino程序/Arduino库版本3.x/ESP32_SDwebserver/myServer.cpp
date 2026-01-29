#include "myServer.h"

// extern TaskHandle_t Task_Server;

extern WebServer esp32_server;  //网页服务
extern WebServer server;        //WIFI配网

extern bool hasSD;
extern bool ONE_BIT_MODE;

extern bool mode_switch;
extern bool mode_switch2;
extern char mode_wifi;
extern bool serverState;

extern String IPAD;
extern String ssid;
extern String password;
extern char channel;
extern char ssid_hidden;
extern char autoconnect;
extern String pressid;
extern String prepassword;

const String htmlHeader = "<!DOCTYPE html><html lang=\"zh-CN\"><head><meta charset=\"UTF-8\"><meta name=\"viewport\" content=\"width=device-width\">";

TaskHandle_t Task_Sntp;  //网络时间同步任务

//模式转换
void changemode() {
  esp32_server.sendHeader("Refresh", "1; url=/wificonnect");  //等待1秒刷新
  esp32_server.sendHeader("Connection", "close");
  esp32_server.send(200, "text/html", htmlHeader + "</head><body><center><h2>请稍等</h2></center></body></html>");
  mode_switch = 0;  //使下面的函数跳出while循环，从而在loop函数中进入下一个模式
}

//返回AP模式
void backToAP() {
  server.sendHeader("Refresh", "1; url=/");  //等待1秒刷新
  server.sendHeader("Connection", "close");
  server.send(200, "text/html", htmlHeader + "</head><body><center><h2>请稍等</h2></center></body></html>");
  mode_switch2 = 0;  //跳过STA模式，直接进入AP模式
  mode_switch = 0;   //使下面的函数跳出while循环，从而在loop函数中进入下一个模式
}

//读取SD卡中保存的热点名称和密码
void WiFiconfigRead() {
  char buff[CONFIG_MAX_LENGTH];
  if (configRead(config_fs, "ssid", "/config.txt", buff, CONFIG_MAX_LENGTH)) {
    ssid = buff;
  }
  if (configRead(config_fs, "password", "/config.txt", buff, CONFIG_MAX_LENGTH)) {
    password = buff;
  }
  if (configRead(config_fs, "channel", "/config.txt", buff, CONFIG_MAX_LENGTH)) {
    channel = String2Char((char *)buff);
  }
  if (configRead(config_fs, "autoconnect", "/config.txt", buff, CONFIG_MAX_LENGTH)) {
    autoconnect = String2Char((char *)buff);
  }
  if (configRead(config_fs, "pressid", "/config.txt", buff, CONFIG_MAX_LENGTH)) {
    pressid = buff;
  }
  if (configRead(config_fs, "prepassword", "/config.txt", buff, CONFIG_MAX_LENGTH)) {
    prepassword = buff;
  }
}

//AP模式
void server_ap() {
  // UBaseType_t istack;
  mode_wifi = 1;
  // WiFi.setAutoReconnect(false);
  WiFi.disconnect(true, true);
  // WiFi.softAPdisconnect(true);
  WiFi.mode(WIFI_AP);
  
  IPAddress local_IP(192, 168, 1, 1);  //配置ESP32的IP地址
  IPAddress gateway(192, 168, 1, 1);   //配置ESP32的网关
  IPAddress subnet(255, 255, 255, 0);  //配置ESP32的子网

  WiFi.softAPConfig(local_IP, gateway, subnet);
  //WiFi.softAP(ssid, passphrase, channel, ssid_hidden, max_connection)
  WiFi.softAP((char *)ssid.c_str(), (char *)password.c_str(), channel, ssid_hidden, 4);  //启动AP模式
  // Serial.println(ssid);
  // Serial.println(password);
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

  esp32_server.onNotFound(handleUserRequest);                                  //fallback函数
  esp32_server.on("/filelist", HTTP_GET, listUploadFile);                      //列出文件
  esp32_server.on("/downloadUploadFile", HTTP_GET, downloadUploadFile);        //下载文件
  esp32_server.on("/deleteUploadFile", HTTP_GET, deleteUploadFile);            //删除文件
  esp32_server.on("/upload", HTTP_POST, uploadFileRespond, handleFileUpload);  //上传文件
  esp32_server.on("/videolist", HTTP_GET, listvideo);                          //列出视频列表
  esp32_server.on("/openvideo", HTTP_GET, openVideo);                          //打开视频
  esp32_server.on("/gamelist", HTTP_GET, listGame);                            //列出游戏列表
  esp32_server.on("/opengame", HTTP_GET, openGame);                            //打开游戏
  esp32_server.on("/saveText", HTTP_POST, handleSaveText);                     //保存文本到txt文件
  esp32_server.on("/getText", HTTP_GET, handleGetText);                        //读取文本到txt文件
  esp32_server.on("/wificonnect", HTTP_GET, changemode);                       //模式转换
  esp32_server.on("/setTime", HTTP_GET, setTime);                              //设置时间

  esp32_server.begin();  // 启动网站服务
  // istack = uxTaskGetStackHighWaterMark(Task_Server);
  // Serial.printf("Task_Server istack = %d\n", istack);

  while (mode_switch)  //监听用户请求，直到模式转换
  {
    esp32_server.handleClient();  // 处理用户请求
    vTaskDelay(2 / portTICK_PERIOD_MS);
  }
  // vTaskDelay(1000 / portTICK_PERIOD_MS);
  mode_switch = 1;
  esp32_server.close();  //关闭网站服务
}

void server_ap_sta() {
  mode_wifi = 2;
  WiFi.mode(WIFI_AP_STA);

  server.onNotFound(wifi_handleNotFound);                                //请求失败回调函数
  server.on("/wificonnect", HTTP_GET, handleRoot);                       //发送配网页面
  server.on("/HandleWifi", HTTP_GET, HandleWifi);                        //尝试连接网页发送的WIFI
  server.on("/HandleScanWifi", HTTP_GET, HandleScanWifi);                //扫描附近WIFI并返回
  server.on("/configAP", HTTP_GET, configAP);                            //配置热点
  server.on("/pageConfigAP", HTTP_GET, pageConfigAP);                    //发送配置热点网页
  server.on("/pageConfigAutoConnect", HTTP_GET, pageConfigAutoConnect);  //发送配置WiFi自动连接网页
  server.on("/configAutoConnect", HTTP_GET, configAutoConnect);          //保存iFi自动连接配置
  server.on("/", backToAP);                                              //返回AP模式

  server.begin();  // 启动网站服务

  while (mode_switch) {
    server.handleClient();
    vTaskDelay(2 / portTICK_PERIOD_MS);
  }
  mode_switch = 1;
  server.close();
}

void server_sta() {
  // UBaseType_t istack;
  mode_wifi = 3;
  WiFi.softAPdisconnect(true);
  // WiFi.setAutoReconnect(true);
  WiFi.mode(WIFI_STA);
  // Serial.println('\n');
  // Serial.print("Connected to ");
  // Serial.println(WiFi.SSID());              // 通过串口监视器输出连接的WiFi名称
  // Serial.print("IP address:\t");
  // Serial.println(WiFi.localIP());           // 通过串口监视器输出ESP32-NodeMCU的IP

  IPAD = WiFi.localIP().toString();

  esp32_server.onNotFound(handleUserRequest);                                  //fallback函数
  esp32_server.on("/filelist", HTTP_GET, listUploadFile);                      //列出文件
  esp32_server.on("/downloadUploadFile", HTTP_GET, downloadUploadFile);        //下载文件
  esp32_server.on("/deleteUploadFile", HTTP_GET, deleteUploadFile);            //删除文件
  esp32_server.on("/upload", HTTP_POST, uploadFileRespond, handleFileUpload);  //上传文件
  esp32_server.on("/videolist", HTTP_GET, listvideo);                          //列出视频列表
  esp32_server.on("/openvideo", HTTP_GET, openVideo);                          //打开视频
  esp32_server.on("/gamelist", HTTP_GET, listGame);                            //列出游戏列表
  esp32_server.on("/opengame", HTTP_GET, openGame);                            //打开游戏
  esp32_server.on("/saveText", HTTP_POST, handleSaveText);                     //保存文本到txt文件
  esp32_server.on("/getText", HTTP_GET, handleGetText);                        //读取文本到txt文件
  esp32_server.on("/wificonnect", HTTP_GET, changemode);                       //模式转换
  esp32_server.on("/setTime", HTTP_GET, setTime);                              //设置时间

  esp32_server.begin();

  xTaskCreatePinnedToCore(task_sntp, "Task_Sntp", 1536, NULL, 5, &Task_Sntp, 0);  //创建网络时间同步任务
  // istack = uxTaskGetStackHighWaterMark(Task_Server);
  // Serial.printf("Task_Server istack = %d\n", istack);

  while (mode_switch)  //监听用户请求，直到模式转换
  {
    esp32_server.handleClient();
    vTaskDelay(2 / portTICK_PERIOD_MS);
  }
  mode_switch = 1;
  esp32_server.close();
}

void server_presta() {
  // UBaseType_t istack;
  char flag_presta = 0;
  mode_wifi = 4;
  WiFi.mode(WIFI_STA);

  //尝试连接上次成功连接WIFI
  WiFi.begin((char *)pressid.c_str(), (char *)prepassword.c_str());
  for (int i = 0; i < 20; i++)  //超时判断
  {
    if (WiFi.status() == WL_CONNECTED)  //如果检测到状态为成功连接WIFI
    {
      flag_presta = 1;
      serverState = 1;
      break;
    } else {
      vTaskDelay(500 / portTICK_PERIOD_MS);
    }
  }

  if (flag_presta) {
    mode_wifi = 3;
    IPAD = WiFi.localIP().toString();

    esp32_server.onNotFound(handleUserRequest);                                  //fallback函数
    esp32_server.on("/filelist", HTTP_GET, listUploadFile);                      //列出文件
    esp32_server.on("/downloadUploadFile", HTTP_GET, downloadUploadFile);        //下载文件
    esp32_server.on("/deleteUploadFile", HTTP_GET, deleteUploadFile);            //删除文件
    esp32_server.on("/upload", HTTP_POST, uploadFileRespond, handleFileUpload);  //上传文件
    esp32_server.on("/videolist", HTTP_GET, listvideo);                          //列出视频列表
    esp32_server.on("/openvideo", HTTP_GET, openVideo);                          //打开视频
    esp32_server.on("/gamelist", HTTP_GET, listGame);                            //列出游戏列表
    esp32_server.on("/opengame", HTTP_GET, openGame);                            //打开游戏
    esp32_server.on("/saveText", HTTP_POST, handleSaveText);                     //保存文本到txt文件
    esp32_server.on("/getText", HTTP_GET, handleGetText);                        //读取文本到txt文件
    esp32_server.on("/wificonnect", HTTP_GET, changemode);                       //模式转换
    esp32_server.on("/setTime", HTTP_GET, setTime);                              //设置时间

    esp32_server.begin();  //启动网站服务

    xTaskCreatePinnedToCore(task_sntp, "Task_Sntp", 1536, NULL, 5, &Task_Sntp, 0);  //创建网络时间同步任务
    // istack = uxTaskGetStackHighWaterMark(Task_Server);
    // Serial.printf("Task_Server istack = %d\n", istack);

    while (mode_switch)  //监听用户请求，直到模式转换
    {
      esp32_server.handleClient();
      vTaskDelay(2 / portTICK_PERIOD_MS);
    }
    mode_switch = 1;
    esp32_server.close();
  }
}


void handleUserRequest() {
  String path = esp32_server.uri();
  // bool fileReadOK = false;
  String contentType = "";

  if (path.endsWith("/")) {  //如果访问地址以"/"为结尾
    path = "/index.html";    //则将访问地址修改为/index.html便于SPIFFS访问
    contentType = "text/html";
  } else if (path.endsWith(".html")) contentType = "text/html";
  else if (path.endsWith(".ts")) contentType = "video/mp2t";
  else if (path.endsWith(".css")) contentType = "text/css";
  else if (path.endsWith(".js")) contentType = "application/javascript";
  else if (path.endsWith(".png")) contentType = "image/png";
  else if (path.endsWith(".gif")) contentType = "image/gif";
  else if (path.endsWith(".jpg")) contentType = "image/jpeg";
  else if (path.endsWith(".ico")) contentType = "image/x-icon";
  else if (path.endsWith(".m3u8")) contentType = "application/x-mpegurl";
  else contentType = "text/plain";

  if (my_fs.exists(path)) {  //如果访问的文件可以在SD卡中找到
    File file = my_fs.open(path, FILE_READ);
    esp32_server.streamFile(file, contentType);
    file.close();
    // fileReadOK = true;
  } else {
    esp32_server.send(404, "text/plain", "Not found");
    // fileReadOK = false;
  }
  
  //检测SD卡意外弹出
  // if (!fileReadOK) {
  //   my_fs.end();
  //   if (my_fs.begin("/sdcard", ONE_BIT_MODE)) {  //SD卡初始化
  //     if (!hasSD) {
  //       request->send(404, "text/plain", "Card Mount Succeed");  //如果在SD卡初始化成功，则回复Card Mount Succeed
  //       hasSD = true;
  //     } else {
  //       request->send(404, "text/plain", "Not found");  //如果在SD卡无法找到用户访问的资源，则回复404 Not Found
  //     }

  //   } else {
  //     hasSD = false;
  //     request->send(404, "text/plain", "Card Mount Failed");  // 如果无法读取SD卡，则回复Card Mount Failed
  //   }
  // }
}

