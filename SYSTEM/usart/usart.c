#include "usart.h"
#include "timer.h"
#include "malloc.h"
#include "delay.h"
#include "BackGround.h"
////////////////////////////////////////////////////////////////////////////////// 	 
//���ʹ��os,����������ͷ�ļ�����.
#if SYSTEM_SUPPORT_OS
#include "includes.h"					//os ʹ��	  
#endif
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32H7������
//����1��ʼ��		   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//�޸�����:2017/6/8
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2009-2019
//All rights reserved
//********************************************************************************
//V1.0�޸�˵�� 
////////////////////////////////////////////////////////////////////////////////// 	  
//�������´���,֧��printf����,������Ҫѡ��use MicroLIB	  
//#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)	
#if 1
//#pragma import(__use_no_semihosting)             
//��׼����Ҫ��֧�ֺ���                 
struct __FILE 
{ 
	int handle; 
}; 

FILE __stdout;       
//����_sys_exit()�Ա���ʹ�ð�����ģʽ    
void _sys_exit(int x) 
{ 
	x = x; 
} 
//�ض���fputc���� 
int fputc(int ch, FILE *f)
{ 	
	while((USART1->ISR&0X40)==0);//ѭ������,ֱ���������   
	USART1->TDR=(u8)ch;      
	return ch;
}
#endif 

#if EN_USART1_RX   //���ʹ���˽���
//����1�жϷ������
//ע��,��ȡUSARTx->SR�ܱ���Ī������Ĵ���   	
u8 USART_RX_BUF[USART_REC_LEN];     //���ջ���,���USART_REC_LEN���ֽ�.

////���ڷ��ͻ����� 	
//__align(8) u8 USART3_TX_BUF[USART3_MAX_SEND_LEN]; 	//���ͻ���,���USART3_MAX_SEND_LEN�ֽ�  	  
////���ڽ��ջ����� 	
//u8 Usart3Data.USART3_RX_BUF[USART3_MAX_RECV_LEN]; 				//���ջ���,���USART3_MAX_RECV_LEN���ֽ�.

//����״̬
//bit15��	������ɱ�־
//bit14��	���յ�0x0d
//bit13~0��	���յ�����Ч�ֽ���Ŀ
u16 USART_RX_STA=0;       //����״̬���	

Usart3_Data Usart3Data;

u8 aRxBuffer[RXBUFFERSIZE];//HAL��ʹ�õĴ��ڽ��ջ���
UART_HandleTypeDef UART1_Handler; //UART���
UART_HandleTypeDef UART3_Handler; //UART���



OS_MUTEX	Usart3Data_TX_MUTEX;		//����һ�������ź���,�����ڶ������б�������������
OS_MUTEX	Usart3Data_RX_MUTEX;		//����һ�������ź���,�����ڶ������б�������������

//��ʼ��IO ����1 
//bound:������
void uart_init(u32 bound)
{	
	//UART ��ʼ������
	UART1_Handler.Instance=USART1;					    //USART1
	UART1_Handler.Init.BaudRate=bound;				    //������
	UART1_Handler.Init.WordLength=UART_WORDLENGTH_8B;   //�ֳ�Ϊ8λ���ݸ�ʽ
	UART1_Handler.Init.StopBits=UART_STOPBITS_1;	    //һ��ֹͣλ
	UART1_Handler.Init.Parity=UART_PARITY_NONE;		    //����żУ��λ
	UART1_Handler.Init.HwFlowCtl=UART_HWCONTROL_NONE;   //��Ӳ������
	UART1_Handler.Init.Mode=UART_MODE_TX_RX;		    //�շ�ģʽ
	HAL_UART_Init(&UART1_Handler);					    //HAL_UART_Init()��ʹ��UART1
	
	HAL_UART_Receive_IT(&UART1_Handler, (u8 *)aRxBuffer, RXBUFFERSIZE);//�ú����Ὺ�������жϣ���־λUART_IT_RXNE���������ý��ջ����Լ����ջ���������������
  
}

