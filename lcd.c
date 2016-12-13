/*************************************************
*描述：
*     LCD真彩液晶屏指令设置
*功能：
*    1. 通过单片机串口显示数字，汉字；
*    2. 液晶屏光标设置及初始化；

************************************************/
#include "STC15F2K60S2.h"
#include "intrins.h"
#include "config.h" 
#include "absacc.h"

void Txbyte(uchar idata i);
//void Txbyte_1(uchar idata i);
void Tword(uint idata i);
void Tword_1(uint idata i);
void TxEOF();
void TxEOF_1();

/**************************
*  向串口2发送一个字节
***************************/
void Txbyte(uchar idata i)
{
   uchar idata temp=0;
   IE2=0x00;            //关串口2中断,ES2=0
//   S2CON= S2CON&0xFD;   //B'11111101,清零串口2发送完成中断请求标志
   S2BUF=i;
   do
   {
     temp=S2CON;
     temp=temp&0x02;
    }
	while(temp==0);
    S2CON=S2CON&0xFD;   //B'11111101,清零串口2发送完成中断请求标志
    IE2=0x01;           //允许串口2中断,ES2=1
}

/**************************
*  向串口1(DCS)发送一个字节
***************************/
void T1byte(uchar idata i)
{
   uchar idata temp=0;
   IE&=0xEF;            //关串口1中断,ES=0
//   S2CON= S2CON&0xFD;   //B'11111101,清零串口2发送完成中断请求标志
   SBUF=i;
	while(TI==0);
	TI=0;
    IE|=0x10;           //允许串口1中断,ES=1
}


/**************************
*  向串口2发送一个字
***************************/
void Tword(uint idata i)
 {
     uchar idata x,y;
     x=(i/256);
     Txbyte(x);
     y=i-256*x;
     Txbyte(y);
 }

/****************************
*    向串口2发送帧结束符
*****************************/
void TxEOF()       //发送帧结束符CC 33 C3 3C
{
	 Txbyte(0xCC);
	 Txbyte(0x33);
	 Txbyte(0xC3);
	 Txbyte(0x3C);
 }

/****************************
*      红色数字显示
******************************/
void Trnumber(int idata x,int idata y,uint idata n)
	{
		 uchar idata a,b,c,d,e;
		 Txbyte(0xAA);
		 Txbyte(0x98);
		 Tword(x);				//x坐标
		 Tword(y);				//y坐标
		 Txbyte(0x24);
		 Txbyte(0xC1);			//显示背景色
		 Txbyte(0x05);
		 Txbyte(0xff);			//白色
		 Txbyte(0xff);			//红色
		 Txbyte(0xf8);//红色
		 Txbyte(0x00);
		 a=n/10000; 		//数据转换成ASCII码
		 b=n%10000/1000;
		 c=n%10000%1000/100;
		 d=n%10000%1000%100/10;
		 e=n%10000%1000%100%10;
		 if(a!=0)
		 {
			 Txbyte(a+0x30);
			 Txbyte(b+0x30);
			 Txbyte(c+0x30);
			 Txbyte(d+0x30);
			 Txbyte(e+0x30);
		 }	
		 else
		 {
			  if(b!=0)
			   {
				 Txbyte(b+0x30);
				 Txbyte(c+0x30);
				 Txbyte(d+0x30);
				 Txbyte(e+0x30);
			   } 
			  else
			   {
				if(c!=0)
				{
					 Txbyte(c+0x30);
					 Txbyte(d+0x30);
					 Txbyte(e+0x30);
				 } 
				 else
				  {
					if(d!=0)
					{
						 Txbyte(d+0x30);
						 Txbyte(e+0x30);
					 } 
					else
					 {
						Txbyte(e+0x30);
					 }
				}
			}
		}
		  
		 TxEOF();		   //发送结束符
		 shortdelay(20);
	}
	

/****************************
*      串口2数据显示
******************************/ 
void Tnumber(int idata x,int idata y,uint idata n)
{
	 uchar idata a,b,c,d,e;
	 Txbyte(0xAA);
	 Txbyte(0x98);
	 Tword(x);              //x坐标
	 Tword(y);	            //y坐标
     Txbyte(0x24);
     Txbyte(0x81);          //不显示背景色
     Txbyte(0x05);
     Txbyte(0xff);
     Txbyte(0xff);
     Txbyte(0x00);
     Txbyte(0x1f);
	 a=n/10000;	        //数据转换成ASCII码
	 b=n%10000/1000;
	 c=n%10000%1000/100;
	 d=n%10000%1000%100/10;
	 e=n%10000%1000%100%10;
	 if(a!=0)
	 {
		 Txbyte(a+0x30);
		 Txbyte(b+0x30);
	     Txbyte(c+0x30);
	     Txbyte(d+0x30);
	     Txbyte(e+0x30);
	 }  
     else
	 {
		  if(b!=0)
		   {
			 Txbyte(b+0x30);
			 Txbyte(c+0x30);
		     Txbyte(d+0x30);
		     Txbyte(e+0x30);
		   } 
 		  else
		   {
		    if(c!=0)
		    {
				 Txbyte(c+0x30);
	             Txbyte(d+0x30);
			     Txbyte(e+0x30);
		     } 
			 else
			  {
			    if(d!=0)
			    {
					 Txbyte(d+0x30);
	                 Txbyte(e+0x30);
			     } 
				else
				 {
                    Txbyte(e+0x30);
				 }
			}
		}
    }
	  
	 TxEOF();	       //发送结束符
	 shortdelay(20);
}

