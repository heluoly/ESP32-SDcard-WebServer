#ifndef __MYSERVER_H
#define __MYSERVER_H

#include <WiFi.h>
#include "common.h"
#include "game.h"
#include "upload.h"
#include "video.h"
#include "copy.h"
#include "wifiConnect.h"
#include "oledClock.h"

void changemode(AsyncWebServerRequest *request);
void backToAP(AsyncWebServerRequest *request);
void WiFiconfigRead();
void server_ap();
void server_ap_sta();
void server_sta();
void server_presta();
void handleUserRequest(AsyncWebServerRequest *request);

#endif