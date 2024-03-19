/*************************************************
*������
*      ʱ������
*���ܣ�
*      �趨����������ʱ�䣻

**************************************************/
#include "STC15F2K60S2.h"
#include "absacc.h"
#include "config.h"
#include "intrins.h"

uchar Inctime;                 // ʱ�䰴�����ӱ�־
double Real_Count_Display[8];  // AA1*ԭΪRedrawtime
uchar Setted_Time[8];          // AB1+
uchar Init_Time = 10;          // AD1*Ĭ��ʱ��Ϊ10s

void settime()
{
    Flag_RefrInput = 0;  // ̽ͷ���������ٴΰ��±�־
    Flag_RefrPara = 0;   // ������������ٴΰ��±�־
    Flag_RefrRight = 0;  // ���ư����ٴΰ��±�־
    Inctime++;           // ��ʱ������������������ʱ��ı�5s����60s
    /*��ʾ����ʱ��  ��*/
    if (Flag_RefrTime == 0) {
        Flag_RefrTime = 1;
        Clear();
        Lcd_Clear();
        Txtext(128, 216, "ʱ�����ã�");
        Tnumber(368, 216, 0);  // ʱ���޸ĵ�5�룬�޸�ʱ��2012.6.15
        Tnumber(416, 216, 0);
        Tnumber(464, 216, 5);
        Txtext(512, 216, "��");
        Init_Time = 5;  // AA1* ԭΪRearawtime
    } else {
        if ((Inctime == 1) || (Inctime >= 8))  // AA1*,>=11,ԭ���ʱ��120s��ռ�ռ�̫��
        {
            Inctime = 1;
            Init_Time = 5;  // AA1* ԭΪRearawtime
            Tnumber(368, 216, 0);
            Tnumber(416, 216, 0);
            Tnumber(464, 216, 5);  // ˢ��ʱ��Ϊ5��
        } else if (Inctime == 2) {
            Init_Time = 10;  // AA1* ԭΪRearawtime
            Tnumber(368, 216, 0);
            Tnumber(416, 216, 1);
            Tnumber(464, 216, 0);
        } else if (Inctime == 3) {
            Init_Time = 20;  // AA1* ԭΪRearawtime
            Tnumber(368, 216, 0);
            Tnumber(416, 216, 2);
            Tnumber(464, 216, 0);
        } else if (Inctime == 4) {
            Init_Time = 30;  // AA1* ԭΪRearawtime
            Tnumber(368, 216, 0);
            Tnumber(416, 216, 3);
            Tnumber(464, 216, 0);
        } else if (Inctime == 5) {
            Init_Time = 40;  // AA1* ԭΪRearawtime
            Tnumber(368, 216, 0);
            Tnumber(416, 216, 4);
            Tnumber(464, 216, 0);
        } else if (Inctime == 6) {
            Init_Time = 50;  // AA1* ԭΪRearawtime
            Tnumber(368, 216, 0);
            Tnumber(416, 216, 5);
            Tnumber(464, 216, 0);
        } else if (Inctime == 7) {
            Init_Time = 60;  // AA1* ԭΪRearawtime
            Tnumber(368, 216, 0);
            Tnumber(416, 216, 6);
            Tnumber(464, 216, 0);
        }
    }
}
