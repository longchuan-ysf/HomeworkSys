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
#include "string.h"
#include "BackGround.h"
#include "MessageHandle.h"
u8 ovx_mode=0;							//bit0:0,RGB565模式;1,JPEG模式 
u16 curline=0;							//摄像头输出数据,当前行编号
u16 yoffset=50;							//y方向的偏移量

#define jpeg_buf_size   4*1024*1024		//定义JPEG数据缓存jpeg_buf的大小(4M字节)
#define jpeg_line_size	2*1024			//定义DMA接收数据时,一行数据的最大值

u32 *dcmi_line_buf[2];					//RGB屏时,摄像头采用一行一行读取,定义行缓存  
u32 *jpeg_data_buf;						//JPEG数据缓存buf 

u16 *rgb565_data_buff;					//rgb565数据数据暂存区

volatile u32 jpeg_data_len=0; 			//buf中的JPEG有效数据长度 
volatile u32 rgb565_data_len=0; 			//buf中的rgb565有效数据长度 
volatile u8 jpeg_data_ok=0;				//JPEG数据采集完成标志 
										//0,数据没有采集完;
										//1,数据采集完了,但是还没处理;
										//2,数据已经处理完成了,可以开始下一帧接收
u8 rgb565_data_ok=0;
u16 outputheight=0;

//处理JPEG数据
//当采集完一帧JPEG数据后,调用此函数,切换JPEG BUF.开始下一帧采集.
void jpeg_data_process(void)
{
	u16 i;
	u16 rlen;			//剩余数据长度
	u32 *pbuf;
	
	//行数复位
	if(ovx_mode&0X01)	//只有在JPEG格式下,才需要做处理.
	{
		if(jpeg_data_ok==0)	//jpeg数据还未采集完?
		{
			DMA1_Stream1->CR&=~(1<<0);		//停止当前传输
			while(DMA1_Stream1->CR&0X01);	//等待DMA1_Stream1可配置 
			
			rlen=jpeg_line_size-DMA1_Stream1->NDTR;//得到剩余数据长度
			
			pbuf=jpeg_data_buf+jpeg_data_len;//偏移到有效数据末尾,继续添加
			
			if(DMA1_Stream1->CR&(1<<19))
			{
				for(i=0;i<rlen;i++)
					pbuf[i]=dcmi_line_buf[1][i];//读取buf1里面的剩余数据
			}
			else 
			{
				for(i=0;i<rlen;i++)
					pbuf[i]=dcmi_line_buf[0][i];//读取buf0里面的剩余数据 
			}
			jpeg_data_len+=rlen;			//加上剩余长度
			jpeg_data_ok=1; 				//标记JPEG数据采集完按成,等待其他函数处理
		}
		if(jpeg_data_ok==2)	//上一次的jpeg数据已经被处理了
		{
			DMA1_Stream1->NDTR=jpeg_line_size;//传输长度为jpeg_buf_size*4字节
			DMA1_Stream1->CR|=1<<0;			//重新传输
			jpeg_data_ok=0;					//标记数据未采集
			jpeg_data_len=0;				//数据重新开始
		}
	}
	else
	{
		curline=0;
		rgb565_data_len = 0;
		rgb565_data_ok  = 1;
		DCMI_Stop(); 			//停止DMA搬运
		
	}
} 
//jpeg数据接收回调函数
void jpeg_dcmi_rx_callback(void)
{  
	u16 i;
	u32 *pbuf;
	pbuf=jpeg_data_buf+jpeg_data_len;//偏移到有效数据末尾
	if(DMA1_Stream1->CR&(1<<19))//buf0已满,正常处理buf1
	{ 
		for(i=0;i<jpeg_line_size;i++)
			pbuf[i]=dcmi_line_buf[0][i];//读取buf0里面的数据
		
		jpeg_data_len+=jpeg_line_size;//偏移
	}else //buf1已满,正常处理buf0
	{
		for(i=0;i<jpeg_line_size;i++)
			pbuf[i]=dcmi_line_buf[1][i];//读取buf1里面的数据
		
		jpeg_data_len+=jpeg_line_size;//偏移 
	} 
} 

//RGB屏数据接收回调函数
void rgblcd_dcmi_rx_callback(void)
{  
	u16 *pbuf;
	uint32_t i;

	if(DMA1_Stream1->CR&(1<<19))//DMA使用buf1,读取buf0
	{ 
		pbuf=(u16*)dcmi_line_buf[0]; 
	}else 						//DMA使用buf0,读取buf1
	{
		pbuf=(u16*)dcmi_line_buf[1]; 
	}
	for(i=0;i<lcddev.width;i++)
	{
		rgb565_data_buff[rgb565_data_len+i] = pbuf[i];
	}
	rgb565_data_len+=i;
	//printf("i=%d\r\n",i);
	//LTDC_Color_Fill(0,curline,lcddev.width-1,curline,pbuf);//DM2D填充 
//	if(curline<lcddev.height)
		curline++;

}

