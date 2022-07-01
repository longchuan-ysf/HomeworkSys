#ifndef __IMAGEDISPLAY_H
#define __IMAGEDISPLAY_H
#include "sys.h"
#include "DIALOG.h"
//出于效率考虑使用宏定义指定目录下可扫描的最大文件数量
#define DIR_MAX_NUM				255
typedef struct 
{
	char** file_name;//文件名指针数组
	uint32_t *file_size;//文件大小数组
	uint32_t file_num;//有效的文件名个数
	uint8_t ScanResult;//扫描结果
}File_Scan;
#endif

extern void Display_Image_byName(IMAGE_Handle hObj,uint8_t *FileName);
