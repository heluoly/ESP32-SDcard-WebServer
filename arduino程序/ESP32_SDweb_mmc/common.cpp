#include "common.h"

/*
//从文件路径中获取文件名
String indexOfFilename(String filename)
{
  char filename2[200];
  uint8_t len = 0;
  uint8_t cout = 0;
  uint8_t i,j = 0;
  len = filename.length();
  for(i=0;i<len;i++)
  {
      if(filename[i]=='/')
      {
         cout = i;
      }   
  }
  for(i=cout+1;i<len;i++)
  {   
      if(j<195)
      {
          filename2[j] = filename[i];
          j++;
      }
  }       
  j=0;
  filename = filename2;
  return filename;
}
*/
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


void listDir(fs::FS &fs, const char * dirname, uint8_t levels){
  // Serial.printf("Listing directory: %s\n", dirname);

  File root = fs.open(dirname);
  if(!root){
      // Serial.println("Failed to open directory");
      return;
  }
  if(!root.isDirectory()){
      // Serial.println("Not a directory");
      return;
  }

  File file = root.openNextFile();
  while(file){
      if(file.isDirectory()){
          // Serial.print("  DIR : ");
          // Serial.println(file.name());
          if(levels){
              listDir(fs, file.name(), levels -1);
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

char deleteFile(fs::FS &fs, const char * path){
    // Serial.printf("Deleting file: %s\n", path);
    if(fs.remove(path)){
        return 1;
    } else {
        return 0;
    }
}

char writeFile(fs::FS &fs, const char * path, const char * message){
    File file = fs.open(path, FILE_WRITE);
    if(!file){
        return 0;
    }
    if(file.print(message)){
        return 1;
    } else {
        return 0;
    }
    file.close();
}

//字符串转数字 
char String2Char(char *str)
{
    char flag = '+';//指示结果是否带符号 
    long res = 0;
    
    if(*str=='-')//字符串带负号 
    {
        ++str;//指向下一个字符 
        flag = '-';//将标志设为负号 
    } 
    //逐个字符转换，并累加到结果res 
    while(*str>=48 && *str<=57)//如果是数字才进行转换，数字0~9的ASCII码：48~57 
    {
        res = 10*res+  *str++-48;//字符'0'的ASCII码为48,48-48=0刚好转化为数字0 
    } 
 
    if(flag == '-')//处理是负数的情况
    {
        res = -res;
    }
 
    return (char)res;
}
