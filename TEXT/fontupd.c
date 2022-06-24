#include "fontupd.h"
#include "ff.h"	  
#include "w25qxx.h"   
#include "tftlcd.h"  
#include "string.h"
#include "malloc.h"
#include "delay.h"
#include "usart.h"
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ���������ɣ��������������κ���;
//ALIENTEK STM32������ 
//�ֿ���� ��������	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2016/1/15
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) �������������ӿƼ����޹�˾ 2014-2024
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 	 

//�ֿ�����ռ�õ�����������С(4���ֿ�+unigbk��+�ֿ���Ϣ=6302984�ֽ�,Լռ1539��W25QXX����,һ������4K�ֽ�)
#define FONTSECSIZE	 	1539
//�ֿ�����ʼ��ַ 
#define FONTINFOADDR 	1024*1024*25 					//Apollo STM32�������Ǵ�25M��ַ�Ժ�ʼ����ֿ�
														//ǰ��25M��fatfsռ����.
														//25M�Ժ����4���ֿ�+UNIGBK.BIN,�ܴ�С6.01M,���ֿ�ռ����,���ܶ�!
														//31.01M�Ժ�,�û���������ʹ��.
														
//���������ֿ������Ϣ����ַ����С��
_font_info ftinfo;

//�ֿ����ڴ����е�·��
u8*const GBK_PATH[5]=
{
"/SYSTEM/FONT/UNIGBK.BIN",	//UNIGBK.BIN�Ĵ��λ��
"/SYSTEM/FONT/GBK12.FON",	//GBK12�Ĵ��λ��
"/SYSTEM/FONT/GBK16.FON",	//GBK16�Ĵ��λ��
"/SYSTEM/FONT/GBK24.FON",	//GBK24�Ĵ��λ��
"/SYSTEM/FONT/GBK32.FON",	//GBK32�Ĵ��λ��
}; 
//����ʱ����ʾ��Ϣ
u8*const UPDATE_REMIND_TBL[5]=
{
"Updating UNIGBK.BIN",	//��ʾ���ڸ���UNIGBK.bin
"Updating GBK12.FON",	//��ʾ���ڸ���GBK12
"Updating GBK16.FON",	//��ʾ���ڸ���GBK16
"Updating GBK24.FON",	//��ʾ���ڸ���GBK24
"Updating GBK32.FON",	//��ʾ���ڸ���GBK32
}; 

