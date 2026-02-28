#include "serverConfig.h"

extern String ssid;
extern String password;
extern char channel;
extern char ssid_hidden;
extern char startupMode;
extern String pressid;
extern String prepassword;

const String htmlAP1 = "<!DOCTYPE html><html><head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\" /><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\" /><meta http-equiv=\"X-UA-Compatible\" content=\"ie=edge\" /><title>配置热点</title><style>.container {width: 320px;margin: 0 auto;}</style></head><body><div class=\"container\">";
const String htmlAP2 = "<h2>配置热点</h2><form action=\"/configAP\">热点名称： <input name=\"hotspotName\" type=\"text\" value=\"";
const String htmlAP3 = "\" required=\"required\" maxlength=\"63\" placeholder=\"请输入热点名称\" /><br>热点密码： <input name=\"hotspotPassword\" type=\"text\" value=\"";
const String htmlAP4 = "\" onkeyup=\"value=value.replace(/[\u4e00-\u9fa5]/ig,&#39;&#39;)\" required=\"required\" maxlength=\"63\" placeholder=\"密码请不要少于8位\" /><br>WIFI信道： <select name=\"channel\">";
const String htmlAP5 = "</select><br>WIFI隐身： <input name=\"hidden\" type=\"radio\" value=\"0\"";
const String htmlAP6 = "<br><br><input type=\"submit\" value=\"提交\" /></form><p><a href=\"/\">返回服务器</a> <a href=\"/pageConfigAutoConnect\">配置WiFi</a></p></div></body></html>";

const String htmlAutoConnect1 = "<!DOCTYPE html><html><head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\" /><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\" /><meta http-equiv=\"X-UA-Compatible\" content=\"ie=edge\" /><title>配置WiFi自动连接</title><style>.container {width: 320px;margin: 0 auto;}</style></head><body><div class=\"container\">";
const String htmlAutoConnect2 = "<h2>配置WiFi自动连接</h2><form action=\"/configAutoConnect\">";
const String htmlAutoConnect3 = "WiFi名称： <input name=\"pressid\" type=\"text\" value=\"";
const String htmlAutoConnect4 = "\" required=\"required\" maxlength=\"63\" placeholder=\"请输入WiFi名称\" /><br>WiFi密码： <input name=\"prepassword\" type=\"text\" value=\"";
const String htmlAutoConnect5 = "\" required=\"required\" maxlength=\"63\" placeholder=\"密码请不要少于8位\" />";
const String htmlAutoConnect6 = "<br><br><input type=\"submit\" value=\"提交\" /></form><p><a href=\"/\">返回服务器</a> <a href=\"/pageConfigAP\">配置热点</a></p></div></body></html>";

const String htmlStartupMode1 = "<br>上电默认模式： <input name=\"startupMode\" type=\"radio\" value=\"1\" checked>AP<input name=\"startupMode\" type=\"radio\" value=\"2\">STA<input name=\"startupMode\" type=\"radio\" value=\"3\">AP+STA";
const String htmlStartupMode2 = "<br>上电默认模式： <input name=\"startupMode\" type=\"radio\" value=\"1\">AP<input name=\"startupMode\" type=\"radio\" value=\"2\" checked>STA<input name=\"startupMode\" type=\"radio\" value=\"3\">AP+STA";
const String htmlStartupMode3 = "<br>上电默认模式： <input name=\"startupMode\" type=\"radio\" value=\"1\">AP<input name=\"startupMode\" type=\"radio\" value=\"2\">STA<input name=\"startupMode\" type=\"radio\" value=\"3\" checked>AP+STA";

const String htmlfoot = "<p><a href=\"/\">返回服务器</a> <a href=\"/pageConfigAP\">配置热点</a> <a href=\"/pageConfigAutoConnect\">配置WiFi</a></p></div></body></html>";


//发送配置AP网页
void pageConfigAP(AsyncWebServerRequest *request) {
  int i = 1;
  String message = htmlAP1 + htmlAP2 + ssid + htmlAP3 + password + htmlAP4;
  for (i = 1; i < 14; i++) {
    message += "<option value=\"";
    message += String(i);
    if (i == channel) {
      message += "\" selected>";
    } else {
      message += "\">";
    }
    message += String(i);
    message += "</option>";
  }
  message += htmlAP5;
  if (ssid_hidden) {
    message += ">关闭<input name=\"hidden\" type=\"radio\" value=\"1\" checked>开启";
  } else {
    message += " checked>关闭<input name=\"hidden\" type=\"radio\" value=\"1\">开启";
  }

  if (startupMode == 1) {
    message += htmlStartupMode1;
  } else if (startupMode == 2) {
    message += htmlStartupMode2;
  } else if (startupMode == 3) {
    message += htmlStartupMode3;
  } else {
    message += htmlStartupMode1;
  }
  message += htmlAP6;
  request->send(200, "text/html", message);
}

