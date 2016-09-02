/*******************************************************************
* 描述：
       参数设置
* 功能：
*     1.  根据设定好的探头个数，设置每个探头的参数,
*         每个探头包含五组参数，每组参数包含参数a和b;
*     2.  参数位数通过参数右移键设置，千分位，百分位，十分位，个位；
*     3.  参数的每一位可在0到9之间循环选择

********************************************************************/
#include "STC15F2K60S2.h"
#include "intrins.h"
#include "config.h" 
#include "absacc.h"

uchar Incdataright;							//右移按键按下次数标志
uchar Incdata;								//参数大小标志??????
uchar idata bz;                               //显示设置参数个数标志
uchar idata xh;                               //设置探头个数标志
uchar idata wb;                               //设置参数行数标志
uchar idata Flag_Yu=0; 
uchar idata Flag_Para=0; 
uchar data Flag_Left=0;                       //参数a列的标志
uchar data Flag_Right=0;                      //参数b列的标志
uchar data Flag_Change=0;
uchar YuThouth[8];								//阈值千分位
uchar YuCent[8];								//阈值百分位
uchar YuTenth[8];								//阈值十分位
uchar YuGe[8];									//阈值个位

extern void dispaly_para();
extern void dispaly_paramove();
extern void Side_RChange();
extern void Side_LChange();
extern void Line_Change();
/******************************
*         参数设置
******************************/
void setparameter()
{
	Flag_RefrInput=0;
	Flag_RefrTime=0;
	Flag_RefrRight=1;
	Flag_RefrLeft=1;
    if(wb==1)						//参数行数标志
    { 
        if(Incdataright==1)
		 {
			YuThouth[xh-1]=Incdata;			//YuThough为所有八个探头阈值的千分位,xh为设置探头个数标志，Incdata为
			dt_in[bz*4-3]=YuThouth[xh-1];
		 }
		else if(Incdataright==2)
		 {
			YuCent[xh-1]=Incdata;
		    dt_in[bz*4-2]=YuCent[xh-1];
		 }
		else if(Incdataright==3)
		 {
			YuTenth[xh-1]=Incdata;
		    dt_in[bz*4-1]=YuTenth[xh-1];
		 }
		else if(Incdataright==4)
		 {
			YuGe[xh-1]=Incdata;
		    dt_in[bz*4]=YuGe[xh-1];
		 }   
    }
    else if(wb!=1)
    {
		/*设置参数的每一位*/
		if(Incdataright==1)
		{
			DataThouth[bz-xh]=Incdata;
			dt_in[bz*4-3]=DataThouth[bz-xh];
		 }
		else if(Incdataright==2)
		 {
			DataCent[bz-xh]=Incdata;
		    dt_in[bz*4-2]=DataCent[bz-xh];
		 }
		else if(Incdataright==3)
		 {
			DataTenth[bz-xh]=Incdata;
		    dt_in[bz*4-1]=DataTenth[bz-xh];
		 }
		else if(Incdataright==4)
		 {
			DataGe[bz-xh]=Incdata;
		    dt_in[bz*4]=DataGe[bz-xh];
		 }
     }
 	  dispaly_para();
}
  
/********************************
*       参数右移设置
*********************************/
void parameterright()
{
	Flag_RefrInput=0;
	Flag_RefrTime=0;
	if(Flag_RefrRight==0)
	{
		Flag_RefrPara=0;
	}
	Incdata=0;
	Incdataright++;                               //计算右移键被按下次数
    Side_RChange();									//坐标左右变动函数
	dispaly_paramove();								//参数右移光标显示
}


