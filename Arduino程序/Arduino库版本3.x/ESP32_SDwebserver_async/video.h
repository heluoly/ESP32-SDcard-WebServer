#ifndef __VIDEO_H
#define __VIDEO_H

#include "common.h"

void listVideoCategories(AsyncWebServerRequest *request);
void listvideo(AsyncWebServerRequest *request);
void openVideo(AsyncWebServerRequest *request);

#endif