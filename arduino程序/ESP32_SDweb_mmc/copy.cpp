#include "copy.h"

extern WebServer esp32_server;
extern String htmlHeader;

//读取txt文件
String readFile2(fs::FS &fs, const char * path){
  int i=0;
  char readbuff[2048];
  String message="";

  File file = fs.open(path);
  if(!file){
      // Serial.println("Failed to open file for reading");
      return message;
  }

  while(file.available()){
      if(i<2040)
      {
          readbuff[i] = file.read();
          i++;
      }
      else{
        break;
      }
  }
  file.close();
  readbuff[i]='\0';
  message = readbuff;
  return message;
}

//覆盖写入txt文件
void editTxt(){
  String txtPath = esp32_server.arg("txtpath");   //提取文件路径
  String txtContent = esp32_server.arg("con");    //提取要保存的内容
  char flag=0;
  flag=writeFile(SD_MMC, (char*)txtPath.c_str(), (char*)txtContent.c_str());
  if(flag){
    esp32_server.send(200,"text/html","保存成功");
  } else {
    esp32_server.send(200,"text/html","保存失败");
  }  
}

//剪切板HTML代码
void clipBoard(){
  String content="";
  content = readFile2(SD_MMC,"/copy.txt");  //读取copy.txt的内容
  String message = htmlHeader + "<title>剪切板</title></head><body><h2>剪切板</h2><br><label style=\"display: block\"><textarea id=\"textArea\" rows=\"8\" style=\"width:100%;\">";
  message += content;
  message += "</textarea></label><br><button type=\"button\" onclick=\"theCopy()\">复制</button> <button type=\"button\" onclick=\"theSave()\">保存</button></body><script>function theCopy() {var textArea = document.getElementById('textArea');console.log(textArea);textArea.select();document.execCommand('copy');alert('复制成功');}</script><script>function theSave() {var text=document.getElementById('textArea').value;var con=encodeURI(text).replace(/#/g, '%23');var xmlhttp=new XMLHttpRequest();xmlhttp.open(\"GET\",\"/edittxt?txtpath=/copy.txt&con=\"+con,true);xmlhttp.send();xmlhttp.onload = function(e){alert(this.responseText);}}</script></html>";
  
  esp32_server.send(200,"text/html",message);
}




