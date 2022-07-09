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

}WiFi_config;

#define MAX_WIFI_NUM 10
typedef struct
{
	char* SSID[MAX_WIFI_NUM];//WIFI 名称数组 最多处理10个wifi
	char* RSSI[MAX_WIFI_NUM];// 信号强度数组
	unsigned char number;//检索到的有多少个wifi
}SSID_SCAN_TABLE;
typedef struct
{
	uint32_t scan:1;//打开扫描
	uint32_t close:1;//关闭wifi
	uint32_t SelectWifi:5;//0 表示没有选择 非零表示选择列表中某个wifi
	
	uint32_t ConnectServer:2;//0 表示无操作 1表示连接服务器，2表示断开服务器连接
	
	uint32_t WaitForWifi:1;//等待wifi连接
}WIFI_FLAG_STRUCT;
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
void WIFI_Flag_Handle(void);
//用户配置参数
extern WiFi_config WiFiConfig;
extern WIFI_FLAG_STRUCT WIFIFlag;
extern SSID_SCAN_TABLE SSIDTable;
#endif





