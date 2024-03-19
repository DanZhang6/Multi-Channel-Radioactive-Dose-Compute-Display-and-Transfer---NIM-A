/*************************************************
*������
*     ��Ƭ������2����
*���ܣ�
*     ͨ������2����ָ���Һ��������ʾ

************************************************/
#include "STC15F2K60S2.h"
#include "intrins.h"
#include "config.h" 
#include "absacc.h"

#define RELOAD_COUNT  0xFA        //22.1184MHz,12T,SMOD=0,115200bps

/*************************
*   ����1��ʼ��   DCS��15
**************************/
void serial_port_one_initial()
{
    //SCON = 0x50;              //0101,0000 8λ�ɱ䲨���ʣ�����żУ��λ
    //BRT = RELOAD_COUNT;       //BRTR = 1, S1BRS = 1, EXTRAM = 1 ENABLE EXTRAM
    //AUXR = 0x15;              // T0x12,T1x12,UART_M0x6,BRTR,S2SMOD,BRTx12,EXTRAM,S1BRS
    //ES = 1;                   //�������ж�
    //EA = 1;                   //�����ж�
	SCON = 0x50;		//8λ����,�ɱ䲨����
	AUXR |= 0x40;		//��ʱ��1ʱ��ΪFosc,��1T
	AUXR &= 0xFE;		//����1ѡ��ʱ��1Ϊ�����ʷ�����
	TMOD &= 0x0F;		//�趨��ʱ��1Ϊ16λ�Զ���װ��ʽ
	TL1 = 0x00;		//�趨��ʱ��ֵ
	TH1 = 0xF7;		//�趨��ʱ��ֵ
	ET1 = 0;		//��ֹ��ʱ��1�ж�
	TR1 = 1;		//������ʱ��1
}

/*************************
*   ����2��ʼ��
**************************/
void serial_port_two_initial()
{
	//AH1*��Ϊ15ϵ�д��ڶ�����
    //S2CON = 0x50;             //0101,0000 8λ�ɱ䲨���ʣ�����żУ��λ,�������
    //BRT = RELOAD_COUNT;       // BRTR = 1, S1BRS = 1, EXTRAM = 0 ENABLE EXTRAM
    //AUXR = 0x15;              // T0x12,T1x12,UART_M0x6,BRTR,S2SMOD,BRTx12,EXTRAM,S1BRS
    //IE2 = 0x01;               //������2�ж�,ES2=1
    //EA = 1;                   //�����ж�
	S2CON = 0x50;		//8λ����,�ɱ䲨����
	AUXR |= 0x04;		//��ʱ��2ʱ��ΪFosc,��1T
	T2L = 0xD0;		//�趨��ʱ��ֵ
	T2H = 0xFF;		//�趨��ʱ��ֵ
	AUXR |= 0x10;		//������ʱ��2
}

/**************************
*   ����1�ж�
***************************/
void UART_one_Interrupt_Receive(void) interrupt 4
{
    //if(TI)
    //{
    //  return;
	//}
	uchar data k = 0;
    k = SCON ;
    k = k&0x10;              //S1TI
    if(k == 1)       
    {
     return;    
    }
}
/**************************
*   ����2�ж�
***************************/
void UART_two_Interrupt_Receive(void) interrupt 8
{
    uchar data k = 0;
    k = S2CON ;
    k = k&0x10;              //S2TI
    if(k == 1)       
    {
     return;    
    }
}

