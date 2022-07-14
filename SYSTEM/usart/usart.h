#ifndef _USART_H
#define _USART_H
#include "sys.h"
#include "stdio.h"	
#include "stdarg.h"	 	 	 	 
#include "string.h"	
#include "includes.h"
typedef struct
{

	
    u8 *USART3_RX_BUF; 		//���ջ���,���USART3_MAX_RECV_LEN�ֽ�
    u8  *USART3_TX_BUF; 	//���ͻ���,���USART3_MAX_SEND_LEN�ֽ� 
    u32 USART3_RX_STA;   	//��������״̬������������Ҫ�ǵ�����

    u32 USART3_MAX_RECV_LEN;		//�����ջ����ֽ���
    u32 USART3_MAX_SEND_LEN;		//����ͻ����ֽ���
	u8 RxSRAMIdex;          //��������SRAM
    u8 TxSRAMIdex;          //��������SRAM
    
	u8 EN_USART1;
}Usart3_Data;

#define RECEIVE_OK_U3       ((u32)1<<31) //���ܳɹ�����Ҫλ�ƶ��٣��Ӷ������λ��1
#define RECEIVE_OK_MARK_U3  (0x80000000)//����3���ܳɹ��ж�����
#define RECEIVE_LEN_MARK_U3 (0x7FFFFFFF)//����3�������ݳ�������



#define USART_REC_LEN  			200  	//�����������ֽ��� 200
#define EN_USART1_RX 			1		//ʹ�ܣ�1��/��ֹ��0������1����
#define RXBUFFERSIZE   1 //�����С

#define USART3_CMD_BUFF_LEN     256
#define USART3_DATA_BUFF_LEN     (3*1024*1024+10)//���ֻ����4M�����ݴ���


	  	
extern u8  USART_RX_BUF[USART_REC_LEN]; //���ջ���,���USART_REC_LEN���ֽ�.ĩ�ֽ�Ϊ���з� 
extern u16 USART_RX_STA;         		//����״̬���	
extern UART_HandleTypeDef UART1_Handler; //UART���
extern UART_HandleTypeDef UART3_Handler; //UART���

extern u8 aRxBuffer[RXBUFFERSIZE];//HAL��USART����Buffer

extern Usart3_Data Usart3Data;
extern OS_MUTEX	Usart3Data_TX_MUTEX;
extern OS_MUTEX	Usart3Data_RX_MUTEX;
//����봮���жϽ��գ��벻Ҫע�����º궨��
void uart_init(u32 bound);

void usart3_init(u32 bound);				//����3��ʼ�� 

//void Usart3Data_Buff_Change(u32 RXsize, u32 TXsize, u32 timeout);

void usart3_send(u8 *data,u32 length);

//void TIM7_Int_Init(u16 arr,u16 psc);
void u3_printf(char* fmt, ...);
#endif
