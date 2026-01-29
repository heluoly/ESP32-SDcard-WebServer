#ifndef __MYSERVER_H
#define __MYSERVER_H

#include "common.h"
#include <WiFi.h>
#include "game.h"
#include "upload.h"
#include "video.h"
#include "copy.h"
#include "wifiConnect.h"
#include "oledClock.h"

void changemode();
void backToAP();
void WiFiconfigRead();
void server_ap();
void server_ap_sta();
void server_sta();
void server_presta();
void handleUserRequest();

#endif