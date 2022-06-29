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

////////////////////////��Ϣ����//////////////////////////////

#define USB_WR_Q_NUM	3	//�������ݵ���Ϣ���е�����
OS_Q USB_WR_Msg;		//����һ����Ϣ���У������Ƿ���Ҫ������дUSB
/*
�ⲿ������
->USBH_Malloc_CtrlStruct �����д���ƽṹ��
->USBH_Malloc_Path �����ڴ���д��ȡ·��
->USBH_ApplyFor_WR ����USB��ȡ
->USBH_WR_MsgFree ���������ڴ�

д����
->USBH_Malloc_CtrlStruct �����д���ƽṹ��
->USBH_Malloc_Path �����ڴ���д��ȡ·��
->USBH_Malloc_WriteBuf �����ڴ汣��Ҫд������ 
->USBH_ApplyFor_WR ����USBд
->USBH_WR_MsgFree ���������ڴ�
*/

 /**
****************************************************************************************
@brief:    USBH_Malloc_CtrlStruct �����ڴ��Կ��ƶ�д�ṹ��
@Input��    NULL
@Output��   NULL
@Return��   ���뵽���ڴ�ָ��
@Warning:   NULL   
@note:    uint16_t�ĳ���һ�㹻����
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
@brief:    USBH_Malloc_Path �����ڴ�����д·��
@Input��    size ·���ַ�����ռ�ַ�������
			pUSBwrMsg ��д���ƽṹ��
@Output��   NULL
@Return��   ���뵽���ڴ�ָ��
@Warning:   NULL   
@note:    uint16_t�ĳ���һ�㹻����
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
@brief:    USBH_Malloc_WriteBuf �����ڴ��Ա���Ҫд������
@Input��    size ���ݴ�С
			pUSBwrMsg ��д���ƽṹ��
			ExBuff �ⲿ���ݻ�������ָ��
@Output��   NULL
@Return��   ���뵽���ڴ�ָ��
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
@brief:    USBH_WR_MsgFree �ͷŶ�д������������ڴ�
@Input��    pUSBwrMsg ������д���ƽṹ��ָ��
			IsClearDateBuf �Ƿ��������
@Output��   NULL
@Return��   NULL
@Warning:   NULL   
@note:    
****************************************************************************************
 **/
