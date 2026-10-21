#ifndef RANGEFILERESPONSE_H
#define RANGEFILERESPONSE_H

#include "common.h"

void rangeFileServe(AsyncWebServerRequest *request, const String &path);
void rangeFileServeDownload(AsyncWebServerRequest *request, const String &path, const String &downloadName);

#endif