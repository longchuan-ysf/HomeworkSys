#ifndef __USBH_APP_H
#define __USBH_APP_H
#include "stdint.h"

#define RD_ALL_DATA 0
typedef struct
{
	uint8_t *path;//��д��·���� 3:/PICT/test.txt
	uint8_t *data;//����
	uint8_t mode;//��д������ģʽ FA_READ FA_WRITE�ȣ�ͨ�������ж��Ƕ�����д
	uint32_t length;//������д�ĳ��� ����ʱlength=0��˵�����������ļ� ��дʱlength=0��˵���������󣬲�����д����
	uint32_t bread;//����������д�ĳ���
	uint8_t result;//�����Ľ��
}USBH_WR_MSG,*pUSBH_WR_MSG;

extern void usb_app_main(void);
extern void usb_app_wr(void);

extern void *USBH_Malloc_Path(uint16_t size);
extern void *USBH_Malloc_WriteBuf(uint32_t size);
extern void USBH_WR_MsgFree(pUSBH_WR_MSG pUSBwrMsg);
extern void *USBH_ApplyFor_WR(uint8_t *path,uint8_t *data,uint32_t DataLength,uint8_t mode);
#endif


