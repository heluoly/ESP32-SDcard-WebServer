#include "wifiConnect.h"

extern WebServer server;
extern bool mode_switch;

extern String ssid;
extern String password;
extern char channel;
extern char ssid_hidden;
extern char autoconnect;
extern String pressid;
extern String prepassword;

// const String html1 = "<!DOCTYPE html><html><head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\"><meta http-equiv=\"X-UA-Compatible\" content=\"ie=edge\"><title>网页配网</title><script type=\"text/javascript\">function wifi(){var ssid = s.value;var password = p.value;var ip2 = ip.value;var staticIP2 = staticIP.value;var gateway2 = gateway.value;var subnet2 = subnet.value;var dns2 = dns.value;var xmlhttp=new XMLHttpRequest();xmlhttp.open(\"GET\",\"/HandleWifi?ssid=\"+ssid+\"&password=\"+password+\"&ip=\"+ip2+\"&staticIP=\"+staticIP2+\"&gateway=\"+gateway2+\"&subnet=\"+subnet2+\"&dns=\"+dns2,true);xmlhttp.send();document.getElementById(\"loader\").style.display = \"block\";xmlhttp.onload = function(e){document.getElementById(\"loader\").style.display = \"none\";alert(this.responseText);}}</script><script>function c(l){document.getElementById('s').value=l.innerText||l.textContent;document.getElementById('p').focus();}</script><script>function agree(){if(document.getElementById('ip').checked){document.getElementById('ip').value=\"1\";document.getElementById('staticIP').disabled=false;document.getElementById('gateway').disabled=false;document.getElementById('subnet').disabled=false;document.getElementById('dns').disabled=false;}else{document.getElementById('ip').value=\"0\";document.getElementById('staticIP').disabled='disabled';document.getElementById('gateway').disabled='disabled';document.getElementById('subnet').disabled='disabled';document.getElementById('dns').disabled='disabled';}}</script><script>function scan(){var xmlhttp2=new XMLHttpRequest();xmlhttp2.open(\"GET\",\"/HandleScanWifi\",true);xmlhttp2.send();document.getElementById(\"loader\").style.display = \"block\";xmlhttp2.onload = function(e){var element=document.getElementById(\"scan\");document.getElementById(\"loader\").style.display = \"none\";element.innerHTML=this.responseText;}}</script><style>#loader {position: fixed;left: 50%;top: 50%;z-index: 1;width: 120px;height: 120px;margin: -76px 0 0 -76px;border: 16px solid #f3f3f3;border-radius: 50%;border-top: 16px solid #3498db;-webkit-animation: spin 2s linear infinite;animation: spin 2s linear infinite;backdrop-filter: blur(2px);}@-webkit-keyframes spin {0% { -webkit-transform: rotate(0deg); }100% { -webkit-transform: rotate(360deg); }}@keyframes spin {0% { transform: rotate(0deg); }100% { transform: rotate(360deg); }} .container {width: 320px;margin: 0 auto;}</style></head><body><div style=\"display:none;\" id=\"loader\"></div><div class=\"container\"><h2>网页配网</h2><form>WiFi名称：<input id='s' name='s' type=\"text\" placeholder=\"请输入您WiFi的名称\"><br>WiFi密码：<input id='p' name='p' type=\"text\" placeholder=\"请输入您WiFi的密码\"><br><label for=\"ip\">静态IP</label><input name=\"ip\" id=\"ip\" type=\"checkbox\" onclick=\"agree();\"><br>IP地址：<input name=\"staticIP\" id=\"staticIP\" type=\"text\" value=\"192.168.0.80\" disabled=\"\"><br>网关：<input name=\"gateway\" id=\"gateway\" type=\"text\" value=\"192.168.1.1\" disabled=\"\"><br>子网：<input name=\"subnet\" id=\"subnet\" type=\"text\" value=\"255.255.255.0\" disabled=\"\"><br>DNS：<input name=\"dns\" id=\"dns\" type=\"text\" value=\"223.5.5.5\" disabled=\"\"><br><br><input type=\"button\" value=\"扫描\" onclick=\"scan()\"> <input type=\"button\" value=\"连接\" onclick=\"wifi()\"></form><div id=\"scan\"></div><p><a href=\"/\">返回AP模式</a>  <a href=\"/pageConfigAP\">配置热点</a> <a href=\"/pageConfigAutoConnect\">配置WiFi</a></p></div></body></html>";

