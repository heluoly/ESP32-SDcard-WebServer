#ifndef __SERVERCONFIG_H
#define __SERVERCONFIG_H

#include "common.h"
#include <WiFi.h>

void pageConfigAP(AsyncWebServerRequest *request);
void configAP(AsyncWebServerRequest *request);
void pageConfigAutoConnect(AsyncWebServerRequest *request);
void configAutoConnect(AsyncWebServerRequest *request);
void config_handleNotFound(AsyncWebServerRequest *request);

#endif