//RGB565测试
//RGB数据直接显示在LCD上面
void rgb565_test(void)
{ 
	u8 fac;
	
	ovx_mode=0;
	if(lcdltdc.pwidth!=0)	//RGB屏
	{
		dcmi_rx_callback=rgblcd_dcmi_rx_callback;//RGB屏接收数据回调函数
		DCMI_DMA_Init((u32)dcmi_line_buf[0],(u32)dcmi_line_buf[1],lcddev.width/2,DMA_MDATAALIGN_HALFWORD,DMA_MINC_ENABLE);//DCMI DMA配置  
	}else					//MCU 屏
	{
		DCMI_DMA_Init((u32)&LCD->LCD_RAM,0,1,DMA_MDATAALIGN_HALFWORD,DMA_MINC_DISABLE);			//DCMI DMA配置,MCU屏,竖屏
	}
	OV5640_WR_Reg(0x3035,0X51);//降低输出帧率，否则可能抖动
	yoffset = 50;
	outputheight = 750;
	curline=yoffset;		//行数复位
	fac=800/outputheight;	//得到比例因子
 	OV5640_OutSize_Set((1280-fac*lcddev.width)/2,(800-fac*outputheight)/2,lcddev.width,outputheight); //1:1显示		
	rgb565_data_ok=0;
	//DCMI_Start(); 			//启动传输  
} 

//文件名自增（避免覆盖）
//jpg组合成:形如"0:PHOTO/PIC13141.jpg"的文件名
void camera_new_pathname(u8 *pname)
{	 

//	u16 index=0;
//	index=rand();
//	sprintf((char*)pname,"%s/%s/PIC%05d.jpg",SAVE_DISK,SAVE_FLODER,index);
    u8 res;	
	FIL ftemp;
	u16 index=0;
    printf("%s\r\n",__func__);
	while(index<0XFFFF)
	{
        sprintf((char*)pname,"%s/%s/PIC%05d.jpg",SAVE_DISK,SAVE_FLODER,index);
		//sprintf((char*)pname,"0:PHOTO/PIC%05d.jpg",index);
		res=f_open(&ftemp,(const TCHAR*)pname,FA_READ);//尝试打开
		if(res)
		{
            if(res==FR_NO_FILE)
                break;      //文件不存在
            else
                printf("res=%d\r\n",res);
                
		}
		index++;
	}
}  
uint8_t OV5640_Save_photo(void)
{
	uint8_t *pbuf,*Directory,*path,*data,*Filename;
	uint8_t res,headok;
	uint32_t i,jpgstart,jpglen,br;
    FIL *fp;
    printf("%s\r\n",__func__);
	Directory = mymalloc(SRAMIN,16);
	if(!Directory)
	{
		printf("%s malloc dir err!\r\n",__func__);
		return 1;
	}
	mymemset(Directory,0,16);
	sprintf((char *)Directory,"%s/%s",SAVE_DISK,SAVE_FLODER);
	
	res=f_mkdir((TCHAR *)Directory);		//创建PHOTO文件夹
	if(res!=FR_EXIST&&res!=FR_OK) 	//发生了错误
	{	
		printf("save err in open dir\r\n");	
        myfree(SRAMIN, Directory);
		return 2;		
	}	
	
	
    path = mymalloc(SRAMIN, 32);
    if(!path)
    {
        printf("%s malloc path err!\r\n",__func__);
        myfree(SRAMIN, Directory);
        myfree(SRAMIN, path);
		return 3;
    }
	fp = mymalloc(SRAMIN, sizeof(FIL));
    if(!fp)
    {
        printf("%s malloc FIL err!\r\n",__func__);
        myfree(SRAMIN, Directory);
        myfree(SRAMIN, path);
        myfree(SRAMIN, fp);
		return 3;
    }
	camera_new_pathname(path);

	printf("jpeg data size:%d\r\n",jpeg_data_len*4);//串口打印JPEG文件大小
	pbuf=(u8*)jpeg_data_buf;
	jpglen=0;	//设置jpg文件大小为0
	headok=0;	//清除jpg头标记
	for(i=0;i<jpeg_data_len*4;i++)//查找0XFF,0XD8和0XFF,0XD9,获取jpg文件大小
	{
		if((pbuf[i]==0XFF)&&(pbuf[i+1]==0XD8))//找到FF D8
		{
			jpgstart=i;
			headok=1;	//标记找到jpg头(FF D8)
		}
		if((pbuf[i]==0XFF)&&(pbuf[i+1]==0XD9)&&headok)//找到头以后,再找FF D9
		{
			jpglen=i-jpgstart+2;
			break;
		}
	}

	if(jpglen)			//正常的jpeg数据 
	{
		pbuf+=jpgstart;	//偏移到0XFF,0XD8处
		data = mymalloc(SRAMEX, jpglen);//防止出现字节对齐问题
        if(!data)
        {
            printf("%s malloc data err!\r\n",__func__);
            myfree(SRAMIN, Directory);
            myfree(SRAMIN, path);
            myfree(SRAMIN, fp);
            myfree(SRAMEX, data);
    		return 3;
        }
        mymemcpy(data, pbuf, jpglen);
		res=f_open(fp, (TCHAR *) path, FA_WRITE|FA_CREATE_NEW);
        if(res)
        {
            printf("%s open %s failed!\r\n",__func__,path);
            myfree(SRAMIN, Directory);
            myfree(SRAMIN, path);
            myfree(SRAMIN, fp);
            myfree(SRAMEX, data);
    		return 4;
        }
        res=f_write(fp, data,jpglen,&br);
        if(res)
        {
            printf("%s open %s failed!\r\n",__func__,path);
            myfree(SRAMIN, Directory);
            myfree(SRAMIN, path);
            myfree(SRAMIN, fp);
            myfree(SRAMEX, data);
    		return 4;
        }
        f_close(fp);
		printf("save date ok!\r\n");
		//连上后台，开始上传
		Filename = path+strlen((char *) path);
		Filename -= 12;
		printf("Filename = %s\r\n",Filename);
		if(BackGroundCtrl.ConnectState==2)
		{
			http_post((char *)"/HFS/upload",(char *)Filename,(char *)data,jpglen);
		}
	}
	jpeg_data_len=0;
    
    myfree(SRAMIN, Directory);
    myfree(SRAMIN, path);
    myfree(SRAMIN, fp);
    myfree(SRAMEX, data);
	return res;
}
//OV5640拍照jpg图片
//返回值:0,成功
//    其他,错误代码
u8 ov5640_jpg_photo(void)
{
	
	u8 res=0,fac;
	u16 outputheight=0;
	printf("%s\r\n",__func__);
	ovx_mode=1;
	jpeg_data_ok=0;
	jpeg_data_len=0;	
	OV5640_JPEG_Mode();						//JPEG模式  
	OV5640_OutSize_Set(16,4,800,600);		//设置输出尺寸(SVGA(800*600))  
	dcmi_rx_callback=jpeg_dcmi_rx_callback;	//JPEG接收数据回调函数
	DCMI_DMA_Init((u32)dcmi_line_buf[0],(u32)dcmi_line_buf[1],jpeg_line_size,DMA_MDATAALIGN_WORD,DMA_MINC_ENABLE);//DCMI DMA配置    
	DCMI_Start(); 			//启动传输  
	while(jpeg_data_ok!=1);	//等待第一帧图片采集完
	jpeg_data_ok=2;			//忽略本帧图片,启动下一帧采集 
	while(jpeg_data_ok!=1);	//等待第二帧图片采集完,第二帧,才保存到SD卡去. 
	DCMI_Stop(); 			//停止DMA搬运
	//DCMI_Init();			//DCMI配置
	ovx_mode=0; 
	if(OV5640_Save_photo())
	{
		printf("Save photo err!\r\n");
	}

	OV5640_RGB565_Mode();	//RGB565模式  
	rgb565_test();
	return res;
}  



