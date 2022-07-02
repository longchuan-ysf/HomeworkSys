#include "common.h"

#define PATH_NETCONFIG "3:/config.txt"
WiFi_config WiFiConfig;
/*************************************************
function    :��Ĭ��ֵ��ʼ��WiFi���ò���
input       :NULL
output		:NULL
return		:NULL
description :
**************************************************/
void atk_8266_default_config(void)
{
	//�˿�
	u8 ServerPort[] = "80";
	WiFiConfig.port = (u8 *)mymalloc(SRAMIN,sizeof(ServerPort));
	mymemcpy(WiFiConfig.port,ServerPort,sizeof(ServerPort));
	
	//������IP
	u8 ServerIP[] = "192.168.86.101";
	WiFiConfig.severip = (u8 *)mymalloc(SRAMIN,sizeof(ServerIP));
	mymemcpy(WiFiConfig.severip,ServerIP,sizeof(ServerIP));
	
	//WiFi����
	u8 SSID[] = "SKYWORTH_AC07";
	WiFiConfig.wifista_ssid = (u8 *)mymalloc(SRAMIN,sizeof(SSID));
	mymemcpy(WiFiConfig.wifista_ssid,SSID,sizeof(SSID));
	
	//WiFi����
	u8 Password[] = "L20134470105c";
	WiFiConfig.wifista_password = (u8 *)mymalloc(SRAMIN,sizeof(Password));
	mymemcpy(WiFiConfig.wifista_password,Password,sizeof(Password));
	
	//���ܷ�ʽ
	u8 Encryption[] = "WPA2";
	WiFiConfig.wifista_encryption = (u8 *)mymalloc(SRAMIN,sizeof(Encryption));
	mymemcpy(WiFiConfig.wifista_encryption,Encryption,sizeof(Encryption));
}
/*************************************************
function    :�������ַ����н�������ֵ
input       :ConfigStr �����ַ���
			 ConfigName ��������
			 Config  ����ֵ
output		:NULL
return		:
description :����һ��Ҫ������
			 Name1 : Value1\r\n
			 Name2 : Value2\r\n
**************************************************/
void Decode_config(uint8_t *ConfigStr,uint8_t *ConfigName,uint8_t *Config)
{
	uint8_t *start,*end;
	
	start = (uint8_t *)strstr((char *)ConfigStr,(char *)ConfigName);
	end = (uint8_t *)strstr((char *)start,"\r\n");
	start+=strlen((char *)ConfigName);
	
	mymemcpy(Config,start,end-start);
}
/*************************************************
function    :��ʼ��WiFi���ò���
input       :NULL
output		:NULL
return		:NULL
description :
**************************************************/
void atk_8266_init(void)
{
	FIL *NetConfig;
	uint32_t br;
	uint8_t *data,*temp;
	uint8_t res;
	NetConfig=mymalloc(SRAMIN,sizeof(FIL));
	memset(&WiFiConfig,0,sizeof(WiFiConfig));
	res = f_open(NetConfig,PATH_NETCONFIG,FA_READ);
	if(res)
	{
		printf("open config file err, use default config\r\n");
		atk_8266_default_config();
		return;
	}
	data = mymalloc(SRAMIN, NetConfig->obj.objsize+1);
	if(!data)
	{
		printf("malloc data err!\r\n");
		atk_8266_default_config();
		f_close(NetConfig);
		return;
	}
	mymemset(data,0,NetConfig->obj.objsize+1);
	
	temp =  mymalloc(SRAMIN, 64);
	if(!temp)
	{
		printf("malloc temp err!\r\n");
		atk_8266_default_config();
		f_close(NetConfig);
		myfree(SRAMIN,temp);
		return;
	}
	
	
	res = f_read(NetConfig,data,NetConfig->obj.objsize,&br);
	if(res)
	{
		printf("malloc err!\r\n");
		atk_8266_default_config();
		f_close(NetConfig);
		myfree(SRAMIN,data);
		myfree(SRAMIN,temp);
		return;
	}

	mymemset(temp,0,64);
	Decode_config(data,(uint8_t *)"SSID : ",temp);
	WiFiConfig.wifista_ssid = (u8 *)mymalloc(SRAMIN,strlen((char *)temp));
	strcpy((char *)WiFiConfig.wifista_ssid,(char *)temp);
	printf("ssid = %s\r\n",WiFiConfig.wifista_ssid);
	
	mymemset(temp,0,64);
	Decode_config(data,(uint8_t *)"Password : ",temp);
	WiFiConfig.wifista_password = (u8 *)mymalloc(SRAMIN,strlen((char *)temp));
	strcpy((char *)WiFiConfig.wifista_password,(char *)temp);
	printf("password = %s\r\n",WiFiConfig.wifista_password);
	
	mymemset(temp,0,64);
	Decode_config(data,(uint8_t *)"Encryption : ",temp);
	WiFiConfig.wifista_encryption = (u8 *)mymalloc(SRAMIN,strlen((char *)temp));
	strcpy((char *)WiFiConfig.wifista_encryption,(char *)temp);
	printf("encryption = %s\r\n",WiFiConfig.wifista_encryption);
	
	mymemset(temp,0,64);
	Decode_config(data,(uint8_t *)"ServerPort : ",temp);
	WiFiConfig.port = (u8 *)mymalloc(SRAMIN,strlen((char *)temp));
	strcpy((char *)WiFiConfig.port,(char *)temp);
	printf("port = %s\r\n",WiFiConfig.port);
	
	mymemset(temp,0,64);
	Decode_config(data,(uint8_t *)"ServerIP : ",temp);
	WiFiConfig.severip = (u8 *)mymalloc(SRAMIN,strlen((char *)temp));
	strcpy((char *)WiFiConfig.severip,(char *)temp);
	printf("severip = %s\r\n",WiFiConfig.severip);
	
	f_close(NetConfig);
	myfree(SRAMIN,data);
	myfree(SRAMIN,temp);
//	
	//����������ʱ������
	
}
/*************************************************
function    :�ͷų�ʼ��ʱ������ڴ�
input       :NULL
output		:NULL
return		:NULL
description :
**************************************************/
void atk_8266_uninit(void)
{
	
	//�˿�
	myfree(SRAMIN, WiFiConfig.port);
	//������IP
	myfree(SRAMIN, WiFiConfig.severip);
	//WiFi����
	myfree(SRAMIN, WiFiConfig.wifista_ssid);
	//WiFi����
	myfree(SRAMIN, WiFiConfig.wifista_password);
	//���ܷ�ʽ
	myfree(SRAMIN, WiFiConfig.wifista_encryption);
	//����������ʱ������
	memset(&WiFiConfig,0,sizeof(WiFiConfig));
}

