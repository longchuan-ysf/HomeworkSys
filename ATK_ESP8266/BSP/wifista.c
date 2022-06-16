#include "common.h"
#include "stdlib.h"
/////////////////////////////////////////////////////////////////////////////////////////////////////////// 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK STM32开发板
//ATK-ESP8266 WIFI模块 WIFI STA驱动代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//修改日期:2015/4/3
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2009-2019
//All rights reserved									  
/////////////////////////////////////////////////////////////////////////////////////////////////////////// 

//ATK-ESP8266 WIFI STA测试
//用于测试TCP/UDP连接
//返回值:0,正常
//    其他,错误代码
u8 atk_8266_wifista_test(void)
{
	static u8 netpro=0;	
	OS_ERR err;
	u8 key;
	u8 *p;
	u16 t=2999;		//加速第一次获取链接状态
	u8 res=0;
	u16 rlen=0;
	u8 constate=0;	//连接状态
	p=mymalloc(SRAMIN,32);							//申请32字节内存
	atk_8266_send_cmd("AT+CWMODE=1","OK",50);		//设置WIFI STA模式


	//设置连接到的WIFI网络名称/加密方式/密码,这几个参数需要根据您自己的路由器设置进行修改!! 
	sprintf((char*)p,"AT+CWJAP=\"%s\",\"%s\"",WiFiConfig.wifista_ssid,WiFiConfig.wifista_password);//设置无线参数:ssid,密码
	while(atk_8266_send_cmd(p,"WIFI GOT IP",500));					//连接目标路由器,并且获得IP
PRESTA:
	printf("ATK-ESP WIFI-STA 测试\r\n"); 
	printf("正在配置ATK-ESP模块,请稍等...\r\n");
	atk_8266_send_cmd("AT+CIPMUX=0","OK",500);   //0：单连接，1：多连接
	sprintf((char*)p,"AT+CIPSTART=\"TCP\",\"%s\",%s",WiFiConfig.severip,(u8*)WiFiConfig.port);    //配置目标TCP服务器
	while(atk_8266_send_cmd(p,"OK",1000))
	{

			printf("WK_UP:返回重选\r\n");
			printf("ATK-ESP 连接TCP失败\r\n"); //连接失败	 
			key=KEY_Scan(0);
			if(key==WKUP_PRES)goto PRESTA;
	}	
	atk_8266_send_cmd("AT+CIPMODE=1","OK",500);      //传输模式为：透传			
	Usart3Data.USART3_RX_STA=0;
	while(1)
	{
		key=KEY_Scan(0);
		if(key==WKUP_PRES)			//WK_UP 退出测试		 
		{ 
//			res=0;					
//			atk_8266_quit_trans();	//退出透传
//			atk_8266_send_cmd("AT+CIPMODE=0","OK",20);   //关闭透传模式
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
		else if(key==KEY0_PRES)	//KEY0 发送数据 
		{
			atk_8266_quit_trans();
			atk_8266_send_cmd("AT+CIPSEND","OK",20);         //开始透传           
			sprintf((char*)p,"ATK-8266 TCP客户端 测试%d\r\n",t/10);//测试数据
			OSMutexPend (&Usart3Data_TX_MUTEX,0,OS_OPT_PEND_BLOCKING,0,&err);//请求互斥信号量
        	u3_printf("%s\r\n",p);	//发送命令
        	OSMutexPost(&Usart3Data_TX_MUTEX,OS_OPT_POST_NONE,&err);//释放互斥信号量

		}	
		t++;
		//delay_ms(10);
		OSTimeDlyHMSM(0,0,0,10,OS_OPT_TIME_PERIODIC,&err);//延时10ms
		if(Usart3Data.USART3_RX_STA&RECEIVE_OK_MARK_U3)		//接收到一次数据了
		{ 
			rlen=Usart3Data.USART3_RX_STA&RECEIVE_LEN_MARK_U3;	//得到本次接收到的数据长度
			Usart3Data.USART3_RX_BUF[rlen]=0;		//添加结束符 
			printf("%s",Usart3Data.USART3_RX_BUF);	//发送到串口   
			Usart3Data.USART3_RX_STA=0;
			if(constate!='+')t=1000;		//状态为还未连接,立即更新连接状态
			else t=0;                   //状态为已经连接了,10秒后再检查
		}  
		if(t==3000)//连续3秒钟没有收到任何数据,检查连接是不是还存在.
		{
			constate=atk_8266_consta_check();//得到连接状态
			if(constate=='+')
			{
				printf("连接成功\r\n");  //连接状态
			}
			else 
			{
				printf("连接失败\r\n"); 
			}				
			t=0;
		}

	}
	myfree(SRAMIN,p);		//释放内存 
	return res;		
} 




























