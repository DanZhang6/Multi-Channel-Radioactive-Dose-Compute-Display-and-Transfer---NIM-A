#ifndef _CONFIG_H
#define _CONFIG_H

typedef unsigned char uchar;
typedef unsigned int uint;
typedef unsigned long ulong;
#define Led573      XBYTE[0xc000]       //led信号指示灯
#define Keypress    XBYTE[0xc800]       //按键地址
#define receiveradd XBYTE[0xd000]               //并行传输接收地址
#define sendadd     XBYTE[0xd800]               //并行传输发送地址
#define Signal1     XBYTE[0xa800]      //探头指令信号接计数器
#define Signal2     XBYTE[0xb000]		//探头指令信号接电离室
#define Signal3     XBYTE[0xb800]		//探头指令信号接电离室

#define NUMSENDBYTES    65              // 传输数据个数宏变量
#define TimeFiveHundred 100              //定时500ms
#define TimeThreeThousand 600        //定时1s
#define TimeOneHundred  1000              //定时100ms
extern uchar DataGe[75];             //个位
extern uchar DataTenth[75];          //十分位
extern uchar DataCent[75];           //百分位
extern uchar DataThouth[75];         //千分位
extern uchar DCS_Send[67];			//向DCS发送剂量数据
extern uchar YuThouth[8];
extern uchar YuCent[8];
extern uchar YuTenth[8];
extern uchar YuGe[8];
extern uchar Flag_need_warn[8];                   //需要报警标志
extern uchar State_On_Off[8];                     //指示灯状态标志
extern uchar buf[32];               //AB1*要发送的数据的高低位
extern uchar Channel_Detector[8][2];			//AC1+通道探头选择标志，1:电离室0:计数管
extern ulong Average_Counts[8][10];
extern ulong Count[8][3];						//历史计数
extern uchar send_buf[65];            //add 8  Flag_need_Flash  delect 32 yuzhi   4.21
extern uchar dt_in[331];
extern uchar Flag_need_Flash[8];
extern uchar State_Flash[8];
extern uchar receive_buf[57];
extern uchar Channel_Display[8];//用于显示的通道量程
extern uchar biaoding_input;
extern uchar Max_Time;
extern uchar Flag_dw;
//extern uint compare[32];
//extern uchar backup_data[20];            //存放备用显示的数据
extern uchar data_A[8];
extern uchar data_B[12];
extern uchar count_change_flag[8];
extern float Real_Count_Display[8];
extern uchar Setted_Time[8];
extern float DoseRata[8];                                                                //每组探头测得的剂量率
extern ulong Real_Count[8];
extern uchar Count_Times[8];
extern uchar Init_Time;
extern uchar Inctime;
extern uchar Incinput;
extern uchar Incdataright;
extern uchar Incdata;
extern uchar jishucount;
extern bdata bit Flag_Warn;
extern bdata bit Flag_RefrInput;
extern bdata bit Flag_RefrTime;
extern bdata bit Flag_RefrPara;
extern bdata bit Flag_RefrRight;
extern bdata bit Flag_RefrLeft;
extern bdata bit Flag_KeyLeft;
extern bdata bit Flag_Tim1;
extern bdata bit Flag_Mast;             //主从机标志
extern bdata bit FlagMasColSlavOK;
extern bdata bit Flag_Warn_Led;
extern bdata bit Flag8253Cnting;
extern bdata bit Flag_Warn_Count;
extern bdata bit Flag_Warn_Flash;
extern bdata bit Flag_ParaChange;                       //参数设置改变
extern bdata bit Flag_InputChange;                      //探头设置改变
extern bdata bit Flag_Tim0;                             //定时器0结束标志
//extern bit Flag_serial1_led;
extern bdata bit Flag_Warn_Serial1;                     //备用显示灯定时结束标识
extern bdata bit Flag_Collateral;                       //从机接收指令超时标志
extern bdata bit FlagCollFall;                         //主从机通信失败标志
extern bdata bit FlagCollErr;
extern bdata bit FlagColling;
extern bdata bit measure_flag;
//extern bit Flag_data_change;
//extern bit Flag_Commond;
extern bdata bit Flag_Meant;
extern uchar idata bz;
extern uchar idata xh;
extern uchar idata wb;
extern uint  Tdata;
extern uchar idata Var_Signal1;
extern uchar idata Var_Signal2;
extern uchar idata Var_Signal3;
extern uchar idata Var_Led;
extern uchar Refresh_Time;
extern uint CntWarn;
extern uint CntColTimer;
extern uint NumT0;
extern uint CmOverTime;




extern void Lcd_init();
extern void Lcd_Start();
extern void Lcd_Clear();
extern void serial_port_two_initial();				   //串口2中断初始化(未找到定义)
extern void serial_port_one_initial();
extern void Init_8253();
extern void Init_Time0();
extern void Transfer_DCS();
extern void Txbyte(uchar idata i);
extern void T1byte(uchar idata i);
extern void Tword(uint idata i);
extern void TxEOF();
extern void Tnumber(int idata x,int idata y,uint idata n);
extern void Txtext(uint idata x,uint idata y,uchar *s);
extern void Open (uint idata x,uint idata y);
extern void Close (uint idata x,uint idata y);
extern void WarnLed_On(uint idata x,uint idata y);
extern void WarnLed_Off(uint idata x,uint idata y);
extern void DS_SaveData(uchar *dt);
extern void DS_ReadData(uchar *dt);
extern void setparameter();
extern void parameterright();
extern void GetAndDisdata();
extern void ShowData();
extern void init();
extern void SendData();
extern void setinput();
extern void settime();
extern void measure();
extern void ReSet();
extern void Send_Word(uchar *pbuf,uint ibyte);
extern void Collect_Word(uchar *pub,uint nbyte);
extern void shortdelay(uint idata i);
extern void RedLed_Flash();
extern void Warning_Disp(uchar Num,uchar Flag,uchar State);
extern void Clear();
//extern void picture();
extern uchar CKMasterCmd();
extern void Alarm();
extern void Led_Flash(void);
//extern void BackUp_Display(void);

sbit scl = P1^4 ;			//时钟线接p14
sbit sda = P1^5 ;			//数据线接p15
sbit GATE= P1^3;            //AH1*,原1.1，8253门控信号
sbit Speak=P1^2;            //AH1*,原1.0，蜂鸣器，高电平表示不响
sbit Judge_Speak=P1^7;      //蜂鸣器标志信号，开关按键，高电平表示蜂鸣器不工作
sbit Zhu_Cong=P1^6;         //主从机标志信号，开关按键，高电平为主机
sbit PCOLSIG=P3^3;          //显示从机数据，应该没有实体按键，当虚拟按键使用，高电平表示不采集数据
sbit Note0=P3^4;            //并口从机发送数据标志
sbit Ack0=P3^5;             //并口主机接收完数据应答信号标志
sbit Note1=P3^7;            //AH1*,原4.4，并口从机接收数据标志
sbit Ack1=P3^6;             //AH1*,原4.6，并口主机接收完数据应答信号标志
sbit PALE=P4^5;             //备用显示屏按键，高电平表示不显示备用机

#endif
