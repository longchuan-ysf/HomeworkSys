#include "common.h"
#include "stdlib.h"
/////////////////////////////////////////////////////////////////////////////////////////////////////////// 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32������
//ATK-ESP8266 WIFIģ�� WIFI STA��������	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//�޸�����:2015/4/3
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2009-2019
//All rights reserved									  
/////////////////////////////////////////////////////////////////////////////////////////////////////////// 

//ATK-ESP8266 WIFI STA����
//���ڲ���TCP/UDP����
//����ֵ:0,����
//    ����,�������
u8 atk_8266_wifista_test(void)
{
	static u8 netpro=0;	
	OS_ERR err;
	u8 key;
	u8 *p;
	u16 t=2999;		//���ٵ�һ�λ�ȡ����״̬
	u8 res=0;
	u16 rlen=0;
	u8 constate=0;	//����״̬
	p=mymalloc(SRAMIN,32);							//����32�ֽ��ڴ�
	atk_8266_send_cmd("AT+CWMODE=1","OK",50);		//����WIFI STAģʽ


	//�������ӵ���WIFI��������/���ܷ�ʽ/����,�⼸��������Ҫ�������Լ���·�������ý����޸�!! 
	sprintf((char*)p,"AT+CWJAP=\"%s\",\"%s\"",WiFiConfig.wifista_ssid,WiFiConfig.wifista_password);//�������߲���:ssid,����
	while(atk_8266_send_cmd(p,"WIFI GOT IP",500));					//����Ŀ��·����,���һ��IP
PRESTA:
	printf("ATK-ESP WIFI-STA ����\r\n"); 
	printf("��������ATK-ESPģ��,���Ե�...\r\n");
	atk_8266_send_cmd("AT+CIPMUX=0","OK",500);   //0�������ӣ�1��������
	sprintf((char*)p,"AT+CIPSTART=\"TCP\",\"%s\",%s",WiFiConfig.severip,(u8*)WiFiConfig.port);    //����Ŀ��TCP������
	while(atk_8266_send_cmd(p,"OK",1000))
	{

			printf("WK_UP:������ѡ\r\n");
			printf("ATK-ESP ����TCPʧ��\r\n"); //����ʧ��	 
			key=KEY_Scan(0);
			if(key==WKUP_PRES)goto PRESTA;
	}	
	atk_8266_send_cmd("AT+CIPMODE=1","OK",500);      //����ģʽΪ��͸��			
	Usart3Data.USART3_RX_STA=0;
	while(1)
	{
		key=KEY_Scan(0);
		if(key==WKUP_PRES)			//WK_UP �˳�����		 
		{ 
//			res=0;					
//			atk_8266_quit_trans();	//�˳�͸��
//			atk_8266_send_cmd("AT+CIPMODE=0","OK",20);   //�ر�͸��ģʽ
//			break;
//            if(!netpro)
//            {
//			    Usart3Data_Buff_Change(USART3_DATA_BUFF_LEN,USART3_CMD_BUFF_LEN,100);
//            }
//            else
//            {
//                Usart3Data_Buff_Change(USART3_CMD_BUFF_LEN,USART3_CMD_BUFF_LEN,100);
//            }
//            netpro = ~netpro;
		}
		else if(key==KEY0_PRES)	//KEY0 �������� 
		{
			atk_8266_quit_trans();
			atk_8266_send_cmd("AT+CIPSEND","OK",20);         //��ʼ͸��           
			sprintf((char*)p,"ATK-8266 TCP�ͻ��� ����%d\r\n",t/10);//��������
			OSMutexPend (&Usart3Data_TX_MUTEX,0,OS_OPT_PEND_BLOCKING,0,&err);//���󻥳��ź���
        	u3_printf("%s\r\n",p);	//��������
        	OSMutexPost(&Usart3Data_TX_MUTEX,OS_OPT_POST_NONE,&err);//�ͷŻ����ź���

		}	
		t++;
		//delay_ms(10);
		OSTimeDlyHMSM(0,0,0,10,OS_OPT_TIME_PERIODIC,&err);//��ʱ10ms
		if(Usart3Data.USART3_RX_STA&RECEIVE_OK_MARK_U3)		//���յ�һ��������
		{ 
			rlen=Usart3Data.USART3_RX_STA&RECEIVE_LEN_MARK_U3;	//�õ����ν��յ������ݳ���
			Usart3Data.USART3_RX_BUF[rlen]=0;		//��ӽ����� 
			printf("%s",Usart3Data.USART3_RX_BUF);	//���͵�����   
			Usart3Data.USART3_RX_STA=0;
			if(constate!='+')t=1000;		//״̬Ϊ��δ����,������������״̬
			else t=0;                   //״̬Ϊ�Ѿ�������,10����ټ��
		}  
		if(t==3000)//����3����û���յ��κ�����,��������ǲ��ǻ�����.
		{
			constate=atk_8266_consta_check();//�õ�����״̬
			if(constate=='+')
			{
				printf("���ӳɹ�\r\n");  //����״̬
			}
			else 
			{
				printf("����ʧ��\r\n"); 
			}				
			t=0;
		}

	}
	myfree(SRAMIN,p);		//�ͷ��ڴ� 
	return res;		
} 




























