/*********************************************************************************************************************
* 仪器名称：
           r辐射仪
* 功能：
        1.   时间键：设置测量时间，可通过液晶屏显示；
       	2.   探头键：设置探头个数，可通过液晶屏显示；
		3.   测量键：计数探头计数结果，可通过液晶屏显示每个探头计数率；
		4.   参数键：设置参数大小，可通过液晶屏显示；
		5.   参数右移键：设置参数的位，与参数键结合，来设置每位参数的大小，可通过液晶屏光标显示；
        6.   重设键： 将设置返回到初始化状态；

* 按键：   探头设置键，测量键，复位键，时间设置键，参数设置键，右移键，左移键；
* 接口：
           输入：1.	六个按键通过74HC244接单片机的数据总线；
		         2. 探头1-15接到8053计数器，五片8253数据端接单片机的数据总线；

		   输出： 8253门信号--P1.1,蜂鸣--P1.0，主机/从机选择--P1.6，复位--P4.7，蜂鸣器开关--P1.7,从机显示开关--P3.3;
		          主机请求信号Note1--P4.4,主机应答信号ACKO--P3.5,从机请求信号Note0--P3.4,从机应答信号ACK1--P4.6；
				  数据总线--P0口，地址总线--P2口；
*--------------------------------------------------------------------------------------------------------------------
*相对于原始程序的修改记录:
*2016年4月14日，修改剂量显示单位为uGy/h,通道名称:Ch1...
*2016年4月18日(AA1)，
*2016年4月19日(AB1),
*2016年4月20日(AC1),
*2016年4月22日(AD1),
*2016年4月24日(AE1),
*********************************************************************************************************************/
/********************************
待修订的：参数报警阈值，参数段设置的阈值
*********************************/

#include "STC15F2K60S2.h"
#include "intrins.h"
#include "config.h"
#include "absacc.h"


bdata bit Flag_KeyInput;                         //探头按键标志，默认0
bdata bit Flag_KeyTime;                          //测量时间标志，0
bdata bit Flag_KeyPara;                          //输入参数标志，0
bdata bit Flag_KeyRight;                         //右移标志，默认0
bdata bit Flag_KeyLeft;                          //左移标志，0
bdata bit Flag_KeyReset;                         //重新显示标志，0
bdata bit Flag_KeyOK;                            //测量标志，默认0
bdata bit Flag_Warn;                             //报警标志
bdata bit Flag_Mast;                             //主从机标志
bdata bit Flag_RefrInput;                        //探头个数按键再次按下标志
bdata bit Flag_RefrTime;                         //测量时间按键再次按下标志
bdata bit Flag_RefrPara;                         //输入参数按键再次按下标志
bdata bit Flag_RefrRight;                        //右移按键再次按下标志
bdata bit Flag_RefrLeft;                         //左移按键再次按下标志
bdata bit measure_flag;							//默认0
bdata bit bSwOld;                                //实体采集按键旧的状态
bdata bit bSwNew;                                //实体采集按键新的状态
bdata bit bSbOld;                                //备用机显示按键旧的状态
bdata bit bSbNew;                                //备用机显示按键新的状态
bdata bit bSsOld;                                //蜂鸣器按键旧的状态
bdata bit bSsNew;                                //蜂鸣器按键新的状态
bdata bit Flag_Warn_Led;                         //屏幕上报警灯闪烁标志，默认0
bdata bit Flag_Warn_Flash;                       //LED灯闪烁标志
bdata bit FlagMasColSlavOK;                      //主机采集数据时灯闪烁定时标志，默认0
bdata bit Flag8253Cnting;                        //8253计数定时开始标志，默认0,在按下测量按键后置1
bdata bit Flag_ParaChange;                       //参数设置改变，默认0
bdata bit Flag_InputChange;                      //探头设置改变，默认0
bdata bit Flag_Collateral;                       //从机接收指令超时标志
bdata bit Flag_Meant;
bdata bit Flag_NIMB_Sended;						//已经从NIM-B接收数据标志
//bit Flag_Commond;
uchar dt_in[331];                       	 //IIC存储数组，四位一组，共83组,芯片内部默认0为255
uchar data Keyvar=0xFF;                    //按键变量
uchar jishucount;							//第几次计数，用来求平均值时知道这是第几次计数
uchar  Average_Times[8];								//AJ1+滑动平均次数
uchar Count_Times[8];						//AJ1+累加计数次数
ulong Average_Counts[8][5];//平滑平均变量
uchar Display_Flag[8];
uchar Refresh_Time=3;
uchar DCS_Send[67];


