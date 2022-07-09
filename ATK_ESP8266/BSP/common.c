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
function    :用默认值初始化WiFi配置参数
input       :NULL
output		:NULL
return		:NULL
description :
**************************************************/
void atk_8266_default_config(void)
{
	//端口
	u8 ServerPort[] = "80";
	WiFiConfig.port = (u8 *)mymalloc(SRAMIN,sizeof(ServerPort));
	mymemcpy(WiFiConfig.port,ServerPort,sizeof(ServerPort));
	
	//服务器IP
	u8 ServerIP[] = "192.168.86.101";
	WiFiConfig.severip = (u8 *)mymalloc(SRAMIN,sizeof(ServerIP));
	mymemcpy(WiFiConfig.severip,ServerIP,sizeof(ServerIP));
	
	//WiFi名称
	u8 SSID[] = "SKYWORTH_AC07";
	WiFiConfig.wifista_ssid = (u8 *)mymalloc(SRAMIN,sizeof(SSID));
	mymemcpy(WiFiConfig.wifista_ssid,SSID,sizeof(SSID));
	
	//WiFi密码
	u8 Password[] = "L20134470105c";
	WiFiConfig.wifista_password = (u8 *)mymalloc(SRAMIN,sizeof(Password));
	mymemcpy(WiFiConfig.wifista_password,Password,sizeof(Password));
	
	//加密方式
	u8 Encryption[] = "WPA2";
	WiFiConfig.wifista_encryption = (u8 *)mymalloc(SRAMIN,sizeof(Encryption));
	mymemcpy(WiFiConfig.wifista_encryption,Encryption,sizeof(Encryption));
}
/*************************************************
function    :从配置字符串中解码配置值
input       :ConfigStr 配置字符串
			 ConfigName 配置名称
			 Config  配置值
output		:NULL
return		:
description :配置一定要是这样
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
function    :初始化WiFi配置参数
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

	EDIT_SetText(EDIT_ServerIP, (char *)WiFiConfig.severip);
			
	EDIT_SetText(EDIT_ServerPort, (char *)WiFiConfig.port);
}
/*************************************************
function    :释放初始化时申请的内存
input       :NULL
output		:NULL
return		:NULL
description :
**************************************************/
void atk_8266_uninit(void)
{
	
	//端口
	myfree(SRAMIN, WiFiConfig.port);
	//服务器IP
	myfree(SRAMIN, WiFiConfig.severip);
	//WiFi名称
	myfree(SRAMIN, WiFiConfig.wifista_ssid);
	//WiFi密码
	myfree(SRAMIN, WiFiConfig.wifista_password);
	//加密方式
	myfree(SRAMIN, WiFiConfig.wifista_encryption);
	//其他参数暂时不关心
	memset(&WiFiConfig,0,sizeof(WiFiConfig));
}

