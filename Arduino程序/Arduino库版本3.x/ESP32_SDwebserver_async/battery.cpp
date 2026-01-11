#include "battery.h"

//电池电压检测
int readBatteryVoltage() {
  int analogVolts = 0;
  digitalWrite(BAT_EN_PIN, HIGH);
  vTaskDelay(200 / portTICK_PERIOD_MS);
  for (char i = 0; i < 4; i++) {
    analogVolts += analogReadMilliVolts(BAT_ADC_PIN);
  }
  analogVolts = analogVolts / 4;  //测量4次取平均
  analogVolts = analogVolts * 2;  //分压系数
  Serial.printf("ADC millivolts value = %d\n", analogVolts);
  digitalWrite(BAT_EN_PIN, LOW);
  return analogVolts;
}

//电池电压转换为百分比
int voltageToPercent(int voltage_mv) {
  if (voltage_mv >= 4200)
    return 100;
  else if (voltage_mv >= 4000)
    return 80 + (voltage_mv - 4000) * 10 / 100;
  else if (voltage_mv >= 3700)
    return 20 + (voltage_mv - 3700) * 20 / 100;
  else if (voltage_mv >= 3600)
    return 10 + (voltage_mv - 3600) * 10 / 100;
  else if (voltage_mv >= 3500)
    return 5 + (voltage_mv - 3500) * 5 / 100;
  else if (voltage_mv >= 3300)
    return (voltage_mv - 3300) * 5 / 200;
  else
    return 0;
}
