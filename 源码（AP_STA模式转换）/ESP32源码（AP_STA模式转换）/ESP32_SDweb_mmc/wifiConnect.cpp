#include"wifiConnect.h"

extern WebServer server;
extern char mode_switch;;


const String html1 = "<!DOCTYPE html><html><head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\"><meta http-equiv=\"X-UA-Compatible\" content=\"ie=edge\"><title>网页配网</title><script type=\"text/javascript\">function wifi(){var ssid = s.value;var password = p.value;var ip2 = ip.value;var staticIP2 = staticIP.value;var gateway2 = gateway.value;var subnet2 = subnet.value;var dns2 = dns.value;var xmlhttp=new XMLHttpRequest();xmlhttp.open(\"GET\",\"/HandleWifi?ssid=\"+ssid+\"&password=\"+password+\"&ip=\"+ip2+\"&staticIP=\"+staticIP2+\"&gateway=\"+gateway2+\"&subnet=\"+subnet2+\"&dns=\"+dns2,true);xmlhttp.send();xmlhttp.onload = function(e){alert(this.responseText);}}</script><script>function c(l){document.getElementById('s').value=l.innerText||l.textContent;document.getElementById('p').focus();}</script><script>function agree(){if(document.getElementById('ip').checked){document.getElementById('ip').value=\"1\";document.getElementById('staticIP').disabled=false;document.getElementById('gateway').disabled=false;document.getElementById('subnet').disabled=false;document.getElementById('dns').disabled=false;}else{document.getElementById('ip').value=\"0\";document.getElementById('staticIP').disabled='disabled';document.getElementById('gateway').disabled='disabled';document.getElementById('subnet').disabled='disabled';document.getElementById('dns').disabled='disabled';}}</script></head><body><h2>网页配网</h2><form>WiFi名称：<input id='s' name='s' type=\"text\" placeholder=\"请输入您WiFi的名称\"><br>WiFi密码：<input id='p' name='p' type=\"text\" placeholder=\"请输入您WiFi的密码\"><br><label for=\"ip\">静态IP</label><input name=\"ip\" id=\"ip\" type=\"checkbox\" onclick=\"agree();\"><br>IP地址：<input name=\"staticIP\" id=\"staticIP\" type=\"text\" value=\"192.168.0.80\" disabled=\"\"><br>网关：<input name=\"gateway\" id=\"gateway\" type=\"text\" value=\"192.168.1.1\" disabled=\"\"><br>子网：<input name=\"subnet\" id=\"subnet\" type=\"text\" value=\"255.255.255.0\" disabled=\"\"><br>DNS：<input name=\"dns\" id=\"dns\" type=\"text\" value=\"223.5.5.5\" disabled=\"\"><br><br><input type=\"button\" value=\"扫描\" onclick=\"window.location.href = '/HandleScanWifi'\"> <input type=\"button\" value=\"连接\" onclick=\"wifi()\"></form>";
const String html2 = "<p><a href=\"/\">返回AP模式</a>  <a href=\"/pageConfigAP\">配置热点</a></p></body></html>";

//定义一个结构体，用于存放4位IP地址
struct struct_ipaddr
{
  uint8_t ipaddr_temp[4];
};

//发送配网页面
void handleRoot() {
  
  String str = html1 + html2;
  server.send(200, "text/html", str);
}

//扫描附近WIFI并返回
void HandleScanWifi() {
//  Serial.println("scan start");

  String scanstr = "";
  // WiFi.scanNetworks will return the number of networks found
  int n = WiFi.scanNetworks();
//  Serial.println("scan done");
  scanstr += html1;
  if (n <= 0) {
    Serial.println("no networks found");
    scanstr += "NO WIFI !!!";
  }
  else {
//    Serial.print(n);
//    Serial.println(" networks found");
    scanstr += "<table><tr><th>序号</th><th>名称</th><th>强度</th></tr>";
    for (int i = 0; i < n; i++) {
      // Print SSID and RSSI for each network found
     // Serial.print(i + 1);
     // Serial.print(": ");
     // Serial.print(WiFi.SSID(i));
     // Serial.print(" (");
     // Serial.print(WiFi.RSSI(i));
     // Serial.print(")");
      scanstr += "<tr><td align=\"center\">" + String(i + 1) + "</td><td align=\"center\">" + "<a href='#p' onclick='c(this)'>" + WiFi.SSID(i) + "</a>" + "</td><td align=\"center\">" + WiFi.RSSI(i) + "</td></tr>";
    }
   // Serial.println("");
    scanstr += "</table>";
  }
  scanstr += html2;
  server.send(200, "text/html", scanstr);
  
  
}

