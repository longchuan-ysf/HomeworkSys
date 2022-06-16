#include "usart.h"
#include "timer.h"
#include "malloc.h"
#include "delay.h"
#include "BackGround.h"
////////////////////////////////////////////////////////////////////////////////// 	 
//如果使用os,则包括下面的头文件即可.
#if SYSTEM_SUPPORT_OS
#include "includes.h"					//os 使用	  
#endif
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK STM32H7开发板
//串口1初始化		   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//修改日期:2017/6/8
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2009-2019
//All rights reserved
//********************************************************************************
//V1.0修改说明 
////////////////////////////////////////////////////////////////////////////////// 	  
//加入以下代码,支持printf函数,而不需要选择use MicroLIB	  
//#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)	
#if 1
//#pragma import(__use_no_semihosting)             
//标准库需要的支持函数                 
struct __FILE 
{ 
	int handle; 
}; 

FILE __stdout;       
//定义_sys_exit()以避免使用半主机模式    
void _sys_exit(int x) 
{ 
	x = x; 
} 
//重定义fputc函数 
int fputc(int ch, FILE *f)
{ 	
	while((USART1->ISR&0X40)==0);//循环发送,直到发送完毕   
	USART1->TDR=(u8)ch;      
	return ch;
}
#endif 

#if EN_USART1_RX   //如果使能了接收
//串口1中断服务程序
//注意,读取USARTx->SR能避免莫名其妙的错误   	
u8 USART_RX_BUF[USART_REC_LEN];     //接收缓冲,最大USART_REC_LEN个字节.

////串口发送缓存区 	
//__align(8) u8 USART3_TX_BUF[USART3_MAX_SEND_LEN]; 	//发送缓冲,最大USART3_MAX_SEND_LEN字节  	  
////串口接收缓存区 	
//u8 Usart3Data.USART3_RX_BUF[USART3_MAX_RECV_LEN]; 				//接收缓冲,最大USART3_MAX_RECV_LEN个字节.

//接收状态
//bit15，	接收完成标志
//bit14，	接收到0x0d
//bit13~0，	接收到的有效字节数目
u16 USART_RX_STA=0;       //接收状态标记	

Usart3_Data Usart3Data;

u8 aRxBuffer[RXBUFFERSIZE];//HAL库使用的串口接收缓冲
UART_HandleTypeDef UART1_Handler; //UART句柄
UART_HandleTypeDef UART3_Handler; //UART句柄



OS_MUTEX	Usart3Data_TX_MUTEX;		//定义一个互斥信号量,用于在多任务中保护收送区数据
OS_MUTEX	Usart3Data_RX_MUTEX;		//定义一个互斥信号量,用于在多任务中保护接收区数据

//初始化IO 串口1 
//bound:波特率
void uart_init(u32 bound)
{	
	//UART 初始化设置
	UART1_Handler.Instance=USART1;					    //USART1
	UART1_Handler.Init.BaudRate=bound;				    //波特率
	UART1_Handler.Init.WordLength=UART_WORDLENGTH_8B;   //字长为8位数据格式
	UART1_Handler.Init.StopBits=UART_STOPBITS_1;	    //一个停止位
	UART1_Handler.Init.Parity=UART_PARITY_NONE;		    //无奇偶校验位
	UART1_Handler.Init.HwFlowCtl=UART_HWCONTROL_NONE;   //无硬件流控
	UART1_Handler.Init.Mode=UART_MODE_TX_RX;		    //收发模式
	HAL_UART_Init(&UART1_Handler);					    //HAL_UART_Init()会使能UART1
	
	HAL_UART_Receive_IT(&UART1_Handler, (u8 *)aRxBuffer, RXBUFFERSIZE);//该函数会开启接收中断：标志位UART_IT_RXNE，并且设置接收缓冲以及接收缓冲接收最大数据量
  
}

