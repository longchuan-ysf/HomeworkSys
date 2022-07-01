
/*********************************************************************
��ʾͼƬ�Ĵ�����
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
File_Scan DownloadPicture;//Ŀ¼ɨ�������

uint8_t PictureIndex;
//�ͷ��ڴ棬�ڶ�ȡ����Ϣ������ͷ�
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
ɨ��ָ��·���µ�ĳ�������ļ�,ɨ��һ�����͵��ļ�����Ҫ����һ��File_Scan
���͵Ľṹ��,�ڶ�����������Ϣ�󣬵���Dir_Scan_Free�ͷ��ڴ�
dir_path:�ļ���·��������:"0:/APP/ICON"
use_file:ɨ��ĺ�׺��,����:"txt",������Ϊ"*"����ɨ��Ŀ¼�µ����д��ں�׺�����ļ�;
scanfile:�ѷ����ڴ�ľ��
*/

FRESULT Dir_Scan(const char* dir_path,const char* use_file,File_Scan* scanfile)
{
	FRESULT res = FR_OK;
	FILINFO* FileInfo;	//�ļ���Ϣ
	DIR* dir;  			//Ŀ¼
	
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
			res = f_readdir(dir, FileInfo);     //��ȡĿ¼�µ�һ���ļ�
	        if ((res != FR_OK)||(FileInfo->fname[0]==0) ||(scanfile->file_num>DIR_MAX_NUM))
			{
				printf("f_readdir err %d\r\n",res);
				break;  						//������//��ĩβ��//�ﵽ����ȡ��Ŀ,�˳�
			}
			if(FileInfo->fattrib&AM_DIR)continue;//����Ŀ¼
			pstr=strrchr(FileInfo->fname,'.');	//�����������'.'��λ��
			if(pstr==NULL)continue;				//�޺�׺��������
			pstr++;								//ȥ��'.'
			
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
@brief:    test_ff ɨ��ָ��·���µ�ָ����׺���ļ�
@Input��    scan_dir ·���ַ���
			choose_file ָ����׺��
@Output��   NULL
@Return��   NULL
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
 @brief:     load_image �����ļ���RAM��
 @Input��    FileName �ļ��� ��������·��
 @Output��   NULL
 @Return��   �ɹ��򷵻�USB��д���ƽṹ��ָ�� ʧ�ܷ���0
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
        if(timeout>10*1000)//10s�ĳ�ʱ�ж�
        {
		    return NULL;
        }
		delay_ms(10);//��ʱ10ms
	}
    return  pMsgRD;
}
 /**
****************************************************************************************
@brief:    Display_Image_byNameͨ��������ʾͼƬ
@Input��    obj ��ʾ���ĸ�ͼƬ�ؼ���
            FileName �ļ���  ��������·��
@Output��   NULL
@Return��   NULL
@Warning:   NULL   
@note:      �ú�������test_ffɨ��������������ʾ ��Ҫ��Ϊ��ȷ��U������
****************************************************************************************
 **/

void Display_Image_byName(IMAGE_Handle hObj,uint8_t *FileName)
{
    uint8_t *temp;
    pUSBH_WR_MSG USBImageFileCtrl;
 
    if(connect_usb == 1)//ȷ��U������
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

    if(connect_usb == 1)//???U??????
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

	if(DownloadPicture.file_num==1)//??????????????��??????????��?��USB
		return;
	if(key)
	{ 	
		switch(key)
		{				    
			case KEY0_PRES:	//????????
			{
				if(PictureIndex<DownloadPicture.file_num-1)
					PictureIndex++;
				else
					PictureIndex=0;
				
				WM_InvalidateWindow(WM_Picture);//??????
			}
			break;
			case KEY1_PRES:	
			{
				printf("%s KEY1_PRES\r\n",__func__);
			}
			break;
			case KEY2_PRES:
			{
				if(PictureIndex>0)
					PictureIndex--;
				else
					PictureIndex=DownloadPicture.file_num-1;
				
				WM_InvalidateWindow(WM_Picture);//??????
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