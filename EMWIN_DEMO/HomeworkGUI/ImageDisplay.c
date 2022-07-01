
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
File_Scan pbuf;//Ŀ¼ɨ�������


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
	Dir_Scan_Free(&pbuf);
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

//	//�ƶ�ָ�뵽Ӧ�ö�ȡ��λ��
//	if(Off == 1) readaddress = 0;
//	else readaddress=Off;
//	
//	#if SYSTEM_SUPPORT_OS
//		OS_CRITICAL_ENTER();	//�����ٽ���
//	#endif
//		
//	f_lseek(phFile,readaddress); 
//	
//	//��ȡ���ݵ���������
//	f_read(phFile,jpegBuffer,NumBytesReq,&NumBytesRead);
//	
//	#if SYSTEM_SUPPORT_OS
//		OS_CRITICAL_EXIT();//�˳��ٽ���
//	#endif
//	
//	*ppData = (U8 *)jpegBuffer;
//	return NumBytesRead;//���ض�ȡ�����ֽ���
//}

////��ָ��λ����ʾ���ص�RAM�е�JPEGͼƬ
////JPEGFileName:ͼƬ��SD�����������洢�豸�е�·��(���ļ�ϵͳ֧�֣�)
////mode:��ʾģʽ
////		0 ��ָ��λ����ʾ���в���x,yȷ����ʾλ��
////		1 ��LCD�м���ʾͼƬ����ѡ���ģʽ��ʱ�����x,y��Ч��
////x:ͼƬ���Ͻ���LCD�е�x��λ��(������modeΪ1ʱ���˲�����Ч)
////y:ͼƬ���Ͻ���LCD�е�y��λ��(������modeΪ1ʱ���˲�����Ч)
////member:  ���ű����ķ�����
////denom:���ű����ķ�ĸ��
////����ֵ:0 ��ʾ����,���� ʧ��
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

//	result = f_open(&JPEGFile,(const TCHAR*)JPEGFileName,FA_READ);	//���ļ�
//	//�ļ��򿪴�������ļ�����JPEGMEMORYSIZE
//	if((result != FR_OK) || (JPEGFile.obj.objsize>JPEGMEMORYSIZE)) 	
//		return 1;
//	
//	jpegbuffer=mymalloc(SRAMEX,JPEGFile.obj.objsize);	//�����ڴ�
//	if(jpegbuffer == NULL) 
//		return 2;
//	
//	#if SYSTEM_SUPPORT_OS
//		OS_CRITICAL_ENTER();		//�ٽ���
//	#endif
//		
//	result = f_read(&JPEGFile,jpegbuffer,JPEGFile.obj.objsize,(UINT *)&bread); //��ȡ����
//	if(result != FR_OK) 
//		return 3;
//	
//	#if SYSTEM_SUPPORT_OS
//		OS_CRITICAL_EXIT();	//�˳��ٽ���
//	#endif
//	
//	GUI_JPEG_GetInfo(jpegbuffer,JPEGFile.obj.objsize,&JpegInfo); //��ȡJEGPͼƬ��Ϣ
//	XSize = JpegInfo.XSize;	//��ȡJPEGͼƬ��X���С
//	YSize = JpegInfo.YSize;	//��ȡJPEGͼƬ��Y���С
//	switch(mode)
//	{
//		case 0:	//��ָ��λ����ʾͼƬ
//			if((member == 1) && (denom == 1)) //�������ţ�ֱ�ӻ���
//			{
//				GUI_JPEG_Draw(jpegbuffer,JPEGFile.obj.objsize,x,y);	//��ָ��λ����ʾJPEGͼƬ
//			}else //����ͼƬ��Ҫ����
//			{
//				GUI_JPEG_DrawScaled(jpegbuffer,JPEGFile.obj.objsize,x,y,member,denom);
//			}
//			break;
//		case 1:	//��LCD�м���ʾͼƬ
//			if((member == 1) && (denom == 1)) //�������ţ�ֱ�ӻ���
//			{
//				//��LCD�м���ʾͼƬ
//				GUI_JPEG_Draw(jpegbuffer,JPEGFile.obj.objsize,(lcddev.width-XSize)/2-1,(lcddev.height-YSize)/2-1);
//			}else //����ͼƬ��Ҫ����
//			{
//				Xflag = (float)XSize*((float)member/(float)denom);
//				Yflag = (float)YSize*((float)member/(float)denom);
//				XSize = (lcddev.width-(int)Xflag)/2-1;
//				YSize = (lcddev.height-(int)Yflag)/2-1;
//				GUI_JPEG_DrawScaled(jpegbuffer,JPEGFile.obj.objsize,XSize,YSize,member,denom);
//			}
//			break;
//	}
//	f_close(&JPEGFile);			//�ر�JPEGFile�ļ�
//	myfree(SRAMEX,jpegbuffer);	//�ͷ��ڴ�
//	return 0;
//}

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

    if(connect_usb == 1)//ȷ��U������
    {

    }
}
