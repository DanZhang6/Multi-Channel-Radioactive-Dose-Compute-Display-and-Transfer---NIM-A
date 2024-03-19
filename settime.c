/*************************************************
*描述：
*      时间设置
*功能：
*      设定计数器测量时间；

**************************************************/
#include "STC15F2K60S2.h"
#include "absacc.h"
#include "config.h"
#include "intrins.h"

uchar Inctime;                 // 时间按键增加标志
double Real_Count_Display[8];  // AA1*原为Redrawtime
uchar Setted_Time[8];          // AB1+
uchar Init_Time = 10;          // AD1*默认时间为10s

void settime()
{
    Flag_RefrInput = 0;  // 探头个数按键再次按下标志
    Flag_RefrPara = 0;   // 输入参数按键再次按下标志
    Flag_RefrRight = 0;  // 右移按键再次按下标志
    Inctime++;           // 按时间输入键次数，则测量时间改变5s或者60s
    /*显示设置时间  秒*/
    if (Flag_RefrTime == 0) {
        Flag_RefrTime = 1;
        Clear();
        Lcd_Clear();
        Txtext(128, 216, "时间设置：");
        Tnumber(368, 216, 0);  // 时间修改到5秒，修改时间2012.6.15
        Tnumber(416, 216, 0);
        Tnumber(464, 216, 5);
        Txtext(512, 216, "秒");
        Init_Time = 5;  // AA1* 原为Rearawtime
    } else {
        if ((Inctime == 1) || (Inctime >= 8))  // AA1*,>=11,原最大时间120s但占空间太大
        {
            Inctime = 1;
            Init_Time = 5;  // AA1* 原为Rearawtime
            Tnumber(368, 216, 0);
            Tnumber(416, 216, 0);
            Tnumber(464, 216, 5);  // 刷新时间为5秒
        } else if (Inctime == 2) {
            Init_Time = 10;  // AA1* 原为Rearawtime
            Tnumber(368, 216, 0);
            Tnumber(416, 216, 1);
            Tnumber(464, 216, 0);
        } else if (Inctime == 3) {
            Init_Time = 20;  // AA1* 原为Rearawtime
            Tnumber(368, 216, 0);
            Tnumber(416, 216, 2);
            Tnumber(464, 216, 0);
        } else if (Inctime == 4) {
            Init_Time = 30;  // AA1* 原为Rearawtime
            Tnumber(368, 216, 0);
            Tnumber(416, 216, 3);
            Tnumber(464, 216, 0);
        } else if (Inctime == 5) {
            Init_Time = 40;  // AA1* 原为Rearawtime
            Tnumber(368, 216, 0);
            Tnumber(416, 216, 4);
            Tnumber(464, 216, 0);
        } else if (Inctime == 6) {
            Init_Time = 50;  // AA1* 原为Rearawtime
            Tnumber(368, 216, 0);
            Tnumber(416, 216, 5);
            Tnumber(464, 216, 0);
        } else if (Inctime == 7) {
            Init_Time = 60;  // AA1* 原为Rearawtime
            Tnumber(368, 216, 0);
            Tnumber(416, 216, 6);
            Tnumber(464, 216, 0);
        }
    }
}
