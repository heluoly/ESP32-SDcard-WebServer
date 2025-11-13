#include "common.h"

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

//读取配置文件 configRead(SD_MMC,参数名称,文件路径,参数值)
char configRead(fs::FS &fs, const char *key, const char *filename, char *buf) {
  char flag_line = 0;
  char sLine[configMaximumLength];
  char temp;
  char *n;
  int i = 0;

  File file = fs.open(filename);
  if (!file) {
    // Serial.println("Failed to open file for reading");
    return 0;
  }
  while (file.available()) {
    temp = file.read();
    if (temp == '\r') {
      flag_line = 1;  //读取完一行
      sLine[i] = '\0';
      i = 0;
    } else if (temp == '\n') {
      if (i > 2) {
        flag_line = 1;  //读取完一行
        sLine[i] = '\0';
      }
      i = 0;
    } else if (temp == '\0') {
      if (i > 2) {
        flag_line = 1;  //读取完
        sLine[i] = '\0';
      }
    } else {
      sLine[i] = temp;
      if (i < configMaximumLength - 1) {
        i++;  //读取下一位
      } else {
        return 0;
      }
    }
    //行操作
    if (flag_line) {
      flag_line = 0;
      if (strncmp(key, sLine, strlen(key)) == 0)  //如果关键词是目标值
      {
        n = strchr(sLine, '=');  //=第一次出现的位置
        if (n != NULL) {
          strcpy(buf, n + 1);  //将值复制到buf输出
          file.close();
          return 1;
        }
      }
    }
  }
  file.close();
  return 0;
}

//修改配置文件 configWrite(SD_MMC,参数名称,修改的参数值,文件路径)
char configWrite(fs::FS &fs, const char *key, const char *val, const char *filename) {
  char flag_line = 0;
  char flag_ok = 0;
  char fileAll[configMaximumLength] = { 0 };
  char fileLine[configMaximumLength];
  char temp;
  char *n;
  int i = 0, k = 0;

  File file = fs.open(filename);
  if (!file) {
    // Serial.println("Failed to open file for reading");
    return 0;
  }
  while (file.available()) {
    if (flag_ok == 0) {
      temp = file.read();
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
        if (i < configMaximumLength - 1) {
          i++;  //读取下一位
        } else {
          return 0;
        }
      }
    } else {
      if (k < configMaximumLength - 1) {  //找到修改值后保存修改值后面的数据
        fileAll[k] = file.read();
        k++;
      }
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
  file.close();
  if (flag_ok == 1) {
    fileAll[k] = '\0';
    file = fs.open(filename, FILE_WRITE);
    file.print(fileAll);
    file.close();
    return 1;
  } else {
    file.close();
    return 0;
  }
}

// char filetxt[configMaximumLength] = { 0 };
//一次写入配置文件多个参数1
char configWriteOpen(fs::FS &fs, const char *filename, char *filetxt) {
  uint16_t i = 0;

  File file = fs.open(filename);
  if (!file) {
    // Serial.println("Failed to open file for reading");
    return 0;
  }
  while (file.available()) {
    if (i < configMaximumLength - 1) {
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
  char fileAll[configMaximumLength] = { 0 };
  char fileLine[configMaximumLength];
  char temp;
  char *n;
  int i = 0, k = 0, j = 0;

  for (j = 0; j < configMaximumLength; j++) {
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
        if (i < configMaximumLength - 1) {
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
