#include "common.h"

#define CONFIG_LINE_END_WINDOWS 1  // \r\n
#define CONFIG_LINE_END_UNIX 2     // \n

//计算文件大小
String formatBytes(size_t bytes) {
  if (bytes < 1024) {
    return String(bytes) + "B";
  } else if (bytes < (1024 * 1024)) {
    return String(bytes / 1024.0) + "KB";
  } else if (bytes < (1024 * 1024 * 1024)) {
    return String(bytes / 1024.0 / 1024.0) + "MB";
  } else {
    return String(bytes / 1024.0 / 1024.0 / 1024.0) + "GB";
  }
}


void listDir(fs::FS &fs, const char *dirname, uint8_t levels) {
  // Serial.printf("Listing directory: %s\n", dirname);

  File root = fs.open(dirname);
  if (!root) {
    // Serial.println("Failed to open directory");
    return;
  }
  if (!root.isDirectory()) {
    // Serial.println("Not a directory");
    return;
  }

  File file = root.openNextFile();
  while (file) {
    if (file.isDirectory()) {
      // Serial.print("  DIR : ");
      // Serial.println(file.name());
      if (levels) {
        listDir(fs, file.name(), levels - 1);
      }
    } else {
      // Serial.print("  FILE: ");
      // Serial.print(file.name());
      // Serial.print("  SIZE: ");
      // Serial.println(file.size());
    }
    file = root.openNextFile();
  }
}

char deleteFile(fs::FS &fs, const char *path) {
  // Serial.printf("Deleting file: %s\n", path);
  if (fs.remove(path)) {
    return 1;
  } else {
    return 0;
  }
}

char writeFile(fs::FS &fs, const char *path, const char *message) {
  File file = fs.open(path, FILE_WRITE);
  if (!file) {
    return 0;
  }
  if (file.print(message)) {
    file.close();
    return 1;
  } else {
    file.close();
    return 0;
  }
}

//下载带中文名文件
void downloadFile(AsyncWebServerRequest *request) {
  String filePath = request->getParam("filePath")->value();
  AsyncWebServerResponse *response = request->beginResponse(my_fs, filePath, String());
  request->send(response);
}

//字符串转数字
char String2Char(char *str) {
  char flag = '+';  //指示结果是否带符号
  long res = 0;

  if (*str == '-')  //字符串带负号
  {
    ++str;       //指向下一个字符
    flag = '-';  //将标志设为负号
  }
  //逐个字符转换，并累加到结果res
  while (*str >= 48 && *str <= 57)  //如果是数字才进行转换，数字0~9的ASCII码：48~57
  {
    res = 10 * res + *str++ - 48;  //字符'0'的ASCII码为48,48-48=0刚好转化为数字0
  }

  if (flag == '-')  //处理是负数的情况
  {
    res = -res;
  }

  return (char)res;
}


// 读取一行配置，返回行结束符类型
int readConfigLine(File &file, char *buffer, int bufferSize) {
  int index = 0;
  char temp;
  
  while (file.available() && index < bufferSize - 1) {
    temp = file.read();
    
    // 处理行结束符
    if (temp == '\r') {
      buffer[index] = '\0';
      // 检查下一个字符是否是\n
      if (file.available() && file.peek() == '\n') {
        file.read(); // 消耗\n
        return CONFIG_LINE_END_WINDOWS;
      }
      return CONFIG_LINE_END_WINDOWS; // 只有\r的情况
    }
    
    if (temp == '\n') {
      buffer[index] = '\0';
      return CONFIG_LINE_END_UNIX;
    }
    
    // 处理文件结束（\0字符file.available()返回false，这里可能用不到）
    if (temp == '\0') {
      buffer[index] = '\0';
      return 0; // 文件结束
    }
    
    buffer[index++] = temp;
  }
  
  // 检查文件是否结束
  if (!file.available() && index > 0) {
    // 文件结束，但已经读取了一些数据（最后一行没有换行符）
    buffer[index] = '\0';
    return 0; // 文件结束，但成功读取了一行
  }
  
  // 缓冲区已满但文件中还有数据
  if (index >= bufferSize - 1 && file.available()) {
    // 缓冲区不足，但文件中还有数据
    if (index < bufferSize) {
      buffer[index] = '\0'; // 确保字符串以\0结尾
    }
    return -1; // 缓冲区不足
  }
  
  // 没有读取到任何数据
  return -1; // 文件结束或错误
}

