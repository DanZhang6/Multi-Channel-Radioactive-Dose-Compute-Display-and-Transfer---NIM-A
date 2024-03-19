/*************************************************
*描述：
*     单片机串口2设置
*功能：
*     通过串口2发送指令，在液晶屏上显示

************************************************/
#include "STC15F2K60S2.h"
#include "intrins.h"
#include "config.h" 
#include "absacc.h"

#define RELOAD_COUNT  0xFA        //22.1184MHz,12T,SMOD=0,115200bps

/*************************
*   串口1初始化   DCS改15
**************************/
void serial_port_one_initial()
{
    //SCON = 0x50;              //0101,0000 8位可变波特率，无奇偶校验位
    //BRT = RELOAD_COUNT;       //BRTR = 1, S1BRS = 1, EXTRAM = 1 ENABLE EXTRAM
    //AUXR = 0x15;              // T0x12,T1x12,UART_M0x6,BRTR,S2SMOD,BRTx12,EXTRAM,S1BRS
    //ES = 1;                   //允许串口中断
    //EA = 1;                   //开总中断
	SCON = 0x50;		//8位数据,可变波特率
	AUXR |= 0x40;		//定时器1时钟为Fosc,即1T
	AUXR &= 0xFE;		//串口1选择定时器1为波特率发生器
	TMOD &= 0x0F;		//设定定时器1为16位自动重装方式
	TL1 = 0x00;		//设定定时初值
	TH1 = 0xF7;		//设定定时初值
	ET1 = 0;		//禁止定时器1中断
	TR1 = 1;		//启动定时器1
}

/*************************
*   串口2初始化
**************************/
void serial_port_two_initial()
{
	//AH1*改为15系列串口二程序
    //S2CON = 0x50;             //0101,0000 8位可变波特率，无奇偶校验位,允许接收
    //BRT = RELOAD_COUNT;       // BRTR = 1, S1BRS = 1, EXTRAM = 0 ENABLE EXTRAM
    //AUXR = 0x15;              // T0x12,T1x12,UART_M0x6,BRTR,S2SMOD,BRTx12,EXTRAM,S1BRS
    //IE2 = 0x01;               //允许串口2中断,ES2=1
    //EA = 1;                   //开总中断
	S2CON = 0x50;		//8位数据,可变波特率
	AUXR |= 0x04;		//定时器2时钟为Fosc,即1T
	T2L = 0xD0;		//设定定时初值
	T2H = 0xFF;		//设定定时初值
	AUXR |= 0x10;		//启动定时器2
}

/**************************
*   串口1中断
***************************/
void UART_one_Interrupt_Receive(void) interrupt 4
{
    //if(TI)
    //{
    //  return;
	//}
	uchar data k = 0;
    k = SCON ;
    k = k&0x10;              //S1TI
    if(k == 1)       
    {
     return;    
    }
}
/**************************
*   串口2中断
***************************/
void UART_two_Interrupt_Receive(void) interrupt 8
{
    uchar data k = 0;
    k = S2CON ;
    k = k&0x10;              //S2TI
    if(k == 1)       
    {
     return;    
    }
}

