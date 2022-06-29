
/*********************************************************************
显示图片的处理函数
*
**********************************************************************
*/
#include "ImageDisplay.h"
#include "HomeworkGUI.h"
#include "usbh_app.h"
#include "ff.h"
#include "malloc.h"
#include "string.h"
#include "delay.h"
#include "jpegdisplay.h"
File_Scan pbuf;//目录扫描结果存放


//释放内存，在读取完信息后进行释放
void Dir_Scan_Free(File_Scan* pbuf)
{
	for(uint16_t i=0;i<pbuf->file_num;i++)
	{
		myfree(SRAMIN,pbuf->file_name[i]);
	}
	myfree(SRAMIN,pbuf->file_name);
	myfree(SRAMIN,pbuf->file_size);
	pbuf->file_num=0;
    pbuf->ScanResult = 0;
}

/*
扫描指定路径下的某种类型文件,扫描一种类型的文件就需要定义一个File_Scan
类型的结构体,在读出包含的信息后，调用Dir_Scan_Free释放内存
dir_path:文件夹路径，形如:"0:/APP/ICON"
use_file:扫描的后缀名,形如:"txt",若输入为"*"，则扫描目录下的所有存在后缀名的文件;
scanfile:已分配内存的句柄
*/

FRESULT Dir_Scan(const char* dir_path,const char* use_file,File_Scan* scanfile)
{
	FRESULT res = FR_OK;
	FILINFO* FileInfo;	//文件信息
	DIR* dir;  			//目录
	
	dir=(DIR*)mymalloc(SRAMIN,sizeof(DIR));
	
	res=f_opendir(dir,dir_path);
	
	FileInfo=(FILINFO*)mymalloc(SRAMIN,sizeof(FILINFO));
	if(!FileInfo)
	{
		res = FR_NOT_ENOUGH_CORE; 
		printf("fileinfo malloc err!\r\n");
	}
	
	scanfile->file_name=(char**)mymalloc(SRAMIN,DIR_MAX_NUM*sizeof(char*));
	scanfile->file_size = (uint32_t *)mymalloc(SRAMIN,DIR_MAX_NUM*sizeof(uint32_t));
	
	if(!scanfile->file_name)
	{
		res = FR_NOT_ENOUGH_CORE; 
		printf("scanfile malloc err!\r\n");
	}
	
	scanfile->file_num=0;
	
	if(res==FR_OK)
	{
		while(1)
		{
			char* pstr;
			res = f_readdir(dir, FileInfo);     //读取目录下的一个文件
	        if ((res != FR_OK)||(FileInfo->fname[0]==0) ||(scanfile->file_num>DIR_MAX_NUM))
			{
				printf("f_readdir err %d\r\n",res);
				break;  						//错误了//到末尾了//达到最大读取数目,退出
			}
			if(FileInfo->fattrib&AM_DIR)continue;//跳过目录
			pstr=strrchr(FileInfo->fname,'.');	//从右向左查找'.'的位置
			if(pstr==NULL)continue;				//无后缀名的跳过
			pstr++;								//去掉'.'
			
			if((strcasecmp(pstr,use_file)==0)||(use_file[0]=='*'))
			{

				scanfile->file_name[scanfile->file_num]=
                    (char*)mymalloc(SRAMIN,
                                    strlen(FileInfo->fname)+strlen(dir_path)+1);
				mymemset(scanfile->file_name[scanfile->file_num],
                         0,
                         strlen(FileInfo->fname)+strlen(dir_path)+1);
                sprintf(scanfile->file_name[scanfile->file_num],"%s/%s",
                        dir_path,FileInfo->fname);
				scanfile->file_size[scanfile->file_num] = FileInfo->fsize;
				
				scanfile->file_num++;
			}			
		}
	}
	f_closedir(dir);
	myfree(SRAMIN,dir);
	myfree(SRAMIN,FileInfo);
	return res;
}


void test_ff_free(void)
{
	Dir_Scan_Free(&pbuf);
}
 /**
****************************************************************************************
@brief:    test_ff 扫描指定路劲下的指定后缀名文件
@Input：    scan_dir 路径字符串
			choose_file 指定后缀名
@Output：   NULL
@Return：   NULL
@Warning:   NULL   
@note:      NULL
****************************************************************************************
 **/
