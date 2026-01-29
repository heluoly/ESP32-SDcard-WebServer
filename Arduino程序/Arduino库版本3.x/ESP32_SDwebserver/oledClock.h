#ifndef __OLEDCLOCK_H
#define __OLEDCLOCK_H

#include "common.h"
#include "oled.h"
#include "time.h"
#include "esp_sntp.h"

//时钟中心位置
#define clockCenterX 64  //时钟X轴
#define clockCenterY 32  //时钟Y轴
#define clockRadius 31   //表盘大小

void setTime();
void oledClock_Display();
void clockRun();
void task_sntp(void *pvParameters);

#endif