extern void Serial_Port_Two_Initial();
extern void Serial_Port_One_Initial();
extern void display_b();
extern void Init_MCU();
extern void Init_RAM();
extern void Init_Para();
extern bit Speak_Alarm();
extern void Select_Mast();
extern void key_function();
extern void RedLed_Flash();
extern void Warning_Disp(uchar Num,uchar Flag,uchar State);
extern void Updata_Led(uchar j);
//extern void bakeup_conv_data();
extern void Backup_Flash_serial1(uchar *flag,uchar *state);
extern void Init_IO();
extern void Transfer();
extern void parameterleft();
extern void BackUp_displayA();
extern void display_covn_data();
extern void Init_Buf();
extern uint idata compare[24];
//extern float idata save[16];

/*********************************
*            主函数
**********************************/
void main()
{
	Init_IO();                                    //I/O口初始化,
	Init_MCU();                                   //主函数初始化，T0定时器工作在方式1初始化，显示"请稍等，初始化中",GATE=0,
	Init_RAM();                                   //位变量初始化
	Init_Para();                                  //参数初始化,初始化参数，从IIC读取阈值等信息赋予给参量
	Init_Buf();									//并口通讯数组初始化
	//Select_Mast();                                //判断主从机
	Init_Time0();	                             //开启定时器0。定时器0初始化，工作在方式一，定时初值：H:0xDC,L:0x00;中断计数初始化？定时5ms,NumT0=0;
    Lcd_Clear();								//LCD清屏
	Lcd_Start();								//显示"清华大学/n核能与新能源技术研究院"
	EX0=1;                                        //只允许外部0中断，中断0为键盘
	IT0=1;                                        //外部中断0
	while(1)
	{
//	   BackUp_Display();
	   Select_Mast();                                //判断主从机，写入主从机标志Flag_Mast=1(主机)0(从机)
	   key_function(); 								//按键功能，在while循环中不断检测按键标志，按键标志由按键外部中断来更改
	   if(Flag_Tim0)	                         //8253计数定时结束时，读取探头的计数，刚开始Flag_Tim0=0，仅当Flag8253Counting计数定时标识为1，且计数器中断次数大于设定的中断次数时，Flag_timo才会等于1
	    {										//Flag_Tim0为计数器结束标志,初始为0,开机打开定时器中断后,定时器5ms中断一次,检查Flag_Tim0一次,为一表示计数器计时结束
			Flag_Tim0 = 0;		                 //定时标志清0
			Flag_Warn = 0;		                 //报警标志清0
			GetAndDisdata(); 	                 //从8253的锁存器得到测量计数器结果,存入至buf数组
			ShowData();                          //显示测量数据
			//shortdelay(1000);	          		//
			if(Zhu_Cong==1)
			{
	    	Transfer();                          //并行传输数据
			}
	    Transfer_DCS();
//			bakeup_conv_data();
			Init_8253();   						//初始化8253
			                                      //定时结束时，立刻又开始初始化进行计数
	   }
	    if(Flag_Tim0 == 0)                        //开启8253计数过程中，处理报警中断，报警有中断吗？
       {
			if(Flag_Warn_Flash == 1)                   //主机LED指示灯闪烁间隔定时，Flag_Warn_Flash为LED灯闪烁标志
			{
				Flag_Warn_Flash = 0;
	            Led_Flash();
			}
			if((Flag_Warn_Led==1) && (PCOLSIG==0))      //屏幕上红灯闪烁间隔定时,程序中没有找到PCOLSIG=0的程序段
			{
				Flag_Warn_Led = 0;
				RedLed_Flash();							//灯闪烁
			 }
	    }
	}
}


