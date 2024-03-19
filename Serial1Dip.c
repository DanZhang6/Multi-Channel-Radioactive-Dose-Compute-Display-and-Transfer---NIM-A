/*******************************************************************
*描述：
*     备用显示处理
*功能：
*     备用机显示NIM-A和NIM-B传输数据，如果需要报警，在屏幕上显示指示灯

*******************************************************************/
#include "STC15F2K60S2.h"
#include "absacc.h"
#include "config.h"
#include "intrins.h"
#include "math.h"
#include "stdio.h"

/**变量初始化*/
// uchar backup_data[20];            //存放备用显示的数据
// uchar data_A[8];
// uchar data_B[12];
// bit Flag_data_change;           //备用此案时数据更新标志
// uchar alarm_data = 0x00;
// bit Flag_serial1_led;                        //备用机的闪烁开启定时标志

// void Write_Data (uchar Pchar);
// uchar Backup_Covn(uchar temp_data,uchar code_data);
// void BackUp_displayA();
// void SegLED_Clear();
/*************************************
 *         备用显示屏显示NIM_A
 *************************************/
/*void BackUp_displayA()//4.20晚修改
{
    uchar idata z,x;
        uchar idata uGy,mGy,Gy;

        for(z=0;z<2;z++)
          {
                        if(z == 0)      //显示第三个探头
                        {
                                 x = 2;
                        }
                        if(z == 1)      //显示第八个探头
                        {
                                 x = 7;
                        }*/
/******报警的设置******/
/*			if(x == 2)
                        {
                                if(send_buf[8*x+7] == 1)
                                {
                                        alarm_data = 0x42;
                                        backup_data[6*z] = alarm_data ;
                                }
                                else if(send_buf[8*x+7] == 0)
                                {
                                        alarm_data = 0x02;
                                        backup_data[6*z] = alarm_data ;
                                }
                        }
                        if(x == 7)
                        {
                                if(send_buf[8*x+7] == 1)
                                {
                                        alarm_data |= 0x20;
                                        backup_data[6*z] = alarm_data ;
                                }
                                else if(send_buf[8*x+7] == 0)
                                {
                                        alarm_data |= 0x00;
                                        backup_data[6*z] = alarm_data ;
                                }
                        }*/
/*****单位的选择*****/
/*			if(send_buf[8*x+6] == 1)
                        {
                                uGy = 1;
                                backup_data[6*z+1] = 0x30;
                        }
                        if(send_buf[8*x+6] == 2)
                        {
                                mGy = 1;
                backup_data[6*z+1] = 0x28;
                        }
                        if(send_buf[8*x+6] == 3)
                        {
                                Gy = 1;
                                backup_data[6*z+1] = 0x24;
                        }*/
/*****数据的显示******/
/*			if(send_buf[8*x] != 0)   //如果千位不为零
                        {
                                data_A[4*z+3] = Backup_Covn(send_buf[8*x],data_A[4*z+3]);
                            data_A[4*z+2] = Backup_Covn(send_buf[8*x+1],data_A[4*z+2]);
                                data_A[4*z+1] = Backup_Covn(send_buf[8*x+2],data_A[4*z+1]);
                                data_A[4*z] = Backup_Covn(send_buf[8*x+3],data_A[4*z]);
                        }
                        else if(send_buf[8*x] == 0)   //如果千位为零
                        {
                                data_A[4*z+3] = 0x03;
                                if(send_buf[8*x+1] != 0)   //如果百位不为零
                                {
                                    data_A[4*z+2] = Backup_Covn(send_buf[8*x+1],data_A[4*z+2]);
                                }
                                else if(send_buf[8*x+1] == 0)   //如果百位为零
                                {
                                    data_A[4*z+2] = 0x03;
                                }
                                data_A[4*z+1] = Backup_Covn(send_buf[8*x+2],data_A[4*z+1]);  //不管十位是否为零，都要显示
                            data_A[4*z] = Backup_Covn(send_buf[8*x+3],data_A[4*z]);
                        }
                        data_A[4*z+3] = data_A[4*z+3] << 4;
                        backup_data[4*z+3] = data_A[4*z+3] | data_A[4*z+2];
                        data_A[4*z+1] = data_A[4*z+1] << 4;
                        backup_data[4*z+2] = data_A[4*z+1] | data_A[4*z];
                }
}*/
/**********************************
 *         备用显示关闭处理
 ***********************************/
/*void SegLED_Clear()
{
                Write_Data(0x00);
                Write_Data(0x00);
                Write_Data(0x33);
                Write_Data(0x33);
                Write_Data(0x33);
}*/

/***********************************
 *          备用显示数据译码
 ************************************/
/*uchar Backup_Covn(uchar temp_data,uchar code_data)
{
    uchar cov_data[10] = {0x00,0x08,0x04,0x0c,0x02,0x0a,0x06,0x0e,0x01,0x09};
        uchar i;
        for(i=0;i<10;i++)
        {
                if(temp_data == i)
                {
                        code_data = cov_data[i];
                        return code_data;
                }
         }

}*/
/*************************************
 *         备用显示屏显示NIM_B
 *************************************/