void USBH_WR_MsgFree(pUSBH_WR_MSG pUSBwrMsg)
{
	//�����д·��������ڴ�
	myfree(SRAMIN,pUSBwrMsg->path);
	
	//���дbuf������USBģ������Ĳ���Ҫ�ͷ�
	if(pUSBwrMsg->WRbufCtrl == WR_BUFF_USB)
	{
		//�������������ڴ�
		myfree(SRAMEX,pUSBwrMsg->data);
	}
	
	//���WR_Msg
	myfree(SRAMIN,pUSBwrMsg);
}
 /**
****************************************************************************************
@brief:    USBH_WR_MsgHandle ��д��Ϣ����
@Input��    pUSBH_WR_MSG pUSBwrMsg��Ϣ�ṹ��
@Output��   NULL
@Return��   NULL
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
	if(pUSBwrMsg->mode&FA_READ)//������
	{
		res = f_open(&File,(const TCHAR*)pUSBwrMsg->path,pUSBwrMsg->mode);	//���ļ�
		if((res != FR_OK)) 	
		{
			pUSBwrMsg->result  = res;
			printf("open failed,result = %d\r\n",pUSBwrMsg->result);
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
		//�����ڴ��ѱ����ȡ����
		pUSBwrMsg->data = mymalloc(SRAMEX,RWLength);
		if(!pUSBwrMsg->data)
		{
			printf("malloc memory for read err!\r\n");
			pUSBwrMsg->result = FR_INVALID_PARAMETER;
			return;
		}
		pUSBwrMsg->WRbufCtrl = WR_BUFF_USB;
		res = f_read(&File,pUSBwrMsg->data,RWLength,(UINT *)&bread);
		if((res != FR_OK)) 	
		{
			pUSBwrMsg->result  = res;
			printf("read failed,result = %d\r\n",pUSBwrMsg->result);			
			return ;
		}
		pUSBwrMsg->bread = bread;
		res = f_close(&File);	//�ر��ļ�
		if(res != FR_OK) 	
		{
			pUSBwrMsg->result  = res;
			printf("close failed,result = %d\r\n",pUSBwrMsg->result);
			return ;
		}	
		pUSBwrMsg->result  = res;
	}
	else if(pUSBwrMsg->mode&FA_WRITE)//д�ļ�
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
		res = f_open(&File,(const TCHAR*)pUSBwrMsg->path,pUSBwrMsg->mode);	//���ļ�
		if(res != FR_OK)	
		{
			pUSBwrMsg->result  = res;
			printf("open failed,result = %d\r\n",pUSBwrMsg->result);
			return ;
		}	

		res = f_write(&File,pUSBwrMsg->data,RWLength,(UINT *)&bread);
		if(res != FR_OK)	
		{
			pUSBwrMsg->result  = res;
			printf("write failed,result = %d\r\n",pUSBwrMsg->result);
			return ;
		}
		pUSBwrMsg->bread = bread;
		res = f_close(&File);	//�ر��ļ�
		if(res != FR_OK)	
		{
			pUSBwrMsg->result  = res;
			printf("close failed,result = %d\r\n",pUSBwrMsg->result);
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
@brief:    pUSBH_WR_MSG ����USB����һ�ζ�����д
@Input��    pUSBwrMsg
			mode ������д��ģʽ
			DataLength ���ݳ��� 
			���ݻ�����  ·���Ѿ������
@Output��   NULL
@Return��   ָ����Ϣ���͵Ľṹ��ָ��
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
	
	//���͵���Ϣ��ֻ�ǽṹ��ָ��
	OSQPost((OS_Q *		)&USB_WR_Msg,
			(void *		)pUSBwrMsg,
			(OS_MSG_SIZE)sizeof(pUSBwrMsg),
			(OS_OPT		)OS_OPT_POST_FIFO,
			(OS_ERR *	)&err);
			
	return ;
}

 /**
****************************************************************************************
@brief:    USBH_UserProcess USB�û���������
@Input��    phost �������
			id �¼�ID
@Output��   NULL
@Return��   NULL
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
				f_mount(0,"3:",1); 	//ж��U��   
				printf("%s\r\n","�豸������..."); 
				connect_usb=0;	
				test_ff_free();				
			}			
            break;
        case HOST_USER_CLASS_ACTIVE:
			{
				printf("%s\r\n","�豸���ӳɹ�!");	
				f_mount(fs[3],"3:",1); 	//���¹���U��
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
					printf("U�̴洢�ռ��ȡʧ��\r\n");
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
		OSTimeDlyHMSM(0,0,0,10,OS_OPT_TIME_PERIODIC,&err);//��ʱ10ms
    }	 
}

void usb_app_wr(void)
{  
 	OS_ERR err;
	OS_MSG_SIZE size;
	pUSBH_WR_MSG pUSBwrMsg;
	 //������Ϣ����DATA_Msg
    OSQCreate ((OS_Q *		)&USB_WR_Msg,
               (CPU_CHAR *	)"USB WR Msgg",
               (OS_MSG_QTY	)USB_WR_Q_NUM,
               (OS_ERR *	)&err);
    while (1)
    {
		if(connect_usb)
		{
			//������ϢKEY_Msg
			pUSBwrMsg = (pUSBH_WR_MSG)OSQPend((OS_Q *			)&USB_WR_Msg,
											  (OS_TICK		)0,
											  (OS_OPT			)OS_OPT_PEND_BLOCKING,
											  (OS_MSG_SIZE *	)&size,
											  (CPU_TS *		)0,
											  (OS_ERR *		)&err);
			printf("receive usb wr cmd size=%d\r\n",size);
			USBH_WR_MsgHandle(pUSBwrMsg);
			
		}     
		OSTimeDlyHMSM(0,0,0,10,OS_OPT_TIME_PERIODIC,&err);//��ʱ10ms �����ʱ�Ǳ�֤�Ͽ�������usb_app_wr����ռCPU
    }	 
}
