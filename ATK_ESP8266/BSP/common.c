#include "common.h"
#include "HomeworkGUI.h"
#include "delay.h"
#include "background.h"
#include "keypad.h"
#define PATH_NETCONFIG "3:/config.txt"
WiFi_config WiFiConfig;
WIFI_FLAG_STRUCT WIFIFlag;
SSID_SCAN_TABLE SSIDTable;
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
	if(!start)
	{
		printf("not have config:%s\r\n",ConfigName);
		return;
	}
	end = (uint8_t *)strstr((char *)start,"\r\n");
	if(!end)
	{
		printf("format err s\r\n");
		return;
	}
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
	printf("read data;\r\n%s",data);
	mymemset(temp,0,64);
	Decode_config(data,(uint8_t *)"SSID : ",temp);
	WiFiConfig.wifista_ssid = (u8 *)mymalloc(SRAMIN,strlen((char *)temp));
	strcpy((char *)WiFiConfig.wifista_ssid,(char *)temp);
	printf("Decode ssid = %s\r\n",WiFiConfig.wifista_ssid);
	
	mymemset(temp,0,64);
	Decode_config(data,(uint8_t *)"Password : ",temp);
	WiFiConfig.wifista_password = (u8 *)mymalloc(SRAMIN,strlen((char *)temp));
	strcpy((char *)WiFiConfig.wifista_password,(char *)temp);
	printf("Decode password = %s\r\n",WiFiConfig.wifista_password);
	
	mymemset(temp,0,64);
	Decode_config(data,(uint8_t *)"Encryption : ",temp);
	WiFiConfig.wifista_encryption = (u8 *)mymalloc(SRAMIN,strlen((char *)temp));
	strcpy((char *)WiFiConfig.wifista_encryption,(char *)temp);
	printf("Decode encryption = %s\r\n",WiFiConfig.wifista_encryption);
	
	mymemset(temp,0,64);
	Decode_config(data,(uint8_t *)"ServerPort : ",temp);
	WiFiConfig.port = (u8 *)mymalloc(SRAMIN,strlen((char *)temp));
	strcpy((char *)WiFiConfig.port,(char *)temp);
	printf("Decode port = %s\r\n",WiFiConfig.port);
	
	mymemset(temp,0,64);
	Decode_config(data,(uint8_t *)"ServerIP : ",temp);
	WiFiConfig.severip = (u8 *)mymalloc(SRAMIN,strlen((char *)temp));
	strcpy((char *)WiFiConfig.severip,(char *)temp);
	printf("Decode severip = %s\r\n",WiFiConfig.severip);
	
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
	printf("size WIFIFlag = %d\r\n",sizeof(WIFIFlag));
	mymemset(&WIFIFlag,0,sizeof(WIFIFlag));
}
/*************************************************
function    :ListWifi
input       :Respon �յ�����Ӧ�ַ���
output		:NULL
return		:
description :
**************************************************/
void ListWifi(void)
{
	uint8_t i;

	for (i = 0; i < SSIDTable.number; i++)
	{
		printf("ssid:%s  erri;%s\r\n", SSIDTable.SSID[i], SSIDTable.RSSI[i]);
		LISTBOX_AddString(WM_WIFIList,SSIDTable.SSID[i]);
		delay_ms(10);
	}
}