//usmart֧�ֲ���
//���յ���ATָ��Ӧ�����ݷ��ظ����Դ���
//mode:0,������USART3_RX_STA;
//     1,����USART3_RX_STA;
void atk_8266_at_response(u8 mode)
{
	if(Usart3Data.USART3_RX_STA&RECEIVE_OK_MARK_U3)		//���յ�һ��������
	{ 
		Usart3Data.USART3_RX_BUF[Usart3Data.USART3_RX_STA&RECEIVE_LEN_MARK_U3]=0;//��ӽ�����
		printf("%s",Usart3Data.USART3_RX_BUF);	//���͵�����
		if(mode)Usart3Data.USART3_RX_STA=0;
	} 
}
//ATK-ESP8266���������,�����յ���Ӧ��
//str:�ڴ���Ӧ����
//����ֵ:0,û�еõ��ڴ���Ӧ����
//    ����,�ڴ�Ӧ������λ��(str��λ��)
u8* atk_8266_check_cmd(u8 *str)
{
	
	char *strx=0;
	if(Usart3Data.USART3_RX_STA&RECEIVE_OK_MARK_U3)		//���յ�һ��������
	{ 
		Usart3Data.USART3_RX_BUF[Usart3Data.USART3_RX_STA&RECEIVE_LEN_MARK_U3]=0;//��ӽ�����
		printf("resp:%s\r\n",Usart3Data.USART3_RX_BUF);
		strx=strstr((const char*)Usart3Data.USART3_RX_BUF,(const char*)str);
	} 
	return (u8*)strx;
}
//��ATK-ESP8266��������
//cmd:���͵������ַ���
//ack:�ڴ���Ӧ����,���Ϊ��,���ʾ����Ҫ�ȴ�Ӧ��
//waittime:�ȴ�ʱ��(��λ:10ms)
//����ֵ:0,���ͳɹ�(�õ����ڴ���Ӧ����)
//       1,����ʧ��
u8 atk_8266_send_cmd(u8 *cmd,u8 *ack,u16 waittime)
{
	u8 res=0; 
    OS_ERR err;
	Usart3Data.USART3_RX_STA=0;
	printf("cmd:%s\r\n",cmd);
    OSMutexPend (&Usart3Data_TX_MUTEX,0,OS_OPT_PEND_BLOCKING,0,&err);//���󻥳��ź���
	u3_printf("%s\r\n",cmd);	//��������
	OSMutexPost(&Usart3Data_TX_MUTEX,OS_OPT_POST_NONE,&err);//�ͷŻ����ź���
	if(ack&&waittime)		//��Ҫ�ȴ�Ӧ��
	{
		while(--waittime)	//�ȴ�����ʱ
		{
			delay_ms(10);
			if(Usart3Data.USART3_RX_STA&RECEIVE_OK_MARK_U3)//���յ��ڴ���Ӧ����
			{
				if(atk_8266_check_cmd(ack))
				{
					break;//�õ���Ч���� 
				}
					Usart3Data.USART3_RX_STA=0;
			} 
		}
		if(waittime==0)res=1; 
	}
	return res;
} 
//��ATK-ESP8266����ָ������
//data:���͵�����(����Ҫ��ӻس���)
//ack:�ڴ���Ӧ����,���Ϊ��,���ʾ����Ҫ�ȴ�Ӧ��
//waittime:�ȴ�ʱ��(��λ:10ms)
//����ֵ:0,���ͳɹ�(�õ����ڴ���Ӧ����)luojian
u8 atk_8266_send_data(u8 *data,u8 *ack,u16 waittime)
{
	u8 res=0;
    OS_ERR err;
	Usart3Data.USART3_RX_STA=0;
	OSMutexPend (&Usart3Data_TX_MUTEX,0,OS_OPT_PEND_BLOCKING,0,&err);//���󻥳��ź���
	u3_printf("%s\r\n",data);	//��������
	OSMutexPost(&Usart3Data_TX_MUTEX,OS_OPT_POST_NONE,&err);//�ͷŻ����ź���
	if(ack&&waittime)		//��Ҫ�ȴ�Ӧ��
	{
		while(--waittime)	//�ȴ�����ʱ
		{

			delay_ms(10);
			if(Usart3Data.USART3_RX_STA&RECEIVE_OK_MARK_U3)//���յ��ڴ���Ӧ����
			{
				if(atk_8266_check_cmd(ack))break;//�õ���Ч���� 
				Usart3Data.USART3_RX_STA=0;
			} 
		}
		if(waittime==0)res=1; 
	}
	return res;
}
//ATK-ESP8266�˳�͸��ģʽ
//����ֵ:0,�˳��ɹ�;
//       1,�˳�ʧ��
u8 atk_8266_quit_trans(void)
{
	while((USART3->ISR&0X40)==0);	//�ȴ����Ϳ�
	USART3->TDR='+';      
	delay_ms(15);					//���ڴ�����֡ʱ��(10ms)
	while((USART3->ISR&0X40)==0);	//�ȴ����Ϳ�
	USART3->TDR='+';      
	delay_ms(15);					//���ڴ�����֡ʱ��(10ms)
	while((USART3->ISR&0X40)==0);	//�ȴ����Ϳ�
	USART3->TDR='+';      
	delay_ms(500);					//�ȴ�500ms
	return atk_8266_send_cmd("AT","OK",20);//�˳�͸���ж�.
}
//��ȡATK-ESP8266ģ���AP+STA����״̬
//����ֵ:0��δ����;1,���ӳɹ�
u8 atk_8266_apsta_check(void)
{
	if(atk_8266_quit_trans())return 0;			//�˳�͸�� 
	atk_8266_send_cmd("AT+CIPSTATUS",":",50);	//����AT+CIPSTATUSָ��,��ѯ����״̬
	if(atk_8266_check_cmd("+CIPSTATUS:0")&&
		 atk_8266_check_cmd("+CIPSTATUS:1")&&
		 atk_8266_check_cmd("+CIPSTATUS:2")&&
		 atk_8266_check_cmd("+CIPSTATUS:4"))
		return 0;
	else return 1;
}
//��ȡATK-ESP8266ģ�������״̬
//����ֵ:0,δ����;1,���ӳɹ�.
u8 atk_8266_consta_check(void)
{
	u8 *p;
	u8 res;
	if(atk_8266_quit_trans())return 0;			//�˳�͸�� 
	atk_8266_send_cmd("AT+CIPSTATUS",":",50);	//����AT+CIPSTATUSָ��,��ѯ����״̬
	p=atk_8266_check_cmd("+CIPSTATUS:"); 
	res=*p;									//�õ�����״̬	
	return res;
}



