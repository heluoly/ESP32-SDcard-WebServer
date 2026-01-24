#include "battery.h"

extern int batteryPercent;
extern TaskHandle_t Task_Bat;
extern SemaphoreHandle_t batTaskDoneSem;

//电池电压检测
int readBatteryVoltage() {
  int analogVolts = 0;
  digitalWrite(BAT_EN_PIN, HIGH);
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  for (char i = 0; i < 4; i++) {
    analogVolts += analogReadMilliVolts(BAT_ADC_PIN);
  }
  analogVolts = analogVolts / 4;  //测量4次取平均
  analogVolts = analogVolts * 2;  //分压系数
  // Serial.printf("ADC millivolts value = %d\n", analogVolts);
  digitalWrite(BAT_EN_PIN, LOW);
  vTaskDelay(500 / portTICK_PERIOD_MS);
  return analogVolts;
}

//电池电压转换为百分比
int voltageToPercent(int voltage_mv) {
  if (voltage_mv >= 4100)
    return 100;
  else if (voltage_mv >= 3700)
    return 50 + (voltage_mv - 3700) * 50 / 400;
  else if (voltage_mv >= 3500)
    return 20 + (voltage_mv - 3500) * 30 / 200;
  else if (voltage_mv >= 3300)
    return (voltage_mv - 3300) * 20 / 200;
  else
    return 0;
}
//电池电压转换为百分比
// int voltageToPercent(int voltage_mv) {
//   if (voltage_mv >= 4200)
//     return 100;
//   else if (voltage_mv >= 4000)
//     return 80 + (voltage_mv - 4000) * 10 / 100;
//   else if (voltage_mv >= 3700)
//     return 20 + (voltage_mv - 3700) * 20 / 100;
//   else if (voltage_mv >= 3600)
//     return 10 + (voltage_mv - 3600) * 10 / 100;
//   else if (voltage_mv >= 3500)
//     return 5 + (voltage_mv - 3500) * 5 / 100;
//   else if (voltage_mv >= 3300)
//     return (voltage_mv - 3300) * 5 / 200;
//   else
//     return 0;
// }

//更新电池电量
void task_bat(void* pvParameters) {
  int batteryVoltage = 0;                             //电池电压
  batteryVoltage = readBatteryVoltage();              //获取电池电压
  batteryPercent = voltageToPercent(batteryVoltage);  //电池电压转换为百分比
  xSemaphoreGive(batTaskDoneSem);                     // 通知任务已完成

  // UBaseType_t istack;
  // istack = uxTaskGetStackHighWaterMark(Task_Bat);
  // Serial.printf("Task_Bat istack = %d\n", istack);

  Task_Bat = NULL;
  vTaskDelete(NULL);  //删除任务
}

//创建更新电池电量任务
bool createBatTaskOnce() {
  // 已经有任务在运行
  if (Task_Bat != NULL) {
    // Serial.println("TaskBat already running");
    return false;
  }
  // 创建完成信号量（只创建一次）
  if (batTaskDoneSem == NULL) {
    batTaskDoneSem = xSemaphoreCreateBinary();
  }
  BaseType_t ret = xTaskCreatePinnedToCore(task_bat, "Task_Bat", 2560, NULL, 5, &Task_Bat, 1);  //更新电池电压
  if (ret != pdPASS) {
    Task_Bat = NULL;
    // Serial.println("TaskBat create failed");
    return false;
  }
  // Serial.println("TaskBat created");
  return true;
}
