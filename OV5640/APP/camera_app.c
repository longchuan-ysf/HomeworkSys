#include "sys.h"
#include "delay.h"
#include "usart.h" 
#include "led.h"
#include "key.h"
#include "ltdc.h"
#include "tftlcd.h"
#include "sdram.h"
#include "usmart.h"
#include "pcf8574.h"
#include "ov5640.h"
#include "mpu.h"
#include "dcmi.h"
#include "malloc.h"
#include "w25qxx.h"
#include "sdmmc_sdcard.h"
#include "nand.h"    
#include "ftl.h"  
#include "ff.h"
#include "exfuns.h"
#include "fontupd.h"
#include "text.h"
#include "camera_app.h"
#include "string.h"		
#include "math.h"
#include "jpegcodec.h"
/************************************************
 ALIENTEK ������STM32H7������ ʵ��46
 �����ʵ��-HAL�⺯����
 ����֧�֣�www.openedv.com
 �Ա����̣�http://eboard.taobao.com 
 ��ע΢�Ź���ƽ̨΢�źţ�"����ԭ��"����ѻ�ȡSTM32���ϡ�
 ������������ӿƼ����޹�˾  
 ���ߣ�����ԭ�� @ALIENTEK
************************************************/

vu8 bmp_request=0;						//bmp��������:0,��bmp��������;1,��bmp��������,��Ҫ��֡�ж�����,�ر�DCMI�ӿ�.
u8 ovx_mode=0;							//bit0:0,RGB565ģʽ;1,JPEGģʽ 
u16 curline=0;							//����ͷ�������,��ǰ�б��
u16 yoffset=0;							//y�����ƫ����

#define jpeg_buf_size   6*1024*1024		//����JPEG���ݻ���jpeg_buf�Ĵ�С(6M�ֽ�)
#define jpeg_line_size	2*1024			//����DMA��������ʱ,һ�����ݵ����ֵ

u32 *dcmi_line_buf[2];					//RGB��ʱ,����ͷ����һ��һ�ж�ȡ,�����л���  
u32 *jpeg_data_buf;						//JPEG���ݻ���buf 

volatile u32 jpeg_data_len=0; 			//buf�е�JPEG��Ч���ݳ��� 
volatile u8 jpeg_data_ok=0;				//JPEG���ݲɼ���ɱ�־ 
										//0,����û�вɼ���;
										//1,���ݲɼ�����,���ǻ�û����;
										//2,�����Ѿ����������,���Կ�ʼ��һ֡����
//����JPEG����
//���ɼ���һ֡JPEG���ݺ�,���ô˺���,�л�JPEG BUF.��ʼ��һ֡�ɼ�.
void jpeg_data_process(void)
{
	u16 i;
	u16 rlen;			//ʣ�����ݳ���
	u32 *pbuf;
	curline=yoffset;	//������λ
	if(ovx_mode&0X01)	//ֻ����JPEG��ʽ��,����Ҫ������.
	{
		if(jpeg_data_ok==0)	//jpeg���ݻ�δ�ɼ���?
		{
			DMA1_Stream1->CR&=~(1<<0);		//ֹͣ��ǰ����
			while(DMA1_Stream1->CR&0X01);	//�ȴ�DMA1_Stream1������ 
			
			rlen=jpeg_line_size-DMA1_Stream1->NDTR;//�õ�ʣ�����ݳ���
			
			pbuf=jpeg_data_buf+jpeg_data_len;//ƫ�Ƶ���Ч����ĩβ,�������
			
			if(DMA1_Stream1->CR&(1<<19))
			{
				for(i=0;i<rlen;i++)
					pbuf[i]=dcmi_line_buf[1][i];//��ȡbuf1�����ʣ������
			}
			else 
			{
				for(i=0;i<rlen;i++)
					pbuf[i]=dcmi_line_buf[0][i];//��ȡbuf0�����ʣ������ 
			}
			jpeg_data_len+=rlen;			//����ʣ�೤��
			jpeg_data_ok=1; 				//���JPEG���ݲɼ��갴��,�ȴ�������������
		}
		if(jpeg_data_ok==2)	//��һ�ε�jpeg�����Ѿ���������
		{
			DMA1_Stream1->NDTR=jpeg_line_size;//���䳤��Ϊjpeg_buf_size*4�ֽ�
			DMA1_Stream1->CR|=1<<0;			//���´���
			jpeg_data_ok=0;					//�������δ�ɼ�
			jpeg_data_len=0;				//�������¿�ʼ
		}
	}
} 
//jpeg���ݽ��ջص�����
void jpeg_dcmi_rx_callback(void)
{  
	u16 i;
	u32 *pbuf;
	pbuf=jpeg_data_buf+jpeg_data_len;//ƫ�Ƶ���Ч����ĩβ
	if(DMA1_Stream1->CR&(1<<19))//buf0����,��������buf1
	{ 
		for(i=0;i<jpeg_line_size;i++)
			pbuf[i]=dcmi_line_buf[0][i];//��ȡbuf0���������
		
		jpeg_data_len+=jpeg_line_size;//ƫ��
	}else //buf1����,��������buf0
	{
		for(i=0;i<jpeg_line_size;i++)
			pbuf[i]=dcmi_line_buf[1][i];//��ȡbuf1���������
		
		jpeg_data_len+=jpeg_line_size;//ƫ�� 
	} 
} 

