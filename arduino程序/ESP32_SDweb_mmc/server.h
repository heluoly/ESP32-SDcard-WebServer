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
void readFile3(fs::FS &fs, const char * path);
void server_ap();
void server_ap_sta();
void server_sta();
