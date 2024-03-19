/*************************************************
*描述：
*      探头设置
*功能：
*      测量期间，按此键显示当前探头个数
*      测量结束，按此键设置探头个数
*      选择范围 1-8

**************************************************/
#include "STC15F2K60S2.h"
#include "absacc.h"
#include "config.h"
#include "intrins.h"

/*探头按键按下*/
void setinput()
{
    Flag_RefrPara = 0;   // 输入参数按键再次按下标志
    Flag_RefrRight = 0;  // 右移再次按下标志
    Flag_RefrTime = 0;   // 测量时间再次按下标志
    Incinput++;          // 探头个数标志
    if (Incinput > 16)   // 最多兼容15个探头
    {
        Incinput = 1;
    }
    /*显示设置的探头个数，最多个数为10个*/
    if (Flag_RefrInput == 0) {
        Flag_RefrInput = 1;
        Clear();      // 清除光标
        Lcd_Clear();  // LCD清屏
        if (Incinput < 9) {
            Txtext(104, 216, "设置测量探头共计：");
            Tnumber(522, 216, Incinput);
            Txtext(542, 216, "个");  //"个"
        } else {
            measure_flag = 0;
            Clear();
            Lcd_Clear();
            biaoding_input = Incinput - 8;
            Txtext(14, 216, "对应要标定的探头：第");
            Tnumber(514, 216, biaoding_input);
            Txtext(544, 216, "个");
        }
    } else {
        if (Incinput < 9) {
            measure_flag = 1;
            Clear();
            Lcd_Clear();
            Txtext(104, 216, "设置测量探头共计：");
            Tnumber(522, 216, Incinput);
            Txtext(542, 216, "个");
        } else if (Incinput > 8) {
            measure_flag = 0;
            Clear();
            Lcd_Clear();
            biaoding_input = Incinput - 8;
            Txtext(14, 216, "对应要标定的探头：第");
            Tnumber(514, 216, biaoding_input);
            Txtext(544, 216, "个");
        }
    }
}