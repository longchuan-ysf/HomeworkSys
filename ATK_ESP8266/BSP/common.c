#include "common.h"

#define PATH_NETCONFIG "3:/config.txt"
WiFi_config WiFiConfig;
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
	end = (uint8_t *)strstr((char *)start,"\r\n");
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
	//其他参数暂时不关心
	
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
	
	//atk_8266_wifista_test();	
}

void atk_8266_scan(void)
{
	
	while(atk_8266_send_cmd("AT+CWLAP","OK",200));
	
}

















































