#include"upload.h"

extern WebServer esp32_server;
extern File fsUploadFile;

//列出上传的文件
String listUploadDir(fs::FS &fs, const char * dirname, uint8_t levels)
{
  String filename = "";
  String filename2 = "";
  
  String message="";
  File root = fs.open(dirname);
  if(!root){
      message += "Failed to open directory <br />";
      return message;
  }
  if(!root.isDirectory()){
      message += "Not a directory <br />";
      return message;
  }
  File file = root.openNextFile();
  while(file){
      if(file.isDirectory()){
          message +="  DIR : ";
          message += String(file.path())+String("<br />");
          if(levels){
              message += listUploadDir(fs, file.path(), levels -1);
          }
      } else {
          filename = String(file.path());
          //filename2 = indexOfFilename(filename);
          filename2 = String(file.name());
          
          message += "<form onSubmit=\"if(!confirm('确认删除')){return false;}\" action=\"/deleteUploadFile\">  FILE: " + filename2 + "<br/>  SIZE: " + formatBytes(file.size()) + "  <a href=\"" + filename + "\" download=\"" + filename2 + "\">download</a>  ";
          message += "<input type=\"hidden\" name=\"deletePath\" value=\"" + filename + "\"><input type=\"submit\" value=\"delete\"><br/></form>";

      }
      file = root.openNextFile();
  }
  return message;
}

void listUploadFile() {
  String header = "<html><meta charset=\"UTF-8\"><body>";
  String message= header + "<h2>upload:</h2>";
  message += listUploadDir(SD,"/upload",1);
  message += "<p><a href=\"/index.html\">返回首页</a></p>";
  esp32_server.send(200,"text/html",message); 
  
} 


void handleFileUpload(){  
  
  HTTPUpload& upload = esp32_server.upload();
  
  if(upload.status == UPLOAD_FILE_START){                     // 如果上传状态为UPLOAD_FILE_START

    if (SD.exists((char *)upload.filename.c_str())) {
      SD.remove((char *)upload.filename.c_str());
    }
    
    String filename = upload.filename;                        // 建立字符串变量用于存放上传文件名
    if(!filename.startsWith("/")) filename = "/upload/" + filename;  // 为上传文件名前加上"/"
    Serial.println("File Name: " + filename);                 // 通过串口监视器输出上传文件的名称

    fsUploadFile = SD.open(filename, FILE_WRITE);            // 在SD卡中建立文件用于写入用户上传的文件数据
    
  }
  else if(upload.status == UPLOAD_FILE_WRITE){          // 如果上传状态为UPLOAD_FILE_WRITE      
    
    if(fsUploadFile)
      fsUploadFile.write(upload.buf, upload.currentSize); // 向SD卡文件写入浏览器发来的文件数据
      
  }
  else if(upload.status == UPLOAD_FILE_END){            // 如果上传状态为UPLOAD_FILE_END 
    if(fsUploadFile) {                                    // 如果文件成功建立
      fsUploadFile.close();                               // 将文件关闭
      vTaskDelay(100/portTICK_PERIOD_MS);
      esp32_server.sendHeader("Location","/success.html");  // 将浏览器跳转到/success.html（成功上传页面）
      esp32_server.send(303);                               // 发送相应代码303（重定向到新页面） 
    }
    else {                                              // 如果文件未能成功建立
   //   Serial.println("File upload failed");               // 通过串口监视器输出报错信息
      esp32_server.send(500, "text/plain", "500: couldn't create file"); // 向浏览器发送相应代码500（服务器错误）
    }    
  }
}

void deleteUploadFile(){
    String deletePath = esp32_server.arg("deletePath");
    char flag=0;
    flag=deleteFile(SD, (char*)deletePath.c_str());
    if(flag){
        listUploadFile();       
    } else {
        Serial.println("Delete failed");
    }
}



  
