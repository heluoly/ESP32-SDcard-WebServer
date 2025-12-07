#include "game.h"

//列出内存卡的游戏
void listGame(AsyncWebServerRequest *request) {
  String folder = request->getParam("folder")->value();  //获取游戏文件夹路径
  String page = request->getParam("page")->value();      //获取页数
  String filePath = "";
  String fileName = "";
  uint8_t i = 1;
  const char pageBreak = 20;  //设定分页区间，每20个一页
  char page0 = String2Char((char *)page.c_str());
  char page1 = (page0 - 1) * 20;  //设定分页区间，每20个一页
  char page2 = page0 * 20 + 1;
  int pageTotal = 1;
  String message = "";

  File root = my_fs.open((char *)folder.c_str());
  if (!root) {
    request->send(404, "text/plain", "Not found");
  } else if (!root.isDirectory()) {
    request->send(404, "text/plain", "Not found");
  } else {
    message += "{\"games\": [ ";
    File file = root.openNextFile();
    while (file) {
      if (file.isDirectory()) {
        // 文件夹不处理
      } else if (i > page1 && i < page2) {
        filePath = String(file.path());
        fileName = String(file.name());

        message += "{ \"fileName\": \"";
        message += fileName;
        message += "\", \"size\": \"";
        message += formatBytes(file.size());
        message += "\", \"path\": \"";
        message += filePath;
        message += "\" },";
        i++;
      } else {
        i++;
        // 非分页范围忽略，最后统计总文件数量
      }
      file = root.openNextFile();
    }
    message.remove(message.length() - 1);  //删除最后的","

    pageTotal = (i + pageBreak - 2) / pageBreak;
    message += " ], \"currentPage\": ";
    message += page;
    message += " , \"totalPages\": ";
    message += pageTotal;
    message += "}";
  }
  request->send(200, "application/json", message);
}