/************************************
*            按键中断程序
**************************************/
void Int0() interrupt 0
 {
	uint  idata i;
	uchar idata Ptem1,Ptem2;
	ES=0;                                           //禁止串口中断
	IE2=0x00;                                       //禁止串口2中断
	EX0=0;                                          //禁止外部0中断
	Keyvar=Keypress;
	Ptem1=Keyvar;                                   //读取端口的地址是否先给它全部置1
	Ptem1=Ptem1&0x7F;
	for(i=0;i<3000;i++);                            //延时去抖
	Ptem2=Keyvar;
	Ptem2=Ptem2&0x7F;
	if(Ptem1!=Ptem2)
	{
	  EX0=1;
	}
	else
	{
	   if(Keypress==0x7E)                     //按键1：参数设置
			 Flag_KeyPara=1;
	   else if(Keypress==0x7D)                //按键2：测量
		     Flag_KeyOK=1;
	   else if(Keypress==0x7B)                //按键3：参数右移
		     Flag_KeyRight=1;
	   else if(Keypress==0x77)                //按键4：重新设置
		     Flag_KeyReset=1;
	   else if(Keypress==0x6F)                //按键5：时间设置
		     Flag_KeyTime=1;
	   else if(Keypress==0x5F)                //按键6：探头设置
		     Flag_KeyInput=1;
       else if(Keypress==0x3F)                //按键7：参数左移
             Flag_KeyLeft=1;
	}
	EX0=1;                         //need change
}