/*************************************************
function    :SSID_Scan_Decode 
input       :Respon �յ�����Ӧ�ַ���
output		:NULL
return		:
description :����wifiɨ����Ӧ
**************************************************/
void SSID_Scan_Decode(uint8_t *Respon)
{
	char* start, * starttemp, * end, * endtemp, * temp;
	int len, OneLineLen;
	len = strlen((char *)Respon);
	SSIDTable.number = 0;
	temp = (char*)mymalloc(SRAMIN,32);
	if (!temp)
	{
		return;
	}
	start = (char*)Respon;
	while (len)
	{
		start = (char*)strstr(start, "+CWLAP");
		if (!start)
		{
			printf("find %d wifi\r\n", SSIDTable.number);
			break;
		}
		end = strstr(start, "\r\n");
		end += 2;
		OneLineLen = end - start;
		len -= OneLineLen;
		starttemp = start;
		starttemp += 11;//���� +CWLAP:(4,"	
		mymemset(temp, 0, 32);
		endtemp = strstr(starttemp, "\"");

		//wifi���Ʋ�Ϊ��
		if (endtemp-starttemp)//
		{
			//wifi����
			mymemcpy(temp, starttemp, endtemp - starttemp);
			SSIDTable.SSID[SSIDTable.number] = (char*)mymalloc(SRAMIN,strlen(temp));
			strcpy(SSIDTable.SSID[SSIDTable.number], temp);
			//wifiǿ��
			memset(temp, 0, 32);
			starttemp = endtemp + 2;
			endtemp = strstr(starttemp, ",");
			mymemcpy(temp, starttemp, endtemp - starttemp);
			SSIDTable.RSSI[SSIDTable.number] = (char*)mymalloc(SRAMIN,strlen(temp));
			strcpy(SSIDTable.RSSI[SSIDTable.number], temp);
			SSIDTable.number++;
			if (SSIDTable.number > MAX_WIFI_NUM)
			{
				break;
			}
		}
		start = end;

	}
	ListWifi();
	myfree(SRAMIN,temp);
	
}
/*************************************************
function    :atk_8266_scan 
input       :NULL
output		:NULL
return		:
description :ɨ����Χ����wifi
**************************************************/
void atk_8266_scan(void)
{
	uint8_t *Respon;
	while(atk_8266_send_cmd("AT+CWLAP","OK",200));
	printf("cmd ok,decode\r\n");
	Respon = mymalloc(SRAMEX,(Usart3Data.USART3_RX_STA&RECEIVE_LEN_MARK_U3) + 1);
	if(!Respon)
		return;
	mymemset(Respon,0,(Usart3Data.USART3_RX_STA&RECEIVE_LEN_MARK_U3) + 1);
	if(!Respon)
	{
		printf("malloc for ssid scan respon err!");
		return;
	}
	mymemcpy(Respon,Usart3Data.USART3_RX_BUF,Usart3Data.USART3_RX_STA&RECEIVE_LEN_MARK_U3);

	SSID_Scan_Decode(Respon);
	printf("decode ok\r\n");
	
	myfree(SRAMEX,Respon);
	
}

