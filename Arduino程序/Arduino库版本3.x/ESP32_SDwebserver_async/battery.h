#ifndef __BATTERY_H
#define __BATTERY_H

#define BAT_EN_PIN 1   // 使能引脚
#define BAT_ADC_PIN 2  // 检测引脚

#include <Arduino.h>

int readBatteryVoltage();
int voltageToPercent(int voltage_mv);

#endif