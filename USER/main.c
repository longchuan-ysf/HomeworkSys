#include "main.h"
/************************************************
 ALIENTEK STM32开发板STemWin实验
 STemwin JPEG图片显示
 
 UCOSIII中以下优先级用户程序不能使用，ALIENTEK
 将这些优先级分配给了UCOSIII的5个系统内部任务
 优先级0：中断服务服务管理任务 OS_IntQTask()
 优先级1：时钟节拍任务 OS_TickTask()
 优先级2：定时任务 OS_TmrTask()
 优先级OS_CFG_PRIO_MAX-2：统计任务 OS_StatTask()
 优先级OS_CFG_PRIO_MAX-1：空闲任务 OS_IdleTask()
 技术支持：www.openedv.com
 淘宝店铺：http://eboard.taobao.com  
 关注微信公众平台微信号："正点原子"，免费获取STM32资料。
 广州市星翼电子科技有限公司  
 作者：正点原子 @ALIENTEK
************************************************/

//任务优先级
#define START_TASK_PRIO				3
//任务堆栈大小	
#define START_STK_SIZE 				128
//任务控制块
OS_TCB StartTaskTCB;
//任务堆栈	
CPU_STK START_TASK_STK[START_STK_SIZE];
//任务函数
void start_task(void *p_arg);

//TOUCH任务
//设置任务优先级
#define TOUCH_TASK_PRIO				4
//任务堆栈大小
#define TOUCH_STK_SIZE				256
//任务控制块
OS_TCB TouchTaskTCB;
//任务堆栈
CPU_STK TOUCH_TASK_STK[TOUCH_STK_SIZE];
//touch任务
void touch_task(void *p_arg);
//EMWINDEMO任务
//设置任务优先级
#define EMWINDEMO_TASK_PRIO			5
//任务堆栈大小
#define EMWINDEMO_STK_SIZE			1024
//任务控制块
OS_TCB EmwindemoTaskTCB;
//任务堆栈
CPU_STK EMWINDEMO_TASK_STK[EMWINDEMO_STK_SIZE];
//emwindemo_task任务
void emwindemo_task(void *p_arg);

//按键任务
//设置任务优先级
#define KEY_TASK_PRIO 				6
//任务堆栈大小
#define KEY_STK_SIZE				512
//任务控制块
OS_TCB KeyTaskTCB;
//任务堆栈
CPU_STK KEY_TASK_STK[KEY_STK_SIZE];
//led0任务
void key_task(void *p_arg);

//LED0任务
//设置任务优先级
#define LED0_TASK_PRIO 				7
//任务堆栈大小
#define LED0_STK_SIZE				256
//任务控制块
OS_TCB Led0TaskTCB;
//任务堆栈
CPU_STK LED0_TASK_STK[LED0_STK_SIZE];
//led0任务
void led0_task(void *p_arg);

//USB任务
//设置任务优先级
#define USB_TASK_PRIO 				8
//任务堆栈大小
#define USB_STK_SIZE				(512)
//任务控制块
OS_TCB USB_TaskTCB;
//任务堆栈
CPU_STK USB_TASK_STK[USB_STK_SIZE];
//USB任务
void USB_task(void *p_arg);

//USB读写任务
//设置任务优先级
#define USB_WR_TASK_PRIO 				9
//任务堆栈大小
#define USB_WR_STK_SIZE				(512)
//任务控制块
OS_TCB USB_WR_TaskTCB;
//任务堆栈
CPU_STK USB_WR_TASK_STK[USB_WR_STK_SIZE];
//USB读写任务
void USB_WR_task(void *p_arg);

