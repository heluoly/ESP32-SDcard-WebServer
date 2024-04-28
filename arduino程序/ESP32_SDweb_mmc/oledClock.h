#ifndef __OLEDCLOCK_H
#define __OLEDCLOCK_H

#include "common.h"
#include <WebServer.h>
#include "oled.h"
#include "time.h"
#include "sntp.h"

void setTime();
void oledClock_Display();
void clockRun();
void task_sntp(void *pvParameters);

#endif