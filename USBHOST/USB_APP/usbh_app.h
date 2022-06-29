#ifndef __USBH_APP_H
#define __USBH_APP_H
#include "stdint.h"

#define RD_ALL_DATA 0//读取所有的数据

/*
读的缓存区一定是由USB模块申请，方便USB模块释放
写的特殊性导致写可以是外面的buff比如串口的接收区等等
*/

#define WR_BUFF_EX   0//写缓存区不需要USB模块申请
#define WR_BUFF_USB  1//写缓存区需要USB模块申请 
typedef struct
{
	uint8_t *path;//读写的路径如 3:/PICT/test.txt
	uint8_t *data;//数据
	uint8_t mode;//读写操作的模式 FA_READ FA_WRITE等，通过这来判断是读还是写
	uint32_t length;//读或者写的长度 当读时length=0；说明读完所有文件 当写时length=0；说明不需要USB模块申请内存
	uint32_t bread;//真正读或者写的长度
	uint8_t result;//操作的结果
	uint8_t WRbufCtrl;//写buff是需要USB模块申请还是说不需要
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


