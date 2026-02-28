#ifndef __WIFICONNECT_H
#define __WIFICONNECT_H

#include "common.h"
#include <WiFi.h>

void handleRoot(AsyncWebServerRequest *request);
void HandleScanWifi(AsyncWebServerRequest *request);
void HandleWifi(AsyncWebServerRequest *request);
void wifi_handleNotFound(AsyncWebServerRequest *request);
struct struct_ipaddr StringToIPAddress(String ipaddr);

#endif