#include "battery.h"

extern int batteryPercent;
extern TaskHandle_t Task_Bat;
extern SemaphoreHandle_t batTaskDoneSem;

// 电池电压检测
int readBatteryVoltage() {
  int analogVolts = 0;
  digitalWrite(BAT_EN_PIN, HIGH);
  vTaskDelay(1500 / portTICK_PERIOD_MS);
  for (char i = 0; i < 4; i++) {
    analogVolts += analogReadMilliVolts(BAT_ADC_PIN);
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
  analogVolts = analogVolts / 4;  // 测量4次取平均
  // analogVolts = analogReadMilliVolts(BAT_ADC_PIN);
  analogVolts = analogVolts * 2;  // 分压系数
  // Serial.printf("ADC millivolts value = %d\n", analogVolts);
  digitalWrite(BAT_EN_PIN, LOW);
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  return analogVolts;
}

struct SocPoint {
  int voltage;  // mV
  int soc;      // SOC
};

// 亿纬INR18650/35V
// const SocPoint socTable[] = {
//   { 4150, 100 },
//   { 4100, 94 },
//   { 4050, 77 },
//   { 4000, 72 },
//   { 3950, 66 },
//   { 3900, 59 },
//   { 3850, 53 },
//   { 3800, 47 },
//   { 3750, 40 },
//   { 3700, 35 },
//   { 3650, 28 },
//   { 3600, 22 },
//   { 3550, 18 },
//   { 3500, 13 },
//   { 3450, 7 },
//   { 3400, 4 },
//   { 3350, 2 },
//   { 3300, 0 }
// };

// 远东18650-4000mA
// const SocPoint socTable[] = {
//   { 4100, 100 },
//   { 4050, 84 },
//   { 4000, 78 },
//   { 3950, 73 },
//   { 3900, 69 },
//   { 3850, 64 },
//   { 3800, 60 },
//   { 3750, 55 },
//   { 3700, 52 },
//   { 3650, 49 },
//   { 3600, 45 },
//   { 3550, 42 },
//   { 3500, 38 },
//   { 3450, 34 },
//   { 3400, 30 },
//   { 3350, 26 },
//   { 3300, 21 },
//   { 3250, 17 },
//   { 3200, 13 },
//   { 3150, 9 },
//   { 3100, 6 },
//   { 3050, 3 },
//   { 3000, 0 }
// };

// 振华18650-40MP
const SocPoint socTable[] = {
  { 4100, 100 },
  { 4050, 86 },
  { 4000, 82 },
  { 3950, 78 },
  { 3900, 71 },
  { 3850, 62 },
  { 3800, 58 },
  { 3750, 53 },
  { 3700, 50 },
  { 3650, 45 },
  { 3600, 42 },
  { 3550, 37 },
  { 3500, 33 },
  { 3450, 29 },
  { 3400, 26 },
  { 3350, 22 },
  { 3300, 19 },
  { 3250, 15 },
  { 3200, 12 },
  { 3150, 8 },
  { 3100, 6 },
  { 3050, 3 },
  { 3000, 0 }
};

int voltageToPercent(int voltage_mv) {
  const int count = sizeof(socTable) / sizeof(socTable[0]);
  // if (voltage_mv >= 4150)
  if (voltage_mv >= 4100)
    return 100;
  // if (voltage_mv <= 3300)
  if (voltage_mv <= 3000)
    return 0;
  for (int i = 0; i < count - 1; i++) {
    int vHigh = socTable[i].voltage;
    int vLow = socTable[i + 1].voltage;
    if (voltage_mv <= vHigh && voltage_mv >= vLow) {
      int socHigh = socTable[i].soc;
      int socLow = socTable[i + 1].soc;
      return socLow + (voltage_mv - vLow) * (socHigh - socLow) / (vHigh - vLow);
    }
  }
  return 0;
}

// 更新电池电量
void task_bat(void* pvParameters) {
  int batteryVoltage = readBatteryVoltage();          // 电池原始电压
  batteryPercent = voltageToPercent(batteryVoltage);  // 电池电压转换为百分比

  xSemaphoreGive(batTaskDoneSem);  // 通知任务已完成

  // UBaseType_t istack;
  // istack = uxTaskGetStackHighWaterMark(Task_Bat);
  // Serial.printf("Task_Bat istack = %d\n", istack);

  Task_Bat = NULL;
  vTaskDelete(NULL);  // 删除任务
}

// 创建更新电池电量任务
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
  BaseType_t ret = xTaskCreatePinnedToCore(task_bat, "Task_Bat", 2560, NULL, 5, &Task_Bat, 1);  // 更新电池电压
  if (ret != pdPASS) {
    Task_Bat = NULL;
    // Serial.println("TaskBat create failed");
    return false;
  }
  // Serial.println("TaskBat created");
  return true;
}