int main(void)
{
    OS_ERR err;
	CPU_SR_ALLOC();
    
    Write_Through();                //Cahce强制透写
    MPU_Memory_Protection();        //保护相关存储区域
    Cache_Enable();                 //打开L1-Cache
	HAL_Init();				        //初始化HAL库
	Stm32_Clock_Init(160,5,2,4);    //设置时钟,400Mhz 
	delay_init(400);			    //延时初始化
	uart_init(115200);			    //串口初始化
	usmart_dev.init(200); 		    //初始化USMART	
	LED_Init();					    //初始化LED
	PCF8574_Init();
	KEY_Init();					    //初始化按键
	SDRAM_Init();                   //初始化SDRAM
	TFTLCD_Init();				    //初始化LCD
	tp_dev.init();				    //触摸屏初始化 
    W25QXX_Init();				    //初始化W25Q256
    my_mem_init(SRAMIN);		    //初始化内部内存池
	my_mem_init(SRAMEX);		    //初始化外部内存池
	my_mem_init(SRAMDTCM);		    //初始化DTCM内存池
	
    //里面有用到内存管理的函数，一定要在内存初始化完在初始化串口
	usart3_init(115200);  			//初始化串口3波特率为115200
	MYDMA_Config(DMA2_Stream7);				//初始化DMA
	
    exfuns_init();			        //为fatfs相关变量申请内存				 
  	f_mount(fs[0],"0:",1); 		    //挂载SD卡 
    f_mount(fs[1],"1:",1); 	        //挂载FLASH.	
    f_mount(fs[2],"2:",1); 		    //挂载NAND FLASH.
	
    while(font_init())		        //初始化字库
	{
		LCD_ShowString(30,70,200,16,16,"Font Error!");
		while(SD_Init())	        //检测SD卡
		{
			LCD_ShowString(30,90,200,16,16,"SD Card Failed!");
			delay_ms(200);
			LCD_Fill(30,90,200+30,70+16,WHITE);
			delay_ms(200);		    
		}
		update_font(30,90,16,"0:");	//如果字库不存在就更新字库
		delay_ms(2000);
		LCD_Clear(WHITE);	//清屏
		break;
	}
    OSInit(&err);		            //初始化UCOSIII
	OS_CRITICAL_ENTER();            //进入临界区
	//创建开始任务
	OSTaskCreate((OS_TCB 	* )&StartTaskTCB,		//任务控制块
				 (CPU_CHAR	* )"start task", 		//任务名字
                 (OS_TASK_PTR )start_task, 			//任务函数
                 (void		* )0,					//传递给任务函数的参数
                 (OS_PRIO	  )START_TASK_PRIO,     //任务优先级
                 (CPU_STK   * )&START_TASK_STK[0],	//任务堆栈基地址
                 (CPU_STK_SIZE)START_STK_SIZE/10,	//任务堆栈深度限位
                 (CPU_STK_SIZE)START_STK_SIZE,		//任务堆栈大小
                 (OS_MSG_QTY  )0,					//任务内部消息队列能够接收的最大消息数目,为0时禁止接收消息
                 (OS_TICK	  )0,					//当使能时间片轮转时的时间片长度，为0时为默认长度，
                 (void   	* )0,					//用户补充的存储区
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR, //任务选项
                 (OS_ERR 	* )&err);				//存放该函数错误时的返回值
	OS_CRITICAL_EXIT();	            //退出临界区	 
	OSStart(&err);                  //开启UCOSIII
	while(1);						     
}

//开始任务函数
void start_task(void *p_arg)
{
	OS_ERR err;
	CPU_SR_ALLOC();
	p_arg = p_arg;

	CPU_Init();
#if OS_CFG_STAT_TASK_EN > 0u
   OSStatTaskCPUUsageInit(&err);  	//统计任务                
#endif
	
#ifdef CPU_CFG_INT_DIS_MEAS_EN		//如果使能了测量中断关闭时间
    CPU_IntDisMeasMaxCurReset();	
#endif

#if	OS_CFG_SCHED_ROUND_ROBIN_EN  //当使用时间片轮转的时候
	//使能时间片轮转调度功能,设置默认的时间片长度
	OSSchedRoundRobinCfg(DEF_ENABLED,1,&err);  
#endif		
	__HAL_RCC_CRC_CLK_ENABLE();		//使能CRC时钟
	GUI_Init();  			//STemWin初始化
    printf("emWin Version:%s\r\n",(char*)GUI_GetVersionString());
    //WM_SetCreateFlags(WM_CF_MEMDEV);
	WM_MULTIBUF_Enable(1);  //开启STemWin多缓冲,RGB屏可能会用到
	OS_CRITICAL_ENTER();	//进入临界区
	//STemWin Demo任务	
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
	//触摸屏任务
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
	//LED0任务
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
	//按键任务
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
	//USB任务
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
	//USB读写任务
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
	OS_TaskSuspend((OS_TCB*)&StartTaskTCB,&err);		//挂起开始任务			 
	OS_CRITICAL_EXIT();	//退出临界区
}

//EMWINDEMO任务
void emwindemo_task(void *p_arg)
{
	CreateFramewin();
	while(1)
	{
       GUI_Delay(100);
	}
}
extern void test_http_post(void);
extern void test_http_get(void);
extern void test_post(void);
extern void MessageTx(void);
void test()
{
	uint32_t i;
    for(i=0; i< 0xffff+0xffff+2048;i++)
    {
        BackGroundCtrl.Message_TXBuffer[i] = i;
    }
    BackGroundCtrl.Message_TxLen = i;
    MessageTx();
}

