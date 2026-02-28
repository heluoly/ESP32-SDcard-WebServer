#include "myServer.h"

// extern TaskHandle_t Task_Server;

extern AsyncWebServer esp32_server;  //网页服务

extern bool hasSD;
extern bool ONE_BIT_MODE;

extern bool mode_switch;
extern bool isServerInitialized;

extern String APIPAD;
extern String STAIPAD;
extern String ssid;
extern String password;
extern char channel;
extern char ssid_hidden;
extern char startupMode;
extern String pressid;
extern String prepassword;

// ap:1 sta:2 ap+sta:3 wificonnect:4 serverconfig:5
extern char previousServerState;
extern char currentServerState;
extern char nextServerState;

extern char serverDisplayState;

IPAddress apIP;  //开启NAT时用

const String htmlHeader = "<!DOCTYPE html><html lang=\"zh-CN\"><head><meta charset=\"UTF-8\"><meta name=\"viewport\" content=\"width=device-width\">";

TaskHandle_t Task_Sntp;  //网络时间同步任务

//跳转网页配网
void wifiConnect(AsyncWebServerRequest *request) {
  AsyncWebServerResponse *response = request->beginResponse(200, "text/html", htmlHeader + "</head><body><center><h2>请稍等</h2></center></body></html>");
  response->addHeader("Refresh", "1; url=/wifiConnect");  //等待3秒刷新
  response->addHeader("Connection", "close");
  request->send(response);
  nextServerState = MY_SERVER_STATE_WIFI_CONNECT;
  mode_switch = 0;  //使下面的函数跳出while循环，从而在loop函数中进入下一个模式
}
//跳转服务器配置
void serverConfig(AsyncWebServerRequest *request) {
  AsyncWebServerResponse *response = request->beginResponse(200, "text/html", htmlHeader + "</head><body><center><h2>请稍等</h2></center></body></html>");
  response->addHeader("Refresh", "1; url=/serverConfig");  //等待3秒刷新
  response->addHeader("Connection", "close");
  request->send(response);
  nextServerState = MY_SERVER_STATE_CONFIG;
  mode_switch = 0;  //使下面的函数跳出while循环，从而在loop函数中进入下一个模式
}

