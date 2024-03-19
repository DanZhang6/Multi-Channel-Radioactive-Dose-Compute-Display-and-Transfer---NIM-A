/************************************************************
*������
*     ��ʱ��0����
*���ܣ�
*     ��ʱ��0�����ڷ�ʽ1��ÿ5ms�ж�һ�Σ���ʱ��һֱ����

***********************************************************/
#include "STC15F2K60S2.h"
#include "absacc.h"
#include "config.h"
#include "intrins.h"
uint NumT0;             // 8253��ʱ�жϴ���,�ڰ��²����������ʼ��Ϊ0;
uint CntFlhWarnT0;      // �����ɼ�����ʱ����˸��ʱ�жϴ���
uint CntWarn;           // LED����˸��ʱ�жϴ���
uint CntS1Warn;         // ������ʾ���ϵ���˸��ʱ�жϴ���
uint CmOverTime;        // ���ӻ�ͨ�����ֶ�ʱ�жϴ���
bit Flag_Warn_Serial1;  // ������ʾ�ƶ�ʱ������ʶ
bit Flag_Tim0;          // ��ʱ��0������־��Ĭ��0��
bit FlagCollFall;       // ����ͨ��ʧ�ܱ�־��0
/*************************************
 *         ��ʱ��0�жϳ���
 **************************************/
void Tim0(void) interrupt 1  // tim0��ʱ�����������ն˳���(��ʱ�ж�)
{
    TF0 = 0;     // ���ö�ʱ�����ƶ�ʱ��,���ö�ʱ����ֵ
    TH0 = 0xDC;  // ��ʱ��0ÿ5ms�ж�һ��
    TL0 = 0x00;
    if (Flag_Mast)  // �����ӻ���ťΪ����
    {
        if (FlagMasColSlavOK)  // �����ɼ�����ʱ����˸��ʱ
        {
            if (++CntFlhWarnT0 >= TimeFiveHundred)  // TimeFiveHundred=100,��ʱ��5ms�ж�һ��,��������500msˢ��һ��
            {
                CntFlhWarnT0 = 0;
                Flag_Warn_Led = 1;  // ��״̬��ʼ�任��־
            }
        }
    }
    if (Flag_Warn_Count)  // ��������ʱ����˸��ʱ
    {
        if (++CntWarn >= TimeFiveHundred)  // 500msˢ��һ��
        {
            CntWarn = 0;
            Flag_Warn_Flash = 1;  // ��״̬��ʼ�任��־
        }
    }
    if (Flag8253Cnting)  // 8253������ʱ��ʼ��־�ж�,���²���������=1 ��ʼ��ʱ
    {
        if (NumT0 == 0) {
            GATE = 1;
        }
        NumT0 = ++NumT0;
        if (NumT0 >= (200 * Refresh_Time))  // AA1*,1S���ж�200�Σ�Redrawtimeʱ�����ж�200*Redrawtime��,���ж�1���ִ��,�ر�8253�ſ��ź�
        {
            GATE = 0;  // �ر�8253�ſ��ź�
            NumT0 = 0;
            Flag_Tim0 = 1;       // ��ʱ��������־����main�����в��ϼ���Ƿ�Ϊ1��Ϊһʱ����ѭ��
            Flag8253Cnting = 0;  // ��ʱ������־����
        }
    }
    if (FlagColling)  // ���ݲɼ����ʱ�䶨ʱ
    {
        if (++CntColTimer >= TimeOneHundred)  // 100msˢ��һ��
        {
            FlagCollErr = 1;
            CntColTimer = 0;
        }
    }
    if (Flag_Collateral)  // ���ӻ�ͨ�����ֶ�ʱ
    {
        if (++CmOverTime >= TimeThreeThousand) {
            FlagCollFall = 1;
            CmOverTime = 0;
        }
    }
}

/*************************************
 *         ��ʱ��0��ʼ��
 **************************************/
void Init_Time0()
{
    TMOD = 0x01;  // ��ʱ��0�����ڷ�ʽ1
    TH0 = 0xDC;   // Ԥ�ö�ʱ��ֵDC00H,����Ƶ��22.1184MHz,��ʱʱ��Լ5ms
    TL0 = 0x00;
    NumT0 = 0;  // �жϼ�����ʼ��
    CntFlhWarnT0 = 0;
    CntWarn = 0;
    CntS1Warn = 0;
    CmOverTime = 0;
    CntColTimer = 0;
    EA = 1;   // �������ж�
    ET0 = 1;  // ������ʱ��0ʹ��
    TR0 = 1;  // ������ʱ��0
}
