#include <WiFi.h>
#include <WebServer.h>
#include "SD_MMC.h"
#include "common.h"

void handleRoot();
void HandleScanWifi();
void HandleWifi();
void pageConfigAP();
void configAP();
void wifi_handleNotFound();
struct struct_ipaddr StringToIPAddress(String ipaddr);
