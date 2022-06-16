/***************************************************************************************************
                                   xxxxx公司


文件:   BackGround.h
作者:
说明:   后台任务
***************************************************************************************************/
#ifndef __BACKGROUN_H__                     //防重包含.
#define __BACKGROUN_H__
/**************************************************************************************************/

/***************************************************************************************************
包含:   头文件.
***************************************************************************************************/


#include "FIFO.h"
#include "sys.h"

typedef enum
{
	BYTE_ = 0,//单字节数据
	SHORT_LittleEndian = 1,//小端模式两字节
	SHORT_BigEndian = 2,//大端模式两字节
	INT_LittleEndian = 3,//小端模式四字节
	INT_BigEndian = 4,//大端模式四字节
	
}DataFormat;

typedef struct
{
	u8 ConnectState;//0:断开连接 1:连上WiFi 2：连上服务器
	u8 *Message_TXBuffer;
	u8 *Message_RXBuffer;
	u32 Message_TxLen;
	u32 Message_rxLen;
	
	u32 MaxTxBufferLen;
	u32 MaxRxBufferLen;
	
	u8 RxState;//0表示没有收到数据，1表示收到数据
	
	void (*BackgroundSend)(u8 * data,u32 sendlen);//后台发送函数，根据实际情况指向实际发送函数
}BackGround_Ctrl;

typedef struct 
{
	uint32_t POST_send : 1;//发送post命令
}Background_Flag;

extern BackGround_Ctrl BackGroundCtrl;
extern Background_Flag BackgroundFlag;

extern void BackGroundTx(void );
extern void BackGroundRx(void );
extern void BackGroundInit(void);
extern void BackGround_Check_WiFiState(void);




#endif