/****************************************
*         CPU初始化
****************************************/
void Init_MCU()
{
	/*定时计数器的初始化*/
	TMOD=0x01;                                 //T0为定时器，工作在方式1
	GATE=0;                                    //8253不工作
	EA=0;                                      //先屏蔽所有中断
	TR0=0;										//暂时关闭定时器0
    IE0=0;                                     //将外部中断0清0
	/*为LCD显示  禁止串口中断*/
	EX0=0;                                     //禁止外部1中断
	ET0=0;                                     //禁止定时0中断
	ES=0;                                      //禁止串口1中断
	IE2=0x00;                                  //禁止串口2中断
	/*******串口初始化*********/
    serial_port_two_initial();                 //串口2中断初始化(未找到定义)
    serial_port_one_initial();
	/*液晶初始化显示*/
    Lcd_Clear();                               //串口2清屏
    //picture();                               //串口2初始图片
    Lcd_init();                                //串口2初始化LCD显示（请稍等，进行初始化中”
		Clear();                                   //清除光标
}
/****************************************
*         参数初始化
****************************************/
void Init_Para()
{
	uchar i,j,k,y,m,n,q,x;//,w ,jj
//	uchar s,z;
	uchar p=0;

	for(i=0;i<75;i++)                           //初始化参数数组
	{
		DataGe[i]=0;      						//各个位
		DataTenth[i]=0;      					//十分位
		DataCent[i]=0;     						//百分位
		DataThouth[i]=0;						//千分位
	}
	for(j=0;j<8;j++)
	{
	    YuGe[j]=0;
		YuTenth[j]=0;
		YuCent[j]=0;
		YuThouth[j]=0;
        Flag_need_warn[j]=0;
        State_On_Off[j]=0;
		DoseRata[j] = 0;						//每组探头测得的剂量率
		count_change_flag[j]=0;
		Channel_Detector[j][0]=3;
		Channel_Detector[j][1]=4;					//AC1+初始均为DL3
		for(i=0;i<6;i++)
		{
			Average_Counts[j][i]=0;
		}
	}
	for(m=0;m<83;m++)                           //初始化存储数组
 	{
		dt_in[4*m]=0;
		dt_in[4*m+1]=0;
		dt_in[4*m+2]=0;
		dt_in[4*m+3]=0;
    }
	for(n=0;n<65;n++)                           //初始化接收数组
	{
       send_buf[n]=0;							//NIM_A向NIM_B发送数据的数组
	}
/*	for(s=0;s<20;s++)
	{
		backup_data[s] = 0;
	}*/
	for(y=0;y<8;y++)
	{
        Flag_need_warn[y]=0;
        State_On_Off[y]=0;
//		data_A[y]=0;
	}
/*	for(z=0;z<12;z++)
	{
		data_B[z]=0;
	}*/
	/*从AT24C16中读取数据*/
    DS_ReadData(dt_in);							//IIC EEPROM读取数据
	for(x=0;x<83;x++)                          //芯片内部默认0为255
	{
		if(dt_in[x*4]==255)
		{
			dt_in[4*x]=0;
		}
		if(dt_in[x*4+1]==255)
		{
			dt_in[4*x+1]=0;
		}
		if(dt_in[x*4+2]==255)
		{
			dt_in[4*x+2]=0;
		}
		if(dt_in[x*4+3]==255)
		{
			dt_in[4*x+3]=0;
		}
	}
    Incinput=dt_in[330];                       // 读取IIC中的探头个数标志
	/*********读取参数*********/
    for(q=0;q<74;q++)
	{
		p++;
		if(p%11==0)
		{
			 p++;
		 }
	    if((dt_in[p*4+1] <= 9)&&(dt_in[p*4+2] <= 9)&&(dt_in[p*4+3] <= 9)&&(dt_in[p*4+4] <= 9))
        {
			DataThouth[q+1] = dt_in[p*4+1];		//千分位
			DataCent[q+1] = dt_in[p*4+2];		//百分位
			DataTenth[q+1] = dt_in[p*4+3];		//十分位
			DataGe[q+1] = dt_in[p*4+4]; 		//个位

		  }
	   }
	/********读取阈值***********/
    for(k=0;k<8;k++)
	{
	 if((dt_in[(k*44)+1] <= 9)&&(dt_in[(k*44)+2] <= 9)&&(dt_in[(k*44)+3] <= 9)&&(dt_in[(k*44)+4] <= 9))
       {
	        YuThouth[k] = dt_in[(k*44)+1];
			YuCent[k] = dt_in[(k*44)+2];
			YuTenth[k] = dt_in[(k*44)+3];
			YuGe[k] = dt_in[(k*44)+4];
	    }
	}
   if(dt_in[1]<=9)
   {
	   Incdata=dt_in[1];
   }
}

/****************************************
*         数组初始化
****************************************/
void Init_Buf()
{
	uchar n;
	for(n=0;n<57;n++)                             //初始化接收数组,**放在此处原因：放在参数初始换函数中，在此处调用时，数据已经发生变化** can not understand
	{
	   receive_buf[n] = 0;
	}
}

