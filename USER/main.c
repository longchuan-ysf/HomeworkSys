#include "main.h"
/************************************************
 ALIENTEK STM32������STemWinʵ��
 STemwin JPEGͼƬ��ʾ
 
 UCOSIII���������ȼ��û�������ʹ�ã�ALIENTEK
 ����Щ���ȼ��������UCOSIII��5��ϵͳ�ڲ�����
 ���ȼ�0���жϷ������������� OS_IntQTask()
 ���ȼ�1��ʱ�ӽ������� OS_TickTask()
 ���ȼ�2����ʱ���� OS_TmrTask()
 ���ȼ�OS_CFG_PRIO_MAX-2��ͳ������ OS_StatTask()
 ���ȼ�OS_CFG_PRIO_MAX-1���������� OS_IdleTask()
 ����֧�֣�www.openedv.com
 �Ա����̣�http://eboard.taobao.com  
 ��ע΢�Ź���ƽ̨΢�źţ�"����ԭ��"����ѻ�ȡSTM32���ϡ�
 ������������ӿƼ����޹�˾  
 ���ߣ�����ԭ�� @ALIENTEK
************************************************/

//�������ȼ�
#define START_TASK_PRIO				3
//�����ջ��С	
#define START_STK_SIZE 				128
//������ƿ�
OS_TCB StartTaskTCB;
//�����ջ	
CPU_STK START_TASK_STK[START_STK_SIZE];
//������
void start_task(void *p_arg);

//TOUCH����
//�����������ȼ�
#define TOUCH_TASK_PRIO				4
//�����ջ��С
#define TOUCH_STK_SIZE				256
//������ƿ�
OS_TCB TouchTaskTCB;
//�����ջ
CPU_STK TOUCH_TASK_STK[TOUCH_STK_SIZE];
//touch����
void touch_task(void *p_arg);
//EMWINDEMO����
//�����������ȼ�
#define EMWINDEMO_TASK_PRIO			5
//�����ջ��С
#define EMWINDEMO_STK_SIZE			1024
//������ƿ�
OS_TCB EmwindemoTaskTCB;
//�����ջ
CPU_STK EMWINDEMO_TASK_STK[EMWINDEMO_STK_SIZE];
//emwindemo_task����
void emwindemo_task(void *p_arg);

//��������
//�����������ȼ�
#define KEY_TASK_PRIO 				6
//�����ջ��С
#define KEY_STK_SIZE				1024
//������ƿ�
OS_TCB KeyTaskTCB;
//�����ջ
CPU_STK KEY_TASK_STK[KEY_STK_SIZE];
//led0����
void key_task(void *p_arg);

//LED0����
//�����������ȼ�
#define LED0_TASK_PRIO 				7
//�����ջ��С
#define LED0_STK_SIZE				256
//������ƿ�
OS_TCB Led0TaskTCB;
//�����ջ
CPU_STK LED0_TASK_STK[LED0_STK_SIZE];
//led0����
void led0_task(void *p_arg);

//USB����
//�����������ȼ�
#define USB_TASK_PRIO 				8
//�����ջ��С
#define USB_STK_SIZE				(512)
//������ƿ�
OS_TCB USB_TaskTCB;
//�����ջ
CPU_STK USB_TASK_STK[USB_STK_SIZE];
//USB����
void USB_task(void *p_arg);

//USB��д����
//�����������ȼ�
#define USB_WR_TASK_PRIO 				9
//�����ջ��С
#define USB_WR_STK_SIZE				(512)
//������ƿ�
OS_TCB USB_WR_TaskTCB;
//�����ջ
CPU_STK USB_WR_TASK_STK[USB_WR_STK_SIZE];
//USB��д����
void USB_WR_task(void *p_arg);

