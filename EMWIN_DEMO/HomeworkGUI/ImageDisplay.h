#ifndef __IMAGEDISPLAY_H
#define __IMAGEDISPLAY_H
#include "sys.h"
#include "DIALOG.h"
//����Ч�ʿ���ʹ�ú궨��ָ��Ŀ¼�¿�ɨ�������ļ�����
#define DIR_MAX_NUM				255
typedef struct 
{
	char** file_name;//�ļ���ָ������
	uint32_t *file_size;//�ļ���С����
	uint32_t file_num;//��Ч���ļ�������
	uint8_t ScanResult;//ɨ����
}File_Scan,*pFile_Scan;

extern uint8_t PictureIndex;
extern uint8_t PaintPic;

extern File_Scan DownloadPicture;//Ŀ¼ɨ�������

extern void Display_Image_byIndex(IMAGE_Handle hObj,uint8_t index);
extern void Image_Display_Key(uint8_t key);
extern void Display_Image_byName(IMAGE_Handle hObj,uint8_t *FileName);
#endif