/************************************
*          位标识初始化
*************************************/
void Init_RAM()
{
	uchar i;
    bz = 1;                                      //参数设置初始参数个数标志
	xh = 1;                                      //参数设置初始探头个数标志
	wb = 1;                                      //参数设置初始参数行数标志
	Flag_dw = 1;                                 //初始单位标志
    Note0 = 1;                                   //并口从机发送数据标志初始化(P3^4)
	Note1 = 1;									//并口从机接收数据标志初始化(P4^4)
	Ack0 = 1;									//并口主机接收完数据应答信号标志(P3^5)
	Ack1 = 1;  									//并口主机发送完数据应答信号标志(P4^6)
	Speak = 1;                                    //高电平代表蜂鸣器不响(P1^0)
	Incdata = 0;                                  //参数大小初始化
	Inctime = 0;                                  //时间按键增加标志初始化
	Incinput = 8;                                 //默认显示探头个数为8个
	for(i=0;i<8;i++)								//AA1+
	{
		Real_Count_Display[i]=0;                     //AA1*默认刷新时间为60秒
		Setted_Time[i]=60;							//AB1+初始化设置时间变量
		Average_Times[i]=0;								//AJ1+滑动平均次数
		Display_Flag[i]=0;
		Channel_Display[i]=0;
	}
	Incdataright = 1;							//右移按键初始化
	jishucount=0;
    Var_Led = 0xff;                               //led报警指示灯
	Led573 = Var_Led; 								//用XBYTE传输数据给LED
    Var_Signal1 = 0x00;                           //探头控制信号标志，接计数管
    Var_Signal2 = 0x00; 							//探头控制信号标志，接电离室
    Var_Signal3 = 0xff;								//探头控制信号标志，接电离室II，开机默认
	Signal1 = Var_Signal1;							//下面三条传输信号，默认全接计数管
    Signal2 = Var_Signal2;
    Signal3 = Var_Signal3;
    sendadd = 0x00;                                //给并行从机发送初始指令
    PCOLSIG = 1;                                   //高电平代表不采集数据，P3^3,
    bSwNew = PCOLSIG;	                           //采集数据（bswNew为位布尔变量）
    bSwOld = bSwNew;    							//老按键
    PALE = 1;                                      //高电平代表不显示备用机,P4^5,备用显示屏按键
    bSbOld = 1;   									//备用机显示按键布尔标志
	Judge_Speak = 1;                               //高电平代表蜂鸣器不工作，P1^7蜂鸣器开关按键
	bSsNew = Judge_Speak;	                       //蜂鸣器工作
    bSsOld = bSsNew;
    Zhu_Cong = 1;                                  //主从机标志信号，高电平代表是主机
	Flag_KeyInput = 0;                             //探头个数标志
	Flag_KeyTime = 0;                              //测量时间标志
	Flag_KeyPara = 0;                              //输入参数标志
	Flag_KeyRight = 0;                             //右移标志
	Flag_KeyOK = 0;                                //开始测量标志
	Flag_KeyReset = 0;                             //重新显示标志;
	Flag_KeyLeft = 0;
	Flag_Tim0 = 0;
	Flag_Warn_Led = 0;
	FlagMasColSlavOK = 0;
	Flag8253Cnting = 0;
	Flag_Warn_Count = 0;
	measure_flag=1;
	FlagCollErr = 0;
    CntColTimer = 0;
    FlagColling = 0;
	Flag_ParaChange = 0;
	Flag_InputChange = 0;
//	Flag_serial1_led = 0;
	CmOverTime = 0;
    Flag_Collateral = 0;
    FlagCollFall = 0;
	Flag_Meant = 0;
//	Flag_data_change = 0;
//	Flag_Commond = 1;
	Max_Time=0;										//AA1+所有通道最长的计数时间
}

/************************************
*         I/O端口初始化
*************************************/
void Init_IO()                       //设置I/O端口类型
{
    	//P4SW = 0x70;                    //P4.4 P4.5 P4.6作为普通I/O口
	/*AH1*改为15系列端口初始化*/
	P3M1 |= 0x30;                   //设置P3.5，P3.4口输入
	P3M0 &= 0xCF;

	P3M1 &= 0x3C;
	P3M0 |= 0xC3;                   //设置P3.1, p3.0口为强推挽输出

	//P4M1 &= 0xAF;                   //设置P4口为强推挽输出
	//P4M0 |= 0x50;

}

/************************************
*          采集数据按键处理
*************************************/
/*bit Select_collect(void)
{
	bit bRet;
	bRet = 0;
	bSwNew=PCOLSIG;
    if(bSwNew != bSwOld)
    {
    	if(bSwOld==0)
         {
          	bRet = 0;          //不采集从机数据
	     }
        else
         {
			bRet = 1;          //采集从机数据
         }
		bSwOld=bSwNew;
    }
   	return bRet;
}*/

