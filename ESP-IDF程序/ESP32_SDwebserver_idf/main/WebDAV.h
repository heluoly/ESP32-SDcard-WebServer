#ifndef WEBDAV_H
#define WEBDAV_H

#include "common.h"
#include "rangeFileResponse.h"

void handleWebDAV(AsyncWebServerRequest *request);
void startWebDavService();
void stopWebDavService();
void handleWebDavService();

#endif