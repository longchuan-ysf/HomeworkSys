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
	u8 *severip;//������ip��ַ
	u8* wifista_ssid;			//·����SSID��
    u8* wifista_encryption;	//���ܷ�ʽ
    u8* wifista_password; 	//��������

}WiFi_config;

#define MAX_WIFI_NUM 10
typedef struct
{
	char* SSID[MAX_WIFI_NUM];//WIFI �������� ��ദ��10��wifi
	char* RSSI[MAX_WIFI_NUM];// �ź�ǿ������
	unsigned char number;//���������ж��ٸ�wifi
}SSID_SCAN_TABLE;
typedef struct
{
	uint32_t scan:1;//��ɨ��
	uint32_t close:1;//�ر�wifi
	uint32_t SelectWifi:5;//0 ��ʾû��ѡ�� �����ʾѡ���б���ĳ��wifi
	
	uint32_t ConnectServer:2;//0 ��ʾ�޲��� 1��ʾ���ӷ�������2��ʾ�Ͽ�����������
	
	uint32_t WaitForWifi:1;//�ȴ�wifi����
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

u8 atk_8266_wifista_test(void);	//WIFI STA����

void atk_8266_scan(void);
void WIFI_Flag_Handle(void);
//�û����ò���
extern WiFi_config WiFiConfig;
extern WIFI_FLAG_STRUCT WIFIFlag;
extern SSID_SCAN_TABLE SSIDTable;

extern uint32_t ResponeLen;

#endif





