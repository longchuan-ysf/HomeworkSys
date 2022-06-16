#include "sdmmc_sdcard.h"
#include "string.h"
#include "delay.h"
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32H7������
//SD����������	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2017/8/15
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2014-2024
//All rights reserved									  
//////////////////////////////////////////////////////////////////////////////////

SD_HandleTypeDef        SDCARD_Handler;             //SD�����   
HAL_SD_CardInfoTypeDef  SDCardInfo;					//SD����Ϣ�ṹ��
DMA_HandleTypeDef SDTxDMAHandler,SDRxDMAHandler;    //SD��DMA���ͺͽ��վ��

//���ʹ��DMA�Ļ��������������������SD����д�Ƿ����
static volatile u8 SDCardWriteStatus=0,SDCardReadStatus=0; 

//SD_ReadDisk/SD_WriteDisk����ר��buf,�����������������ݻ�������ַ����4�ֽڶ����ʱ��,
//��Ҫ�õ�������,ȷ�����ݻ�������ַ��4�ֽڶ����.
__align(4) u8 SDIO_DATA_BUFFER[512];

//SD����ʼ��,SDMMCƵ��Ϊ200MHz��SD�����Ƶ��25MHz
//����ֵ:0 ��ʼ����ȷ������ֵ����ʼ������
u8 SD_Init(void)
{
    u8 SD_Error;
    
    //��ʼ��ʱ��ʱ�Ӳ��ܴ���400KHZ 
    SDCARD_Handler.Instance=SDMMC1;
    SDCARD_Handler.Init.ClockEdge=SDMMC_CLOCK_EDGE_RISING;              //������     
    SDCARD_Handler.Init.ClockPowerSave=SDMMC_CLOCK_POWER_SAVE_DISABLE;  //����ʱ���ر�ʱ�ӵ�Դ
    SDCARD_Handler.Init.BusWide=SDMMC_BUS_WIDE_4B;                      //4λ������
    SDCARD_Handler.Init.HardwareFlowControl=SDMMC_HARDWARE_FLOW_CONTROL_DISABLE;//�ر�Ӳ������
    SDCARD_Handler.Init.ClockDiv=SDMMC_NSpeed_CLK_DIV;               	//SD����ʱ��Ƶ�����25MHZ
    
    SD_Error=HAL_SD_Init(&SDCARD_Handler);
    if(SD_Error!=HAL_OK) return 1;
	
	//��ȡSD����Ϣ
	HAL_SD_GetCardInfo(&SDCARD_Handler,&SDCardInfo);
    return 0;
}

//SDMMC�ײ�������ʱ��ʹ�ܣ��������ã�DMA����
//�˺����ᱻHAL_SD_Init()����
//hsd:SD�����
void HAL_SD_MspInit(SD_HandleTypeDef *hsd)
{
    GPIO_InitTypeDef GPIO_Initure;

    __HAL_RCC_SDMMC1_CLK_ENABLE();  //ʹ��SDMMC1ʱ��
    __HAL_RCC_GPIOC_CLK_ENABLE();   //ʹ��GPIOCʱ��
    __HAL_RCC_GPIOD_CLK_ENABLE();   //ʹ��GPIODʱ��
    
    //PC8,9,10,11,12
    GPIO_Initure.Pin=GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_11|GPIO_PIN_12;
    GPIO_Initure.Mode=GPIO_MODE_AF_PP;      //���츴��
    GPIO_Initure.Pull=GPIO_PULLUP;          //����
    GPIO_Initure.Speed=GPIO_SPEED_FREQ_VERY_HIGH;     //����
    GPIO_Initure.Alternate=GPIO_AF12_SDIO1; //����ΪSDIO
    HAL_GPIO_Init(GPIOC,&GPIO_Initure);     //��ʼ��
    
    //PD2
    GPIO_Initure.Pin=GPIO_PIN_2;            
    HAL_GPIO_Init(GPIOD,&GPIO_Initure);     //��ʼ��

#if (SD_DMA_MODE==1)                        //ʹ��DMAģʽ
    HAL_NVIC_SetPriority(SDMMC1_IRQn,2,0);  //����SDMMC1�жϣ���ռ���ȼ�2�������ȼ�0
    HAL_NVIC_EnableIRQ(SDMMC1_IRQn);        //ʹ��SDMMC1�ж�
#endif
}

//�õ�����Ϣ
//cardinfo:����Ϣ�洢��
//����ֵ:����״̬
u8 SD_GetCardInfo(HAL_SD_CardInfoTypeDef *cardinfo)
{
    u8 sta;
	sta=HAL_SD_GetCardInfo(&SDCARD_Handler,cardinfo);
    return sta;
}

