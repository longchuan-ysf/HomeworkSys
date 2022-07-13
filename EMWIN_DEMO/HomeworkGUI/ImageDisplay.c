
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
#include "key.h"
#include "BackGround.h"
File_Scan DownloadPicture;//目录扫描结果存放

uint8_t PictureIndex;
uint8_t PaintPic=0;
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
	Dir_Scan_Free(&DownloadPicture);
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
	DownloadPicture.ScanResult=Dir_Scan(scan_dir,choose_file,&DownloadPicture);
    if(DownloadPicture.ScanResult == FR_OK)
    {
    	for(uint8_t i=0;i<DownloadPicture.file_num;i++)
    	{
    		//printf("\r\n%s",scan_dir);
    		printf("fiel = %s, size = %d\r\n",DownloadPicture.file_name[i],DownloadPicture.file_size[i]);
    	}
		PictureIndex=0;
    }
}



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
			USBH_WR_MsgFree(USBImageFileCtrl);
           
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
void Display_Image_byIndex(IMAGE_Handle hObj,uint8_t index)
{
	uint8_t *FileName;
	uint8_t *temp;
    pUSBH_WR_MSG USBImageFileCtrl;
    if(connect_usb == 1)//确定U盘在线
    {
		FileName = (uint8_t *)DownloadPicture.file_name[index];
		printf("display index = %d FileName=%s\r\n",index,FileName);
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
			USBH_WR_MsgFree(USBImageFileCtrl);
           
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
void Image_Display_Key(uint8_t key)
{

	
	if(key)
	{ 	
		switch(key)
		{				    
			case KEY0_PRES:	//下一张
			{
                if(DownloadPicture.file_num==1)//只有一张图片不需要切换显示，造成多次读写USB
		            return;
				if(PictureIndex<DownloadPicture.file_num-1)
					PictureIndex++;
				else
					PictureIndex=0;
				PaintPic=1;
				WM_InvalidateWindow(WM_Picture);//绘制图片
			}
			break;
			case KEY1_PRES:	//更新
			{
				printf("%s KEY1_PRES\r\n",__func__);
				if(BackGroundCtrl.ConnectState==2)
				{
                    BackgroundFlag.UpdataPicture=1;
                    
                }
			}
			break;
			case KEY2_PRES://上一张
			{
                if(DownloadPicture.file_num==1)//只有一张图片不需要切换显示，造成多次读写USB
		            return;
				if(PictureIndex>0)
					PictureIndex--;
				else
					PictureIndex=DownloadPicture.file_num-1;
				PaintPic=1;
				WM_InvalidateWindow(WM_Picture);//绘制图片
			}
			break;
			case WKUP_PRES:
			{
				printf("%s WKUP_PRES\r\n",__func__);
			}
			break;
			default:
				break;
			
		}
		
	} 

}