//打开游戏
void openGame(AsyncWebServerRequest *request) {
  String gamePath = request->getParam("gamePath")->value();  //获取游戏路径
  String message = "<!DOCTYPE html><html lang=\"zh-CN\"><head><meta charset=\"UTF-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\"><title>Flash游戏</title><style>* { margin: 0; padding: 0; box-sizing: border-box; } body { background: #000; color: #fff; font-family: Arial, sans-serif; overflow: hidden; height: 100vh; } #fullscreenControls { position: fixed; top: 10px; right: 10px; z-index: 1000; background: rgba(0, 0, 0, 0.7); padding: 8px; border-radius: 4px; border: 1px solid #444; } #fullscreenBtn { background: #333; color: white; border: 1px solid #666; padding: 8px 16px; cursor: pointer; border-radius: 3px; font-size: 14px; min-width: 100px; } #fullscreenBtn:hover { background: #444; } #fullscreenHint { position: fixed; bottom: 20px; left: 0; right: 0; text-align: center; color: #888; font-size: 12px; z-index: 1000; display: none; } #gameContainer { position: absolute; top: 50%; left: 50%; transform: translate(-50%, -50%); width: 800px; height: 600px; background: #000; transition: all 0.3s; overflow: hidden; } .fullscreen-mode #gameContainer { width: 100vw; height: 100vh; transform: none; top: 0; left: 0; } #aspectRatioContainer { position: relative; width: 100%; height: 100%; } #flashWrapper { position: absolute; top: 0; left: 0; width: 100%; height: 100%; } .fullscreen-mode #flashWrapper { width: 100%; height: 100%; } #blackBars { position: absolute; top: 0; left: 0; width: 100%; height: 100%; z-index: -1; background: #000; } :-webkit-full-screen #gameContainer { width: 100vw !important; height: 100vh !important; transform: none !important; top: 0 !important; left: 0 !important; } :-moz-full-screen #gameContainer { width: 100vw !important; height: 100vh !important; transform: none !important; top: 0 !important; left: 0 !important; } :-ms-fullscreen #gameContainer { width: 100vw !important; height: 100vh !important; transform: none !important; top: 0 !important; left: 0 !important; }</style></head><body><div id=\"fullscreenControls\"><button id=\"fullscreenBtn\" onclick=\"toggleFullScreen()\">进入全屏</button></div><div id=\"fullscreenHint\">点击退出全屏按钮或按ESC键退出全屏模式</div><div id=\"gameContainer\"><div id=\"aspectRatioContainer\"><div id=\"flashWrapper\"><object class=\"ObjectyMe\" data=\"/download?filePath=";
  message += gamePath;
  message += "\" type=\"application/x-shockwave-flash\" width=\"100%\" height=\"100%\" id=\"flashObject\"><param name=\"movie\" value=\"/download?filePath=";
  message += gamePath;
  message += "\"><param name=\"allowFullScreen\" value=\"true\"><param name=\"quality\" value=\"high\"><param name=\"scale\" value=\"exactfit\"><param name=\"wmode\" value=\"opaque\"><param name=\"allowScriptAccess\" value=\"always\"><param name=\"salign\" value=\"lt\"><param name=\"menu\" value=\"false\"><param name=\"allowFullScreenInteractive\" value=\"true\"><param name=\"allowNetworking\" value=\"all\">您的浏览器不支持Flash，请使用支持Flash的浏览器访问。</object></div><div id=\"blackBars\"></div></div></div><script type=\"text/javascript\" src=\"/webgame/objecty/objecty.js\"></script><script>var isFullscreen = false; var fullscreenBtn = document.getElementById('fullscreenBtn'); var flashObject = document.getElementById('flashObject'); var gameContainer = document.getElementById('gameContainer'); var fullscreenHint = document.getElementById('fullscreenHint'); var originalWidth = 800; var originalHeight = 600; var aspectRatio = originalWidth / originalHeight; var fullscreenEnabled = document.fullscreenEnabled || document.webkitFullscreenEnabled || document.mozFullScreenEnabled || document.msFullscreenEnabled; function toggleFullScreen() { if (!isFullscreen) { enterFullscreen(); } else { exitFullscreen(); } } function enterFullscreen() { var element = document.documentElement; document.body.classList.add('fullscreen-mode'); if (element.requestFullscreen) { element.requestFullscreen(); } else if (element.webkitRequestFullscreen) { element.webkitRequestFullscreen(); } else if (element.mozRequestFullScreen) { element.mozRequestFullScreen(); } else if (element.msRequestFullscreen) { element.msRequestFullscreen(); } else { fallbackFullscreen(); return; } isFullscreen = true; updateUI(); adjustFlashForFullscreen(); } function exitFullscreen() { if (document.exitFullscreen) { document.exitFullscreen(); } else if (document.webkitExitFullscreen) { document.webkitExitFullscreen(); } else if (document.mozCancelFullScreen) { document.mozCancelFullScreen(); } else if (document.msExitFullscreen) { document.msExitFullscreen(); } else { exitFallbackFullscreen(); } } function fallbackFullscreen() { isFullscreen = true; document.body.classList.add('fullscreen-mode'); document.body.style.overflow = 'hidden'; adjustFlashForFullscreen(); updateUI(); fullscreenHint.style.display = 'block'; } function exitFallbackFullscreen() { isFullscreen = false; document.body.classList.remove('fullscreen-mode'); document.body.style.overflow = ''; restoreFlashSize(); updateUI(); fullscreenHint.style.display = 'none'; } function adjustFlashForFullscreen() { if (!isFullscreen) return; var screenWidth = window.innerWidth; var screenHeight = window.innerHeight; var containerWidth, containerHeight; var screenRatio = screenWidth / screenHeight; if (screenRatio > aspectRatio) { containerHeight = screenHeight; containerWidth = containerHeight * aspectRatio; } else { containerWidth = screenWidth; containerHeight = containerWidth / aspectRatio; } containerWidth = Math.floor(containerWidth); containerHeight = Math.floor(containerHeight); gameContainer.style.width = containerWidth + 'px'; gameContainer.style.height = containerHeight + 'px'; var leftPos = (screenWidth - containerWidth) / 2; var topPos = (screenHeight - containerHeight) / 2; gameContainer.style.left = leftPos + 'px'; gameContainer.style.top = topPos + 'px'; } function restoreFlashSize() { gameContainer.style.width = originalWidth + 'px'; gameContainer.style.height = originalHeight + 'px'; gameContainer.style.left = '50%'; gameContainer.style.top = '50%'; gameContainer.style.transform = 'translate(-50%, -50%)'; } function updateUI() { if (isFullscreen) { fullscreenBtn.textContent = '退出全屏'; fullscreenHint.style.display = 'block'; } else { fullscreenBtn.textContent = '进入全屏'; fullscreenHint.style.display = 'none'; } } document.addEventListener('fullscreenchange', handleFullscreenChange); document.addEventListener('webkitfullscreenchange', handleFullscreenChange); document.addEventListener('mozfullscreenchange', handleFullscreenChange); document.addEventListener('MSFullscreenChange', handleFullscreenChange); function handleFullscreenChange() { var isCurrentlyFullscreen = document.fullscreenElement || document.webkitFullscreenElement || document.mozFullScreenElement || document.msFullscreenElement; if (isCurrentlyFullscreen) { isFullscreen = true; document.body.classList.add('fullscreen-mode'); adjustFlashForFullscreen(); } else { isFullscreen = false; document.body.classList.remove('fullscreen-mode'); restoreFlashSize(); } updateUI(); } document.addEventListener('keydown', function(e) { if (e.keyCode === 27) { if (isFullscreen) { e.preventDefault(); exitFullscreen(); } } if (e.keyCode === 122) { e.preventDefault(); toggleFullScreen(); } }); window.addEventListener('resize', function() { if (isFullscreen) { adjustFlashForFullscreen(); } }); document.addEventListener('click', function(e) { if (isFullscreen && !gameContainer.contains(e.target) && e.target !== fullscreenBtn && !fullscreenBtn.contains(e.target)) { exitFullscreen(); } }); window.addEventListener('load', function() { var isOldBrowser = !window.addEventListener || (navigator.userAgent.indexOf('MSIE') !== -1) || (navigator.userAgent.indexOf('Trident') !== -1); if (isOldBrowser) { console.log('检测到老浏览器，使用兼容模式'); fullscreenHint.textContent = '点击退出全屏按钮退出全屏模式'; } var hasFlash = false; try { hasFlash = Boolean(new ActiveXObject('ShockwaveFlash.ShockwaveFlash')); } catch (e) { hasFlash = !!(navigator.mimeTypes && navigator.mimeTypes['application/x-shockwave-flash'] && navigator.mimeTypes['application/x-shockwave-flash'].enabledPlugin); } if (!hasFlash) { alert('您的浏览器未安装或未启用Flash插件，游戏可能无法正常运行。'); } });</script></body></html>";

  request->send(200, "text/html", message);
}
