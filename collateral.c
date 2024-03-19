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
    float temp,temp1;
	  sendadd=0x00;                         //给从机发送开始指令
	  //Lcd_Clear();
	  //Txtext(100,182,"主机进行接收中...");
	  Ack1=1;
    i=0;
	  FlagColling = 0;
	  while(i<nbyte)
	  {
      CntColTimer = 0;                    //字节之间定时中断次数清零,重新计时
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
	       shortdelay(200);
	       display_b();                      //显示从机数据
		   for(t=0;t<7;t++)				//0-6共7个数据
		   {
         temp=(float)(receive_buf[8*t]*100)+(float)(receive_buf[8*t+1]*10)+(float)(receive_buf[8*t+2]);
         temp+=((float)receive_buf[8*t+3]/10.0)+((float)receive_buf[8*t+4]/100.0)+((float)receive_buf[8*t+5]/1000.0);
         if(receive_buf[8*t+6]==2)
         {
           temp*=1000;
         }
         else if(receive_buf[8*t+6]==3)
         {
           temp*=1000000;
         }
         /*****DCS发送数据准备****/
         if(temp<0)                                                 //若计算剂量小于零向DCS发送0
         {
           DCS_Send[t*4+32+4]=0x30;
           DCS_Send[t*4+32+5]=0x30;
           DCS_Send[t*4+32+6]=0x30;
           DCS_Send[t*4+32+7]=0x30;
         }
         else if(temp>=0)
         {
           if(temp<1)
           {
             temp1=(float)temp*1000;
             DCS_Send[t*4+32+4]=(uchar)((int)temp1/100)+0x30;
             DCS_Send[t*4+32+5]=(uchar)((int)temp1%100/10)+0x30;
             DCS_Send[t*4+32+6]=(uchar)((int)temp1%100%10)+0x30;
             DCS_Send[t*4+32+7]=0x30;
           }
           else if((temp>=1)&&(temp<10))
           {
             temp1=(float)temp*100;
             DCS_Send[t*4+32+4]=(uchar)((int)temp1/100)+0x30;
             DCS_Send[t*4+32+5]=(uchar)((int)temp1%100/10)+0x30;
             DCS_Send[t*4+32+6]=(uchar)((int)temp1%100%10)+0x30;
             DCS_Send[t*4+32+7]=0x31;
           }
           else if((temp>=10)&&(temp<100))
           {
             temp1=(float)temp*10;
             DCS_Send[t*4+32+4]=(uchar)((int)temp1/100)+0x30;
             DCS_Send[t*4+32+5]=(uchar)((int)temp1%100/10)+0x30;
             DCS_Send[t*4+32+6]=(uchar)((int)temp1%100%10)+0x30;
             DCS_Send[t*4+32+7]=0x32;
           }
           else if((temp>=100)&&(temp<1000))
           {
             temp1=(float)temp;
             DCS_Send[t*4+32+4]=(uchar)((int)temp1/100)+0x30;
             DCS_Send[t*4+32+5]=(uchar)((int)temp1%100/10)+0x30;
             DCS_Send[t*4+32+6]=(uchar)((int)temp1%100%10)+0x30;
             DCS_Send[t*4+32+7]=0x33;
           }
           else if((temp>=1000)&&(temp<10000))
           {
             temp1=(float)temp/10.0;
             DCS_Send[t*4+32+4]=(uchar)((int)temp1/100)+0x30;
             DCS_Send[t*4+32+5]=(uchar)((int)temp1%100/10)+0x30;
             DCS_Send[t*4+32+6]=(uchar)((int)temp1%100%10)+0x30;
             DCS_Send[t*4+32+7]=0x34;
           }
           else if((temp>=10000)&&(temp<100000))
           {
             temp1=(float)temp/100.0;
             DCS_Send[t*4+32+4]=(uchar)((int)temp1/100)+0x30;
             DCS_Send[t*4+32+5]=(uchar)((int)temp1%100/10)+0x30;
             DCS_Send[t*4+32+6]=(uchar)((int)temp1%100%10)+0x30;
             DCS_Send[t*4+32+7]=0x35;
           }
           else if((temp>=100000)&&(temp<1000000))
           {
             temp1=(float)temp/1000.0;
             DCS_Send[t*4+32+4]=(uchar)((int)temp1/100)+0x30;
             DCS_Send[t*4+32+5]=(uchar)((int)temp1%100/10)+0x30;
             DCS_Send[t*4+32+6]=(uchar)((int)temp1%100%10)+0x30;
             DCS_Send[t*4+32+7]=0x36;
           }
           else if((temp>=1000000)&&(temp<10000000))
           {
             temp1=(float)temp/10000.0;
             DCS_Send[t*4+32+4]=(uchar)((int)temp1/100)+0x30;
             DCS_Send[t*4+32+5]=(uchar)((int)temp1%100/10)+0x30;
             DCS_Send[t*4+32+6]=(uchar)((int)temp1%100%10)+0x30;
             DCS_Send[t*4+32+7]=0x37;
           }
           else if((temp>=10000000)&&(temp<100000000))
           {
             temp1=(float)temp/100000.0;
             DCS_Send[t*4+32+4]=(uchar)((int)temp1/100)+0x30;
             DCS_Send[t*4+32+5]=(uchar)((int)temp1%100/10)+0x30;
             DCS_Send[t*4+32+6]=(uchar)((int)temp1%100%10)+0x30;
             DCS_Send[t*4+32+7]=0x38;
           }
           else if((temp>=100000000)&&(temp<1000000000))
           {
             temp1=(float)temp/1000000.0;
             DCS_Send[t*4+32+4]=(uchar)((int)temp1/100)+0x30;
             DCS_Send[t*4+32+5]=(uchar)((int)temp1%100/10)+0x30;
             DCS_Send[t*4+32+6]=(uchar)((int)temp1%100%10)+0x30;
             DCS_Send[t*4+32+7]=0x39;
           }
         }
         if(receive_buf[8*t+7])
         {
           DCS_Send[65]=DCS_Send[65]|Svar1[t];
         }
         else
         {
           DCS_Send[65]=DCS_Send[65]&Svar0[t];
         }
		   }
		   FlagMasColSlavOK = 1;             //指示灯闪烁定时开始标志
     }
}
