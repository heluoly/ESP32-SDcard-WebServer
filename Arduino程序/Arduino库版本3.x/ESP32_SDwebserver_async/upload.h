#ifndef __UPLOAD_H
#define __UPLOAD_H

#include "common.h"

void listUploadFile(AsyncWebServerRequest *request);
void uploadFileRespond(AsyncWebServerRequest *request);
void handleFileUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final);
void deleteUploadFile(AsyncWebServerRequest *request);
void downloadUploadFile(AsyncWebServerRequest *request);

#endif