//返回AP模式
void backToAP(AsyncWebServerRequest *request) {
  AsyncWebServerResponse *response = request->beginResponse(200, "text/html", htmlHeader + "</head><body><center><h2>请稍等</h2></center></body></html>");
  response->addHeader("Refresh", "1; url=/");  //等待3秒刷新
  response->addHeader("Connection", "close");
  request->send(response);
  mode_switch = 0;  //使下面的函数跳出while循环，从而在loop函数中进入下一个模式
}
//返回服务器
void backToServer(AsyncWebServerRequest *request) {
  AsyncWebServerResponse *response = request->beginResponse(200, "text/html", htmlHeader + "</head><body><center><h2>请稍等</h2></center></body></html>");
  response->addHeader("Refresh", "3; url=/");  //等待3秒刷新
  response->addHeader("Connection", "close");
  request->send(response);

  mode_switch = 0;  //使下面的函数跳出while循环，从而在loop函数中进入下一个模式
}
//发送模式转换页面
void pageModeConversion(AsyncWebServerRequest *request) {
  String message = htmlHeader;
  message += "<title>服务器模式转换</title><style>.container { width: 320px; margin: 0 auto; }</style></head><body><div class=\"container\"><h2>服务器模式转换</h2>";
  if (currentServerState == MY_SERVER_STATE_AP) {
    message += "<p><a href=\"/modeConversion?nextServerState=2\">转换为STA模式</a></p><p><a href=\"/modeConversion?nextServerState=3\">转换为AP+STA模式</a></p><p><a href=\"/wifiConnect\">网页配网</a></p><p><a href=\"/\">返回主页</a></p></div></body></html>";
  } else if (currentServerState == MY_SERVER_STATE_STA) {
    message += "<p><a href=\"/modeConversion?nextServerState=1\">转换为AP模式</a></p><p><a href=\"/modeConversion?nextServerState=3\">转换为AP+STA模式</a></p><p><a href=\"/\">返回主页</a></p></div></body></html>";
  } else if (currentServerState == MY_SERVER_STATE_AP_STA) {
    message += "<p><a href=\"/modeConversion?nextServerState=1\">转换为AP模式</a></p><p><a href=\"/modeConversion?nextServerState=2\">转换为STA模式</a></p><p><a href=\"/\">返回主页</a></p></div></body></html>";
  }
  request->send(200, "text/html", message);
}
//模式转换处理函数
void modeConversion(AsyncWebServerRequest *request) {
  char nextServerState3 = 0;
  String nextServerState2 = request->getParam("nextServerState")->value();
  nextServerState3 = String2Char((char *)nextServerState2.c_str());
  nextServerState = nextServerState3;
  AsyncWebServerResponse *response = request->beginResponse(200, "text/html", htmlHeader + "</head><body><center><h2>转换完成</h2></center></body></html>");

  if (currentServerState == MY_SERVER_STATE_AP) {
    if (nextServerState == MY_SERVER_STATE_AP_STA) {
      response->addHeader("Refresh", "3; url=/");  //等待3秒刷新
      response->addHeader("Connection", "close");
    }
  } else if (currentServerState == MY_SERVER_STATE_STA) {
    if (nextServerState == MY_SERVER_STATE_AP_STA) {
      response->addHeader("Refresh", "3; url=/");  //等待3秒刷新
      response->addHeader("Connection", "close");
    }
  } else if (currentServerState == MY_SERVER_STATE_AP_STA) {
    response->addHeader("Refresh", "3; url=/");  //等待3秒刷新
    response->addHeader("Connection", "close");
  }

  request->send(response);
  mode_switch = 0;  //使下面的函数跳出while循环，从而在loop函数中进入下一个模式
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
  if (configRead(config_fs, "startupMode", "/config.txt", buff, CONFIG_MAX_LENGTH)) {
    startupMode = String2Char((char *)buff);
  }
  if (configRead(config_fs, "pressid", "/config.txt", buff, CONFIG_MAX_LENGTH)) {
    pressid = buff;
  }
  if (configRead(config_fs, "prepassword", "/config.txt", buff, CONFIG_MAX_LENGTH)) {
    prepassword = buff;
  }
}

//开启NAT
void enable_napt(void *arg) {
  IPAddress *ip = (IPAddress *)arg;
  ip_napt_enable(*ip, 1);
  Serial.println(*ip);
  // Serial.println("NAT enabled from tcpip thread");
}
//关闭NAT
void disable_napt(void *arg) {
  IPAddress *ip = (IPAddress *)arg;
  ip_napt_enable(*ip, 0);
  Serial.println(*ip);
  // Serial.println("NAT disabled from tcpip thread");
}