int main(void)
{
    OS_ERR err;
	CPU_SR_ALLOC();
    
    Write_Through();                //Cahceǿ��͸д
    MPU_Memory_Protection();        //������ش洢����
    Cache_Enable();                 //��L1-Cache
	HAL_Init();				        //��ʼ��HAL��
	Stm32_Clock_Init(160,5,2,4);    //����ʱ��,400Mhz 
	delay_init(400);			    //��ʱ��ʼ��
	uart_init(115200);			    //���ڳ�ʼ��
	usmart_dev.init(200); 		    //��ʼ��USMART	
	LED_Init();					    //��ʼ��LED
	PCF8574_Init();
	KEY_Init();					    //��ʼ������
	SDRAM_Init();                   //��ʼ��SDRAM
	TFTLCD_Init();				    //��ʼ��LCD
	tp_dev.init();				    //��������ʼ�� 
    W25QXX_Init();				    //��ʼ��W25Q256
    my_mem_init(SRAMIN);		    //��ʼ���ڲ��ڴ��
	my_mem_init(SRAMEX);		    //��ʼ���ⲿ�ڴ��
	my_mem_init(SRAMDTCM);		    //��ʼ��DTCM�ڴ��
	
    //�������õ��ڴ����ĺ�����һ��Ҫ���ڴ��ʼ�����ڳ�ʼ������
	usart3_init(115200);  			//��ʼ������3������Ϊ115200
	MYDMA_Config(DMA2_Stream7);				//��ʼ��DMA
	
    exfuns_init();			        //Ϊfatfs��ر��������ڴ�				 
  	f_mount(fs[0],"0:",1); 		    //����SD�� 
    f_mount(fs[1],"1:",1); 	        //����FLASH.	
    f_mount(fs[2],"2:",1); 		    //����NAND FLASH.
	
    while(font_init())		        //��ʼ���ֿ�
	{
		LCD_ShowString(30,70,200,16,16,"Font Error!");
		while(SD_Init())	        //���SD��
		{
			LCD_ShowString(30,90,200,16,16,"SD Card Failed!");
			delay_ms(200);
			LCD_Fill(30,90,200+30,70+16,WHITE);
			delay_ms(200);		    
		}
		update_font(30,90,16,"0:");	//����ֿⲻ���ھ͸����ֿ�
		delay_ms(2000);
		LCD_Clear(WHITE);	//����
		break;
	}
    OSInit(&err);		            //��ʼ��UCOSIII
	OS_CRITICAL_ENTER();            //�����ٽ���
	//������ʼ����
	OSTaskCreate((OS_TCB 	* )&StartTaskTCB,		//������ƿ�
				 (CPU_CHAR	* )"start task", 		//��������
                 (OS_TASK_PTR )start_task, 			//������
                 (void		* )0,					//���ݸ��������Ĳ���
                 (OS_PRIO	  )START_TASK_PRIO,     //�������ȼ�
                 (CPU_STK   * )&START_TASK_STK[0],	//�����ջ����ַ
                 (CPU_STK_SIZE)START_STK_SIZE/10,	//�����ջ�����λ
                 (CPU_STK_SIZE)START_STK_SIZE,		//�����ջ��С
                 (OS_MSG_QTY  )0,					//�����ڲ���Ϣ�����ܹ����յ������Ϣ��Ŀ,Ϊ0ʱ��ֹ������Ϣ
                 (OS_TICK	  )0,					//��ʹ��ʱ��Ƭ��תʱ��ʱ��Ƭ���ȣ�Ϊ0ʱΪĬ�ϳ��ȣ�
                 (void   	* )0,					//�û�����Ĵ洢��
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR, //����ѡ��
                 (OS_ERR 	* )&err);				//��Ÿú�������ʱ�ķ���ֵ
	OS_CRITICAL_EXIT();	            //�˳��ٽ���	 
	OSStart(&err);                  //����UCOSIII
	while(1);						     
}