/*******************************
*       坐标位置右变动
*********************************/
void Side_RChange()
{
	if(xh%2!=0)                                  //探头显示为单数（探头1，3，5，7）
	 {
		if(Incdataright == 5)                    
		{    
			Incdataright = 1;
			bz++;                                //参数共五组，每组包含2对，每个设置完加1 
			if((bz-1)%11==0)                     //换页处理（每个探头共10个换算系数）
			{
				xh++;                            //十个参数设置完，探头标志加1
			    if(xh==8)
		        {
			        Flag_RefrPara=0;  
		        }
		        if(xh > Incinput)
		        {
				     Flag_RefrPara=0;             //刷新数据
			         xh=1;                       //探头显示标志不能超过探头个数
					 bz=1;
		        }
			}  
			
			if((xh%2)==0)                         //单号探头转双号探头处理
			{
				if((bz%2)==0)                     //换行处理    
				{ 
					wb++;
				}
			}
            if((xh%2)!=0)                         //单号探头换行处理
			{
				if((((bz-1)%2)!=0)||(bz==1))                        
				{
					wb++; 
				}
			}
			if(wb==7)                           //参数设置共6行，其中第一行为阈值设置，其余五行为参数
			{
				wb=1;
				Flag_Change=1;
			} 				
		}	   
	}
	if(xh%2==0)                                 //探头显示为单数（探头2，4，6，8）
	{
		if(Incdataright >= 5)
		{    
			Incdataright = 1;
			bz++;                                //前面探头参数设置完，探头加1 
			if(bz==83)
			{
				xh=1;
				bz=1;
				wb=1;
                Flag_RefrPara=0;
                Flag_Change=1;
			}
			else
			{
				if((bz-1)%11 == 0)                 //换页处理（每个探头共10个换算系数）
				{
					xh++;                           //十个参数设置完，探头标志加1
					if(xh==8)
			        {
				        Flag_RefrPara=0;  
			        }
			        if(xh > Incinput)
			        {
					     Flag_RefrPara=0;           //刷新数据
				         xh=1;                     //探头显示标志不能超过探头个数
						 bz=1;
			        }
				}  
				if((xh%2)!=0)                       //双号探头转单号探头处理  
				{
					if((bz%2)!=0)                   
					{
						wb++; 
					}
				}
				if((xh%2)==0)                       //双号探头换行处理                          
				{
					if(((bz-1)%2)==0)                   
					{ 
						wb++;
					}
				}
				if(xh<8)
				{
					if(wb==7)
					{
						wb=1;
						Flag_Change=1;            //刷新探头参数设置
					} 
				}
			}
		}
	} 
    Line_Change();
}
/********************************
*       参数左移设置
*********************************/
void parameterleft()                           
{
	Flag_RefrInput=0;
	Flag_RefrTime=0;
	if(Flag_RefrLeft==0)
	  {
        Flag_RefrPara=0;
	  }
  	Incdata=0;
	Incdataright--;                             //计算右移键被按下次数
    Side_LChange();								//坐标位置左变动
	dispaly_paramove();							//参数右移光标显示
}

