#ifndef __COMMON_H__
#define __COMMON_H__	 
#include "sys.h"
#include "usart.h"	
//#include "usart3.h"		
#include "delay.h"	
#include "led.h"   	 
#include "key.h"	 	 	 	 	 
#include "lcd.h"   
#include "w25qxx.h" 	 
#include "touch.h" 	 
#include "malloc.h"
#include "string.h"    
#include "text.h"		
#include "ff.h"
#include "usart.h"
#if SYSTEM_SUPPORT_OS 	
	#include "includes.h"
#endif
typedef struct
{
	//WIFI STA模式
	u8* port;//端口号
	u8 *severip;//服务器ip地址
	u8* wifista_ssid;			//路由器SSID号
    u8* wifista_encryption;	//加密方式
    u8* wifista_password; 	//连接密码

	//WIFI AP模式,模块对外的无线参数,可自行修改.
	 u8* wifiap_ssid;			//对外SSID号
	 u8* wifiap_encryption;	//wpa/wpa2 aes加密方式
	 u8* wifiap_password; 		//连接密码 
}WiFi_config;

void atk_8266_init(void);

void atk_8266_at_response(u8 mode);
u8* atk_8266_check_cmd(u8 *str);
u8 atk_8266_apsta_check(void);
u8 atk_8266_send_data(u8 *data,u8 *ack,u16 waittime);
u8 atk_8266_send_cmd(u8 *cmd,u8 *ack,u16 waittime);
u8 atk_8266_quit_trans(void);
u8 atk_8266_consta_check(void);



void atk_8266_test(void);

u8 atk_8266_wifista_test(void);	//WIFI STA测试

void atk_8266_scan(void);
//用户配置参数
extern WiFi_config WiFiConfig;


#endif