//��ʼ������
void start_task(void *p_arg)
{
	OS_ERR err;
	CPU_SR_ALLOC();
	p_arg = p_arg;

	CPU_Init();
#if OS_CFG_STAT_TASK_EN > 0u
   OSStatTaskCPUUsageInit(&err);  	//ͳ������                
#endif
	
#ifdef CPU_CFG_INT_DIS_MEAS_EN		//���ʹ���˲����жϹر�ʱ��
    CPU_IntDisMeasMaxCurReset();	
#endif

#if	OS_CFG_SCHED_ROUND_ROBIN_EN  //��ʹ��ʱ��Ƭ��ת��ʱ��
	//ʹ��ʱ��Ƭ��ת���ȹ���,����Ĭ�ϵ�ʱ��Ƭ����
	OSSchedRoundRobinCfg(DEF_ENABLED,1,&err);  
#endif		
	__HAL_RCC_CRC_CLK_ENABLE();		//ʹ��CRCʱ��
	GUI_Init();  			//STemWin��ʼ��
    printf("emWin Version:%s\r\n",(char*)GUI_GetVersionString());
    //WM_SetCreateFlags(WM_CF_MEMDEV);
	WM_MULTIBUF_Enable(1);  //����STemWin�໺��,RGB�����ܻ��õ�
	OS_CRITICAL_ENTER();	//�����ٽ���
	//STemWin Demo����	
	OSTaskCreate((OS_TCB*     )&EmwindemoTaskTCB,		
				 (CPU_CHAR*   )"Emwindemo task", 		
                 (OS_TASK_PTR )emwindemo_task, 			
                 (void*       )0,					
                 (OS_PRIO	  )EMWINDEMO_TASK_PRIO,     
                 (CPU_STK*    )&EMWINDEMO_TASK_STK[0],	
                 (CPU_STK_SIZE)EMWINDEMO_STK_SIZE/10,	
                 (CPU_STK_SIZE)EMWINDEMO_STK_SIZE,		
                 (OS_MSG_QTY  )0,					
                 (OS_TICK	  )0,  					
                 (void*       )0,					
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR|OS_OPT_TASK_SAVE_FP,
                 (OS_ERR*     )&err);
	//����������
	OSTaskCreate((OS_TCB*     )&TouchTaskTCB,		
				 (CPU_CHAR*   )"Touch task", 		
                 (OS_TASK_PTR )touch_task, 			
                 (void*       )0,					
                 (OS_PRIO	  )TOUCH_TASK_PRIO,     
                 (CPU_STK*    )&TOUCH_TASK_STK[0],	
                 (CPU_STK_SIZE)TOUCH_STK_SIZE/10,	
                 (CPU_STK_SIZE)TOUCH_STK_SIZE,		
                 (OS_MSG_QTY  )0,					
                 (OS_TICK	  )0,  					
                 (void*       )0,					
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR|OS_OPT_TASK_SAVE_FP,
                 (OS_ERR*     )&err);			 
	//LED0����
	OSTaskCreate((OS_TCB*     )&Led0TaskTCB,		
				 (CPU_CHAR*   )"Led0 task", 		
                 (OS_TASK_PTR )led0_task, 			
                 (void*       )0,					
                 (OS_PRIO	  )LED0_TASK_PRIO,     
                 (CPU_STK*    )&LED0_TASK_STK[0],	
                 (CPU_STK_SIZE)LED0_STK_SIZE/10,	
                 (CPU_STK_SIZE)LED0_STK_SIZE,		
                 (OS_MSG_QTY  )0,					
                 (OS_TICK	  )0,  					
                 (void*       )0,					
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR|OS_OPT_TASK_SAVE_FP,
                 (OS_ERR*     )&err);	                 
; 
	//��������
	OSTaskCreate((OS_TCB*     )&KeyTaskTCB,		
				 (CPU_CHAR*   )"Key task", 		
                 (OS_TASK_PTR )key_task, 			
                 (void*       )0,					
                 (OS_PRIO	  )KEY_TASK_PRIO,     
                 (CPU_STK*    )&KEY_TASK_STK[0],	
                 (CPU_STK_SIZE)KEY_STK_SIZE/10,	
                 (CPU_STK_SIZE)KEY_STK_SIZE,		
                 (OS_MSG_QTY  )0,					
                 (OS_TICK	  )0,  					
                 (void*       )0,					
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR,
                 (OS_ERR*     )&err); 
	//USB����
	OSTaskCreate((OS_TCB*     )&USB_TaskTCB,		
				 (CPU_CHAR*   )"USB task", 		
                 (OS_TASK_PTR )USB_task, 			
                 (void*       )0,					
                 (OS_PRIO	  )USB_TASK_PRIO,     
                 (CPU_STK*    )&USB_TASK_STK[0],	
                 (CPU_STK_SIZE)USB_STK_SIZE/10,	
                 (CPU_STK_SIZE)USB_STK_SIZE,		
                 (OS_MSG_QTY  )0,					
                 (OS_TICK	  )0,  					
                 (void*       )0,					
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR,
                 (OS_ERR*     )&err);   
	//USB��д����
	OSTaskCreate((OS_TCB*     )&USB_WR_TaskTCB,		
				 (CPU_CHAR*   )"USB_WR task", 		
                 (OS_TASK_PTR )USB_WR_task, 			
                 (void*       )0,					
                 (OS_PRIO	  )USB_WR_TASK_PRIO,     
                 (CPU_STK*    )&USB_WR_TASK_STK[0],	
                 (CPU_STK_SIZE)USB_WR_STK_SIZE/10,	
                 (CPU_STK_SIZE)USB_WR_STK_SIZE,		
                 (OS_MSG_QTY  )0,					
                 (OS_TICK	  )0,  					
                 (void*       )0,					
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR,
                 (OS_ERR*     )&err); 				 
	OS_TaskSuspend((OS_TCB*)&StartTaskTCB,&err);		//����ʼ����			 
	OS_CRITICAL_EXIT();	//�˳��ٽ���
}

