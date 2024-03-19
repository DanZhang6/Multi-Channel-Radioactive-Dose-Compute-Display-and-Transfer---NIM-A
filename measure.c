/*************************************************
*������
*    ������������
*���ܣ�
*    1.ѡ��̽ͷ��������ʼ��8253����������ʱ����
*    2.��ʾʱ���̽ͷ���ã��ȴ���ʱ����
*    3.��ʱ����ˢ����Ļ����ʾ�����ʣ�

**************************************************/
#include "STC15F2K60S2.h"
#include "intrins.h"
#include "config.h" 
#include "absacc.h"	 

void ShowSetResult();

void measure()
{   
     Signal1=Var_Signal1;                   //ѡ��̽ͷ
     Signal2=Var_Signal2;
     Signal3=Var_Signal3; 
     ShowSetResult(); 					   //��ʾ��ǰ�������
     if((Flag_ParaChange == 1)||(Flag_InputChange == 1))   //̽ͷ��������޸�
     {
	     Flag_ParaChange = 0;
   	     Flag_InputChange = 0;
	  /*������Ĳ����洢��AT24C16*/
	     if(Incinput<9)	 
            dt_in[330]=Incinput;			//IIC�洢����(̽ͷ������־)
	     else								//ע�⵽����ǿ��̽ͷ��������С��9
		 {
	  	    Incinput=8;
			dt_in[330]=Incinput;
		 }
	     DS_SaveData(dt_in);                 //�ѵ���ϵ�������ݱ��浽AT24C16оƬ�� 
     }
	 jishucount=0;
	 Init_8253();                             //8253��ʼ��     
}

void ShowSetResult()
{
  Flag_RefrInput=0;
  Flag_RefrPara=0;
  Flag_RefrRight=0;
  Flag_RefrTime=0;
  Clear();
  Lcd_Clear();
  if(measure_flag==1)						//��û�а�̽ͷ�궨����
  {
     Txtext(102,98,"̽ͷ���ƣ�");
     Tnumber(342,98,Incinput);
     Txtext(390,98,"��");
     Txtext(102,166,"����ʱ�䣺");
     Tnumber(342,166,3);				//AA1*ԭΪRedrawtime;
     Txtext(390,166,"��");
	 Txtext(102,234,"̽ͷ��ʼ���뾲��30��");
     Txtext(102,302,"���в�����..."); 
  }
  else
  {
     Txtext(152,148,"�궨��������");
     Tnumber(450,148,biaoding_input);
     Txtext(484,148,"��̽ͷ");
     Txtext(152,216,"����ʱ�䣺");
     Tnumber(392,216,Init_Time);			//AA1*ԭΪRedrawtime;
     Txtext(480,216,"��");
     Txtext(152,284,"���б궨������..."); 
  } 
 }
