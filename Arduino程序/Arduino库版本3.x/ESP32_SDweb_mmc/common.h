#ifndef __COMMON_H
#define __COMMON_H

#include "FS.h"
#include "SD_MMC.h"

#define configMaximumLength 1024   //配置文件缓冲区数值

//String indexOfFilename(String filename);
String formatBytes(size_t bytes);
void listDir(fs::FS &fs, const char * dirname, uint8_t levels);
char deleteFile(fs::FS &fs, const char * path);
char writeFile(fs::FS &fs, const char * path, const char * message);
char String2Char(char *str);
char configRead(fs::FS &fs , const char *key, const char *filename, char *buf);
char configWrite(fs::FS &fs , const char *key, const char *val, const char *filename);

#endif