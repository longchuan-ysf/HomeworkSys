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
#if SYSTEM_SUPPORT_OS 	
	#include "includes.h"
#endif
USBH_HandleTypeDef hUSBHost;

static u8 conect;



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
				conect=0;				
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
					conect=1;
					mf_scan_files("3:");
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
	conect = 0;
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