//usmart支持部分
//将收到的AT指令应答数据返回给电脑串口
//mode:0,不清零USART3_RX_STA;
//     1,清零USART3_RX_STA;
void atk_8266_at_response(u8 mode)
{
	if(Usart3Data.USART3_RX_STA&RECEIVE_OK_MARK_U3)		//接收到一次数据了
	{ 
		Usart3Data.USART3_RX_BUF[Usart3Data.USART3_RX_STA&RECEIVE_LEN_MARK_U3]=0;//添加结束符
		printf("%s",Usart3Data.USART3_RX_BUF);	//发送到串口
		if(mode)Usart3Data.USART3_RX_STA=0;
	} 
}
//ATK-ESP8266发送命令后,检测接收到的应答
//str:期待的应答结果
//返回值:0,没有得到期待的应答结果
//    其他,期待应答结果的位置(str的位置)
u8* atk_8266_check_cmd(u8 *str)
{
	
	char *strx=0;
	if(Usart3Data.USART3_RX_STA&RECEIVE_OK_MARK_U3)		//接收到一次数据了
	{ 
		Usart3Data.USART3_RX_BUF[Usart3Data.USART3_RX_STA&RECEIVE_LEN_MARK_U3]=0;//添加结束符
		printf("resp:%s\r\n",Usart3Data.USART3_RX_BUF);
		strx=strstr((const char*)Usart3Data.USART3_RX_BUF,(const char*)str);
	} 
	return (u8*)strx;
}
//向ATK-ESP8266发送命令
//cmd:发送的命令字符串
//ack:期待的应答结果,如果为空,则表示不需要等待应答
//waittime:等待时间(单位:10ms)
//返回值:0,发送成功(得到了期待的应答结果)
//       1,发送失败
u8 atk_8266_send_cmd(u8 *cmd,u8 *ack,u16 waittime)
{
	u8 res=0; 
    OS_ERR err;
	Usart3Data.USART3_RX_STA=0;
	printf("cmd:%s\r\n",cmd);
    OSMutexPend (&Usart3Data_TX_MUTEX,0,OS_OPT_PEND_BLOCKING,0,&err);//请求互斥信号量
	u3_printf("%s\r\n",cmd);	//发送命令
	OSMutexPost(&Usart3Data_TX_MUTEX,OS_OPT_POST_NONE,&err);//释放互斥信号量
	if(ack&&waittime)		//需要等待应答
	{
		while(--waittime)	//等待倒计时
		{
			delay_ms(10);
			if(Usart3Data.USART3_RX_STA&RECEIVE_OK_MARK_U3)//接收到期待的应答结果
			{
				if(atk_8266_check_cmd(ack))
				{
					break;//得到有效数据 
				}
					Usart3Data.USART3_RX_STA=0;
			} 
		}
		if(waittime==0)res=1; 
	}
	return res;
} 
//向ATK-ESP8266发送指定数据
//data:发送的数据(不需要添加回车了)
//ack:期待的应答结果,如果为空,则表示不需要等待应答
//waittime:等待时间(单位:10ms)
//返回值:0,发送成功(得到了期待的应答结果)luojian
u8 atk_8266_send_data(u8 *data,u8 *ack,u16 waittime)
{
	u8 res=0;
    OS_ERR err;
	Usart3Data.USART3_RX_STA=0;
	OSMutexPend (&Usart3Data_TX_MUTEX,0,OS_OPT_PEND_BLOCKING,0,&err);//请求互斥信号量
	u3_printf("%s\r\n",data);	//发送命令
	OSMutexPost(&Usart3Data_TX_MUTEX,OS_OPT_POST_NONE,&err);//释放互斥信号量
	if(ack&&waittime)		//需要等待应答
	{
		while(--waittime)	//等待倒计时
		{

			delay_ms(10);
			if(Usart3Data.USART3_RX_STA&RECEIVE_OK_MARK_U3)//接收到期待的应答结果
			{
				if(atk_8266_check_cmd(ack))break;//得到有效数据 
				Usart3Data.USART3_RX_STA=0;
			} 
		}
		if(waittime==0)res=1; 
	}
	return res;
}
//ATK-ESP8266退出透传模式
//返回值:0,退出成功;
//       1,退出失败
u8 atk_8266_quit_trans(void)
{
	while((USART3->ISR&0X40)==0);	//等待发送空
	USART3->TDR='+';      
	delay_ms(15);					//大于串口组帧时间(10ms)
	while((USART3->ISR&0X40)==0);	//等待发送空
	USART3->TDR='+';      
	delay_ms(15);					//大于串口组帧时间(10ms)
	while((USART3->ISR&0X40)==0);	//等待发送空
	USART3->TDR='+';      
	delay_ms(500);					//等待500ms
	return atk_8266_send_cmd("AT","OK",20);//退出透传判断.
}
//获取ATK-ESP8266模块的AP+STA连接状态
//返回值:0，未连接;1,连接成功
u8 atk_8266_apsta_check(void)
{
	if(atk_8266_quit_trans())return 0;			//退出透传 
	atk_8266_send_cmd("AT+CIPSTATUS",":",50);	//发送AT+CIPSTATUS指令,查询连接状态
	if(atk_8266_check_cmd("+CIPSTATUS:0")&&
		 atk_8266_check_cmd("+CIPSTATUS:1")&&
		 atk_8266_check_cmd("+CIPSTATUS:2")&&
		 atk_8266_check_cmd("+CIPSTATUS:4"))
		return 0;
	else return 1;
}
//获取ATK-ESP8266模块的连接状态
//返回值:0,未连接;1,连接成功.
u8 atk_8266_consta_check(void)
{
	u8 *p;
	u8 res;
	if(atk_8266_quit_trans())return 0;			//退出透传 
	atk_8266_send_cmd("AT+CIPSTATUS",":",50);	//发送AT+CIPSTATUS指令,查询连接状态
	p=atk_8266_check_cmd("+CIPSTATUS:"); 
	res=*p;									//得到连接状态	
	return res;
}



