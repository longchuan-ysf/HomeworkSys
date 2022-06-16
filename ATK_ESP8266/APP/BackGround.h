/***************************************************************************************************
                                   xxxxx��˾


�ļ�:   BackGround.h
����:
˵��:   ��̨����
***************************************************************************************************/
#ifndef __BACKGROUN_H__                     //���ذ���.
#define __BACKGROUN_H__
/**************************************************************************************************/

/***************************************************************************************************
����:   ͷ�ļ�.
***************************************************************************************************/


#include "FIFO.h"
#include "sys.h"

typedef enum
{
	BYTE_ = 0,//���ֽ�����
	SHORT_LittleEndian = 1,//С��ģʽ���ֽ�
	SHORT_BigEndian = 2,//���ģʽ���ֽ�
	INT_LittleEndian = 3,//С��ģʽ���ֽ�
	INT_BigEndian = 4,//���ģʽ���ֽ�
	
}DataFormat;

typedef struct
{
	u8 ConnectState;//0:�Ͽ����� 1:����WiFi 2�����Ϸ�����
	u8 *Message_TXBuffer;
	u8 *Message_RXBuffer;
	u32 Message_TxLen;
	u32 Message_rxLen;
	
	u32 MaxTxBufferLen;
	u32 MaxRxBufferLen;
	
	u8 RxState;//0��ʾû���յ����ݣ�1��ʾ�յ�����
	
	void (*BackgroundSend)(u8 * data,u32 sendlen);//��̨���ͺ���������ʵ�����ָ��ʵ�ʷ��ͺ���
}BackGround_Ctrl;

typedef struct 
{
	uint32_t POST_send : 1;//����post����
}Background_Flag;

extern BackGround_Ctrl BackGroundCtrl;
extern Background_Flag BackgroundFlag;

extern void BackGroundTx(void );
extern void BackGroundRx(void );
extern void BackGroundInit(void);
extern void BackGround_Check_WiFiState(void);




#endif