//��ʾ��ǰ������½���
//x,y:����
//size:�����С
//fsize:�����ļ���С
//pos:��ǰ�ļ�ָ��λ��
u32 fupd_prog(u16 x,u16 y,u8 size,u32 fsize,u32 pos)
{
	float prog;
	u8 t=0XFF;
	prog=(float)pos/fsize;
	prog*=100;
	if(t!=prog)
	{
		LCD_ShowString(x+3*size/2,y,240,320,size,"%");		
		t=prog;
		if(t>100)t=100;
		LCD_ShowNum(x,y,t,3,size);//��ʾ��ֵ
	}
	return 0;					    
} 
//����ĳһ��
//x,y:����
//size:�����С
//fxpath:·��
//fx:���µ����� 0,ungbk;1,gbk12;2,gbk16;3,gbk24;4,gbk32;
//����ֵ:0,�ɹ�;����,ʧ��.
u8 updata_fontx(u16 x,u16 y,u8 size,u8 *fxpath,u8 fx)
{
	u32 flashaddr=0;								    
	FIL * fftemp;
	u8 *tempbuf;
 	u8 res;	
	u16 bread;
	u32 offx=0;
	u8 rval=0;	     
	fftemp=(FIL*)mymalloc(SRAMIN,sizeof(FIL));	//�����ڴ�	
	if(fftemp==NULL)rval=1;
	tempbuf=mymalloc(SRAMIN,4096);				//����4096���ֽڿռ�
	if(tempbuf==NULL)rval=1;
 	res=f_open(fftemp,(const TCHAR*)fxpath,FA_READ); 
 	if(res)rval=2;//���ļ�ʧ��  
 	if(rval==0)	 
	{
		switch(fx)
		{
			case 0:												//����UNIGBK.BIN
				ftinfo.ugbkaddr=FONTINFOADDR+sizeof(ftinfo);	//��Ϣͷ֮�󣬽���UNIGBKת�����
				ftinfo.ugbksize=fftemp->obj.objsize;					//UNIGBK��С
				flashaddr=ftinfo.ugbkaddr;
				break;
			case 1:
				ftinfo.f12addr=ftinfo.ugbkaddr+ftinfo.ugbksize;	//UNIGBK֮�󣬽���GBK12�ֿ�
				ftinfo.gbk12size=fftemp->obj.objsize;					//GBK12�ֿ��С
				flashaddr=ftinfo.f12addr;						//GBK12����ʼ��ַ
				break;
			case 2:
				ftinfo.f16addr=ftinfo.f12addr+ftinfo.gbk12size;	//GBK12֮�󣬽���GBK16�ֿ�
				ftinfo.gbk16size=fftemp->obj.objsize;					//GBK16�ֿ��С
				flashaddr=ftinfo.f16addr;						//GBK16����ʼ��ַ
				break;
			case 3:
				ftinfo.f24addr=ftinfo.f16addr+ftinfo.gbk16size;	//GBK16֮�󣬽���GBK24�ֿ�
				ftinfo.gbk24size=fftemp->obj.objsize;					//GBK24�ֿ��С
				flashaddr=ftinfo.f24addr;						//GBK24����ʼ��ַ
				break;
			case 4:
				ftinfo.f32addr=ftinfo.f24addr+ftinfo.gbk24size;	//GBK24֮�󣬽���GBK32�ֿ�
				ftinfo.gbk32size=fftemp->obj.objsize;					//GBK32�ֿ��С
				flashaddr=ftinfo.f32addr;						//GBK32����ʼ��ַ
				break;
		} 
			
		while(res==FR_OK)//��ѭ��ִ��
		{
	 		res=f_read(fftemp,tempbuf,4096,(UINT *)&bread);		//��ȡ����	 
			if(res!=FR_OK)break;								//ִ�д���
			W25QXX_Write(tempbuf,offx+flashaddr,4096);		//��0��ʼд��4096������  
	  		offx+=bread;	  
			fupd_prog(x,y,size,fftemp->obj.objsize,offx);	 			//������ʾ
			if(bread!=4096)break;								//������.
	 	} 	
		f_close(fftemp);		
	}			 
	myfree(SRAMIN,fftemp);	//�ͷ��ڴ�
	myfree(SRAMIN,tempbuf);	//�ͷ��ڴ�
	return res;
} 
//���������ļ�,UNIGBK,GBK12,GBK16,GBK24,GBK32һ�����
//x,y:��ʾ��Ϣ����ʾ��ַ
//size:�����С
//src:�ֿ���Դ����."0:",SD��;"1:",FLASH��,"2:",U��.
//��ʾ��Ϣ�����С										  
//����ֵ:0,���³ɹ�;
//		 ����,�������.	  
u8 update_font(u16 x,u16 y,u8 size,u8* src)
{	
	u8 *pname;
	u32 *buf;
	u8 res=0;		   
 	u16 i,j;
	FIL *fftemp;
	u8 rval=0; 
	res=0XFF;		
	ftinfo.fontok=0XFF;
	pname=mymalloc(SRAMIN,100);	//����100�ֽ��ڴ�  
	buf=mymalloc(SRAMIN,4096);	//����4K�ֽ��ڴ�  
	fftemp=(FIL*)mymalloc(SRAMIN,sizeof(FIL));	//�����ڴ�	
	if(buf==NULL||pname==NULL||fftemp==NULL)
	{
		myfree(SRAMIN,fftemp);
		myfree(SRAMIN,pname);
		myfree(SRAMIN,buf);
		return 5;		//�ڴ�����ʧ��
	}
	for(i=0;i<5;i++)	//�Ȳ����ļ�UNIGBK,GBK12,GBK16,GBK24,GBK32�Ƿ����� 
	{ 
		strcpy((char*)pname,(char*)src);				//copy src���ݵ�pname
		strcat((char*)pname,(char*)GBK_PATH[i]);		//׷�Ӿ����ļ�·�� 
		res=f_open(fftemp,(const TCHAR*)pname,FA_READ);	//���Դ�
		if(res)
		{
			rval|=1<<7;	//��Ǵ��ļ�ʧ��  
			break;		//������,ֱ���˳�
		}
	} 
	myfree(SRAMIN,fftemp);	//�ͷ��ڴ�
	if(rval==0)				//�ֿ��ļ�������.
	{  
		LCD_ShowString(x,y,240,320,size,"Erasing sectors... ");//��ʾ���ڲ�������	
		for(i=0;i<FONTSECSIZE;i++)			//�Ȳ����ֿ�����,���д���ٶ�
		{
			fupd_prog(x+20*size/2,y,size,FONTSECSIZE,i);//������ʾ
			W25QXX_Read((u8*)buf,((FONTINFOADDR/4096)+i)*4096,4096);//������������������
			for(j=0;j<1024;j++)//У������
			{
				if(buf[j]!=0XFFFFFFFF)break;//��Ҫ����  	  
			}
			if(j!=1024)W25QXX_Erase_Sector((FONTINFOADDR/4096)+i);	//��Ҫ����������
		}
		for(i=0;i<5;i++)	//���θ���UNIGBK,GBK12,GBK16,GBK24,GBK32
		{
			LCD_ShowString(x,y,240,320,size,UPDATE_REMIND_TBL[i]);		
			strcpy((char*)pname,(char*)src);				//copy src���ݵ�pname
			strcat((char*)pname,(char*)GBK_PATH[i]); 		//׷�Ӿ����ļ�·�� 
			res=updata_fontx(x+20*size/2,y,size,pname,i);	//�����ֿ�
			if(res)
			{
				myfree(SRAMIN,buf);
				myfree(SRAMIN,pname);
				return 1+i;
			} 
		} 
		//ȫ�����º���
		ftinfo.fontok=0XAA;
		W25QXX_Write((u8*)&ftinfo,FONTINFOADDR,sizeof(ftinfo));	//�����ֿ���Ϣ
	}
	myfree(SRAMIN,pname);//�ͷ��ڴ� 
	myfree(SRAMIN,buf);
	return rval;//�޴���.			 
} 
//��ʼ������
//����ֵ:0,�ֿ����.
//		 ����,�ֿⶪʧ
u8 font_init(void)
{		
	u8 t=0;
	W25QXX_Init();  
	while(t<10)//������ȡ10��,���Ǵ���,˵��ȷʵ��������,�ø����ֿ���
	{
		t++;
		W25QXX_Read((u8*)&ftinfo,FONTINFOADDR,sizeof(ftinfo));//����ftinfo�ṹ������
		if(ftinfo.fontok==0XAA)break;
		delay_ms(20);
	}
	if(ftinfo.fontok!=0XAA)return 1;
	return 0;		    
}




























