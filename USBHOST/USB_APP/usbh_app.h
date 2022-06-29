#ifndef __USBH_APP_H
#define __USBH_APP_H
#include "stdint.h"

#define RD_ALL_DATA 0//��ȡ���е�����

/*
���Ļ�����һ������USBģ�����룬����USBģ���ͷ�
д�������Ե���д�����������buff���紮�ڵĽ������ȵ�
*/

#define WR_BUFF_EX   0//д����������ҪUSBģ������
#define WR_BUFF_USB  1//д��������ҪUSBģ������ 
typedef struct
{
	uint8_t *path;//��д��·���� 3:/PICT/test.txt
	uint8_t *data;//����
	uint8_t mode;//��д������ģʽ FA_READ FA_WRITE�ȣ�ͨ�������ж��Ƕ�����д
	uint32_t length;//������д�ĳ��� ����ʱlength=0��˵�����������ļ� ��дʱlength=0��˵������ҪUSBģ�������ڴ�
	uint32_t bread;//����������д�ĳ���
	uint8_t result;//�����Ľ��
	uint8_t WRbufCtrl;//дbuff����ҪUSBģ�����뻹��˵����Ҫ
}USBH_WR_MSG,*pUSBH_WR_MSG;

extern uint8_t connect_usb;
extern void usb_app_main(void);
extern void usb_app_wr(void);

pUSBH_WR_MSG USBH_Malloc_CtrlStruct(void);
extern void USBH_Malloc_Path(pUSBH_WR_MSG pUSBwrMsg,uint16_t size);
extern void USBH_Malloc_WriteBuf(pUSBH_WR_MSG pUSBwrMsg,uint32_t size,uint8_t *ExBuff);
extern void USBH_WR_MsgFree(pUSBH_WR_MSG pUSBwrMsg);
extern void USBH_ApplyFor_WR(pUSBH_WR_MSG pUSBwrMsg,uint32_t DataLength,uint8_t mode);
#endif


