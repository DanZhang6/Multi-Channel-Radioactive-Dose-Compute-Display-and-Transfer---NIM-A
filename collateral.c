/************************************************************
*������
*     ���д�������
*���ܣ�
*     NIM-A��NIM-B֮�����ݴ��䣬ͨ��������ʾ�ӻ��ļ������

***********************************************************/
#include "STC15F2K60S2.h"
#include "intrins.h"
#include "config.h"
#include "absacc.h"

bit	FlagCollErr;								//Ĭ��0
bit FlagColling ;								//Ĭ��0
uint  CntColTimer;								//Ĭ��0

//#define receiveradd XBYTE[0xd000]               //���д�����յ�ַ
//#define sendadd     XBYTE[0xd800]               //���д��䷢�͵�ַ
#define TURE 1
#define FALSE 0

extern void display_b();
uchar ChkData(uchar *pub) ;
/***********************************
*        �����������
***********************************/
uchar CKMasterCmd()
{
    volatile uchar var;
	do
	{
	    if(FlagCollFall)              //�ȴ�����1S,���ӻ�ͨ��ʧ��
		{
			FlagCollFall = 0;
			Flag_Collateral = 0;
			CmOverTime = 0;
			return FALSE;
		}
		var=receiveradd;
	}
	while(var!=0x00);               //�ȴ������Ŀ�ʼ����0x55
	if(var==0x00)
	{
	    FlagCollFall = 0;
		Flag_Collateral = 0;
		CmOverTime = 0;
		return TURE;
	}
	else
	{
	    FlagCollFall = 0;
		Flag_Collateral = 0;
		CmOverTime = 0;
	    return FALSE;
	}

}
/*************************************
*       �ӻ���������
*************************************/
void Send_Word(uchar *pbuf,uint ibyte)
{
   uint i;
   bit  bAck0;

   if(CKMasterCmd())
   {
       //Lcd_Clear();
       //Txtext(100,182,"�ӻ����з�����...");
	   i=0;
	   while(i<ibyte)
	   {
	  		do
			{
				if(CKMasterCmd()==FALSE)            //�ж����������Ƿ���0x55
                {
				     //Lcd_Clear();
				     //Txtext(100,182,"���ӻ��޷�ͨ��");
                     return;                        //����0x55������������
				}
				bAck0=Ack0;
			}
			while(bAck0==0);                        //�ȴ�������������״̬ˢ��
            sendadd=pbuf[i];                        //��������״̬ˢ�£��ӻ������͵�ַ������
			Note1=0;                                //���ݷ������
	        do
			{
				if(CKMasterCmd()==FALSE)            //�ж����������Ƿ���0x55
                {
                     return;                        //����0x55������������
				}
				bAck0=Ack0;
			}
			while(bAck0==1);                        //�ȴ����������������
            Note1=1;
			i++;
	   }
   }
   else
   {
      Lcd_Clear();
	  Tnumber(100,82,receiveradd);
      Txtext(100,182,"�ӻ�������������ʧ��");
	}
}

