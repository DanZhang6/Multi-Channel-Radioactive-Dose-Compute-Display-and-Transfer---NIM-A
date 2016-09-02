/*************************************************
*描述：
*     AT24C16数据存取设置
*功能：
*     存储时间，探头个数，设置参数，计数率

************************************************/

#include "STC15F2K60S2.h"
#include "intrins.h"
#include "config.h"

void delay_n5ms_24(uchar idata n);


/********开始函数****************/
void start_24(void)     //时钟线保持高电平期间，数据线电平从高到低的跳变作为总线的起始信号
{
     _nop_();
	 scl = 0 ;
	 sda = 1 ;
	 scl = 1 ;
     _nop_();      
	 sda = 0 ;
     _nop_();
     _nop_();
	 scl = 0 ;
     _nop_();
     _nop_();
     sda = 1 ;
}

/*******结束函数****************/
void stop_24(void)     //时钟线保持高电平期间，数据线电平从低到高的跳变作为总线的停止信号
{
	 scl = 0 ;
	 sda = 0 ;
	 scl = 1 ; _nop_() ;
	 sda = 1 ; _nop_() ;
	 scl = 0 ;
}
/*******************************
        读8 Bit 数据
 ******************************/
uchar read_8_bit_24(void)
{
	 uchar idata r_8_bit;
	 uchar idata n;
	 uchar idata m;
	 uchar idata temp[8] = 0 ;
	 
	 r_8_bit = 0 ;
	 scl = 0 ;scl = 1 ;
	 for(n=0; n<8; n++)					//得到数据，由高到低位
		{
			scl = 1 ;
			_nop_() ;
			temp[n] = sda ;
			scl = 0 ;
		}
	 scl = 0 ;
	 sda = 1 ;scl = 1 ; 
	 _nop_() ; _nop_() ;
	 scl = 0 ;							//处理确认位
	 
	 for(m=0; m<8; m++)					//数据处理
	 	{
			r_8_bit = r_8_bit << 1 ;
			if(temp[m]==1) r_8_bit = r_8_bit|0x01 ;
			else r_8_bit = r_8_bit|0x00 ;
		}
	 return r_8_bit ;
}

/***************************************
            写8 Bit 数据 x
****************************************/
void write_8_bit_24(uchar idata x)
{
	 uchar idata w_8_bit;
	 uchar idata n, m, a;
	 uchar idata temp[8] = 0 ;
	 w_8_bit = x ;
	 
	 for(n=0; n<8; n++)
	 	{
			a = w_8_bit&0x80 ;				//高位在前
			if(a==0x80) temp[n] = 1 ;
			else temp[n] = 0 ;
			w_8_bit = w_8_bit << 1 ;
		}
	 
	 sda = 1 ;scl = 0 ;
	 for(m=0; m<8; m++)						//先写高位
		{
			sda = temp[m] ;
			scl = 1 ;
			_nop_() ;
			scl = 0 ;
		}
	 
	 scl = 1 ; _nop_() ;
	 scl = 0 ;								//处理确认位

 }

/************************************************************************
 *                         读1 Byte 数据
 * 输入参数：r_add      要读的数据的地址,格式为:1010+页码(3位)+0+地址(8位)
 * 返回参数：r_1_byte   读得到的数据
**************************************************************************/
uchar read_1_byte_24(uint idata r_add)
{
	 uchar idata r_1_byte ;
	 uchar idata H , L, h;
	 
	 H = r_add / 256 ;						//得到高8位数据
	 L = r_add % 256 ;						//得到低8位数据
	 h = H+1 ;								//得到读指令的数据
	 
	 delay_n5ms_24(1);
	 
	 start_24();
	 write_8_bit_24(H);						//发写指令
	 	_nop_() ;_nop_() ;
        _nop_(); _nop_();
	 write_8_bit_24(L);						//写入地址
	 	_nop_() ;_nop_() ;
        _nop_(); _nop_();
	 
	 start_24();
	 write_8_bit_24(h);						//发读命令
	 	_nop_() ;_nop_() ;
        _nop_(); _nop_();
	
	 r_1_byte = read_8_bit_24() ;			//读得数据
	 	_nop_() ;_nop_() ;
        _nop_(); _nop_();
	 stop_24();
	
	return r_1_byte ;
}

/******************************************************************
 *                          写1 Byte 数据
 * 输入参数：w_add     写的地址，格式为:1010+页码(3位)+0+地址(8位)
 *		     w_1_byte  要写入的数据
*******************************************************************/
void write_1_byte_24(uint idata w_add , uchar idata w_1_byte)
{
	 uchar idata H,L;
	 H = w_add / 256 ;
	 L = w_add % 256 ;
	 
	 delay_n5ms_24(1);							//加延时，解决了连续写要延时的问题
	 
	 start_24();
	 write_8_bit_24(H);							//发写指令,写高8位
	   	_nop_() ;_nop_() ;
        _nop_(); _nop_();
	 
	 write_8_bit_24(L);							//写入地址,写低8位
	   	_nop_() ;_nop_() ;
        _nop_(); _nop_();
	 
	 write_8_bit_24(w_1_byte);					//写入数据
	   	_nop_() ;_nop_() ; 
        _nop_(); _nop_();
	 stop_24();

}

/************************************
 *        延时函数 5n ms
 ************************************/
void delay_n5ms_24(uchar idata n)
{
	 uchar idata q ;
	 uint idata p ;
	 for(q=0;q<n;q++)
	 for(p=0;p<5000;p++){};
}

/*************************************
 *         存储数据
*************************************/
void DS_SaveData(uchar *dt)
{
	uint data i;
	uint data w=0xa000;
	for(i=0;i<331;i++)
	 {
		write_1_byte_24(w,dt[i]);
        delay_n5ms_24(2);
        w++;
		if(w==0xa100)
	    {
		  w=0xa200;
		  }
	 }
}  

/*************************************
 *          读取数据
*************************************/
void DS_ReadData(uchar *dt)
{
	uint data j;
	uint data w=0xa000;
	for(j=0;j<331;j++)
	{
		dt[j]=read_1_byte_24(w);					//读取1Byte数据
        delay_n5ms_24(2);							//延时2*5=10ms
        w++;
		if(w==0xa100)
	    { 
	    w=0xa200;
		}
	}
}  
