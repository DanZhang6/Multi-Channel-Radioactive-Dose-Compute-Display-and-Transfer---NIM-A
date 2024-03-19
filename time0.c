/************************************************************
*描述：
*     定时器0设置
*功能：
*     定时器0工作在方式1，每5ms中断一次，定时器一直开着

***********************************************************/
#include "STC15F2K60S2.h"
#include "absacc.h"
#include "config.h"
#include "intrins.h"
uint NumT0;             // 8253定时中断次数,在按下测量按键后初始化为0;
uint CntFlhWarnT0;      // 主机采集数据时灯闪烁定时中断次数
uint CntWarn;           // LED灯闪烁定时中断次数
uint CntS1Warn;         // 备用显示屏上灯闪烁定时中断次数
uint CmOverTime;        // 主从机通信握手定时中断次数
bit Flag_Warn_Serial1;  // 备用显示灯定时结束标识
bit Flag_Tim0;          // 定时器0结束标志，默认0；
bit FlagCollFall;       // 主从通信失败标志，0
/*************************************
 *         定时器0中断程序
 **************************************/
void Tim0(void) interrupt 1  // tim0定时结束后运行终端程序(定时中断)
{
    TF0 = 0;     // 设置定时器控制定时器,重置定时器初值
    TH0 = 0xDC;  // 定时器0每5ms中断一次
    TL0 = 0x00;
    if (Flag_Mast)  // 若主从机按钮为主机
    {
        if (FlagMasColSlavOK)  // 主机采集数据时灯闪烁定时
        {
            if (++CntFlhWarnT0 >= TimeFiveHundred)  // TimeFiveHundred=100,定时器5ms中断一次,报警灯则500ms刷新一次
            {
                CntFlhWarnT0 = 0;
                Flag_Warn_Led = 1;  // 灯状态开始变换标志
            }
        }
    }
    if (Flag_Warn_Count)  // 按键测量时灯闪烁定时
    {
        if (++CntWarn >= TimeFiveHundred)  // 500ms刷新一次
        {
            CntWarn = 0;
            Flag_Warn_Flash = 1;  // 灯状态开始变换标志
        }
    }
    if (Flag8253Cnting)  // 8253计数定时开始标志中断,按下测量按键后=1 开始定时
    {
        if (NumT0 == 0) {
            GATE = 1;
        }
        NumT0 = ++NumT0;
        if (NumT0 >= (200 * Refresh_Time))  // AA1*,1S钟中断200次，Redrawtime时间内中断200*Redrawtime次,即中断1秒后执行,关闭8253门控信号
        {
            GATE = 0;  // 关闭8253门控信号
            NumT0 = 0;
            Flag_Tim0 = 1;       // 计时器结束标志，在main函数中不断检测是否为1，为一时进入循环
            Flag8253Cnting = 0;  // 定时计数标志清零
        }
    }
    if (FlagColling)  // 数据采集间隔时间定时
    {
        if (++CntColTimer >= TimeOneHundred)  // 100ms刷新一次
        {
            FlagCollErr = 1;
            CntColTimer = 0;
        }
    }
    if (Flag_Collateral)  // 主从机通信握手定时
    {
        if (++CmOverTime >= TimeThreeThousand) {
            FlagCollFall = 1;
            CmOverTime = 0;
        }
    }
}

/*************************************
 *         定时器0初始化
 **************************************/
void Init_Time0()
{
    TMOD = 0x01;  // 定时器0工作在方式1
    TH0 = 0xDC;   // 预置定时初值DC00H,晶振频率22.1184MHz,定时时长约5ms
    TL0 = 0x00;
    NumT0 = 0;  // 中断计数初始化
    CntFlhWarnT0 = 0;
    CntWarn = 0;
    CntS1Warn = 0;
    CmOverTime = 0;
    CntColTimer = 0;
    EA = 1;   // 开启总中断
    ET0 = 1;  // 开启定时器0使能
    TR0 = 1;  // 开启定时器0
}
