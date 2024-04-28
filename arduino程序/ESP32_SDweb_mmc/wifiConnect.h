#ifndef __WIFICONNECT_H
#define __WIFICONNECT_H

#include <WiFi.h>
#include <WebServer.h>
#include "SD_MMC.h"
#include "common.h"

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