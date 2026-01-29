#ifndef __COMMON_H
#define __COMMON_H

#include "FS.h"
#include "SD_MMC.h"
#include "FFat.h"
#include <WebServer.h>

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

String formatBytes(size_t bytes);
char deleteFile(fs::FS &fs, const char *path);
char writeFile(fs::FS &fs, const char *path, const char *message);
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