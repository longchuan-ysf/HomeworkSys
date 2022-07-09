/*************************************************************************************************** 
                                   xxxx公司
  
                  

文件:   background.c 
作者:   龙川
说明:   后台任务 主要是网络连接重连————网络协议会话层
***************************************************************************************************/
#include "BackGround.h"
#include "MessageHandle.h"
#include "malloc.h"
#include "common.h"
BackGround_Ctrl BackGroundCtrl;

Background_Flag BackgroundFlag;

 /**
 ****************************************************************************************
 @brief:    BackGround_Connect_WiFi 连接WiFi
 @Input：   NULL
 @Output：  NULL
 @Return：  NULL
 @Warning:  NULL   
 @note:     龙川 2019-4-18
 ****************************************************************************************
  **/
 void BackGround_Connect_WiFi(void)
 {
    u8 *p=mymalloc(SRAMIN,32);	
    atk_8266_send_cmd("AT+CWMODE=1","OK",50);		//设置WIFI STA模式

	//设置连接到的WIFI网络名称/加密方式/密码,这几个参数需要根据您自己的路由器设置进行修改!! 
	sprintf((char*)p,"AT+CWJAP=\"%s\",\"%s\"",WiFiConfig.wifista_ssid,WiFiConfig.wifista_password);//设置无线参数:ssid,密码
	while(atk_8266_send_cmd(p,"WIFI GOT IP",500));	//连接目标路由器,并且获得IP
	BackGroundCtrl.ConnectState = 1;
	 
	printf("ATK-ESP WIFI-STA 测试\r\n"); 
	printf("正在配置ATK-ESP模块,请稍等...\r\n");
	atk_8266_send_cmd("AT+CIPMUX=0","OK",500);   //0：单连接，1：多连接
	sprintf((char*)p,"AT+CIPSTART=\"TCP\",\"%s\",%s",WiFiConfig.severip,(u8*)WiFiConfig.port);    //配置目标TCP服务器
	while(atk_8266_send_cmd(p,"OK",1000))
	{

			printf("WK_UP:返回重选\r\n");
			printf("ATK-ESP 连接TCP失败\r\n"); //连接失败	 
			return;
	}	
	atk_8266_send_cmd("AT+CIPMODE=1","OK",500);      //传输模式为：透传			
	Usart3Data.USART3_RX_STA=0;
	
	BackGroundCtrl.ConnectState = 2;
    myfree(SRAMIN,p);		//释放内存 
 }
 
  /**
 ****************************************************************************************
 @brief:    BackGround_Check_WiFiState 检查WiFi连接状态
 @Input：   NULL
 @Output：  NULL
 @Return：  NULL
 @Warning:  NULL   
 @note:     龙川 2019-4-18
 ****************************************************************************************
  **/
 void BackGround_Check_WiFiState(void)
 {
    u8 constate=0;	//连接状态
    constate=atk_8266_consta_check();//得到连接状态
	if(constate=='+')
	{
		printf("连接成功\r\n");  //连接状态
	}
	else 
	{
		printf("连接失败\r\n"); 
	}			
 }
 /**
 ****************************************************************************************
 @brief:    BackGroundInit 后台初始化
 @Input：   NULL
 @Output：  NULL
 @Return：  NULL
 @Warning:  NULL   
 @note:     龙川 2019-4-18
 ****************************************************************************************
  **/
 void BackGroundInit(void)
 {
	 memset(&BackGroundCtrl,0,sizeof(BackGroundCtrl));
	 //BackGround_Connect_WiFi();
	 
	 BackGroundCtrl.MaxTxBufferLen = Usart3Data.USART3_MAX_SEND_LEN;
	 BackGroundCtrl.MaxRxBufferLen = Usart3Data.USART3_MAX_RECV_LEN;
	 
	 BackGroundCtrl.Message_TXBuffer = Usart3Data.USART3_TX_BUF;
	 BackGroundCtrl.Message_RXBuffer = Usart3Data.USART3_RX_BUF;
	 BackGroundCtrl.Message_TxLen = 0;
	 
	 BackGroundCtrl.BackgroundSend = usart3_send;
	 
	 memset(&BackgroundFlag,0,sizeof(BackgroundFlag));
 }

 /**
****************************************************************************************
@brief:    BackGroundTx 后台发送任务
@Input：   NULL
@Output：  NULL
@Return    NULL
@Warning:  NULL  
@note:     龙川2019-4-18
****************************************************************************************
 **/
void BackGroundTx(void)
{

	
}

 /**
****************************************************************************************
@brief:    BackGroundTask 后台接收任务
@Input：   NULL
@Output：  NULL
@Return    NULL
@Warning:  NULL  
@note:     如果是普通的命令，在MessageHandle.C中处理，如果是图片文件则转存到SD卡中
****************************************************************************************
 **/
void BackGroundRx(void )
{
	MessageRxHandle();
}