const String htmlWIFIConnect1 = "<!DOCTYPE html><html><head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\"><meta http-equiv=\"X-UA-Compatible\" content=\"ie=edge\"><title>网页配网</title><script type=\"text/javascript\">function wifi(){var ssid = s.value;var password = p.value;var ip2 = ip.value;var staticIP2 = staticIP.value;var gateway2 = gateway.value;var subnet2 = subnet.value;var dns2 = dns.value;var xmlhttp=new XMLHttpRequest();xmlhttp.open(\"GET\",\"/HandleWifi?ssid=\"+ssid+\"&password=\"+password+\"&ip=\"+ip2+\"&staticIP=\"+staticIP2+\"&gateway=\"+gateway2+\"&subnet=\"+subnet2+\"&dns=\"+dns2,true);xmlhttp.send();document.getElementById(\"loader\").style.display = \"block\";xmlhttp.onload = function(e){document.getElementById(\"loader\").style.display = \"none\";alert(this.responseText);}}</script><script>function c(l){document.getElementById('s').value=l.innerText||l.textContent;document.getElementById('p').focus();}</script><script>function agree(){if(document.getElementById('ip').checked){document.getElementById('ip').value=\"1\";document.getElementById('staticIP').disabled=false;document.getElementById('gateway').disabled=false;document.getElementById('subnet').disabled=false;document.getElementById('dns').disabled=false;}else{document.getElementById('ip').value=\"0\";document.getElementById('staticIP').disabled='disabled';document.getElementById('gateway').disabled='disabled';document.getElementById('subnet').disabled='disabled';document.getElementById('dns').disabled='disabled';}}</script><script>function scan(){var xmlhttp2=new XMLHttpRequest();xmlhttp2.open(\"GET\",\"/HandleScanWifi\",true);xmlhttp2.send();document.getElementById(\"loader\").style.display = \"block\";xmlhttp2.onload = function(e){var element=document.getElementById(\"scan\");document.getElementById(\"loader\").style.display = \"none\";element.innerHTML=this.responseText;}}</script><style>#loader {position: fixed;left: 50%;top: 50%;z-index: 1;width: 120px;height: 120px;margin: -76px 0 0 -76px;border: 16px solid #f3f3f3;border-radius: 50%;border-top: 16px solid #3498db;-webkit-animation: spin 2s linear infinite;animation: spin 2s linear infinite;backdrop-filter: blur(2px);}@-webkit-keyframes spin {0% { -webkit-transform: rotate(0deg); }100% { -webkit-transform: rotate(360deg); }}@keyframes spin {0% { transform: rotate(0deg); }100% { transform: rotate(360deg); }} .container {width: 320px;margin: 0 auto;}</style></head><body><div style=\"display:none;\" id=\"loader\"></div><div class=\"container\"><h2>网页配网</h2><form>WiFi名称：<input id='s' name='s' type=\"text\" placeholder=\"请输入您WiFi的名称\"><br>WiFi密码：<input id='p' name='p' type=\"text\" placeholder=\"请输入您WiFi的密码\"><br><label for=\"ip\">静态IP</label><input name=\"ip\" id=\"ip\" type=\"checkbox\" onclick=\"agree();\"><br>IP地址：<input name=\"staticIP\" id=\"staticIP\" type=\"text\" value=\"";
const String htmlWIFIConnect2 = "\" disabled=\"\"><br>网关：<input name=\"gateway\" id=\"gateway\" type=\"text\" value=\"";
const String htmlWIFIConnect3 = "\" disabled=\"\"><br>子网：<input name=\"subnet\" id=\"subnet\" type=\"text\" value=\"";
const String htmlWIFIConnect4 = "\" disabled=\"\"><br>DNS：<input name=\"dns\" id=\"dns\" type=\"text\" value=\"";
const String htmlWIFIConnect5 = "\" disabled=\"\"><br><br><input type=\"button\" value=\"扫描\" onclick=\"scan()\"> <input type=\"button\" value=\"连接\" onclick=\"wifi()\"></form><div id=\"scan\"></div><p><a href=\"/\">返回AP模式</a>  <a href=\"/pageConfigAP\">配置热点</a> <a href=\"/pageConfigAutoConnect\">配置WiFi</a></p></div></body></html>";

