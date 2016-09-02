/************************************************************
*描述：
*     并行传输设置
*功能：
*     NIM-A和NIM-B之间数据传输，通过主机显示从机的计数结果

***********************************************************/
#include "STC15F2K60S2.h"
#include "intrins.h"
#include "config.h" 
#include "absacc.h"

bit	FlagCollErr;								//默认0
bit FlagColling ;								//默认0
uint  CntColTimer;								//默认0

//#define receiveradd XBYTE[0xd000]               //并行传输接收地址
//#define sendadd     XBYTE[0xd800]               //并行传输发送地址
#define TURE 1
#define FALSE 0

extern void display_b();
uchar ChkData(uchar *pub) ;
/***********************************
*        检测主机命令
***********************************/
uchar CKMasterCmd()
{
    volatile uchar var;
	do
	{
	    if(FlagCollFall)              //等待超过1S,主从机通信失败
		{
			FlagCollFall = 0;
			Flag_Collateral = 0;
			CmOverTime = 0;
			return FALSE;
		}
		var=receiveradd; 
	}
	while(var!=0x00);               //等待主机的开始命令0x55
	if(var==0x00)
	{
	    FlagCollFall = 0;
		Flag_Collateral = 0;
		CmOverTime = 0;
		return TURE;
	}
	else
	{
	    FlagCollFall = 0;
		Flag_Collateral = 0;
		CmOverTime = 0; 
	    return FALSE;
	}

}
/*************************************
*       从机发送数据
*************************************/
void Send_Word(uchar *pbuf,uint ibyte)        
{	
   uint i;
   bit  bAck0;
    	
   if(CKMasterCmd())
   {
       //Lcd_Clear();
       //Txtext(100,182,"从机进行发送中...");
	   i=0;
	   while(i<ibyte)
	   {
	  		do
			{	
				if(CKMasterCmd()==FALSE)            //判断主机命令是否是0x55
                {
				     //Lcd_Clear();
				     //Txtext(100,182,"主从机无法通信");
                     return;                        //不是0x55，则跳出函数
				}
				bAck0=Ack0;	
			}
			while(bAck0==0);                        //等待主机接收数据状态刷新
            sendadd=pbuf[i];                        //主机接收状态刷新，从机往发送地址里送数
			Note1=0;                                //数据发送完毕
	        do
			{	
				if(CKMasterCmd()==FALSE)            //判断主机命令是否是0x55
                {
                     return;                        //不是0x55，则跳出函数
				}
				bAck0=Ack0;	
			}
			while(bAck0==1);                        //等待主机接收完毕数据
            Note1=1; 
			i++;
	   }
   }
   else
   {
      Lcd_Clear();
	  Tnumber(100,82,receiveradd);
      Txtext(100,182,"从机接收握手命令失败");
	}
}

/*************************************
*       主机采集数据
*************************************/
void Collect_Word(uchar *pub,uint nbyte)     //主机采集数据              
{
	  uint i,t;
      bit bNote0;	 
	  sendadd=0x00;                         //给从机发送开始指令
	  //Lcd_Clear();
	  //Txtext(100,182,"主机进行接收中...");
	  Ack1=1;                               
      i=0;
	  FlagColling = 0;           
	  while(i<nbyte)
	  {	
	    FlagColling = 1;                    //字节之间时间定时
		CntColTimer = 0;                    //字节之间定时中断次数清零,重新计时
		do                              
		{  
			if(FlagCollErr == 1)            //等待超过0.5S
			{
			   FlagCollErr = 0;             //超时标志清零
			   FlagColling = 0;             //字节之间定时清零
		       CntColTimer = 0;             //字节之间定时中断次数清零
			   //Lcd_Clear();
			   //Txtext(100,182,"主机接收失败");
               FlagMasColSlavOK = 0;        //灯闪烁定时标志清零，灯不需要闪烁
			   Flag_Warn_Led = 0;            //add 5.24
		       return;
			}
			bNote0=Note0;
		}
		while(bNote0==1);                   //等待从机刷新数据
		pub[i]=receiveradd;                 //从机刷新完数据，主机从接收地址里读入字节      
		Ack1=0;	                            //主机字节接收完毕
        i++; 
        do                              
		{  
			if(FlagCollErr == 1)            //等待超过0.5S
			{
			   FlagCollErr = 0;             //超时标志清零
			   FlagColling = 0;             //字节之间定时清零
		       CntColTimer = 0;             //字节之间定时中断次数清零                
               FlagMasColSlavOK = 0;        //灯闪烁定时标志清零，灯不需要闪烁
			   Flag_Warn_Led = 0;            //add 5.24
		       return;
			}
			bNote0=Note0;
		}
		while(bNote0==0);                   //need define
        Ack1=1;
	  }
	   FlagColling = 0;                     //超时标志清零
       CntColTimer = 0;                     //字节之间定时清零
	   FlagCollErr = 0;                     //字节之间定时中断次数清零 
	   if(PCOLSIG==0)                       //要求显示从的数据按键按下   add4.21
	   {
	       shortdelay(2000);
	       display_b();                      //显示从机数据
	       DCS_Send[64]=1;
		   for(t=65;t<121;t++)				//准备数据发给DCS
		   {
				DCS_Send[t]=receive_buf[t-65];
		   }
		   FlagMasColSlavOK = 1;             //指示灯闪烁定时开始标志
		}
       
 }