void key_task(void *pdata)
{
	OS_ERR err;
	u8* path,*data;
	pUSBH_WR_MSG pMsgRD;
	pUSBH_WR_MSG pMsgWR;
	u32 cnt;
//	atk_8266_test();
//	BackGroundInit();
	u8 key;
	while(1)
	{
//		MessageRxHandle();
		key = KEY_Scan(0);
		switch(key)
		{
			case KEY0_PRES:
			{
				//mf_scan_files("3:");	
				path = USBH_Malloc_Path(128);	
				sprintf((char *)path,"3:usbtest.txt");
				data = USBH_Malloc_WriteBuf(200);
				cnt = sprintf((char *)data,"\r\n------WebKitFormBoundaryO2WAD3vh5VDNQCJf\r\n"\
										"Content-Disposition: form-data; name=\"file\"; filename=""\r\n"\
										"Content-Type: application/octet-stream\r\n\r\n"\
										"\r\n------WebKitFormBoundaryO2WAD3vh5VDNQCJf--\r\n");
				pMsgWR = USBH_ApplyFor_WR(path,data,cnt,FA_WRITE|FA_CREATE_ALWAYS);
				while(1)
				{
					if(pMsgWR->result==FR_OK)
					{
						USBH_WR_MsgFree(pMsgWR);
						printf("write ok\r\n");
						break;
					}
					OSTimeDlyHMSM(0,0,0,10,OS_OPT_TIME_PERIODIC,&err);//延时10ms
				}
				
			}
			break;
			case KEY1_PRES:
			{
				path = USBH_Malloc_Path(128);	
				sprintf((char *)path,"3:zhongli.jpg");
				pMsgRD = USBH_ApplyFor_WR(path,0,RD_ALL_DATA,FA_READ);
				while(1)
				{
					if(pMsgRD->result==FR_OK)
					{
						printf("read ok,data:\r\n%s",pMsgRD->data);
						break;
					}
					OSTimeDlyHMSM(0,0,0,10,OS_OPT_TIME_PERIODIC,&err);//延时10ms
				}
				
				
				path = USBH_Malloc_Path(128);	
				sprintf((char *)path,"3:zhongli_backup.jpg");
				data = USBH_Malloc_WriteBuf(pMsgRD->bread);
				pMsgWR = USBH_ApplyFor_WR(path,pMsgRD->data,pMsgRD->bread,FA_WRITE|FA_CREATE_ALWAYS);
				while(1)
				{
					if(pMsgWR->result==FR_OK)
					{
						USBH_WR_MsgFree(pMsgWR);
						printf("write ok\r\n");
						break;
					}
					OSTimeDlyHMSM(0,0,0,10,OS_OPT_TIME_PERIODIC,&err);//延时10ms
				}
				USBH_WR_MsgFree(pMsgRD);
				//test_http_get();
			}
			break;
		}
		OSTimeDlyHMSM(0,0,0,10,OS_OPT_TIME_PERIODIC,&err);//延时10ms

	}
}

//TOUCH任务
void touch_task(void *p_arg)
{
	OS_ERR err;
	while(1)
	{
        tp_dev.scan(0);
		GUI_TOUCH_Exec();	
		OSTimeDlyHMSM(0,0,0,10,OS_OPT_TIME_PERIODIC,&err);//延时5ms
	}
}
void Memory_Usage()
{
	OS_ERR err;
	u8 paddr[20];				    		//存放P Addr:+p地址的ASCII值
	u16 memused=0;
	CPU_STK_SIZE myfree,myused;
	memused=my_mem_perused(SRAMIN);
	sprintf((char*)paddr,"%d.%01d%%",memused/10,memused%10);
	printf("内部内存使用率：%s\r\n",paddr);	//显示内部内存使用率
	
	
	memused=my_mem_perused(SRAMEX);
	sprintf((char*)paddr,"%d.%01d%%",memused/10,memused%10);
	printf("外部内存使用率：%s\r\n",paddr);	//显示外部内存使用率
	
	OSTaskStkChk(&EmwindemoTaskTCB,&myfree,&myused,&err);
	printf("EmwindemoTaskTCB has free:%d, Usage:%%%d\r\n",myfree,(myused*100)/(myfree+myused));	//计算任务1的空闲字节数和堆栈使用率
	
	OSTaskStkChk(&TouchTaskTCB,&myfree,&myused,&err);
	printf("TouchTaskTCB has free:%d, Usage:%%%d\r\n",myfree,(myused*100)/(myfree+myused));	//计算任务1的空闲字节数和堆栈使用率
	
	OSTaskStkChk(&Led0TaskTCB,&myfree,&myused,&err);
	printf("Led0TaskTCB has free:%d, Usage:%%%d\r\n",myfree,(myused*100)/(myfree+myused));	//计算任务1的空闲字节数和堆栈使用率
	
	OSTaskStkChk(&KeyTaskTCB,&myfree,&myused,&err);
	printf("KeyTaskTCB has free:%d, Usage:%%%d\r\n",myfree,(myused*100)/(myfree+myused));	//计算任务1的空闲字节数和堆栈使用率
	
	OSTaskStkChk(&USB_TaskTCB,&myfree,&myused,&err);
	printf("USB_TaskTCB has free:%d, Usage:%%%d\r\n",myfree,(myused*100)/(myfree+myused));	//计算任务1的空闲字节数和堆栈使用率

}

//LED0任务
void led0_task(void *p_arg)
{
	OS_ERR err;
	u8 time=0;	
	while(1)
	{
		
		LED0_Toggle;
		time++;
		OSTimeDlyHMSM(0,0,0,500,OS_OPT_TIME_PERIODIC,&err);//延时500ms
		if(time == 10)//5秒输出一次内存使用率
		{
			Memory_Usage();
			time = 0;
		}
	}
}
//USBH_Process的主要处理任务
void USB_task(void *pdata)
{
    usb_app_main();
}
//该任务的主要功能就是处理其他任务丢给USB的读写指令
void USB_WR_task(void *p_arg)
{
	usb_app_wr();
}

