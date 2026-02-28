#ifndef __MYSERVER_H
#define __MYSERVER_H

#include "common.h"
#include <WiFi.h>
#include "lwip/lwip_napt.h"
#include <lwip/tcpip.h>
#include "game.h"
#include "upload.h"
#include "video.h"
#include "video_mp4.h"
#include "copy.h"
#include "wifiConnect.h"
#include "serverConfig.h"
#include "oledClock.h"

void wifiConnect(AsyncWebServerRequest *request);
void backToAP(AsyncWebServerRequest *request);
void serverConfig(AsyncWebServerRequest *request);
void WiFiconfigRead();
void server_ap();
void server_sta();
void server_ap_sta();
void server_wifi_connect();
void server_config();
void handleUserRequest(AsyncWebServerRequest *request);

#endif