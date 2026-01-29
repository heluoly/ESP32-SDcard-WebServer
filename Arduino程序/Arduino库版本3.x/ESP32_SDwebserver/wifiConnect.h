#ifndef __WIFICONNECT_H
#define __WIFICONNECT_H

#include "common.h"
#include <WiFi.h>

void handleRoot();
void HandleScanWifi();
void HandleWifi();
void pageConfigAP();
void configAP();
void pageConfigAutoConnect();
void configAutoConnect();
void wifi_handleNotFound();
struct struct_ipaddr StringToIPAddress(String ipaddr);

#endif