// 解析键值对
bool parseKeyValue(const char *line, const char *key, char *value, int valueSize) {
  // 查找键
  if (strncmp(key, line, strlen(key)) != 0) {
    return false;
  }
  
  // 查找等号
  const char *equalSign = strchr(line, '=');
  if (equalSign == NULL) {
    return false;
  }
  
  // 跳过等号复制值
  const char *valueStart = equalSign + 1;
  int valueLength = strlen(valueStart);
  
  // 确保不会溢出缓冲区
  if (valueLength >= valueSize) {
    strncpy(value, valueStart, valueSize - 1);
    value[valueSize - 1] = '\0';
  } else {
    strcpy(value, valueStart);
  }
  
  return true;
}

// 读取配置文件
bool configRead(fs::FS &fs, const char *key, const char *filename, char *buf, int bufSize) {
  if (!key || !filename || !buf || bufSize <= 0) {
    return false;
  }
  
  File file = fs.open(filename, FILE_READ);
  if (!file) {
    // Serial.printf("Failed to open file for reading: %s\n", filename);
    return false;
  }
  
  char lineBuffer[CONFIG_MAX_LENGTH];
  bool found = false;
  
  while (file.available() && !found) {
    int lineEndType = readConfigLine(file, lineBuffer, sizeof(lineBuffer));
    
    if (lineEndType < 0) {
      // Serial.println("Line too long or buffer overflow");
      break;
    }
    
    // 跳过空行和注释行
    if (strlen(lineBuffer) == 0 || lineBuffer[0] == '#') {
      continue;
    }
    
    // 解析键值对
    if (parseKeyValue(lineBuffer, key, buf, bufSize)) {
      found = true;
    }
  }
  
  file.close();
  return found;
}

// 修改配置文件
bool configWrite(fs::FS &fs, const char *key, const char *val, const char *filename) {
  if (!key || !val || !filename) {
    return false;
  }
  
  // 先读取整个文件到内存（配置文件通常不大）
  File file = fs.open(filename, FILE_READ);
  if (!file) {
    // Serial.printf("Failed to open file for reading: %s\n", filename);
    return false;
  }
  
  // 计算文件大小
  size_t fileSize = file.size();
  if (fileSize > 4096) { // 限制配置文件大小
    // Serial.println("Config file too large");
    file.close();
    return false;
  }
  
  // 分配内存存储文件内容
  char *fileContent = new char[fileSize + 1];
  if (!fileContent) {
    // Serial.println("Memory allocation failed");
    file.close();
    return false;
  }
  
  // 读取文件内容
  size_t bytesRead = file.readBytes(fileContent, fileSize);
  fileContent[bytesRead] = '\0';
  file.close();
  
  // 处理文件内容
  bool keyFound = false;
  String output;
  int lineStart = 0;
  
  for (int i = 0; i <= bytesRead; i++) {
    // 检测行结束或文件结束
    if (i == bytesRead || fileContent[i] == '\n' || fileContent[i] == '\r') {
      // 提取一行
      int lineLength = i - lineStart;
      if (lineLength > 0) {
        char line[CONFIG_MAX_LENGTH];
        strncpy(line, &fileContent[lineStart], min(lineLength, CONFIG_MAX_LENGTH - 1));
        line[min(lineLength, CONFIG_MAX_LENGTH - 1)] = '\0';
        
        // 解析并处理键值对
        char currentKey[CONFIG_MAX_LENGTH];
        char currentValue[CONFIG_MAX_LENGTH];
        
        if (parseKeyValue(line, key, currentValue, sizeof(currentValue))) {
          // 找到目标键，替换值
          output += key;
          output += "=";
          output += val;
          keyFound = true;
        } else {
          // 保持原行不变
          output += line;
        }
      }
      
      // 添加行结束符（保持原格式）
      if (i < bytesRead) {
        if (fileContent[i] == '\r' && i + 1 < bytesRead && fileContent[i + 1] == '\n') {
          output += "\r\n";
          i++; // 跳过\n
        } else {
          output += fileContent[i];
        }
      }
      
      lineStart = i + 1;
    }
  }
  
  // 如果键不存在，在文件末尾添加
  // if (!keyFound) {
  //   // 确保以换行符开始
  //   if (bytesRead > 0 && fileContent[bytesRead - 1] != '\n') {
  //     output += "\n";
  //   }
  //   output += key;
  //   output += "=";
  //   output += val;
  //   output += "\n";
  // }
  
  // 释放内存
  delete[] fileContent;
  
  // 写入文件
  file = fs.open(filename, FILE_WRITE);
  if (!file) {
    // Serial.printf("Failed to open file for writing: %s\n", filename);
    return false;
  }
  
  size_t bytesWritten = file.print(output);
  file.close();
  
  return (bytesWritten > 0);
}

