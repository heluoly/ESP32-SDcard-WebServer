//////////////////////////////////////////////////////////////////////////////////   
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//中景园电子
//店铺地址：https://oled-zjy.taobao.com/
//
//  文 件 名   : main.c
//  版 本 号   : v2.0
//  作    者   : HuangKai
//  生成日期   : 2019-3-19
//  功能描述   : arduino UNO OLED显示屏例程
// 作    者   : HuangKai
//Copyright(C) 中景园电子2019-3-19

#include "oled.h"

extern unsigned char oled_RAM[128][8];

//反显函数
void OLED_ColorTurn(unsigned char i)
{
  if(!i) OLED_WR_Byte(0xA6,OLED_CMD);//正常显示
  else  OLED_WR_Byte(0xA7,OLED_CMD);//反色显示
}

//屏幕旋转180度
void OLED_DisplayTurn(unsigned char i)
{
  if(i==0)
    {
      OLED_WR_Byte(0xC8,OLED_CMD);//正常显示
      OLED_WR_Byte(0xA1,OLED_CMD);
    }
else
    {
      OLED_WR_Byte(0xC0,OLED_CMD);//反转显示
      OLED_WR_Byte(0xA0,OLED_CMD);
    }
}

//发送一个字节
//向SSD1306写入一个字节。
//mode:数据/命令标志 0,表示命令;1,表示数据;
void OLED_WR_Byte(unsigned char dat,unsigned char mode)
{
  Wire.beginTransmission(0x3c);
  if(mode){Wire.write(0x40);}
  else{Wire.write(0x00);}
  Wire.write(dat); // sends one byte
  Wire.endTransmission();    // stop transmitting
}

//坐标设置

void OLED_Set_Pos(unsigned char x, unsigned char y) 
{ 
  OLED_WR_Byte(0xb0+y,OLED_CMD);
  OLED_WR_Byte(((x&0xf0)>>4)|0x10,OLED_CMD);
  OLED_WR_Byte((x&0x0f),OLED_CMD);
}       
//开启OLED显示    
void OLED_Display_On(void)
{
  OLED_WR_Byte(0X8D,OLED_CMD);  //SET DCDC命令
  OLED_WR_Byte(0X14,OLED_CMD);  //DCDC ON
  OLED_WR_Byte(0XAF,OLED_CMD);  //DISPLAY ON
}
//关闭OLED显示     
void OLED_Display_Off(void)
{
  OLED_WR_Byte(0X8D,OLED_CMD);  //SET DCDC命令
  OLED_WR_Byte(0X10,OLED_CMD);  //DCDC OFF
  OLED_WR_Byte(0XAE,OLED_CMD);  //DISPLAY OFF
}            
//清屏函数,清完屏,整个屏幕是黑色的!和没点亮一样!!!   
void OLED_Clear(void)  
{  
  unsigned char i,n;       
  for(i=0;i<8;i++)  
  {  
    OLED_WR_Byte (0xb0+i,OLED_CMD);    //设置页地址（0~7）
    OLED_WR_Byte (0x00,OLED_CMD);      //设置显示位置—列低地址
    OLED_WR_Byte (0x10,OLED_CMD);      //设置显示位置—列高地址   
    for(n=0;n<128;n++)
    {
      OLED_WR_Byte(0,OLED_DATA); 
      oled_RAM[n][i]=0;
    }
  } //更新显示
}