//AP模式
void server_ap() {
  // UBaseType_t istack;
  serverDisplayState = MY_SERVER_DP_STATE_AP;
  currentServerState = MY_SERVER_STATE_AP;
  nextServerState = MY_SERVER_STATE_AP;
  // WiFi.setAutoReconnect(false);
  WiFi.disconnect(true, true);
  // WiFi.softAPdisconnect(true);
  WiFi.mode(WIFI_AP);

  // IPAddress local_IP(192, 168, 1, 1);  //配置ESP32的IP地址
  // IPAddress gateway(192, 168, 1, 1);   //配置ESP32的网关
  // IPAddress subnet(255, 255, 255, 0);  //配置ESP32的子网
  // WiFi.softAPConfig(local_IP, gateway, subnet);

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

  APIPAD = WiFi.softAPIP().toString();  //将当前IP地址存储起来


  if (!isServerInitialized) {  //网页服务器未开启
    esp32_server.begin();
    isServerInitialized = 1;
  }

  esp32_server.onNotFound(handleUserRequest);                                  //fallback函数
  esp32_server.on("/filelist", HTTP_GET, listUploadFile);                      //列出文件
  esp32_server.on("/downloadUploadFile", HTTP_GET, downloadUploadFile);        //下载文件，断点续传
  esp32_server.on("/download", HTTP_GET, downloadFile);                        //下载文件，带中文
  esp32_server.on("/deleteUploadFile", HTTP_GET, deleteUploadFile);            //删除文件
  esp32_server.on("/upload", HTTP_POST, uploadFileRespond, handleFileUpload);  //上传文件
  esp32_server.on("/videolist", HTTP_GET, listvideo);                          //列出视频列表
  esp32_server.on("/openvideo", HTTP_GET, openVideo);                          //打开视频
  esp32_server.on("/videolist_mp4", HTTP_GET, listVideo_mp4);                  //列出视频列表（mp4）
  esp32_server.on("/openvideo_mp4", HTTP_GET, openVideo_mp4);                  //打开视频（mp4）
  esp32_server.on("/gamelist", HTTP_GET, listGame);                            //列出游戏列表
  esp32_server.on("/opengame", HTTP_GET, openGame);                            //打开游戏
  esp32_server.on("/saveText", HTTP_POST, handleSaveText);                     //保存文本到txt文件
  esp32_server.on("/getText", HTTP_GET, handleGetText);                        //读取文本到txt文件
  esp32_server.on("/wifiConnect", HTTP_GET, wifiConnect);                      //跳转到网页配网
  esp32_server.on("/serverConfig", HTTP_GET, serverConfig);                    //跳转到服务器配置
  esp32_server.on("/pageModeConversion", HTTP_GET, pageModeConversion);        //模式转换网页
  esp32_server.on("/modeConversion", HTTP_GET, modeConversion);                //模式转换处理
  esp32_server.on("/setTime", HTTP_GET, setTime);                              //设置时间

  while (mode_switch)  //监听用户请求，直到模式转换
  {
    // istack = uxTaskGetStackHighWaterMark(Task_Server);
    // Serial.printf("Task_Server istack = %d\n", istack);
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
  // vTaskDelay(1000 / portTICK_PERIOD_MS);
  mode_switch = 1;
  previousServerState = MY_SERVER_STATE_AP;
  esp32_server.reset();
}

//STA模式
void server_sta() {
  // UBaseType_t istack;
  bool connectSuccess = 0;
  serverDisplayState = MY_SERVER_DP_STATE_STA_CONNECT;
  currentServerState = MY_SERVER_STATE_STA;
  nextServerState = MY_SERVER_STATE_AP;
  WiFi.softAPdisconnect(true);
  WiFi.mode(WIFI_STA);

  //尝试连接上次成功连接WIFI
  WiFi.begin((char *)pressid.c_str(), (char *)prepassword.c_str());
  for (char i = 0; i < 20; i++)  //超时判断
  {
    if (WiFi.status() == WL_CONNECTED)  //如果检测到状态为成功连接WIFI
    {
      connectSuccess = 1;
      // Serial.println('\n');
      // Serial.print("Connected to ");
      // Serial.println(WiFi.SSID());              // 通过串口监视器输出连接的WiFi名称
      // Serial.print("IP address:\t");
      // Serial.println(WiFi.localIP());           // 通过串口监视器输出ESP32-NodeMCU的IP
      break;
    } else {
      vTaskDelay(500 / portTICK_PERIOD_MS);
    }
  }

  if (connectSuccess) {
    serverDisplayState = MY_SERVER_DP_STATE_STA;
    STAIPAD = WiFi.localIP().toString();

    if (!isServerInitialized) {  //网页服务器未开启
      esp32_server.begin();
      isServerInitialized = 1;
    }

    esp32_server.onNotFound(handleUserRequest);                                  //fallback函数
    esp32_server.on("/filelist", HTTP_GET, listUploadFile);                      //列出文件
    esp32_server.on("/downloadUploadFile", HTTP_GET, downloadUploadFile);        //下载文件，断点续传
    esp32_server.on("/download", HTTP_GET, downloadFile);                        //下载文件，带中文
    esp32_server.on("/deleteUploadFile", HTTP_GET, deleteUploadFile);            //删除文件
    esp32_server.on("/upload", HTTP_POST, uploadFileRespond, handleFileUpload);  //上传文件
    esp32_server.on("/videolist", HTTP_GET, listvideo);                          //列出视频列表
    esp32_server.on("/openvideo", HTTP_GET, openVideo);                          //打开视频
    esp32_server.on("/videolist_mp4", HTTP_GET, listVideo_mp4);                  //列出视频列表（mp4）
    esp32_server.on("/openvideo_mp4", HTTP_GET, openVideo_mp4);                  //打开视频（mp4）
    esp32_server.on("/gamelist", HTTP_GET, listGame);                            //列出游戏列表
    esp32_server.on("/opengame", HTTP_GET, openGame);                            //打开游戏
    esp32_server.on("/saveText", HTTP_POST, handleSaveText);                     //保存文本到txt文件
    esp32_server.on("/getText", HTTP_GET, handleGetText);                        //读取文本到txt文件
    esp32_server.on("/serverConfig", HTTP_GET, serverConfig);                    //服务器配置
    esp32_server.on("/pageModeConversion", HTTP_GET, pageModeConversion);        //模式转换网页
    esp32_server.on("/modeConversion", HTTP_GET, modeConversion);                //模式转换处理
    esp32_server.on("/setTime", HTTP_GET, setTime);                              //设置时间

    // Serial.println("HTTP server started");
    // vTaskDelay(3000 / portTICK_PERIOD_MS);
    xTaskCreatePinnedToCore(task_sntp, "Task_Sntp", 2048, NULL, 5, &Task_Sntp, 0);  //创建网络时间同步任务

    while (mode_switch)  //监听用户请求，直到模式转换
    {
      // istack = uxTaskGetStackHighWaterMark(Task_Server);
      // Serial.printf("Task_Server istack = %d\n", istack);
      vTaskDelay(100 / portTICK_PERIOD_MS);
    }
    // vTaskDelay(1000 / portTICK_PERIOD_MS);
    mode_switch = 1;
    previousServerState = MY_SERVER_STATE_STA;
    esp32_server.reset();
  }
}

//AP+STA模式
void server_ap_sta() {
  // UBaseType_t istack;
  bool connectSuccess = 0;
  serverDisplayState = MY_SERVER_DP_STATE_AP_STA_CONNECT;
  currentServerState = MY_SERVER_STATE_AP_STA;
  nextServerState = MY_SERVER_STATE_AP;
  WiFi.mode(WIFI_AP_STA);

  //尝试连接上次成功连接WIFI
  WiFi.begin((char *)pressid.c_str(), (char *)prepassword.c_str());
  for (char i = 0; i < 20; i++)  //超时判断
  {
    if (WiFi.status() == WL_CONNECTED)  //如果检测到状态为成功连接WIFI
    {
      connectSuccess = 1;
      // Serial.println('\n');
      // Serial.print("Connected to ");
      // Serial.println(WiFi.SSID());              // 通过串口监视器输出连接的WiFi名称
      // Serial.print("IP address:\t");
      // Serial.println(WiFi.localIP());           // 通过串口监视器输出ESP32-NodeMCU的IP
      break;
    } else {
      vTaskDelay(500 / portTICK_PERIOD_MS);
    }
  }

  if (connectSuccess) {
    serverDisplayState = MY_SERVER_DP_STATE_AP_STA;
    STAIPAD = WiFi.localIP().toString();

    //WiFi.softAP(ssid, passphrase, channel, ssid_hidden, max_connection)
    WiFi.softAP((char *)ssid.c_str(), (char *)password.c_str(), channel, ssid_hidden, 4);  //启动AP模式

    APIPAD = WiFi.softAPIP().toString();  //将当前IP地址存储起来
    apIP = WiFi.softAPIP();
    // vTaskDelay(1000 / portTICK_PERIOD_MS);
    tcpip_callback(enable_napt, &apIP);  //开启NAT

    if (!isServerInitialized) {  //网页服务器未开启
      esp32_server.begin();
      isServerInitialized = 1;
    }

    esp32_server.onNotFound(handleUserRequest);                                  //fallback函数
    esp32_server.on("/filelist", HTTP_GET, listUploadFile);                      //列出文件
    esp32_server.on("/downloadUploadFile", HTTP_GET, downloadUploadFile);        //下载文件，断点续传
    esp32_server.on("/download", HTTP_GET, downloadFile);                        //下载文件，带中文
    esp32_server.on("/deleteUploadFile", HTTP_GET, deleteUploadFile);            //删除文件
    esp32_server.on("/upload", HTTP_POST, uploadFileRespond, handleFileUpload);  //上传文件
    esp32_server.on("/videolist", HTTP_GET, listvideo);                          //列出视频列表
    esp32_server.on("/openvideo", HTTP_GET, openVideo);                          //打开视频
    esp32_server.on("/videolist_mp4", HTTP_GET, listVideo_mp4);                  //列出视频列表（mp4）
    esp32_server.on("/openvideo_mp4", HTTP_GET, openVideo_mp4);                  //打开视频（mp4）
    esp32_server.on("/gamelist", HTTP_GET, listGame);                            //列出游戏列表
    esp32_server.on("/opengame", HTTP_GET, openGame);                            //打开游戏
    esp32_server.on("/saveText", HTTP_POST, handleSaveText);                     //保存文本到txt文件
    esp32_server.on("/getText", HTTP_GET, handleGetText);                        //读取文本到txt文件
    esp32_server.on("/serverConfig", HTTP_GET, serverConfig);                    //服务器配置
    esp32_server.on("/pageModeConversion", HTTP_GET, pageModeConversion);        //模式转换网页
    esp32_server.on("/modeConversion", HTTP_GET, modeConversion);                //模式转换处理
    esp32_server.on("/setTime", HTTP_GET, setTime);                              //设置时间

    // Serial.println("HTTP server started");
    // vTaskDelay(3000 / portTICK_PERIOD_MS);
    xTaskCreatePinnedToCore(task_sntp, "Task_Sntp", 2048, NULL, 5, &Task_Sntp, 0);  //创建网络时间同步任务

    while (mode_switch)  //监听用户请求，直到模式转换
    {
      // istack = uxTaskGetStackHighWaterMark(Task_Server);
      // Serial.printf("Task_Server istack = %d\n", istack);
      vTaskDelay(100 / portTICK_PERIOD_MS);
    }
    // vTaskDelay(500 / portTICK_PERIOD_MS);
    tcpip_callback(disable_napt, &apIP);  //关闭NAT
    // vTaskDelay(500 / portTICK_PERIOD_MS);
    mode_switch = 1;
    previousServerState = MY_SERVER_STATE_AP_STA;
    esp32_server.reset();
  }
}


//网页配网
void server_wifi_connect() {
  serverDisplayState = MY_SERVER_DP_STATE_WIFI_SCAN;
  currentServerState = MY_SERVER_STATE_WIFI_CONNECT;
  nextServerState = MY_SERVER_STATE_AP;
  WiFi.mode(WIFI_AP_STA);

  esp32_server.onNotFound(wifi_handleNotFound);                  //请求失败回调函数
  esp32_server.on("/wifiConnect", HTTP_GET, handleRoot);         //发送配网页面
  esp32_server.on("/HandleWifi", HTTP_GET, HandleWifi);          //尝试连接网页发送的WIFI
  esp32_server.on("/HandleScanWifi", HTTP_GET, HandleScanWifi);  //扫描附近WIFI并返回
  esp32_server.on("/", HTTP_GET, backToAP);                      //返回AP模式

  while (mode_switch) {
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
  vTaskDelay(1000 / portTICK_PERIOD_MS);  //如果接收不到IP地址，增大该时延
  mode_switch = 1;
  esp32_server.reset();
  // previousServerState = MY_SERVER_STATE_WIFI_CONNECT;
}

//服务器配置
void server_config() {
  serverDisplayState = MY_SERVER_DP_STATE_CONFIG;
  currentServerState = MY_SERVER_STATE_CONFIG;

  esp32_server.onNotFound(config_handleNotFound);                              //请求失败回调函数
  esp32_server.on("/serverConfig", HTTP_GET, pageConfigAP);                    //发送配置主页
  esp32_server.on("/pageConfigAP", HTTP_GET, pageConfigAP);                    //发送配置热点网页
  esp32_server.on("/configAP", HTTP_GET, configAP);                            //配置热点
  esp32_server.on("/pageConfigAutoConnect", HTTP_GET, pageConfigAutoConnect);  //发送配置WiFi自动连接网页
  esp32_server.on("/configAutoConnect", HTTP_GET, configAutoConnect);          //保存iFi自动连接配置
  esp32_server.on("/", HTTP_GET, backToServer);                                //返回AP模式
  while (mode_switch) {
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
  mode_switch = 1;
  nextServerState = previousServerState;
  esp32_server.reset();
}


// 自定义响应类
class RangeFileResponse : public AsyncAbstractResponse {
private:
  File _file;
  size_t _start;
  size_t _end;
  size_t _position;

public:
  RangeFileResponse(String path, String contentType, size_t start, size_t end, size_t total, int statusCode, bool addContentRange) {
    _file = my_fs.open(path, FILE_READ);
    if (!_file) {
      _code = 404;
      return;
    }
    if (!_file.seek(start)) {
      _file.close();
      _code = 500;
      return;
    }
    _code = statusCode;
    _contentLength = end - start + 1;
    _contentType = contentType;
    _start = start;
    _end = end;
    _position = 0;

    addHeader("Accept-Ranges", "bytes");
    if (addContentRange) {
      char contentRange[64];
      snprintf(contentRange, sizeof(contentRange), "bytes %d-%d/%d", _start, _end, total);
      addHeader("Content-Range", contentRange);
    }
  }

  ~RangeFileResponse() {
    if (_file) _file.close();
    // Serial.println("File close");
  }

  bool _sourceValid() const override {
    return !!_file;
  }

  size_t _fillBuffer(uint8_t *buf, size_t maxLen) override {
    // Serial.printf("_fillBuffer called, position=%d, contentLength=%d\n", _position, _contentLength);
    if (!_file || _position >= _contentLength) return 0;
    size_t available = _contentLength - _position;
    size_t len = maxLen;
    if (len > available) len = available;
    size_t read = _file.read(buf, len);
    _position += read;
    return read;
  }
};


void handleUserRequest(AsyncWebServerRequest *request) {
  String path = request->url();
  String contentType = "";
  if (path.endsWith("/")) {
    path = "/index.html";
    contentType = "text/html";
  } else if (path.endsWith(".html")) contentType = "text/html";
  else if (path.endsWith(".mp4")) contentType = "video/mp4";
  else if (path.endsWith(".ts")) contentType = "video/mp2t";
  else if (path.endsWith(".css")) contentType = "text/css";
  else if (path.endsWith(".js")) contentType = "application/javascript";
  else if (path.endsWith(".png")) contentType = "image/png";
  else if (path.endsWith(".gif")) contentType = "image/gif";
  else if (path.endsWith(".jpg")) contentType = "image/jpeg";
  else if (path.endsWith(".ico")) contentType = "image/x-icon";
  else if (path.endsWith(".m3u8")) contentType = "application/x-mpegurl";
  else contentType = "application/octet-stream";

  if (!my_fs.exists(path)) {
    request->send(404, "text/plain", "File not found");
    return;
  }

  File file = my_fs.open(path, FILE_READ);
  if (!file) {
    request->send(500, "text/plain", "Failed to open file");
    return;
  }
  size_t fileSize = file.size();
  file.close();

  // 检查Range头
  if (request->hasHeader("Range")) {
    String rangeHeader = request->getHeader("Range")->value();
    if (rangeHeader.startsWith("bytes=")) {
      String range = rangeHeader.substring(6);
      range.trim();
      int dashIndex = range.indexOf('-');

      if (dashIndex > 0) {
        String startStr = range.substring(0, dashIndex);
        String endStr = range.substring(dashIndex + 1);
        size_t start = 0, end = fileSize - 1;

        if (startStr.length() > 0) start = startStr.toInt();
        if (endStr.length() > 0) end = endStr.toInt();
        else end = fileSize - 1;  // 如果结束位置为空，则到文件尾

        // 验证范围有效性
        if (start >= fileSize || end >= fileSize || start > end) {
          request->send(416, "text/plain", "Range not satisfiable");
          return;
        }

        RangeFileResponse *response = new RangeFileResponse(path, contentType, start, end, fileSize, 206, true);
        if (!response->_sourceValid()) {
          delete response;
          request->send(500, "text/plain", "Internal Server Error");
          return;
        }
        request->send(response);
        return;
      } else if (dashIndex == 0) {
        // 后缀范围: "-suffix"
        String suffixStr = range.substring(1);
        size_t suffix = suffixStr.toInt();
        if (suffix > fileSize) suffix = fileSize;
        size_t start = fileSize - suffix;
        size_t end = fileSize - 1;

        RangeFileResponse *response = new RangeFileResponse(path, contentType, start, end, fileSize, 206, true);
        if (!response->_sourceValid()) {
          delete response;
          request->send(500, "text/plain", "Internal Server Error");
          return;
        }
        request->send(response);
        return;
      } else {
        request->send(400, "text/plain", "Bad Request");
        return;
      }
    } else {
      request->send(400, "text/plain", "Bad Request");
      return;
    }
  }

  // 无Range：发送完整文件
  RangeFileResponse *response = new RangeFileResponse(path, contentType, 0, fileSize - 1, fileSize, 200, false);
  if (!response->_sourceValid()) {
    delete response;
    request->send(500, "text/plain", "Internal Server Error");
    return;
  }
  request->send(response);
}


// void handleUserRequest(AsyncWebServerRequest *request) {
//   String path = request->url();
//   // bool fileReadOK = false;
//   String contentType = "";

//   if (path.endsWith("/")) {  //如果访问地址以"/"为结尾
//     path = "/index.html";    //则将访问地址修改为/index.html便于SPIFFS访问
//     contentType = "text/html";
//   } else if (path.endsWith(".html")) contentType = "text/html";
//   else if (path.endsWith(".ts")) contentType = "video/mp2t";
//   else if (path.endsWith(".css")) contentType = "text/css";
//   else if (path.endsWith(".js")) contentType = "application/javascript";
//   else if (path.endsWith(".png")) contentType = "image/png";
//   else if (path.endsWith(".gif")) contentType = "image/gif";
//   else if (path.endsWith(".jpg")) contentType = "image/jpeg";
//   else if (path.endsWith(".ico")) contentType = "image/x-icon";
//   else if (path.endsWith(".m3u8")) contentType = "application/x-mpegurl";
//   else contentType = "text/plain";

//   if (my_fs.exists(path)) {  //如果访问的文件可以在SD卡中找到
//     AsyncWebServerResponse *response = request->beginResponse(my_fs, path, contentType);
//     request->send(response);
//     // fileReadOK = true;
//   } else {
//     request->send(404, "text/plain", "Not found");
//     // fileReadOK = false;
//   }

//   //检测SD卡意外弹出
//   // if (!fileReadOK) {
//   //   my_fs.end();
//   //   if (my_fs.begin("/sdcard", ONE_BIT_MODE, false, BOARD_MAX_SDMMC_FREQ, 8)) {  //SD卡初始化
//   //     if (!hasSD) {
//   //       request->send(404, "text/plain", "Card Mount Succeed");  //如果在SD卡初始化成功，则回复Card Mount Succeed
//   //       hasSD = true;
//   //     } else {
//   //       request->send(404, "text/plain", "Not found");  //如果在SD卡无法找到用户访问的资源，则回复404 Not Found
//   //     }

//   //   } else {
//   //     hasSD = false;
//   //     request->send(404, "text/plain", "Card Mount Failed");  // 如果无法读取SD卡，则回复Card Mount Failed
//   //   }
//   // }
// }
