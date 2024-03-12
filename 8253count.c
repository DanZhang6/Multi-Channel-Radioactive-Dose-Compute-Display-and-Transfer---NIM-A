/********************************************************************************************
*������
*   8253��������
*���ܣ�
*   1.��8253�ж�ȡ������ÿ��8253�к���3�������������8253��15��̽ͷ�ļ�����
*     ǰ�߸�̽ͷ��˫̽ͷ���ͼ���̽ͷ�͸߼���̽ͷ�����ڰ˸�̽ͷ�ǵ�̽ͷ���߼���̽ͷ����
*     ���ݼ�����ѡ���ĸ�̽ͷ������
*   2.T0��ʱ�ж�
*     T0������16λ�Զ�����ģʽ�����ϼ�����ÿѭ��һ�Σ��ܶ�ʱ5Ms,������ֵDC00������22.1184M��
*   3.��ʱ������ȡ�����������趨�Ĳ�������ʾ��̽ͷ�ļ����ʣ�
*   4.���ݼ����ʵĲ�ͬ��ѡ��ͬ�ĵ�λ��uGy/s,mGy/s,Gy/s��;

***********************************************************************************************/
#include "STC15F2K60S2.h"
#include "intrins.h"
#include "config.h"
#include "absacc.h"
#include "stdio.h"
#include "math.h"

#define C82531C XBYTE[0x8300]                                       //8253������˿ڣ���ַ����CS4=1,CS3=1,CS2=1,CS1=1,CS0=0,A1A0=11��
#define C825310D XBYTE[0x8000]                                      //������0CS4=1,CS3=1,CS2=1,CS1=1,CS0=0,A1A0=00��
#define C825311D XBYTE[0x8100]                                      //������1CS4=1,CS3=1,CS2=1,CS1=1,CS0=0,A1A0=01;
#define C825312D XBYTE[0x8200]                                      //������2CS4=1,CS3=1,CS2=1,CS1=1,CS0=0,A1A0=10;

#define C82532C XBYTE[0x8b00]                                       // XBYTE�������ǽ��ⲿI/O�˿���Ϊ�������ڲ���16λ�����ƣ���Ϊ����
#define C825320D XBYTE[0x8800]                                      //ʹ�ܶˣ� XBYTE�����û�������ʹ�ܶ˺��Զ�����wr,rd.
#define C825321D XBYTE[0x8900]
#define C825322D XBYTE[0x8a00]

#define C82533C XBYTE[0x9300]
#define C825330D XBYTE[0x9000]
#define C825331D XBYTE[0x9100]
#define C825332D XBYTE[0x9200]

#define C82534C XBYTE[0x9b00]
#define C825340D XBYTE[0x9800]
#define C825341D XBYTE[0x9900]
#define C825342D XBYTE[0x9a00]

#define C82535C XBYTE[0xa300]
#define C825350D XBYTE[0xa000]
#define C825351D XBYTE[0xa100]
#define C825352D XBYTE[0xa200]

uchar buf[32];                                                                    //AB1*��8253�ж����ļ��������ݣ�15�����������Ͱ�λ�߰�λ����3-32��1Ϊ̽ͷ������2Ϊ����ʱ��
uchar Channel_Detector[8][2];                                                     //AC1+ͨ��̽ͷѡ���־��1:������0:������
uchar DataGe[75];                                                                 //̽�����궨�����ĸ�λ����ʼʱ��ʮ��ǧ��Ϊ0
uchar DataTenth[75];                                                              //̽�����궨������ʮ��λ
uchar DataCent[75];                                                               //̽�����궨�����İٷ�λ
uchar DataThouth[75];                                                             //̽�����궨������ǧ��λ
uchar send_buf[65];                                                               //NIM_A��NIM_B�������ݵ�����
uchar Incinput;                                                                   //̽ͷ������־��Ĭ����ʾ̽ͷ����Ϊ8��
float idata Para[10];                                                             //�趨�õĲ�������
float DoseRata[8];                                                                //ÿ��̽ͷ��õļ�����
uchar Channel_Display[8];															//������ʾ��ͨ������
float jishuguan_DoseRata;
float dianlishi_DoseRata;
uchar Calculated[8];                                                              //�Ƿ�������־
uchar Flag_dw;                                                                    //��λ��־
uchar Max_Time;                                                                   //AA1+����ͨ����ļ���ʱ��
uint Tdata;                                                                 //̽ͷ������
uint Real_Count[8];
ulong Count[8][3];																//һ�������ʷ
uint idata jishuguan_data;                                                        //�궨ʱ�ļ����ܼ���ֵ�൱��Tdata
uint idata dianlishi_data;                                                        //�궨ʱ�ĵ����Ҽ���ֵ�൱��Tdata
uchar idata Var_Signal1;                                                          //̽ͷ�����ź�1���Ӽ����ܣ�Ĭ��ff���Ӽ�����
uchar idata Var_Signal2;                                                          //̽ͷ�����ź�2���ӵ����ң�Ĭ��00��
uchar idata Var_Signal3;                                                          //̽ͷ�����ź�3���ӵ����ң�Ĭ��00��
uchar idata Var_Led;                                                              //LED����ָʾ��,Ϊ1ʱ��,��ֵ0xff,
uchar biaoding_input;
bit Flag_Warn_Count;                                                              //Ĭ��0��
uchar Flag_need_Flash[8];                                                         //LED��˸��־
uchar State_Flash[8];
uchar count_change_flag[8];                                                       //�����ı��־��
float code a0=0.0625;
float code a1=0.0625;
float code a2=0.125;
float code a3=0.25;
float code a4=0.5;
uchar code Svar1[8]={0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80};                   //�ź�����
uchar code Svar0[8]={0xFE,0xFD,0xFB,0xF7,0xEF,0xDF,0xBF,0x7F};
extern uchar jishucount;
extern uchar  Average_Times[8];													//AJ1+����ƽ������
extern uchar Display_Flag[8];

extern bit Speak_Alarm();