//尝试连接网页发送的WIFI
void HandleWifi()
{
  String wifis = server.arg("ssid");    //获取WIFI名称
  String wifip = server.arg("password");    //获取WIFI密码
  String ip2 = server.arg("ip");    //判断是DHCP连接还是静态IP连接
  String staticIP2 = server.arg("staticIP");    //静获取态IP地址
  String gateway2 = server.arg("gateway");    //静获取态IP网关
  String subnet2 = server.arg("subnet");    //静获取态IP子网
  String dns2 = server.arg("dns");    //静获取态IP的dns
  struct_ipaddr x;
  String IPAD3 = "";
  Serial.println("received:" + wifis);
/*
  Serial.println("ip:" + ip2);
  Serial.println("staticIP:" + staticIP2);
  Serial.println("gateway:" + gateway2);
  Serial.println("subnet:" + subnet2);
  Serial.println("dns:" + dns2);
  */
  WiFi.disconnect(true,true);

  
  if(ip2=="1")  //配置静态IP情况
  {
    x = StringToIPAddress(staticIP2);
    IPAddress staticIP(x.ipaddr_temp[0],x.ipaddr_temp[1],x.ipaddr_temp[2],x.ipaddr_temp[3]);
    x = StringToIPAddress(gateway2);
    IPAddress gateway(x.ipaddr_temp[0],x.ipaddr_temp[1],x.ipaddr_temp[2],x.ipaddr_temp[3]);
    x = StringToIPAddress(subnet2);
    IPAddress subnet(x.ipaddr_temp[0],x.ipaddr_temp[1],x.ipaddr_temp[2],x.ipaddr_temp[3]);
    x = StringToIPAddress(dns2);
    IPAddress dns(x.ipaddr_temp[0],x.ipaddr_temp[1],x.ipaddr_temp[2],x.ipaddr_temp[3]);
    Serial.println(staticIP);
    Serial.println(gateway);
    Serial.println(subnet);
    Serial.println(dns);
    WiFi.config(staticIP, gateway, subnet, dns);
  }
  else    //DHCP情况
  {
    IPAddress test(0,0,0,0);
    WiFi.config(test, test, test, test);
  }
  
  //尝试连接WIFI
  WiFi.begin((char*)wifis.c_str(), (char*)wifip.c_str());
  for (int i = 0; i < 20; i++)        //超时判断
  {
    if (WiFi.status() == WL_CONNECTED)    //如果检测到状态为成功连接WIFI
    {
      vTaskDelay(2000/portTICK_PERIOD_MS);
      Serial.printf("SSID:%s\r\n", WiFi.SSID().c_str());
      Serial.printf("PSW:%s\r\n", WiFi.psk().c_str());
      IPAD3 = WiFi.localIP().toString();
      server.send(200, "text/html", "连接成功 IP：" + IPAD3);    //发送连接的IP地址
      vTaskDelay(10000/portTICK_PERIOD_MS);
      mode_switch = 0;                             //函数跳出while循环，从而在loop函数中进入下一个模式
      return;                                      //如果成功连接，则返回到主函数
    }
    else
    {
      vTaskDelay(500/portTICK_PERIOD_MS);
    }
  }
  server.send(200, "text/html", "连接失败");
  
}

//将IP地址字符串转换为结构体，分别存储4位IP地址
struct struct_ipaddr StringToIPAddress(String ipaddr){
  struct struct_ipaddr y;
  y.ipaddr_temp[0]=0;
  y.ipaddr_temp[1]=0;
  y.ipaddr_temp[2]=0;
  y.ipaddr_temp[3]=0;
  uint8_t len = 0;
  uint8_t count = 0;
  uint8_t i = 0,j = 0,k = 0;
  uint8_t temp[3]={1,10,100};
  len = ipaddr.length();
  for(i=0;i<len;i++)
  {
    if(ipaddr[i]=='.')    //通过.来分割IP地址
    {
      for(k=0;k<i-j;k++){
        y.ipaddr_temp[count] += (ipaddr[i-k-1]&0x0f)*temp[k];   //个位*1 + 十位*10 + 百位*100
      }
      j=i+1;
      count++;
    }
  }
  for(k=0;k<i-j;k++){
    y.ipaddr_temp[count] += (ipaddr[i-k-1]&0x0f)*temp[k];
  }
  return y;

}


void wifi_handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}