/*************************************************
function    :atk_8266_close 
input       :NULL
output		:NULL
return		:
description :�ر�wifi
**************************************************/
void atk_8266_close(void)
{
	uint8_t i;

	for( i=0;i<SSIDTable.number;i++)
	{
		
		myfree(SRAMIN,SSIDTable.SSID[i]);
		myfree(SRAMIN,SSIDTable.RSSI[i]);

	}
	SSIDTable.number=0;
		
	while(atk_8266_send_cmd("AT+CWQAP","OK",200));//�ر�WiFi
	while(LISTBOX_GetNumItems(WM_WIFIList))
	{
		LISTBOX_DeleteItem(WM_WIFIList,0);
		delay_ms(10);
	}
	LISTBOX_DeleteItem(WM_WIFIConnect,0);
	LISTBOX_AddString(WM_WIFIConnect,"WIFIδ����");
	LISTBOX_AddString(WM_WIFIList,"���WIFI����");
}
/*************************************************
function    :Save_New_Wifi 
input       :NULL
output		:NULL
return		:
description :
**************************************************/
void Save_New_Wifi(void)
{
	FIL *NetConfig;
	uint32_t br;
	uint8_t *data;
	uint8_t res;
	
	NetConfig=mymalloc(SRAMIN,sizeof(FIL));
	
	res = f_open(NetConfig,PATH_NETCONFIG,FA_WRITE);
	if(res)
	{
		printf("open config file err, use default config\r\n");
		return;
	}
	data = mymalloc(SRAMIN,128);
	if(!data)
	{
		printf("malloc err\r\n");
		return;
	}
	mymemset(data,0,128);
	sprintf((char *)data,"SSID : %s\r\n",WiFiConfig.wifista_ssid);
	res = f_write(NetConfig,data,strlen((char *)data),&br);
	
	mymemset(data,0,128);
	sprintf((char *)data,"Password : %s\r\n",WiFiConfig.wifista_password);
	res = f_write(NetConfig,data,strlen((char *)data),&br);
	
	mymemset(data,0,128);
	sprintf((char *)data,"Encryption : %s\r\n",WiFiConfig.wifista_encryption);
	res = f_write(NetConfig,data,strlen((char *)data),&br);
	
	mymemset(data,0,128);
	sprintf((char *)data,"ServerPort : %s\r\n",WiFiConfig.port);
	res = f_write(NetConfig,data,strlen((char *)data),&br);
	
	mymemset(data,0,128);
	sprintf((char *)data,"ServerIP : %s\r\n",WiFiConfig.severip);
	res = f_write(NetConfig,data,strlen((char *)data),&br);
	
	f_close(NetConfig);
	
	myfree(SRAMIN,data);

//	
	//����������ʱ������
}
/*************************************************
function    :SelectWiFiHandle 
input       :NULL
output		:NULL
return		:
description :ѡ��wifi֮��Ĵ���
**************************************************/
void SelectWiFiHandle(uint8_t index)
{
	int res; 
	index=index-1;//����0��ʼ������
	
	res = strcmp((char *)WiFiConfig.wifista_ssid,(char *)SSIDTable.SSID[index]);
	if(!res)//�洢���м�¼
	{
		printf("connect record wifi, SSID=%s\r\n",SSIDTable.SSID[index]);
		u8 *p=mymalloc(SRAMIN,32);	
		atk_8266_send_cmd("AT+CWMODE=1","OK",50);		//����WIFI STAģʽ

		//�������ӵ���WIFI��������/���ܷ�ʽ/����,�⼸��������Ҫ�������Լ���·�������ý����޸�!! 
		sprintf((char*)p,"AT+CWJAP=\"%s\",\"%s\"",WiFiConfig.wifista_ssid,WiFiConfig.wifista_password);//�������߲���:ssid,����
		while(atk_8266_send_cmd(p,"WIFI GOT IP",1000));	//����Ŀ��·����,���һ��IP
		BackGroundCtrl.ConnectState = 1;
		myfree(SRAMIN,p);
		
		LISTBOX_DeleteItem(WM_WIFIConnect,0);
		LISTBOX_DeleteItem(WM_WIFIList,index);
		LISTBOX_AddString(WM_WIFIConnect,SSIDTable.SSID[index]);
	}
	else//�洢���޼�¼��Ҫ������
	{
		printf("connect new wifi, SSID=%s\r\n",SSIDTable.SSID[index]);
		keypad_dev.Finish =0;
		DisplayDialogMsg.x0	=80;
		DisplayDialogMsg.y0	= 80;
		DisplayDialogMsg.xSize = 300;
		DisplayDialogMsg.ySize = 200;
		DisplayDialogMsg.DialogTiltle= "��ʾ";
		DisplayDialogMsg.Editname = "������WiFi����";
		DisplayDialogMsg.hFrame=CreatDispalyDialog(DialogSelectWiFi,&DisplayDialogMsg);
		WM_SetFocus(WM_GetDialogItem(DisplayDialogMsg.hFrame, GUI_ID_EDIT9));
		while(!keypad_dev.Finish)
		{
			delay_ms(500);
		}
		printf("FinalData = %s\r\n",keypad_dev.FinalData);
		u8 *p=mymalloc(SRAMIN,32);	
		atk_8266_send_cmd("AT+CWMODE=1","OK",50);		//����WIFI STAģʽ

		//�������ӵ���WIFI��������/���ܷ�ʽ/����,�⼸��������Ҫ�������Լ���·�������ý����޸�!! 
		sprintf((char*)p,"AT+CWJAP=\"%s\",\"%s\"",SSIDTable.SSID[index],keypad_dev.FinalData);//�������߲���:ssid,����
		while(atk_8266_send_cmd(p,"WIFI GOT IP",1000));	//����Ŀ��·����,���һ��IP
		BackGroundCtrl.ConnectState = 1;
		myfree(SRAMIN,p);
		
		myfree(SRAMIN,WiFiConfig.wifista_ssid);
		WiFiConfig.wifista_ssid = mymalloc(SRAMIN,strlen(SSIDTable.SSID[index]));
		strcpy((char *)WiFiConfig.wifista_ssid,(char *)SSIDTable.SSID[index]);
		
		myfree(SRAMIN,WiFiConfig.wifista_password);
		WiFiConfig.wifista_password = mymalloc(SRAMIN,strlen((char *)keypad_dev.FinalData));
		strcpy((char *)WiFiConfig.wifista_password,(char *)keypad_dev.FinalData);
		printf("new ssdi=%s,pwd=%s\r\n",WiFiConfig.wifista_ssid,WiFiConfig.wifista_password);
		Save_New_Wifi();
		LISTBOX_DeleteItem(WM_WIFIConnect,0);
		LISTBOX_DeleteItem(WM_WIFIList,index);
		LISTBOX_AddString(WM_WIFIConnect,SSIDTable.SSID[index]);
	}
}
/*************************************************
function    :WIFI_Flag_Handle 
input       :NULL
output		:NULL
return		:
description :��GUI������һЩ��־����
**************************************************/
void WIFI_Flag_Handle(void)
{
	if(WIFIFlag.scan)
	{
		WIFIFlag.scan=0;
		
		atk_8266_scan();
		printf("scan wifi ok\r\n");
	}
	else if(WIFIFlag.close)
	{
		WIFIFlag.close = 0;
		printf("close wifi\r\n");
		atk_8266_close();
	}
	else if(WIFIFlag.SelectWifi)
	{
		printf("select %d wifi\r\n",WIFIFlag.SelectWifi);
		SelectWiFiHandle(WIFIFlag.SelectWifi);
		WIFIFlag.SelectWifi=0;	
	}
}












