//在指定位置显示一个字符
//x:0~127
//y:0~63         
//sizey:选择字体 6x8  8x16
void OLED_ShowChar(unsigned char x,unsigned char y,const unsigned char chr,unsigned char sizey)
{       
  unsigned char c=0,sizex=sizey/2,temp;
  uint16_t i=0,size1;
  if(sizey==8)size1=6;
  else size1=(sizey/8+((sizey%8)?1:0))*(sizey/2);
  c=chr-' ';//得到偏移后的值
  OLED_Set_Pos(x,y);
  for(i=0;i<size1;i++)
  {
    if(i%sizex==0&&sizey!=8) OLED_Set_Pos(x,y++);
    if(sizey==8)
    {
      temp=pgm_read_byte(&asc2_0806[c][i]);
      OLED_WR_Byte(temp,OLED_DATA);//6X8字号
    }
    else if(sizey==16) 
    {
      temp=pgm_read_byte(&asc2_1608[c][i]);
      OLED_WR_Byte(temp,OLED_DATA);//8x16字号
    }
    else return;
  }
}
//m^n函数
uint32_t oled_pow(unsigned char m,unsigned char n)
{
  uint32_t result=1;  
  while(n--)result*=m;    
  return result;
}         
//显示数字
//x,y :起点坐标
//num:要显示的数字
//len :数字的位数
//sizey:字体大小      
void OLED_ShowNum(unsigned char x,unsigned char y,uint32_t num,unsigned char len,unsigned char sizey)
{           
  unsigned char t,temp,m=0;
  unsigned char enshow=0;
  if(sizey==8)m=2;
  for(t=0;t<len;t++)
  {
    temp=(num/oled_pow(10,len-t-1))%10;
    if(enshow==0&&t<(len-1))
    {
      if(temp==0)
      {
        OLED_ShowChar(x+(sizey/2+m)*t,y,' ',sizey);
        continue;
      }else enshow=1;
    }
    OLED_ShowChar(x+(sizey/2+m)*t,y,temp+'0',sizey);
  }
}
//显示一个字符号串
void OLED_ShowString(unsigned char x,unsigned char y,const char *chr,unsigned char sizey)
{
  unsigned char j=0;
  while (chr[j]!='\0')
  {   
    OLED_ShowChar(x,y,chr[j++],sizey);
    if(sizey==8)x+=6;
    else x+=sizey/2;
  }
}
/*
//显示汉字
void OLED_ShowChinese(unsigned char x,unsigned char y,const unsigned char no,unsigned char sizey)
{
  uint16_t i,size1=(sizey/8+((sizey%8)?1:0))*sizey;
  unsigned char temp;
  for(i=0;i<size1;i++)
  {
    if(i%sizey==0) OLED_Set_Pos(x,y++);
    if(sizey==16) 
    {
      temp=pgm_read_byte(&Hzk[no][i]);
      OLED_WR_Byte(temp,OLED_DATA);//16x16字号
    }
//    else if(sizey==xx) OLED_WR_Byte(xxx[c][i],OLED_DATA);//用户添加字号
    else return;
  }       
}
*/
/*
//显示图片
//x,y显示坐标
//sizex,sizey,图片长宽
//BMP：要显示的图片
void OLED_DrawBMP(unsigned char x,unsigned char y,unsigned char sizex, unsigned char sizey,const unsigned char BMP[])
{   
  uint16_t j=0;
  unsigned char i,m,temp;
  sizey=sizey/8+((sizey%8)?1:0);
  for(i=0;i<sizey;i++)
  {
    OLED_Set_Pos(x,i+y);
    for(m=0;m<sizex;m++)
    {
       temp=pgm_read_byte(&BMP[j++]);
       OLED_WR_Byte(temp,OLED_DATA);
    }
  }
}
*/
//OLED的初始化
void OLED_Init(void)
{
  //pinMode(res,OUTPUT);//RES
  Wire.begin(21,22,400000); // join i2c bus (address optional for master)
  // Wire.begin(21,22);
  //OLED_RES_Clr();
  delay(200);
  //OLED_RES_Set();
  
  OLED_WR_Byte(0xAE,OLED_CMD);//--turn off oled panel
  OLED_WR_Byte(0x00,OLED_CMD);//---set low column address
  OLED_WR_Byte(0x10,OLED_CMD);//---set high column address
  OLED_WR_Byte(0x40,OLED_CMD);//--set start line address  Set Mapping RAM Display Start Line (0x00~0x3F)
  OLED_WR_Byte(0x81,OLED_CMD);//--set contrast control register
  OLED_WR_Byte(0xCF,OLED_CMD); // Set SEG Output Current Brightness
  OLED_WR_Byte(0xA1,OLED_CMD);//--Set SEG/Column Mapping     0xa0左右反置 0xa1正常
  OLED_WR_Byte(0xC8,OLED_CMD);//Set COM/Row Scan Direction   0xc0上下反置 0xc8正常
  OLED_WR_Byte(0xA6,OLED_CMD);//--set normal display
  OLED_WR_Byte(0xA8,OLED_CMD);//--set multiplex ratio(1 to 64)
  OLED_WR_Byte(0x3f,OLED_CMD);//--1/64 duty
  OLED_WR_Byte(0xD3,OLED_CMD);//-set display offset Shift Mapping RAM Counter (0x00~0x3F)
  OLED_WR_Byte(0x00,OLED_CMD);//-not offset
  OLED_WR_Byte(0xd5,OLED_CMD);//--set display clock divide ratio/oscillator frequency
  OLED_WR_Byte(0x80,OLED_CMD);//--set divide ratio, Set Clock as 100 Frames/Sec
  OLED_WR_Byte(0xD9,OLED_CMD);//--set pre-charge period
  OLED_WR_Byte(0xF1,OLED_CMD);//Set Pre-Charge as 15 Clocks & Discharge as 1 Clock
  OLED_WR_Byte(0xDA,OLED_CMD);//--set com pins hardware configuration
  OLED_WR_Byte(0x12,OLED_CMD);
  OLED_WR_Byte(0xDB,OLED_CMD);//--set vcomh
  OLED_WR_Byte(0x40,OLED_CMD);//Set VCOM Deselect Level
  OLED_WR_Byte(0x20,OLED_CMD);//-Set Page Addressing Mode (0x00/0x01/0x02)
  OLED_WR_Byte(0x02,OLED_CMD);//
  OLED_WR_Byte(0x8D,OLED_CMD);//--set Charge Pump enable/disable
  OLED_WR_Byte(0x14,OLED_CMD);//--set(0x10) disable
  OLED_WR_Byte(0xA4,OLED_CMD);// Disable Entire Display On (0xa4/0xa5)
  OLED_WR_Byte(0xA6,OLED_CMD);// Disable Inverse Display On (0xa6/a7) 
  OLED_Clear();
  OLED_WR_Byte(0xAF,OLED_CMD); /*display ON*/ 
}