//保存修改的热点名称和密码
void configAP(AsyncWebServerRequest *request) {
  String ssid2 = request->getParam("hotspotName")->value();          //获取热点名称
  String password2 = request->getParam("hotspotPassword")->value();  //获取热点密码
  String channel2 = request->getParam("channel")->value();           //获取AP信道
  String hidden2 = request->getParam("hidden")->value();             //获取wifi隐身配置
  String startupMode2 = request->getParam("startupMode")->value();   //获取上电默认模式
  String message = "";
  char filetxt[CONFIG_FILE_MAX_LENGTH] = { 0 };
  char flag = 0;
  // flag = flag + configWrite(config_fs, "ssid", (char*)ssid2.c_str(), "/config.txt");
  // flag = flag + configWrite(config_fs, "password", (char*)password2.c_str(), "/config.txt");
  // flag = flag + configWrite(config_fs, "channel", (char*)channel2.c_str(), "/config.txt");
  // flag = flag + configWrite(config_fs, "startupMode", (char*)startupMode2.c_str(), "/config.txt");
  configWriteOpen(config_fs, "/config.txt", filetxt);
  flag = flag + configRewrite("ssid", (char *)ssid2.c_str(), (char *)filetxt);
  flag = flag + configRewrite("password", (char *)password2.c_str(), (char *)filetxt);
  flag = flag + configRewrite("channel", (char *)channel2.c_str(), (char *)filetxt);
  flag = flag + configRewrite("startupMode", (char *)startupMode2.c_str(), (char *)filetxt);
  configWriteClose(config_fs, "/config.txt", (char *)filetxt);

  if (flag == 4) {
    ssid = ssid2;
    password = password2;
    channel = String2Char((char *)channel2.c_str());
    ssid_hidden = String2Char((char *)hidden2.c_str());
    message += htmlAP1 + "<h2>配置成功</h2>" + htmlfoot;
    request->send(200, "text/html", message);
  } else {
    message += htmlAP1 + "<h2>配置失败</h2>" + htmlfoot;
    request->send(200, "text/html", message);
  }
}

//发送配置WiFi自动连接网页
void pageConfigAutoConnect(AsyncWebServerRequest *request) {
  String message = htmlAutoConnect1 + htmlAutoConnect2 + htmlAutoConnect3;
  message += pressid;
  message += htmlAutoConnect4;
  message += prepassword;
  message += htmlAutoConnect5;
  if (startupMode == 1) {
    message += htmlStartupMode1;
  } else if (startupMode == 2) {
    message += htmlStartupMode2;
  } else if (startupMode == 3) {
    message += htmlStartupMode3;
  } else {
    message += htmlStartupMode1;
  }
  message += htmlAutoConnect6;
  request->send(200, "text/html", message);
}

//保存WiFi自动连接配置
void configAutoConnect(AsyncWebServerRequest *request) {
  String pressid2 = request->getParam("pressid")->value();          //获取WiFi名称
  String prepassword2 = request->getParam("prepassword")->value();  //获取WiFi密码
  String startupMode2 = request->getParam("startupMode")->value();  //获取上电默认模式
  String message = "";
  char filetxt[CONFIG_FILE_MAX_LENGTH] = { 0 };
  char flag = 0;
  // flag = flag + configWrite(config_fs, "startupMode", (char*)startupMode2.c_str(), "/config.txt");
  // flag = flag + configWrite(config_fs, "pressid", (char*)pressid2.c_str(), "/config.txt");
  // flag = flag + configWrite(config_fs, "prepassword", (char*)prepassword2.c_str(), "/config.txt");
  configWriteOpen(config_fs, "/config.txt", filetxt);
  flag = flag + configRewrite("startupMode", (char *)startupMode2.c_str(), (char *)filetxt);
  flag = flag + configRewrite("pressid", (char *)pressid2.c_str(), (char *)filetxt);
  flag = flag + configRewrite("prepassword", (char *)prepassword2.c_str(), (char *)filetxt);
  configWriteClose(config_fs, "/config.txt", (char *)filetxt);

  if (flag == 3) {
    startupMode = String2Char((char *)startupMode2.c_str());
    pressid = pressid2;
    prepassword = prepassword2;
    message += htmlAutoConnect1 + "<h2>配置成功</h2>" + htmlfoot;
    request->send(200, "text/html", message);
  } else {
    message += htmlAutoConnect1 + "<h2>配置失败</h2>" + htmlfoot;
    request->send(500, "text/html", message);
  }
}

void config_handleNotFound(AsyncWebServerRequest *request) {
  String message = "File Not Found\n\nClient:" + request->client()->remoteIP().toString() + " " + request->url();
  request->send(404, "text/plain", message);
}
