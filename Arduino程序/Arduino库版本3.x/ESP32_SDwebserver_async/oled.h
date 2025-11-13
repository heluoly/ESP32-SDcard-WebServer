#ifndef __OLED_H
#define __OLED_H

#include "oledfont.h"
#include "Wire.h"
#define OLED_CMD  0  //写命令
#define OLED_DATA 1 //写数据

#define oled_sda 21  //oled屏接口定义
#define oled_scl 22  //oled屏接口定义

void OLED_ColorTurn(unsigned char i);
void OLED_DisplayTurn(unsigned char i);
void OLED_WR_Byte(unsigned char dat,unsigned char mode);
void OLED_Set_Pos(unsigned char x, unsigned char y);
void OLED_Display_On(void);
void OLED_Display_Off(void);
void OLED_Clear(void);
void OLED_ShowChar(unsigned char x,unsigned char y,const unsigned char chr,unsigned char sizey);
uint32_t oled_pow(unsigned char m,unsigned char n);
void OLED_ShowNum(unsigned char x,unsigned char y,uint32_t num,unsigned char len,unsigned char sizey);
void OLED_ShowString(unsigned char x,unsigned char y,const char *chr,unsigned char sizey);
//void OLED_ShowChinese(unsigned char x,unsigned char y,const unsigned char no,unsigned char sizey);
//void OLED_DrawBMP(unsigned char x,unsigned char y,unsigned char sizex, unsigned char sizey,const unsigned char BMP[]);
void OLED_Init(void);

void OLED_Display(void);
void OLED_DrawPoint(unsigned char x,unsigned char y, unsigned char c);
void OLED_Line(unsigned char x1, unsigned char y1, unsigned char x2, unsigned char y2, unsigned char c);
void OLED_Rectangle(unsigned char x1, unsigned char y1, unsigned char x2, unsigned char y2, unsigned char c);
void OLED_Circle(unsigned char x0, unsigned char y0, unsigned char r, unsigned char c);

#endif