/********************************************************************************************
*描述：
*   8253计数设置
*功能：
*   1.从8253中读取计数，每个8253中含有3个计数器，五个8253计15个探头的计数，
*     前七个探头是双探头（低计数探头和高计数探头），第八个探头是单探头（高计数探头），
*     根据计数率选择哪个探头工作；
*   2.T0定时中断
*     T0工作在16位自动重载模式，向上计数，每循环一次，总定时5Ms,计数初值DC00，晶振22.1184M；
*   3.定时到，读取计数，根据设定的参数，显示各探头的计数率；
*   4.根据计数率的不同，选择不同的单位（uGy/s,mGy/s,Gy/s）;

***********************************************************************************************/
#include "STC15F2K60S2.h"
#include "intrins.h"
#include "config.h"
#include "absacc.h"
#include "stdio.h"
#include "math.h"

#define C82531C XBYTE[0x8300]                                       //8253的命令端口（地址），CS4=1,CS3=1,CS2=1,CS1=1,CS0=0,A1A0=11；
#define C825310D XBYTE[0x8000]                                      //计数器0CS4=1,CS3=1,CS2=1,CS1=1,CS0=0,A1A0=00；
#define C825311D XBYTE[0x8100]                                      //计数器1CS4=1,CS3=1,CS2=1,CS1=1,CS0=0,A1A0=01;
#define C825312D XBYTE[0x8200]                                      //计数器2CS4=1,CS3=1,CS2=1,CS1=1,CS0=0,A1A0=10;

#define C82532C XBYTE[0x8b00]                                       // XBYTE的作用是将外部I/O端口置为中括号内部的16位二进制，即为设置
#define C825320D XBYTE[0x8800]                                      //使能端， XBYTE的作用还有设置使能端后，自动设置wr,rd.
#define C825321D XBYTE[0x8900]
#define C825322D XBYTE[0x8a00]

#define C82533C XBYTE[0x9300]
#define C825330D XBYTE[0x9000]
#define C825331D XBYTE[0x9100]
#define C825332D XBYTE[0x9200]

#define C82534C XBYTE[0x9b00]
#define C825340D XBYTE[0x9800]
#define C825341D XBYTE[0x9900]
#define C825342D XBYTE[0x9a00]

#define C82535C XBYTE[0xa300]
#define C825350D XBYTE[0xa000]
#define C825351D XBYTE[0xa100]
#define C825352D XBYTE[0xa200]

uchar buf[32];                                                                    //AB1*从8253中读到的计数器数据，15个计数器，低八位高八位，从3-32，1为探头个数，2为测量时间
uchar Channel_Detector[8][2];                                                     //AC1+通道探头选择标志，1:电离室0:计数管
uchar DataGe[75];                                                                 //探测器标定参数的个位，初始时个十百千均为0
uchar DataTenth[75];                                                              //探测器标定参数的十分位
uchar DataCent[75];                                                               //探测器标定参数的百分位
uchar DataThouth[75];                                                             //探测器标定参数的千分位
uchar send_buf[65];                                                               //NIM_A向NIM_B发送数据的数组
uchar Incinput;                                                                   //探头个数标志，默认显示探头个数为8个
float idata Para[10];                                                             //设定好的参数数组
float DoseRata[8];                                                                //每组探头测得的剂量率
uchar Channel_Display[8];															//用于显示的通道量程
float jishuguan_DoseRata;
float dianlishi_DoseRata;
uchar Calculated[8];                                                              //是否计算过标志
uchar Flag_dw;                                                                    //单位标志
uchar Max_Time;                                                                   //AA1+所有通道最长的计数时间
uint Tdata;                                                                 //探头计数率
uint Real_Count[8];
ulong Count[8][3];																//一秒计数历史
uint idata jishuguan_data;                                                        //标定时的计数管计数值相当于Tdata
uint idata dianlishi_data;                                                        //标定时的电离室计数值相当于Tdata
uchar idata Var_Signal1;                                                          //探头控制信号1，接计数管，默认ff均接计数管
uchar idata Var_Signal2;                                                          //探头控制信号2，接电离室，默认00；
uchar idata Var_Signal3;                                                          //探头控制信号3，接电离室，默认00；
uchar idata Var_Led;                                                              //LED报警指示灯,为1时灭,初值0xff,
uchar biaoding_input;
bit Flag_Warn_Count;                                                              //默认0；
uchar Flag_need_Flash[8];                                                         //LED闪烁标志
uchar State_Flash[8];
uchar count_change_flag[8];                                                       //计数改变标志？
float code a0=0.0625;
float code a1=0.0625;
float code a2=0.125;
float code a3=0.25;
float code a4=0.5;
uchar code Svar1[8]={0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80};                   //信号数组
uchar code Svar0[8]={0xFE,0xFD,0xFB,0xF7,0xEF,0xDF,0xBF,0x7F};
extern uchar jishucount;
extern uchar  Average_Times[8];													//AJ1+滑动平均次数
extern uchar Display_Flag[8];

extern bit Speak_Alarm();



