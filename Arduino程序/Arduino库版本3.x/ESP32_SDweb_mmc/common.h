#ifndef __COMMON_H
#define __COMMON_H

#include "FS.h"
#include "SD_MMC.h"
#include "SPIFFS.h"

#define configMaximumLength 1024   //配置文件缓冲区数值
#define my_fs SD_MMC
//选择配置文件储存位置，0为SPIFFS，1为SD_MMC
#define CONFIG_SD 0
#if CONFIG_SD
#define config_fs SD_MMC
#else
#define config_fs SPIFFS
#endif


//String indexOfFilename(String filename);
String formatBytes(size_t bytes);
void listDir(fs::FS &fs, const char * dirname, uint8_t levels);
char deleteFile(fs::FS &fs, const char * path);
char writeFile(fs::FS &fs, const char * path, const char * message);
char String2Char(char *str);
char configRead(fs::FS &fs , const char *key, const char *filename, char *buf);
char configWrite(fs::FS &fs , const char *key, const char *val, const char *filename);
//一次写入配置文件多个参数
char configWriteOpen(fs::FS &fs, const char *filename, char *filetxt);
char configRewrite(const char *key, const char *val, char *filetxt);
char configWriteClose(fs::FS &fs, const char *filename, char *filetxt);

#endif