//��ʼ��IO ����3 
//bound:������
void usart3_init(u32 bound)
{	
	//UART ��ʼ������
	UART3_Handler.Instance=USART3;					    //USART3
	UART3_Handler.Init.BaudRate=bound;				    //������
	UART3_Handler.Init.WordLength=UART_WORDLENGTH_8B;   //�ֳ�Ϊ8λ���ݸ�ʽ
	UART3_Handler.Init.StopBits=UART_STOPBITS_1;	    //һ��ֹͣλ
	UART3_Handler.Init.Parity=UART_PARITY_NONE;		    //����żУ��λ
	UART3_Handler.Init.HwFlowCtl=UART_HWCONTROL_NONE;   //��Ӳ������
	UART3_Handler.Init.Mode=UART_MODE_TX_RX;		    //�շ�ģʽ
	HAL_UART_Init(&UART3_Handler);					    //HAL_UART_Init()��ʹ��UART1
}
/**
****************************************************************************************
@brief:   Usart3Data_Init��Ĭ��ֵ��ʼ��
@Input��  Null
@Output�� NULL
@Return   NULL
@Warning: NULL
@note:   
****************************************************************************************
 **/
void Usart3Data_Init(void)
{
	OS_ERR err;
    Usart3Data.USART3_RX_STA = 0;
	Usart3Data.USART3_MAX_RECV_LEN = USART3_DATA_BUFF_LEN;
	Usart3Data.USART3_MAX_SEND_LEN = USART3_DATA_BUFF_LEN;

    Usart3Data.RxSRAMIdex = SRAMEX;
    Usart3Data.TxSRAMIdex = SRAMEX;
	
	Usart3Data.USART3_RX_BUF = \
        mymalloc(Usart3Data.RxSRAMIdex,Usart3Data.USART3_MAX_RECV_LEN);
	Usart3Data.USART3_TX_BUF = \
        mymalloc(Usart3Data.TxSRAMIdex,Usart3Data.USART3_MAX_SEND_LEN);
    
	Usart3Data.EN_USART1 = 1;
	
	OSMutexCreate((OS_MUTEX*	)&Usart3Data_TX_MUTEX,
				  (CPU_CHAR*	)"Usart3Data_TX_MUTEX",
                  (OS_ERR*		)&err);
	
	OSMutexCreate((OS_MUTEX*	)&Usart3Data_RX_MUTEX,
				  (CPU_CHAR*	)"Usart3Data_RX_MUTEX",
                  (OS_ERR*		)&err);
}

/**
****************************************************************************************
@brief:   Usart3Data_Buff_Change �л����ڽ���buff
@Input��  RXSize �л���Ľ�����buff��С
          TxSize �л���ķ�����buff��С
		  timeout �ȴ�����״̬�ĳ�ʱʱ��
@Output�� NULL
@Return   NULL
@Warning: NULL
@note:   
****************************************************************************************
 **/
/*
void Usart3Data_Buff_Change(u32 RXsize, u32 TXsize, u32 timeout)
{
	OS_ERR err;

    //�ȴ������������л�
	while(Usart3Data.USART3_RX_STA)
	{      
       delay_ms(10); 
       if(!timeout)
       {
            printf("%s timeout\r\n",__func__);
            return;
       }
       else
       {
            timeout--;
       }
    }
	OSMutexPend (&Usart3Data_TX_MUTEX,0,OS_OPT_PEND_BLOCKING,0,&err);//���󻥳��ź���
	myfree(Usart3Data.RxSRAMIdex, Usart3Data.USART3_RX_BUF);
    myfree(Usart3Data.TxSRAMIdex, Usart3Data.USART3_TX_BUF);
    
    Usart3Data.USART3_RX_STA = 0;
	Usart3Data.USART3_MAX_RECV_LEN = RXsize;
	Usart3Data.USART3_MAX_SEND_LEN = TXsize;

    if(RXsize>256)
    {
        Usart3Data.RxSRAMIdex = SRAMEX;
    }
    else
    {
        Usart3Data.RxSRAMIdex = SRAMIN;
    }
    if(TXsize>256)
    {
        Usart3Data.TxSRAMIdex = SRAMEX;
    }
    else
    {
        Usart3Data.TxSRAMIdex = SRAMIN;
    }
    
	Usart3Data.USART3_RX_BUF = \
        mymalloc(Usart3Data.RxSRAMIdex,Usart3Data.USART3_MAX_RECV_LEN);
	Usart3Data.USART3_TX_BUF = \
        mymalloc(Usart3Data.TxSRAMIdex,Usart3Data.USART3_MAX_SEND_LEN);
	
	 BackGroundCtrl.MaxTxBufferLen = Usart3Data.USART3_MAX_SEND_LEN;
	 BackGroundCtrl.MaxRxBufferLen = Usart3Data.USART3_MAX_RECV_LEN;
	 
	 BackGroundCtrl.Message_TXBuffer = Usart3Data.USART3_TX_BUF;
	 BackGroundCtrl.Message_RXBuffer = Usart3Data.USART3_RX_BUF;
	 BackGroundCtrl.Message_TxLen = 0;
    
	OSMutexPost(&Usart3Data_TX_MUTEX,OS_OPT_POST_NONE,&err);
	
	
	
	
}
*/
//UART�ײ��ʼ����ʱ��ʹ�ܣ��������ã��ж�����
//�˺����ᱻHAL_UART_Init()����
//huart:���ھ��

