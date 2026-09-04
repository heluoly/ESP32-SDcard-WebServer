#ifndef RANGEFILERESPONSE_H
#define RANGEFILERESPONSE_H

#include "common.h"

void rangeFileServe(AsyncWebServerRequest *request, const String& path);

#endif