//�ж�SD���Ƿ���Դ���(��д)����
//����ֵ:SD_TRANSFER_OK ������ɣ����Լ�����һ�δ���
//		 SD_TRANSFER_BUSY SD����æ�������Խ�����һ�δ���
u8 SD_GetCardState(void)
{
  return((HAL_SD_GetCardState(&SDCARD_Handler)==HAL_SD_CARD_TRANSFER )?SD_TRANSFER_OK:SD_TRANSFER_BUSY);
}

#if (SD_DMA_MODE==1)        //DMAģʽ

//��SD��
//buf:�����ݻ�����
//sector:������ַ
//cnt:��������	
//����ֵ:����״̬;0,����;����,�������;
u8 SD_ReadDisk(u8* buf,u32 sector,u32 cnt)
{
    u8 sta=HAL_ERROR;
	SDCardReadStatus=0;
	
	if(HAL_SD_ReadBlocks_DMA(&SDCARD_Handler,(uint8_t*)buf,(uint32_t)sector,(uint32_t)cnt)==HAL_OK)
	{
		while(SDCardReadStatus==0){};	//�ȴ������
		
		SDCardReadStatus=0;
		while(SD_GetCardState()){};		//�ȴ�SD������
		sta=HAL_OK;
	}
	
    return sta;
}  

//дSD��
//buf:д���ݻ�����
//sector:������ַ
//cnt:��������	
//����ֵ:����״̬;0,����;����,�������;	
u8 SD_WriteDisk(u8 *buf,u32 sector,u32 cnt)
{   
    u8 sta=HAL_ERROR;
	
	SDCardWriteStatus=0;
	
	if(HAL_SD_WriteBlocks_DMA(&SDCARD_Handler,(uint8_t*)buf,(uint32_t)sector,(uint32_t)cnt)==HAL_OK)
	{
		while(SDCardWriteStatus==0){};	//�ȴ�д���
		
		SDCardWriteStatus=0;
		while(SD_GetCardState()){};		//�ȴ�SD������
		sta=HAL_OK;
	}
		
    return sta;
} 

//SDMMC1�жϷ�����
void SDMMC1_IRQHandler(void)
{
    HAL_SD_IRQHandler(&SDCARD_Handler);
}

//SDMMC1д��ɻص�����
void HAL_SD_TxCpltCallback(SD_HandleTypeDef *hsd)
{
	SDCardWriteStatus=1; //���д���
}

//SDMMC1����ɻص�����
void HAL_SD_RxCpltCallback(SD_HandleTypeDef *hsd)
{
	SDCardReadStatus=1;	//��Ƕ����
}

#else                                   //��ѵģʽ

//��SD��
//buf:�����ݻ�����
//sector:������ַ
//cnt:��������	
//����ֵ:����״̬;0,����;����,�������;
u8 SD_ReadDisk(u8* buf,u32 sector,u32 cnt)
{
    u8 sta=HAL_OK;
	u32 timeout=SD_TIMEOUT;
    long long lsector=sector;
    INTX_DISABLE();//�ر����ж�(POLLINGģʽ,�Ͻ��жϴ��SDIO��д����!!!)
	sta=HAL_SD_ReadBlocks(&SDCARD_Handler, (uint8_t*)buf,lsector,cnt,SD_TIMEOUT);//���sector�Ķ�����
	
	//�ȴ�SD������
	while(SD_GetCardState()!=SD_TRANSFER_OK)
    {
		if(timeout-- == 0)
		{	
			sta=SD_TRANSFER_BUSY;
		}
    }
    INTX_ENABLE();//�������ж�
    return sta;
}  


//дSD��
//buf:д���ݻ�����
//sector:������ַ
//cnt:��������	
//����ֵ:����״̬;0,����;����,�������;	
u8 SD_WriteDisk(u8 *buf,u32 sector,u32 cnt)
{   
    u8 sta=HAL_OK;
	u32 timeout=SD_TIMEOUT;
    long long lsector=sector;
    INTX_DISABLE();//�ر����ж�(POLLINGģʽ,�Ͻ��жϴ��SDIO��д����!!!)
	sta=HAL_SD_WriteBlocks(&SDCARD_Handler,(uint8_t*)buf,lsector,cnt,SD_TIMEOUT);//���sector��д����
		
	//�ȴ�SD��д��
	while(SD_GetCardState()!=SD_TRANSFER_OK)
    {
		if(timeout-- == 0)
		{	
			sta=SD_TRANSFER_BUSY;
		}
    }    
	INTX_ENABLE();//�������ж�
    return sta;
}
#endif




