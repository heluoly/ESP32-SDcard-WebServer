/*
ESP32_SD卡服务器
作者：B站 狼尾巴的猫
项目实现文件上传下载、网页flash游戏、播放视频、模式转换（AP STA AP+STA互相转换）、剪切板功能
SD卡代码参考 https://youtu.be/e1xOgZsnAuw
网页响应代码参考 http://www.taichi-maker.com/homepage/esp8266-nodemcu-iot/iot-c/spiffs/spiffs-web-server/file-upload-server/
文件上传代码参考 https://github.com/smford/esp32-asyncwebserver-fileupload-example
flash播放器使用objecty
视频播放器使用videoJS
网页配网代码参考 https://github.com/yuan910715/Esp8266_NTP_Clock_Weather 中的网页配网部分
OLED屏幕时钟参考 https://github.com/ThingPulse/esp8266-oled-ssd1306 中的 examples/SSD1306ClockDemo
*/

#include <WiFi.h>
//#include <WiFiMulti.h>
#include <WebServer.h>
#include "FS.h"
#include "SD_MMC.h"
#include "SPIFFS.h"
#include "common.h"
#include "myServer.h"
#include "game.h"
#include "upload.h"
#include "video.h"
#include "web.h"
#include "copy.h"
#include "wifiConnect.h"
#include "time.h"
#include "esp_sntp.h"
#include <ESPmDNS.h>

#include "Wire.h"
#include "oled.h"
#include "oledfont.h"

// #define LED_ON HIGH
// #define LED_OFF LOW
#define SWITCH_ON LOW
#define SWITCH_OFF HIGH
#define FORMAT_SPIFFS_IF_FAILED false  //如出现SPIFFS初始化失败，将该参数改true格式化SPIFFS

bool hasSD = false;         //是否有SD卡
bool ONE_BIT_MODE = false;  //设置SD卡模式 1bit：true 4bit：false

WebServer esp32_server(80);  // 建立网络服务器对象，该对象用于响应HTTP请求。监听端口（80）
WebServer server(80);        //WIFI配网
File fsUploadFile;           // 建立文件对象用于闪存文件上传

bool mode_switch = 1;   //用于控制模式变换中的跳出while循环
bool mode_switch2 = 1;  //用于跳过STA模式，转换到AP模式
bool mode_switch3 = 0;  //用于关闭WIFI标志
char mode_wifi = 0;     //用于显示当前WiFi模式

String mdnsName = "esp32";            //mdns名字
String IPAD = "192.168.1.1";          //在AP和STA模式下存储ESP32的IP地址
String ssid = "ESP32_WebServer";      //wifi名称
String password = "123456789";        //wifi密码（注意WiFi密码位数不要小于8位）
char channel = 1;                     //wifi信道
char ssid_hidden = 0;                 //WiFi隐身
char autoconnect = 0;                 //开机自动连接上次成功连接WiFi
String pressid = "yourwifi";          //上次成功连接wifi名称
String prepassword = "yourpassword";  //上次成功连接wifi密码（注意WiFi密码位数不要小于8位）

TaskHandle_t Task_Server;   //第1核心任务
TaskHandle_t Task_Display;  //第2核心任务
hw_timer_t *tim1 = NULL;    // 定时器1，用于定时息屏
hw_timer_t *tim2 = NULL;    // 定时器2，用于时钟计数

const char switchInput = 0;  //IO0作为按键
const char led = 33;         //IO33作为led指示灯
//状态标志
char switchState;       //按键状态
char LEDState;          //led灯状态
char flag_tim1 = 0;     //定时器1中断标志
char flag_tim2 = 0;     //定时器2中断标志
char flag_timeSet = 0;  //更新时间标志
char oledState = 0;     //oled状态标志（点亮、熄灭）
char oledFrame = 1;     //oled显示内容标志（服务器信息、时钟）
//时间
char hour = 10;
char minute = 10;
char second = 30;
//网页时间缓存
char hour2;
char minute2;
char second2;
//网络时间同步服务器地址
const char *ntpServer1 = "pool.ntp.org";
const char *ntpServer2 = "ntp.aliyun.com";

//oled显存
unsigned char oled_RAM[128][8];