const String htmlAP1 = "<!DOCTYPE html><html><head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\" /><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\" /><meta http-equiv=\"X-UA-Compatible\" content=\"ie=edge\" /><title>配置热点</title><style>.container {width: 320px;margin: 0 auto;}</style></head><body></div><div class=\"container\">";
const String htmlAP2 = "<h2>配置热点</h2><form action=\"/configAP\">热点名称： <input name=\"hotspotName\" type=\"text\" value=\"";
const String htmlAP3 = "\" required=\"required\" maxlength=\"63\" placeholder=\"请输入热点名称\" /><br />热点密码： <input name=\"hotspotPassword\" type=\"text\" value=\"";
const String htmlAP4 = "\" onkeyup=\"value=value.replace(/[\u4e00-\u9fa5]/ig,&#39;&#39;)\" required=\"required\" maxlength=\"63\" placeholder=\"密码请不要少于8位\" /><br>WIFI信道： <select name=\"channel\">";
const String htmlAP5 = "</select><br>WIFI隐身： <input name=\"hidden\" type=\"radio\" value=\"0\"";
const String htmlAP6 = "<br /><br /><input type=\"submit\" value=\"提交\" /></form><p><a href=\"/\">返回AP模式</a> <a href=\"/wificonnect\">网页配网</a> <a href=\"/pageConfigAutoConnect\">配置WiFi</a></p></div></body></html>";


const String htmlAutoConnect1 = "<!DOCTYPE html><html><head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\" /><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\" /><meta http-equiv=\"X-UA-Compatible\" content=\"ie=edge\" /><title>配置WiFi自动连接</title><style>.container {width: 320px;margin: 0 auto;}</style></head><body></div><div class=\"container\">";
const String htmlAutoConnect2 = "<h2>配置WiFi自动连接</h2><form action=\"/configAutoConnect\">WiFi自动连接： <input name=\"autoconnect\" type=\"radio\" value=\"0\"";
const String htmlAutoConnect3 = "<br>WiFi名称： <input name=\"pressid\" type=\"text\" value=\"";
const String htmlAutoConnect4 = "\" required=\"required\" maxlength=\"63\" placeholder=\"请输入WiFi名称\" /><br>WiFi密码： <input name=\"prepassword\" type=\"text\" value=\"";
const String htmlAutoConnect5 = "\" required=\"required\" maxlength=\"63\" placeholder=\"密码请不要少于8位\" /><br /><br /><input type=\"submit\" value=\"提交\" /></form><p><a href=\"/\">返回AP模式</a> <a href=\"/wificonnect\">网页配网</a> <a href=\"/pageConfigAP\">配置热点</a></p></div></body></html>";

const String htmlfoot = "<p><a href=\"/\">返回AP模式</a> <a href=\"/wificonnect\">网页配网</a> <a href=\"/pageConfigAP\">配置热点</a> <a href=\"/pageConfigAutoConnect\">配置WiFi</a></p></div></body></html>";

//定义一个结构体，用于存放4位IP地址
struct struct_ipaddr {
  uint8_t ipaddr_temp[4];
};

//发送配网页面
// void handleRoot() {
//   server.send(200, "text/html", html1);
// }

