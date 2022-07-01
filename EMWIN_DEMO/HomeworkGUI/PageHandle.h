#ifndef __PAGEHANDLE_H
#define __PAGEHANDLE_H
#include "sys.h"
#include "WM.h"
//typedef enum {
//	PAGE_VIEW = 0,				/* �鿴��ҵ */
//	PAGE_CAMERA.			    /* �����ϴ� */
//	PAGE_SYS				/* ϵͳ���� */
//	
//}PAGE_ENUM;
typedef enum {
	PAGE_VIEW = 0,			/* (0)  �鿴��ҵ */
	PAGE_CAMERA,			/* (1) �����ϴ� */
	PAGE_SYS	/* (2) ϵͳ���� */
} PAGE_ENUM;

typedef struct
{
	uint8_t page;//�ڼ�ҳ
	WM_HWIN hItem;
	
	
}PAGEHANDLE_PARA;
extern uint8_t CurrentPage;//��ǰҳ��
extern uint8_t PreviousPage;//֮ǰҳ��
extern void PageHnadle_main(uint8_t PageIndex);

#endif
