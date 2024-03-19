/***************************************************
* 描述：
       复位函数
* 功能：
*      将时间，探头个数，参数设置返回到初始化状态；

****************************************************/
#include "STC15F2K60S2.h"
#include "intrins.h"
#include "config.h" 
#include "absacc.h"

void ReSet()
{
	 Clear();
	 Lcd_Clear(); 
	 Txtext(188,216,"重 新 设 置");	
	 IE0 = 0;                                     //将外部中断0清0    
	 ES = 0;                                      //禁止串口中断
	 IE2 = 0x00;                                  //禁止串口2中断
	 Incdata = YuThouth[0];
	 Incdataright = 1;								//右移按键增加标志?
	 Inctime = 1;                                 //时间按键增加标志
	 Incinput = 8 ;                               //默认显示探头个数为8个
	 wb = 1;
     xh = 1;
	 bz = 1;
	 Flag_RefrInput = 0;
	 Flag_RefrPara = 0;
	 Flag_RefrRight = 0;
	 Flag_RefrTime = 0;
	 EX0 = 1;
}		     