//初始化IO 串口3 
//bound:波特率
void usart3_init(u32 bound)
{	
	//UART 初始化设置
	UART3_Handler.Instance=USART3;					    //USART3
	UART3_Handler.Init.BaudRate=bound;				    //波特率
	UART3_Handler.Init.WordLength=UART_WORDLENGTH_8B;   //字长为8位数据格式
	UART3_Handler.Init.StopBits=UART_STOPBITS_1;	    //一个停止位
	UART3_Handler.Init.Parity=UART_PARITY_NONE;		    //无奇偶校验位
	UART3_Handler.Init.HwFlowCtl=UART_HWCONTROL_NONE;   //无硬件流控
	UART3_Handler.Init.Mode=UART_MODE_TX_RX;		    //收发模式
	HAL_UART_Init(&UART3_Handler);					    //HAL_UART_Init()会使能UART1
}
/**
****************************************************************************************
@brief:   Usart3Data_Init用默认值初始化
@Input：  Null
@Output： NULL
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
@brief:   Usart3Data_Buff_Change 切换串口接受buff
@Input：  RXSize 切换后的接收区buff大小
          TxSize 切换后的发送区buff大小
		  timeout 等待接受状态的超时时间
@Output： NULL
@Return   NULL
@Warning: NULL
@note:   
****************************************************************************************
 **/
