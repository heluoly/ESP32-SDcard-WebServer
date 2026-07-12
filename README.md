# ESP32-SDcard-WebServer

一个小型私人文件服务器，利用ESP32的WiFi热点或接入局域网，轻松访问内存卡中的文件或在线播放视频。项目由ESP32 Arduino core框架编写，实现文件上传下载、播放视频、模式转换等功能。

- **文件上传下载** -- 可将终端设备的文件上传到服务器的内存卡中，也可将服务器中的文件取回，支持多线程下载、断点续传
- **网页视频播放** -- 可用浏览器播放服务器内存卡中存放的视频，视频播放器使用videoJS，支持自由拖拽进度、外挂字幕
- **剪切板** -- 一个临时的文本中转站，可将文本临时保存在服务器上
- **模式转换（WiFi配网）** -- 服务器可在AP、STA、AP+STA模式互相转换，默认为AP模式，可通过网页让服务器连接WiFi接入局域网，然后通过局域网访问服务器
- **OLED屏显示** -- 显示服务器状态、IP地址、时钟等

![image](/assets/ESP32-S3服务器硬件.jpg "ESP32-S3服务器硬件")

![image](/assets/ESP32-S3服务器主页.jpg "ESP32-S3服务器主页")


# 重要提醒

- 本项目适用于ESP32-S3系列或更高性能模组，普通的ESP32模组会因为异步读取内存卡数据而卡死


# 准备工作01 - 硬件准备

准备一块ESP32-S3开发板、一个SDIO接口的SD或TF卡卡座、一块I2C接口的OLED显示屏，按照下图接线

![image](/assets/ESP32-S3接线图.jpg "ESP32-S3接线图")

或者使用项目的PCB电路板打样制作


# 准备工作02 - 固件烧录

1. 在arduino中安装如下依赖库：
**ESPAsyncWebServer** https://github.com/ESP32Async/ESPAsyncWebServer
**AsyncTCP** https://github.com/ESP32Async/AsyncTCP

2. 首次编译烧录，在arduino的"工具"下拉菜单中的"Erase All Flash Before Sketch Upload"选择"Enabled"

3. 在arduino的"工具"下拉菜单中的"Partition Scheme"选择包含"FATFS"格式的分区表

4. 根据你的模组选择是否开启PSRAM功能，默认为禁用，开启需要在arduino的"工具"下拉菜单中的"PSRAM"配置，2MB PSRAM选择"QSPI PSRAM" ,8MB PSRAM选择"OPI PSRAM"，并在common.h中将"CONFIG_PSRAM"参数设置为"1"

5. SD卡引脚定义在ESP32_SDwebserver_async.ino中设置，默认clk: 11，cmd: 12，d0: 10，d1: 9，d2: 14，d3: 13

6. 电池电压检测引脚在battery.h中设置，默认使能引脚BAT_EN_PIN: 1，检测引脚BAT_ADC_PIN: 2，在battery.cpp中的readBatteryVoltage()函数内设置分压系数

7. OLED显示屏的I2C引脚在oled.cpp中修改，默认SDA: 15，SCL: 16

完成上述操作即可编译并烧录


# 准备工作03 - 内存卡准备

1. 准备一张32G以下的内存卡，格式化为FAT32格式

2. 将项目中"内存卡"文件夹中的所有文件复制到内存卡根目录

3. 准备需要在线播放的视频MP4文件，视频码率推荐在1000kbps左右

4. 将视频文件moov原子移动到视频文件前面，推荐使用工具mp4-optimizer：https://github.com/billytoe/mp4-optimizer

5. 使用批处理脚本"process_videos.py"，将多个MP4视频文件进行改名、生成视频预览图并生成本项目ESP32能够读取的文件结构格式
> 需要安装FFmpeg https://ffmpeg.org/download.html 和 python https://www.python.org/downloads/ ，并配置环境变量

使用示例：
```python
python process_videos.py /视频输入文件夹路径 /视频输出文件夹路径
```
"视频输入文件夹路径"存放你准备好的多个MP4文件，"视频输入文件夹路径"将得到本项目ESP32能够读取的文件结构格式，每个子文件夹包含0.jpg，0.txt，video.mp4

6. 运行程序后，将"视频输出文件夹路径"内文件全部复制到内存卡中的/video-mp4/shortvideo或/video-mp4/movie内，手动配置方式请参考项目中/内存卡/README.txt文件

大功告成，将内存卡插入内存卡座，接通电源，即可运行服务器


# 使用说明

- BOOT按键（0号引脚按键）功能
单击：服务器状态/时钟表盘切换显示
长按：关闭/开启WIFI

- 如何连接服务器
服务器上电默认处于AP模式，使用手机或者电脑，找到ESP32_webserver这个WIFI进行连接，默认密码为123456789，连接成功后，用浏览器访问192.168.4.1即可进入服务器主页。

- 如何WiFi配网
在服务器主页点击"模式转换" - "网页配网"，即可进入配网页面，按照提示输入需要配网的WiFi名称和密码后点击"提交"即可配网（注意：网页配网页面只有在AP模式下可以进入）

- 如何修改WiFi热点名称和密码
在服务器主页点击"服务器配置"，按照提示输入热点名称和密码后点击"提交"即可修改


# 功能展示：

![image](/assets/文件管理.jpg "文件管理")

![image](/assets/文件上传.jpg "文件上传")

![image](/assets/文件下载.jpg "文件下载")

![image](/assets/视频列表.jpg "视频列表")

![image](/assets/视频播放.jpg "视频播放")


# 视频演示：

- https://www.bilibili.com/video/BV1BoNg6kE8S/


# 参考项目

SD卡代码参考 https://youtu.be/e1xOgZsnAuw

网页响应代码参考 http://www.taichi-maker.com/homepage/esp8266-nodemcu-iot/iot-c/spiffs/spiffs-web-server/file-upload-server/

文件上传代码参考 https://github.com/smford/esp32-asyncwebserver-fileupload-example

视频播放器使用videoJS https://github.com/videojs/video.js

网页配网代码参考 https://github.com/yuan910715/Esp8266_NTP_Clock_Weather 中的网页配网部分

OLED屏幕时钟参考 https://github.com/ThingPulse/esp8266-oled-ssd1306 中的 examples/SSD1306ClockDemo

Font Awesome https://fontawesome.com/

ESPAsyncWebServer https://github.com/ESP32Async/ESPAsyncWebServer

AsyncTCP https://github.com/ESP32Async/AsyncTCP


# 额外说明

如需旧版代码，请到release或分支中下载