void test_ff(char* scan_dir,char* choose_file)
{
	pbuf.ScanResult=Dir_Scan(scan_dir,choose_file,&pbuf);
    if(pbuf.ScanResult == FR_OK)
    {
    	for(uint8_t i=0;i<pbuf.file_num;i++)
    	{
    		//printf("\r\n%s",scan_dir);
    		printf("fiel = %s, size = %d\r\n",pbuf.file_name[i],pbuf.file_size[i]);
    	}
    }
}


/*********************************************************************
*
*       _GetData
*
* Function description
*   This routine is called by GUI_JPEG_DrawEx(). The routine is responsible
*   for setting the data pointer to a valid data location with at least
*   one valid byte.
*
* Parameters:
*   p           - Pointer to application defined data.
*   NumBytesReq - Number of bytes requested.
*   ppData      - Pointer to data pointer. This pointer should be set to
*                 a valid location.
*   StartOfFile - If this flag is 1, the data pointer should be set to the
*                 beginning of the data stream.
*
* Return value:
*   Number of data bytes available.
*/
//static int JpegGetData(void * p, const U8 ** ppData, unsigned NumBytesReq, U32 Off) 
//{
//	static int readaddress=0;
//	FIL * phFile;
//	UINT NumBytesRead;
//	#if SYSTEM_SUPPORT_OS
//		CPU_SR_ALLOC();
//	#endif

//	phFile = (FIL *)p;
//	
//	if (NumBytesReq > sizeof(jpegBuffer)) 
//	{
//		NumBytesReq = sizeof(jpegBuffer);
//	}

//	//移动指针到应该读取的位置
//	if(Off == 1) readaddress = 0;
//	else readaddress=Off;
//	
//	#if SYSTEM_SUPPORT_OS
//		OS_CRITICAL_ENTER();	//进入临界区
//	#endif
//		
//	f_lseek(phFile,readaddress); 
//	
//	//读取数据到缓冲区中
//	f_read(phFile,jpegBuffer,NumBytesReq,&NumBytesRead);
//	
//	#if SYSTEM_SUPPORT_OS
//		OS_CRITICAL_EXIT();//退出临界区
//	#endif
//	
//	*ppData = (U8 *)jpegBuffer;
//	return NumBytesRead;//返回读取到的字节数
//}

////在指定位置显示加载到RAM中的JPEG图片
////JPEGFileName:图片在SD卡或者其他存储设备中的路径(需文件系统支持！)
////mode:显示模式
////		0 在指定位置显示，有参数x,y确定显示位置
////		1 在LCD中间显示图片，当选择此模式的时候参数x,y无效。
////x:图片左上角在LCD中的x轴位置(当参数mode为1时，此参数无效)
////y:图片左上角在LCD中的y轴位置(当参数mode为1时，此参数无效)
////member:  缩放比例的分子项
////denom:缩放比例的分母项
////返回值:0 显示正常,其他 失败
//int displyjpeg(u8 *JPEGFileName,u8 mode,u32 x,u32 y,int member,int denom)
//{
//	u16 bread;
//	char *jpegbuffer;
//	char result;
//	int XSize,YSize;
//	GUI_JPEG_INFO JpegInfo;
//	float Xflag,Yflag;
//	
//	#if SYSTEM_SUPPORT_OS
//		CPU_SR_ALLOC();
//	#endif