void camera_app_init(void)
{
	outputheight=750;						 
	dcmi_line_buf[0]=mymalloc(SRAMIN,jpeg_line_size*4);	//为jpeg dma接收申请内存	*4是因为int占4字节
	dcmi_line_buf[1]=mymalloc(SRAMIN,jpeg_line_size*4);	//为jpeg dma接收申请内存	
	jpeg_data_buf=mymalloc(SRAMEX,jpeg_buf_size);		//为jpeg文件申请内存(最大4MB)
 	rgb565_data_buff = mymalloc(SRAMEX,480*750*sizeof(unsigned short));//因为在buttonbmp.c中位图的大小是480*750的unsigned short数组
	
	bm2_c.pData = (unsigned char * )rgb565_data_buff;
	while(OV5640_Init())//初始化OV5640
	{
		printf("camera err! please check it!\r\n");
		delay_ms(1000);
	}	
	OV5640_RGB565_Mode();	//RGB565模式	
	OV5640_Focus_Init();
	OV5640_Light_Mode(0);	//自动模式
	OV5640_Color_Saturation(3);//色彩饱和度0
	OV5640_Brightness(4);	//亮度0
	OV5640_Contrast(3);		//对比度0
	OV5640_Sharpness(33);	//自动锐度
	OV5640_Focus_Constant();//启动持续对焦
	DCMI_Init();			//DCMI配置
}
void Close_Camera(void)
{
	DCMI_Stop(); 			//停止DMA搬运
}
void Start_Camera(void)
{
	ovx_mode=0;
	rgb565_data_len = 0;
	rgb565_data_ok  = 0;
	DCMI_Start(); 			//启动传输
}

void camera_key_handle(uint8_t key)
{
	if(key)
	{ 	
		switch(key)
		{				    
			case KEY0_PRES:	//对比度设置
			{
				OV5640_Focus_Single();
			}
			break;
			case KEY1_PRES:	//执行一次自动对焦
			{
				if(!connect_usb)
					return;
				rgb565_data_ok=0;//防止接下来刷屏
				DCMI_Stop(); //非KEY1按下,停止显示_调节对比度
				ov5640_jpg_photo();
				Start_Camera();	//重新开始传输
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