void setup() {
  // Serial.begin(115200);  // 启动串口通讯
  // Serial.println("");
  #if CONFIG_SD
  //SD卡初始化
  /*
  //ESP32-S3 SD卡引脚定义
  int clk = 41;
  int cmd = 40;
  int d0  = 42;
  int d1  = 2;
  int d2  = 38;
  int d3  = 39;
  SD_MMC.setPins(clk, cmd, d0, d1, d2, d3);
  */
  // Serial.println("SD");
  if (!config_fs.begin("/sdcard", ONE_BIT_MODE))  //SD卡初始化
  {
    // Serial.println("Card Mount Failed");
    hasSD = false;
    return;
  } else {
    // Serial.println("SD Card Ready!");
    hasSD = true;
  }
  #else
  // Serial.println("SPIFFS");
  //SPIFFS初始化
  if (!config_fs.begin(FORMAT_SPIFFS_IF_FAILED)) {
    // Serial.println("SPIFFS Mount Failed");
    hasSD = false;
    return;
  } else {
    // Serial.println("SPIFFS Ready!");
    hasSD = true;
  }
  #endif

  //SPIFFS里是否存在配置文件config.txt
  if (!config_fs.exists("/config.txt")) {
    File configFile = config_fs.open("/config.txt", FILE_WRITE);
    if (!configFile) {
      // Serial.println("failed to open file for writing");
      return;
    }
    configFile.print("ssid=" + ssid + "\r\npassword=" + password + "\r\nchannel=1\r\nautoconnect=1\r\npressid=" + pressid + "\r\nprepassword=" + prepassword + "\r\nstaticIP=192.168.1.80\r\ngateway=192.168.1.1\r\nsubnet=255.255.255.0\r\ndns=223.5.5.5\r\n\0");
    configFile.close();
  }
}

void loop(void) {
  xTaskCreatePinnedToCore(task_server, "Task_Server", 30720, NULL, 1, &Task_Server, 1);    //创建第1核心服务器任务
  xTaskCreatePinnedToCore(task_display, "Task_Display", 4096, NULL, 1, &Task_Display, 0);  //创建第2核心显示任务
  vTaskDelete(NULL);
}


//第1核心任务
void task_server(void *pvParameters) {
  setCpuFrequencyMhz(240);  //CPU频率变为240MHz
  //function takes the following frequencies as valid values:
  //  240, 160, 80    <<< For all XTAL types
  //  40, 20, 10      <<< For 40MHz XTAL
  //  26, 13          <<< For 26MHz XTAL
  //  24, 12          <<< For 24MHz XTAL

  /*
  //ESP32-S3 SD卡引脚定义
  int clk = 41;
  int cmd = 40;
  int d0  = 42;
  int d1  = 2;
  int d2  = 38;
  int d3  = 39;
  SD_MMC.setPins(clk, cmd, d0, d1, d2, d3);
  */
  if (!my_fs.begin("/sdcard", ONE_BIT_MODE))  //SD卡初始化
  {
    // Serial.println("Card Mount Failed");
    hasSD = false;
    return;
  } else {
    // Serial.println("SD Card Ready!");
    hasSD = true;
  }
  // uint8_t cardType = my_fs.cardType();
  // if(cardType == CARD_NONE){
  //   // Serial.println("No SD card attached");
  //   return;
  // }
  // Serial.print("SD Card Type: ");   //输出SD卡信息
  // if(cardType == CARD_MMC){
  //   Serial.println("MMC");
  // } else if(cardType == CARD_SD){
  //   Serial.println("SDSC");
  // } else if(cardType == CARD_SDHC){
  //   Serial.println("SDHC");
  // } else {
  //   Serial.println("UNKNOWN");
  // }
  // uint64_t cardSize = my_fs.cardSize() / (1024 * 1024);
  // Serial.printf("SD Card Size: %lluMB\n", cardSize);
  // listDir(my_fs, "/", 0);
  // Serial.printf("Total space: %lluMB\n", my_fs.totalBytes() / (1024 * 1024));
  // Serial.printf("Used space: %lluMB\n", my_fs.usedBytes() / (1024 * 1024));

  if (hasSD) {
    WiFiconfigRead();  //读取保存的AP名称和密码
  }
  if (autoconnect == 1) {
    server_presta();
    closeServer();
  }
  while (1) {
    server_ap();
    closeServer();

    server_ap_sta();
    closeServer();

    if (mode_switch2) {
      ssid_hidden = 0;
      server_sta();
    }
    mode_switch2 = 1;
    closeServer();
  }
}