//EMWINDEMO����

void emwindemo_task(void *p_arg)
{
	OS_ERR err;
	uint32_t i;
	CreateFramewin();
	while(1)
	{
       GUI_Delay(5);
	   //OSTimeDlyHMSM(0,0,0,10,OS_OPT_TIME_PERIODIC,&err);//��ʱ10ms
	}
}
extern void test_http_post(void);
extern void test_http_get(void);
extern void test_post(void);
void key_task(void *pdata)
{
	OS_ERR err;
//	atk_8266_test();
//	BackGroundInit();
	camera_app_init();
	u8 key;
	while(1)
	{
//		MessageRxHandle();
		if(CurrentPage==PAGE_CAMERA)
		{			
			key = KEY_Scan(0);
			camera_key_handle(key);
			if(rgb565_data_ok)
			{
				rgb565_data_ok=0;
				WM_InvalidateWindow(WM_Camera);
				
			}
		}
		else if(CurrentPage==PAGE_VIEW)
		{
		
		}
		OSTimeDlyHMSM(0,0,0,5,OS_OPT_TIME_PERIODIC,&err);//��ʱ1ms

	}
}

//TOUCH����
void touch_task(void *p_arg)
{
	OS_ERR err;
	while(1)
	{
        tp_dev.scan(0);
		GUI_TOUCH_Exec();	
		OSTimeDlyHMSM(0,0,0,10,OS_OPT_TIME_PERIODIC,&err);//��ʱ5ms
	}
}
void Memory_Usage()
{
	OS_ERR err;
	u8 paddr[20];				    		//���P Addr:+p��ַ��ASCIIֵ
	u16 memused=0;
	CPU_STK_SIZE myfree,myused;
	memused=my_mem_perused(SRAMIN);
	sprintf((char*)paddr,"%d.%01d%%",memused/10,memused%10);
	printf("�ڲ��ڴ�ʹ���ʣ�%s\r\n",paddr);	//��ʾ�ڲ��ڴ�ʹ����
	
	
	memused=my_mem_perused(SRAMEX);
	sprintf((char*)paddr,"%d.%01d%%",memused/10,memused%10);
	printf("�ⲿ�ڴ�ʹ���ʣ�%s\r\n",paddr);	//��ʾ�ⲿ�ڴ�ʹ����
	
	OSTaskStkChk(&EmwindemoTaskTCB,&myfree,&myused,&err);
	printf("EmwindemoTaskTCB has free:%d, Usage:%%%d\r\n",myfree,(myused*100)/(myfree+myused));	//��������1�Ŀ����ֽ����Ͷ�ջʹ����
	
	OSTaskStkChk(&TouchTaskTCB,&myfree,&myused,&err);
	printf("TouchTaskTCB has free:%d, Usage:%%%d\r\n",myfree,(myused*100)/(myfree+myused));	//��������1�Ŀ����ֽ����Ͷ�ջʹ����
	
	OSTaskStkChk(&Led0TaskTCB,&myfree,&myused,&err);
	printf("Led0TaskTCB has free:%d, Usage:%%%d\r\n",myfree,(myused*100)/(myfree+myused));	//��������1�Ŀ����ֽ����Ͷ�ջʹ����
	
	OSTaskStkChk(&KeyTaskTCB,&myfree,&myused,&err);
	printf("KeyTaskTCB has free:%d, Usage:%%%d\r\n",myfree,(myused*100)/(myfree+myused));	//��������1�Ŀ����ֽ����Ͷ�ջʹ����
	
	OSTaskStkChk(&USB_TaskTCB,&myfree,&myused,&err);
	printf("USB_TaskTCB has free:%d, Usage:%%%d\r\n",myfree,(myused*100)/(myfree+myused));	//��������1�Ŀ����ֽ����Ͷ�ջʹ����

}

//LED0����
void led0_task(void *p_arg)
{
	OS_ERR err;
	u8 time=0;	
	while(1)
	{
		
		LED0_Toggle;
		time++;
		OSTimeDlyHMSM(0,0,0,500,OS_OPT_TIME_PERIODIC,&err);//��ʱ500ms
		if(time == 10)//5�����һ���ڴ�ʹ����
		{
			//Memory_Usage();
			time = 0;
		}
	}
}
//USBH_Process����Ҫ��������
void USB_task(void *pdata)
{
    usb_app_main();
}
//���������Ҫ���ܾ��Ǵ����������񶪸�USB�Ķ�дָ��
void USB_WR_task(void *p_arg)
{
	usb_app_wr();
}