void shortdelay(uint i);
void Alarm();
void Led_Disp(uchar Num,uchar Flag,uchar State);
void Updata_Flash(uchar j);
extern bitSpeak_Alarm();
/*************************************
*8253计数值
**************************************/
void GetAndDisdata()                                                              //计数完成后第一个运行的程序
{
  uchar j;
  uchar th=0;
  uchar tl=0;                                                                        //buf数组初始化
  for(j=0;j<10;j++)
  {
    Para[j]=0;
  }
  for(j=0;j<8;j++)
  {
	Calculated[j]=0;
  }
  /*===========锁存8253的计数值==========*/
  C82531C=0x84;                                                                   //锁存第一片8253计数器2的计数值
  tl=C825312D;                                                                    //read825316bitsdata(firstLthenH)
  buf[2]=0xff-tl;
  th=C825312D;
  buf[3]=0xff-th;

  C82531C=0x44;                                                                   //锁存第一片8253计数器1的计数值
  tl=C825311D;                                                                    //read825316bitsdata(firstLthenH)
  buf[4]=0xff-tl;
  th=C825311D;
  buf[5]=0xff-th;

  C82531C=0x04;                                                                   //锁存第一片8253计数器0的计数值
  tl=C825310D;                                                                    //read825316bitsdata(firstLthenH)
  buf[6]=0xff-tl;
  th=C825310D;
  buf[7]=0xff-th;

  C82532C=0x84;                                                                   //锁存第二片8253计数器2的计数值
  tl=C825322D;                                                                    //read825316bitsdata(firstLthenH)
  buf[8]=0xff-tl;
  th=C825322D;
  buf[9]=0xff-th;

  C82532C=0x44;                                                                   //锁存第二片8253计数器1的计数值
  tl=C825321D;                                                                    //read825316bitsdata(firstLthenH)
  buf[10]=0xff-tl;
  th=C825321D;
  buf[11]=0xff-th;

  C82532C=0x04;                                                                   //锁存第二片8253计数器0的计数值
  tl=C825320D;                                                                    //read825316bitsdata(firstLthenH)
  buf[12]=0xff-tl;
  th=C825320D;
  buf[13]=0xff-th;

  C82533C=0x84;                                                                   //锁存第三片8253计数器2的计数值
  tl=C825332D;                                                                    //read825316bitsdata(firstLthenH)
  buf[14]=0xff-tl;
  th=C825332D;
  buf[15]=0xff-th;

  C82533C=0x44;                                                                   //锁存第三片8253计数器1的计数值
  tl=C825331D;                                                                    //read825316bitsdata(firstLthenH)
  buf[16]=0xff-tl;
  th=C825331D;
  buf[17]=0xff-th;

  C82533C=0x04;                                                                   //锁存第三片8253计数器0的计数值
  tl=C825330D;                                                                    //read825316bitsdata(firstLthenH)
  buf[18]=0xff-tl;
  th=C825330D;
  buf[19]=0xff-th;

  C82534C=0x84;                                                                   //锁存第四片8253计数器2的计数值
  tl=C825342D;                                                                    //read825316bitsdata(firstLthenH)
  buf[20]=0xff-tl;
  th=C825342D;
  buf[21]=0xff-th;

  C82534C=0x44;                                                                   //锁存第四片8253计数器1的计数值
  tl=C825341D;                                                                    //read825316bitsdata(firstLthenH)
  buf[22]=0xff-tl;
  th=C825341D;
  buf[23]=0xff-th;

  C82534C=0x04;                                                                   //锁存第四片8253计数器0的计数值
  tl=C825340D;                                                                    //read825316bitsdata(firstLthenH)
  buf[24]=0xff-tl;
  th=C825340D;
  buf[25]=0xff-th;

  C82535C=0x84;                                                                   //锁存第五片8253计数器2的计数值
  tl=C825352D;                                                                    //read825316bitsdata(firstLthenH)
  buf[26]=0xff-tl;
  th=C825352D;
  buf[27]=0xff-th;

  C82535C=0x44;                                                                   //锁存第五片8253计数器1的计数值
  tl=C825351D;                                                                    //read825316bitsdata(firstLthenH)
  buf[28]=0xff-tl;
  th=C825351D;
  buf[29]=0xff-th;

  C82535C=0x04;                                                                   //锁存第五片8253计数器0的计数值
  tl=C825350D;                                                                    //read825316bitsdata(firstLthenH)
  buf[30]=0xff-tl;
  th=C825350D;
  buf[31]=0xff-th;

  buf[0]=Incinput;                                                                //探头个数
  buf[1]=Max_Time;                                                              //测量时间（刷新时间标志）
}
/*******************************************
*8253计数器初始化
********************************************/
void Init_8253()
{
  uchar i;//AB1+
  //=======装入初值再开始计数=======//
  C82531C=0xb4;                                                                   //10110100
  C825312D=0xff;                                                                  //10：通道2,11：先低字节后高字节，010：按方式2工作，0：二进制计数。写初值0xff后开始计数
  C825312D=0xff;

  C82531C=0x74;                                                                   //01110100
  C825311D=0xff;                                                                  //同上
  C825311D=0xff;

  C82531C=0x34;                                                                   //00110100
  C825310D=0xff;                                                                  //写初值先低字节后高字节82531写初值后就开始计数
  C825310D=0xff;                                                                  //二三行用来装入初值

  C82532C=0xb4;                                                                   //10110100
  C825322D=0xff;
  C825322D=0xff;

  C82532C=0x74;                                                                   //01110100
  C825321D=0xff;
  C825321D=0xff;

  C82532C=0x34;                                                                   //00110100
  C825320D=0xff;
  C825320D=0xff;

  C82533C=0xb4;                                                                   //10110100
  C825332D=0xff;
  C825332D=0xff;

  C82533C=0x74;                                                                   //01110100
  C825331D=0xff;
  C825331D=0xff;

  C82533C=0x34;                                                                   //00110100
  C825330D=0xff;
  C825330D=0xff;

  C82534C=0xb4;                                                                   //10110100
  C825342D=0xff;
  C825342D=0xff;

  C82534C=0x74;                                                                   //01110100
  C825341D=0xff;
  C825341D=0xff;

  C82534C=0x34;                                                                   //00110100
  C825340D=0xff;
  C825340D=0xff;

  C82535C=0xb4;                                                                   //10110100
  C825352D=0xff;
  C825352D=0xff;

  C82535C=0x74;                                                                   //01110100
  C825351D=0xff;
  C825351D=0xff;

  C82535C=0x34;                                                                   //00110100
  C825350D=0xff;
  C825350D=0xff;
  for(i=0;i<67;i++)
  {
    DCS_Send[i]=0;
  }
  for(i=0;i<=7;i++)
  {
    DoseRata[i] = 0;						//每组探头测得的剂量率
  }
  Var_Led=0xff;                                                                   //led指示
  Led573=Var_Led;
  EX0=0;                                                                          //关外部中断0
  /*定时计数器的初始化*/                                                                         //AJ1-,移动至定时中断里开Gate,8253开启计数
  Flag8253Cnting=1;
  NumT0=0;
}
/************************************
*显示计数
************************************/
void ShowData()
{
  uchar i,m,j,k;                                                                    //AE1-:Var1用来判断使用的探测器的局部变量，相当于Channel_Detector,不再需要
  ulong count_temp,jishuguan_count_temp,dianlishi_count_temp;
  float idata temp,jtemp,yudecide,jishuguan_rata,jishuguan_jtemp,dianlishi_jtemp;
  double yu,yudata,mtemp;
  uchar Tbcd[12];                                                                 //测得的数据的bcd码数组
  Lcd_Clear();
  /**程序说明:
  **1,为保证即使多个通道不在同一量程，高量程的通道也能有其响应更快的更新速度，程序一秒计数一次，10秒计数为十次一秒计数的相加
  **  ，剂量率显示一秒刷新一次。
  **2,程序中计数率计算、量程换挡、剂量计算显示各部分分开执行
  */
  for(i=0;i<=7;i++)                                                               //8个探头逐个检测
  {
      /**
       * 1.遍历每一个探头（0~7）；
       * 2.先从buf（从8253计数器中取回的计数）中计算计数，高8位*256+低8位，记录历次计数值，每3秒计数一次防止溢出；
       * 3.根据每次的计数（3S计数时长）以及当前量程和预先确定的换挡频率，判断换挡和执行并记录历次量程档位信息；
       * 4.需要根据历史的档位信息进行判断，换挡后的操作与不换挡的操作不同，故分开执行；
       * 5.
       **/
		if(Channel_Detector[i][0]==Channel_Detector[i][1])//当前一次量程和此次量程一样时
		{
			if((Channel_Detector[i][1]==4)&&(Calculated[i]==0))//DL3量程，定时计数3秒，直接用来计算
			{
				Real_Count[i]=0;//此量程每次都要清零
				Count[i][0]=Count[i][1];//历史计数更新
				Count[i][1]=Count[i][2];
				if(i<7)//前6个探头和第七个探头取计数位置不一样
				{
					Count[i][2]=buf[4*i+5]*256+buf[4*i+4];			//加一是因为在计数周期内会由硬件给一个脉冲
					Real_Count[i]=(uint)(buf[4*i+5]*256+buf[4*i+4]);
				}
				else if(i==7)//前6个探头和第七个探头取计数位置不一样
				{
					Count[i][2]=buf[31]*256+buf[30];
					Real_Count[i]=(uint)(buf[31]*256+buf[30]);
				}
				Calculated[i]=1;//计算过标志
				if((Count[i][1]<(37*Refresh_Time))&&(Count[i][2]<(37*Refresh_Time))&&(i<7))//量程切换前7个探头判定条件
				{
					Channel_Detector[i][0]=Channel_Detector[i][1];
					Channel_Detector[i][1]=3;
					Var_Signal1=Var_Signal1|Svar1[i];                                       //控制信号1接计数管，高电平高压关
          			Var_Signal2=Var_Signal2|Svar1[i];                                       //控制信号2接电离室10(8),高电平
          			Var_Signal3=Var_Signal3&Svar0[i];                                       //控制信号2接电离室10(6),低电平
				}
				if((Count[i][1]<(28*Refresh_Time))&&(Count[i][2]<(28*Refresh_Time))&&(i==7))//量程切换第八个探头判定条件
				{
					Channel_Detector[i][0]=Channel_Detector[i][1];
					Channel_Detector[i][1]=3;
					Var_Signal1=Var_Signal1|Svar1[i];                                       //控制信号1接计数管，高电平高压关
          			Var_Signal2=Var_Signal2|Svar1[i];                                       //控制信号2接电离室10(8),高电平
          			Var_Signal3=Var_Signal3&Svar0[i];                                       //控制信号2接电离室10(6),低电平
				}
				Real_Count_Display[i]=(float)((Real_Count[i]*5)/Refresh_Time);
				Channel_Display[i]=4;
				Display_Flag[i]=1;//直接允许显示
			}
			if((Channel_Detector[i][1]==3)&&(Calculated[i]==0))//DL2量程，两次定时计数一秒相加
			{
				Count[i][0]=Count[i][1];
				Count[i][1]=Count[i][2];
				if(i<7)//前6个探头和第七个探头取计数位置不一样
				{
					Count[i][2]=buf[4*i+5]*256+buf[4*i+4];
					Real_Count[i]=(uint)(buf[4*i+5]*256+buf[4*i+4]);
				}
				else if(i==7)//前6个探头和第七个探头取计数位置不一样
				{
					Count[i][2]=buf[31]*256+buf[30];
					Real_Count[i]=(uint)(buf[31]*256+buf[30]);
				}
				Calculated[i]=1;//计算过标志
				if((Count[i][1]>(5719*Refresh_Time))&&(Count[i][2]>(5719*Refresh_Time))&&(i<7))//量程切换
				{
					Channel_Detector[i][0]=Channel_Detector[i][1];
					Channel_Detector[i][1]=4;
					Var_Signal1=Var_Signal1|Svar1[i];                                       //控制信号1接计数管，高电平高压关
          			Var_Signal2=Var_Signal2&Svar0[i];                                       //控制信号2接电离室10(8),高电平
          			Var_Signal3=Var_Signal3|Svar1[i];                                       //控制信号2接电离室10(6),低电平
				}
				if((Count[i][1]>(7447*Refresh_Time))&&(Count[i][2]>(7447*Refresh_Time))&&(i==7))//量程切换
				{
					Channel_Detector[i][0]=Channel_Detector[i][1];
					Channel_Detector[i][1]=4;
					Var_Signal1=Var_Signal1|Svar1[i];                                       //控制信号1接计数管，高电平高压关
          			Var_Signal2=Var_Signal2&Svar0[i];                                       //控制信号2接电离室10(8),高电平
          			Var_Signal3=Var_Signal3|Svar1[i];                                       //控制信号2接电离室10(6),低电平
				}
				if(((Count[i][1]<(35*Refresh_Time))&&(Count[i][2]<(35*Refresh_Time)))&&(i!=7))//第七个探头只有两个量程
				{
					Channel_Detector[i][0]=Channel_Detector[i][1];
					Channel_Detector[i][1]=2;
					Var_Signal1=Var_Signal1|Svar1[i];                                       //控制信号1接计数管，高电平高压关
          			Var_Signal2=Var_Signal2&Svar0[i];                                       //控制信号2接电离室10(8),高电平
          			Var_Signal3=Var_Signal3&Svar0[i];                                       //控制信号2接电离室10(6),低电平
				}
				Real_Count_Display[i]=(float)(Real_Count[i]);
				Real_Count_Display[i]*=5;
				Real_Count_Display[i]/=3;
				Display_Flag[i]=1;//允许显示
				Channel_Display[i]=3;
				Real_Count[i]=0;
				if((Display_Flag[i]==1)&&((Real_Count_Display[i]/5)>5719)&&(Channel_Detector[i][0]==Channel_Detector[i][1])&&(i<7))//量程切换,加后两句是防止重复换挡
				{
					Channel_Detector[i][0]=Channel_Detector[i][1];
					Channel_Detector[i][1]=4;
					Display_Flag[i]=0;
					Var_Signal1=Var_Signal1|Svar1[i];                                       //控制信号1接计数管，高电平高压关
          			Var_Signal2=Var_Signal2&Svar0[i];                                       //控制信号2接电离室10(8),高电平
          			Var_Signal3=Var_Signal3|Svar1[i];                                       //控制信号2接电离室10(6),低电平
				}
				if((Display_Flag[i]==1)&&((Real_Count_Display[i]/5)>7447)&&(Channel_Detector[i][0]==Channel_Detector[i][1])&&(i==7))//量程切换,加后两句是防止重复换挡
				{
					Channel_Detector[i][0]=Channel_Detector[i][1];
					Channel_Detector[i][1]=4;
					Display_Flag[i]=0;
					Var_Signal1=Var_Signal1|Svar1[i];                                       //控制信号1接计数管，高电平高压关
          			Var_Signal2=Var_Signal2&Svar0[i];                                       //控制信号2接电离室10(8),高电平
          			Var_Signal3=Var_Signal3|Svar1[i];                                       //控制信号2接电离室10(6),低电平
				}
				if((Display_Flag[i]==1)&&((Real_Count_Display[i]/5)<35)&&(Channel_Detector[i][0]==Channel_Detector[i][1])&&(i!=7))
				{
					Channel_Detector[i][0]=Channel_Detector[i][1];
					Channel_Detector[i][1]=2;
					Display_Flag[i]=0;
					Var_Signal1=Var_Signal1|Svar1[i];                                       //控制信号1接计数管，高电平高压关
          			Var_Signal2=Var_Signal2&Svar0[i];                                       //控制信号2接电离室10(8),高电平
          			Var_Signal3=Var_Signal3&Svar0[i];                                       //控制信号2接电离室10(6),低电平
				}
			}
			if((Channel_Detector[i][1]==2)&&(Calculated[i]==0))//DL1量程，5次定时计数一秒相加
			{
				Count[i][0]=Count[i][1];
				Count[i][1]=Count[i][2];
				Count[i][2]=buf[4*i+5]*256+buf[4*i+4];
				Real_Count[i]=(uint)(buf[4*i+5]*256+buf[4*i+4]);
				Calculated[i]=1;//计算过标志
				if((Count[i][1]>(5411*Refresh_Time))&&(Count[i][2]>(5411*Refresh_Time)))//量程切换
				{
					Channel_Detector[i][0]=Channel_Detector[i][1];
					Channel_Detector[i][1]=3;
					Var_Signal1=Var_Signal1|Svar1[i];                                       //控制信号1接计数管，高电平高压关
          			Var_Signal2=Var_Signal2|Svar1[i];                                       //控制信号2接电离室10(8),高电平
          			Var_Signal3=Var_Signal3&Svar0[i];                                       //控制信号2接电离室10(6),低电平
				}
				if((Count[i][1]<(158*Refresh_Time))&&(Count[i][2]<(158*Refresh_Time)))
				{
					Channel_Detector[i][0]=Channel_Detector[i][1];
					Channel_Detector[i][1]=1;
					Var_Signal1=Var_Signal1&Svar0[i];                                       //控制信号1接计数管，低电平高压开
          			Var_Signal2=Var_Signal2&Svar0[i];                                       //控制信号2接电离室10(8),低电平
          			Var_Signal3=Var_Signal3&Svar0[i];                                       //控制信号2接电离室10(6),低电平
				}
					Real_Count_Display[i]=(float)(Real_Count[i]);
					Real_Count_Display[i]*=5;//这里是CP5S
					Real_Count_Display[i]/=Refresh_Time;
					Display_Flag[i]=1;//允许显示
					Channel_Display[i]=2;
					Real_Count[i]=0;
					if((Display_Flag[i]==1)&&((Real_Count_Display[i]/5)>5411)&&(Channel_Detector[i][0]==Channel_Detector[i][1]))//量程切换
					{
						Channel_Detector[i][0]=Channel_Detector[i][1];
						Channel_Detector[i][1]=3;
						Display_Flag[i]=0;
						Var_Signal1=Var_Signal1|Svar1[i];                                       //控制信号1接计数管，高电平高压关
          				Var_Signal2=Var_Signal2|Svar1[i];                                       //控制信号2接电离室10(8),高电平
          				Var_Signal3=Var_Signal3&Svar0[i];                                       //控制信号2接电离室10(6),低电平
					}
					if((Display_Flag[i]==1)&&((Real_Count_Display[i]/5)<158)&&(Channel_Detector[i][0]==Channel_Detector[i][1]))
					{
						Channel_Detector[i][0]=Channel_Detector[i][1];
						Channel_Detector[i][1]=1;
						Display_Flag[i]=0;
						Var_Signal1=Var_Signal1&Svar0[i];                                       //控制信号1接计数管，低电平高压开
          				Var_Signal2=Var_Signal2&Svar0[i];                                       //控制信号2接电离室10(8),低电平
          				Var_Signal3=Var_Signal3&Svar0[i];                                       //控制信号2接电离室10(6),低电平
					}
			}
			if((Channel_Detector[i][1]==1)&&(Calculated[i]==0))//GM2量程，10次定时计数一秒相加，5次平滑平均
			{
				Count[i][0]=Count[i][1];
				Count[i][1]=Count[i][2];
				Count[i][2]=buf[4*i+3]*256+buf[4*i+2];
				Real_Count[i]=(uint)(buf[4*i+3]*256+buf[4*i+2]);
				Calculated[i]=1;//计算过标志
				if((Count[i][0]>(6019*Refresh_Time))&&(Count[i][1]>(6019*Refresh_Time))&&(Count[i][2]>(6019*Refresh_Time)))//量程切换
				{
					Channel_Detector[i][0]=Channel_Detector[i][1];
					Channel_Detector[i][1]=2;
					Var_Signal1=Var_Signal1|Svar1[i];                                       //控制信号1接计数管，高电平高压关
          			Var_Signal2=Var_Signal2&Svar0[i];                                       //控制信号2接电离室10(8),低电平
          			Var_Signal3=Var_Signal3&Svar0[i];                                       //控制信号2接电离室10(6),低电平
				}
				if((Count[i][0]<(1.32*Refresh_Time))&&(Count[i][1]<(1.32*Refresh_Time))&&(Count[i][2]<(1.32*Refresh_Time)))
				{
					Channel_Detector[i][0]=Channel_Detector[i][1];
					Channel_Detector[i][1]=0;
					Var_Signal1=Var_Signal1&Svar0[i];                                       //控制信号1接计数管，低电平高压开
          			Var_Signal2=Var_Signal2&Svar0[i];                                       //控制信号2接电离室10(8),低电平
          			Var_Signal3=Var_Signal3&Svar0[i];                                       //控制信号2接电离室10(6),低电平
				}
				Real_Count[i]*=20;//换算为CPM(注意对应的是3秒的 Refresh Time)
				Average_Counts[i][Average_Times[i]]=Real_Count[i];
				Average_Times[i]+=1;
				if(Average_Times[i]<5)
				{
					switch(Average_Times[i])
					{
						case 1: Real_Count_Display[i]=Average_Counts[i][0]; break;
						case 2: Real_Count_Display[i]=Average_Counts[i][0]*0.4+Average_Counts[i][1]*0.6; break;
						case 3: Real_Count_Display[i]=Average_Counts[i][0]*a3+Average_Counts[i][1]*a3+Average_Counts[i][2]*a4; break;
						case 4: Real_Count_Display[i]=Average_Counts[i][0]*a2+Average_Counts[i][1]*a2+Average_Counts[i][2]*a3+Average_Counts[i][3]*a4; break;
					}
					Display_Flag[i]=1;
					Channel_Display[i]=0;
				}
				if(Average_Times[i]==5)
				{
					Real_Count_Display[i]=Average_Counts[i][0]*a0+Average_Counts[i][1]*a1+Average_Counts[i][2]*a2+Average_Counts[i][3]*a3+Average_Counts[i][4]*a4;
					for(j=0;j<4;j++)
					{
						Average_Counts[i][j]=Average_Counts[i][j+1];
					}
					Average_Times[i]-=1;//为了让次数
					Display_Flag[i]=1;
					Channel_Display[i]=1;
				}
				//if(Count_Times[i]==2)
				//{
					//Average_Counts[i][Average_Times[i]]=Real_Count[i];//将计数率赋值给平均值
					//Average_Times[i]+=1;//平滑平均次数加1
					//Count_Times[i]=0;
					//Real_Count[i]=0;
				//}
				//if((Average_Times[i]>=2)&&(Average_Times[i]<=5))//在有2个计数值后开始算平均值并显示
				//{
					//for(j=0;j<Average_Times[i];j++)
					//{
						//Real_Count_Display[i]+=(float)(Average_Counts[i][j]);
					//}
					//Real_Count_Display[i]/=(float)(Average_Times[i]);
					//Real_Count_Display[i]*=(60/(Refresh_Time*2));//因为要换算成CPM
					//Display_Flag[i]=1;//允许显示
					//Channel_Display[i]=0;
					//if(Average_Times[i]==5)
					//{
						//for(j=0;j<(Average_Times[i]-1);j++)
						//{
							//Average_Counts[i][j]=Average_Counts[i][j+1];
						//}
						//Average_Times[i]-=1;//为了让次数
					//}
				//}
				//if((Display_Flag[i]==1)&&((Real_Count_Display[i]/60)>6019)&&(Channel_Detector[i][0]==Channel_Detector[i][1]))//量程切换
				//{
					//Channel_Detector[i][0]=Channel_Detector[i][1];
					//Channel_Detector[i][1]=2;
					//Display_Flag[i]=0;
					//Var_Signal1=Var_Signal1&Svar0[i];                                       //控制信号1接计数管，低电平
          			//Var_Signal2=Var_Signal2&Svar0[i];                                       //控制信号2接电离室10(8),低电平
          			//Var_Signal3=Var_Signal3&Svar0[i];                                       //控制信号2接电离室10(6),低电平
				//}
				//if((Display_Flag[i]==1)&&((Real_Count_Display[i]/60)<1.32)&&(Channel_Detector[i][0]==Channel_Detector[i][1]))
				//{
					//Channel_Detector[i][0]=Channel_Detector[i][1];
					//Channel_Detector[i][1]=0;
					//Display_Flag[i]=0;
					//Var_Signal1=Var_Signal1|Svar1[i];                                       //控制信号1接计数管，为高电平
          			//Var_Signal2=Var_Signal2&Svar0[i];                                       //控制信号2接电离室10(8),低电平
          			//Var_Signal3=Var_Signal3&Svar0[i];                                       //控制信号2接电离室10(6),低电平
				//}
			}
			if((Channel_Detector[i][1]==0)&&(Calculated[i]==0))//GM1量程，2次定时计数3秒相加，5次平滑平均
			{
				Count[i][0]=Count[i][1];
				Count[i][1]=Count[i][2];
				Count[i][2]=buf[4*i+3]*256+buf[4*i+2];
				Real_Count[i]=(uint)(buf[4*i+3]*256+buf[4*i+2]);
				Calculated[i]=1;//计算过标志
				if((Count[i][0]>(1.32*Refresh_Time))&&(Count[i][1]>(1.32*Refresh_Time))&&(Count[i][2]>(1.32*Refresh_Time)))//量程切换
				{
					Channel_Detector[i][0]=Channel_Detector[i][1];
					Channel_Detector[i][1]=1;
					Var_Signal1=Var_Signal1&Svar0[i];                                       //控制信号1接计数管，低电平高压开
          			Var_Signal2=Var_Signal2&Svar0[i];                                       //控制信号2接电离室10(8),低电平
          			Var_Signal3=Var_Signal3&Svar0[i];                                       //控制信号2接电离室10(6),低电平
				}
					Real_Count[i]*=20;//换算为CPM(注意对应的是3秒的 Refresh Time)
					Average_Counts[i][Average_Times[i]]=Real_Count[i];
					Average_Times[i]+=1;
					if(Average_Times[i]<5)
					{
						switch(Average_Times[i])
						{
							case 1: Real_Count_Display[i]=Average_Counts[i][0]; break;
							case 2: Real_Count_Display[i]=Average_Counts[i][0]*0.4+Average_Counts[i][1]*0.6; break;
							case 3: Real_Count_Display[i]=Average_Counts[i][0]*a3+Average_Counts[i][1]*a3+Average_Counts[i][2]*a4; break;
							case 4: Real_Count_Display[i]=Average_Counts[i][0]*a2+Average_Counts[i][1]*a2+Average_Counts[i][2]*a3+Average_Counts[i][3]*a4; break;
						}
						Display_Flag[i]=1;
						Channel_Display[i]=0;
					}
					else if(Average_Times[i]==5)
					{
						Real_Count_Display[i]=Average_Counts[i][0]*a0+Average_Counts[i][1]*a1+Average_Counts[i][2]*a2+Average_Counts[i][3]*a3+Average_Counts[i][4]*a4;
						for(j=0;j<4;j++)
						{
							Average_Counts[i][j]=Average_Counts[i][j+1];
						}
						Average_Times[i]-=1;//为了让次数
						Display_Flag[i]=1;
						Channel_Display[i]=0;
					}
					//if(Count_Times[i]==2)
					//{
					//	Average_Counts[i][Average_Times[i]]=Real_Count[i];//将计数率赋值给平均值
					//	Average_Times[i]+=1;//平滑平均次数加1
					//	Count_Times[i]=0;
					//	Real_Count[i]=0;
					//}
					//if((Average_Times[i]>=2)&&(Average_Times[i]<=5))//在有2个计数值后开始算平均值并显示
					//{
						//for(j=0;j<Average_Times[i];j++)
						//{
							//Real_Count_Display[i]+=(float)(Average_Counts[i][j]);
						//}
						//Real_Count_Display[i]/=(float)(Average_Times[i]);
						//Real_Count_Display[i]*=(60/(Refresh_Time*2));//因为要换算成CPM
						//Display_Flag[i]=1;//允许显示
						//Channel_Display[i]=0;
						//if(Average_Times[i]==5)
						//{
							//for(j=0;j<(Average_Times[i]-1);j++)
							//{
								//Average_Counts[i][j]=Average_Counts[i][j+1];
							//}
							//Average_Times[i]-=1;//为了让次数
						//}
					//}
					//if((Display_Flag[i]==1)&&((Real_Count_Display[i]/60)>1.32)&&(Channel_Detector[i][0]==Channel_Detector[i][1]))//量程切换
					//{
						//Channel_Detector[i][0]=Channel_Detector[i][1];
						//Channel_Detector[i][1]=1;
						//Display_Flag[i]=0;
						//Var_Signal1=Var_Signal1|Svar1[i];                                       //控制信号1接计数管，为高电平
          				//Var_Signal2=Var_Signal2&Svar0[i];                                       //控制信号2接电离室10(8),低电平
          				//Var_Signal3=Var_Signal3&Svar0[i];                                       //控制信号2接电离室10(6),低电平
					//}
			}
		}
		else if(Channel_Detector[i][0]!=Channel_Detector[i][1])//当前后两次计数的量程不一样
		{
			if((Channel_Detector[i][1]==4)&&(Calculated[i]==0))//DL3量程，定时计数一秒，直接用来计算
			{
				for(k=0;k<=2;k++)//初始化单次跳档判断变量，防止意外降档
				{
					Count[i][k]=1000*Refresh_Time;//一秒计数使其不换档
				}
				Real_Count[i]=0;//跳转到此量程需要清零
				if(i<7)//前6个探头和第七个探头取计数位置不一样
				{
					Count[i][2]=buf[4*i+5]*256+buf[4*i+4];
					Real_Count[i]=(uint)(buf[4*i+5]*256+buf[4*i+4]);
				}
				else if(i==7)//前6个探头和第七个探头取计数位置不一样
				{
					Count[i][2]=buf[31]*256+buf[30];
					Real_Count[i]=(uint)(buf[31]*256+buf[30]);
				}
				Calculated[i]=1;//计算过标志
				Real_Count_Display[i]=(float)((Real_Count[i]*5)/Refresh_Time);
				Display_Flag[i]=1;//直接允许显示
				Channel_Display[i]=4;
				Channel_Detector[i][0]=Channel_Detector[i][1];//更新量程历史状态
			}
			if((Channel_Detector[i][1]==3)&&(Calculated[i]==0))//DL2量程，两次定时计数一秒相加
			{
				for(k=0;k<=2;k++)
				{
					Count[i][k]=2000*Refresh_Time;//一秒计数使其不换档
				}
				Real_Count[i]=0;//跳转量程后需要清零
				Display_Flag[i]=0;//显示及换挡标志清零
				if(i<7)//前6个探头和第七个探头取计数位置不一样
				{
					Count[i][2]=buf[4*i+5]*256+buf[4*i+4];
					Real_Count[i]=(uint)(buf[4*i+5]*256+buf[4*i+4]);
				}
				else if(i==7)//前6个探头和第七个探头取计数位置不一样
				{
					Count[i][2]=buf[31]*256+buf[30];
					Real_Count[i]=(uint)(buf[31]*256+buf[30]);
				}
				Calculated[i]=1;//计算过标志
				Channel_Detector[i][0]=Channel_Detector[i][1];//更新量程历史状态
			}
			if((Channel_Detector[i][1]==2)&&(Calculated[i]==0))//DL1量程，5次定时计数一秒相加
			{
				for(k=0;k<=2;k++)
				{
					Count[i][k]=2000*Refresh_Time;//一秒计数使其不换档
				}
				Real_Count[i]=0;//跳转量程后需要清零
				Display_Flag[i]=0;//显示及换挡标志清零
				if(i<7)//前6个探头和第七个探头取计数位置不一样
				{
					Count[i][2]=buf[4*i+5]*256+buf[4*i+4];
					Real_Count[i]=(uint)(buf[4*i+5]*256+buf[4*i+4]);
				}
				else if(i==7)//前6个探头和第七个探头取计数位置不一样
				{
					Count[i][2]=buf[31]*256+buf[30];
					Real_Count[i]=(uint)(buf[31]*256+buf[30]);
					Channel_Detector[i][1]=3;
					Channel_Detector[i][0]=3;
				}

				Calculated[i]=1;//计算过标志
				Channel_Detector[i][0]=Channel_Detector[i][1];//更新量程历史状态
			}
			if((Channel_Detector[i][1]==1)&&(Calculated[i]==0))//GM2量程，10次定时计数一秒相加，5次平滑平均
			{
				for(k=0;k<=2;k++)
				{
					Count[i][k]=1000*Refresh_Time;//一秒计数使其不换档
				}
				Real_Count[i]=0;//跳转量程后需要清零
				Display_Flag[i]=0;//显示及换挡标志清零
				Average_Times[i]=0;//平滑平均次数清零
				for(j=0;j<10;j++)
				{
					Average_Counts[i][j]=0;//平滑平均清零
				}
				Count[i][2]=buf[4*i+3]*256+buf[4*i+2];
				Real_Count[i]=(uint)(buf[4*i+3]*256+buf[4*i+2]);
				
				Real_Count[i]*=20;//换算为CPM(注意对应的是3秒的 Refresh Time)
				Average_Counts[i][Average_Times[i]]=Real_Count[i];
				Average_Times[i]+=1;
				Real_Count_Display[i]=Average_Counts[i][Average_Times[i]];
				Display_Flag[i]=1;
				Channel_Display[i]=1;
				Calculated[i]=1;//计算过标志
				Channel_Detector[i][0]=Channel_Detector[i][1];//更新量程历史状态
			}
			if((Channel_Detector[i][1]==0)&&(Calculated[i]==0))//GM2量程，10次定时计数一秒相加，5次平滑平均
			{
				for(k=0;k<=2;k++)
				{
					Count[i][k]=1*Refresh_Time;//一秒计数使其不换档
				}
				Real_Count[i]=0;//跳转量程后需要清零
				Display_Flag[i]=0;//显示及换挡标志清零
				Average_Times[i]=0;//平滑平均次数清零
				for(j=0;j<10;j++)
				{
					Average_Counts[i][j]=0;//平滑平均清零
				}
				Count[i][2]=buf[4*i+3]*256+buf[4*i+2];
				Real_Count[i]=(uint)(buf[4*i+3]*256+buf[4*i+2]);
				Real_Count[i]*=20;//换算为CPM(注意对应的是3秒的 Refresh Time)
				Average_Counts[i][Average_Times[i]]=Real_Count[i];
				Average_Times[i]+=1;
				Real_Count_Display[i]=Average_Counts[i][Average_Times[i]];
				Display_Flag[i]=1;
				Channel_Display[i]=0;
				Calculated[i]=1;//计算过标志
				Channel_Detector[i][0]=Channel_Detector[i][1];//更新量程历史状态
			}
		}
  }
  /*将根据剂量率值重新设置的探头信号改变标志执行*/
  Signal1=Var_Signal1;//探头信号转变
  Signal2=Var_Signal3;//2016-8-26调换Var_Signal2,Var_Signal3，因为两者在硬件上被弄混了
  Signal3=Var_Signal2;
  /*AC1+在这里buf(十六位高低计数数据)要转换成uint 类型的Tdata数据,并且探头数目要清楚
  加入选探头标志*/
  if(measure_flag)                                                                //(1)measure_flag在Init_RAM中初始化变为1
  {
    for(j=0;j<8;j++)                                                       //(2)存计数区为buf[3]-buf[32];探头个数标志
    {
      Tdata=(uint)(Count[j][2]/3);                                                        //探头计数率uint ,紫色为全局变量，蓝色为局部变量
      jtemp=0;                                                                    //float
      yudata=0;                                                                   //double
      mtemp=0;                                                                    //double
      yu=0;                                                                       //double
      Tbcd[5]=0;                                                                  //uchar 数组共12位，测得的数据的bcd码数组
      Tbcd[4]=0;
      Tbcd[3]=0;
      Tbcd[2]=0;
      Tbcd[1]=0;
      Tbcd[0]=0;
      m=j*10;                                                                     //保留
      /*********判断计数率***********/
      /*********前七个探头探测器设置，取得8253计数值记录在Tdata*/
      /**【2024】根据通道序号计算标定参数，用于CPM和剂量率的转换**/
	  if(j<7)                                                                     //(3)前七个探头既有电离室也有计数器，第八个只有电离室故分开处理
      {
      		Display_Flag[j]=0;
			if(Channel_Display[j]==0)
			{
				Para[0]=(float)(DataThouth[m+1]*1000+DataCent[m+1]*100+DataTenth[m+1]*10+DataGe[m+1]);//调节参数
				Para[1]=(float)((float)(DataThouth[m+2]*1000+DataCent[m+2]*100+DataTenth[m+2]*10+DataGe[m+2])/1000000);
				DoseRata[j]=(float)(Para[1]*((uint)Real_Count_Display[j]-Para[0]));
			}
			else if(Channel_Display[j]==1)
			{
				Para[2]=(float)(DataThouth[m+3]*1000+DataCent[m+3]*100+DataTenth[m+3]*10+DataGe[m+3]);//调节参数
        		Para[3]=(float)((float)(DataThouth[m+4]*1000+DataCent[m+4]*100+DataTenth[m+4]*10+DataGe[m+4])/1000000);
				DoseRata[j]=(float)(Para[3]*((uint)Real_Count_Display[j]-Para[2]));
			}
			else if(Channel_Display[j]==2)
			{
				Para[4]=(float)(DataThouth[m+5]*1000+DataCent[m+5]*100+DataTenth[m+5]*10+DataGe[m+5]);//调节参数
        		Para[5]=(float)((float)(DataThouth[m+6]*1000+DataCent[m+6]*100+DataTenth[m+6]*10+DataGe[m+6])/1000);
				DoseRata[j]=(float)(Para[5]*((uint)Real_Count_Display[j]-Para[4]));
			}
			else if(Channel_Display[j]==3)
			{
				Para[6]=(float)(DataThouth[m+7]*1000+DataCent[m+7]*100+DataTenth[m+7]*10+DataGe[m+7]);//调节参数
        		Para[7]=(float)((float)(DataThouth[m+8]*1000+DataCent[m+8]*100+DataTenth[m+8]*10+DataGe[m+8])/10);
				DoseRata[j]=(float)(Para[7]*((uint)Real_Count_Display[j]-Para[6]));
			}
			else if(Channel_Display[j]==4)
			{
				Para[8]=(float)(DataThouth[m+9]*1000+DataCent[m+9]*100+DataTenth[m+9]*10+DataGe[m+9]);//调节参数
        		Para[9]=(float)((float)((DataThouth[m+10]*1000+DataCent[m+10]*100+DataTenth[m+10]*10+DataGe[m+10])*10));
				DoseRata[j]=(float)(Para[9]*((uint)Real_Count_Display[j]-Para[8]));
			}
		}
		else if(j==7)
		{
			Display_Flag[j]=0;
			//Txtext(10,13+(j*58),"Dh");
			if(Channel_Display[j]==3)
			{
				Para[0]=(float)(DataThouth[m+1]*1000+DataCent[m+1]*100+DataTenth[m+1]*10+DataGe[m+1]);//调节参数
				Para[1]=(float)((float)(DataThouth[m+2]*1000+DataCent[m+2]*100+DataTenth[m+2]*10+DataGe[m+2])/10);
				DoseRata[j]=(float)(Para[1]*((uint)Real_Count_Display[j]-Para[0]));
			}
			else if(Channel_Display[j]==4)
			{
				Para[2]=(float)(DataThouth[m+3]*1000+DataCent[m+3]*100+DataTenth[m+3]*10+DataGe[m+3]);//调节参数
				Para[3]=(float)((float)(DataThouth[m+4]*1000+DataCent[m+4]*100+DataTenth[m+4]*10+DataGe[m+4])*10);
				DoseRata[j]=(float)(Para[3]*((uint)Real_Count_Display[j]-Para[2]));
			}
		}
      /**********单位换算后为jtemp************/
      if(DoseRata[j]<0)                                                         //修改2012.6.15通过均值计算
      {
        jtemp=0.0;
        Flag_dw=1;
      }
      else if(DoseRata[j]>=0)                                                   //单位为uGy/h
      {
        if(DoseRata[j]<1000)                                                    //Mean_Value<1000表示单位为uGy/h
        {
          jtemp=(float)DoseRata[j];
          Flag_dw=1;
        }
        else if((DoseRata[j]>=1000)&&(DoseRata[j]<1000000))                   //1000<Mean_Value<1000000表示mGy/h
        {
          jtemp=(float)(DoseRata[j]/1000);
          Flag_dw=2;
        }
        else if(DoseRata[j]>=1000000)                                           //Mean_Value>=1000000表示Gy/h
        {
          jtemp=(float)(DoseRata[j]/1000000);
          Flag_dw=3;
        }
      }
      /*****DCS发送数据准备****/
      if(DoseRata[j]<0)                                                         //若计算剂量小于零向DCS发送0
      {
        DCS_Send[j*4+4]=0x30;
        DCS_Send[j*4+5]=0x30;
        DCS_Send[j*4+6]=0x30;
        DCS_Send[j*4+7]=0x30;
      }
      else if(DoseRata[j]>=0)
      {
        if(DoseRata[j]<1)
        {
          temp=(float)DoseRata[j]*1000;
          DCS_Send[j*4+4]=(uchar)((int)temp/100)+0x30;
          DCS_Send[j*4+5]=(uchar)((int)temp%100/10)+0x30;
          DCS_Send[j*4+6]=(uchar)((int)temp%100%10)+0x30;
          DCS_Send[j*4+7]=0x30;
        }
        else if((DoseRata[j]>=1)&&(DoseRata[j]<10))
        {
          temp=(float)DoseRata[j]*100;
          DCS_Send[j*4+4]=(uchar)((int)temp/100)+0x30;
          DCS_Send[j*4+5]=(uchar)((int)temp%100/10)+0x30;
          DCS_Send[j*4+6]=(uchar)((int)temp%100%10)+0x30;
          DCS_Send[j*4+7]=0x31;
        }
        else if((DoseRata[j]>=10)&&(DoseRata[j]<100))
        {
          temp=(float)DoseRata[j]*10;
          DCS_Send[j*4+4]=(uchar)((int)temp/100)+0x30;
          DCS_Send[j*4+5]=(uchar)((int)temp%100/10)+0x30;
          DCS_Send[j*4+6]=(uchar)((int)temp%100%10)+0x30;
          DCS_Send[j*4+7]=0x32;
        }
        else if((DoseRata[j]>=100)&&(DoseRata[j]<1000))
        {
          temp=(float)DoseRata[j];
          DCS_Send[j*4+4]=(uchar)((int)temp/100)+0x30;
          DCS_Send[j*4+5]=(uchar)((int)temp%100/10)+0x30;
          DCS_Send[j*4+6]=(uchar)((int)temp%100%10)+0x30;
          DCS_Send[j*4+7]=0x33;
        }
        else if((DoseRata[j]>=1000)&&(DoseRata[j]<10000))
        {
          temp=(float)DoseRata[j]/10.0;
          DCS_Send[j*4+4]=(uchar)((int)temp/100)+0x30;
          DCS_Send[j*4+5]=(uchar)((int)temp%100/10)+0x30;
          DCS_Send[j*4+6]=(uchar)((int)temp%100%10)+0x30;
          DCS_Send[j*4+7]=0x34;
        }
        else if((DoseRata[j]>=10000)&&(DoseRata[j]<100000))
        {
          temp=(float)DoseRata[j]/100.0;
          DCS_Send[j*4+4]=(uchar)((int)temp/100)+0x30;
          DCS_Send[j*4+5]=(uchar)((int)temp%100/10)+0x30;
          DCS_Send[j*4+6]=(uchar)((int)temp%100%10)+0x30;
          DCS_Send[j*4+7]=0x35;
        }
        else if((DoseRata[j]>=100000)&&(DoseRata[j]<1000000))
        {
          temp=(float)DoseRata[j]/1000.0;
          DCS_Send[j*4+4]=(uchar)((int)temp/100)+0x30;
          DCS_Send[j*4+5]=(uchar)((int)temp%100/10)+0x30;
          DCS_Send[j*4+6]=(uchar)((int)temp%100%10)+0x30;
          DCS_Send[j*4+7]=0x36;
        }
        else if((DoseRata[j]>=1000000)&&(DoseRata[j]<10000000))
        {
          temp=(float)DoseRata[j]/10000.0;
          DCS_Send[j*4+4]=(uchar)((int)temp/100)+0x30;
          DCS_Send[j*4+5]=(uchar)((int)temp%100/10)+0x30;
          DCS_Send[j*4+6]=(uchar)((int)temp%100%10)+0x30;
          DCS_Send[j*4+7]=0x37;
        }
        else if((DoseRata[j]>=10000000)&&(DoseRata[j]<100000000))
        {
          temp=(float)DoseRata[j]/100000.0;
          DCS_Send[j*4+4]=(uchar)((int)temp/100)+0x30;
          DCS_Send[j*4+5]=(uchar)((int)temp%100/10)+0x30;
          DCS_Send[j*4+6]=(uchar)((int)temp%100%10)+0x30;
          DCS_Send[j*4+7]=0x38;
        }
        else if((DoseRata[j]>=100000000)&&(DoseRata[j]<1000000000))
        {
          temp=(float)DoseRata[j]/1000000.0;
          DCS_Send[j*4+4]=(uchar)((int)temp/100)+0x30;
          DCS_Send[j*4+5]=(uchar)((int)temp%100/10)+0x30;
          DCS_Send[j*4+6]=(uchar)((int)temp%100%10)+0x30;
          DCS_Send[j*4+7]=0x39;
        }
      }
      /***********剂量率转换单位后转换成可以显示的BCD码*************/
      if((Tbcd[5]<=9)&&(Tbcd[4]<=9)&&(Tbcd[3]<=9)&&(Tbcd[2]<=9)&&(Tbcd[1]<=9)&&(Tbcd[0]<=9))//4.21似乎多余，可不可以修改掉
      {
        Tbcd[5]=(uchar)(((int)jtemp)/100);                                         //百位
        Tbcd[4]=(uchar)(((int)jtemp)%100/10);                                      //十位
        Tbcd[3]=(uchar)(((int)jtemp)%100%10);                                      //个位
        count_temp=(jtemp-(int)jtemp)*1000;                                         //计算小于1的部分，把小数部分变大，保留三位小数
        Tbcd[2]=(uchar)(count_temp/100);                                           //小小数十分位
        Tbcd[1]=(uchar)(count_temp%100/10);                                        //小数百分位
        Tbcd[0]=(uchar)(count_temp%100%10);                                        //小数千分位
        if(Flag_dw==1)                                                              //根据Tbcd[]计算yudecide(float)用来判断剂量率是否超过阈值
          yudecide=(float)(Tbcd[5]*100+Tbcd[4]*10+Tbcd[3]+Tbcd[2]*0.1+Tbcd[1]*0.01+Tbcd[0]*0.001);
        if(Flag_dw==2)
          yudecide=(float)((Tbcd[5]*100+Tbcd[4]*10+Tbcd[3]+Tbcd[2]*0.1+Tbcd[1]*0.01+Tbcd[0]*0.001)*1000);
        if(Flag_dw==3)
          yudecide=(float)((Tbcd[5]*100+Tbcd[4]*10+Tbcd[3]+Tbcd[2]*0.1+Tbcd[1]*0.01+Tbcd[0]*0.001)*1000000);
      }
      /***********判断阈值*************/
      mtemp=(YuThouth[j]+YuCent[j]*0.1+YuTenth[j]*0.01);                          //暂时没根据位数修改2012.6.15
      yu=(double)(pow(10,YuGe[j]));
      yudata=(double)(mtemp*yu);
      if(yudecide>=yudata)                                                        //如果计数超过某一个范围，则声音报警
      {
        if((Judge_Speak==0)||(Speak_Alarm()==1))                                  //蜂鸣器报警，Speak_Alarm返回1蜂鸣器工作
        {
          Flag_Warn=1;                                                            //报警标志置一
        }
        Flag_need_Flash[j]=1;                                                     //LED闪烁标志置1
        State_Flash[j]=1;
        Var_Led=Var_Led&Svar0[j];                                                 //LED报警指示灯（char）Svar0[8]={0xFE,0xFD,0xFB,0xF7,0xEF,0xDF,0xBF,0x7F}
      }
      else if(yudecide<yudata)
      {
        Flag_need_Flash[j]=0;
        State_Flash[j]=0;
        Var_Led=Var_Led|Svar1[j];
      }
      /*AI1*剂量率显示应当在探测器标志改变之前******剂量率显示**********/
	  //if(Channel_Detector[j]==0)
	  //{
      //	Txtext(10,13+(j*58),"Gh");                                                  //AH1显示探头
	  //}
	  //else
	  //{
		//Txtext(10,13+(j*58),"Dh");
	  //}
	  /*AI1*******剂量率显示**********/
	  if(Flag_need_Flash[j]==0)
	  {
	  	if((Channel_Display[j]==0)||(Channel_Display[j]==1))
		{
			Txtext(10,13+(j*58),"GM");												  //AH1显示探头
		}
		else
		{
			Txtext(10,13+(j*58),"DL");
		}
      	Tnumber(60,13+(j*58),j+1);                                                  //显示探头号
      	Txtext(90,13+(j*58),":");                                                   //显示冒号
	  	Tnumber(470,13+(j*58),Channel_Detector[j][1]);
	  	Tnumber(440,13+(j*58),Channel_Detector[j][0]);
	  	Tnumber(390,13+(j*58),Average_Times[j]);
      	if(Tbcd[5]!=0)                                                              //若百位非零，根据数的大小来显示
      	{
        	Tnumber(130,13+(j*58),Tbcd[5]);
        	Tnumber(160,13+(j*58),Tbcd[4]);
        	Tnumber(190,13+(j*58),Tbcd[3]);
        	Txtext(220,13+(j*58),".");
        	Tnumber(240,13+(j*58),Tbcd[2]);
        	Tnumber(270,13+(j*58),Tbcd[1]);
        	Tnumber(300,13+(j*58),Tbcd[0]);
        	if(Tdata<10)                                                              //用于显示Tdata锁存的计数值显示
        	Tnumber(576,13+(j*58),Tdata);                                             //测试完成后去掉2012.6.24
        	else if((Tdata>=10)&&(Tdata<100))
        	Tnumber(556,13+(j*58),Tdata);
        	else if((Tdata>=100)&&(Tdata<1000))
        	Tnumber(536,13+(j*58),Tdata);
        	else if((Tdata>=1000)&&(Tdata<10000))
        	Tnumber(516,13+(j*58),Tdata);
        	else if(Tdata>=10000)
        	Tnumber(496,13+(j*58),Tdata);
      	}
      	else
      	{
        	if(Tbcd[4]!=0)                                                            //百位为零十位非零
        	{
          	Tnumber(160,13+(j*58),Tbcd[4]);
          	Tnumber(190,13+(j*58),Tbcd[3]);
          	Txtext(220,13+(j*58),".");
          	Tnumber(240,13+(j*58),Tbcd[2]);
          	Tnumber(270,13+(j*58),Tbcd[1]);
          	Tnumber(300,13+(j*58),Tbcd[0]);
          	if(Tdata<10)
            	Tnumber(576,13+(j*58),Tdata);                                         //测试完成后去掉2012.6.24
          	else if((Tdata>=10)&&(Tdata<100))
            	Tnumber(556,13+(j*58),Tdata);
          	else if((Tdata>=100)&&(Tdata<1000))
            	Tnumber(536,13+(j*58),Tdata);
          	else if((Tdata>=1000)&&(Tdata<10000))
            	Tnumber(516,13+(j*58),Tdata);
          	else if(Tdata>=10000)
            	Tnumber(496,13+(j*58),Tdata);                                         //测试完成后去掉2012.6.24
        	}
        	else                                                                      //百位十位位均为零，个位非零
        	{
          	Tnumber(190,13+(j*58),Tbcd[3]);
          	Txtext(220,13+(j*58),".");                                              //.占20个像素
          	Tnumber(240,13+(j*58),Tbcd[2]);                                         //一个数字占30个像素
          	Tnumber(270,13+(j*58),Tbcd[1]);
          	Tnumber(300,13+(j*58),Tbcd[0]);
          	if(Tdata<10)
            	Tnumber(576,13+(j*58),Tdata);                                         //测试完成后去掉2012.6.24
          	else if((Tdata>=10)&&(Tdata<100))                                       //可以看见一个数字占20像素
            	Tnumber(556,13+(j*58),Tdata);
          	else if((Tdata>=100)&&(Tdata<1000))
            	Tnumber(536,13+(j*58),Tdata);
          	else if((Tdata>=1000)&&(Tdata<10000))
            	Tnumber(516,13+(j*58),Tdata);
          	else if(Tdata>=10000)
            	Tnumber(496,13+(j*58),Tdata);
        	}
	  	}
      	/**********计量单位显示***********/
      	if(Flag_dw==1)                                                              //测试完成后加上2012.6.24
      	{
        	Txtext(340,13+(j*58),"u");
      	}
      	else if(Flag_dw==2)
      	{
        	Txtext(340,13+(j*58),"m");
      	}
      	else if(Flag_dw==3)
      	{
        	Txtext(340,13+(j*58),"G");
      	}
      	Alarm();                                                                    //报警
	  	}
	  if(Flag_need_Flash[j]==1)
	  {
		if((Channel_Display[j]==0)||(Channel_Display[j]==1))
		{
			Trxtext(7,13+(j*58),"  ");
			Trxtext(10,13+(j*58),"GM                        ");												//AH1显示探头
		}
		else
		{
			Trxtext(7,13+(j*58),"  ");
			Trxtext(10,13+(j*58),"DL                        ");
		}
      	Trnumber(60,13+(j*58),j+1);                                                  //显示探头号
      	Trxtext(90,13+(j*58),":");                                                   //显示冒号
	  	Trnumber(470,13+(j*58),Channel_Detector[j][1]);
	  	Trnumber(440,13+(j*58),Channel_Detector[j][0]);
	  	Trnumber(390,13+(j*58),Average_Times[j]);
      	if(Tbcd[5]!=0)                                                              //若百位非零，根据数的大小来显示
      	{
        	Trnumber(130,13+(j*58),Tbcd[5]);
        	Trnumber(160,13+(j*58),Tbcd[4]);
        	Trnumber(190,13+(j*58),Tbcd[3]);
        	Trxtext(220,13+(j*58),".");
        	Trnumber(240,13+(j*58),Tbcd[2]);
        	Trnumber(270,13+(j*58),Tbcd[1]);
        	Trnumber(300,13+(j*58),Tbcd[0]);
        	if(Tdata<10)                                                              //用于显示Tdata锁存的计数值显示
        	Trnumber(576,13+(j*58),Tdata);                                             //测试完成后去掉2012.6.24
        	else if((Tdata>=10)&&(Tdata<100))
        	Trnumber(556,13+(j*58),Tdata);
        	else if((Tdata>=100)&&(Tdata<1000))
        	Trnumber(536,13+(j*58),Tdata);
        	else if((Tdata>=1000)&&(Tdata<10000))
        	Trnumber(516,13+(j*58),Tdata);
        	else if(Tdata>=10000)
        	Trnumber(496,13+(j*58),Tdata);
      	}
      	else
      	{
        	if(Tbcd[4]!=0)                                                            //百位为零十位非零
        	{
          	Trnumber(160,13+(j*58),Tbcd[4]);
          	Trnumber(190,13+(j*58),Tbcd[3]);
          	Trxtext(220,13+(j*58),".");
          	Trnumber(240,13+(j*58),Tbcd[2]);
          	Trnumber(270,13+(j*58),Tbcd[1]);
          	Trnumber(300,13+(j*58),Tbcd[0]);
          	if(Tdata<10)
            	Trnumber(576,13+(j*58),Tdata);                                         //测试完成后去掉2012.6.24
          	else if((Tdata>=10)&&(Tdata<100))
            	Trnumber(556,13+(j*58),Tdata);
          	else if((Tdata>=100)&&(Tdata<1000))
            	Trnumber(536,13+(j*58),Tdata);
          	else if((Tdata>=1000)&&(Tdata<10000))
            	Trnumber(516,13+(j*58),Tdata);
          	else if(Tdata>=10000)
            	Trnumber(496,13+(j*58),Tdata);                                         //测试完成后去掉2012.6.24
        	}
        	else                                                                      //百位十位位均为零，个位非零
        	{
          	Trnumber(190,13+(j*58),Tbcd[3]);
          	Trxtext(220,13+(j*58),".");                                              //.占20个像素
          	Trnumber(240,13+(j*58),Tbcd[2]);                                         //一个数字占30个像素
          	Trnumber(270,13+(j*58),Tbcd[1]);
          	Trnumber(300,13+(j*58),Tbcd[0]);
          	if(Tdata<10)
            	Trnumber(576,13+(j*58),Tdata);                                         //测试完成后去掉2012.6.24
          	else if((Tdata>=10)&&(Tdata<100))                                       //可以看见一个数字占20像素
            	Trnumber(556,13+(j*58),Tdata);
          	else if((Tdata>=100)&&(Tdata<1000))
            	Trnumber(536,13+(j*58),Tdata);
          	else if((Tdata>=1000)&&(Tdata<10000))
            	Trnumber(516,13+(j*58),Tdata);
          	else if(Tdata>=10000)
            	Trnumber(496,13+(j*58),Tdata);
        	}
	  	}
      	/**********计量单位显示***********/
      	if(Flag_dw==1)                                                              //测试完成后加上2012.6.24
      	{
        	Trxtext(340,13+(j*58),"u");
      	}
      	else if(Flag_dw==2)
      	{
        	Trxtext(340,13+(j*58),"m");
      	}
      	else if(Flag_dw==3)
      	{
        	Trxtext(340,13+(j*58),"G");
      	}
      	Alarm();                                                                    //报警
	  	}
      send_buf[8*j]=Tbcd[5];                                                      //向NIM-B发送显示数据
      send_buf[8*j+1]=Tbcd[4];
      send_buf[8*j+2]=Tbcd[3];
      send_buf[8*j+3]=Tbcd[2];
      send_buf[8*j+4]=Tbcd[1];
      send_buf[8*j+5]=Tbcd[0];
      send_buf[8*j+6]=Flag_dw;                                                    //发送单位标志
      send_buf[8*j+7]=Flag_need_Flash[j];                                         //LED闪烁标志
    }
    for(j=0;j<8;j++)                                                              //置DCS数据的报警信号
    {
      if(Flag_need_Flash[j])
      {
        DCS_Send[64]=DCS_Send[64]|Svar1[j];
      }
      else
      {
        DCS_Send[64]=DCS_Send[64]&Svar0[j];
      }
    }                                                                             //8个探头均显示执行完
    Flag_Warn_Count=1;
    jishucount++;                                                                 //计数次数增加（算平均值时用）
    if(jishucount==255)                                                           //计数次数因为是char型变量，最多支持255个，所以要置零
      jishucount=8;
    if(jishucount>4)
      Flag_Meant=1;                                                               //前五次的取值不要，第五次才开始要，置Flag_Meant=1；否则均置0
    else
      Flag_Meant=0;
    //shortdelay(5000);                                                             //延时
    //for(i=0;i<33;i++)//显示完清0
    //buf[i]=0;
  }                                                                               //测量标志
  else
  {
    jishuguan_data=0;                                                             //needthinkwhetherneed?4.21,int计数管数据，
    dianlishi_data=0;                                                             //电离室数据
    jishuguan_rata=0;                                                             //计数管计数率
    jishuguan_jtemp=0;                                                            //计数管剂量值(单位转换后)
    dianlishi_jtemp=0;                                                            //电离室剂量值（单位转换后）
    Tbcd[11]=0;
    Tbcd[10]=0;
    Tbcd[9]=0;
    Tbcd[8]=0;
    Tbcd[7]=0;
    Tbcd[6]=0;
    Tbcd[5]=0;
    Tbcd[4]=0;
    Tbcd[3]=0;
    Tbcd[2]=0;
    Tbcd[1]=0;
    Tbcd[0]=0;
    j=10*biaoding_input-7;                                                        //biaodin_input为要标定的探头序号
    jishuguan_data=buf[4*biaoding_input-1]*256+buf[4*biaoding_input-2];       //相当于Tdata
    jishuguan_rata=jishuguan_data*12;                                             //计数率，直接乘以12?
    dianlishi_data=buf[4*biaoding_input+1]*256+buf[4*biaoding_input];     //电离室Tdata
    Para[0]=(float)(DataThouth[j]*1000+DataCent[j]*100+DataTenth[j]*10+DataGe[j]);//调节参数
    Para[1]=(float)((float)(DataThouth[j+1]*1000+DataCent[j+1]*100+DataTenth[j+1]*10+DataGe[j+1])/1000000);
    jishuguan_DoseRata=(float)(Para[1]*(jishuguan_rata-Para[0]));
    if(jishuguan_DoseRata<0)
      jishuguan_DoseRata=0.0;
    jishuguan_jtemp=(float)jishuguan_DoseRata;
    if((Tbcd[11]<=9)&&(Tbcd[10]<=9)&&(Tbcd[9]<=9)&&(Tbcd[8]<=9)&&(Tbcd[7]<=9)&&(Tbcd[6]<=9))//4.21似乎多余，可不可以修改掉
    {
      Tbcd[11]=(uchar)(((int)jishuguan_jtemp)/100);                               //千位
      Tbcd[10]=(uchar)(((int)jishuguan_jtemp)%100/10);                            //百位
      Tbcd[9]=(uchar)(((int)jishuguan_jtemp)%100%10);                             //个位
      jishuguan_count_temp=(jishuguan_jtemp-(int)jishuguan_jtemp)*1000;
      Tbcd[8]=(uchar)(jishuguan_count_temp/100);
      Tbcd[7]=(uchar)(jishuguan_count_temp%100/10);
      Tbcd[6]=(uchar)(jishuguan_count_temp%100%10);
    }
    Para[2]=(float)(DataThouth[j+2]*1000+DataCent[j+2]*100+DataTenth[j+2]*10+DataGe[j+2]);//调节参数
    Para[3]=(float)((float)(DataThouth[j+3]*1000+DataCent[j+3]*100+DataTenth[j+3]*10+DataGe[j+3])/1000);
    dianlishi_DoseRata=(float)(Para[3]*(dianlishi_data-Para[2]));
    if(dianlishi_DoseRata<0)
      dianlishi_DoseRata=0.0;
    dianlishi_jtemp=(float)(dianlishi_DoseRata/1000);
    if((Tbcd[5]<=9)&&(Tbcd[4]<=9)&&(Tbcd[3]<=9)&&(Tbcd[2]<=9)&&(Tbcd[1]<=9)&&(Tbcd[0]<=9))//4.21似乎多余，可不可以修改掉
    {
      Tbcd[5]=(uchar)(((int)dianlishi_jtemp)/100);                                //千位
      Tbcd[4]=(uchar)(((int)dianlishi_jtemp)%100/10);                             //百位
      Tbcd[3]=(uchar)(((int)dianlishi_jtemp)%100%10);                             //个位
      dianlishi_count_temp=(dianlishi_jtemp-(int)dianlishi_jtemp)*1000;
      Tbcd[2]=(uchar)(dianlishi_count_temp/100);
      Tbcd[1]=(uchar)(dianlishi_count_temp%100/10);
      Tbcd[0]=(uchar)(dianlishi_count_temp%100%10);
    }
    Txtext(200,58,"标定探头");
    Tnumber(400,58,biaoding_input);
    Txtext(10,116,"盖革管");;
    Txtext(160,116,":");
    Txtext(220,174,"uGy/h");
    Txtext(520,174,"CPM");
    Txtext(10,290,"电离室");;
    Txtext(160,290,":");
    Txtext(220,348,"mGy/h");
    Txtext(520,348,"CP5S");
    if(Tbcd[11]!=0)
    {
      Tnumber(10,174,Tbcd[11]);
      Tnumber(40,174,Tbcd[10]);
      Tnumber(70,174,Tbcd[9]);
      Txtext(100,174,".");
      Tnumber(130,174,Tbcd[8]);
      Tnumber(160,174,Tbcd[7]);
      Tnumber(190,174,Tbcd[6]);
      if(jishuguan_data<10)
        Tnumber(490,174,jishuguan_data);                                          //测试完成后去掉2012.6.24
      else if((jishuguan_data>=10)&&(jishuguan_data<100))
        Tnumber(460,174,jishuguan_data);
      else if((jishuguan_data>=100)&&(jishuguan_data<1000))
        Tnumber(430,174,jishuguan_data);
      else if((jishuguan_data>=1000)&&(jishuguan_data<10000))
        Tnumber(400,174,jishuguan_data);
      else if(jishuguan_data>=10000)
        Tnumber(370,174,jishuguan_data);
    }
    else
    {
      if(Tbcd[10]!=0)
      {
        Tnumber(40,174,Tbcd[10]);
        Tnumber(70,174,Tbcd[9]);
        Txtext(100,174,".");
        Tnumber(130,174,Tbcd[8]);
        Tnumber(160,174,Tbcd[7]);
        Tnumber(190,174,Tbcd[6]);
        if(jishuguan_data<10)
        Tnumber(490,174,jishuguan_data);                                          //测试完成后去掉2012.6.24
        else if((jishuguan_data>=10)&&(jishuguan_data<100))
        Tnumber(460,174,jishuguan_data);
        else if((jishuguan_data>=100)&&(jishuguan_data<1000))
        Tnumber(430,174,jishuguan_data);
        else if((jishuguan_data>=1000)&&(jishuguan_data<10000))
        Tnumber(400,174,jishuguan_data);
        else if(jishuguan_data>=10000)
        Tnumber(370,174,jishuguan_data);
      }
      else
      {
        Tnumber(70,174,Tbcd[9]);
        Txtext(100,174,".");
        Tnumber(130,174,Tbcd[8]);
        Tnumber(160,174,Tbcd[7]);
        Tnumber(190,174,Tbcd[6]);
        if(jishuguan_data<10)
          Tnumber(490,174,jishuguan_data);                                        //测试完成后去掉2012.6.24
        else if((jishuguan_data>=10)&&(jishuguan_data<100))
          Tnumber(460,174,jishuguan_data);
        else if((jishuguan_data>=100)&&(jishuguan_data<1000))
          Tnumber(430,174,jishuguan_data);
        else if((jishuguan_data>=1000)&&(jishuguan_data<10000))
          Tnumber(400,174,jishuguan_data);
        else if(jishuguan_data>=10000)
          Tnumber(370,174,jishuguan_data);
      }
    }
    if(Tbcd[5]!=0)
    {
      Tnumber(10,348,Tbcd[5]);
      Tnumber(40,348,Tbcd[4]);
      Tnumber(70,348,Tbcd[3]);
      Txtext(100,348,".");
      Tnumber(130,348,Tbcd[2]);
      Tnumber(160,348,Tbcd[1]);
      Tnumber(190,348,Tbcd[0]);
      if(dianlishi_data<10)
        Tnumber(490,348,dianlishi_data);                                          //测试完成后去掉2012.6.24
      else if((dianlishi_data>=10)&&(dianlishi_data<100))
        Tnumber(460,348,dianlishi_data);
      else if((dianlishi_data>=100)&&(dianlishi_data<1000))
        Tnumber(430,348,dianlishi_data);
      else if((dianlishi_data>=1000)&&(dianlishi_data<10000))
        Tnumber(400,348,dianlishi_data);
      else if(dianlishi_data>=10000)
        Tnumber(370,348,dianlishi_data);
    }
    else
    {
      if(Tbcd[4]!=0)
      {
        Tnumber(40,348,Tbcd[4]);
        Tnumber(70,348,Tbcd[3]);
        Txtext(100,348,".");
        Tnumber(130,348,Tbcd[2]);
        Tnumber(160,348,Tbcd[1]);
        Tnumber(190,348,Tbcd[0]);
        if(dianlishi_data<10)
          Tnumber(490,348,dianlishi_data);                                        //测试完成后去掉2012.6.24
        else if((dianlishi_data>=10)&&(dianlishi_data<100))
          Tnumber(460,348,dianlishi_data);
        else if((dianlishi_data>=100)&&(dianlishi_data<1000))
          Tnumber(430,348,dianlishi_data);
        else if((dianlishi_data>=1000)&&(dianlishi_data<10000))
          Tnumber(400,348,dianlishi_data);
        else if(dianlishi_data>=10000)
          Tnumber(370,348,dianlishi_data);
      }
      else
      {
        Tnumber(70,348,Tbcd[3]);
        Txtext(100,348,".");
        Tnumber(130,348,Tbcd[2]);
        Tnumber(160,348,Tbcd[1]);
        Tnumber(190,348,Tbcd[0]);
        if(dianlishi_data<10)
          Tnumber(490,348,dianlishi_data);                                        //测试完成后去掉2012.6.24
        else if((dianlishi_data>=10)&&(dianlishi_data<100))
          Tnumber(460,348,dianlishi_data);
        else if((dianlishi_data>=100)&&(dianlishi_data<1000))
          Tnumber(430,348,dianlishi_data);
        else if((dianlishi_data>=1000)&&(dianlishi_data<10000))
          Tnumber(400,348,dianlishi_data);
        else if(dianlishi_data>=10000)
          Tnumber(370,348,dianlishi_data);
      }
    }
  }
}