void shortdelay(uint i);
void Alarm();
void Led_Disp(uchar Num,uchar Flag,uchar State);
void Updata_Flash(uchar j);
extern bitSpeak_Alarm();
/*************************************
*8253����ֵ
**************************************/
void GetAndDisdata()                                                              //������ɺ��һ�����еĳ���
{
  uchar j;
  uchar th=0;
  uchar tl=0;                                                                        //buf�����ʼ��
  for(j=0;j<10;j++)
  {
    Para[j]=0;
  }
  for(j=0;j<8;j++)
  {
	Calculated[j]=0;
  }
  /*===========����8253�ļ���ֵ==========*/
  C82531C=0x84;                                                                   //�����һƬ8253������2�ļ���ֵ
  tl=C825312D;                                                                    //read825316bitsdata(firstLthenH)
  buf[2]=0xff-tl;
  th=C825312D;
  buf[3]=0xff-th;

  C82531C=0x44;                                                                   //�����һƬ8253������1�ļ���ֵ
  tl=C825311D;                                                                    //read825316bitsdata(firstLthenH)
  buf[4]=0xff-tl;
  th=C825311D;
  buf[5]=0xff-th;

  C82531C=0x04;                                                                   //�����һƬ8253������0�ļ���ֵ
  tl=C825310D;                                                                    //read825316bitsdata(firstLthenH)
  buf[6]=0xff-tl;
  th=C825310D;
  buf[7]=0xff-th;

  C82532C=0x84;                                                                   //����ڶ�Ƭ8253������2�ļ���ֵ
  tl=C825322D;                                                                    //read825316bitsdata(firstLthenH)
  buf[8]=0xff-tl;
  th=C825322D;
  buf[9]=0xff-th;

  C82532C=0x44;                                                                   //����ڶ�Ƭ8253������1�ļ���ֵ
  tl=C825321D;                                                                    //read825316bitsdata(firstLthenH)
  buf[10]=0xff-tl;
  th=C825321D;
  buf[11]=0xff-th;

  C82532C=0x04;                                                                   //����ڶ�Ƭ8253������0�ļ���ֵ
  tl=C825320D;                                                                    //read825316bitsdata(firstLthenH)
  buf[12]=0xff-tl;
  th=C825320D;
  buf[13]=0xff-th;

  C82533C=0x84;                                                                   //�������Ƭ8253������2�ļ���ֵ
  tl=C825332D;                                                                    //read825316bitsdata(firstLthenH)
  buf[14]=0xff-tl;
  th=C825332D;
  buf[15]=0xff-th;

  C82533C=0x44;                                                                   //�������Ƭ8253������1�ļ���ֵ
  tl=C825331D;                                                                    //read825316bitsdata(firstLthenH)
  buf[16]=0xff-tl;
  th=C825331D;
  buf[17]=0xff-th;

  C82533C=0x04;                                                                   //�������Ƭ8253������0�ļ���ֵ
  tl=C825330D;                                                                    //read825316bitsdata(firstLthenH)
  buf[18]=0xff-tl;
  th=C825330D;
  buf[19]=0xff-th;

  C82534C=0x84;                                                                   //�������Ƭ8253������2�ļ���ֵ
  tl=C825342D;                                                                    //read825316bitsdata(firstLthenH)
  buf[20]=0xff-tl;
  th=C825342D;
  buf[21]=0xff-th;

  C82534C=0x44;                                                                   //�������Ƭ8253������1�ļ���ֵ
  tl=C825341D;                                                                    //read825316bitsdata(firstLthenH)
  buf[22]=0xff-tl;
  th=C825341D;
  buf[23]=0xff-th;

  C82534C=0x04;                                                                   //�������Ƭ8253������0�ļ���ֵ
  tl=C825340D;                                                                    //read825316bitsdata(firstLthenH)
  buf[24]=0xff-tl;
  th=C825340D;
  buf[25]=0xff-th;

  C82535C=0x84;                                                                   //�������Ƭ8253������2�ļ���ֵ
  tl=C825352D;                                                                    //read825316bitsdata(firstLthenH)
  buf[26]=0xff-tl;
  th=C825352D;
  buf[27]=0xff-th;

  C82535C=0x44;                                                                   //�������Ƭ8253������1�ļ���ֵ
  tl=C825351D;                                                                    //read825316bitsdata(firstLthenH)
  buf[28]=0xff-tl;
  th=C825351D;
  buf[29]=0xff-th;

  C82535C=0x04;                                                                   //�������Ƭ8253������0�ļ���ֵ
  tl=C825350D;                                                                    //read825316bitsdata(firstLthenH)
  buf[30]=0xff-tl;
  th=C825350D;
  buf[31]=0xff-th;

  buf[0]=Incinput;                                                                //̽ͷ����
  buf[1]=Max_Time;                                                              //����ʱ�䣨ˢ��ʱ���־��
}
/*******************************************
*8253��������ʼ��
********************************************/
void Init_8253()
{
  uchar i;//AB1+
  //=======װ���ֵ�ٿ�ʼ����=======//
  C82531C=0xb4;                                                                   //10110100
  C825312D=0xff;                                                                  //10��ͨ��2,11���ȵ��ֽں���ֽڣ�010������ʽ2������0�������Ƽ�����д��ֵ0xff��ʼ����
  C825312D=0xff;

  C82531C=0x74;                                                                   //01110100
  C825311D=0xff;                                                                  //ͬ��
  C825311D=0xff;

  C82531C=0x34;                                                                   //00110100
  C825310D=0xff;                                                                  //д��ֵ�ȵ��ֽں���ֽ�82531д��ֵ��Ϳ�ʼ����
  C825310D=0xff;                                                                  //����������װ���ֵ

  C82532C=0xb4;                                                                   //10110100
  C825322D=0xff;
  C825322D=0xff;

  C82532C=0x74;                                                                   //01110100
  C825321D=0xff;
  C825321D=0xff;

  C82532C=0x34;                                                                   //00110100
  C825320D=0xff;
  C825320D=0xff;

  C82533C=0xb4;                                                                   //10110100
  C825332D=0xff;
  C825332D=0xff;

  C82533C=0x74;                                                                   //01110100
  C825331D=0xff;
  C825331D=0xff;

  C82533C=0x34;                                                                   //00110100
  C825330D=0xff;
  C825330D=0xff;

  C82534C=0xb4;                                                                   //10110100
  C825342D=0xff;
  C825342D=0xff;

  C82534C=0x74;                                                                   //01110100
  C825341D=0xff;
  C825341D=0xff;

  C82534C=0x34;                                                                   //00110100
  C825340D=0xff;
  C825340D=0xff;

  C82535C=0xb4;                                                                   //10110100
  C825352D=0xff;
  C825352D=0xff;

  C82535C=0x74;                                                                   //01110100
  C825351D=0xff;
  C825351D=0xff;

  C82535C=0x34;                                                                   //00110100
  C825350D=0xff;
  C825350D=0xff;
  for(i=0;i<67;i++)
  {
    DCS_Send[i]=0;
  }
  for(i=0;i<=7;i++)
  {
    DoseRata[i] = 0;						//ÿ��̽ͷ��õļ�����
  }
  Var_Led=0xff;                                                                   //ledָʾ
  Led573=Var_Led;
  EX0=0;                                                                          //���ⲿ�ж�0
  /*��ʱ�������ĳ�ʼ��*/                                                                         //AJ1-,�ƶ�����ʱ�ж��￪Gate,8253��������
  Flag8253Cnting=1;
  NumT0=0;
}
/************************************
*��ʾ����
************************************/
void ShowData()
{
  uchar i,m,j,k;                                                                    //AE1-:Var1�����ж�ʹ�õ�̽�����ľֲ��������൱��Channel_Detector,������Ҫ
  ulong count_temp,jishuguan_count_temp,dianlishi_count_temp;
  float idata temp,jtemp,yudecide,jishuguan_rata,jishuguan_jtemp,dianlishi_jtemp;
  double yu,yudata,mtemp;
  uchar Tbcd[12];                                                                 //��õ����ݵ�bcd������
  Lcd_Clear();
  /**����˵��:
  **1,Ϊ��֤��ʹ���ͨ������ͬһ���̣������̵�ͨ��Ҳ��������Ӧ����ĸ����ٶȣ�����һ�����һ�Σ�10�����Ϊʮ��һ����������
  **  ����������ʾһ��ˢ��һ�Ρ�
  **2,�����м����ʼ��㡢���̻���������������ʾ�����ַֿ�ִ��
  */
  for(i=0;i<=7;i++)                                                               //8��̽ͷ������
  {
      /**
       * 1.����ÿһ��̽ͷ��0~7����
       * 2.�ȴ�buf����8253��������ȡ�صļ������м����������8λ*256+��8λ����¼���μ���ֵ��ÿ3�����һ�η�ֹ�����
       * 3.����ÿ�εļ�����3S����ʱ�����Լ���ǰ���̺�Ԥ��ȷ���Ļ���Ƶ�ʣ��жϻ�����ִ�в���¼�������̵�λ��Ϣ��
       * 4.��Ҫ������ʷ�ĵ�λ��Ϣ�����жϣ�������Ĳ����벻�����Ĳ�����ͬ���ʷֿ�ִ�У�
       * 5.
       **/
		if(Channel_Detector[i][0]==Channel_Detector[i][1])//��ǰһ�����̺ʹ˴�����һ��ʱ
		{
			if((Channel_Detector[i][1]==4)&&(Calculated[i]==0))//DL3���̣���ʱ����3�룬ֱ����������
			{
				Real_Count[i]=0;//������ÿ�ζ�Ҫ����
				Count[i][0]=Count[i][1];//��ʷ��������
				Count[i][1]=Count[i][2];
				if(i<7)//ǰ6��̽ͷ�͵��߸�̽ͷȡ����λ�ò�һ��
				{
					Count[i][2]=buf[4*i+5]*256+buf[4*i+4];			//��һ����Ϊ�ڼ��������ڻ���Ӳ����һ������
					Real_Count[i]=(uint)(buf[4*i+5]*256+buf[4*i+4]);
				}
				else if(i==7)//ǰ6��̽ͷ�͵��߸�̽ͷȡ����λ�ò�һ��
				{
					Count[i][2]=buf[31]*256+buf[30];
					Real_Count[i]=(uint)(buf[31]*256+buf[30]);
				}
				Calculated[i]=1;//�������־
				if((Count[i][1]<(37*Refresh_Time))&&(Count[i][2]<(37*Refresh_Time))&&(i<7))//�����л�ǰ7��̽ͷ�ж�����
				{
					Channel_Detector[i][0]=Channel_Detector[i][1];
					Channel_Detector[i][1]=3;
					Var_Signal1=Var_Signal1|Svar1[i];                                       //�����ź�1�Ӽ����ܣ��ߵ�ƽ��ѹ��
          			Var_Signal2=Var_Signal2|Svar1[i];                                       //�����ź�2�ӵ�����10(8),�ߵ�ƽ
          			Var_Signal3=Var_Signal3&Svar0[i];                                       //�����ź�2�ӵ�����10(6),�͵�ƽ
				}
				if((Count[i][1]<(28*Refresh_Time))&&(Count[i][2]<(28*Refresh_Time))&&(i==7))//�����л��ڰ˸�̽ͷ�ж�����
				{
					Channel_Detector[i][0]=Channel_Detector[i][1];
					Channel_Detector[i][1]=3;
					Var_Signal1=Var_Signal1|Svar1[i];                                       //�����ź�1�Ӽ����ܣ��ߵ�ƽ��ѹ��
          			Var_Signal2=Var_Signal2|Svar1[i];                                       //�����ź�2�ӵ�����10(8),�ߵ�ƽ
          			Var_Signal3=Var_Signal3&Svar0[i];                                       //�����ź�2�ӵ�����10(6),�͵�ƽ
				}
				Real_Count_Display[i]=(float)((Real_Count[i]*5)/Refresh_Time);
				Channel_Display[i]=4;
				Display_Flag[i]=1;//ֱ��������ʾ
			}
			if((Channel_Detector[i][1]==3)&&(Calculated[i]==0))//DL2���̣����ζ�ʱ����һ�����
			{
				Count[i][0]=Count[i][1];
				Count[i][1]=Count[i][2];
				if(i<7)//ǰ6��̽ͷ�͵��߸�̽ͷȡ����λ�ò�һ��
				{
					Count[i][2]=buf[4*i+5]*256+buf[4*i+4];
					Real_Count[i]=(uint)(buf[4*i+5]*256+buf[4*i+4]);
				}
				else if(i==7)//ǰ6��̽ͷ�͵��߸�̽ͷȡ����λ�ò�һ��
				{
					Count[i][2]=buf[31]*256+buf[30];
					Real_Count[i]=(uint)(buf[31]*256+buf[30]);
				}
				Calculated[i]=1;//�������־
				if((Count[i][1]>(5719*Refresh_Time))&&(Count[i][2]>(5719*Refresh_Time))&&(i<7))//�����л�
				{
					Channel_Detector[i][0]=Channel_Detector[i][1];
					Channel_Detector[i][1]=4;
					Var_Signal1=Var_Signal1|Svar1[i];                                       //�����ź�1�Ӽ����ܣ��ߵ�ƽ��ѹ��
          			Var_Signal2=Var_Signal2&Svar0[i];                                       //�����ź�2�ӵ�����10(8),�ߵ�ƽ
          			Var_Signal3=Var_Signal3|Svar1[i];                                       //�����ź�2�ӵ�����10(6),�͵�ƽ
				}
				if((Count[i][1]>(7447*Refresh_Time))&&(Count[i][2]>(7447*Refresh_Time))&&(i==7))//�����л�
				{
					Channel_Detector[i][0]=Channel_Detector[i][1];
					Channel_Detector[i][1]=4;
					Var_Signal1=Var_Signal1|Svar1[i];                                       //�����ź�1�Ӽ����ܣ��ߵ�ƽ��ѹ��
          			Var_Signal2=Var_Signal2&Svar0[i];                                       //�����ź�2�ӵ�����10(8),�ߵ�ƽ
          			Var_Signal3=Var_Signal3|Svar1[i];                                       //�����ź�2�ӵ�����10(6),�͵�ƽ
				}
				if(((Count[i][1]<(35*Refresh_Time))&&(Count[i][2]<(35*Refresh_Time)))&&(i!=7))//���߸�̽ͷֻ����������
				{
					Channel_Detector[i][0]=Channel_Detector[i][1];
					Channel_Detector[i][1]=2;
					Var_Signal1=Var_Signal1|Svar1[i];                                       //�����ź�1�Ӽ����ܣ��ߵ�ƽ��ѹ��
          			Var_Signal2=Var_Signal2&Svar0[i];                                       //�����ź�2�ӵ�����10(8),�ߵ�ƽ
          			Var_Signal3=Var_Signal3&Svar0[i];                                       //�����ź�2�ӵ�����10(6),�͵�ƽ
				}
				Real_Count_Display[i]=(float)(Real_Count[i]);
				Real_Count_Display[i]*=5;
				Real_Count_Display[i]/=3;
				Display_Flag[i]=1;//������ʾ
				Channel_Display[i]=3;
				Real_Count[i]=0;
				if((Display_Flag[i]==1)&&((Real_Count_Display[i]/5)>5719)&&(Channel_Detector[i][0]==Channel_Detector[i][1])&&(i<7))//�����л�,�Ӻ������Ƿ�ֹ�ظ�����
				{
					Channel_Detector[i][0]=Channel_Detector[i][1];
					Channel_Detector[i][1]=4;
					Display_Flag[i]=0;
					Var_Signal1=Var_Signal1|Svar1[i];                                       //�����ź�1�Ӽ����ܣ��ߵ�ƽ��ѹ��
          			Var_Signal2=Var_Signal2&Svar0[i];                                       //�����ź�2�ӵ�����10(8),�ߵ�ƽ
          			Var_Signal3=Var_Signal3|Svar1[i];                                       //�����ź�2�ӵ�����10(6),�͵�ƽ
				}
				if((Display_Flag[i]==1)&&((Real_Count_Display[i]/5)>7447)&&(Channel_Detector[i][0]==Channel_Detector[i][1])&&(i==7))//�����л�,�Ӻ������Ƿ�ֹ�ظ�����
				{
					Channel_Detector[i][0]=Channel_Detector[i][1];
					Channel_Detector[i][1]=4;
					Display_Flag[i]=0;
					Var_Signal1=Var_Signal1|Svar1[i];                                       //�����ź�1�Ӽ����ܣ��ߵ�ƽ��ѹ��
          			Var_Signal2=Var_Signal2&Svar0[i];                                       //�����ź�2�ӵ�����10(8),�ߵ�ƽ
          			Var_Signal3=Var_Signal3|Svar1[i];                                       //�����ź�2�ӵ�����10(6),�͵�ƽ
				}
				if((Display_Flag[i]==1)&&((Real_Count_Display[i]/5)<35)&&(Channel_Detector[i][0]==Channel_Detector[i][1])&&(i!=7))
				{
					Channel_Detector[i][0]=Channel_Detector[i][1];
					Channel_Detector[i][1]=2;
					Display_Flag[i]=0;
					Var_Signal1=Var_Signal1|Svar1[i];                                       //�����ź�1�Ӽ����ܣ��ߵ�ƽ��ѹ��
          			Var_Signal2=Var_Signal2&Svar0[i];                                       //�����ź�2�ӵ�����10(8),�ߵ�ƽ
          			Var_Signal3=Var_Signal3&Svar0[i];                                       //�����ź�2�ӵ�����10(6),�͵�ƽ
				}
			}
			if((Channel_Detector[i][1]==2)&&(Calculated[i]==0))//DL1���̣�5�ζ�ʱ����һ�����
			{
				Count[i][0]=Count[i][1];
				Count[i][1]=Count[i][2];
				Count[i][2]=buf[4*i+5]*256+buf[4*i+4];
				Real_Count[i]=(uint)(buf[4*i+5]*256+buf[4*i+4]);
				Calculated[i]=1;//�������־
				if((Count[i][1]>(5411*Refresh_Time))&&(Count[i][2]>(5411*Refresh_Time)))//�����л�
				{
					Channel_Detector[i][0]=Channel_Detector[i][1];
					Channel_Detector[i][1]=3;
					Var_Signal1=Var_Signal1|Svar1[i];                                       //�����ź�1�Ӽ����ܣ��ߵ�ƽ��ѹ��
          			Var_Signal2=Var_Signal2|Svar1[i];                                       //�����ź�2�ӵ�����10(8),�ߵ�ƽ
          			Var_Signal3=Var_Signal3&Svar0[i];                                       //�����ź�2�ӵ�����10(6),�͵�ƽ
				}
				if((Count[i][1]<(158*Refresh_Time))&&(Count[i][2]<(158*Refresh_Time)))
				{
					Channel_Detector[i][0]=Channel_Detector[i][1];
					Channel_Detector[i][1]=1;
					Var_Signal1=Var_Signal1&Svar0[i];                                       //�����ź�1�Ӽ����ܣ��͵�ƽ��ѹ��
          			Var_Signal2=Var_Signal2&Svar0[i];                                       //�����ź�2�ӵ�����10(8),�͵�ƽ
          			Var_Signal3=Var_Signal3&Svar0[i];                                       //�����ź�2�ӵ�����10(6),�͵�ƽ
				}
					Real_Count_Display[i]=(float)(Real_Count[i]);
					Real_Count_Display[i]*=5;//������CP5S
					Real_Count_Display[i]/=Refresh_Time;
					Display_Flag[i]=1;//������ʾ
					Channel_Display[i]=2;
					Real_Count[i]=0;
					if((Display_Flag[i]==1)&&((Real_Count_Display[i]/5)>5411)&&(Channel_Detector[i][0]==Channel_Detector[i][1]))//�����л�
					{
						Channel_Detector[i][0]=Channel_Detector[i][1];
						Channel_Detector[i][1]=3;
						Display_Flag[i]=0;
						Var_Signal1=Var_Signal1|Svar1[i];                                       //�����ź�1�Ӽ����ܣ��ߵ�ƽ��ѹ��
          				Var_Signal2=Var_Signal2|Svar1[i];                                       //�����ź�2�ӵ�����10(8),�ߵ�ƽ
          				Var_Signal3=Var_Signal3&Svar0[i];                                       //�����ź�2�ӵ�����10(6),�͵�ƽ
					}
					if((Display_Flag[i]==1)&&((Real_Count_Display[i]/5)<158)&&(Channel_Detector[i][0]==Channel_Detector[i][1]))
					{
						Channel_Detector[i][0]=Channel_Detector[i][1];
						Channel_Detector[i][1]=1;
						Display_Flag[i]=0;
						Var_Signal1=Var_Signal1&Svar0[i];                                       //�����ź�1�Ӽ����ܣ��͵�ƽ��ѹ��
          				Var_Signal2=Var_Signal2&Svar0[i];                                       //�����ź�2�ӵ�����10(8),�͵�ƽ
          				Var_Signal3=Var_Signal3&Svar0[i];                                       //�����ź�2�ӵ�����10(6),�͵�ƽ
					}
			}
			if((Channel_Detector[i][1]==1)&&(Calculated[i]==0))//GM2���̣�10�ζ�ʱ����һ����ӣ�5��ƽ��ƽ��
			{
				Count[i][0]=Count[i][1];
				Count[i][1]=Count[i][2];
				Count[i][2]=buf[4*i+3]*256+buf[4*i+2];
				Real_Count[i]=(uint)(buf[4*i+3]*256+buf[4*i+2]);
				Calculated[i]=1;//�������־
				if((Count[i][0]>(6019*Refresh_Time))&&(Count[i][1]>(6019*Refresh_Time))&&(Count[i][2]>(6019*Refresh_Time)))//�����л�
				{
					Channel_Detector[i][0]=Channel_Detector[i][1];
					Channel_Detector[i][1]=2;
					Var_Signal1=Var_Signal1|Svar1[i];                                       //�����ź�1�Ӽ����ܣ��ߵ�ƽ��ѹ��
          			Var_Signal2=Var_Signal2&Svar0[i];                                       //�����ź�2�ӵ�����10(8),�͵�ƽ
          			Var_Signal3=Var_Signal3&Svar0[i];                                       //�����ź�2�ӵ�����10(6),�͵�ƽ
				}
				if((Count[i][0]<(1.32*Refresh_Time))&&(Count[i][1]<(1.32*Refresh_Time))&&(Count[i][2]<(1.32*Refresh_Time)))
				{
					Channel_Detector[i][0]=Channel_Detector[i][1];
					Channel_Detector[i][1]=0;
					Var_Signal1=Var_Signal1&Svar0[i];                                       //�����ź�1�Ӽ����ܣ��͵�ƽ��ѹ��
          			Var_Signal2=Var_Signal2&Svar0[i];                                       //�����ź�2�ӵ�����10(8),�͵�ƽ
          			Var_Signal3=Var_Signal3&Svar0[i];                                       //�����ź�2�ӵ�����10(6),�͵�ƽ
				}
				Real_Count[i]*=20;//����ΪCPM(ע���Ӧ����3��� Refresh Time)
				Average_Counts[i][Average_Times[i]]=Real_Count[i];
				Average_Times[i]+=1;
				if(Average_Times[i]<5)
				{
					switch(Average_Times[i])
					{
						case 1: Real_Count_Display[i]=Average_Counts[i][0]; break;
						case 2: Real_Count_Display[i]=Average_Counts[i][0]*0.4+Average_Counts[i][1]*0.6; break;
						case 3: Real_Count_Display[i]=Average_Counts[i][0]*a3+Average_Counts[i][1]*a3+Average_Counts[i][2]*a4; break;
						case 4: Real_Count_Display[i]=Average_Counts[i][0]*a2+Average_Counts[i][1]*a2+Average_Counts[i][2]*a3+Average_Counts[i][3]*a4; break;
					}
					Display_Flag[i]=1;
					Channel_Display[i]=0;
				}
				if(Average_Times[i]==5)
				{
					Real_Count_Display[i]=Average_Counts[i][0]*a0+Average_Counts[i][1]*a1+Average_Counts[i][2]*a2+Average_Counts[i][3]*a3+Average_Counts[i][4]*a4;
					for(j=0;j<4;j++)
					{
						Average_Counts[i][j]=Average_Counts[i][j+1];
					}
					Average_Times[i]-=1;//Ϊ���ô���
					Display_Flag[i]=1;
					Channel_Display[i]=1;
				}
				//if(Count_Times[i]==2)
				//{
					//Average_Counts[i][Average_Times[i]]=Real_Count[i];//�������ʸ�ֵ��ƽ��ֵ
					//Average_Times[i]+=1;//ƽ��ƽ��������1
					//Count_Times[i]=0;
					//Real_Count[i]=0;
				//}
				//if((Average_Times[i]>=2)&&(Average_Times[i]<=5))//����2������ֵ��ʼ��ƽ��ֵ����ʾ
				//{
					//for(j=0;j<Average_Times[i];j++)
					//{
						//Real_Count_Display[i]+=(float)(Average_Counts[i][j]);
					//}
					//Real_Count_Display[i]/=(float)(Average_Times[i]);
					//Real_Count_Display[i]*=(60/(Refresh_Time*2));//��ΪҪ�����CPM
					//Display_Flag[i]=1;//������ʾ
					//Channel_Display[i]=0;
					//if(Average_Times[i]==5)
					//{
						//for(j=0;j<(Average_Times[i]-1);j++)
						//{
							//Average_Counts[i][j]=Average_Counts[i][j+1];
						//}
						//Average_Times[i]-=1;//Ϊ���ô���
					//}
				//}
				//if((Display_Flag[i]==1)&&((Real_Count_Display[i]/60)>6019)&&(Channel_Detector[i][0]==Channel_Detector[i][1]))//�����л�
				//{
					//Channel_Detector[i][0]=Channel_Detector[i][1];
					//Channel_Detector[i][1]=2;
					//Display_Flag[i]=0;
					//Var_Signal1=Var_Signal1&Svar0[i];                                       //�����ź�1�Ӽ����ܣ��͵�ƽ
          			//Var_Signal2=Var_Signal2&Svar0[i];                                       //�����ź�2�ӵ�����10(8),�͵�ƽ
          			//Var_Signal3=Var_Signal3&Svar0[i];                                       //�����ź�2�ӵ�����10(6),�͵�ƽ
				//}
				//if((Display_Flag[i]==1)&&((Real_Count_Display[i]/60)<1.32)&&(Channel_Detector[i][0]==Channel_Detector[i][1]))
				//{
					//Channel_Detector[i][0]=Channel_Detector[i][1];
					//Channel_Detector[i][1]=0;
					//Display_Flag[i]=0;
					//Var_Signal1=Var_Signal1|Svar1[i];                                       //�����ź�1�Ӽ����ܣ�Ϊ�ߵ�ƽ
          			//Var_Signal2=Var_Signal2&Svar0[i];                                       //�����ź�2�ӵ�����10(8),�͵�ƽ
          			//Var_Signal3=Var_Signal3&Svar0[i];                                       //�����ź�2�ӵ�����10(6),�͵�ƽ
				//}
			}
			if((Channel_Detector[i][1]==0)&&(Calculated[i]==0))//GM1���̣�2�ζ�ʱ����3����ӣ�5��ƽ��ƽ��
			{
				Count[i][0]=Count[i][1];
				Count[i][1]=Count[i][2];
				Count[i][2]=buf[4*i+3]*256+buf[4*i+2];
				Real_Count[i]=(uint)(buf[4*i+3]*256+buf[4*i+2]);
				Calculated[i]=1;//�������־
				if((Count[i][0]>(1.32*Refresh_Time))&&(Count[i][1]>(1.32*Refresh_Time))&&(Count[i][2]>(1.32*Refresh_Time)))//�����л�
				{
					Channel_Detector[i][0]=Channel_Detector[i][1];
					Channel_Detector[i][1]=1;
					Var_Signal1=Var_Signal1&Svar0[i];                                       //�����ź�1�Ӽ����ܣ��͵�ƽ��ѹ��
          			Var_Signal2=Var_Signal2&Svar0[i];                                       //�����ź�2�ӵ�����10(8),�͵�ƽ
          			Var_Signal3=Var_Signal3&Svar0[i];                                       //�����ź�2�ӵ�����10(6),�͵�ƽ
				}
					Real_Count[i]*=20;//����ΪCPM(ע���Ӧ����3��� Refresh Time)
					Average_Counts[i][Average_Times[i]]=Real_Count[i];
					Average_Times[i]+=1;
					if(Average_Times[i]<5)
					{
						switch(Average_Times[i])
						{
							case 1: Real_Count_Display[i]=Average_Counts[i][0]; break;
							case 2: Real_Count_Display[i]=Average_Counts[i][0]*0.4+Average_Counts[i][1]*0.6; break;
							case 3: Real_Count_Display[i]=Average_Counts[i][0]*a3+Average_Counts[i][1]*a3+Average_Counts[i][2]*a4; break;
							case 4: Real_Count_Display[i]=Average_Counts[i][0]*a2+Average_Counts[i][1]*a2+Average_Counts[i][2]*a3+Average_Counts[i][3]*a4; break;
						}
						Display_Flag[i]=1;
						Channel_Display[i]=0;
					}
					else if(Average_Times[i]==5)
					{
						Real_Count_Display[i]=Average_Counts[i][0]*a0+Average_Counts[i][1]*a1+Average_Counts[i][2]*a2+Average_Counts[i][3]*a3+Average_Counts[i][4]*a4;
						for(j=0;j<4;j++)
						{
							Average_Counts[i][j]=Average_Counts[i][j+1];
						}
						Average_Times[i]-=1;//Ϊ���ô���
						Display_Flag[i]=1;
						Channel_Display[i]=0;
					}
					//if(Count_Times[i]==2)
					//{
					//	Average_Counts[i][Average_Times[i]]=Real_Count[i];//�������ʸ�ֵ��ƽ��ֵ
					//	Average_Times[i]+=1;//ƽ��ƽ��������1
					//	Count_Times[i]=0;
					//	Real_Count[i]=0;
					//}
					//if((Average_Times[i]>=2)&&(Average_Times[i]<=5))//����2������ֵ��ʼ��ƽ��ֵ����ʾ
					//{
						//for(j=0;j<Average_Times[i];j++)
						//{
							//Real_Count_Display[i]+=(float)(Average_Counts[i][j]);
						//}
						//Real_Count_Display[i]/=(float)(Average_Times[i]);
						//Real_Count_Display[i]*=(60/(Refresh_Time*2));//��ΪҪ�����CPM
						//Display_Flag[i]=1;//������ʾ
						//Channel_Display[i]=0;
						//if(Average_Times[i]==5)
						//{
							//for(j=0;j<(Average_Times[i]-1);j++)
							//{
								//Average_Counts[i][j]=Average_Counts[i][j+1];
							//}
							//Average_Times[i]-=1;//Ϊ���ô���
						//}
					//}
					//if((Display_Flag[i]==1)&&((Real_Count_Display[i]/60)>1.32)&&(Channel_Detector[i][0]==Channel_Detector[i][1]))//�����л�
					//{
						//Channel_Detector[i][0]=Channel_Detector[i][1];
						//Channel_Detector[i][1]=1;
						//Display_Flag[i]=0;
						//Var_Signal1=Var_Signal1|Svar1[i];                                       //�����ź�1�Ӽ����ܣ�Ϊ�ߵ�ƽ
          				//Var_Signal2=Var_Signal2&Svar0[i];                                       //�����ź�2�ӵ�����10(8),�͵�ƽ
          				//Var_Signal3=Var_Signal3&Svar0[i];                                       //�����ź�2�ӵ�����10(6),�͵�ƽ
					//}
			}
		}
		else if(Channel_Detector[i][0]!=Channel_Detector[i][1])//��ǰ�����μ��������̲�һ��
		{
			if((Channel_Detector[i][1]==4)&&(Calculated[i]==0))//DL3���̣���ʱ����һ�룬ֱ����������
			{
				for(k=0;k<=2;k++)//��ʼ�����������жϱ�������ֹ���⽵��
				{
					Count[i][k]=1000*Refresh_Time;//һ�����ʹ�䲻����
				}
				Real_Count[i]=0;//��ת����������Ҫ����
				if(i<7)//ǰ6��̽ͷ�͵��߸�̽ͷȡ����λ�ò�һ��
				{
					Count[i][2]=buf[4*i+5]*256+buf[4*i+4];
					Real_Count[i]=(uint)(buf[4*i+5]*256+buf[4*i+4]);
				}
				else if(i==7)//ǰ6��̽ͷ�͵��߸�̽ͷȡ����λ�ò�һ��
				{
					Count[i][2]=buf[31]*256+buf[30];
					Real_Count[i]=(uint)(buf[31]*256+buf[30]);
				}
				Calculated[i]=1;//�������־
				Real_Count_Display[i]=(float)((Real_Count[i]*5)/Refresh_Time);
				Display_Flag[i]=1;//ֱ��������ʾ
				Channel_Display[i]=4;
				Channel_Detector[i][0]=Channel_Detector[i][1];//����������ʷ״̬
			}
			if((Channel_Detector[i][1]==3)&&(Calculated[i]==0))//DL2���̣����ζ�ʱ����һ�����
			{
				for(k=0;k<=2;k++)
				{
					Count[i][k]=2000*Refresh_Time;//һ�����ʹ�䲻����
				}
				Real_Count[i]=0;//��ת���̺���Ҫ����
				Display_Flag[i]=0;//��ʾ��������־����
				if(i<7)//ǰ6��̽ͷ�͵��߸�̽ͷȡ����λ�ò�һ��
				{
					Count[i][2]=buf[4*i+5]*256+buf[4*i+4];
					Real_Count[i]=(uint)(buf[4*i+5]*256+buf[4*i+4]);
				}
				else if(i==7)//ǰ6��̽ͷ�͵��߸�̽ͷȡ����λ�ò�һ��
				{
					Count[i][2]=buf[31]*256+buf[30];
					Real_Count[i]=(uint)(buf[31]*256+buf[30]);
				}
				Calculated[i]=1;//�������־
				Channel_Detector[i][0]=Channel_Detector[i][1];//����������ʷ״̬
			}
			if((Channel_Detector[i][1]==2)&&(Calculated[i]==0))//DL1���̣�5�ζ�ʱ����һ�����
			{
				for(k=0;k<=2;k++)
				{
					Count[i][k]=2000*Refresh_Time;//һ�����ʹ�䲻����
				}
				Real_Count[i]=0;//��ת���̺���Ҫ����
				Display_Flag[i]=0;//��ʾ��������־����
				if(i<7)//ǰ6��̽ͷ�͵��߸�̽ͷȡ����λ�ò�һ��
				{
					Count[i][2]=buf[4*i+5]*256+buf[4*i+4];
					Real_Count[i]=(uint)(buf[4*i+5]*256+buf[4*i+4]);
				}
				else if(i==7)//ǰ6��̽ͷ�͵��߸�̽ͷȡ����λ�ò�һ��
				{
					Count[i][2]=buf[31]*256+buf[30];
					Real_Count[i]=(uint)(buf[31]*256+buf[30]);
					Channel_Detector[i][1]=3;
					Channel_Detector[i][0]=3;
				}

				Calculated[i]=1;//�������־
				Channel_Detector[i][0]=Channel_Detector[i][1];//����������ʷ״̬
			}
			if((Channel_Detector[i][1]==1)&&(Calculated[i]==0))//GM2���̣�10�ζ�ʱ����һ����ӣ�5��ƽ��ƽ��
			{
				for(k=0;k<=2;k++)
				{
					Count[i][k]=1000*Refresh_Time;//һ�����ʹ�䲻����
				}
				Real_Count[i]=0;//��ת���̺���Ҫ����
				Display_Flag[i]=0;//��ʾ��������־����
				Average_Times[i]=0;//ƽ��ƽ����������
				for(j=0;j<10;j++)
				{
					Average_Counts[i][j]=0;//ƽ��ƽ������
				}
				Count[i][2]=buf[4*i+3]*256+buf[4*i+2];
				Real_Count[i]=(uint)(buf[4*i+3]*256+buf[4*i+2]);
				
				Real_Count[i]*=20;//����ΪCPM(ע���Ӧ����3��� Refresh Time)
				Average_Counts[i][Average_Times[i]]=Real_Count[i];
				Average_Times[i]+=1;
				Real_Count_Display[i]=Average_Counts[i][Average_Times[i]];
				Display_Flag[i]=1;
				Channel_Display[i]=1;
				Calculated[i]=1;//�������־
				Channel_Detector[i][0]=Channel_Detector[i][1];//����������ʷ״̬
			}
			if((Channel_Detector[i][1]==0)&&(Calculated[i]==0))//GM2���̣�10�ζ�ʱ����һ����ӣ�5��ƽ��ƽ��
			{
				for(k=0;k<=2;k++)
				{
					Count[i][k]=1*Refresh_Time;//һ�����ʹ�䲻����
				}
				Real_Count[i]=0;//��ת���̺���Ҫ����
				Display_Flag[i]=0;//��ʾ��������־����
				Average_Times[i]=0;//ƽ��ƽ����������
				for(j=0;j<10;j++)
				{
					Average_Counts[i][j]=0;//ƽ��ƽ������
				}
				Count[i][2]=buf[4*i+3]*256+buf[4*i+2];
				Real_Count[i]=(uint)(buf[4*i+3]*256+buf[4*i+2]);
				Real_Count[i]*=20;//����ΪCPM(ע���Ӧ����3��� Refresh Time)
				Average_Counts[i][Average_Times[i]]=Real_Count[i];
				Average_Times[i]+=1;
				Real_Count_Display[i]=Average_Counts[i][Average_Times[i]];
				Display_Flag[i]=1;
				Channel_Display[i]=0;
				Calculated[i]=1;//�������־
				Channel_Detector[i][0]=Channel_Detector[i][1];//����������ʷ״̬
			}
		}
  }
  /*�����ݼ�����ֵ�������õ�̽ͷ�źŸı��־ִ��*/
  Signal1=Var_Signal1;//̽ͷ�ź�ת��
  Signal2=Var_Signal3;//2016-8-26����Var_Signal2,Var_Signal3����Ϊ������Ӳ���ϱ�Ū����
  Signal3=Var_Signal2;
  /*AC1+������buf(ʮ��λ�ߵͼ�������)Ҫת����uint ���͵�Tdata����,����̽ͷ��ĿҪ���
  ����ѡ̽ͷ��־*/
  if(measure_flag)                                                                //(1)measure_flag��Init_RAM�г�ʼ����Ϊ1
  {
    for(j=0;j<8;j++)                                                       //(2)�������Ϊbuf[3]-buf[32];̽ͷ������־
    {
      Tdata=(uint)(Count[j][2]/3);                                                        //̽ͷ������uint ,��ɫΪȫ�ֱ�������ɫΪ�ֲ�����
      jtemp=0;                                                                    //float
      yudata=0;                                                                   //double
      mtemp=0;                                                                    //double
      yu=0;                                                                       //double
      Tbcd[5]=0;                                                                  //uchar ���鹲12λ����õ����ݵ�bcd������
      Tbcd[4]=0;
      Tbcd[3]=0;
      Tbcd[2]=0;
      Tbcd[1]=0;
      Tbcd[0]=0;
      m=j*10;                                                                     //����
      /*********�жϼ�����***********/
      /*********ǰ�߸�̽ͷ̽�������ã�ȡ��8253����ֵ��¼��Tdata*/
      /**��2024������ͨ����ż���궨����������CPM�ͼ����ʵ�ת��**/
	  if(j<7)                                                                     //(3)ǰ�߸�̽ͷ���е�����Ҳ�м��������ڰ˸�ֻ�е����ҹʷֿ�����
      {
      		Display_Flag[j]=0;
			if(Channel_Display[j]==0)
			{
				Para[0]=(float)(DataThouth[m+1]*1000+DataCent[m+1]*100+DataTenth[m+1]*10+DataGe[m+1]);//���ڲ���
				Para[1]=(float)((float)(DataThouth[m+2]*1000+DataCent[m+2]*100+DataTenth[m+2]*10+DataGe[m+2])/1000000);
				DoseRata[j]=(float)(Para[1]*((uint)Real_Count_Display[j]-Para[0]));
			}
			else if(Channel_Display[j]==1)
			{
				Para[2]=(float)(DataThouth[m+3]*1000+DataCent[m+3]*100+DataTenth[m+3]*10+DataGe[m+3]);//���ڲ���
        		Para[3]=(float)((float)(DataThouth[m+4]*1000+DataCent[m+4]*100+DataTenth[m+4]*10+DataGe[m+4])/1000000);
				DoseRata[j]=(float)(Para[3]*((uint)Real_Count_Display[j]-Para[2]));
			}
			else if(Channel_Display[j]==2)
			{
				Para[4]=(float)(DataThouth[m+5]*1000+DataCent[m+5]*100+DataTenth[m+5]*10+DataGe[m+5]);//���ڲ���
        		Para[5]=(float)((float)(DataThouth[m+6]*1000+DataCent[m+6]*100+DataTenth[m+6]*10+DataGe[m+6])/1000);
				DoseRata[j]=(float)(Para[5]*((uint)Real_Count_Display[j]-Para[4]));
			}
			else if(Channel_Display[j]==3)
			{
				Para[6]=(float)(DataThouth[m+7]*1000+DataCent[m+7]*100+DataTenth[m+7]*10+DataGe[m+7]);//���ڲ���
        		Para[7]=(float)((float)(DataThouth[m+8]*1000+DataCent[m+8]*100+DataTenth[m+8]*10+DataGe[m+8])/10);
				DoseRata[j]=(float)(Para[7]*((uint)Real_Count_Display[j]-Para[6]));
			}
			else if(Channel_Display[j]==4)
			{
				Para[8]=(float)(DataThouth[m+9]*1000+DataCent[m+9]*100+DataTenth[m+9]*10+DataGe[m+9]);//���ڲ���
        		Para[9]=(float)((float)((DataThouth[m+10]*1000+DataCent[m+10]*100+DataTenth[m+10]*10+DataGe[m+10])*10));
				DoseRata[j]=(float)(Para[9]*((uint)Real_Count_Display[j]-Para[8]));
			}
		}
		else if(j==7)
		{
			Display_Flag[j]=0;
			//Txtext(10,13+(j*58),"Dh");
			if(Channel_Display[j]==3)
			{
				Para[0]=(float)(DataThouth[m+1]*1000+DataCent[m+1]*100+DataTenth[m+1]*10+DataGe[m+1]);//���ڲ���
				Para[1]=(float)((float)(DataThouth[m+2]*1000+DataCent[m+2]*100+DataTenth[m+2]*10+DataGe[m+2])/10);
				DoseRata[j]=(float)(Para[1]*((uint)Real_Count_Display[j]-Para[0]));
			}
			else if(Channel_Display[j]==4)
			{
				Para[2]=(float)(DataThouth[m+3]*1000+DataCent[m+3]*100+DataTenth[m+3]*10+DataGe[m+3]);//���ڲ���
				Para[3]=(float)((float)(DataThouth[m+4]*1000+DataCent[m+4]*100+DataTenth[m+4]*10+DataGe[m+4])*10);
				DoseRata[j]=(float)(Para[3]*((uint)Real_Count_Display[j]-Para[2]));
			}
		}
      /**********��λ�����Ϊjtemp************/
      if(DoseRata[j]<0)                                                         //�޸�2012.6.15ͨ����ֵ����
      {
        jtemp=0.0;
        Flag_dw=1;
      }
      else if(DoseRata[j]>=0)                                                   //��λΪuGy/h
      {
        if(DoseRata[j]<1000)                                                    //Mean_Value<1000��ʾ��λΪuGy/h
        {
          jtemp=(float)DoseRata[j];
          Flag_dw=1;
        }
        else if((DoseRata[j]>=1000)&&(DoseRata[j]<1000000))                   //1000<Mean_Value<1000000��ʾmGy/h
        {
          jtemp=(float)(DoseRata[j]/1000);
          Flag_dw=2;
        }
        else if(DoseRata[j]>=1000000)                                           //Mean_Value>=1000000��ʾGy/h
        {
          jtemp=(float)(DoseRata[j]/1000000);
          Flag_dw=3;
        }
      }
      /*****DCS��������׼��****/
      if(DoseRata[j]<0)                                                         //���������С������DCS����0
      {
        DCS_Send[j*4+4]=0x30;
        DCS_Send[j*4+5]=0x30;
        DCS_Send[j*4+6]=0x30;
        DCS_Send[j*4+7]=0x30;
      }
      else if(DoseRata[j]>=0)
      {
        if(DoseRata[j]<1)
        {
          temp=(float)DoseRata[j]*1000;
          DCS_Send[j*4+4]=(uchar)((int)temp/100)+0x30;
          DCS_Send[j*4+5]=(uchar)((int)temp%100/10)+0x30;
          DCS_Send[j*4+6]=(uchar)((int)temp%100%10)+0x30;
          DCS_Send[j*4+7]=0x30;
        }
        else if((DoseRata[j]>=1)&&(DoseRata[j]<10))
        {
          temp=(float)DoseRata[j]*100;
          DCS_Send[j*4+4]=(uchar)((int)temp/100)+0x30;
          DCS_Send[j*4+5]=(uchar)((int)temp%100/10)+0x30;
          DCS_Send[j*4+6]=(uchar)((int)temp%100%10)+0x30;
          DCS_Send[j*4+7]=0x31;
        }
        else if((DoseRata[j]>=10)&&(DoseRata[j]<100))
        {
          temp=(float)DoseRata[j]*10;
          DCS_Send[j*4+4]=(uchar)((int)temp/100)+0x30;
          DCS_Send[j*4+5]=(uchar)((int)temp%100/10)+0x30;
          DCS_Send[j*4+6]=(uchar)((int)temp%100%10)+0x30;
          DCS_Send[j*4+7]=0x32;
        }
        else if((DoseRata[j]>=100)&&(DoseRata[j]<1000))
        {
          temp=(float)DoseRata[j];
          DCS_Send[j*4+4]=(uchar)((int)temp/100)+0x30;
          DCS_Send[j*4+5]=(uchar)((int)temp%100/10)+0x30;
          DCS_Send[j*4+6]=(uchar)((int)temp%100%10)+0x30;
          DCS_Send[j*4+7]=0x33;
        }
        else if((DoseRata[j]>=1000)&&(DoseRata[j]<10000))
        {
          temp=(float)DoseRata[j]/10.0;
          DCS_Send[j*4+4]=(uchar)((int)temp/100)+0x30;
          DCS_Send[j*4+5]=(uchar)((int)temp%100/10)+0x30;
          DCS_Send[j*4+6]=(uchar)((int)temp%100%10)+0x30;
          DCS_Send[j*4+7]=0x34;
        }
        else if((DoseRata[j]>=10000)&&(DoseRata[j]<100000))
        {
          temp=(float)DoseRata[j]/100.0;
          DCS_Send[j*4+4]=(uchar)((int)temp/100)+0x30;
          DCS_Send[j*4+5]=(uchar)((int)temp%100/10)+0x30;
          DCS_Send[j*4+6]=(uchar)((int)temp%100%10)+0x30;
          DCS_Send[j*4+7]=0x35;
        }
        else if((DoseRata[j]>=100000)&&(DoseRata[j]<1000000))
        {
          temp=(float)DoseRata[j]/1000.0;
          DCS_Send[j*4+4]=(uchar)((int)temp/100)+0x30;
          DCS_Send[j*4+5]=(uchar)((int)temp%100/10)+0x30;
          DCS_Send[j*4+6]=(uchar)((int)temp%100%10)+0x30;
          DCS_Send[j*4+7]=0x36;
        }
        else if((DoseRata[j]>=1000000)&&(DoseRata[j]<10000000))
        {
          temp=(float)DoseRata[j]/10000.0;
          DCS_Send[j*4+4]=(uchar)((int)temp/100)+0x30;
          DCS_Send[j*4+5]=(uchar)((int)temp%100/10)+0x30;
          DCS_Send[j*4+6]=(uchar)((int)temp%100%10)+0x30;
          DCS_Send[j*4+7]=0x37;
        }
        else if((DoseRata[j]>=10000000)&&(DoseRata[j]<100000000))
        {
          temp=(float)DoseRata[j]/100000.0;
          DCS_Send[j*4+4]=(uchar)((int)temp/100)+0x30;
          DCS_Send[j*4+5]=(uchar)((int)temp%100/10)+0x30;
          DCS_Send[j*4+6]=(uchar)((int)temp%100%10)+0x30;
          DCS_Send[j*4+7]=0x38;
        }
        else if((DoseRata[j]>=100000000)&&(DoseRata[j]<1000000000))
        {
          temp=(float)DoseRata[j]/1000000.0;
          DCS_Send[j*4+4]=(uchar)((int)temp/100)+0x30;
          DCS_Send[j*4+5]=(uchar)((int)temp%100/10)+0x30;
          DCS_Send[j*4+6]=(uchar)((int)temp%100%10)+0x30;
          DCS_Send[j*4+7]=0x39;
        }
      }
      /***********������ת����λ��ת���ɿ�����ʾ��BCD��*************/
      if((Tbcd[5]<=9)&&(Tbcd[4]<=9)&&(Tbcd[3]<=9)&&(Tbcd[2]<=9)&&(Tbcd[1]<=9)&&(Tbcd[0]<=9))//4.21�ƺ����࣬�ɲ������޸ĵ�
      {
        Tbcd[5]=(uchar)(((int)jtemp)/100);                                         //��λ
        Tbcd[4]=(uchar)(((int)jtemp)%100/10);                                      //ʮλ
        Tbcd[3]=(uchar)(((int)jtemp)%100%10);                                      //��λ
        count_temp=(jtemp-(int)jtemp)*1000;                                         //����С��1�Ĳ��֣���С�����ֱ�󣬱�����λС��
        Tbcd[2]=(uchar)(count_temp/100);                                           //СС��ʮ��λ
        Tbcd[1]=(uchar)(count_temp%100/10);                                        //С���ٷ�λ
        Tbcd[0]=(uchar)(count_temp%100%10);                                        //С��ǧ��λ
        if(Flag_dw==1)                                                              //����Tbcd[]����yudecide(float)�����жϼ������Ƿ񳬹���ֵ
          yudecide=(float)(Tbcd[5]*100+Tbcd[4]*10+Tbcd[3]+Tbcd[2]*0.1+Tbcd[1]*0.01+Tbcd[0]*0.001);
        if(Flag_dw==2)
          yudecide=(float)((Tbcd[5]*100+Tbcd[4]*10+Tbcd[3]+Tbcd[2]*0.1+Tbcd[1]*0.01+Tbcd[0]*0.001)*1000);
        if(Flag_dw==3)
          yudecide=(float)((Tbcd[5]*100+Tbcd[4]*10+Tbcd[3]+Tbcd[2]*0.1+Tbcd[1]*0.01+Tbcd[0]*0.001)*1000000);
      }
      /***********�ж���ֵ*************/
      mtemp=(YuThouth[j]+YuCent[j]*0.1+YuTenth[j]*0.01);                          //��ʱû����λ���޸�2012.6.15
      yu=(double)(pow(10,YuGe[j]));
      yudata=(double)(mtemp*yu);
      if(yudecide>=yudata)                                                        //�����������ĳһ����Χ������������
      {
        if((Judge_Speak==0)||(Speak_Alarm()==1))                                  //������������Speak_Alarm����1����������
        {
          Flag_Warn=1;                                                            //������־��һ
        }
        Flag_need_Flash[j]=1;                                                     //LED��˸��־��1
        State_Flash[j]=1;
        Var_Led=Var_Led&Svar0[j];                                                 //LED����ָʾ�ƣ�char��Svar0[8]={0xFE,0xFD,0xFB,0xF7,0xEF,0xDF,0xBF,0x7F}
      }
      else if(yudecide<yudata)
      {
        Flag_need_Flash[j]=0;
        State_Flash[j]=0;
        Var_Led=Var_Led|Svar1[j];
      }
      /*AI1*��������ʾӦ����̽������־�ı�֮ǰ******��������ʾ**********/
	  //if(Channel_Detector[j]==0)
	  //{
      //	Txtext(10,13+(j*58),"Gh");                                                  //AH1��ʾ̽ͷ
	  //}
	  //else
	  //{
		//Txtext(10,13+(j*58),"Dh");
	  //}
	  /*AI1*******��������ʾ**********/
	  if(Flag_need_Flash[j]==0)
	  {
	  	if((Channel_Display[j]==0)||(Channel_Display[j]==1))
		{
			Txtext(10,13+(j*58),"GM");												  //AH1��ʾ̽ͷ
		}
		else
		{
			Txtext(10,13+(j*58),"DL");
		}
      	Tnumber(60,13+(j*58),j+1);                                                  //��ʾ̽ͷ��
      	Txtext(90,13+(j*58),":");                                                   //��ʾð��
	  	Tnumber(470,13+(j*58),Channel_Detector[j][1]);
	  	Tnumber(440,13+(j*58),Channel_Detector[j][0]);
	  	Tnumber(390,13+(j*58),Average_Times[j]);
      	if(Tbcd[5]!=0)                                                              //����λ���㣬�������Ĵ�С����ʾ
      	{
        	Tnumber(130,13+(j*58),Tbcd[5]);
        	Tnumber(160,13+(j*58),Tbcd[4]);
        	Tnumber(190,13+(j*58),Tbcd[3]);
        	Txtext(220,13+(j*58),".");
        	Tnumber(240,13+(j*58),Tbcd[2]);
        	Tnumber(270,13+(j*58),Tbcd[1]);
        	Tnumber(300,13+(j*58),Tbcd[0]);
        	if(Tdata<10)                                                              //������ʾTdata����ļ���ֵ��ʾ
        	Tnumber(576,13+(j*58),Tdata);                                             //������ɺ�ȥ��2012.6.24
        	else if((Tdata>=10)&&(Tdata<100))
        	Tnumber(556,13+(j*58),Tdata);
        	else if((Tdata>=100)&&(Tdata<1000))
        	Tnumber(536,13+(j*58),Tdata);
        	else if((Tdata>=1000)&&(Tdata<10000))
        	Tnumber(516,13+(j*58),Tdata);
        	else if(Tdata>=10000)
        	Tnumber(496,13+(j*58),Tdata);
      	}
      	else
      	{
        	if(Tbcd[4]!=0)                                                            //��λΪ��ʮλ����
        	{
          	Tnumber(160,13+(j*58),Tbcd[4]);
          	Tnumber(190,13+(j*58),Tbcd[3]);
          	Txtext(220,13+(j*58),".");
          	Tnumber(240,13+(j*58),Tbcd[2]);
          	Tnumber(270,13+(j*58),Tbcd[1]);
          	Tnumber(300,13+(j*58),Tbcd[0]);
          	if(Tdata<10)
            	Tnumber(576,13+(j*58),Tdata);                                         //������ɺ�ȥ��2012.6.24
          	else if((Tdata>=10)&&(Tdata<100))
            	Tnumber(556,13+(j*58),Tdata);
          	else if((Tdata>=100)&&(Tdata<1000))
            	Tnumber(536,13+(j*58),Tdata);
          	else if((Tdata>=1000)&&(Tdata<10000))
            	Tnumber(516,13+(j*58),Tdata);
          	else if(Tdata>=10000)
            	Tnumber(496,13+(j*58),Tdata);                                         //������ɺ�ȥ��2012.6.24
        	}
        	else                                                                      //��λʮλλ��Ϊ�㣬��λ����
        	{
          	Tnumber(190,13+(j*58),Tbcd[3]);
          	Txtext(220,13+(j*58),".");                                              //.ռ20������
          	Tnumber(240,13+(j*58),Tbcd[2]);                                         //һ������ռ30������
          	Tnumber(270,13+(j*58),Tbcd[1]);
          	Tnumber(300,13+(j*58),Tbcd[0]);
          	if(Tdata<10)
            	Tnumber(576,13+(j*58),Tdata);                                         //������ɺ�ȥ��2012.6.24
          	else if((Tdata>=10)&&(Tdata<100))                                       //���Կ���һ������ռ20����
            	Tnumber(556,13+(j*58),Tdata);
          	else if((Tdata>=100)&&(Tdata<1000))
            	Tnumber(536,13+(j*58),Tdata);
          	else if((Tdata>=1000)&&(Tdata<10000))
            	Tnumber(516,13+(j*58),Tdata);
          	else if(Tdata>=10000)
            	Tnumber(496,13+(j*58),Tdata);
        	}
	  	}
      	/**********������λ��ʾ***********/
      	if(Flag_dw==1)                                                              //������ɺ����2012.6.24
      	{
        	Txtext(340,13+(j*58),"u");
      	}
      	else if(Flag_dw==2)
      	{
        	Txtext(340,13+(j*58),"m");
      	}
      	else if(Flag_dw==3)
      	{
        	Txtext(340,13+(j*58),"G");
      	}
      	Alarm();                                                                    //����
	  	}
	  if(Flag_need_Flash[j]==1)
	  {
		if((Channel_Display[j]==0)||(Channel_Display[j]==1))
		{
			Trxtext(7,13+(j*58),"  ");
			Trxtext(10,13+(j*58),"GM                        ");												//AH1��ʾ̽ͷ
		}
		else
		{
			Trxtext(7,13+(j*58),"  ");
			Trxtext(10,13+(j*58),"DL                        ");
		}
      	Trnumber(60,13+(j*58),j+1);                                                  //��ʾ̽ͷ��
      	Trxtext(90,13+(j*58),":");                                                   //��ʾð��
	  	Trnumber(470,13+(j*58),Channel_Detector[j][1]);
	  	Trnumber(440,13+(j*58),Channel_Detector[j][0]);
	  	Trnumber(390,13+(j*58),Average_Times[j]);
      	if(Tbcd[5]!=0)                                                              //����λ���㣬�������Ĵ�С����ʾ
      	{
        	Trnumber(130,13+(j*58),Tbcd[5]);
        	Trnumber(160,13+(j*58),Tbcd[4]);
        	Trnumber(190,13+(j*58),Tbcd[3]);
        	Trxtext(220,13+(j*58),".");
        	Trnumber(240,13+(j*58),Tbcd[2]);
        	Trnumber(270,13+(j*58),Tbcd[1]);
        	Trnumber(300,13+(j*58),Tbcd[0]);
        	if(Tdata<10)                                                              //������ʾTdata����ļ���ֵ��ʾ
        	Trnumber(576,13+(j*58),Tdata);                                             //������ɺ�ȥ��2012.6.24
        	else if((Tdata>=10)&&(Tdata<100))
        	Trnumber(556,13+(j*58),Tdata);
        	else if((Tdata>=100)&&(Tdata<1000))
        	Trnumber(536,13+(j*58),Tdata);
        	else if((Tdata>=1000)&&(Tdata<10000))
        	Trnumber(516,13+(j*58),Tdata);
        	else if(Tdata>=10000)
        	Trnumber(496,13+(j*58),Tdata);
      	}
      	else
      	{
        	if(Tbcd[4]!=0)                                                            //��λΪ��ʮλ����
        	{
          	Trnumber(160,13+(j*58),Tbcd[4]);
          	Trnumber(190,13+(j*58),Tbcd[3]);
          	Trxtext(220,13+(j*58),".");
          	Trnumber(240,13+(j*58),Tbcd[2]);
          	Trnumber(270,13+(j*58),Tbcd[1]);
          	Trnumber(300,13+(j*58),Tbcd[0]);
          	if(Tdata<10)
            	Trnumber(576,13+(j*58),Tdata);                                         //������ɺ�ȥ��2012.6.24
          	else if((Tdata>=10)&&(Tdata<100))
            	Trnumber(556,13+(j*58),Tdata);
          	else if((Tdata>=100)&&(Tdata<1000))
            	Trnumber(536,13+(j*58),Tdata);
          	else if((Tdata>=1000)&&(Tdata<10000))
            	Trnumber(516,13+(j*58),Tdata);
          	else if(Tdata>=10000)
            	Trnumber(496,13+(j*58),Tdata);                                         //������ɺ�ȥ��2012.6.24
        	}
        	else                                                                      //��λʮλλ��Ϊ�㣬��λ����
        	{
          	Trnumber(190,13+(j*58),Tbcd[3]);
          	Trxtext(220,13+(j*58),".");                                              //.ռ20������
          	Trnumber(240,13+(j*58),Tbcd[2]);                                         //һ������ռ30������
          	Trnumber(270,13+(j*58),Tbcd[1]);
          	Trnumber(300,13+(j*58),Tbcd[0]);
          	if(Tdata<10)
            	Trnumber(576,13+(j*58),Tdata);                                         //������ɺ�ȥ��2012.6.24
          	else if((Tdata>=10)&&(Tdata<100))                                       //���Կ���һ������ռ20����
            	Trnumber(556,13+(j*58),Tdata);
          	else if((Tdata>=100)&&(Tdata<1000))
            	Trnumber(536,13+(j*58),Tdata);
          	else if((Tdata>=1000)&&(Tdata<10000))
            	Trnumber(516,13+(j*58),Tdata);
          	else if(Tdata>=10000)
            	Trnumber(496,13+(j*58),Tdata);
        	}
	  	}
      	/**********������λ��ʾ***********/
      	if(Flag_dw==1)                                                              //������ɺ����2012.6.24
      	{
        	Trxtext(340,13+(j*58),"u");
      	}
      	else if(Flag_dw==2)
      	{
        	Trxtext(340,13+(j*58),"m");
      	}
      	else if(Flag_dw==3)
      	{
        	Trxtext(340,13+(j*58),"G");
      	}
      	Alarm();                                                                    //����
	  	}
      send_buf[8*j]=Tbcd[5];                                                      //��NIM-B������ʾ����
      send_buf[8*j+1]=Tbcd[4];
      send_buf[8*j+2]=Tbcd[3];
      send_buf[8*j+3]=Tbcd[2];
      send_buf[8*j+4]=Tbcd[1];
      send_buf[8*j+5]=Tbcd[0];
      send_buf[8*j+6]=Flag_dw;                                                    //���͵�λ��־
      send_buf[8*j+7]=Flag_need_Flash[j];                                         //LED��˸��־
    }
    for(j=0;j<8;j++)                                                              //��DCS���ݵı����ź�
    {
      if(Flag_need_Flash[j])
      {
        DCS_Send[64]=DCS_Send[64]|Svar1[j];
      }
      else
      {
        DCS_Send[64]=DCS_Send[64]&Svar0[j];
      }
    }                                                                             //8��̽ͷ����ʾִ����
    Flag_Warn_Count=1;
    jishucount++;                                                                 //�����������ӣ���ƽ��ֵʱ�ã�
    if(jishucount==255)                                                           //����������Ϊ��char�ͱ��������֧��255��������Ҫ����
      jishucount=8;
    if(jishucount>4)
      Flag_Meant=1;                                                               //ǰ��ε�ȡֵ��Ҫ������βſ�ʼҪ����Flag_Meant=1���������0
    else
      Flag_Meant=0;
    //shortdelay(5000);                                                             //��ʱ
    //for(i=0;i<33;i++)//��ʾ����0
    //buf[i]=0;
  }                                                                               //������־
  else
  {
    jishuguan_data=0;                                                             //needthinkwhetherneed?4.21,int���������ݣ�
    dianlishi_data=0;                                                             //����������
    jishuguan_rata=0;                                                             //�����ܼ�����
    jishuguan_jtemp=0;                                                            //�����ܼ���ֵ(��λת����)
    dianlishi_jtemp=0;                                                            //�����Ҽ���ֵ����λת����
    Tbcd[11]=0;
    Tbcd[10]=0;
    Tbcd[9]=0;
    Tbcd[8]=0;
    Tbcd[7]=0;
    Tbcd[6]=0;
    Tbcd[5]=0;
    Tbcd[4]=0;
    Tbcd[3]=0;
    Tbcd[2]=0;
    Tbcd[1]=0;
    Tbcd[0]=0;
    j=10*biaoding_input-7;                                                        //biaodin_inputΪҪ�궨��̽ͷ���
    jishuguan_data=buf[4*biaoding_input-1]*256+buf[4*biaoding_input-2];       //�൱��Tdata
    jishuguan_rata=jishuguan_data*12;                                             //�����ʣ�ֱ�ӳ���12?
    dianlishi_data=buf[4*biaoding_input+1]*256+buf[4*biaoding_input];     //������Tdata
    Para[0]=(float)(DataThouth[j]*1000+DataCent[j]*100+DataTenth[j]*10+DataGe[j]);//���ڲ���
    Para[1]=(float)((float)(DataThouth[j+1]*1000+DataCent[j+1]*100+DataTenth[j+1]*10+DataGe[j+1])/1000000);
    jishuguan_DoseRata=(float)(Para[1]*(jishuguan_rata-Para[0]));
    if(jishuguan_DoseRata<0)
      jishuguan_DoseRata=0.0;
    jishuguan_jtemp=(float)jishuguan_DoseRata;
    if((Tbcd[11]<=9)&&(Tbcd[10]<=9)&&(Tbcd[9]<=9)&&(Tbcd[8]<=9)&&(Tbcd[7]<=9)&&(Tbcd[6]<=9))//4.21�ƺ����࣬�ɲ������޸ĵ�
    {
      Tbcd[11]=(uchar)(((int)jishuguan_jtemp)/100);                               //ǧλ
      Tbcd[10]=(uchar)(((int)jishuguan_jtemp)%100/10);                            //��λ
      Tbcd[9]=(uchar)(((int)jishuguan_jtemp)%100%10);                             //��λ
      jishuguan_count_temp=(jishuguan_jtemp-(int)jishuguan_jtemp)*1000;
      Tbcd[8]=(uchar)(jishuguan_count_temp/100);
      Tbcd[7]=(uchar)(jishuguan_count_temp%100/10);
      Tbcd[6]=(uchar)(jishuguan_count_temp%100%10);
    }
    Para[2]=(float)(DataThouth[j+2]*1000+DataCent[j+2]*100+DataTenth[j+2]*10+DataGe[j+2]);//���ڲ���
    Para[3]=(float)((float)(DataThouth[j+3]*1000+DataCent[j+3]*100+DataTenth[j+3]*10+DataGe[j+3])/1000);
    dianlishi_DoseRata=(float)(Para[3]*(dianlishi_data-Para[2]));
    if(dianlishi_DoseRata<0)
      dianlishi_DoseRata=0.0;
    dianlishi_jtemp=(float)(dianlishi_DoseRata/1000);
    if((Tbcd[5]<=9)&&(Tbcd[4]<=9)&&(Tbcd[3]<=9)&&(Tbcd[2]<=9)&&(Tbcd[1]<=9)&&(Tbcd[0]<=9))//4.21�ƺ����࣬�ɲ������޸ĵ�
    {
      Tbcd[5]=(uchar)(((int)dianlishi_jtemp)/100);                                //ǧλ
      Tbcd[4]=(uchar)(((int)dianlishi_jtemp)%100/10);                             //��λ
      Tbcd[3]=(uchar)(((int)dianlishi_jtemp)%100%10);                             //��λ
      dianlishi_count_temp=(dianlishi_jtemp-(int)dianlishi_jtemp)*1000;
      Tbcd[2]=(uchar)(dianlishi_count_temp/100);
      Tbcd[1]=(uchar)(dianlishi_count_temp%100/10);
      Tbcd[0]=(uchar)(dianlishi_count_temp%100%10);
    }
    Txtext(200,58,"�궨̽ͷ");
    Tnumber(400,58,biaoding_input);
    Txtext(10,116,"�Ǹ��");;
    Txtext(160,116,":");
    Txtext(220,174,"uGy/h");
    Txtext(520,174,"CPM");
    Txtext(10,290,"������");;
    Txtext(160,290,":");
    Txtext(220,348,"mGy/h");
    Txtext(520,348,"CP5S");
    if(Tbcd[11]!=0)
    {
      Tnumber(10,174,Tbcd[11]);
      Tnumber(40,174,Tbcd[10]);
      Tnumber(70,174,Tbcd[9]);
      Txtext(100,174,".");
      Tnumber(130,174,Tbcd[8]);
      Tnumber(160,174,Tbcd[7]);
      Tnumber(190,174,Tbcd[6]);
      if(jishuguan_data<10)
        Tnumber(490,174,jishuguan_data);                                          //������ɺ�ȥ��2012.6.24
      else if((jishuguan_data>=10)&&(jishuguan_data<100))
        Tnumber(460,174,jishuguan_data);
      else if((jishuguan_data>=100)&&(jishuguan_data<1000))
        Tnumber(430,174,jishuguan_data);
      else if((jishuguan_data>=1000)&&(jishuguan_data<10000))
        Tnumber(400,174,jishuguan_data);
      else if(jishuguan_data>=10000)
        Tnumber(370,174,jishuguan_data);
    }
    else
    {
      if(Tbcd[10]!=0)
      {
        Tnumber(40,174,Tbcd[10]);
        Tnumber(70,174,Tbcd[9]);
        Txtext(100,174,".");
        Tnumber(130,174,Tbcd[8]);
        Tnumber(160,174,Tbcd[7]);
        Tnumber(190,174,Tbcd[6]);
        if(jishuguan_data<10)
        Tnumber(490,174,jishuguan_data);                                          //������ɺ�ȥ��2012.6.24
        else if((jishuguan_data>=10)&&(jishuguan_data<100))
        Tnumber(460,174,jishuguan_data);
        else if((jishuguan_data>=100)&&(jishuguan_data<1000))
        Tnumber(430,174,jishuguan_data);
        else if((jishuguan_data>=1000)&&(jishuguan_data<10000))
        Tnumber(400,174,jishuguan_data);
        else if(jishuguan_data>=10000)
        Tnumber(370,174,jishuguan_data);
      }
      else
      {
        Tnumber(70,174,Tbcd[9]);
        Txtext(100,174,".");
        Tnumber(130,174,Tbcd[8]);
        Tnumber(160,174,Tbcd[7]);
        Tnumber(190,174,Tbcd[6]);
        if(jishuguan_data<10)
          Tnumber(490,174,jishuguan_data);                                        //������ɺ�ȥ��2012.6.24
        else if((jishuguan_data>=10)&&(jishuguan_data<100))
          Tnumber(460,174,jishuguan_data);
        else if((jishuguan_data>=100)&&(jishuguan_data<1000))
          Tnumber(430,174,jishuguan_data);
        else if((jishuguan_data>=1000)&&(jishuguan_data<10000))
          Tnumber(400,174,jishuguan_data);
        else if(jishuguan_data>=10000)
          Tnumber(370,174,jishuguan_data);
      }
    }
    if(Tbcd[5]!=0)
    {
      Tnumber(10,348,Tbcd[5]);
      Tnumber(40,348,Tbcd[4]);
      Tnumber(70,348,Tbcd[3]);
      Txtext(100,348,".");
      Tnumber(130,348,Tbcd[2]);
      Tnumber(160,348,Tbcd[1]);
      Tnumber(190,348,Tbcd[0]);
      if(dianlishi_data<10)
        Tnumber(490,348,dianlishi_data);                                          //������ɺ�ȥ��2012.6.24
      else if((dianlishi_data>=10)&&(dianlishi_data<100))
        Tnumber(460,348,dianlishi_data);
      else if((dianlishi_data>=100)&&(dianlishi_data<1000))
        Tnumber(430,348,dianlishi_data);
      else if((dianlishi_data>=1000)&&(dianlishi_data<10000))
        Tnumber(400,348,dianlishi_data);
      else if(dianlishi_data>=10000)
        Tnumber(370,348,dianlishi_data);
    }
    else
    {
      if(Tbcd[4]!=0)
      {
        Tnumber(40,348,Tbcd[4]);
        Tnumber(70,348,Tbcd[3]);
        Txtext(100,348,".");
        Tnumber(130,348,Tbcd[2]);
        Tnumber(160,348,Tbcd[1]);
        Tnumber(190,348,Tbcd[0]);
        if(dianlishi_data<10)
          Tnumber(490,348,dianlishi_data);                                        //������ɺ�ȥ��2012.6.24
        else if((dianlishi_data>=10)&&(dianlishi_data<100))
          Tnumber(460,348,dianlishi_data);
        else if((dianlishi_data>=100)&&(dianlishi_data<1000))
          Tnumber(430,348,dianlishi_data);
        else if((dianlishi_data>=1000)&&(dianlishi_data<10000))
          Tnumber(400,348,dianlishi_data);
        else if(dianlishi_data>=10000)
          Tnumber(370,348,dianlishi_data);
      }
      else
      {
        Tnumber(70,348,Tbcd[3]);
        Txtext(100,348,".");
        Tnumber(130,348,Tbcd[2]);
        Tnumber(160,348,Tbcd[1]);
        Tnumber(190,348,Tbcd[0]);
        if(dianlishi_data<10)
          Tnumber(490,348,dianlishi_data);                                        //������ɺ�ȥ��2012.6.24
        else if((dianlishi_data>=10)&&(dianlishi_data<100))
          Tnumber(460,348,dianlishi_data);
        else if((dianlishi_data>=100)&&(dianlishi_data<1000))
          Tnumber(430,348,dianlishi_data);
        else if((dianlishi_data>=1000)&&(dianlishi_data<10000))
          Tnumber(400,348,dianlishi_data);
        else if(dianlishi_data>=10000)
          Tnumber(370,348,dianlishi_data);
      }
    }
  }
}

