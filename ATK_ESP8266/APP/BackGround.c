/*************************************************************************************************** 
                                   xxxx��˾
  
                  

�ļ�:   background.c 
����:   ����
˵��:   ��̨���� ��Ҫ��������������������������Э��Ự��
***************************************************************************************************/
#include "BackGround.h"
#include "MessageHandle.h"
#include "malloc.h"
#include "common.h"
BackGround_Ctrl BackGroundCtrl;

Background_Flag BackgroundFlag;

 /**
 ****************************************************************************************
 @brief:    BackGround_Connect_WiFi ����WiFi
 @Input��   NULL
 @Output��  NULL
 @Return��  NULL
 @Warning:  NULL   
 @note:     ���� 2019-4-18
 ****************************************************************************************
  **/
 void BackGround_Connect_WiFi(void)
 {
    u8 *p=mymalloc(SRAMIN,32);	
    atk_8266_send_cmd("AT+CWMODE=1","OK",50);		//����WIFI STAģʽ

	//�������ӵ���WIFI��������/���ܷ�ʽ/����,�⼸��������Ҫ�������Լ���·�������ý����޸�!! 
	sprintf((char*)p,"AT+CWJAP=\"%s\",\"%s\"",WiFiConfig.wifista_ssid,WiFiConfig.wifista_password);//�������߲���:ssid,����
	while(atk_8266_send_cmd(p,"WIFI GOT IP",500));	//����Ŀ��·����,���һ��IP
	BackGroundCtrl.ConnectState = 1;
	 
	printf("ATK-ESP WIFI-STA ����\r\n"); 
	printf("��������ATK-ESPģ��,���Ե�...\r\n");
	atk_8266_send_cmd("AT+CIPMUX=0","OK",500);   //0�������ӣ�1��������
	sprintf((char*)p,"AT+CIPSTART=\"TCP\",\"%s\",%s",WiFiConfig.severip,(u8*)WiFiConfig.port);    //����Ŀ��TCP������
	while(atk_8266_send_cmd(p,"OK",1000))
	{

			printf("WK_UP:������ѡ\r\n");
			printf("ATK-ESP ����TCPʧ��\r\n"); //����ʧ��	 
			return;
	}	
	atk_8266_send_cmd("AT+CIPMODE=1","OK",500);      //����ģʽΪ��͸��			
	Usart3Data.USART3_RX_STA=0;
	
	BackGroundCtrl.ConnectState = 2;
    myfree(SRAMIN,p);		//�ͷ��ڴ� 
 }
 
  /**
 ****************************************************************************************
 @brief:    BackGround_Check_WiFiState ���WiFi����״̬
 @Input��   NULL
 @Output��  NULL
 @Return��  NULL
 @Warning:  NULL   
 @note:     ���� 2019-4-18
 ****************************************************************************************
  **/
 void BackGround_Check_WiFiState(void)
 {
    u8 constate=0;	//����״̬
    constate=atk_8266_consta_check();//�õ�����״̬
	if(constate=='+')
	{
		printf("���ӳɹ�\r\n");  //����״̬
	}
	else 
	{
		printf("����ʧ��\r\n"); 
	}			
 }
 /**
 ****************************************************************************************
 @brief:    BackGroundInit ��̨��ʼ��
 @Input��   NULL
 @Output��  NULL
 @Return��  NULL
 @Warning:  NULL   
 @note:     ���� 2019-4-18
 ****************************************************************************************
  **/
 extern  Picture_List PictureList;
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

     PictureList.Name = 0;
     PictureList.number =0;
 }

 /**
****************************************************************************************
@brief:    BackGroundTx ��̨��������
@Input��   NULL
@Output��  NULL
@Return    NULL
@Warning:  NULL  
@note:     ����2019-4-18
****************************************************************************************
 **/
void BackGroundTx(void)
{

	
}

 /**
****************************************************************************************
@brief:    BackGroundTask ��̨��������
@Input��   NULL
@Output��  NULL
@Return    NULL
@Warning:  NULL  
@note:     �������ͨ�������MessageHandle.C�д��������ͼƬ�ļ���ת�浽SD����
****************************************************************************************
 **/
void BackGroundRx(void )
{
	MessageRxHandle();
}
