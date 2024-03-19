/********************************************************************
*描述：
*     处理主机显示从机数据报警
*功能：
*     LED指示灯（屏幕上）闪烁报警，直到下次数据更新，再重新判断灯闪烁

*********************************************************************/
#include "STC15F2K60S2.h"
#include "absacc.h"
#include "config.h"
#include "intrins.h"
#include "math.h"
#include "stdio.h"
uchar receive_buf[57];    // 接收从机数据
uchar Flag_need_warn[8];  // 需要报警标志，初始0
uchar State_On_Off[8];    // 指示灯状态标志.初始0

void Warning_Disp(uchar Num, uchar Flag, uchar State);
void Updata_Led(uchar j);

/*************************************
 *         显示从机计数率
 *************************************/
void display_b()
{
    uchar idata j;
    Lcd_Clear();
    for (j = 0; j < receive_buf[56]; j++) {
        /********剂量率显示**********/
        Txtext(60, 42 + (j * 58), "探头");
        Tnumber(160, 42 + (j * 58), j + 9);
        Txtext(210, 42 + (j * 58), ":");
        if (receive_buf[8 * j] != 0) {
            Tnumber(230, 42 + (j * 58), receive_buf[8 * j]);
            Tnumber(270, 42 + (j * 58), receive_buf[8 * j + 1]);
            Tnumber(310, 42 + (j * 58), receive_buf[8 * j + 2]);
            Txtext(350, 42 + (j * 58), ".");
            Tnumber(390, 42 + (j * 58), receive_buf[8 * j + 3]);
            Tnumber(430, 42 + (j * 58), receive_buf[8 * j + 4]);
            Tnumber(470, 42 + (j * 58), receive_buf[8 * j + 5]);
        } else {
            if (receive_buf[8 * j + 1] != 0) {
                Tnumber(270, 42 + (j * 58), receive_buf[8 * j + 1]);
                Tnumber(310, 42 + (j * 58), receive_buf[8 * j + 2]);
                Txtext(350, 42 + (j * 58), ".");
                Tnumber(390, 42 + (j * 58), receive_buf[8 * j + 3]);
                Tnumber(430, 42 + (j * 58), receive_buf[8 * j + 4]);
                Tnumber(470, 42 + (j * 58), receive_buf[8 * j + 5]);
            } else {
                Tnumber(310, 42 + (j * 58), receive_buf[8 * j + 2]);
                Txtext(350, 42 + (j * 58), ".");
                Tnumber(390, 42 + (j * 58), receive_buf[8 * j + 3]);
                Tnumber(430, 42 + (j * 58), receive_buf[8 * j + 4]);
                Tnumber(470, 42 + (j * 58), receive_buf[8 * j + 5]);
            }
        }
        /**********计量单位显示***********/
        if (receive_buf[8 * j + 6] == 1) {
            Txtext(510, 42 + (j * 58), "Bq/L");
        } else if (receive_buf[8 * j + 6] == 2) {
            Txtext(510, 42 + (j * 58), "KBq/L");
        } else if (receive_buf[8 * j + 6] == 3) {
            Txtext(510, 42 + (j * 58), "MBq/L");
        }
        if (receive_buf[8 * j + 7] == 1) {
            WarnLed_On(5, 42 + (j * 58));  // 修改
            Flag_need_warn[j] = 1;
            State_On_Off[j] = 1;
        } else if (receive_buf[8 * j + 7] == 0) {
            WarnLed_Off(0, 42 + (j * 58));  // 修改
            Flag_need_warn[j] = 0;
            State_On_Off[j] = 0;
        }
    }
}

/***********************************
 *          灯闪烁
 ************************************/
void RedLed_Flash(void)
{
    uchar idata i;
    for (i = 0; i < receive_buf[56]; i++) {
        Updata_Led(i);
        Warning_Disp(i, Flag_need_warn[i], State_On_Off[i]);
    }
}

/***********************************
 *          更新指示灯状态
 ************************************/
void Updata_Led(uchar j)
{
    if (Flag_need_warn[j]) {
        if (State_On_Off[j]) {
            State_On_Off[j] = 0;
        } else {
            State_On_Off[j] = 1;
        }
    } else
        State_On_Off[j] = 0;
}
/************************************
 *          警告显示
 *************************************/
void Warning_Disp(uchar Num, uchar Flag, uchar State)
{
    if (Flag) {
        if (State) {
            WarnLed_On(5, 42 + (Num * 58));  // 修改
            return;
        }
    }
    WarnLed_Off(0, 42 + (Num * 58));  // 修改
}
