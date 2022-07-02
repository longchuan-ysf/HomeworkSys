#include "ftl.h"  
#include "ff.h"
#include "exfuns.h"
#include "usbh_core.h"
#include "usbh_msc.h"
#include "diskio.h"
#include "sys.h"
#include "delay.h"
#include "usbh_app.h"
#include "fattester.h"
#include "malloc.h"
#if SYSTEM_SUPPORT_OS 	
	#include "includes.h"
#endif
USBH_HandleTypeDef hUSBHost;

uint8_t connect_usb;

////////////////////////消息队列//////////////////////////////

#define USB_WR_Q_NUM	3	//发送数据的消息队列的数量
OS_Q USB_WR_Msg;		//定义一个消息队列，接受是否需要读或者写USB
/*
外部读流程
->USBH_Malloc_CtrlStruct 申请读写控制结构体
->USBH_Malloc_Path 申请内存填写读取路径
->USBH_ApplyFor_WR 请求USB读取
->USBH_WR_MsgFree 清空申请的内存

写流程
->USBH_Malloc_CtrlStruct 申请读写控制结构体
->USBH_Malloc_Path 申请内存填写读取路径
->USBH_Malloc_WriteBuf 申请内存保存要写的数据 
->USBH_ApplyFor_WR 请求USB写
->USBH_WR_MsgFree 清空申请的内存
*/

 /**
****************************************************************************************
@brief:    USBH_Malloc_CtrlStruct 申请内存以控制读写结构体
@Input：    NULL
@Output：   NULL
@Return：   申请到的内存指针
@Warning:   NULL   
@note:    uint16_t的长度一般够用了
****************************************************************************************
 **/
pUSBH_WR_MSG USBH_Malloc_CtrlStruct(void)
{ 
	pUSBH_WR_MSG pUSBwrMsg = mymalloc(SRAMIN,sizeof(pUSBH_WR_MSG));
	if(!pUSBwrMsg)
	{
		printf("%s malloc for pUSBwrMsg err!\r\n",__func__);
		return 	NULL;
	}
	
	return pUSBwrMsg;
}

 /**
****************************************************************************************
@brief:    USBH_Malloc_Path 申请内存以填写路径
@Input：    size 路径字符串所占字符串长度
			pUSBwrMsg 读写控制结构体
@Output：   NULL
@Return：   申请到的内存指针
@Warning:   NULL   
@note:    uint16_t的长度一般够用了
****************************************************************************************
 **/
void USBH_Malloc_Path(pUSBH_WR_MSG pUSBwrMsg,uint16_t size)
{ 
	uint8_t *p=mymalloc(SRAMIN,size);
	mymemset(p,0,size);
	pUSBwrMsg->path = p;
	return ;
}
 /**
****************************************************************************************
@brief:    USBH_Malloc_WriteBuf 申请内存以保存要写的内容
@Input：    size 数据大小
			pUSBwrMsg 读写控制结构体
			ExBuff 外部数据缓存区的指针
@Output：   NULL
@Return：   申请到的内存指针
@Warning:   NULL   
@note:    
****************************************************************************************
 **/
void USBH_Malloc_WriteBuf(pUSBH_WR_MSG pUSBwrMsg,uint32_t size,uint8_t *ExBuff)
{
	if(size == WR_BUFF_EX)
	{
		pUSBwrMsg->WRbufCtrl = WR_BUFF_EX;
		pUSBwrMsg->data = ExBuff;
	}
	else
	{
		uint8_t *p=mymalloc(SRAMEX,size);
		mymemset(p,0,size);
		pUSBwrMsg->WRbufCtrl = WR_BUFF_USB;
		pUSBwrMsg->data = p;
	}
	
	return ;
}
 /**
****************************************************************************************
@brief:    USBH_WR_MsgFree 释放读写过程中申请的内存
@Input：    pUSBwrMsg 读或者写控制结构体指针
			IsClearDateBuf 是否清除数据
@Output：   NULL
@Return：   NULL
@Warning:   NULL   
@note:    
****************************************************************************************
 **/
