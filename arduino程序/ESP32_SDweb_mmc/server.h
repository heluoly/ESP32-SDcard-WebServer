#ifndef __SERVER_H
#define __SERVER_H

#include <WiFi.h>
#include <WebServer.h> 
#include "FS.h"
#include "SD_MMC.h"
#include "common.h"
#include "game.h"
#include "upload.h"
#include "video.h"
#include "web.h"
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

#endif