/************************************
*����ʱ
************************************/
void shortdelay(uint i)
{
  uint k;
  uint n;
  for(k=0;k<i;k++)
    for(n=200;n>0;n--);
}

/********************************
*ָʾ�ƺͱ���
********************************/
void Alarm()
{
  uchar n;
  for(n=0;n<10;n++)
  {
    Led573=Var_Led;
    if(Flag_Warn==1)                                                                  //��������
    {
      Speak=1;
    }
  }
}

/*******************************
*����ָʾ��״̬
*******************************/
void Updata_Flash(uchar j)
{
  if(Flag_need_Flash[j])
  {
    if(State_Flash[j]==1)
    {
      State_Flash[j]=0;
    }
    else
    {
      State_Flash[j]=1;
    }
  }
  else
  State_Flash[j]=0;
}

/***********************************
*ָʾ����˸
************************************/
void Led_Flash(void )
{
  uchar i;
  for(i=0;i<8;i++)
  {
    Updata_Flash(i);                                                                  //״̬ȡ��
    Led_Disp(i,Flag_need_Flash[i],State_Flash[i]);                                    //��״̬��ʾ
  }
}

/************************************
*ָʾ��״̬��ʾ
*************************************/
void Led_Disp(uchar Num,uchar Flag,uchar State)
{
  if(Flag)
  {
    if(State)
    {
      Var_Led=Var_Led&Svar0[Num];                                                       //ָʾ����
      Led573=Var_Led;
      if(Flag_Warn==1)                                                                  //��Ҫ����������
      {
        Speak=0;                                                                          //��������
        shortdelay(100);
      }
      return;
    }
  }
  Var_Led=Var_Led|Svar1[Num];                                                       //ָʾ����
  Led573=Var_Led;
  if(Flag_Warn==1)                                                                  //��Ҫ����������
  {
    Speak=1;                                                                          //����������
    shortdelay(100);
  }
}
