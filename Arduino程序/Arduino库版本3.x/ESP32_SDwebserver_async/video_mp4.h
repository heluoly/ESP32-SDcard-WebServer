#ifndef __VIDEO_MP4_H
#define __VIDEO_MP4_H

#include "common.h"

String readTxtFile(fs::FS &fs, const char *path);
void listVideo_mp4(AsyncWebServerRequest *request);
void openVideo_mp4(AsyncWebServerRequest *request);

#endif