// 添加配置项（如果不存在）
bool configAdd(fs::FS &fs, const char *key, const char *val, const char *filename) {
  char buffer[CONFIG_MAX_LENGTH];
  
  // 检查是否已存在
  if (configRead(fs, key, filename, buffer, sizeof(buffer))) {
    return false; // 已存在
  }
  
  // 追加到文件末尾
  File file = fs.open(filename, FILE_APPEND);
  if (!file) {
    // Serial.printf("Failed to open file for appending: %s\n", filename);
    return false;
  }
  
  String line = String(key) + "=" + val + "\n";
  size_t bytesWritten = file.print(line);
  file.close();
  
  return (bytesWritten == line.length());
}

// 删除配置项
bool configDelete(fs::FS &fs, const char *key, const char *filename) {
  // 类似于configWrite，但跳过要删除的行
  File file = fs.open(filename, FILE_READ);
  if (!file) {
    // Serial.printf("Failed to open file for reading: %s\n", filename);
    return false;
  }
  
  size_t fileSize = file.size();
  if (fileSize > CONFIG_FILE_MAX_LENGTH) {
    // Serial.println("Config file too large");
    file.close();
    return false;
  }
  
  char *fileContent = new char[fileSize + 1];
  if (!fileContent) {
    // Serial.println("Memory allocation failed");
    file.close();
    return false;
  }
  
  size_t bytesRead = file.readBytes(fileContent, fileSize);
  fileContent[bytesRead] = '\0';
  file.close();
  
  String output;
  int lineStart = 0;
  bool keyFound = false;
  
  for (int i = 0; i <= bytesRead; i++) {
    if (i == bytesRead || fileContent[i] == '\n' || fileContent[i] == '\r') {
      int lineLength = i - lineStart;
      if (lineLength > 0) {
        char line[CONFIG_MAX_LENGTH];
        strncpy(line, &fileContent[lineStart], min(lineLength, CONFIG_MAX_LENGTH - 1));
        line[min(lineLength, CONFIG_MAX_LENGTH - 1)] = '\0';
        
        // 检查是否为要删除的键
        char currentKey[CONFIG_MAX_LENGTH];
        char currentValue[CONFIG_MAX_LENGTH];
        
        if (!parseKeyValue(line, key, currentValue, sizeof(currentValue))) {
          // 不是目标键，保留该行
          output += line;
          
          // 添加行结束符
          if (i < bytesRead) {
            if (fileContent[i] == '\r' && i + 1 < bytesRead && fileContent[i + 1] == '\n') {
              output += "\r\n";
              i++;
            } else {
              output += fileContent[i];
            }
          }
        } else {
          keyFound = true;
          // 跳过行结束符的添加，即删除该行
        }
      } else {
        // 空行，保持格式
        if (i < bytesRead) {
          output += fileContent[i];
        }
      }
      
      lineStart = i + 1;
    }
  }
  
  delete[] fileContent;
  
  // 写入文件
  file = fs.open(filename, FILE_WRITE);
  if (!file) {
    // Serial.printf("Failed to open file for writing: %s\n", filename);
    return false;
  }
  
  size_t bytesWritten = file.print(output);
  file.close();
  
  return keyFound && (bytesWritten > 0);
}

