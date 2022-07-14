#ifndef _USART_H
#define _USART_H
#include "sys.h"
#include "stdio.h"	
#include "stdarg.h"	 	 	 	 
#include "string.h"	
#include "includes.h"
typedef struct
{

	
    u8 *USART3_RX_BUF; 		//接收缓冲,最大USART3_MAX_RECV_LEN字节
    u8  *USART3_TX_BUF; 	//发送缓冲,最大USART3_MAX_SEND_LEN字节 
    u32 USART3_RX_STA;   	//接收数据状态，接收完数据要记得清零

    u32 USART3_MAX_RECV_LEN;		//最大接收缓存字节数
    u32 USART3_MAX_SEND_LEN;		//最大发送缓存字节数
	u8 RxSRAMIdex;          //接收区的SRAM
    u8 TxSRAMIdex;          //发送区的SRAM
    
	u8 EN_USART1;
}Usart3_Data;

#define RECEIVE_OK_U3       ((u32)1<<31) //接受成功后需要位移多少，从而将最高位置1
#define RECEIVE_OK_MARK_U3  (0x80000000)//串口3接受成功判断掩码
#define RECEIVE_LEN_MARK_U3 (0x7FFFFFFF)//串口3接受数据长度掩码



#define USART_REC_LEN  			200  	//定义最大接收字节数 200
#define EN_USART1_RX 			1		//使能（1）/禁止（0）串口1接收
#define RXBUFFERSIZE   1 //缓存大小

#define USART3_CMD_BUFF_LEN     256
#define USART3_DATA_BUFF_LEN     (3*1024*1024+10)//最大只允许4M的数据传输


	  	
extern u8  USART_RX_BUF[USART_REC_LEN]; //接收缓冲,最大USART_REC_LEN个字节.末字节为换行符 
extern u16 USART_RX_STA;         		//接收状态标记	
extern UART_HandleTypeDef UART1_Handler; //UART句柄
extern UART_HandleTypeDef UART3_Handler; //UART句柄

extern u8 aRxBuffer[RXBUFFERSIZE];//HAL库USART接收Buffer

extern Usart3_Data Usart3Data;
extern OS_MUTEX	Usart3Data_TX_MUTEX;
extern OS_MUTEX	Usart3Data_RX_MUTEX;
//如果想串口中断接收，请不要注释以下宏定义
void uart_init(u32 bound);

void usart3_init(u32 bound);				//串口3初始化 

//void Usart3Data_Buff_Change(u32 RXsize, u32 TXsize, u32 timeout);

void usart3_send(u8 *data,u32 length);

//void TIM7_Int_Init(u16 arr,u16 psc);
void u3_printf(char* fmt, ...);
#endif
