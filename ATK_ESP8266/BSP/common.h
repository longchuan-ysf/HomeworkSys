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
	//WIFI STAģʽ
	u8* port;//�˿ں�
	u8 severip[15];//������ip��ַ
	u8* wifista_ssid;			//·����SSID��
    u8* wifista_encryption;	//���ܷ�ʽ
    u8* wifista_password; 	//��������

	//WIFI APģʽ,ģ���������߲���,�������޸�.
	 u8* wifiap_ssid;			//����SSID��
	 u8* wifiap_encryption;	//wpa/wpa2 aes���ܷ�ʽ
	 u8* wifiap_password; 		//�������� 
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

u8 atk_8266_wifista_test(void);	//WIFI STA����


//�û����ò���
extern WiFi_config WiFiConfig;


#endif





