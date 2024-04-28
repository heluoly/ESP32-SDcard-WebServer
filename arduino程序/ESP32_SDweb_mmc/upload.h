#ifndef __UPLOAD_H
#define __UPLOAD_H

#include "common.h"
#include <WebServer.h>

void listUploadFile();
void handleFileUpload();
void deleteUploadFile();
void downloadUploadFile();

#endif