void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{
    //GPIO�˿�����
	GPIO_InitTypeDef GPIO_Initure;
	
	if(huart->Instance==USART1)//����Ǵ���1�����д���1 MSP��ʼ��
	{
		__HAL_RCC_GPIOA_CLK_ENABLE();			//ʹ��GPIOAʱ��
		__HAL_RCC_USART1_CLK_ENABLE();			//ʹ��USART1ʱ��
	
		GPIO_Initure.Pin=GPIO_PIN_9;			//PA9
		GPIO_Initure.Mode=GPIO_MODE_AF_PP;		//�����������
		GPIO_Initure.Pull=GPIO_PULLUP;			//����
		GPIO_Initure.Speed=GPIO_SPEED_FREQ_HIGH;//����
		GPIO_Initure.Alternate=GPIO_AF7_USART1;	//����ΪUSART1
		HAL_GPIO_Init(GPIOA,&GPIO_Initure);	   	//��ʼ��PA9

		GPIO_Initure.Pin=GPIO_PIN_10;			//PA10
		HAL_GPIO_Init(GPIOA,&GPIO_Initure);	   	//��ʼ��PA10
		
#if EN_USART1_RX
		HAL_NVIC_EnableIRQ(USART1_IRQn);				//ʹ��USART1�ж�ͨ��
		HAL_NVIC_SetPriority(USART1_IRQn,3,3);			//��ռ���ȼ�3�������ȼ�3
#endif	
	}
		if(huart==(&UART3_Handler))
	{
		  //GPIO�˿�����
		GPIO_InitTypeDef GPIO_Initure;
	
		__HAL_RCC_GPIOB_CLK_ENABLE();			//ʹ��GPIOBʱ��
		__HAL_RCC_USART3_CLK_ENABLE();			//ʹ��USART3ʱ��
	
		GPIO_Initure.Pin=GPIO_PIN_10;			//PB10
		GPIO_Initure.Mode=GPIO_MODE_AF_PP;		//�����������
		GPIO_Initure.Pull=GPIO_PULLUP;			//����
		GPIO_Initure.Speed=GPIO_SPEED_FREQ_HIGH;		//����
		GPIO_Initure.Alternate=GPIO_AF7_USART3;	//����ΪUSART3
		HAL_GPIO_Init(GPIOB,&GPIO_Initure);	   	//��ʼ��PB10

		GPIO_Initure.Pin=GPIO_PIN_11;			//PB11
		HAL_GPIO_Init(GPIOB,&GPIO_Initure);	   	//��ʼ��PB11
	
//		__HAL_UART_DISABLE_IT(huart,UART_IT_TC);
		__HAL_UART_ENABLE_IT(huart,UART_IT_RXNE);		//���������ж�
		HAL_NVIC_EnableIRQ(USART3_IRQn);				//ʹ��USART3�ж�
		HAL_NVIC_SetPriority(USART3_IRQn,2,3);			//��ռ���ȼ�2�������ȼ�3	
		TIM7_Int_Init(1000-1,9000-1);		//100ms�ж�
		TIM7->CR1&=~(1<<0);        //�رն�ʱ��7
		
		Usart3Data_Init();
	}

}
void usart3_send(u8 *data,u32 length)
{
	uint32_t i;
	if(length>Usart3Data.USART3_MAX_SEND_LEN)
		return;
	for(i=0;i<length;i++)							//ѭ����������
	{
		while((USART3->ISR&0X40)==0);			//ѭ������,ֱ���������   
		USART3->TDR=data[i];  
	} 
}
/*����3,printf ����,��̵�ATָ�������u3_printf,�����ݾͲ����ã���Ϊu3_printf���������Ƿ���heap������
print�ǵ���AT_CMD�������ݾ���ֱ�ӷ���USART3_TX_BUF��
ȷ��һ�η������ݲ�����USART3_MAX_SEND_LEN�ֽ�
*/
u8 AT_CMD[USART3_CMD_BUFF_LEN];
void u3_printf(char* fmt,...)  
{  
	u16 len;
	uint32_t i;
	va_list ap; 
	va_start(ap,fmt);
	vsprintf((char*)AT_CMD,fmt,ap);
	va_end(ap);
	len=strlen((const char*)AT_CMD);		//�˴η������ݵĳ���
	
	if(len>USART3_CMD_BUFF_LEN)
		return;
	for(i=0;i<len;i++)							//ѭ����������
	{
		while((USART3->ISR&0X40)==0);			//ѭ������,ֱ���������   
		USART3->TDR=AT_CMD[i];  
	} 
}


