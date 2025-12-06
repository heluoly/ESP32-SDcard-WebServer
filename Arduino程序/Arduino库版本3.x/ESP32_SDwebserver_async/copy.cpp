#include "copy.h"

const char* dataFilePath = "/copy.txt";

// 保存文本
bool saveTextToFile(const String& text) {
  File file = my_fs.open(dataFilePath, FILE_WRITE);
  if (!file) {
    // Serial.println("Failed to open file for writing");
    return false;
  }
  
  size_t bytesWritten = file.write((const uint8_t*)text.c_str(), text.length());
  file.close();
  
  return (bytesWritten == text.length());
}


// 读取文本
String readTextFromFile() {
  if (!my_fs.exists(dataFilePath)) {
    return String("");  // 文件不存在，返回空字符串
  }
  
  File file = my_fs.open(dataFilePath, FILE_READ);
  if (!file) {
    // Serial.println("Failed to open file for reading");
    return String("");
  }
  
  String content = file.readString();
  file.close();
  
  return content;
}


// 处理获取文本请求
void handleGetText(AsyncWebServerRequest *request) {
  String text = readTextFromFile();
  request->send(200, "text/plain", text);
}

// 处理保存文本请求
void handleSaveText(AsyncWebServerRequest *request) {
  String response = "ERROR";
  if (request->hasParam("text", true)) {
    String text = request->getParam("text", true)->value();
    if (saveTextToFile(text)) {
      response = "OK";
      // Serial.println("Text saved: " + text);
    }
  }
  request->send(200, "text/plain", response);
}