//发送配网页面
void handleRoot() {
  char buff[CONFIG_MAX_LENGTH];
  String staticIP1 = "";
  String gateway1 = "";
  String subnet1 = "";
  String dns1 = "";
  if (configRead(config_fs, "staticIP", "/config.txt", buff, CONFIG_MAX_LENGTH)) {
    staticIP1 = buff;
  }
  if (configRead(config_fs, "gateway", "/config.txt", buff, CONFIG_MAX_LENGTH)) {
    gateway1 = buff;
  }
  if (configRead(config_fs, "subnet", "/config.txt", buff, CONFIG_MAX_LENGTH)) {
    subnet1 = buff;
  }
  if (configRead(config_fs, "dns", "/config.txt", buff, CONFIG_MAX_LENGTH)) {
    dns1 = buff;
  }
  String message = htmlWIFIConnect1 + staticIP1 + htmlWIFIConnect2 + gateway1 + htmlWIFIConnect3 + subnet1 + htmlWIFIConnect4 + dns1 + htmlWIFIConnect5;
  server.send(200, "text/html", message);
}

//扫描附近WIFI并返回
void HandleScanWifi() {
  uint8_t i = 0;
  String scanstr = "";
  // WiFi.scanNetworks will return the number of networks found
  uint8_t n = WiFi.scanNetworks();
  if (n <= 0) {
    // Serial.println("no networks found");
    scanstr += "NO WIFI";
  } else {
    scanstr += "<table><tr><th>序号</th><th>名称</th><th>强度</th></tr>";
    for (i = 0; i < n; i++) {
      scanstr += "<tr><td align=\"center\">" + String(i + 1) + "</td><td align=\"center\">" + "<a href='#p' onclick='c(this)'>" + WiFi.SSID(i) + "</a>" + "</td><td align=\"center\">" + WiFi.RSSI(i) + "</td></tr>";
    }
    scanstr += "</table>";
  }
  server.send(200, "text/html", scanstr);
}

