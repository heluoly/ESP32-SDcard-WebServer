#include "video_mp4.h"

//读取txt文件
String readTxtFile(fs::FS &fs, const char *path) {
  int i = 0;
  #define maximumLength 256
  char readbuff[maximumLength];
  String message = "";

  File file = fs.open(path);
  if (!file) {
    // Serial.println("Failed to open file for reading");
    return message;
  }

  while (file.available()) {
    if (i < maximumLength - 1) {
      readbuff[i] = file.read();
      i++;
    } else {
      break;
    }
  }
  file.close();
  readbuff[i] = '\0';
  message = readbuff;
  return message;
}

//列出视频分类
void listVideoCategories_mp4(AsyncWebServerRequest *request) {
  String message = "{\"categories\":[ ";
  bool first = true;

  File root = my_fs.open("/video-mp4/");
  if (!root || !root.isDirectory()) {
    request->send(200, "application/json", "{\"categories\":[]}");
    return;
  }

  File entry = root.openNextFile();
  while (entry) {
    if (entry.isDirectory()) {
      String folderPath = String(entry.path());
      String folderName = String(entry.name());
      String titlePath = folderPath + "/0.txt";
      String displayName = folderName;

      if (my_fs.exists(titlePath)) {
        String temp = readTxtFile(my_fs, titlePath.c_str());
        temp.trim();
        if (temp.length() > 0) {
          displayName = temp;
        }
      }

      if (!first) message += ",";
      first = false;

      message += "{\"name\":\"" + folderName + "\",\"title\":\"" + displayName + "\",\"path\":\"" + folderPath + "/\"}";
    }
    entry = root.openNextFile();
  }
  message += "]}";
  request->send(200, "application/json", message);
}

//列出内存卡中的视频
void listVideo_mp4(AsyncWebServerRequest *request) {
  String videoTape = request->getParam("videoTape")->value();  //获取视频分区路径
  String page = request->getParam("page")->value();            //获取页数
  uint8_t i = 1;
  const char pageBreak = 20;  //设定分页区间，每20个视频一页
  char page0 = String2Char((char *)page.c_str());
  char page1 = (page0 - 1) * pageBreak;
  char page2 = page0 * pageBreak + 1;
  int pageTotal = 1;
  bool first = true;
  String filePath = "";
  String namePath = "";
  String picPath = "";
  String videoName = "";
  String message = "";

  File root = my_fs.open((char *)videoTape.c_str());
  if (!root) {
    // message += "Failed to open directory <br>";
    request->send(404, "text/plain", "Not found");
    return;
  } else if (!root.isDirectory()) {
    // message += "Not a directory <br>";
    request->send(404, "text/plain", "Not found");
    return;
  } else {
    message += "{\"videos\": [ ";
    File file = root.openNextFile();
    while (file) {
      if (!file.isDirectory()) {
        // 文件夹不处理
      } else if (i > page1 && i < page2) {

        filePath = String(file.path());
        namePath = filePath + "/0.txt";                            //视频标题路径
        picPath = filePath + "/0.jpg";                             //视频预览图路径
        videoName = readTxtFile(my_fs, (char *)namePath.c_str());  //读取视频标题

        if (!first) {
          message += ",";
        }
        first = false;

        message += "{ \"title\": \"";
        message += videoName;
        message += "\", \"cover\": \"";
        message += picPath;
        message += "\", \"path\": \"";
        message += filePath;
        message += "\" }";
        i++;
      } else {
        i++;
      }
      file = root.openNextFile();
    }
    // message.remove(message.length() - 1);  //删除最后的","

    pageTotal = (i + pageBreak - 2) / pageBreak;
    message += " ], \"currentPage\": ";
    message += page;
    message += " , \"totalPages\": ";
    message += pageTotal;
    message += "}";
  }
  request->send(200, "application/json", message);
}