/*
void Usart3Data_Buff_Change(u32 RXsize, u32 TXsize, u32 timeout)
{
	OS_ERR err;

    //等待接收区可以切换
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
	OSMutexPend (&Usart3Data_TX_MUTEX,0,OS_OPT_PEND_BLOCKING,0,&err);//请求互斥信号量
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
//UART底层初始化，时钟使能，引脚配置，中断配置
//此函数会被HAL_UART_Init()调用
//huart:串口句柄

void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{
    //GPIO端口设置
	GPIO_InitTypeDef GPIO_Initure;
	
	if(huart->Instance==USART1)//如果是串口1，进行串口1 MSP初始化
	{
		__HAL_RCC_GPIOA_CLK_ENABLE();			//使能GPIOA时钟
		__HAL_RCC_USART1_CLK_ENABLE();			//使能USART1时钟
	
		GPIO_Initure.Pin=GPIO_PIN_9;			//PA9
		GPIO_Initure.Mode=GPIO_MODE_AF_PP;		//复用推挽输出
		GPIO_Initure.Pull=GPIO_PULLUP;			//上拉
		GPIO_Initure.Speed=GPIO_SPEED_FREQ_HIGH;//高速
		GPIO_Initure.Alternate=GPIO_AF7_USART1;	//复用为USART1
		HAL_GPIO_Init(GPIOA,&GPIO_Initure);	   	//初始化PA9

		GPIO_Initure.Pin=GPIO_PIN_10;			//PA10
		HAL_GPIO_Init(GPIOA,&GPIO_Initure);	   	//初始化PA10
		
#if EN_USART1_RX
		HAL_NVIC_EnableIRQ(USART1_IRQn);				//使能USART1中断通道
		HAL_NVIC_SetPriority(USART1_IRQn,3,3);			//抢占优先级3，子优先级3
#endif	
	}
		if(huart==(&UART3_Handler))
	{
		  //GPIO端口设置
		GPIO_InitTypeDef GPIO_Initure;
	
		__HAL_RCC_GPIOB_CLK_ENABLE();			//使能GPIOB时钟
		__HAL_RCC_USART3_CLK_ENABLE();			//使能USART3时钟
	
		GPIO_Initure.Pin=GPIO_PIN_10;			//PB10
		GPIO_Initure.Mode=GPIO_MODE_AF_PP;		//复用推挽输出
		GPIO_Initure.Pull=GPIO_PULLUP;			//上拉
		GPIO_Initure.Speed=GPIO_SPEED_FREQ_HIGH;		//高速
		GPIO_Initure.Alternate=GPIO_AF7_USART3;	//复用为USART3
		HAL_GPIO_Init(GPIOB,&GPIO_Initure);	   	//初始化PB10

		GPIO_Initure.Pin=GPIO_PIN_11;			//PB11
		HAL_GPIO_Init(GPIOB,&GPIO_Initure);	   	//初始化PB11
	
//		__HAL_UART_DISABLE_IT(huart,UART_IT_TC);
		__HAL_UART_ENABLE_IT(huart,UART_IT_RXNE);		//开启接收中断
		HAL_NVIC_EnableIRQ(USART3_IRQn);				//使能USART3中断
		HAL_NVIC_SetPriority(USART3_IRQn,2,3);			//抢占优先级2，子优先级3	
		TIM7_Int_Init(1000-1,9000-1);		//100ms中断
		TIM7->CR1&=~(1<<0);        //关闭定时器7
		
		Usart3Data_Init();
	}

}
void usart3_send(u8 *data,u32 length)
{
	uint32_t i;
	if(length>Usart3Data.USART3_MAX_SEND_LEN)
		return;
	for(i=0;i<length;i++)							//循环发送数据
	{
		while((USART3->ISR&0X40)==0);			//循环发送,直到发送完毕   
		USART3->TDR=data[i];  
	} 
}
/*串口3,printf 函数,简短的AT指令可以用u3_printf,长数据就不能用，因为u3_printf的数据先是放在heap中再在
print是调到AT_CMD，长数据就是直接放在USART3_TX_BUF中
确保一次发送数据不超过USART3_MAX_SEND_LEN字节
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
	len=strlen((const char*)AT_CMD);		//此次发送数据的长度
	
	if(len>USART3_CMD_BUFF_LEN)
		return;
	for(i=0;i<len;i++)							//循环发送数据
	{
		while((USART3->ISR&0X40)==0);			//循环发送,直到发送完毕   
		USART3->TDR=AT_CMD[i];  
	} 
}


void USART3_IRQHandler(void)
{
	u8 res;	      
	if(__HAL_UART_GET_FLAG(&UART3_Handler,UART_FLAG_RXNE)!=RESET)//接收到数据
	{	 
//		HAL_UART_Receive(&UART3_Handler,&res,1,1000);
		res=USART3->RDR; 			 
		if((Usart3Data.USART3_RX_STA&RECEIVE_OK_U3)==0)//接收完的一批数据,还没有被处理,则不再接收其他数据
		{ 
			if(Usart3Data.USART3_RX_STA<Usart3Data.USART3_MAX_RECV_LEN)	//还可以接收数据
			{
//				__HAL_TIM_SetCounter(&TIM7_Handler,0);	
				TIM7->CNT=0;         				//计数器清空	
				if(Usart3Data.USART3_RX_STA==0) 				//使能定时器7的中断 
				{
//					__HAL_RCC_TIM7_CLK_ENABLE();            //使能TIM7时钟
					TIM7->CR1|=1<<0;     			//使能定时器7
				}
				Usart3Data.USART3_RX_BUF[Usart3Data.USART3_RX_STA++]=res;	//记录接收到的值	 
			}else 
			{
				Usart3Data.USART3_RX_STA|=RECEIVE_OK_U3;				//强制标记接收完成
			} 
		}
	}  				 											 
}


void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart->Instance==USART1)//如果是串口1
	{
		if((USART_RX_STA&0x8000)==0)//接收未完成
		{
			if(USART_RX_STA&0x4000)//接收到了0x0d
			{
				if(aRxBuffer[0]!=0x0a)USART_RX_STA=0;//接收错误,重新开始
				else USART_RX_STA|=0x8000;	//接收完成了 
			}
			else //还没收到0X0D
			{	
				if(aRxBuffer[0]==0x0d)USART_RX_STA|=0x4000;
				else
				{
					USART_RX_BUF[USART_RX_STA&0X3FFF]=aRxBuffer[0] ;
					USART_RX_STA++;
					if(USART_RX_STA>(USART_REC_LEN-1))USART_RX_STA=0;//接收数据错误,重新开始接收	  
				}		 
			}
		}

	}
}
 
//串口1中断服务程序
void USART1_IRQHandler(void)                	
{ 
	u32 timeout=0;
    u32 maxDelay=0x1FFFF;
#if SYSTEM_SUPPORT_OS	 	//使用OS
	OSIntEnter();    
#endif
	
	HAL_UART_IRQHandler(&UART1_Handler);	//调用HAL库中断处理公用函数
	
	timeout=0;
    while (HAL_UART_GetState(&UART1_Handler)!=HAL_UART_STATE_READY)//等待就绪
	{
        timeout++;////超时处理
        if(timeout>maxDelay) break;		
	}
     
	timeout=0;
	while(HAL_UART_Receive_IT(&UART1_Handler,(u8 *)aRxBuffer, RXBUFFERSIZE)!=HAL_OK)//一次处理完成之后，重新开启中断并设置RxXferCount为1
	{
        timeout++; //超时处理
        if(timeout>maxDelay) break;	
	}
#if SYSTEM_SUPPORT_OS	 	//使用OS
	OSIntExit();  											 
#endif
} 
#endif	



 

 