//获取Client ip地址
//ipbuf:ip地址输出缓存区
void atk_8266_get_wanip(u8* ipbuf)
{
	u8 *p,*p1;
	if(atk_8266_send_cmd("AT+CIFSR","OK",100))//获取WAN IP地址失败
	{
		ipbuf[0]=0;
		return;
	}		
	p=atk_8266_check_cmd("\"");
	p1=(u8*)strstr((const char*)(p+1),"\"");
	*p1=0;
	sprintf((char*)ipbuf,"%s",p+1);	
}



//ATK-ESP8266模块测试检测WiFi模块
void atk_8266_test(void)
{
 
	while(atk_8266_send_cmd("AT","OK",100))//检查WIFI模块是否在线
	{
		atk_8266_quit_trans();//退出透传
		atk_8266_send_cmd("AT+CIPMODE=0","OK",200);  //关闭透传模式	
		printf("未检测到模块!!!\r\n");
		delay_ms(800);
		printf("尝试连接模块...\r\n"); 
	} 
	while(atk_8266_send_cmd("ATE0","OK",200));//关闭回显
	printf("size WIFIFlag = %d\r\n",sizeof(WIFIFlag));
	mymemset(&WIFIFlag,0,sizeof(WIFIFlag));
}
/*************************************************
function    :ListWifi
input       :Respon 收到的响应字符串
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
input       :Respon 收到的响应字符串
output		:NULL
return		:
description :解析wifi扫描响应
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
		starttemp += 11;//跳过 +CWLAP:(4,"	
		mymemset(temp, 0, 32);
		endtemp = strstr(starttemp, "\"");

		//wifi名称不为空
		if (endtemp-starttemp)//
		{
			//wifi名称
			mymemcpy(temp, starttemp, endtemp - starttemp);
			SSIDTable.SSID[SSIDTable.number] = (char*)mymalloc(SRAMIN,strlen(temp));
			strcpy(SSIDTable.SSID[SSIDTable.number], temp);
			//wifi强度
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
description :扫描周围可用wifi
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
description :关闭wifi
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
		
	while(atk_8266_send_cmd("AT+CWQAP","OK",200));//关闭WiFi
	while(LISTBOX_GetNumItems(WM_WIFIList))
	{
		LISTBOX_DeleteItem(WM_WIFIList,0);
		delay_ms(10);
	}
	LISTBOX_DeleteItem(WM_WIFIConnect,0);
	LISTBOX_AddString(WM_WIFIConnect,"WIFI未连接");
	LISTBOX_AddString(WM_WIFIList,"请打开WIFI开关");
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
	//其他参数暂时不关心
}
/*************************************************
function    :SelectWiFiHandle 
input       :NULL
output		:NULL
return		:
description :选中wifi之后的处理
**************************************************/
void SelectWiFiHandle(uint8_t index)
{
	int res; 
	index=index-1;//换成0开始的索引
	
	res = strcmp((char *)WiFiConfig.wifista_ssid,(char *)SSIDTable.SSID[index]);
	if(!res)//存储中有记录
	{
		printf("connect record wifi, SSID=%s\r\n",SSIDTable.SSID[index]);
		u8 *p=mymalloc(SRAMIN,32);	
		atk_8266_send_cmd("AT+CWMODE=1","OK",50);		//设置WIFI STA模式

		//设置连接到的WIFI网络名称/加密方式/密码,这几个参数需要根据您自己的路由器设置进行修改!! 
		sprintf((char*)p,"AT+CWJAP=\"%s\",\"%s\"",WiFiConfig.wifista_ssid,WiFiConfig.wifista_password);//设置无线参数:ssid,密码
		while(atk_8266_send_cmd(p,"WIFI GOT IP",1000));	//连接目标路由器,并且获得IP
		BackGroundCtrl.ConnectState = 1;
		myfree(SRAMIN,p);
		
		LISTBOX_DeleteItem(WM_WIFIConnect,0);
		LISTBOX_DeleteItem(WM_WIFIList,index);
		LISTBOX_AddString(WM_WIFIConnect,SSIDTable.SSID[index]);
	}
	else//存储中无记录需要输密码
	{
		printf("connect new wifi, SSID=%s\r\n",SSIDTable.SSID[index]);
		keypad_dev.Finish =0;
		DisplayDialogMsg.x0	=80;
		DisplayDialogMsg.y0	= 80;
		DisplayDialogMsg.xSize = 300;
		DisplayDialogMsg.ySize = 200;
		DisplayDialogMsg.DialogTiltle= "提示";
		DisplayDialogMsg.Editname = "请输入WiFi密码";
		DisplayDialogMsg.hFrame=CreatDispalyDialog(DialogSelectWiFi,&DisplayDialogMsg);
		WM_SetFocus(WM_GetDialogItem(DisplayDialogMsg.hFrame, GUI_ID_EDIT9));
		while(!keypad_dev.Finish)
		{
			delay_ms(500);
		}
		if(keypad_dev.Finish&0x01)//第一位是完成输入 第二位是取消输入
		{
			
			printf("FinalData = %s\r\n",keypad_dev.FinalData);
			u8 *p=mymalloc(SRAMIN,32);	
			atk_8266_send_cmd("AT+CWMODE=1","OK",50);		//设置WIFI STA模式

			//设置连接到的WIFI网络名称/加密方式/密码,这几个参数需要根据您自己的路由器设置进行修改!! 
			sprintf((char*)p,"AT+CWJAP=\"%s\",\"%s\"",SSIDTable.SSID[index],keypad_dev.FinalData);//设置无线参数:ssid,密码
			while(atk_8266_send_cmd(p,"WIFI GOT IP",1000));	//连接目标路由器,并且获得IP
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
		else
		{
			printf("取消输入\r\n");
		}

	}
}

/*************************************************
function    :GUI_connect_server 
input       :NULL
output		:NULL
return		:
description :连接到服务器
**************************************************/

void GUI_Connect_Server(void)
{
	uint8_t res,cnt;
	WM_MESSAGE 	Msg;
	u8 *p=mymalloc(SRAMIN,64);	

	atk_8266_send_cmd("AT+CIPMUX=0","OK",500);   //0：单连接，1：多连接
	sprintf((char*)p,"AT+CIPSTART=\"TCP\",\"%s\",%s",WiFiConfig.severip,(u8*)WiFiConfig.port);    //配置目标TCP服务器
	cnt=0;
	do
	{
		res = atk_8266_send_cmd(p,"OK",500);
		cnt++;
		if(cnt>=5)//尝试连接5次
		{
			ButtonFlag_sever=0;
			BUTTON_SetBitmapEx(BUTTON_ServerSwitch,0,&buttonbmp_tab[0],0,0);
			printf("ATK-ESP 连接TCP 服务器失败\r\n"); //连接失败	
			myfree(SRAMIN,p);		//释放内存 		
			return;
	
		}
	}
	while(res);
		
	atk_8266_send_cmd("AT+CIPMODE=1","OK",500);      //传输模式为：透传			
	Usart3Data.USART3_RX_STA=0;
	
	BackGroundCtrl.ConnectState = 2;
    myfree(SRAMIN,p);		//释放内存 
}
/*************************************************
function    :GUI_connect_server 
input       :NULL
output		:NULL
return		:
description :连接到服务器
**************************************************/

//void GUI_Disconnect_Server(void)
//{
//	atk_8266_send_cmd("AT+CIPCLOSE","OK",500);   //断开tcp连接
//	BackGroundCtrl.ConnectState = 1;
//}
/*************************************************
function    :WIFI_Flag_Handle 
input       :NULL
output		:NULL
return		:
description :与GUI联动的一些标志处理
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
	else if(WIFIFlag.ConnectServer)	
	{
		if(WIFIFlag.ConnectServer == 1)
		{
			GUI_Connect_Server();
		}
		else if(WIFIFlag.ConnectServer == 2)
		{
			//GUI_Disconnect_Server();
			atk_8266_send_cmd("AT+CIPCLOSE","OK",500);   //断开tcp连接
			BackGroundCtrl.ConnectState = 1;	
		}
		WIFIFlag.ConnectServer = 0;
	}
	else if(WIFIFlag.WaitForWifi)
	{
		printf("wait for wifi\r\n");
		WIFIFlag.WaitForWifi=0;
		DisplayDialogMsg.x0	=80;
		DisplayDialogMsg.y0	= 80;
		DisplayDialogMsg.xSize = 300;
		DisplayDialogMsg.ySize = 200;
		DisplayDialogMsg.DialogTiltle= "提示";
		DisplayDialogMsg.Editname = "请先连接wifi";
		DisplayDialogMsg.hFrame = CreatMessageBox_OK(DialogSelectWiFi,&DisplayDialogMsg);
		printf("DisplayDialogMsg.hFrame=%d\r\n",DisplayDialogMsg.hFrame);
		while(!keypad_dev.Finish)
		{
			delay_ms(500);
		}
		keypad_dev.Finish=0;
	}
}












