void USBH_WR_MsgFree(pUSBH_WR_MSG pUSBwrMsg)
{
	//清空填写路径申请的内存
	myfree(SRAMIN,pUSBwrMsg->path);
	
	//如果写buf区是由USB模块申请的才需要释放
	if(pUSBwrMsg->WRbufCtrl == WR_BUFF_USB)
	{
		//清空数据申请的内存
		myfree(SRAMEX,pUSBwrMsg->data);
	}
	
	//清空WR_Msg
	myfree(SRAMIN,pUSBwrMsg);
}
 /**
****************************************************************************************
@brief:    USBH_WR_MsgHandle 读写消息处理
@Input：    pUSBH_WR_MSG pUSBwrMsg消息结构体
@Output：   NULL
@Return：   NULL
@Warning:   NULL   
@note:    
****************************************************************************************
 **/
static void USBH_WR_MsgHandle(pUSBH_WR_MSG pUSBwrMsg)
{

	int bread;
	uint8_t res;
	uint32_t RWLength;
	FIL File;
	if(pUSBwrMsg->mode&FA_READ)//读操作
	{
		res = f_open(&File,(const TCHAR*)pUSBwrMsg->path,pUSBwrMsg->mode);	//打开文件
		if((res != FR_OK)) 	
		{
			pUSBwrMsg->result  = res;
			printf("read open %s failed,result = %d\r\n",pUSBwrMsg->path,pUSBwrMsg->result);
			return ;
		}
		if(pUSBwrMsg->length == RD_ALL_DATA)
		{
			RWLength = File.obj.objsize;		
		}
		else
		{
			RWLength = pUSBwrMsg->length;
		}
		printf("RWLength = %d\r\n",RWLength);
		//申请内存已保存读取数据
		pUSBwrMsg->data = mymalloc(SRAMEX,RWLength);
		if(!pUSBwrMsg->data)
		{
			printf("malloc memory for read err!\r\n");
			pUSBwrMsg->result = FR_INVALID_PARAMETER;
			f_close(&File);	//关闭文件
			return;
		}
		pUSBwrMsg->WRbufCtrl = WR_BUFF_USB;
		res = f_read(&File,pUSBwrMsg->data,RWLength,(UINT *)&bread);
		if((res != FR_OK)) 	
		{
			pUSBwrMsg->result  = res;
			printf("read %s failed,result = %d\r\n",pUSBwrMsg->path,pUSBwrMsg->result);	
			f_close(&File);	//关闭文件			
			return ;
		}
		pUSBwrMsg->bread = bread;
		res = f_close(&File);	//关闭文件
		if(res != FR_OK) 	
		{
			pUSBwrMsg->result  = res;
			printf("close %s failed,result = %d\r\n",pUSBwrMsg->path,pUSBwrMsg->result);
			return ;
		}	
		pUSBwrMsg->result  = res;
	}
	else if(pUSBwrMsg->mode&FA_WRITE)//写文件
	{
		if(!pUSBwrMsg->length)
		{
			pUSBwrMsg->result  = FR_OK;
			return;
		}
		else
		{
			RWLength = pUSBwrMsg->length;
		}
		res = f_open(&File,(const TCHAR*)pUSBwrMsg->path,pUSBwrMsg->mode);	//打开文件
		if(res != FR_OK)	
		{
			pUSBwrMsg->result  = res;
			printf("write open %s failed,result = %d\r\n",pUSBwrMsg->path,pUSBwrMsg->result);
			return ;
		}	

		res = f_write(&File,pUSBwrMsg->data,RWLength,(UINT *)&bread);
		if(res != FR_OK)	
		{
			pUSBwrMsg->result  = res;
			printf("write %s failed,result = %d\r\n",pUSBwrMsg->path,pUSBwrMsg->result);
			f_close(&File);	//关闭文件
			return ;
		}
		pUSBwrMsg->bread = bread;
		res = f_close(&File);	//关闭文件
		if(res != FR_OK)	
		{
			pUSBwrMsg->result  = res;
			printf("close %s failed,result = %d\r\n",pUSBwrMsg->path,pUSBwrMsg->result);
			return ;
		}
		pUSBwrMsg->result  = res;		
	}
	else
	{
		printf("not read or write opt=%x\r\n",pUSBwrMsg->mode);
	}
}
 /**
****************************************************************************************
@brief:    pUSBH_WR_MSG 请求USB进行一次读或者写
@Input：    pUSBwrMsg
			mode 读或者写的模式
			DataLength 数据长度 
			数据缓冲区  路径已经分配好
@Output：   NULL
@Return：   指向消息发送的结构体指针
@Warning:   NULL   
@note:    
****************************************************************************************
 **/