/************************************
*短延时
************************************/
void shortdelay(uint i)
{
  uint k;
  uint n;
  for(k=0;k<i;k++)
    for(n=200;n>0;n--);
}

/********************************
*指示灯和报警
********************************/
void Alarm()
{
  uchar n;
  for(n=0;n<10;n++)
  {
    Led573=Var_Led;
    if(Flag_Warn==1)                                                                  //声音报警
    {
      Speak=1;
    }
  }
}

/*******************************
*更新指示灯状态
*******************************/
void Updata_Flash(uchar j)
{
  if(Flag_need_Flash[j])
  {
    if(State_Flash[j]==1)
    {
      State_Flash[j]=0;
    }
    else
    {
      State_Flash[j]=1;
    }
  }
  else
  State_Flash[j]=0;
}

/***********************************
*指示灯闪烁
************************************/
void Led_Flash(void )
{
  uchar i;
  for(i=0;i<8;i++)
  {
    Updata_Flash(i);                                                                  //状态取反
    Led_Disp(i,Flag_need_Flash[i],State_Flash[i]);                                    //灯状态显示
  }
}

/************************************
*指示灯状态显示
*************************************/
void Led_Disp(uchar Num,uchar Flag,uchar State)
{
  if(Flag)
  {
    if(State)
    {
      Var_Led=Var_Led&Svar0[Num];                                                       //指示灯亮
      Led573=Var_Led;
      if(Flag_Warn==1)                                                                  //需要蜂鸣器工作
      {
        Speak=0;                                                                          //蜂鸣器响
        shortdelay(100);
      }
      return;
    }
  }
  Var_Led=Var_Led|Svar1[Num];                                                       //指示灯灭
  Led573=Var_Led;
  if(Flag_Warn==1)                                                                  //需要蜂鸣器工作
  {
    Speak=1;                                                                          //蜂鸣器不响
    shortdelay(100);
  }
}
