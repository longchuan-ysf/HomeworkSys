#include "sys.h"
#include "delay.h"
#include "usart.h" 
#include "key.h"
#include "ltdc.h"
#include "tftlcd.h"
#include "sdram.h"
#include "pcf8574.h"
#include "ov5640.h"
#include "dcmi.h"
#include "camera_app.h"
#include "malloc.h"		
#include "ff.h"
#include "jpegcodec.h"
#include "usbh_app.h"
#include "buttonbmp.h"

u8 ovx_mode=0;							//bit0:0,RGB565ģʽ;1,JPEGģʽ 
u16 curline=0;							//����ͷ�������,��ǰ�б��
u16 yoffset=50;							//y�����ƫ����

#define jpeg_buf_size   4*1024*1024		//����JPEG���ݻ���jpeg_buf�Ĵ�С(4M�ֽ�)
#define jpeg_line_size	2*1024			//����DMA��������ʱ,һ�����ݵ����ֵ

u32 *dcmi_line_buf[2];					//RGB��ʱ,����ͷ����һ��һ�ж�ȡ,�����л���  
u32 *jpeg_data_buf;						//JPEG���ݻ���buf 

u16 *rgb565_data_buff;					//rgb565���������ݴ���

volatile u32 jpeg_data_len=0; 			//buf�е�JPEG��Ч���ݳ��� 
volatile u32 rgb565_data_len=0; 			//buf�е�rgb565��Ч���ݳ��� 
volatile u8 jpeg_data_ok=0;				//JPEG���ݲɼ���ɱ�־ 
										//0,����û�вɼ���;
										//1,���ݲɼ�����,���ǻ�û����;
										//2,�����Ѿ����������,���Կ�ʼ��һ֡����
u8 rgb565_data_ok=0;
u16 outputheight=0;

