#ifndef _CAMERA_APP_H
#define _CAMERA_APP_H
#include "sys.h"


#define SAVE_DISK     "3:"//���������Ǹ�����
#define SAVE_FLODER   "PHOTO" //���������ĸ��ļ�����
extern u16 *rgb565_data_buff;					//rgb565���������ݴ���
extern u8 rgb565_data_ok;
extern void camera_app_init(void);
extern u8 ov5640_jpg_photo(void);
extern void rgb565_test(void);
extern void Close_Camera(void);
extern void Start_Camera(void);
#endif