//��ȡClient ip��ַ
//ipbuf:ip��ַ���������
void atk_8266_get_wanip(u8* ipbuf)
{
	u8 *p,*p1;
	if(atk_8266_send_cmd("AT+CIFSR","OK",100))//��ȡWAN IP��ַʧ��
	{
		ipbuf[0]=0;
		return;
	}		
	p=atk_8266_check_cmd("\"");
	p1=(u8*)strstr((const char*)(p+1),"\"");
	*p1=0;
	sprintf((char*)ipbuf,"%s",p+1);	
}



//ATK-ESP8266ģ����Լ��WiFiģ��
void atk_8266_test(void)
{
 
	while(atk_8266_send_cmd("AT","OK",100))//���WIFIģ���Ƿ�����
	{
		atk_8266_quit_trans();//�˳�͸��
		atk_8266_send_cmd("AT+CIPMODE=0","OK",200);  //�ر�͸��ģʽ	
		printf("δ��⵽ģ��!!!\r\n");
		delay_ms(800);
		printf("��������ģ��...\r\n"); 
	} 
	while(atk_8266_send_cmd("ATE0","OK",200));//�رջ���
	
	//atk_8266_wifista_test();	
}

void atk_8266_scan(void)
{
	
	while(atk_8266_send_cmd("AT+CWLAP","OK",200));
	
}

















