void USBH_ApplyFor_WR(pUSBH_WR_MSG pUSBwrMsg,uint32_t DataLength,uint8_t mode)
{
	OS_ERR err;
	pUSBwrMsg->mode = mode;
	pUSBwrMsg->length = DataLength;
	pUSBwrMsg->bread = 0;
	pUSBwrMsg->result = 0xff;
	
	//发送的消息就只是结构体指针
	OSQPost((OS_Q *		)&USB_WR_Msg,
			(void *		)pUSBwrMsg,
			(OS_MSG_SIZE)sizeof(pUSBwrMsg),
			(OS_OPT		)OS_OPT_POST_FIFO,
			(OS_ERR *	)&err);
			
	return ;
}

 /**
****************************************************************************************
@brief:    USBH_UserProcess USB用户处理流程
@Input：    phost 主机句柄
			id 事件ID
@Output：   NULL
@Return：   NULL
@Warning:   NULL   
@note:    
****************************************************************************************
 **/
extern void test_ff(char* scan_dir,char* choose_file);
extern void test_ff_free(void);
static void USBH_UserProcess(USBH_HandleTypeDef * phost, uint8_t id)
{
    u32 total,free;
	u8 res=0;
    switch (id)
    {
        case HOST_USER_SELECT_CONFIGURATION:
            break;
        case HOST_USER_DISCONNECTION:
			{
				f_mount(0,"3:",1); 	//卸载U盘   
				printf("%s\r\n","设备连接中..."); 
				connect_usb=0;	
				test_ff_free();				
			}			
            break;
        case HOST_USER_CLASS_ACTIVE:
			{
				printf("%s\r\n","设备连接成功!");	
				f_mount(fs[3],"3:",1); 	//重新挂载U盘
				res=exf_getfree("3:",&total,&free);
				if(res==0)
				{
					printf("%s\r\n","FATFS OK!");	
					printf("U Disk Total Size: %d  MB\r\n",total>>10);	 
					printf("U Disk  Free Size:  %d   MB\r\n",free>>10); 	    
					connect_usb=1;
					mf_scan_files("3:");
					test_ff("3:/download","jpg");
				}
				else
				{
					printf("U盘存储空间获取失败\r\n");
				}
			}
            break;
        case HOST_USER_CONNECTION:
            break;
        default:
            break;
    }
	
}

void usb_app_main(void)
{
	  
 	OS_ERR err;
	connect_usb = 0;
	printf("Init USB\r\n");
    USBH_Init(&hUSBHost, USBH_UserProcess, 0);
    USBH_RegisterClass(&hUSBHost, USBH_MSC_CLASS);
    USBH_Start(&hUSBHost);
    HAL_PWREx_EnableUSBVoltageDetector();
	printf("Init USB ok\r\n");
    while (1)
    {
        USBH_Process(&hUSBHost);
		OSTimeDlyHMSM(0,0,0,10,OS_OPT_TIME_PERIODIC,&err);//延时10ms
    }	 
}

void usb_app_wr(void)
{  
 	OS_ERR err;
	OS_MSG_SIZE size;
	pUSBH_WR_MSG pUSBwrMsg;
	 //创建消息队列DATA_Msg
    OSQCreate ((OS_Q *		)&USB_WR_Msg,
               (CPU_CHAR *	)"USB WR Msgg",
               (OS_MSG_QTY	)USB_WR_Q_NUM,
               (OS_ERR *	)&err);
    while (1)
    {
		if(connect_usb)
		{
			//请求消息KEY_Msg
			pUSBwrMsg = (pUSBH_WR_MSG)OSQPend((OS_Q *			)&USB_WR_Msg,
											  (OS_TICK		)0,
											  (OS_OPT			)OS_OPT_PEND_BLOCKING,
											  (OS_MSG_SIZE *	)&size,
											  (CPU_TS *		)0,
											  (OS_ERR *		)&err);
			printf("receive usb wr cmd size=%d\r\n",size);
			USBH_WR_MsgHandle(pUSBwrMsg);
			
		}     
		OSTimeDlyHMSM(0,0,0,10,OS_OPT_TIME_PERIODIC,&err);//延时10ms 这个延时是保证断开连接是usb_app_wr不独占CPU
    }	 
}