//尝试连接网页发送的WIFI
void HandleWifi() {
  String wifis = server.arg("ssid");          //获取WIFI名称
  String wifip = server.arg("password");      //获取WIFI密码
  String ip2 = server.arg("ip");              //判断是DHCP连接还是静态IP连接
  String staticIP2 = server.arg("staticIP");  //静获取态IP地址
  String gateway2 = server.arg("gateway");    //静获取态IP网关
  String subnet2 = server.arg("subnet");      //静获取态IP子网
  String dns2 = server.arg("dns");            //静获取态IP的dns
  struct_ipaddr x;
  String IPAD3 = "";
  // Serial.println("received:" + wifis);
  /*
  Serial.println("ip:" + ip2);
  Serial.println("staticIP:" + staticIP2);
  Serial.println("gateway:" + gateway2);
  Serial.println("subnet:" + subnet2);
  Serial.println("dns:" + dns2);
  */
  WiFi.disconnect(true, true);

  if (ip2 == "1")  //配置静态IP情况
  {
    x = StringToIPAddress(staticIP2);
    IPAddress staticIP(x.ipaddr_temp[0], x.ipaddr_temp[1], x.ipaddr_temp[2], x.ipaddr_temp[3]);
    x = StringToIPAddress(gateway2);
    IPAddress gateway(x.ipaddr_temp[0], x.ipaddr_temp[1], x.ipaddr_temp[2], x.ipaddr_temp[3]);
    x = StringToIPAddress(subnet2);
    IPAddress subnet(x.ipaddr_temp[0], x.ipaddr_temp[1], x.ipaddr_temp[2], x.ipaddr_temp[3]);
    x = StringToIPAddress(dns2);
    IPAddress dns(x.ipaddr_temp[0], x.ipaddr_temp[1], x.ipaddr_temp[2], x.ipaddr_temp[3]);
    // Serial.println(staticIP);
    // Serial.println(gateway);
    // Serial.println(subnet);
    // Serial.println(dns);
    WiFi.config(staticIP, gateway, subnet, dns);
  } else  //DHCP情况
  {
    IPAddress test(0, 0, 0, 0);
    WiFi.config(test, test, test, test);
  }

  //尝试连接WIFI
  WiFi.begin((char*)wifis.c_str(), (char*)wifip.c_str());
  for (int i = 0; i < 20; i++)  //超时判断
  {
    if (WiFi.status() == WL_CONNECTED)  //如果检测到状态为成功连接WIFI
    {
      // Serial.printf("SSID:%s\r\n", WiFi.SSID().c_str());
      // Serial.printf("PSW:%s\r\n", WiFi.psk().c_str());
      IPAD3 = WiFi.localIP().toString();

      //写入配置文件当前连接的WiFi
      // configWrite(config_fs, "pressid", (char*)wifis.c_str(), "/config.txt");
      // configWrite(config_fs, "prepassword", (char*)wifip.c_str(), "/config.txt");
      char filetxt[CONFIG_FILE_MAX_LENGTH] = { 0 };
      configWriteOpen(config_fs, "/config.txt", (char*)filetxt);
      configRewrite("pressid", (char*)wifis.c_str(), (char*)filetxt);
      configRewrite("prepassword", (char*)wifip.c_str(), (char*)filetxt);
      configWriteClose(config_fs, "/config.txt", (char*)filetxt);
      pressid = wifis;
      prepassword = wifip;
      if (ip2 == "1")  //保存静态IP
      {
        // configWrite(config_fs, "staticIP", (char*)staticIP2.c_str(), "/config.txt");
        // configWrite(config_fs, "gateway", (char*)gateway2.c_str(), "/config.txt");
        // configWrite(config_fs, "subnet", (char*)subnet2.c_str(), "/config.txt");
        // configWrite(config_fs, "dns", (char*)dns2.c_str(), "/config.txt");
        memset(filetxt, 0, sizeof(filetxt));
        configWriteOpen(config_fs, "/config.txt", (char*)filetxt);
        configRewrite("staticIP", (char*)staticIP2.c_str(), (char*)filetxt);
        configRewrite("gateway", (char*)gateway2.c_str(), (char*)filetxt);
        configRewrite("subnet", (char*)subnet2.c_str(), (char*)filetxt);
        configRewrite("dns", (char*)dns2.c_str(), (char*)filetxt);
        configWriteClose(config_fs, "/config.txt", (char*)filetxt);
      }
      server.send(200, "text/html", "连接成功 IP: " + IPAD3);   //发送连接的IP地址
      vTaskDelay(3000 / portTICK_PERIOD_MS);
      mode_switch = 0;  //函数跳出while循环，从而在loop函数中进入下一个模式
      
      return;  //如果成功连接，则返回到主函数

    } else {
      vTaskDelay(500 / portTICK_PERIOD_MS);
    }
  }
  server.send(200, "text/html", "连接失败");
}

//将IP地址字符串转换为结构体，分别存储4位IP地址
struct struct_ipaddr StringToIPAddress(String ipaddr) {
  struct struct_ipaddr y;
  y.ipaddr_temp[0] = 0;
  y.ipaddr_temp[1] = 0;
  y.ipaddr_temp[2] = 0;
  y.ipaddr_temp[3] = 0;
  uint8_t len = 0;
  uint8_t count = 0;
  uint8_t i = 0, j = 0, k = 0;
  uint8_t temp[3] = { 1, 10, 100 };
  len = ipaddr.length();
  for (i = 0; i < len; i++) {
    if (ipaddr[i] == '.')  //通过.来分割IP地址
    {
      for (k = 0; k < i - j; k++) {
        y.ipaddr_temp[count] += (ipaddr[i - k - 1] & 0x0f) * temp[k];  //个位*1 + 十位*10 + 百位*100
      }
      j = i + 1;
      count++;
    }
  }
  for (k = 0; k < i - j; k++) {
    y.ipaddr_temp[count] += (ipaddr[i - k - 1] & 0x0f) * temp[k];
  }
  return y;
}

//发送配置AP网页
void pageConfigAP() {
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
  message += htmlAP6;
  server.send(200, "text/html", message);
}

