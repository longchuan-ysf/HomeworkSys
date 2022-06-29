#ifndef _CAMERA_APP_H
#define _CAMERA_APP_H
#include "sys.h"


#define SAVE_DISK     "3:"//定义存放在那个磁盘
#define SAVE_FLODER   "PHOTO" //定义存放在哪个文件夹下
extern u16 *rgb565_data_buff;					//rgb565数据数据暂存区
extern u8 rgb565_data_ok;
extern void camera_app_init(void);
extern u8 ov5640_jpg_photo(void);
extern void rgb565_test(void);
extern void Close_Camera(void);
extern void Start_Camera(void);
#endif


