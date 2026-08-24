#include "oledClock.h"
#include "battery.h"

// 时间
extern char hour;
extern char minute;
extern char second;
// 网页时间缓存
extern char hour2;
extern char minute2;
extern char second2;
// oled显存
extern unsigned char oled_RAM[128][8];
// 更新时间标志
extern char flag_timeSet;

extern const char *ntpServer1;
extern const char *ntpServer2;

// extern TaskHandle_t Task_Sntp;  //网络时间同步任务

static void updateDisplayTime(const struct tm *timeinfo) {
  hour2 = timeinfo->tm_hour;
  minute2 = timeinfo->tm_min;
  second2 = timeinfo->tm_sec;

  if (hour2 > 11) {
    hour2 = hour2 - 12;
  }
  flag_timeSet = 1;
}

void setTime(AsyncWebServerRequest *request) {
  String t_str = request->getParam("t")->value();
  time_t t = (time_t)strtoll(t_str.c_str(), NULL, 10);
  if (t > 0) {
    setenv("TZ", "CST-8", 1);
    tzset();

    struct timeval tv;
    tv.tv_sec = t;
    tv.tv_usec = 0;
    settimeofday(&tv, NULL);

    struct tm timeinfo;
    localtime_r(&t, &timeinfo);    // 转换成具体的时间参数
    updateDisplayTime(&timeinfo);  // 更新时钟表盘

    char buf[80];
    snprintf(buf, sizeof(buf), "时间配置成功: %04d-%02d-%02d %02d:%02d:%02d",
             timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday,
             timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
    request->send(200, "text/plain; charset=utf-8", buf);
    return;
  }

  request->send(400, "text/plain", "时间配置失败");
}

// 网络时间同步任务
void task_sntp(void *pvParameters) {
  struct tm timeinfo;      // 存放转换后时间
  char ntp_time_out = 0;   // 同步时间超时
  char ntp_completed = 0;  // 同步时间完成

  if (WiFi.status() != WL_CONNECTED) {
    vTaskDelete(NULL);
    return;
  }

  esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);  // 设置单播模式
  esp_sntp_setservername(0, (char *)ntpServer1);
  esp_sntp_setservername(1, (char *)ntpServer2);
  setenv("TZ", "CST-8", 1);  // 东八区
  tzset();                   // 更新本地C库时间
  esp_sntp_init();           // 开始同步时间

  for (ntp_time_out = 0; ntp_time_out < 30; ntp_time_out++) {
    if (sntp_get_sync_status() == SNTP_SYNC_STATUS_COMPLETED) {
      ntp_completed = 1;
      break;
    } else {
      vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
  }
  esp_sntp_stop();  // 结束同步时间

  if (ntp_completed) {
    time_t now;
    time(&now);
    localtime_r(&now, &timeinfo);  // 转换成具体的时间参数
    updateDisplayTime(&timeinfo);  // 更新时钟表盘
  }
  // UBaseType_t istack;
  // istack = uxTaskGetStackHighWaterMark(Task_Sntp);
  // printf("Task_Sntp istack = %d\n", istack);
  vTaskDelete(NULL);  // 删除任务
}


// 画时钟表盘及指针
void oledClock_Display() {
  memset(oled_RAM, 0, 128 * 8 * sizeof(unsigned char));
  // hour ticks
  for (int z = 0; z < 360; z = z + 30) {
    // Begin at 0° and stop at 360°
    float angle = z;
    angle = (angle / 57.29577951);  // Convert degrees to radians
    int x2 = (clockCenterX + (sin(angle) * clockRadius));
    int y2 = (clockCenterY - (cos(angle) * clockRadius));
    int x3 = (clockCenterX + (sin(angle) * (clockRadius - (clockRadius / 8))));
    int y3 = (clockCenterY - (cos(angle) * (clockRadius - (clockRadius / 8))));
    OLED_Line(x2, y2, x3, y3, 1);
  }
  // display second hand
  float angle = second * 6;
  angle = (angle / 57.29577951);  // Convert degrees to radians
  int x3 = (clockCenterX + (sin(angle) * (clockRadius - (clockRadius / 5))));
  int y3 = (clockCenterY - (cos(angle) * (clockRadius - (clockRadius / 5))));
  OLED_Line(clockCenterX, clockCenterY, x3, y3, 1);
  //
  // display minute hand
  angle = minute * 6;
  angle = (angle / 57.29577951);  // Convert degrees to radians
  x3 = (clockCenterX + (sin(angle) * (clockRadius - (clockRadius / 4))));
  y3 = (clockCenterY - (cos(angle) * (clockRadius - (clockRadius / 4))));
  OLED_Line(clockCenterX, clockCenterY, x3, y3, 1);
  //
  // display hour hand
  angle = hour * 30 + int((minute / 12) * 6);
  angle = (angle / 57.29577951);  // Convert degrees to radians
  x3 = (clockCenterX + (sin(angle) * (clockRadius - (clockRadius / 2))));
  y3 = (clockCenterY - (cos(angle) * (clockRadius - (clockRadius / 2))));
  OLED_Line(clockCenterX, clockCenterY, x3, y3, 1);

  OLED_Circle(clockCenterX, clockCenterY, 2, 1);
  OLED_Display();
}

// 时钟进位
void clockRun() {
  second++;
  if (second > 59) {
    second = 0;
    minute++;
    createBatTaskOnce();  // 创建更新电池电量任务
    if (minute > 59) {
      minute = 0;
      hour++;
      if (hour > 11) {
        hour = 0;
      }
    }
  }
}