/*************************************
*       �����ɼ�����
*************************************/
void Collect_Word(uchar *pub,uint nbyte)     //�����ɼ�����
{
	  uint i,t;
    bit bNote0;
    float temp,temp1;
	  sendadd=0x00;                         //���ӻ����Ϳ�ʼָ��
	  //Lcd_Clear();
	  //Txtext(100,182,"�������н�����...");
	  Ack1=1;
    i=0;
	  FlagColling = 0;
	  while(i<nbyte)
	  {
      CntColTimer = 0;                    //�ֽ�֮�䶨ʱ�жϴ�������,���¼�ʱ
      do
      {
        if(FlagCollErr == 1)            //�ȴ�����0.5S
        {
          FlagCollErr = 0;             //��ʱ��־����
          FlagColling = 0;             //�ֽ�֮�䶨ʱ����
          CntColTimer = 0;             //�ֽ�֮�䶨ʱ�жϴ�������
          FlagMasColSlavOK = 0;        //����˸��ʱ��־���㣬�Ʋ���Ҫ��˸
          Flag_Warn_Led = 0;            //add 5.24
          return;
        }
        bNote0=Note0;
      }
      while(bNote0==1);                   //�ȴ��ӻ�ˢ������
      pub[i]=receiveradd;                 //�ӻ�ˢ�������ݣ������ӽ��յ�ַ������ֽ�
      Ack1=0;	                            //�����ֽڽ������
      i++;
      do
      {
        if(FlagCollErr == 1)            //�ȴ�����0.5S
        {
          FlagCollErr = 0;             //��ʱ��־����
          FlagColling = 0;             //�ֽ�֮�䶨ʱ����
          CntColTimer = 0;             //�ֽ�֮�䶨ʱ�жϴ�������
          FlagMasColSlavOK = 0;        //����˸��ʱ��־���㣬�Ʋ���Ҫ��˸
          Flag_Warn_Led = 0;            //add 5.24
          return;
        }
        bNote0=Note0;
      }
      while(bNote0==0);                   //need define
      Ack1=1;
	  }
    FlagColling = 0;                     //��ʱ��־����
    CntColTimer = 0;                     //�ֽ�֮�䶨ʱ����
    FlagCollErr = 0;                     //�ֽ�֮�䶨ʱ�жϴ�������
    if(PCOLSIG==0)                       //Ҫ����ʾ�ӵ����ݰ�������   add4.21
    {
	       shortdelay(200);
	       display_b();                      //��ʾ�ӻ�����
		   for(t=0;t<7;t++)				//0-6��7������
		   {
         temp=(float)(receive_buf[8*t]*100)+(float)(receive_buf[8*t+1]*10)+(float)(receive_buf[8*t+2]);
         temp+=((float)receive_buf[8*t+3]/10.0)+((float)receive_buf[8*t+4]/100.0)+((float)receive_buf[8*t+5]/1000.0);
         if(receive_buf[8*t+6]==2)
         {
           temp*=1000;
         }
         else if(receive_buf[8*t+6]==3)
         {
           temp*=1000000;
         }
         /*****DCS��������׼��****/
         if(temp<0)                                                 //���������С������DCS����0
         {
           DCS_Send[t*4+32+4]=0x30;
           DCS_Send[t*4+32+5]=0x30;
           DCS_Send[t*4+32+6]=0x30;
           DCS_Send[t*4+32+7]=0x30;
         }
         else if(temp>=0)
         {
           if(temp<1)
           {
             temp1=(float)temp*1000;
             DCS_Send[t*4+32+4]=(uchar)((int)temp1/100)+0x30;
             DCS_Send[t*4+32+5]=(uchar)((int)temp1%100/10)+0x30;
             DCS_Send[t*4+32+6]=(uchar)((int)temp1%100%10)+0x30;
             DCS_Send[t*4+32+7]=0x30;
           }
           else if((temp>=1)&&(temp<10))
           {
             temp1=(float)temp*100;
             DCS_Send[t*4+32+4]=(uchar)((int)temp1/100)+0x30;
             DCS_Send[t*4+32+5]=(uchar)((int)temp1%100/10)+0x30;
             DCS_Send[t*4+32+6]=(uchar)((int)temp1%100%10)+0x30;
             DCS_Send[t*4+32+7]=0x31;
           }
           else if((temp>=10)&&(temp<100))
           {
             temp1=(float)temp*10;
             DCS_Send[t*4+32+4]=(uchar)((int)temp1/100)+0x30;
             DCS_Send[t*4+32+5]=(uchar)((int)temp1%100/10)+0x30;
             DCS_Send[t*4+32+6]=(uchar)((int)temp1%100%10)+0x30;
             DCS_Send[t*4+32+7]=0x32;
           }
           else if((temp>=100)&&(temp<1000))
           {
             temp1=(float)temp;
             DCS_Send[t*4+32+4]=(uchar)((int)temp1/100)+0x30;
             DCS_Send[t*4+32+5]=(uchar)((int)temp1%100/10)+0x30;
             DCS_Send[t*4+32+6]=(uchar)((int)temp1%100%10)+0x30;
             DCS_Send[t*4+32+7]=0x33;
           }
           else if((temp>=1000)&&(temp<10000))
           {
             temp1=(float)temp/10.0;
             DCS_Send[t*4+32+4]=(uchar)((int)temp1/100)+0x30;
             DCS_Send[t*4+32+5]=(uchar)((int)temp1%100/10)+0x30;
             DCS_Send[t*4+32+6]=(uchar)((int)temp1%100%10)+0x30;
             DCS_Send[t*4+32+7]=0x34;
           }
           else if((temp>=10000)&&(temp<100000))
           {
             temp1=(float)temp/100.0;
             DCS_Send[t*4+32+4]=(uchar)((int)temp1/100)+0x30;
             DCS_Send[t*4+32+5]=(uchar)((int)temp1%100/10)+0x30;
             DCS_Send[t*4+32+6]=(uchar)((int)temp1%100%10)+0x30;
             DCS_Send[t*4+32+7]=0x35;
           }
           else if((temp>=100000)&&(temp<1000000))
           {
             temp1=(float)temp/1000.0;
             DCS_Send[t*4+32+4]=(uchar)((int)temp1/100)+0x30;
             DCS_Send[t*4+32+5]=(uchar)((int)temp1%100/10)+0x30;
             DCS_Send[t*4+32+6]=(uchar)((int)temp1%100%10)+0x30;
             DCS_Send[t*4+32+7]=0x36;
           }
           else if((temp>=1000000)&&(temp<10000000))
           {
             temp1=(float)temp/10000.0;
             DCS_Send[t*4+32+4]=(uchar)((int)temp1/100)+0x30;
             DCS_Send[t*4+32+5]=(uchar)((int)temp1%100/10)+0x30;
             DCS_Send[t*4+32+6]=(uchar)((int)temp1%100%10)+0x30;
             DCS_Send[t*4+32+7]=0x37;
           }
           else if((temp>=10000000)&&(temp<100000000))
           {
             temp1=(float)temp/100000.0;
             DCS_Send[t*4+32+4]=(uchar)((int)temp1/100)+0x30;
             DCS_Send[t*4+32+5]=(uchar)((int)temp1%100/10)+0x30;
             DCS_Send[t*4+32+6]=(uchar)((int)temp1%100%10)+0x30;
             DCS_Send[t*4+32+7]=0x38;
           }
           else if((temp>=100000000)&&(temp<1000000000))
           {
             temp1=(float)temp/1000000.0;
             DCS_Send[t*4+32+4]=(uchar)((int)temp1/100)+0x30;
             DCS_Send[t*4+32+5]=(uchar)((int)temp1%100/10)+0x30;
             DCS_Send[t*4+32+6]=(uchar)((int)temp1%100%10)+0x30;
             DCS_Send[t*4+32+7]=0x39;
           }
         }
         if(receive_buf[8*t+7])
         {
           DCS_Send[65]=DCS_Send[65]|Svar1[t];
         }
         else
         {
           DCS_Send[65]=DCS_Send[65]&Svar0[t];
         }
		   }
		   FlagMasColSlavOK = 1;             //ָʾ����˸��ʱ��ʼ��־
     }
}