/*******************************
*       坐标位置左变动
*********************************/
void Side_LChange()
{
	if(xh%2!=0)                                 //单号探头（探头1，3， 5， 7）坐标位置设置
	 {
		if(Incdataright ==0)
		{    
			Incdataright = 4;
			bz--;                                //参数共五组，每组包含2对，每个设置完减1 
            if(bz%11==0)                         //每个探头共10个换算系数，和一组阈值设置，设置完11组，跳到下一探头设置
			{   
			    xh--;                            //十个参数设置和阈值设置完，探头标志减1
		        if(xh<1)                         //如果左移到开始位置，在左移就到第Incinput个探头（设定最大探头个数）
		        {
				     Flag_RefrPara=0;            //刷新当前屏幕
			         xh=Incinput;                //探头显示标志到最大设定探头个数
		        }
			}                              			
			if((xh%2)!=0)                        //单号探头的换行处理
			{
				if(((bz%2)!=0)||(bz==0))          //bz为单数时换行
				{ 
					wb--;                        //行数从下往上变小
					if(wb==0)                    //移到最顶行
					{
					   wb=6;
					   bz=xh*11;
                       Flag_Change=1;             //刷新探头数据
					}
				}
			}
			if((xh%2)==0)                        //单号探头转换成双号探头时，换行处理
			{
				if((bz%2)==0)                    //阈值设置完就换行
				{ 
					wb--;
					if(wb==0)                   
					{
					   if(xh==8)                  //第八个探头就一组阈值，两组参数设置，共三行
					   {
					   	   wb=3;                  //从第三行开始左移
						   bz=82;                 //最后一组参数的bz=82
					   }
					   else                       //其余探头都一组阈值，五组参数设置，共六行
					   {
						   wb=6;                  //从第六行开始左移
						   bz=xh*11;
					   }
                       Flag_Change=1;             //刷新探头数据
					}
				}
			}			
		}   
	}
	if(xh%2==0)                                   //双号探头（探头2，4， 6，8）坐标位置设置
	{
		if(Incdataright ==0)
		{    
			Incdataright = 4;	
           	bz--;                                //参数共五组，每组包含2对，每个设置完减1  
            if(bz%11==0)                         //每个探头共10个换算系数，和一组阈值设置，设置完11组，跳到下一探头设置
			{
				xh--;                             
				if(xh==7)                        
		        {
			        Flag_RefrPara=0;              //第8个探头和第7个探头的数据个数不同，在转换时要刷新界面
		        }
		        if(xh<1)
		        {
				     Flag_RefrPara=0; 
			         xh=Incinput;                 //探头显示在1到Incinput之间循环
		        }
			}  		
			if((xh%2)!=0)                         //双号探头转单号探头换行处理
			{
				if((bz%2)!=0)                   
				{
					wb--; 
					if(wb==0)                  
					{
					   wb=6;
					   bz=xh*11;
                       Flag_Change=1;
					}
				}
			}
			if((xh%2)==0)                        //双号探头换行处理
			{
				if((bz%2)==0)                   
				{ 
					wb--;
					if(wb==0)                   
					{
					   wb=6;
					   bz=xh*11;
                       Flag_Change=1;
					}
				}	
			}
		}
	} 
	Line_Change();
}

/****************************************
*         参数行坐标转换
****************************************/
void Line_Change()
{
	if(wb!=1)
	{
	    Flag_Para=1;
		if((xh%2)!=0)
		{
			if(bz%2==0)
			{
				Flag_Left=1;
			}
			if(bz%2!=0)
			{
				Flag_Right=1;            
			}
		}
		else if((xh%2)==0)
		{
			if(bz%2!=0)
			{
				Flag_Left=1;
			}
			if(bz%2==0)
			{
				Flag_Right=1;            
			}
		}  
	}
	else if(wb==1)
	{
		Flag_Yu=1; 	   	
	}
}
/**************************************
*         参数右移光标显示 
*************************************/
void dispaly_paramove()
{
 /*右移光标显示*/
    if(Flag_RefrRight==1)
     {
        if(Flag_Para==1)
		{
		    Flag_Para=0;
		    if(Flag_Left==1)
		    {
				Flag_Left=0;
				if(Incdataright==1)
				   {
					   Open(112+Incdataright*48-48,128+wb*68-68);		
				   }
				else if(Incdataright==2)
				   {
					   Close(112+Incdataright*48-96,128+wb*68-68);
					   Open(160+Incdataright*48-96,128+wb*68-68);
				   }	
				else if(Incdataright==3)
				   {
					   Close(160+Incdataright*48-144,128+wb*68-68);
					   Open(160+Incdataright*48-96,128+wb*68-68);
				   }	
				else if(Incdataright==4)
				   {
					   Close(160+Incdataright*48-144,128+wb*68-68);
					   Open(160+Incdataright*48-96,128+wb*68-68); 
				   }
			 }
			 if(Flag_Right==1)
			 {
				Flag_Right=0;
				if(Incdataright==1)
				   {   
					   Open(432+Incdataright*48-48,128+wb*68-68);		
				   }
				else if(Incdataright==2)
				   {
					   Close(432+Incdataright*48-96,128+wb*68-68);
					   Open(480+Incdataright*48-96,128+wb*68-68);
				   }	
				else if(Incdataright==3)
				   {
					   Close(480+Incdataright*48-144,128+wb*68-68);
					   Open(480+Incdataright*48-98,128+wb*68-68);
				   }	
				else if(Incdataright==4)
				   {
				       Close(432,128);
					   Close(480+Incdataright*48-144,128+wb*68-68);
					   Open(480+Incdataright*48-96,128+wb*68-68); 
				   }
			    }
			 }
		 if(Flag_Change==1)
	     {
	        Flag_Change=0;
	        dispaly_para();
		 }
		 if(Flag_Yu==1)
		 {
			Flag_Yu=0;
			if(Incdataright==1)
			   {   
				   Open(432,128);		
			   }
			else if(Incdataright==2)
			   {
				   Close(432,128);
				   Open(480,128);
			   }	
			else if(Incdataright==3)
			   {
				   Close(480,128);
				   Open(528,128);
			   }	
			else if(Incdataright==4)
			   {
				   Close(528,128);
				   Open(576,128); 
			   }
		  }
    }       
}