void USART3_IRQHandler(void)
{
	u8 res;	      
	if(__HAL_UART_GET_FLAG(&UART3_Handler,UART_FLAG_RXNE)!=RESET)//���յ�����
	{	 
//		HAL_UART_Receive(&UART3_Handler,&res,1,1000);
		res=USART3->RDR; 			 
		if((Usart3Data.USART3_RX_STA&RECEIVE_OK_U3)==0)//�������һ������,��û�б�����,���ٽ�����������
		{ 
			if(Usart3Data.USART3_RX_STA<Usart3Data.USART3_MAX_RECV_LEN)	//�����Խ�������
			{
//				__HAL_TIM_SetCounter(&TIM7_Handler,0);	
				TIM7->CNT=0;         				//���������	
				if(Usart3Data.USART3_RX_STA==0) 				//ʹ�ܶ�ʱ��7���ж� 
				{
//					__HAL_RCC_TIM7_CLK_ENABLE();            //ʹ��TIM7ʱ��
					TIM7->CR1|=1<<0;     			//ʹ�ܶ�ʱ��7
				}
				Usart3Data.USART3_RX_BUF[Usart3Data.USART3_RX_STA++]=res;	//��¼���յ���ֵ	 
			}else 
			{
				Usart3Data.USART3_RX_STA|=RECEIVE_OK_U3;				//ǿ�Ʊ�ǽ������
			} 
		}
	}  				 											 
}


void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart->Instance==USART1)//����Ǵ���1
	{
		if((USART_RX_STA&0x8000)==0)//����δ���
		{
			if(USART_RX_STA&0x4000)//���յ���0x0d
			{
				if(aRxBuffer[0]!=0x0a)USART_RX_STA=0;//���մ���,���¿�ʼ
				else USART_RX_STA|=0x8000;	//��������� 
			}
			else //��û�յ�0X0D
			{	
				if(aRxBuffer[0]==0x0d)USART_RX_STA|=0x4000;
				else
				{
					USART_RX_BUF[USART_RX_STA&0X3FFF]=aRxBuffer[0] ;
					USART_RX_STA++;
					if(USART_RX_STA>(USART_REC_LEN-1))USART_RX_STA=0;//�������ݴ���,���¿�ʼ����	  
				}		 
			}
		}

	}
}
 
//����1�жϷ������
void USART1_IRQHandler(void)                	
{ 
	u32 timeout=0;
    u32 maxDelay=0x1FFFF;
#if SYSTEM_SUPPORT_OS	 	//ʹ��OS
	OSIntEnter();    
#endif
	
	HAL_UART_IRQHandler(&UART1_Handler);	//����HAL���жϴ����ú���
	
	timeout=0;
    while (HAL_UART_GetState(&UART1_Handler)!=HAL_UART_STATE_READY)//�ȴ�����
	{
        timeout++;////��ʱ����
        if(timeout>maxDelay) break;		
	}
     
	timeout=0;
	while(HAL_UART_Receive_IT(&UART1_Handler,(u8 *)aRxBuffer, RXBUFFERSIZE)!=HAL_OK)//һ�δ������֮�����¿����жϲ�����RxXferCountΪ1
	{
        timeout++; //��ʱ����
        if(timeout>maxDelay) break;	
	}
#if SYSTEM_SUPPORT_OS	 	//ʹ��OS
	OSIntExit();  											 
#endif
} 
#endif	



 

 