/*****************************
*       红色字体显示
******************************/
void Trxtext(uint idata x,uint idata y,uchar *s)
  {
	   Txbyte(0xAA);      //帧头0xAA
	   Txbyte(0x98);      //48*48点阵
	   Tword(x);	      //x坐标
	   Tword(y);	      //y坐标
       Txbyte(0x24);
	   Txbyte(0xC1);      //显示背景色
	   Txbyte(0x05);
	   Txbyte(0xff);
	   Txbyte(0xff);
	   Txbyte(0xf8);
	   Txbyte(0x00);
	   while(*s)	      //发送字符串内容
	    {
		  Txbyte(*s);
		  s++;
		  }
	   TxEOF();		      //发送帧结束符
       shortdelay(20);
   }


/*****************************
*       串口2文本显示
******************************/
void Txtext(uint idata x,uint idata y,uchar *s)
  {
	   Txbyte(0xAA);      //帧头0xAA
	   Txbyte(0x98);      //48*48点阵
	   Tword(x);	      //x坐标
	   Tword(y);	      //y坐标
       Txbyte(0x24);
	   Txbyte(0x81);      //不显示背景色
	   Txbyte(0x05);
	   Txbyte(0xff);
	   Txbyte(0xff);
	   Txbyte(0x00);
	   Txbyte(0x1f);
	   while(*s)	      //发送字符串内容
	    {
		  Txbyte(*s);
		  s++;
		  }
	   TxEOF();		      //发送帧结束符
       shortdelay(20);
   }

/*******************************
*        串口2屏幕初始化
*********************************/
void Lcd_init()
{
	 Txtext(250,182,"请 稍 等");
	 Txtext(120,250,"进 行 初 始 化 中"); 
 }

/*******************************
*        串口2屏幕开机
*********************************/
void Lcd_Start()
{
	 Txtext(190,182,"清 华 大 学");
	 Txtext(50,250,"核能与新能源技术研究院"); 
 }

/*******************************
*         串口2  清屏
********************************/
void Lcd_Clear()
{
	 Txbyte(0xAA);
	 Txbyte(0x52);
	 TxEOF();
 }

/*************************************
*          光标打开
*************************************/
void Open (uint idata x,uint idata y)
{
	Txbyte(0xAA);      //帧头0xAA
	Txbyte(0x44);      //光标显示
    Txbyte(0x01);      //光标显示打开
	Tword(x);	       //x坐标
	Tword(y);	       //y坐标
	Txbyte(0x1F);
	Txbyte(0x05);
    TxEOF();
}

/*************************************
*          光标关闭
*************************************/
void Close (uint idata x,uint idata y)
{
	Txbyte(0xAA);     //帧头0xAA
	Txbyte(0x44);     //光标显示
    Txbyte(0x00);     //光标显示关闭
	Tword(x);	     //x坐标
	Tword(y);	     //y坐标
	Txbyte(0x1B);
	Txbyte(0x05);
    TxEOF();
}

/************************************
*         清除光标
*************************************/
void Clear()
{
  Close(112+Incdataright*48-96,128+wb*68-68);
  Close(432+Incdataright*48-96,128+wb*68-68);
  Close(160+Incdataright*48-144,128+wb*68-68);
  Close(480+Incdataright*48-144,128+wb*68-68);
}


/****************************************
*          图片显示
****************************************/
/*void picture()
{
    Txbyte(0xAA);       
    Txbyte(0x70);
	Txbyte(0x01);
    TxEOF();
}*/

/*****************************
*       串口2红灯显示
******************************/
void WarnLed_On(uint idata x,uint idata y)
  {
	   Txbyte(0xAA);      
	   Txbyte(0x9C);      
       Txbyte(0x04); 
       Txbyte(0x00); 
	   Txbyte(0x00); 
       Txbyte(0x00); 
       Txbyte(0x00);
       Txbyte(0x00);  
	   Txbyte(0x28);      
	   Txbyte(0x00);
	   Txbyte(0x28);
       Tword(x);	     //x坐标
   	   Tword(y);	     //y坐标
	   TxEOF();		     //发送帧结束符
   }


/*****************************
*       串口2蓝色显示
******************************/
void WarnLed_Off(uint idata x,uint idata y)
  {
	   Txbyte(0xAA);      
	   Txbyte(0x9C);      
       Txbyte(0x05); 
       Txbyte(0x00); 
	   Txbyte(0x00); 
       Txbyte(0x00); 
       Txbyte(0x00);
       Txbyte(0x00);  
	   Txbyte(0x30);      
	   Txbyte(0x00);
	   Txbyte(0x30);
       Tword(x);	     //x坐标
   	   Tword(y);	     //y坐标
	   TxEOF();		     //发送帧结束符
   }