//����JPEG����
//���ɼ���һ֡JPEG���ݺ�,���ô˺���,�л�JPEG BUF.��ʼ��һ֡�ɼ�.
void jpeg_data_process(void)
{
	u16 i;
	u16 rlen;			//ʣ�����ݳ���
	u32 *pbuf;
	
	//������λ
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
	else
	{
		curline=0;
		rgb565_data_len = 0;
		rgb565_data_ok  = 1;
		DCMI_Stop(); 			//ֹͣDMA����
		
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
	uint32_t i;

	if(DMA1_Stream1->CR&(1<<19))//DMAʹ��buf1,��ȡbuf0
	{ 
		pbuf=(u16*)dcmi_line_buf[0]; 
	}else 						//DMAʹ��buf0,��ȡbuf1
	{
		pbuf=(u16*)dcmi_line_buf[1]; 
	}
	for(i=0;i<lcddev.width;i++)
	{
		rgb565_data_buff[rgb565_data_len+i] = pbuf[i];
	}
	rgb565_data_len+=i;
	//printf("i=%d\r\n",i);
	//LTDC_Color_Fill(0,curline,lcddev.width-1,curline,pbuf);//DM2D��� 
//	if(curline<lcddev.height)
		curline++;

}

//RGB565����
//RGB����ֱ����ʾ��LCD����
void rgb565_test(void)
{ 
	u8 fac;
	
	ovx_mode=0;
	if(lcdltdc.pwidth!=0)	//RGB��
	{
		dcmi_rx_callback=rgblcd_dcmi_rx_callback;//RGB���������ݻص�����
		DCMI_DMA_Init((u32)dcmi_line_buf[0],(u32)dcmi_line_buf[1],lcddev.width/2,DMA_MDATAALIGN_HALFWORD,DMA_MINC_ENABLE);//DCMI DMA����  
	}else					//MCU ��
	{
		DCMI_DMA_Init((u32)&LCD->LCD_RAM,0,1,DMA_MDATAALIGN_HALFWORD,DMA_MINC_DISABLE);			//DCMI DMA����,MCU��,����
	}
	OV5640_WR_Reg(0x3035,0X51);//�������֡�ʣ�������ܶ���
	yoffset = 50;
	outputheight = 750;
	curline=yoffset;		//������λ
	fac=800/outputheight;	//�õ���������
 	OV5640_OutSize_Set((1280-fac*lcddev.width)/2,(800-fac*outputheight)/2,lcddev.width,outputheight); //1:1��ʾ		
	rgb565_data_ok=0;
	//DCMI_Start(); 			//��������  
} 

//�ļ������������⸲�ǣ�
//jpg��ϳ�:����"0:PHOTO/PIC13141.jpg"���ļ���
void camera_new_pathname(u8 *pname)
{	 

	u16 index=0;
	index=rand();
	sprintf((char*)pname,"%s/%s/PIC%05d.jpg",SAVE_DISK,SAVE_FLODER,index);
}  
uint8_t OV5640_Save_photo(void)
{
	uint8_t * pbuf,*dir;;
	uint8_t res,headok;
	uint32_t i,jpgstart,jpglen;
	u16 timeout;
    pUSBH_WR_MSG pMsgWR;
	
	dir = mymalloc(SRAMIN,16);
	if(!dir)
	{
		printf("%s malloc dir err!\r\n",__func__);
		return 1;
	}
	mymemset(dir,0,16);
	sprintf((char *)dir,"%s/%s",SAVE_DISK,SAVE_FLODER);
	
	res=f_mkdir((TCHAR *)dir);		//����PHOTO�ļ���
	if(res!=FR_EXIST&&res!=FR_OK) 	//�����˴���
	{	
		printf("save err in open dir\r\n");	
		return 2;		
	}	
	pMsgWR = USBH_Malloc_CtrlStruct();
	
	USBH_Malloc_Path(pMsgWR,30);
	
	camera_new_pathname(pMsgWR->path);

	printf("jpeg data size:%d\r\n",jpeg_data_len*4);//���ڴ�ӡJPEG�ļ���С
	pbuf=(u8*)jpeg_data_buf;
	jpglen=0;	//����jpg�ļ���СΪ0
	headok=0;	//���jpgͷ���
	for(i=0;i<jpeg_data_len*4;i++)//����0XFF,0XD8��0XFF,0XD9,��ȡjpg�ļ���С
	{
		if((pbuf[i]==0XFF)&&(pbuf[i+1]==0XD8))//�ҵ�FF D8
		{
			jpgstart=i;
			headok=1;	//����ҵ�jpgͷ(FF D8)
		}
		if((pbuf[i]==0XFF)&&(pbuf[i+1]==0XD9)&&headok)//�ҵ�ͷ�Ժ�,����FF D9
		{
			jpglen=i-jpgstart+2;
			break;
		}
	}

	if(jpglen)			//������jpeg���� 
	{
		pbuf+=jpgstart;	//ƫ�Ƶ�0XFF,0XD8��
		
		USBH_Malloc_WriteBuf(pMsgWR,WR_BUFF_EX,pbuf);
	
		USBH_ApplyFor_WR(pMsgWR,jpglen,FA_WRITE|FA_CREATE_ALWAYS);
		
		timeout = 0;
		while(1)
		{
			timeout++;
			if(pMsgWR->result==FR_OK)
			{
				USBH_WR_MsgFree(pMsgWR);
				printf("write ok\r\n");
				res= 0;
				break;
			}
			if(timeout>10*1000)//10s�ĳ�ʱ�ж�
			{
				res= 3;
				break;
			}
			delay_ms(10);//��ʱ10ms
		}
		if(pMsgWR->bread!=jpglen)
			res=4; 
		
	}
	else
	{
		res=5; 
	}
	
	jpeg_data_len=0;
	
	
	return res;
}
//OV5640����jpgͼƬ
//����ֵ:0,�ɹ�
//    ����,�������
u8 ov5640_jpg_photo(void)
{
	
	u8 res=0,fac;
	u16 outputheight=0;
	printf("%s\r\n",__func__);
	ovx_mode=1;
	jpeg_data_ok=0;
	jpeg_data_len=0;	
	OV5640_JPEG_Mode();						//JPEGģʽ  
	OV5640_OutSize_Set(16,4,2592,1944);		//��������ߴ�(500W)  
	dcmi_rx_callback=jpeg_dcmi_rx_callback;	//JPEG�������ݻص�����
	DCMI_DMA_Init((u32)dcmi_line_buf[0],(u32)dcmi_line_buf[1],jpeg_line_size,DMA_MDATAALIGN_WORD,DMA_MINC_ENABLE);//DCMI DMA����    
	DCMI_Start(); 			//��������  
	while(jpeg_data_ok!=1);	//�ȴ���һ֡ͼƬ�ɼ���
	jpeg_data_ok=2;			//���Ա�֡ͼƬ,������һ֡�ɼ� 
	while(jpeg_data_ok!=1);	//�ȴ��ڶ�֡ͼƬ�ɼ���,�ڶ�֡,�ű��浽SD��ȥ. 
	DCMI_Stop(); 			//ֹͣDMA����
	//DCMI_Init();			//DCMI����
	ovx_mode=0; 
	if(OV5640_Save_photo())
	{
		printf("Save photo err!\r\n");
	}

	OV5640_RGB565_Mode();	//RGB565ģʽ  
	rgb565_test();
	return res;
}  



void camera_app_init(void)
{
	outputheight=750;						 
	dcmi_line_buf[0]=mymalloc(SRAMIN,jpeg_line_size*4);	//Ϊjpeg dma���������ڴ�	*4����Ϊintռ4�ֽ�
	dcmi_line_buf[1]=mymalloc(SRAMIN,jpeg_line_size*4);	//Ϊjpeg dma���������ڴ�	
	jpeg_data_buf=mymalloc(SRAMEX,jpeg_buf_size);		//Ϊjpeg�ļ������ڴ�(���4MB)
 	rgb565_data_buff = mymalloc(SRAMEX,480*750*sizeof(unsigned short));//��Ϊ��buttonbmp.c��λͼ�Ĵ�С��480*750��unsigned short����
	
	bm2_c.pData = (unsigned char * )rgb565_data_buff;
	while(OV5640_Init())//��ʼ��OV5640
	{
		printf("camera err! please check it!\r\n");
		delay_ms(1000);
	}	
	OV5640_RGB565_Mode();	//RGB565ģʽ	
	OV5640_Focus_Init();
	OV5640_Light_Mode(0);	//�Զ�ģʽ
	OV5640_Color_Saturation(3);//ɫ�ʱ��Ͷ�0
	OV5640_Brightness(4);	//����0
	OV5640_Contrast(3);		//�Աȶ�0
	OV5640_Sharpness(33);	//�Զ����
	OV5640_Focus_Constant();//���������Խ�
	DCMI_Init();			//DCMI����
}
void Close_Camera(void)
{
	DCMI_Stop(); 			//ֹͣDMA����
}
void Start_Camera(void)
{
	ovx_mode=0;
	rgb565_data_len = 0;
	rgb565_data_ok  = 0;
	DCMI_Start(); 			//��������
}

void camera_key_handle(uint8_t key)
{
	if(key)
	{ 	
		switch(key)
		{				    
			case KEY0_PRES:	//�Աȶ�����
			{
				OV5640_Focus_Single();
			}
			break;
			case KEY1_PRES:	//ִ��һ���Զ��Խ�
			{
				if(!connect_usb)
					return;
				rgb565_data_ok=0;//��ֹ������ˢ��
				DCMI_Stop(); //��KEY1����,ֹͣ��ʾ_���ڶԱȶ�
				ov5640_jpg_photo();
				Start_Camera();	//���¿�ʼ����
			}
			break;
			case KEY2_PRES:
			{
				printf("KEY2_PRES\r\n");
			}
			break;
			case WKUP_PRES:
			{
				printf("WKUP_PRES\r\n");
			}
			break;
			default:
				break;
			
		}
		
	} 
}