// //局部刷新
// void OLED_Display2(void){
// 	unsigned char i,n;
// 	for(i=0;i<8;i++)
// 	{
// 		OLED_WR_Byte (0xb0+i,OLED_CMD);
// 		OLED_WR_Byte (0x00,OLED_CMD);
// 		OLED_WR_Byte (0x10,OLED_CMD);
// 		for(n=32;n<94;n++){
// 			OLED_WR_Byte(oled_RAM[n][i],OLED_DATA);
// 		}
// 	}
// }

//全局刷新
void OLED_Display(void){
	unsigned char i,n;
	for(i=0;i<8;i++)
	{
		OLED_WR_Byte (0xb0+i,OLED_CMD);
		OLED_WR_Byte (0x00,OLED_CMD);
		OLED_WR_Byte (0x10,OLED_CMD);
		for(n=0;n<128;n++){
			OLED_WR_Byte(oled_RAM[n][i],OLED_DATA);
		}
	}
}

//画点
//x,y:坐标
//c  :亮灭
void OLED_DrawPoint(unsigned char x, unsigned char y, unsigned char c){
	unsigned char page=y/8;//0~7
  y=y%8;
  if(c)
  {
      oled_RAM[x][page]|=1<<y; 
  }
  else
  {
      oled_RAM[x][page]&=~(1<<y);
  } 

}

//画线
//x1,y1:起点坐标
//x2,y2:终点坐标  
//c    :亮灭
void OLED_Line(unsigned char x1, unsigned char y1, unsigned char x2, unsigned char y2, unsigned char c)
{
    unsigned char t; 
    int xerr=0,yerr=0,delta_x,delta_y,distance; 
    int incx,incy,uRow,uCol; 
    delta_x=x2-x1; //计算坐标增量 
    delta_y=y2-y1; 
    uRow=x1; 
    uCol=y1; 
    if(delta_x>0)incx=1; //设置单步方向 
    else if(delta_x==0)incx=0;//垂直线 
    else {incx=-1;delta_x=-delta_x;} 
    if(delta_y>0)incy=1; 
    else if(delta_y==0)incy=0;//水平线 
    else{incy=-1;delta_y=-delta_y;} 
    if( delta_x>delta_y)distance=delta_x; //选取基本增量坐标轴 
    else distance=delta_y; 
    for(t=0;t<=distance+1;t++ )//画线输出 
    {  
        OLED_DrawPoint(uRow,uCol,c);//画点 
        xerr+=delta_x ; 
        yerr+=delta_y ; 
        if(xerr>distance) 
        { 
            xerr-=distance; 
            uRow+=incx; 
        } 
        if(yerr>distance) 
        { 
            yerr-=distance; 
            uCol+=incy; 
        } 
    }
}    
//画矩形      
//(x1,y1),(x2,y2):矩形的对角坐标
//c:亮灭
void OLED_Rectangle(unsigned char x1, unsigned char y1, unsigned char x2, unsigned char y2, unsigned char c)
{
    OLED_Line(x1,y1,x2,y1,c);
    OLED_Line(x1,y1,x1,y2,c);
    OLED_Line(x1,y2,x2,y2,c);
    OLED_Line(x2,y1,x2,y2,c); 
}
//在指定位置画一个指定大小的圆
//(x,y):中心点
//r    :半径
//c    :亮灭
void OLED_Circle(unsigned char x0,unsigned char y0,unsigned char r,unsigned char c)
{
    int a,b;
    int di;
    a=0;b=r;      
    di=3-(r<<1);             //判断下个点位置的标志
    while(a<=b)
    {
        OLED_DrawPoint(x0+a,y0-b,c);             //5
        OLED_DrawPoint(x0+b,y0-a,c);             //0           
        OLED_DrawPoint(x0+b,y0+a,c);             //4               
        OLED_DrawPoint(x0+a,y0+b,c);             //6 
        OLED_DrawPoint(x0-a,y0+b,c);             //1       
        OLED_DrawPoint(x0-b,y0+a,c);             
        OLED_DrawPoint(x0-a,y0-b,c);             //2             
        OLED_DrawPoint(x0-b,y0-a,c);             //7                  
        a++;
        //使用Bresenham算法画圆     
        if(di<0)di +=4*a+6;      
        else
        {
            di+=10+4*(a-b);   
            b--;
        }                             
    }
}





