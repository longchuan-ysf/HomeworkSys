#ifndef __PAGEHANDLE_H
#define __PAGEHANDLE_H
#include "sys.h"
#include "WM.h"
//typedef enum {
//	PAGE_VIEW = 0,				/* 查看作业 */
//	PAGE_CAMERA.			    /* 拍照上传 */
//	PAGE_SYS				/* 系统设置 */
//	
//}PAGE_ENUM;
typedef enum {
	PAGE_VIEW = 0,			/* (0)  查看作业 */
	PAGE_CAMERA,			/* (1) 拍照上传 */
	PAGE_SYS	/* (2) 系统设置 */
} PAGE_ENUM;

typedef struct
{
	uint8_t page;//第几页
	WM_HWIN hItem;
	
	
}PAGEHANDLE_PARA;
extern uint8_t CurrentPage;//当前页面
extern uint8_t PreviousPage;//之前页面
extern void PageHnadle_main(uint8_t PageIndex);

#endif
