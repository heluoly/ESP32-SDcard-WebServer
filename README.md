# ESP32-SDcard-WebServer
项目名称：ESP32便携SD(TF)卡服务器

作者：B站 狼尾巴的猫

项目实现文件上传下载、网页flash游戏、播放视频、模式转换（AP STA AP+STA互相转换）、剪切板功能

![image](/image/image1.jpg)

![image](/image/image2.jpg)

# 演示视频：

1、  https://www.bilibili.com/video/BV1pu4m1A7bo/

2、  https://www.bilibili.com/video/BV1r34117746

3、  https://www.bilibili.com/video/BV1SG411x7tz

PCB工程：https://oshwhub.com/heluoly/esp32-fu-wu-qi_copy_copy_copy_copy


# 功能

1、文件上传下载

可将终端设备的文件上传到服务器的内存卡中，也可将服务器中的文件取回。

2、网页视频

可用手机或电脑上的浏览器播放服务器内存卡中存放的视频，视频播放器使用videoJS

3、剪切板

一个临时的文本中转站，可将文本临时保存在服务器上。

4、网页flash游戏

可使用电脑浏览器游玩服务器中的flash游戏，flash播放器使用objecty。

5、WiFi配网（模式转换）

服务器默认为AP模式，可通过网页让服务器连接WiFi接入局域网，然后通过局域网访问服务器。

6、AP修改设置

可通过网页修改服务器的WiFi名称、密码、信道。

7、OLED屏显示服务器状态及时间。

8、服务器低功耗模式。


# 开始前准备

1、使用arduino将程序编译烧录进入ESP32，arduino-esp32库版本为2.0.14+或3.0.0(需选择对应版本程序)，注意，烧录时需要将内存卡从卡槽中取出，同时将跳线帽取下。

2、准备一张32G以下大小的内存卡，将“内存卡”文件夹中的所有文件复制进入内存卡根目录。

3、将内存卡插入内存卡插槽，将跳线帽插上。

4、拨动开关开机。


# 内存卡中网页视频与flash游戏的配置，以及服务器配置

参考视频：https://www.bilibili.com/video/BV1r34117746/

1、flash游戏配置

请将swf文件复制到内存卡中的/webgame/oldGame内。

2、网页视频配置

请先新建一个文件夹（文件夹名称不能带有中文），使用mp4转m3u8软件将视频转换为m3u8的格式并存放在刚刚新建的文件夹中，m3u8文件命名为index.m3u8，将切片好的视频文件连同文件夹一同复制到/video/shortvideo或/video/movie内。

视频的标题存放在与视频相同路径下的 0.txt文件内，编码格式为utf-8。

视频的预览图存放在与视频相同路径下的 0.jpg文件内。

3、服务器配置

服务器配置存放位置通过common.h中的CONFIG_SD设置，0为SPIFFS，1为SD_MMC，默认存放在SPIFFS中，文件名为config.txt，其中存放WiFi名称密码等信息。

# 使用说明

1、Boot键功能

单击：服务器状态/时钟表盘切换显示

长按：关闭/开启WIFI

如果单击boot按键屏幕没有任何反应，可能由于内存卡未初始化成功，请检查内存卡的连接。

2、如何连接服务器

服务器上电默认处于AP模式，使用手机或者电脑，找到ESP32_webserver这个WIFI进行连接，密码是123456789，连接成功后，用浏览器访问192.168.1.1即可进入服务器主页。


# 参考项目

SD卡代码参考 https://youtu.be/e1xOgZsnAuw

网页响应代码参考 http://www.taichi-maker.com/homepage/esp8266-nodemcu-iot/iot-c/spiffs/spiffs-web-server/file-upload-server/

文件上传代码参考 https://github.com/smford/esp32-asyncwebserver-fileupload-example

flash播放器使用objecty

视频播放器使用videoJS

网页配网代码参考 https://github.com/yuan910715/Esp8266_NTP_Clock_Weather 中的网页配网部分

OLED屏幕时钟参考 https://github.com/ThingPulse/esp8266-oled-ssd1306 中的 examples/SSD1306ClockDemo


# 额外说明

如需旧版代码，请到release中下载

2025.11.14新增异步库版本，在ESP32_SDwebserver_async文件夹下，该版本使用ESP32-S3运行能大幅提高视频播放的流畅性，普通的ESP32跑起来反而很卡；使用该版本需要额外安装如下依赖库

https://github.com/ESP32Async/ESPAsyncWebServer

https://github.com/ESP32Async/AsyncTCP