//关闭WIFI并删除任务
void closeServer() {
  if (mode_switch3) {
    ssid_hidden = 0;
    WiFi.mode(WIFI_OFF);     //关闭WIFI
    my_fs.end();             //关闭SD卡
    setCpuFrequencyMhz(80);  //CPU频率变为80MHz

    vTaskDelete(NULL);  //删除任务
  }
}

//第2核心任务
void task_display(void *pvParameters) {
  uint16_t pressTime = 0;      //长按时间计时
  uint8_t flag_press = 0;      //长按标志
  uint8_t wifiState = 254;     //WiFi状态 0:WL_IDLE_STATUS, 1:WL_NO_SSID_AVAIL, 3:WL_CONNECTED, 4:WL_CONNECT_FAILED, 6:WL_DISCONNECTED. 254:WL_STOPPED
  uint8_t flag_wifiState = 0;  //判断WiFi是否掉线
  uint8_t RSSI_value = 0;      //WiFi信号强度

  OLED_Init();  //oled初始化
  // OLED_ColorTurn(0);//0正常显示 1反色显示
  // OLED_DisplayTurn(0);//0正常显示 1翻转180度显示
  pinMode(switchInput, INPUT);  //按键初始化
  pinMode(led, OUTPUT);         //led灯初始化

  //计算息屏时间
  tim1 = timerBegin(1000000);             //定时器频率用于计算分频
  timerAttachInterrupt(tim1, &onTimer1);  //定时器地址指针，中断处理函数
  timerAlarm(tim1, 10000000, false, 0);   //定时器地址指针，定时时长，数值是否重载，重载数值
  timerStop(tim1);
  //时钟计数
  tim2 = timerBegin(1000000);             //定时器频率用于计算分频
  timerAttachInterrupt(tim2, &onTimer2);  //定时器地址指针，中断处理函数
  timerAlarm(tim2, 1000000, true, 0);     //定时器地址指针，定时时长，数值是否重载，重载数值
  // timerStart(tim2);                       //使能定时器2

  oledState = 0;
  // UBaseType_t istack;

  while (1) {
    switchState = digitalRead(switchInput);  //按键检测
    if (switchState == SWITCH_ON) {
      OLED_Display_On();  //开启OLED

      //长按检测
      switchState = digitalRead(switchInput);
      while (switchState == SWITCH_ON) {
        switchState = digitalRead(switchInput);
        vTaskDelay(10 / portTICK_PERIOD_MS);
        if ((oledFrame == 1) && (oledState == 1))  //在第一页，而且是屏幕亮起的状态下长按
        {
          pressTime++;
          if (pressTime > 300)  //长按时间大于3秒，跳出循环，并设置长按标志
          {
            flag_press = 1;
            oledFrame = 1;
            break;
          }
        }
      }
      pressTime = 0;  //长按计数清零

      if (flag_press)  //如果长按了3秒按键
      {

        if (mode_switch3)  //判断当前服务器所处状态，0为运行，1为关闭
        {
          mode_switch3 = 0;  //服务器状态变为开启
          mode_switch = 1;
          if (autoconnect == 1) {
            mode_wifi = 4;
          } else {
            mode_wifi = 1;
          }
          xTaskCreatePinnedToCore(task_server, "Task_Server", 15360, NULL, 1, &Task_Server, 1);  //创建新的服务器任务
        } else {
          mode_switch3 = 1;  //服务器状态变为关闭
          mode_switch = 0;   //让服务器任务跳出循环，运行结束程序
        }
        flag_press = 0;
      } else {
        //判断屏幕状态
        if (oledState == 1) {
          oledFrame++;  //下一页
          if (oledFrame > 2) {
            oledFrame = 1;
          }
          OLED_Clear();
        }
      }
      //第一页显示服务器信息
      if (oledFrame == 1) {
        OLED_ShowString(0, 0, "ESP32 WebServer", 16);
        if (mode_switch3) {
          OLED_ShowString(0, 2, "WLAN OFF    ", 16);
          OLED_ShowString(0, 4, "                ", 16);
          OLED_ShowString(0, 6, "                ", 16);
        } else {
          if (mode_wifi == 1) {
            OLED_ShowString(0, 2, "Mode: AP    ", 16);
            OLED_ShowString(0, 4, "Channel: ", 16);
            OLED_ShowNum(72, 4, channel, 2, 16);
            if (ssid_hidden) {
              OLED_ShowString(96, 4, "*", 16);
            }
            OLED_ShowString(0, 6, (char *)IPAD.c_str(), 16);  //显示当前服务器IP
          } else if (mode_wifi == 2) {
            OLED_ShowString(0, 2, "Mode: AP+STA", 16);
            OLED_ShowString(0, 4, "Channel: ", 16);
            OLED_ShowNum(72, 4, channel, 2, 16);
            if (ssid_hidden) {
              OLED_ShowString(96, 4, "*", 16);
            }
            OLED_ShowString(0, 6, (char *)IPAD.c_str(), 16);  //显示当前服务器IP
          } else if (mode_wifi == 3) {
            wifiState = WiFi.status();  //读取WiFi状态
            OLED_ShowString(0, 2, "Mode: STA   ", 16);
            if (wifiState == WL_CONNECTED) {
              if (flag_wifiState == 1) {
                IPAD = WiFi.localIP().toString();  //刷新IP地址
                flag_wifiState = 0;
              }
              OLED_ShowString(0, 4, (char *)IPAD.c_str(), 16);  //显示当前服务器IP
              OLED_ShowString(0, 6, "RSSI: -", 16);
              RSSI_value = -WiFi.RSSI();
              OLED_ShowNum(56, 6, RSSI_value, 2, 16);
              // Serial.println(WiFi.RSSI());
            } else if (wifiState == WL_IDLE_STATUS) {
              OLED_ShowString(0, 4, "Reconnecting", 16);
              flag_wifiState = 1;
            } else if (wifiState == WL_NO_SSID_AVAIL) {
              OLED_ShowString(0, 4, "No SSID Avail", 16);
              flag_wifiState = 1;
            } else if (wifiState == WL_CONNECT_FAILED) {
              OLED_ShowString(0, 4, "Connect Failed", 16);
              WiFi.reconnect();  //尝试重新连接WiFi
              flag_wifiState = 1;
            } else if (wifiState == WL_CONNECTION_LOST) {
              OLED_ShowString(0, 4, "Connect Lost", 16);
              flag_wifiState = 1;
            } else if (wifiState == WL_DISCONNECTED) {
              OLED_ShowString(0, 4, "Disconnected", 16);
              flag_wifiState = 1;
            } else {
              OLED_ShowString(0, 4, "Failed", 16);
              flag_wifiState = 1;
            }
            // Serial.printf("WL: %d\n", wifiState);

          } else {
            OLED_ShowString(0, 2, "Mode: STA   ", 16);
            OLED_ShowString(0, 4, "Connecting", 16);
          }
        }
      } else  //第二页，时钟
      {
        oledClock_Display();  //显示表盘和指针
      }

      //松手检测
      switchState = digitalRead(switchInput);
      while (switchState == SWITCH_ON) {
        switchState = digitalRead(switchInput);
        vTaskDelay(10 / portTICK_PERIOD_MS);
      }

      //更新息屏倒计时
      if (oledState) {
        timerRestart(tim1);  //重置定时器1
        timerStop(tim1);
        timerStart(tim1);  //使能定时器1
      } else {
        timerRestart(tim1);  //重置定时器1
        timerStart(tim1);    //使能定时器1
      }

      //开定时器，定时完成息屏
      flag_tim1 = 0;
      oledState = 1;  //oled状态变为点亮

      // istack = uxTaskGetStackHighWaterMark(Task_Display);
      // printf("Task_Display istack = %d\n", istack);

    } else {
      //息屏
      if (flag_tim1) {
        OLED_Clear();
        OLED_Display_Off();
        flag_tim1 = 0;
        oledState = 0;
        timerStop(tim1);
      }

      //第二页，亮屏时刷新时钟
      if (oledState && (oledFrame == 2) && flag_tim2) {
        flag_tim2 = 0;
        oledClock_Display();  //显示表盘和指针
      }
    }

    vTaskDelay(2 / portTICK_PERIOD_MS);
  }
}

//定时器1中断
void ARDUINO_ISR_ATTR onTimer1() {
  flag_tim1 = 1;
}
//定时器2中断
void ARDUINO_ISR_ATTR onTimer2() {
  flag_tim2 = 1;
  if (flag_timeSet)  //网页配置时间
  {
    flag_timeSet = 0;
    hour = hour2;
    minute = minute2;
    second = second2;
  }
  clockRun();  //时钟进位
}
