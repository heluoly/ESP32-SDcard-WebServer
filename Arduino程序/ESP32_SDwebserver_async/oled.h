#ifndef __OLED_H
#define __OLED_H

#include <esp32-hal.h>
#include "pgmspace.h"
#include "freertos/FreeRTOS.h"
#include "driver/i2c_master.h"
#include "oledfont.h"

#define OLED_CMD 0   //写命令
#define OLED_DATA 1  //写数据

#define I2C_PORT_NUM I2C_NUM_0
#define OLED_I2C_FREQ 400000
#define OLED_ADDR 0x3C

void i2c_dma_init();
void OLED_WR_Byte(unsigned char dat, unsigned char mode);
void OLED_ColorTurn(unsigned char i);
void OLED_DisplayTurn(unsigned char i);
void OLED_Set_Pos(unsigned char x, unsigned char y);
void OLED_Display_On(void);
void OLED_Display_Off(void);
void OLED_Clear(void);
void OLED_ShowChar(unsigned char x, unsigned char y, unsigned char chr, unsigned char sizey);
void OLED_ShowChar_RAM(unsigned char x, unsigned char y, unsigned char chr, unsigned char sizey);
uint32_t oled_pow(unsigned char m, unsigned char n);
void OLED_ShowNum(unsigned char x, unsigned char y, uint32_t num, unsigned char len, unsigned char sizey);
void OLED_ShowNum_RAM(unsigned char x, unsigned char y, uint32_t num, unsigned char len, unsigned char sizey);
void OLED_ShowString(unsigned char x, unsigned char y, const char *chr, unsigned char sizey);
void OLED_ShowString_RAM(unsigned char x, unsigned char y, const char *chr, unsigned char sizey);
//void OLED_ShowChinese(unsigned char x,unsigned char y,const unsigned char no,unsigned char sizey);
// void OLED_DrawBMP(unsigned char x,unsigned char y,unsigned char sizex, unsigned char sizey,const unsigned char BMP[]);
void OLED_Init(void);

void OLED_WR_Page_DMA(uint8_t *data, size_t len);
void OLED_Display(void);
void OLED_DrawPoint(unsigned char x, unsigned char y, unsigned char c);
void OLED_Line(unsigned char x1, unsigned char y1, unsigned char x2, unsigned char y2, unsigned char c);
void OLED_Rectangle(unsigned char x1, unsigned char y1, unsigned char x2, unsigned char y2, unsigned char c);
void OLED_Circle(unsigned char x0, unsigned char y0, unsigned char r, unsigned char c);

#endif