#include <WiFi.h>
#include <WebServer.h>

void handleRoot();
void HandleScanWifi();
void HandleWifi();
void wifi_handleNotFound();
struct struct_ipaddr StringToIPAddress(String ipaddr);
