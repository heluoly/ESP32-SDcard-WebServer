#include "oledClock.h"

extern WebServer esp32_server;

//时钟中心位置
extern char clockCenterX;
extern char clockCenterY;
extern char clockRadius;  //表盘大小
//时间
extern char hour;
extern char minute;
extern char second;
//网页时间缓存
extern char hour2;
extern char minute2;
extern char second2;
//oled显存
extern unsigned char oled_RAM[128][8];
//更新时间标志
extern char flag_timeSet;

//网页获取当前时间
void setTime() {

  String hour_str = esp32_server.arg("hour");
  String minute_str = esp32_server.arg("minute");
  String second_str = esp32_server.arg("second");

  hour2 = String2Char((char*)hour_str.c_str());
  minute2 = String2Char((char*)minute_str.c_str());
  second2 = String2Char((char*)second_str.c_str());

  if(hour2>11)
  {
    hour2 = hour2 - 12;
  }

  flag_timeSet = 1;

}

//画时钟表盘及指针
void oledClock_Display()
{
  memset(oled_RAM, 0, 128*8*sizeof(unsigned char));
  //hour ticks
  for ( int z = 0; z < 360; z = z + 30 ) {
    //Begin at 0° and stop at 360°
    float angle = z ;
    angle = ( angle / 57.29577951 ) ; //Convert degrees to radians
    int x2 = ( clockCenterX + ( sin(angle) * clockRadius ) );
    int y2 = ( clockCenterY - ( cos(angle) * clockRadius ) );
    int x3 = ( clockCenterX + ( sin(angle) * ( clockRadius - ( clockRadius / 8 ) ) ) );
    int y3 = ( clockCenterY - ( cos(angle) * ( clockRadius - ( clockRadius / 8 ) ) ) );
    OLED_Line( x2, y2, x3, y3,1);
  }
  // display second hand
  float angle = second * 6 ;
  angle = ( angle / 57.29577951 ) ; //Convert degrees to radians
  int x3 = ( clockCenterX + ( sin(angle) * ( clockRadius - ( clockRadius / 5 ) ) ) );
  int y3 = ( clockCenterY - ( cos(angle) * ( clockRadius - ( clockRadius / 5 ) ) ) );
  OLED_Line( clockCenterX , clockCenterY , x3 , y3 , 1);
  //
  // display minute hand
  angle = minute * 6 ;
  angle = ( angle / 57.29577951 ) ; //Convert degrees to radians
  x3 = ( clockCenterX + ( sin(angle) * ( clockRadius - ( clockRadius / 4 ) ) ) );
  y3 = ( clockCenterY - ( cos(angle) * ( clockRadius - ( clockRadius / 4 ) ) ) );
  OLED_Line( clockCenterX , clockCenterY , x3 , y3 , 1);
  //
  // display hour hand
  angle = hour * 30 + int( ( minute / 12 ) * 6 )   ;
  angle = ( angle / 57.29577951 ) ; //Convert degrees to radians
  x3 = ( clockCenterX + ( sin(angle) * ( clockRadius - ( clockRadius / 2 ) ) ) );
  y3 = ( clockCenterY - ( cos(angle) * ( clockRadius - ( clockRadius / 2 ) ) ) );
  OLED_Line( clockCenterX , clockCenterY , x3 , y3 , 1);

  OLED_Circle(clockCenterX, clockCenterY, 2,1);
  OLED_Display();

}

//时钟进位
void clockRun()
{
  second++;
  if(second>59)
  {
    second=0;
    minute++;
    if(minute>59)
    {
      minute=0;
      hour++;
      if(hour>11)
      {
        hour=0;
      }
    }
  }
}