// char filetxt[CONFIG_FILE_MAX_LENGTH] = { 0 };
//一次写入配置文件多个参数1
char configWriteOpen(fs::FS &fs, const char *filename, char *filetxt) {
  uint16_t i = 0;

  File file = fs.open(filename);
  if (!file) {
    // Serial.println("Failed to open file for reading");
    return 0;
  }
  while (file.available()) {
    if (i < CONFIG_FILE_MAX_LENGTH - 1) {
      filetxt[i] = file.read();
      i++;
    } else {
      filetxt[i] = '\0';
      break;
    }
  }
  file.close();
  return 1;
}

//一次写入配置文件多个参数2
char configRewrite(const char *key, const char *val, char *filetxt) {
  char flag_line = 0;
  char flag_ok = 0;
  char fileAll[CONFIG_FILE_MAX_LENGTH] = { 0 };
  char fileLine[CONFIG_FILE_MAX_LENGTH];
  char temp;
  char *n;
  int i = 0, k = 0, j = 0;

  for (j = 0; j < CONFIG_FILE_MAX_LENGTH; j++) {
    if (flag_ok == 0) {
      temp = filetxt[j];
      if (temp == '\r') {
        flag_line = 1;  //读取完一行
        fileLine[i] = '\0';
        i = 0;
      } else if (temp == '\n') {
        if (i > 2) {
          flag_line = 2;  //读取完一行
          fileLine[i] = '\0';
        }
        i = 0;
      } else if (temp == '\0') {
        if (i > 2) {
          flag_line = 2;  //读取完
          fileLine[i] = '\0';
        }
      } else {
        fileLine[i] = temp;
        if (i < CONFIG_FILE_MAX_LENGTH - 1) {
          i++;  //读取下一位
        } else {
          return 0;
        }
      }
    } else {  //找到修改值后保存修改值后面的数据

      fileAll[k] = filetxt[j];
      k++;
    }
    //行操作
    if (flag_line) {
      if (strncmp(key, fileLine, strlen(key)) == 0) {
        n = strchr(fileLine, '=');  //关键字符第一次出现的位置
        if (n != NULL) {
          if (flag_line == 1) {  //判断是那种系统
            sprintf(n + 1, "%s\r\0", val);
          } else {
            sprintf(n + 1, "%s\r\n\0", val);
          }
          strcat(fileAll, n - strlen(key));
          k = strlen(fileAll);
          flag_ok = 1;
        }
      } else {
        strcat(fileAll, fileLine);  //找到修改值前缓存前面数据
        strcat(fileAll, "\r\n");
      }
      flag_line = 0;
    }
  }
  if (flag_ok) {
    strcpy(filetxt, fileAll);  //复制字符串输出
    // Serial.println(filetxt);
    return 1;
  } else {
    return 0;
  }
}

//一次写入配置文件多个参数3
char configWriteClose(fs::FS &fs, const char *filename, char *filetxt) {
  File file = fs.open(filename, FILE_WRITE);
  if (!file) {
    // Serial.println("Failed to open file for reading");
    return 0;
  }
  if (file.print(filetxt)) {
    file.close();
    return 1;
  } else {
    file.close();
    return 0;
  }
}

