#ifndef __COMMON_H
#define __COMMON_H

#include "FS.h"
#include "SD_MMC.h"
#include "FFat.h"
#include <ESPAsyncWebServer.h>

#define CONFIG_FILE_MAX_LENGTH 1024  //配置文件缓冲区数值
#define CONFIG_MAX_LENGTH 256        //单行缓冲区数值
#define my_fs SD_MMC
//选择配置文件储存位置，0为flash，1为SD_MMC
#define CONFIG_SD 0
#if CONFIG_SD
#define config_fs SD_MMC
#else
#define config_fs FFat
#endif

#define BTN_BOOT_PIN 0  //IO0作为按键
// #define LED 33         //IO33作为led指示灯

typedef enum {
  MY_SERVER_STATE_AP = 1,
  MY_SERVER_STATE_STA = 2,
  MY_SERVER_STATE_AP_STA = 3,
  MY_SERVER_STATE_WIFI_CONNECT = 4,
  MY_SERVER_STATE_CONFIG = 5
} my_server_state_t;

typedef enum {
  MY_SERVER_DP_STATE_ERROR = 0,
  MY_SERVER_DP_STATE_AP = 1,
  MY_SERVER_DP_STATE_STA = 2,
  MY_SERVER_DP_STATE_AP_STA = 3,
  MY_SERVER_DP_STATE_WIFI_SCAN = 4,
  MY_SERVER_DP_STATE_CONFIG = 5,
  MY_SERVER_DP_STATE_STA_CONNECT = 6,
  MY_SERVER_DP_STATE_AP_STA_CONNECT = 7
} my_server_display_state_t;

String formatBytes(size_t bytes);
// void listDir(fs::FS &fs, const char *dirname, uint8_t levels);
char deleteFile(fs::FS &fs, const char *path);
char writeFile(fs::FS &fs, const char *path, const char *message);
void downloadFile(AsyncWebServerRequest *request);
char String2Char(char *str);

bool configRead(fs::FS &fs, const char *key, const char *filename, char *buf, int bufSize);
bool configWrite(fs::FS &fs, const char *key, const char *val, const char *filename);
// bool configAdd(fs::FS &fs, const char *key, const char *val, const char *filename);
// bool configDelete(fs::FS &fs, const char *key, const char *filename);
//一次写入配置文件多个参数
char configWriteOpen(fs::FS &fs, const char *filename, char *filetxt);
char configRewrite(const char *key, const char *val, char *filetxt);
char configWriteClose(fs::FS &fs, const char *filename, char *filetxt);


#endif