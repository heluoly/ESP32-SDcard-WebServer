#ifndef __WIFICONNECT_H
#define __WIFICONNECT_H

#include <WiFi.h>
#include "common.h"

void handleRoot(AsyncWebServerRequest *request);
void HandleScanWifi(AsyncWebServerRequest *request);
void HandleWifi(AsyncWebServerRequest *request);
void pageConfigAP(AsyncWebServerRequest *request);
void configAP(AsyncWebServerRequest *request);
void pageConfigAutoConnect(AsyncWebServerRequest *request);
void configAutoConnect(AsyncWebServerRequest *request);
void wifi_handleNotFound(AsyncWebServerRequest *request);
struct struct_ipaddr StringToIPAddress(String ipaddr);

#endif