//RGB�����ݽ��ջص�����
void rgblcd_dcmi_rx_callback(void)
{  
	u16 *pbuf;
	if(DMA1_Stream1->CR&(1<<19))//DMAʹ��buf1,��ȡbuf0
	{ 
		pbuf=(u16*)dcmi_line_buf[0]; 
	}else 						//DMAʹ��buf0,��ȡbuf1
	{
		pbuf=(u16*)dcmi_line_buf[1]; 
	} 	
	LTDC_Color_Fill(0,curline,lcddev.width-1,curline,pbuf);//DM2D��� 
	if(curline<lcddev.height)
	{
		curline++;
	}
}


//�ļ������������⸲�ǣ�
//mode:0,����.bmp�ļ�;1,����.jpg�ļ�.
//bmp��ϳ�:����"0:PHOTO/PIC13141.bmp"���ļ���
//jpg��ϳ�:����"0:PHOTO/PIC13141.jpg"���ļ���
void camera_new_pathname(u8 *pname)
{	 
	u8 res;					 
	u16 index=0;
	while(index<0XFFFF)
	{
		sprintf((char*)pname,"%s/%s/PIC%05d.jpg",SAVE_DISK,SAVE_FLODER,index);
		res=f_open(ftemp,(const TCHAR*)pname,FA_READ);//���Դ�����ļ�
		if(res==FR_NO_FILE)
			break;		//���ļ���������=����������Ҫ��.
		index++;
	}
}  
void OV5640_Save_photo(void)
{
	uint8_t * pname;
	pname=mymalloc(SRAMIN,30);//Ϊ��·�����ļ�������30���ֽڵ��ڴ�	
	camera_new_pathname(pname);   //�õ��ļ���	
}
//OV5640����jpgͼƬ
//����ֵ:0,�ɹ�
//    ����,�������
u8 ov5640_jpg_photo(void)
{
	FIL* f_jpg; 
	u8 res=0,headok=0;
	u32 bwr;
	u32 i,jpgstart,jpglen;
	u8* pbuf;
	f_jpg=(FIL *)mymalloc(SRAMIN,sizeof(FIL));	//����FIL�ֽڵ��ڴ����� 
	if(f_jpg==NULL)return 0XFF;				//�ڴ�����ʧ��.
	ovx_mode=1;
	jpeg_data_ok=0;						
	OV5640_JPEG_Mode();						//JPEGģʽ  
	OV5640_OutSize_Set(16,4,2592,1944);		//��������ߴ�(500W)  
	dcmi_rx_callback=jpeg_dcmi_rx_callback;	//JPEG�������ݻص�����
	DCMI_DMA_Init((u32)dcmi_line_buf[0],(u32)dcmi_line_buf[1],jpeg_line_size,DMA_MDATAALIGN_WORD,DMA_MINC_ENABLE);//DCMI DMA����    
	DCMI_Start(); 			//��������  
	while(jpeg_data_ok!=1);	//�ȴ���һ֡ͼƬ�ɼ���
	jpeg_data_ok=2;			//���Ա�֡ͼƬ,������һ֡�ɼ� 
	while(jpeg_data_ok!=1);	//�ȴ��ڶ�֡ͼƬ�ɼ���,�ڶ�֡,�ű��浽SD��ȥ. 
	DCMI_Stop(); 			//ֹͣDMA����
	ovx_mode=0; 
//	res=f_open(f_jpg,(const TCHAR*)pname,FA_WRITE|FA_CREATE_NEW);//ģʽ0,���߳��Դ�ʧ��,�򴴽����ļ�	 
//	if(res==0)
//	{
//		printf("jpeg data size:%d\r\n",jpeg_data_len*4);//���ڴ�ӡJPEG�ļ���С
//		pbuf=(u8*)jpeg_data_buf;
//		jpglen=0;	//����jpg�ļ���СΪ0
//		headok=0;	//���jpgͷ���
//		for(i=0;i<jpeg_data_len*4;i++)//����0XFF,0XD8��0XFF,0XD9,��ȡjpg�ļ���С
//		{
//			if((pbuf[i]==0XFF)&&(pbuf[i+1]==0XD8))//�ҵ�FF D8
//			{
//				jpgstart=i;
//				headok=1;	//����ҵ�jpgͷ(FF D8)
//			}
//			if((pbuf[i]==0XFF)&&(pbuf[i+1]==0XD9)&&headok)//�ҵ�ͷ�Ժ�,����FF D9
//			{
//				jpglen=i-jpgstart+2;
//				break;
//			}
//		}
//		if(jpglen)			//������jpeg���� 
//		{
//			pbuf+=jpgstart;	//ƫ�Ƶ�0XFF,0XD8��
//			res=f_write(f_jpg,pbuf,jpglen,&bwr);
//			if(bwr!=jpglen)res=0XFE; 
//			
//		}else res=0XFD; 
//	}
//	jpeg_data_len=0;
//	f_close(f_jpg); 
	OV5640_RGB565_Mode();	//RGB565ģʽ  
	if(lcdltdc.pwidth!=0)	//RGB��
	{
		dcmi_rx_callback=rgblcd_dcmi_rx_callback;//RGB���������ݻص�����
		DCMI_DMA_Init((u32)dcmi_line_buf[0],(u32)dcmi_line_buf[1],lcddev.width/2,DMA_MDATAALIGN_HALFWORD,DMA_MINC_ENABLE);//DCMI DMA����  
	}else					//MCU ��
	{
		DCMI_DMA_Init((u32)&LCD->LCD_RAM,0,1,DMA_MDATAALIGN_HALFWORD,DMA_MINC_DISABLE);			//DCMI DMA����,MCU��,����
	}
	myfree(SRAMIN,f_jpg); 
	return res;
}  
//RGB565����
//RGB����ֱ����ʾ��LCD����
void rgb565_test(void)
{ 
	u8 key;
	u8 contrast=2,fac;
	u8 msgbuf[15];	//��Ϣ������ 
	u16 outputheight=0;
	
	LCD_Clear(WHITE);
	//�Զ��Խ���ʼ��
	OV5640_RGB565_Mode();	//RGB565ģʽ	
	OV5640_Focus_Init();
	OV5640_Light_Mode(0);	//�Զ�ģʽ
	OV5640_Color_Saturation(3);//ɫ�ʱ��Ͷ�0
	OV5640_Brightness(4);	//����0
	OV5640_Contrast(3);		//�Աȶ�0
	OV5640_Sharpness(33);	//�Զ����
	OV5640_Focus_Constant();//���������Խ�
	DCMI_Init();			//DCMI����
	if(lcdltdc.pwidth!=0)	//RGB��
	{
		dcmi_rx_callback=rgblcd_dcmi_rx_callback;//RGB���������ݻص�����
        DCMI_DMA_Init((u32)dcmi_line_buf[0],(u32)dcmi_line_buf[1],lcddev.width/2,DMA_MDATAALIGN_HALFWORD,DMA_MINC_ENABLE);
	}else					//MCU ��
	{
        DCMI_DMA_Init((u32)&LCD->LCD_RAM,0,1,DMA_MDATAALIGN_HALFWORD,DMA_MINC_DISABLE);
	}
	
	if(lcddev.height>=800)  
	{
		yoffset=(lcddev.height-800)/2;
		outputheight=800;
		OV5640_WR_Reg(0x3035,0X51);//�������֡�ʣ�������ܶ���
	}else 
	{
		yoffset=0;
		outputheight=lcddev.height;
	}
	curline=yoffset;		//������λ
	fac=800/outputheight;	//�õ���������
 	OV5640_OutSize_Set((1280-fac*lcddev.width)/2,(800-fac*outputheight)/2,lcddev.width,outputheight); //1:1��ʾ
	DCMI_Start(); 			//��������
	while(1)
	{ 
		key=KEY_Scan(0); 
		if(key)
		{ 
			if(key!=KEY1_PRES)
				DCMI_Stop(); //��KEY1����,ֹͣ��ʾ_���ڶԱȶ�
			switch(key)
			{				    
				case KEY0_PRES:	//�Աȶ�����
				{
					contrast++;
					if(contrast>6)contrast=0;
					OV5640_Contrast(contrast);
					printf("Contrast:%d",(signed char)contrast-3);
				}
				break;
				case KEY1_PRES:	//ִ��һ���Զ��Խ�
				{
					OV5640_Focus_Single();
				}
				break;
				default:
					break;
				
			}
			if(key!=KEY1_PRES)	//��KEY1����
			{
				delay_ms(800); 
				DCMI_Start();	//���¿�ʼ����
			}
		} 
		delay_ms(10);		
	}    
} 

void camera_app_main(void)
{
							 
	dcmi_line_buf[0]=mymalloc(SRAMIN,jpeg_line_size*4);	//Ϊjpeg dma���������ڴ�	
	dcmi_line_buf[1]=mymalloc(SRAMIN,jpeg_line_size*4);	//Ϊjpeg dma���������ڴ�	
	jpeg_data_buf=mymalloc(SRAMEX,jpeg_buf_size);		//Ϊjpeg�ļ������ڴ�(���4MB)
 	
	while(OV5640_Init())//��ʼ��OV5640
	{
		printf("camera err! please check it!\r\n");
		delay_ms(1000);
	}	
	
 	while(1)
    {      
		rgb565_test(); 
	}	
}