//	result = f_open(&JPEGFile,(const TCHAR*)JPEGFileName,FA_READ);	//打开文件
//	//文件打开错误或者文件大于JPEGMEMORYSIZE
//	if((result != FR_OK) || (JPEGFile.obj.objsize>JPEGMEMORYSIZE)) 	
//		return 1;
//	
//	jpegbuffer=mymalloc(SRAMEX,JPEGFile.obj.objsize);	//申请内存
//	if(jpegbuffer == NULL) 
//		return 2;
//	
//	#if SYSTEM_SUPPORT_OS
//		OS_CRITICAL_ENTER();		//临界区
//	#endif
//		
//	result = f_read(&JPEGFile,jpegbuffer,JPEGFile.obj.objsize,(UINT *)&bread); //读取数据
//	if(result != FR_OK) 
//		return 3;
//	
//	#if SYSTEM_SUPPORT_OS
//		OS_CRITICAL_EXIT();	//退出临界区
//	#endif
//	
//	GUI_JPEG_GetInfo(jpegbuffer,JPEGFile.obj.objsize,&JpegInfo); //获取JEGP图片信息
//	XSize = JpegInfo.XSize;	//获取JPEG图片的X轴大小
//	YSize = JpegInfo.YSize;	//获取JPEG图片的Y轴大小
//	switch(mode)
//	{
//		case 0:	//在指定位置显示图片
//			if((member == 1) && (denom == 1)) //无需缩放，直接绘制
//			{
//				GUI_JPEG_Draw(jpegbuffer,JPEGFile.obj.objsize,x,y);	//在指定位置显示JPEG图片
//			}else //否则图片需要缩放
//			{
//				GUI_JPEG_DrawScaled(jpegbuffer,JPEGFile.obj.objsize,x,y,member,denom);
//			}
//			break;
//		case 1:	//在LCD中间显示图片
//			if((member == 1) && (denom == 1)) //无需缩放，直接绘制
//			{
//				//在LCD中间显示图片
//				GUI_JPEG_Draw(jpegbuffer,JPEGFile.obj.objsize,(lcddev.width-XSize)/2-1,(lcddev.height-YSize)/2-1);
//			}else //否则图片需要缩放
//			{
//				Xflag = (float)XSize*((float)member/(float)denom);
//				Yflag = (float)YSize*((float)member/(float)denom);
//				XSize = (lcddev.width-(int)Xflag)/2-1;
//				YSize = (lcddev.height-(int)Yflag)/2-1;
//				GUI_JPEG_DrawScaled(jpegbuffer,JPEGFile.obj.objsize,XSize,YSize,member,denom);
//			}
//			break;
//	}
//	f_close(&JPEGFile);			//关闭JPEGFile文件
//	myfree(SRAMEX,jpegbuffer);	//释放内存
//	return 0;
//}

 /**
 ****************************************************************************************
 @brief:     load_image 加载文件到RAM中
 @Input：    FileName 文件名 包含完整路径
 @Output：   NULL
 @Return：   成功则返回USB读写控制结构体指针 失败返回0
 @Warning:   NULL   
 @note:      
 ****************************************************************************************
  **/

pUSBH_WR_MSG load_image(uint8_t *FileName)
{
    u16 timeout;
    pUSBH_WR_MSG pMsgRD;
	
	pMsgRD = USBH_Malloc_CtrlStruct();
    if(!pMsgRD)
    {
        return 0;
    }
	
	USBH_Malloc_Path(pMsgRD,strlen((char *)FileName));

	sprintf((char *)pMsgRD->path,"%s",FileName);

    USBH_ApplyFor_WR(pMsgRD,RD_ALL_DATA,FA_READ);
    timeout = 0;
	while(1)
	{
        timeout++;
		if(pMsgRD->result==FR_OK)
		{
			break;
		}
        if(timeout>10*1000)//10s的超时判断
        {
		    return NULL;
        }
		delay_ms(10);//延时10ms
	}
    return  pMsgRD;
}
 /**
****************************************************************************************
@brief:    Display_Image_byName通过名称显示图片
@Input：    obj 显示在哪个图片控件上
            FileName 文件名  包含完整路径
@Output：   NULL
@Return：   NULL
@Warning:   NULL   
@note:      该函数必须test_ff扫描完后才能正常显示 主要是为了确定U盘在线
****************************************************************************************
 **/

void Display_Image_byName(IMAGE_Handle hObj,uint8_t *FileName)
{
    uint8_t *temp;
    pUSBH_WR_MSG USBImageFileCtrl;
 
    if(connect_usb == 1)//确定U盘在线
    {
        temp=(uint8_t *)strstr((char *)FileName,".");
        if(!temp)
        {
            printf("file name err!\r\n");
            return;
        }   
        temp++;
        if((strcasecmp((char *)temp,(char *)"jpg") == 0) ||
            (strcasecmp((char *)temp,(char *)"jpeg") == 0)
        )
        {
            USBImageFileCtrl = load_image(FileName);
			IMAGE_SetJPEG(hObj,USBImageFileCtrl->data,USBImageFileCtrl->bread);
           
        }
        else if((strcasecmp((char *)temp,(char *)"bmp") == 0))
        {
            //USBImageFileCtrl = load_image(FileName);
            
        }
        else
        {
            printf("not support file format!\r\n");
        }
    }
}
void Display_Image_byIndex(uint8_t index)
{

    if(connect_usb == 1)//确定U盘在线
    {

    }
}