/**********************************************
*             参数显示 
**********************************************/
void dispaly_para()
{
      if(Flag_RefrPara==0)
      {
	   Flag_RefrPara=1;
	   Clear();
	   Lcd_Clear();
	   Txtext(200,12,"第");
	   Tnumber(260,12,xh);
	   Txtext(296,12,"个探头");
       Txtext(112,80,"报警阈值:");
       Tnumber(432,80,YuThouth[xh-1]);
       Txtext(456,80,".");
	   Tnumber(480,80,YuCent[xh-1]);
	   Tnumber(528,80,YuTenth[xh-1]);
	   Tnumber(576,80,YuGe[xh-1]);
	   Txtext(16,148,"N0:");
	   Tnumber(112,148,DataThouth[xh*10-9]);
	   Tnumber(160,148,DataCent[xh*10-9]);
	   Tnumber(208,148,DataTenth[xh*10-9]);
	   Tnumber(256,148,DataGe[xh*10-9]);

	   Txtext(336,148,"f0:");
	   Tnumber(432,148,DataThouth[xh*10-8]);
	   Tnumber(480,148,DataCent[xh*10-8]);
	   Tnumber(528,148,DataTenth[xh*10-8]);
	   Tnumber(576,148,DataGe[xh*10-8]);

	   Txtext(16,216,"N1:");
	   Tnumber(112,216,DataThouth[xh*10-7]);
	   Tnumber(160,216,DataCent[xh*10-7]);
	   Tnumber(208,216,DataTenth[xh*10-7]);
	   Tnumber(256,216,DataGe[xh*10-7]);
	  
       
	   Txtext(336,216,"f1:");
	   Tnumber(432,216,DataThouth[xh*10-6]);
	   Tnumber(480,216,DataCent[xh*10-6]);
	   Tnumber(528,216,DataTenth[xh*10-6]);
	   Tnumber(576,216,DataGe[xh*10-6]);
	   if(xh<=7)                            //第八个探头只有三段参数
	   { 
		   Txtext(16,284,"N2:");
		   Tnumber(112,284,DataThouth[xh*10-5]);
		   Tnumber(160,284,DataCent[xh*10-5]);
		   Tnumber(208,284,DataTenth[xh*10-5]);
		   Tnumber(256,284,DataGe[xh*10-5]);
	       
		   Txtext(336,284,"f2:");
		   Tnumber(432,284,DataThouth[xh*10-4]);
		   Tnumber(480,284,DataCent[xh*10-4]);
		   Tnumber(528,284,DataTenth[xh*10-4]);
		   Tnumber(576,284,DataGe[xh*10-4]);
	     
		   Txtext(16,352,"N3:");
		   Tnumber(112,352,DataThouth[xh*10-3]);
		   Tnumber(160,352,DataCent[xh*10-3]);
	       Tnumber(208,352,DataTenth[xh*10-3]);
	       Tnumber(256,352,DataGe[xh*10-3]);

		   Txtext(336,352,"f3:");
		   Tnumber(432,352,DataThouth[xh*10-2]);
		   Tnumber(480,352,DataCent[xh*10-2]);
	       Tnumber(528,352,DataTenth[xh*10-2]);
	       Tnumber(576,352,DataGe[xh*10-2]);

		   Txtext(16,420,"N4:");
		   Tnumber(112,420,DataThouth[xh*10-1]);
		   Tnumber(160,420,DataCent[xh*10-1]);
	       Tnumber(208,420,DataTenth[xh*10-1]);
	       Tnumber(256,420,DataGe[xh*10-1]);
		
		   Txtext(336,420,"f4:");
		   Tnumber(432,420,DataThouth[xh*10]);
		   Tnumber(480,420,DataCent[xh*10]);
	       Tnumber(528,420,DataTenth[xh*10]);
	       Tnumber(576,420,DataGe[xh*10]); 
	    } 
    }
    else if(Flag_RefrPara==1)
    {   
	   Tnumber(260,12,xh);
       Tnumber(432,80,YuThouth[xh-1]);
	   Tnumber(480,80,YuCent[xh-1]);
	   Tnumber(528,80,YuTenth[xh-1]);
	   Tnumber(576,80,YuGe[xh-1]);
	   Tnumber(112,148,DataThouth[xh*10-9]);
	   Tnumber(160,148,DataCent[xh*10-9]);
	   Tnumber(208,148,DataTenth[xh*10-9]);
	   Tnumber(256,148,DataGe[xh*10-9]);
	
	   Tnumber(432,148,DataThouth[xh*10-8]);
	   Tnumber(480,148,DataCent[xh*10-8]);
	   Tnumber(528,148,DataTenth[xh*10-8]);
	   Tnumber(576,148,DataGe[xh*10-8]);

	   Tnumber(112,216,DataThouth[xh*10-7]);
	   Tnumber(160,216,DataCent[xh*10-7]);
	   Tnumber(208,216,DataTenth[xh*10-7]);
	   Tnumber(256,216,DataGe[xh*10-7]);

	   Tnumber(432,216,DataThouth[xh*10-6]);
	   Tnumber(480,216,DataCent[xh*10-6]);
	   Tnumber(528,216,DataTenth[xh*10-6]);
	   Tnumber(576,216,DataGe[xh*10-6]);
	    if(xh<=7)                            //第八个探头只有三段参数
	     {
		   Tnumber(112,284,DataThouth[xh*10-5]);
		   Tnumber(160,284,DataCent[xh*10-5]);
		   Tnumber(208,284,DataTenth[xh*10-5]);
		   Tnumber(256,284,DataGe[xh*10-5]);
	       
		   Tnumber(432,284,DataThouth[xh*10-4]);
		   Tnumber(480,284,DataCent[xh*10-4]);
		   Tnumber(528,284,DataTenth[xh*10-4]);
		   Tnumber(576,284,DataGe[xh*10-4]);
	     
		   Tnumber(112,352,DataThouth[xh*10-3]);
		   Tnumber(160,352,DataCent[xh*10-3]);
	       Tnumber(208,352,DataTenth[xh*10-3]);
	       Tnumber(256,352,DataGe[xh*10-3]);

		   Tnumber(432,352,DataThouth[xh*10-2]);
		   Tnumber(480,352,DataCent[xh*10-2]);
	       Tnumber(528,352,DataTenth[xh*10-2]);
	       Tnumber(576,352,DataGe[xh*10-2]);

		   Tnumber(112,420,DataThouth[xh*10-1]);
		   Tnumber(160,420,DataCent[xh*10-1]);
	       Tnumber(208,420,DataTenth[xh*10-1]);
	       Tnumber(256,420,DataGe[xh*10-1]);
		
		   Tnumber(432,420,DataThouth[xh*10]);
		   Tnumber(480,420,DataCent[xh*10]);
	       Tnumber(528,420,DataTenth[xh*10]);
	       Tnumber(576,420,DataGe[xh*10]); 
	    } 
    	Incdata++;
        if(Incdata>9)
		{
		 Incdata=0;
		}
	 }
}