/************************************
*          主从机按键处理
*************************************/
void Select_Mast()
{
	if(Zhu_Cong==1)				//P1^6主从机开关按键，高电平为主机
     {
      	Flag_Mast = 1;          //主机
     }
    else if(Zhu_Cong==0)
     {
      	Flag_Mast = 0;         //从机
     }
}

/************************************
*       备用机显示按键处理（****************4.21号上午觉得有问题：备用显示按键是普通按键，不是开关按键形式）
*************************************/
/*void BackUp_Display(void)
{
	bSbNew = PALE;
    if(bSbNew != bSbOld)
    {
		if(bSbNew == 0)
         {
			Flag_Commond = ~Flag_Commond;
         }
		bSbOld = bSbNew;
    }
}*/


/***********************************
*         蜂鸣器按键处理
***********************************/
bit Speak_Alarm()
{
	bit bspeak;					//定义一位Bool型变量
	bspeak = 0;
	bSsNew=Judge_Speak;			//蜂鸣器开关按键(保存为新状态)
    if(bSsNew != bSsOld)		//若蜂鸣开关状态有变动
    {
    	if(bSsOld==0)			//蜂鸣老状态为0
         {
          	bspeak = 0;        //蜂鸣器不工作
	     }
        else
         {
			bspeak = 1;      //蜂鸣器工作
         }
		bSsOld=bSsNew;
    }
   	return bspeak;
}
/**********************************
*          按键功能
**********************************/
void key_function()
{
	  if(Flag_KeyInput)                  //探头按键按下(按下后进行探头的标定动作,同时显示GM计数管和电离室的计数值和计算出来的剂量值)
	   {
	       Flag_InputChange = 1;         //探头设置变动标志置1,表示探头变动过,在改完探头进入测量后,若此项为一,则会先存储改过的设置
		   Flag_KeyInput = 0;			//按键标志位清零
		   setinput();
		   EX0=1;						//开启外部中断0
	   }
	   if(Flag_KeyTime)                   //时间按键按下
	   {
		   Flag_KeyTime = 0;
		   settime();
		   EX0=1;
	   }
	   if(Flag_KeyPara)                   //参数按键按下
	   {
	       Flag_ParaChange = 1;           //参数设置变动标志置1,表示参数变动过,在改完参数进入测量后,若此项为一,则会先存储改过的参数
		   Flag_KeyPara = 0;
		   setparameter();
		   EX0=1;
	   }
	   if(Flag_KeyRight)                  //右移按键按下
	   {
		   Flag_KeyRight = 0;
		   parameterright();
		   EX0=1;
	   }
	   if(Flag_KeyLeft)
	   {
		   Flag_KeyLeft = 0;
		   parameterleft();
		   EX0=1;
	   }
	   if(Flag_KeyOK)                     //测量按键按下
	   {
		   Flag_KeyOK = 0;
		   measure();
           EX0=0;
	   }
	   if(Flag_KeyReset)                  //重设按键按下
	   {
		   Flag_KeyReset = 0;
		   ReSet();
		   EX0=1;
	   }
}


/****************************************
*          主从机并行传输
****************************************/
void Transfer()
{
     if(Flag_Mast==0)                                  //从机
       {
	        Flag_Collateral = 1;
			CmOverTime = 0;
            send_buf[64]=Incinput;
			Send_Word(send_buf,NUMSENDBYTES);         //发送数据
       }
       if(Flag_Mast==1)                               //主机
       {
            Collect_Word(receive_buf,57);

       }

}
/****************************************
*          DCS串行传输
****************************************/
void Transfer_DCS()
{
	uchar i,temp;
	if(measure_flag)
	{
		temp=DCS_Send[0];
		for(i=1;i<66;i++)
		{
			temp=temp^DCS_Send[i];
		}
		DCS_Send[66]=temp;
		for(i=0;i<67;i++)
		{
			T1byte(DCS_Send[i]);
		}
	}
}