//打开视频
void openVideo_mp4(AsyncWebServerRequest *request) {
  String videoPath = request->getParam("videoPath")->value();  //获取视频路径
  String namePath = videoPath + "/0.txt";
  String videoName = readTxtFile(my_fs, (char *)namePath.c_str());  //读取视频标题
  String subtitlePath = videoPath + "/0.vtt";
  bool flag_subtitle = 0;
  if (my_fs.exists(subtitlePath)) {
    flag_subtitle = 1;
  }

  String message = "<!DOCTYPE html><html lang=\"zh-CN\"><head><meta charset=\"UTF-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\"><title>";
  message += videoName;
  message += "</title><link href=\"/bin/videojs/8.23.4/video-js.min.css\" rel=\"stylesheet\"><style>* { margin: 0; padding: 0; box-sizing: border-box; font-family: 'Segoe UI', 'Microsoft YaHei', sans-serif; } body { background-color: #f8f9fa; color: #333; line-height: 1.6; min-height: 100vh; display: flex; flex-direction: column; align-items: center; padding: 20px; } .container { max-width: 800px; width: 100%; display: flex; flex-direction: column; gap: 20px; } .header { text-align: center; margin-bottom: 10px; } h1 { font-size: 28px; color: #333; font-weight: 600; text-shadow: 0 1px 2px rgba(0,0,0,0.1); } .video-card { background: white; border-radius: 15px; overflow: hidden; box-shadow: 0 5px 20px rgba(0, 0, 0, 0.08); border: 1px solid #f0f0f0; } .video-player-container { position: relative; width: 100%; background: #000; border-radius: 15px 15px 0 0; height: 0; padding-bottom: 56.25%; } .video-js { position: absolute; top: 0; left: 0; width: 100%; height: 100%; border-radius: 15px 15px 0 0; } .video-js .vjs-tech { object-fit: contain; } .video-js.vjs-fullscreen .vjs-tech { object-fit: contain; } .video-info { padding: 20px; text-align: center; } .video-title { font-size: 18px; color: #333; font-weight: 500; } .video-js:focus, .video-js *:focus { outline: none !important; } .video-js { -webkit-tap-highlight-color: transparent; } .video-js video:focus { outline: none !important; } .video-js:focus-visible { outline: none !important; } ";
  if (flag_subtitle) {
    message += "video::cue { color: white; background: transparent; text-shadow: 2px 2px 4px black; } .video-js .vjs-text-track-display, .video-js .vjs-text-track-display div, .video-js .vjs-text-track-display span { background: transparent !important; } ";
  }
  message += "@media (max-width: 768px) { body { padding: 15px; } h1 { font-size: 24px; } .video-info { padding: 15px; } .video-title { font-size: 16px; } } @media (max-width: 480px) { .container { gap: 15px; } h1 { font-size: 22px; } }</style></head><body><div class=\"container\"><div class=\"header\"><h1>";
  message += videoName;
  message += "</h1></div><div class=\"video-card\"><div class=\"video-player-container\"><video id=\"video_demo\" class=\"video-js vjs-default-skin vjs-big-play-centered\" controls preload=\"auto\" poster=\"";
  message += videoPath;
  message += "/0.jpg\" data-setup=\"{}\"><source src=\"";
  message += videoPath;
  message += "/video.mp4\" type=\"video/mp4\">"; 
  if (flag_subtitle) {
    message += "<track kind=\"subtitles\" label=\"中文\" srclang=\"zh\" src=\"" + subtitlePath + "\" default>";
  }
  message += "</video></div></div></div><script src=\"/bin/videojs/8.23.4/video.min.js\"></script>";
  message += "<script>(function(){if(!window.videojs){console.warn('Video.js 库未加载完成，稍后重试');return}const videoElement=document.getElementById('video_demo');if(!videoElement)return;let videoSource='';const sourceNode=document.querySelector('#video_demo source');videoSource=sourceNode.src;const STORAGE_KEY='videojs_playback_pos_'+videoSource;let player=null;try{player=videojs('video_demo')}catch(e){console.error('获取播放器实例失败:',e);return}function saveCurrentPlaybackPosition(){try{if(!player||typeof player.currentTime!=='function')return;const isEnded=player.ended&&player.ended();const currentTime=player.currentTime();if(typeof currentTime==='number'&&!isNaN(currentTime)&&currentTime>3&&!isEnded){const duration=player.duration();if(duration&&(duration-currentTime)<3&&isEnded){localStorage.removeItem(STORAGE_KEY);return}localStorage.setItem(STORAGE_KEY,currentTime)}else if(currentTime<=3&&!isEnded){}else if(isEnded){localStorage.removeItem(STORAGE_KEY)}}catch(err){console.warn('保存播放进度失败:',err)}}function restorePlaybackPosition(){try{const savedTimeRaw=localStorage.getItem(STORAGE_KEY);if(!savedTimeRaw)return false;const savedTime=parseFloat(savedTimeRaw);if(isNaN(savedTime)||savedTime<=0){localStorage.removeItem(STORAGE_KEY);return false}const setCurrentTimeSafe=function(){if(!player||typeof player.currentTime!=='function')return;const duration=player.duration();let targetTime=savedTime;if(duration&&!isNaN(duration)&&duration>0&&savedTime>=duration){localStorage.removeItem(STORAGE_KEY);return}if(targetTime>3){player.currentTime(targetTime)}};const checkMetadataLoaded=()=>{if(!player||!player.tech_||!player.tech_.el_)return false;const videoEl=player.tech_.el_||player.el().querySelector('video');if(videoEl&&videoEl.readyState>=1){return true}return false};if(checkMetadataLoaded()){setCurrentTimeSafe()}else{player.one('loadedmetadata',function(){setCurrentTimeSafe()});setTimeout(()=>{if(player&&player.currentTime&&player.currentTime()===0&&savedTime>1){const duration=player.duration();if(duration&&duration>0){const target=Math.min(savedTime,duration-3);if(target>3){player.currentTime(target)}}}},500)}return true}catch(err){console.warn('恢复播放位置失败:',err);return false}}function handleVideoEnded(){try{localStorage.removeItem(STORAGE_KEY)}catch(e){}}let isBeforeUnloadRegistered=false;function registerBeforeUnload(){if(isBeforeUnloadRegistered)return;window.addEventListener('beforeunload',function(){saveCurrentPlaybackPosition()});isBeforeUnloadRegistered=true}function registerPageHideFallback(){window.addEventListener('pagehide',function(){saveCurrentPlaybackPosition()});document.addEventListener('visibilitychange',function(){if(document.visibilityState==='hidden'){saveCurrentPlaybackPosition()}})}player.ready(function(){restorePlaybackPosition();player.on('ended',handleVideoEnded);player.on('pause',function(){if(!player.ended&&player.currentTime()>3){saveCurrentPlaybackPosition()}});player.on('seeked',function(){const cur=player.currentTime();if(!player.ended&&cur>3){saveCurrentPlaybackPosition()}});registerBeforeUnload();registerPageHideFallback()});setTimeout(function(){if(player&&!player.isReady_){registerBeforeUnload();if(!localStorage.getItem(STORAGE_KEY))return;const saved=localStorage.getItem(STORAGE_KEY);if(saved&&parseFloat(saved)>3){player.one('loadedmetadata',function(){if(player.currentTime()<1&&parseFloat(saved)<player.duration()){player.currentTime(parseFloat(saved))}})}player.on('ended',handleVideoEnded);player.on('pause',function(){if(!player.ended&&player.currentTime()>3)saveCurrentPlaybackPosition()})}},500)})();</script>";
  message += "</body></html>";

  request->send(200, "text/html", message);  //发送网页
}