/*void BackUp_displayB()//4.20晚修改
{
    uchar idata z,x;
        uchar idata mBq,kBq,Bq;
        for(z=0;z<3;z++)
          {
                        if(z == 0)      //显示第一个探头
                        {
                         x = 0;
                        }
                        if(z == 1)      //显示第二个探头
                        {
                         x = 1;
                        }
            if(z == 2)      //显示第五个探头
            {
                         x = 4;
            }*/
/******报警的设置******/
/*            if(x == 0)
                        {
                                if(receive_buf[8*x+7] == 1)
                                {
                                        alarm_data |= 0x10;
                                        backup_data[6*z+12] = alarm_data ;
                                }
                                else if(receive_buf[8*x+7] == 0)
                                {
                                        alarm_data |= 0x00;
                                        backup_data[6*z+12] = alarm_data ;
                                }
                        }
                        if(x == 1)
                        {
                                if(receive_buf[8*x+7] == 1)
                                {
                                        alarm_data |= 0x08;
                                        backup_data[6*z+12] = alarm_data ;
                                }
                                else if(receive_buf[8*x+7] == 0)
                                {
                                        alarm_data |= 0x00;
                                        backup_data[6*z+12] = alarm_data ;
                                }
                        }
                        if(x == 4)
                        {
                                if(receive_buf[8*x+7] == 1)
                                {
                                        alarm_data |= 0x04;
                                        backup_data[6*z+12] = alarm_data ;
                                }
                                else if(receive_buf[8*x+7] == 0)
                                {
                                        alarm_data |= 0x00;
                                        backup_data[6*z+12] = alarm_data ;
                                }
                        }*/
/*****单位的选择*****/
/*			if(receive_buf[8*x+6] == 1)
                        {
                                Bq = 1;
                                backup_data[6*z+13] = 0x20;
                                backup_data[6*z+12] |= 0x80;

                        }
                        if(receive_buf[8*x+6] == 2)
                        {
                                kBq = 1;
                backup_data[6*z+13] = 0x21;
                        }
                        if(receive_buf[8*x+6] == 3)
                        {
                                mBq = 1;
                                backup_data[6*z+13] = 0x22;
                        }*/
/*****数据的显示******/
/*			if(receive_buf[8*x] != 0)   //如果千位不为零
                        {
                                data_B[4*z+3] = Backup_Covn(receive_buf[8*x],data_B[4*z+3]);
                            data_B[4*z+2] = Backup_Covn(receive_buf[8*x+1],data_B[4*z+2]);
                                data_B[4*z+1] = Backup_Covn(receive_buf[8*x+2],data_B[4*z+1]);
                                data_B[4*z] = Backup_Covn(receive_buf[8*x+3],data_B[4*z]);
                        }
                        else if(receive_buf[8*x] == 0)   //如果千位为零
                        {
                                data_B[4*z+3] = 0x03;
                                if(receive_buf[8*x+1] != 0)   //如果百位不为零
                                {
                                    data_B[4*z+2] = Backup_Covn(receive_buf[8*x+1],data_B[4*z+2]);
                                }
                                else if(receive_buf[8*x+1] == 0)   //如果百位为零
                                {
                                    data_B[4*z+2] = 0x03;
                                }
                                data_B[4*z+1] = Backup_Covn(receive_buf[8*x+2],data_B[4*z+1]);  //不管十位是否为零，都要显示
                            data_B[4*z] = Backup_Covn(receive_buf[8*x+3],data_B[4*z]);
                        }
                        data_B[4*z+3] = data_B[4*z+3] << 4;
                        backup_data[4*z+11] = data_B[4*z+3] | data_B[4*z+2];
                        data_B[4*z+1] = data_B[4*z+1] << 4;
                        backup_data[4*z+10] = data_B[4*z+1] | data_B[4*z];
                }
}*/

/**************************************
 *        备用显示屏控制
 **************************************/
/*void bakeup_conv_data()
{
        alarm_data = 0x00;
        BackUp_displayA();
        BackUp_displayB();
}*/

/*********************************
 *        串并转换数据写入
 ***********************************/
/*void Write_Data (uchar Pchar)	//UART1发送数据
{
                SBUF  = Pchar;
                while (!TI);		// 等待至TI1为1
                TI	= 0; 			// 将TI1清零
}*/

/**********************************
 *        显示传递到备用屏的数据
 ***********************************/
/*void display_covn_data()
{
        uchar i,t;
        for(i=1;i<6;i++)
        {
//		BackUp_Display();
                if((Flag_Commond == 0) && (Flag_data_change == 1))
                {
                    t = Backup_Covn(i,t);
                    SegLED_Clear();
                        Write_Data(backup_data[4*i-4]);
                        Write_Data(backup_data[4*i-3]);
                        Write_Data(backup_data[4*i-2]);
                        Write_Data(backup_data[4*i-1]);
                        Write_Data(t);
                }
                if((Flag_Commond == 1) || (Flag_data_change == 0))
                {
                        SegLED_Clear();
                        return;
                }
                if(Flag_Warn_Flash)                  //主机LED指示灯闪烁间隔定时
                {
                        Flag_Warn_Flash = 0;
            Led_Flash();
                }
                if((Flag_Warn_Led==1)&&(PCOLSIG==0))  //屏幕上红灯闪烁间隔定时
                {
                        Flag_Warn_Led = 0;
                        RedLed_Flash();
                 }
                shortdelay(2000);
        }
}*/
