#ifndef __USBH_APP_H
#define __USBH_APP_H
#include "stdint.h"

#define RD_ALL_DATA 0
typedef struct
{
	uint8_t *path;//读写的路径如 3:/PICT/test.txt
	uint8_t *data;//数据
	uint8_t mode;//读写操作的模式 FA_READ FA_WRITE等，通过这来判断是读还是写
	uint32_t length;//读或者写的长度 当读时length=0；说明读完所有文件 当写时length=0；说明参数有误，不进行写操作
	uint32_t bread;//真正读或者写的长度
	uint8_t result;//操作的结果
}USBH_WR_MSG,*pUSBH_WR_MSG;

extern void usb_app_main(void);
extern void usb_app_wr(void);

extern void *USBH_Malloc_Path(uint16_t size);
extern void *USBH_Malloc_WriteBuf(uint32_t size);
extern void USBH_WR_MsgFree(pUSBH_WR_MSG pUSBwrMsg);
extern void *USBH_ApplyFor_WR(uint8_t *path,uint8_t *data,uint32_t DataLength,uint8_t mode);
#endif


