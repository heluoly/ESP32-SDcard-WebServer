请将该文件夹内所有文件复制到内存卡根目录下。
注意：
一、内存卡中的文件名和文件夹名称不能带有中文。

二、从网页上传的文件将存放在/upload。

三、flash游戏请存放在/webgame/oldGame内。

四、服务器视频配置说明
1、m3u8格式视频配置说明
请先新建一个文件夹（文件夹名称不能带有中文），使用mp4转m3u8软件将视频转换为m3u8的格式并存放在刚刚新建的文件夹中，m3u8文件命名为index.m3u8，将切片好的视频文件连同文件夹一同复制到/video/shortvideo或/video/movie内。
视频的标题存放在与视频相同路径下的 0.txt文件内，编码格式为utf-8。
视频的预览图存放在与视频相同路径下的 0.jpg文件内。

2、mp4格式视频配置说明
请先新建一个文件夹（文件夹名称不能带有中文），在文件夹内复制准备好的mp4文件，并重命名为video.mp4，注意：需要确保moov原子移在视频文件前面，推荐使用工具mp4-optimizer：https://github.com/billytoe/mp4-optimizer 
视频的标题存放在与视频相同路径下的 0.txt文件内，编码格式为utf-8。
视频的预览图存放在与视频相同路径下的 0.jpg文件内。

五、在根目录下，copy.txt用于存放剪切板内容，编码格式为utf-8。

六、在根目录下，config.txt用于保存AP模式下热点名称、热点密码、热点WiFi信道以及STA模式下WiFi自动连接信息，配置文件最后必须以回车结尾，文件编码格式为utf-8。

七、文件夹结构

```
内存卡_async
├─ bin
│  ├─ font-awesome
│  │  └─ ...
│  └─ videojs
│     ├─ 8.23.4
│     │  └─ ...
│     └─ videojs-contrib-hls
│        └─ ...
├─ clipboard.html         //剪切板网页
├─ config.txt             //存放服务器配置文件
├─ copy.txt               //存放剪切板内容
├─ favicon.ico            //网页图标
├─ index.html             //主页
├─ index_video.html       //m3u8格式视频主页
├─ index_video_mp4.html   //mp4格式视频主页
├─ logo.jpg               //网页图标
├─ README.txt
├─ setTime.html           //设置服务器时间
├─ upload                 //存放上传服务器的文件
│  └─ ...
├─ upload.html            //文件管理网页
├─ video                  //存放m3u8格式视频
│  ├─ movie
│  │  ├─ 1                //每个视频单独一个文件夹，文件夹名称不能带中文及特殊字符
│  │  │  ├─ 0.jpg         //视频预览图
│  │  │  ├─ 0.txt         //视频标题
│  │  │  ├─ 000.ts        //视频序列
│  │  │  ├─ 001.ts
│  │  │  ├─ 002.ts
│  │  │  ├─ ...
│  │  │  └─ index.m3u8    //m3u8视频索引
│  │  ├─ 2
│  │  │  ├─ 0.jpg
│  │  │  ├─ 0.txt
│  │  │  ├─ 000.ts
│  │  │  ├─ 001.ts
│  │  │  ├─ 002.ts
│  │  │  ├─ ...
│  │  │  └─ index.m3u8
│  │  └─ 3
│  ├─ movie.html          //movie类视频索引
│  ├─ shortvideo
│  │  ├─ 1
│  │  ├─ 2
│  │  └─ ...
│  └─ shortvideo.html
├─ video-mp4              //存放mp4格式视频
│  ├─ movie
│  │  ├─ 1                //每个视频单独一个文件夹，文件夹名称不能带中文及特殊字符
│  │  │  ├─ 0.jpg         //视频预览图
│  │  │  ├─ 0.txt         //视频标题
│  │  │  └─ video.mp4     //视频文件，需要把moov原子移到文件前面
│  │  ├─ 2
│  │  │  ├─ 0.jpg
│  │  │  ├─ 0.txt
│  │  │  └─ video.mp4
│  │  └─ ...
│  ├─ movie.html          //movie类视频索引
│  ├─ shortvideo
│  │  ├─ 1
│  │  ├─ 2
│  │  └─ ...
│  └─ shortvideo.html
└─ webgame                //存放flash游戏
   ├─ objecty             //flash播放器
   │  └─ ...
   ├─ oldGame             //存放swf游戏
   │  └─ ...swf
   ├─ oldGame.html        //oldGame类flash游戏索引
   └─ webgame.html        //网页游戏索引
   
```