//保存修改的热点名称和密码
void configAP() {
  String ssid2 = server.arg("hotspotName");          //获取热点名称
  String password2 = server.arg("hotspotPassword");  //获取热点密码
  String channel2 = server.arg("channel");           //获取AP信道
  String hidden2 = server.arg("hidden");             //获取wifi隐身配置
  String str = "";
  char filetxt[CONFIG_FILE_MAX_LENGTH] = { 0 };
  char flag = 0;
  // flag = flag + configWrite(config_fs, "ssid", (char*)ssid2.c_str(), "/config.txt");
  // flag = flag + configWrite(config_fs, "password", (char*)password2.c_str(), "/config.txt");
  // flag = flag + configWrite(config_fs, "channel", (char*)channel2.c_str(), "/config.txt");
  configWriteOpen(config_fs, "/config.txt", filetxt);
  flag = flag + configRewrite("ssid", (char*)ssid2.c_str(), (char*)filetxt);
  flag = flag + configRewrite("password", (char*)password2.c_str(), (char*)filetxt);
  flag = flag + configRewrite("channel", (char*)channel2.c_str(), (char*)filetxt);
  configWriteClose(config_fs, "/config.txt", (char*)filetxt);

  if (flag == 3) {
    ssid = ssid2;
    password = password2;
    channel = String2Char((char*)channel2.c_str());
    ssid_hidden = String2Char((char*)hidden2.c_str());
    str += htmlAP1 + "<h2>配置成功</h2>" + htmlfoot;
    server.send(200, "text/html", str);
  } else {
    str += htmlAP1 + "<h2>配置失败</h2>" + htmlfoot;
    server.send(200, "text/html", str);
  }
}

//发送配置WiFi自动连接网页
void pageConfigAutoConnect() {
  String message = htmlAutoConnect1 + htmlAutoConnect2;
  if (autoconnect) {
    message += ">关闭<input name=\"autoconnect\" type=\"radio\" value=\"1\" checked>开启";
  } else {
    message += " checked>关闭<input name=\"autoconnect\" type=\"radio\" value=\"1\">开启";
  }
  message += htmlAutoConnect3;
  message += pressid;
  message += htmlAutoConnect4;
  message += prepassword;
  message += htmlAutoConnect5;
  server.send(200, "text/html", message);
}

//保存WiFi自动连接配置
void configAutoConnect() {
  String autoconnect2 = server.arg("autoconnect");  //获取自动连接开关
  String pressid2 = server.arg("pressid");          //获取WiFi名称
  String prepassword2 = server.arg("prepassword");  //获取WiFi密码
  String str = "";
  char filetxt[CONFIG_FILE_MAX_LENGTH] = { 0 };
  char flag = 0;
  // flag = flag + configWrite(config_fs, "autoconnect", (char*)autoconnect2.c_str(), "/config.txt");
  // flag = flag + configWrite(config_fs, "pressid", (char*)pressid2.c_str(), "/config.txt");
  // flag = flag + configWrite(config_fs, "prepassword", (char*)prepassword2.c_str(), "/config.txt");
  configWriteOpen(config_fs, "/config.txt", filetxt);
  flag = flag + configRewrite("autoconnect", (char*)autoconnect2.c_str(), (char*)filetxt);
  flag = flag + configRewrite("pressid", (char*)pressid2.c_str(), (char*)filetxt);
  flag = flag + configRewrite("prepassword", (char*)prepassword2.c_str(), (char*)filetxt);
  configWriteClose(config_fs, "/config.txt", (char*)filetxt);

  if (flag == 3) {
    autoconnect = String2Char((char*)autoconnect2.c_str());
    pressid = pressid2;
    prepassword = prepassword2;
    str += htmlAutoConnect1 + "<h2>配置成功</h2>" + htmlfoot;
    server.send(200, "text/html", str);
  } else {
    str += htmlAutoConnect1 + "<h2>配置失败</h2>" + htmlfoot;
    server.send(200, "text/html", str);
    
  }
}

void wifi_handleNotFound() {
  server.send(404, "text/html", "Not found");
}
