/*************************************************************************************************** 
                                   xxxx��˾
  
                  

�ļ�:   MessageHandle.c 
����:   ����
˵��:   Э�鱨�Ĵ���������Э��Ӧ�ò�
***************************************************************************************************/
#include "BackGround.h"
#include "MessageHandle.h"
#include "string.h"
#include "usart.h"
#include "malloc.h"
#include "common.h"
#include "usbh_app.h"
#include "ImageDisplay.h"
#include "HomeworkGUI.h"
#include "EmWinHZFont.h"
#include "EmWinHZFont.h"
#include "ImageDisplay.h"

#define Root_URL "/HFS"
#define TEST_POST 0
#define UPDATA_PICTURE_PATH "list.txt"
Picture_List PictureList;

#if TEST_POST==1
const char testhead[] =\
"POST /HFS/ HTTP/1.1\r\n"\
"Host: 192.168.86.101\r\n"\
"Connection: keep-alive\r\n"\
"Content-Length: 137340\r\n"\
"Cache-Control: max-age=0\r\n"\
"Upgrade-Insecure-Requests: 1\r\n"\
"Origin: http://192.168.86.101\r\n"\
"Content-Type: multipart/form-data; boundary=----WebKitFormBoundaryO2WAD3vh5VDNQCJf\r\n"\
"User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/102.0.5005.124 Safari/537.36 Edg/102.0.1245.41\r\n"\
"Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.9\r\n"\
"Referer: http://192.168.86.101/HFS/\r\n"\
"Accept-Encoding: gzip, deflate\r\n"\
"Accept-Language: zh-CN,zh;q=0.9,en;q=0.8,en-GB;q=0.7,en-US;q=0.6\r\n"\
"Cookie: HFS_SID_=0.165556978899986\r\n \r\n"\
"------WebKitFormBoundaryO2WAD3vh5VDNQCJf\r\n"\
"Content-Disposition: form-data; name=\"file\"; filename=\"zhongli.jpg\"\r\n"\
"Content-Type: image/jpeg\r\n\r\n";

const char testend[] =\
"\r\n------WebKitFormBoundaryO2WAD3vh5VDNQCJf\r\n"\
"Content-Disposition: form-data; name=\"file\"; filename=""\r\n"\
"Content-Type: application/octet-stream\r\n\r\n"\
"\r\n------WebKitFormBoundaryO2WAD3vh5VDNQCJf--\r\n"; 
#endif

Http_Respon  HttpRespon;//http����Ӧͷ����
static uint8_t SaveDir[32];
//�ļ���չ���� ContentType-type֮��Ĺ�ϵ
const Http_File_Attribute HttpFileAttribute[] =
{
	{(uint8_t *)"jpg",(uint8_t *)"image/jpeg"},
	{(uint8_t *)"bmp",(uint8_t *)"application/x-bmp"},
	{(uint8_t *)"png",(uint8_t *)"image/png"},
	{(uint8_t *)"txt",(uint8_t *)"text/plain"},
};
char parse_url(struct http_parser_urt * httpurl, char * strurl)
{
    char ret = 0;
    char * line_start = NULL;
    char * line_end = NULL;
    char * start_temp = NULL;
    char * end_temp = NULL;
    char * line_temp = NULL;
	memset(httpurl,0,sizeof(struct http_parser_urt));
  //������
    if( (httpurl == NULL)||(strurl == NULL) )
    {
        ret = 1;
        return ret;
    }

    line_temp = (char *)strstr(strurl, "\r\n\r\n");
    if (line_temp == NULL)
    {
        ret = 1;
        return ret;
    }

/* ȷ���ļ������׵�ַ��β��ַ */

    line_start = strurl;
    line_end = (char *)strstr(line_start, "\r\n");
    if (line_end == NULL)
    {
        ret = 1;
        return ret;
    }

    /* �ҵ������� */
    if (strncmp(line_start, "GET ", 4) == 0)
    {
        httpurl->method = 0;
        start_temp = line_start + 4;
    }
    else if (strncmp(strurl, "POST ", 5) == 0)
    {
        httpurl->method = 1;
        start_temp = line_start + 5;
    }
    else
    {



    }
    /* �ҵ��ļ������ڵ��ַ��� */
    start_temp++;
    end_temp = (char *)strstr(line_start, " HTTP");
    
    line_start = start_temp;
    line_end = end_temp;
    /*���ַ�������ȡ�ļ���
    ��Ϊ /aaaa/bbb/ccc.txt��ȡΪccc.txt
    ��Ϊ ccc.txt ��ȡΪccc.txt
    ��Ϊ /aaaa/bbb/ �ж�Ϊ���ļ�
    ��Ϊ /aaaa/bbb/ccc �ж�Ϊ���ļ�

    ���濪ʼ��Ҫ�� line_end ���� line_startָ��������һ��/
    */
    end_temp = (char *)strstr(line_start, ".");
    /*Ϊ /aaaa/bbb/ ���� /aaaa/bbb/ccc �ж�Ϊ���ļ�*/
    if(end_temp == NULL)
    {
        ret = 1;
        return ret;
    }
    //�� . ��ʼ��ǰ�� /
    while(end_temp-line_start)
    {
        end_temp--;//��ǰ��һλ
        if(end_temp[0] == '/')
        {
            line_start = end_temp;
        }
    }
    line_start++;
    //��ֹ���� /aa/bb/.�����
    if(line_start == line_end)
    {
        ret = 1;
        return ret;
    }

    strncpy(httpurl->filename, line_start, line_end-line_start);
    return ret;
}
/**
****************************************************************************************
@brief:  MessageTxInit
@Input��  method ������GET POST
@Output��  NULL
@Return��  NULL
@Warning:  NULL   
@note:     http�����к�����ͷ���Ĵ���
****************************************************************************************
 **/
void MessageTxInit(uint8_t method,uint8_t *url)
{
	uint8_t *p;
	uint16_t len;
	
	p = mymalloc(SRAMIN,128);
	if(p == NULL)
	{
		return;
	}
	//��ʼ��
	BackGroundCtrl.Message_TxLen = 0;
	memset(BackGroundCtrl.Message_TXBuffer,0,BackGroundCtrl.MaxTxBufferLen);

	switch (method)
	{
		case GET:
		{
			mymemset(p,0,128);
			sprintf((char *)p,"GET %s HTTP/1.1\r\n",url);
			len = strlen((char *)p);
			mymemcpy(&BackGroundCtrl.Message_TXBuffer[BackGroundCtrl.Message_TxLen],p,len);
			BackGroundCtrl.Message_TxLen  += len;
			
			mymemset(p,0,128);
			sprintf((char *)p,"HOST: %s\r\n",WiFiConfig.severip);
			len = strlen((char *)p);
			mymemcpy(&BackGroundCtrl.Message_TXBuffer[BackGroundCtrl.Message_TxLen],p,len);
			BackGroundCtrl.Message_TxLen  += len;
			
			mymemset(p,0,128);
			sprintf((char *)p,"User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:101.0) Gecko/20100101Firefox/101.0\r\n");
			len = strlen((char *)p);
			mymemcpy(&BackGroundCtrl.Message_TXBuffer[BackGroundCtrl.Message_TxLen],p,len);
			BackGroundCtrl.Message_TxLen  += len;
			
			mymemset(p,0,128);
			sprintf((char *)p,"Accept-Language: zh-CN,zh;q=0.8,zh-TW;q=0.7,zh-HK;q=0.5,en-US;q=0.3,en;q=0.2\r\n");
			len = strlen((char *)p);
			mymemcpy(&BackGroundCtrl.Message_TXBuffer[BackGroundCtrl.Message_TxLen],p,len);
			BackGroundCtrl.Message_TxLen  += len;
			
			mymemset(p,0,128);
			sprintf((char *)p,"Connection: keep-alive\r\n\r\n");
			len = strlen((char *)p);
			mymemcpy(&BackGroundCtrl.Message_TXBuffer[BackGroundCtrl.Message_TxLen],p,len);
			BackGroundCtrl.Message_TxLen  += len;		
			
		}
		break;
		case POST:
		{
			mymemset(p,0,128);
			sprintf((char *)p,"POST %s HTTP/1.1\r\n",url);
			len = strlen((char *)p);
			mymemcpy(&BackGroundCtrl.Message_TXBuffer[BackGroundCtrl.Message_TxLen],p,len);
			BackGroundCtrl.Message_TxLen  += len;
			
			mymemset(p,0,128);
			sprintf((char *)p,"HOST: %s\r\n",WiFiConfig.severip);
			len = strlen((char *)p);
			mymemcpy(&BackGroundCtrl.Message_TXBuffer[BackGroundCtrl.Message_TxLen],p,len);
			BackGroundCtrl.Message_TxLen  += len;
   
            mymemset(p,0,128);
			sprintf((char *)p,"Accept: */*\r\n");
			len = strlen((char *)p);
			mymemcpy(&BackGroundCtrl.Message_TXBuffer[BackGroundCtrl.Message_TxLen],p,len);
			BackGroundCtrl.Message_TxLen  += len;
            
			mymemset(p,0,128);
			sprintf((char *)p,"Content-Type: multipart/form-data; boundary=%s\r\n",BOUNDARY);
			len = strlen((char *)p);
			mymemcpy(&BackGroundCtrl.Message_TXBuffer[BackGroundCtrl.Message_TxLen],p,len);
			BackGroundCtrl.Message_TxLen  += len;
			
			mymemset(p,0,128);
			sprintf((char *)p,"Connection: keep-alive\r\n");
			len = strlen((char *)p);
			mymemcpy(&BackGroundCtrl.Message_TXBuffer[BackGroundCtrl.Message_TxLen],p,len);
			BackGroundCtrl.Message_TxLen  += len;
			//�ڷ���֮ǰ��Ҫ�����������ݵ����ͣ����ݳ��ȡ�Ĭ���ļ�
		
		}
		break;
		default:
		break;
	}
		myfree(SRAMIN,p);
}
/**
****************************************************************************************
@brief:    MessageTx
@Input��   method ���󷽷�
		   data ����
		   datalen ���ݳ���
@Output��  NULL
@Return��  NULL
@Warning:  NULL   
@note:     ���http�������ݣ�������
****************************************************************************************
 **/
void MessageTx(void)
{
	
	printf("BackGround send:%d\r\n data:\r\n%s\r\n",BackGroundCtrl.Message_TxLen,BackGroundCtrl.Message_TXBuffer);
	atk_8266_quit_trans();
	atk_8266_send_cmd("AT+CIPSEND","OK",200);         //��ʼ͸��  
	BackGroundCtrl.BackgroundSend(BackGroundCtrl.Message_TXBuffer,BackGroundCtrl.Message_TxLen);
	printf("BackGround send:ok\r\n");
	

}

void test_http_get()
{
	OS_ERR err;
	char url[] = "/HFS/download/list.txt";
	OSMutexPend (&Usart3Data_TX_MUTEX,0,OS_OPT_PEND_BLOCKING,0,&err);//���󻥳��ź���,��ֹ���߳��������߳����޸ķ���������
	MessageTxInit(GET,(uint8_t *)&url[0]);	
	MessageTx();
    BackGroundCtrl.HttpRespone = 0;
	OSMutexPost(&Usart3Data_TX_MUTEX,OS_OPT_POST_NONE,&err);//�ͷŻ����ź���
	HttpRespon.Method = GET;//��ǵȴ�GET�ظ�
}
 void http_get(char *url)
 {
     OS_ERR err;
     OSMutexPend (&Usart3Data_TX_MUTEX,0,OS_OPT_PEND_BLOCKING,0,&err);//���󻥳��ź���,��ֹ���߳��������߳����޸ķ���������
     MessageTxInit(GET,(uint8_t *)&url[0]);  
     MessageTx();
     BackGroundCtrl.HttpRespone = 0;
     OSMutexPost(&Usart3Data_TX_MUTEX,OS_OPT_POST_NONE,&err);//�ͷŻ����ź���
     HttpRespon.Method = GET;//��ǵȴ�GET�ظ�
 }

 /**
****************************************************************************************
@brief:    Generate_Random_Name �����������
@Input��    NULL
@Output��   NULL
@Return��   NULL
@Warning:   NULL   
@note:     ���� 2022-6-10
****************************************************************************************
 **/
void Generate_Random_Name(void)
{
	uint32_t name;
	name = rand()%1000;
	if(!HttpRespon.ContentType)
	{
		printf("not give type\r\n");
		return;
	}
	
	HttpRespon.ContentDisposition = (uint8_t *)mymalloc(SRAMIN,64);
	mymemset(HttpRespon.ContentDisposition,0,64);	
	sprintf((char *)HttpRespon.ContentDisposition,"%d.%s",name,HttpRespon.ContentType);

	printf("not give default name,use random name :%s\r\n",HttpRespon.ContentDisposition);
}

 /**
****************************************************************************************
@brief:    Http_Save_Date ����http���ܵ�����
@Input��    NULL
@Output��   NULL
@Return��   NULL
@Warning:   NULL   
@note:     ���� 2022-6-11
****************************************************************************************
 **/

uint8_t Http_Save_Date(uint8_t *data,uint32_t len)
{
	
    FIL fp;
    FRESULT res;
    char *path,*pData;
    uint32_t br;
	#if 0
    u16 PathLen,timeout;
    pUSBH_WR_MSG pMsgWR;
	pMsgWR = USBH_Malloc_CtrlStruct();
	PathLen = strlen((char *)HttpRespon.ContentDisposition)+strlen((char *)SaveDir)+1;//1�ǽ�����\0
	USBH_Malloc_Path(pMsgWR,PathLen);

	sprintf((char *)pMsgWR->path,"%s%s",&SaveDir[0],HttpRespon.ContentDisposition);

	USBH_Malloc_WriteBuf(pMsgWR,WR_BUFF_EX,data);
	
    USBH_ApplyFor_WR(pMsgWR,len,FA_WRITE|FA_CREATE_ALWAYS);
    timeout = 0;
	while(1)
	{
        timeout++;
		if(pMsgWR->result==FR_OK)
		{
			USBH_WR_MsgFree(pMsgWR);
			printf("write ok\r\n");
			return 0;
		}
        if(timeout>10*1000)//10s�ĳ�ʱ�ж�
        {
		    return 1;
        }
		delay_ms(10);//��ʱ10ms
	}
    #else
    path = mymalloc(SRAMIN,64);
    if(!path)
    {
        printf("malloc path failed\r\n");
        myfree(SRAMIN, path);
        return 1;
    }
    mymemset(path, 0, sizeof(path));
    //�����ֽڶ������⣬ͨ�����������ڴ�ʵ��
    pData = mymalloc(SRAMEX, len);
    if(!pData)
    {
        printf("malloc pData failed\r\n");
        myfree(SRAMIN, path);
        myfree(SRAMEX, pData);
        return 1;
    }   
    mymemcpy(pData, data, len);
    
    sprintf(path,"3:/download/%s",HttpRespon.ContentDisposition);
    res=f_open(&fp,path,FA_WRITE|FA_CREATE_ALWAYS);
    if(res)
    {
        printf("open %s failed\r\n",path);
        myfree(SRAMIN, path);
        return 1;
    }
    
    res = f_write(&fp, pData, len,&br);
	if(len!=br)
	{
		printf("write %s failed len!=br\r\n",path);
        myfree(SRAMIN, path);
        myfree(SRAMEX, pData);
        f_close(&fp);
        return 1;
	}
    if(res)
    {
        printf("write %s failed\r\n",path);
        myfree(SRAMIN, path);
        myfree(SRAMEX, pData);
        f_close(&fp);
        return 1;
    }
    
    f_close(&fp);
    myfree(SRAMIN, path);
    myfree(SRAMEX, pData);
	printf("save data ok\r\n");
     return 0;
    #endif
   
}



 /**
****************************************************************************************
@brief:    HTTP_Post_AddFileInfo ����������ӱ�Ҫ�ļ���������
@Input��    �ļ���
@Output��   NULL
@Return��   NULL
@Warning:   NULL   
@note:    
****************************************************************************************
 **/
void HTTP_Post_AddFileInfo(uint8_t *FileName,uint8_t *FileDate,uint32_t FileLen)
{
    #if TEST_POST==0
	
	uint8_t *pName,*boundary,*ContentType,*ContentLenTXT,*ContentDisposition,*EndMsg;
	uint8_t i;
	uint32_t DateLen,BoundaryLen,ContentTxtLen,ContentTypeLen,DispostionLen,EndMsgLen;
	//���boundary�ͱ�Ҫ����Ϣ
	boundary = mymalloc(SRAMIN,64);
	if(!boundary)
	{
		printf("malloc boundary err!\r\n");
		myfree(SRAMIN,boundary);
		return;
	}
	ContentType = mymalloc(SRAMIN,128);
	if(!ContentType)
	{
		printf("malloc ContentType err!\r\n");
		myfree(SRAMIN,boundary);
		myfree(SRAMIN,ContentType);
		return;
	}
	ContentDisposition =  mymalloc(SRAMIN,128);
	if(!ContentDisposition)
	{
		printf("malloc ContentDisposition err!\r\n");
		myfree(SRAMIN,boundary);
		myfree(SRAMIN,ContentType);
		myfree(SRAMIN,ContentDisposition);
		return;
	}
	EndMsg =  mymalloc(SRAMIN,256);
	if(!ContentDisposition)
	{
		printf("malloc EndMsg err!\r\n");
		myfree(SRAMIN,boundary);
		myfree(SRAMIN,ContentType);
		myfree(SRAMIN,ContentDisposition);
		myfree(SRAMIN,EndMsg);
		return;
	}
	ContentLenTXT =  mymalloc(SRAMIN,128);
	if(!ContentLenTXT)
	{
		printf("malloc ContentLenTXT err!\r\n");
		myfree(SRAMIN,boundary);
		myfree(SRAMIN,ContentType);
		myfree(SRAMIN,ContentDisposition);
		myfree(SRAMIN,EndMsg);
		myfree(SRAMIN,ContentLenTXT);
		return;
	}
	//��ʼ�ָ���
	mymemset(boundary,0,128);
	sprintf((char *)boundary,"--%s\r\n",BOUNDARY);
	BoundaryLen = strlen((char *)boundary);
	
	//��������
	DateLen = 0;
	pName = (uint8_t *)strrchr((char *)FileName,'.');
	pName++;	
	for(i=0; i<sizeof(HttpFileAttribute)/sizeof(Http_File_Attribute);i++)
	{
		if(!strcasecmp((char *)pName,(char *)HttpFileAttribute[i].ExName))
		{		
			mymemset(ContentType,0,128);
			sprintf((char *)ContentType,"Content-Type: %s\r\n\r\n",HttpFileAttribute[i].ContentType);
			ContentTypeLen = strlen((char *)ContentType);
			break;
		}
	}
	//Ĭ���ļ���
	mymemset(ContentDisposition,0,128);
	sprintf((char *)ContentDisposition,"Content-Disposition: form-data; name=\"file\"; filename=\"%s\"\r\n",FileName);
	DispostionLen = strlen((char *)ContentDisposition);
	
	
	//������Ϣ
	mymemset(EndMsg,0,256);
	//                          �ָ���  ������Ϣ  �ָ���
	sprintf((char *)EndMsg,"\r\n--%s\r\n%s\r\n--%s--\r\n",BOUNDARY,UPLOAD_END,BOUNDARY);
	EndMsgLen = strlen((char *)EndMsg);
	
	//���ݳ���= �ָ���      + ��������       + Ĭ���ļ���    + �ļ���С  + ������
	DateLen = BoundaryLen + ContentTypeLen + DispostionLen + FileLen + EndMsgLen;
	//DateLen = FileLen;
	mymemset(ContentLenTXT,0,128);
	sprintf((char *)ContentLenTXT,"Content-Length: %d\r\n\r\n",DateLen);
	ContentTxtLen = strlen((char *)ContentLenTXT);
	
	//�������ݳ����ֶ�
	mymemcpy(&BackGroundCtrl.Message_TXBuffer[BackGroundCtrl.Message_TxLen],ContentLenTXT,ContentTxtLen);
	BackGroundCtrl.Message_TxLen  += ContentTxtLen;
	
	//����ָ���
	mymemcpy(&BackGroundCtrl.Message_TXBuffer[BackGroundCtrl.Message_TxLen],boundary,BoundaryLen);
	BackGroundCtrl.Message_TxLen  += BoundaryLen;
	
	//�����ļ���
	mymemcpy(&BackGroundCtrl.Message_TXBuffer[BackGroundCtrl.Message_TxLen],ContentDisposition,DispostionLen);
	BackGroundCtrl.Message_TxLen  += DispostionLen;
	
	//�����ļ�����
	mymemcpy(&BackGroundCtrl.Message_TXBuffer[BackGroundCtrl.Message_TxLen],ContentType,ContentTypeLen);
	BackGroundCtrl.Message_TxLen  += ContentTypeLen;
	
	
	//��������
	mymemcpy(&BackGroundCtrl.Message_TXBuffer[BackGroundCtrl.Message_TxLen],FileDate,FileLen);
	BackGroundCtrl.Message_TxLen  += FileLen;
	
	//���������Ϣ
	mymemcpy(&BackGroundCtrl.Message_TXBuffer[BackGroundCtrl.Message_TxLen],EndMsg,EndMsgLen);
	BackGroundCtrl.Message_TxLen  += EndMsgLen;
	
	myfree(SRAMIN,boundary);
	myfree(SRAMIN,ContentType);
	myfree(SRAMIN,ContentDisposition);
	myfree(SRAMIN,EndMsg);
	myfree(SRAMIN,ContentLenTXT);
    #else
    //��ʼ��
	BackGroundCtrl.Message_TxLen = 0;
	mymemset(BackGroundCtrl.Message_TXBuffer,0,BackGroundCtrl.MaxTxBufferLen);
    uint16_t headLen,endlen;

    headLen = strlen((char *)&testhead[0]);
    mymemcpy(&BackGroundCtrl.Message_TXBuffer[BackGroundCtrl.Message_TxLen], (void *)&testhead[0],headLen);
    BackGroundCtrl.Message_TxLen += headLen;

 
    mymemcpy(&BackGroundCtrl.Message_TXBuffer[BackGroundCtrl.Message_TxLen], FileDate,FileLen);
    BackGroundCtrl.Message_TxLen += FileLen;

    endlen = strlen((char *)&testend[0]);
    mymemcpy(&BackGroundCtrl.Message_TXBuffer[BackGroundCtrl.Message_TxLen], (void *)&testend[0],endlen);
    BackGroundCtrl.Message_TxLen += endlen;
    
    #endif
 
}

 /**
****************************************************************************************
@brief:    Http_Post_Date �������ύ����
@Input��    NULL
@Output��   NULL
@Return��   NULL
@Warning:   NULL   
@note:    �������һ�����ܺ� MessageRxHandle ����ͬһ���̣߳���Ϊÿ�η��Ͷ�Ҫ�ȴ��ظ����ܽ�����һ�η���
****************************************************************************************
 **/

void Http_Post_Date(uint8_t *FileName,uint8_t *url)
{

	OS_ERR err;    
    u16 PathLen,timeout;
	uint8_t *filepath;
    pUSBH_WR_MSG pMsgRD;


	OSMutexPend (&Usart3Data_TX_MUTEX,0,OS_OPT_PEND_BLOCKING,0,&err);//���󻥳��ź���,��ֹ���߳��������߳����޸ķ���������

	
	#if TEST_POST==0
	MessageTxInit(POST,url);
	#endif
	pMsgRD = USBH_Malloc_CtrlStruct();
	
	PathLen = strlen((char *)FileName)+strlen("3:/update/")+1;//1�ǽ�����\0

	USBH_Malloc_Path(pMsgRD,PathLen);
	sprintf((char *)pMsgRD->path,"3:/update/%s",FileName);
	
    USBH_ApplyFor_WR(pMsgRD,RD_ALL_DATA,FA_READ);
    timeout = 0;
	while(1)
	{
        timeout++;
		if(pMsgRD->result==FR_OK)
		{
			printf("read ok\r\n");
			break;
		}
        if(timeout>10*1000)//10s�ĳ�ʱ�ж�
        {
            printf("timeout!!!!!!!\r\n");
		   goto post_return ; 
        }
		delay_ms(10);//��ʱ10ms
	}	
	HTTP_Post_AddFileInfo(FileName,pMsgRD->data,pMsgRD->bread);	
    USBH_WR_MsgFree(pMsgRD);
	MessageTx();
post_return:
	myfree(SRAMIN,filepath);
	OSMutexPost(&Usart3Data_TX_MUTEX,OS_OPT_POST_NONE,&err);//�ͷŻ����ź���	
	return ;
}

void test_http_post()
{
	HttpRespon.Method = POST;//��ǵȴ�GET�ظ�	
	Http_Post_Date("zhongli.jpg","/HFS/");
 
}
void test_post(void)
{
    char result;
	int bread;
	FIL File;
    
    BackGroundCtrl.Message_TxLen = 0;
	memset(BackGroundCtrl.Message_TXBuffer,0,BackGroundCtrl.MaxTxBufferLen);

    BackGroundCtrl.Message_TxLen += \
        sprintf((char *)&BackGroundCtrl.Message_TXBuffer[BackGroundCtrl.Message_TxLen],\
       "%s" ,"POST /HFS/ HTTP/1.1\r\n"
    );
    BackGroundCtrl.Message_TxLen += \
        sprintf((char *)&BackGroundCtrl.Message_TXBuffer[BackGroundCtrl.Message_TxLen],\
       "%s" ,"Host: 192.168.86.101\r\n"
    );
    BackGroundCtrl.Message_TxLen += \
        sprintf((char *)&BackGroundCtrl.Message_TXBuffer[BackGroundCtrl.Message_TxLen],\
       "%s" ,"Connection: keep-alive\r\n"
    );
    BackGroundCtrl.Message_TxLen += \
        sprintf((char *)&BackGroundCtrl.Message_TXBuffer[BackGroundCtrl.Message_TxLen],\
       "%s" ,"Content-Length: 137340\r\n"
    );
    BackGroundCtrl.Message_TxLen += \
        sprintf((char *)&BackGroundCtrl.Message_TXBuffer[BackGroundCtrl.Message_TxLen],\
       "%s" ,"Cache-Control: max-age=0\r\n"
    );
    BackGroundCtrl.Message_TxLen += \
        sprintf((char *)&BackGroundCtrl.Message_TXBuffer[BackGroundCtrl.Message_TxLen],\
       "%s" ,"Upgrade-Insecure-Requests: 1\r\n"
    );
    BackGroundCtrl.Message_TxLen += \
        sprintf((char *)&BackGroundCtrl.Message_TXBuffer[BackGroundCtrl.Message_TxLen],\
       "%s" ,"Origin: http://192.168.86.101\r\n"
    );
    BackGroundCtrl.Message_TxLen += \
        sprintf((char *)&BackGroundCtrl.Message_TXBuffer[BackGroundCtrl.Message_TxLen],\
       "%s" ,"Content-Type: multipart/form-data; boundary=----WebKitFormBoundaryJVDvW59t5ghf4dVJ\r\n"
    );
    BackGroundCtrl.Message_TxLen += \
        sprintf((char *)&BackGroundCtrl.Message_TXBuffer[BackGroundCtrl.Message_TxLen],\
       "%s" ,"User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/102.0.5005.124 Safari/537.36 Edg/102.0.1245.41\r\n"
    );
    BackGroundCtrl.Message_TxLen += \
        sprintf((char *)&BackGroundCtrl.Message_TXBuffer[BackGroundCtrl.Message_TxLen],\
       "%s" ,"Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.9\r\n"
    );
    BackGroundCtrl.Message_TxLen += \
        sprintf((char *)&BackGroundCtrl.Message_TXBuffer[BackGroundCtrl.Message_TxLen],\
       "%s" ,"Referer: http://192.168.86.101/HFS/\r\n"
    );
    BackGroundCtrl.Message_TxLen += \
        sprintf((char *)&BackGroundCtrl.Message_TXBuffer[BackGroundCtrl.Message_TxLen],\
       "%s" ,"Accept-Encoding: gzip, deflate\r\n"
    );
    BackGroundCtrl.Message_TxLen += \
        sprintf((char *)&BackGroundCtrl.Message_TXBuffer[BackGroundCtrl.Message_TxLen],\
       "%s" ,"Accept-Language: zh-CN,zh;q=0.9,en;q=0.8,en-GB;q=0.7,en-US;q=0.6\r\n"
    );
    BackGroundCtrl.Message_TxLen += \
        sprintf((char *)&BackGroundCtrl.Message_TXBuffer[BackGroundCtrl.Message_TxLen],\
       "%s" ,"Cookie: HFS_SID_=0.0328552364371717\r\n\r\n"
    );
    BackGroundCtrl.Message_TxLen += \
        sprintf((char *)&BackGroundCtrl.Message_TXBuffer[BackGroundCtrl.Message_TxLen],\
       "%s" ,"------WebKitFormBoundaryJVDvW59t5ghf4dVJ\r\n"
    );
    BackGroundCtrl.Message_TxLen += \
        sprintf((char *)&BackGroundCtrl.Message_TXBuffer[BackGroundCtrl.Message_TxLen],\
       "%s" ,"Content-Disposition: form-data; name=\"file\"; filename=\"zhongli.jpg\"\r\n"
    );
    BackGroundCtrl.Message_TxLen += \
        sprintf((char *)&BackGroundCtrl.Message_TXBuffer[BackGroundCtrl.Message_TxLen],\
       "%s" ,"Content-Type: image/jpeg\r\n\r\n"
    );
    result = f_open(&File,(const TCHAR*)"0:/update/zhongli.jpg",FA_READ);	//���ļ�

	if((result != FR_OK)) 	
	{
		printf("open failed,result = %d\r\n",result);
		return ;
	}	

    result = f_read(&File,\
    &BackGroundCtrl.Message_TXBuffer[BackGroundCtrl.Message_TxLen],File.obj.objsize,(UINT *)&bread);
    if((result != FR_OK)) 	
	{
		printf("open failed,result = %d\r\n",result);
		return ;
	}
    printf("file size = %lld\r\n",File.obj.objsize);    
    BackGroundCtrl.Message_TxLen += File.obj.objsize;

    f_close(&File);

    BackGroundCtrl.Message_TxLen += \
        sprintf((char *)&BackGroundCtrl.Message_TXBuffer[BackGroundCtrl.Message_TxLen],\
       "%s" ,"\r\n------WebKitFormBoundaryJVDvW59t5ghf4dVJ\r\n"
    );
    BackGroundCtrl.Message_TxLen += \
        sprintf((char *)&BackGroundCtrl.Message_TXBuffer[BackGroundCtrl.Message_TxLen],\
       "%s" ,"Content-Disposition: form-data; name=\"file\"; filename=\"\"\r\n"
    );
    BackGroundCtrl.Message_TxLen += \
        sprintf((char *)&BackGroundCtrl.Message_TXBuffer[BackGroundCtrl.Message_TxLen],\
       "%s" ,"Content-Type: application/octet-stream\r\n\r\n"
    );
    BackGroundCtrl.Message_TxLen += \
        sprintf((char *)&BackGroundCtrl.Message_TXBuffer[BackGroundCtrl.Message_TxLen],\
       "%s" ,"\r\n------WebKitFormBoundaryJVDvW59t5ghf4dVJ--\r\n"
    );
    MessageTx();
}
 /**
****************************************************************************************
@brief:    HTTP_Handle ����http��Ӧ
@Input��    data ����
           Len  ���ݳ���
@Output��   NULL
@Return��   NULL
@Warning:  NULL   
@note:     ���� 2022-6-10
****************************************************************************************
 **/
void HTTP_Handle(uint8_t *data,uint16_t len)
{
	static uint8_t temp[128];
	uint8_t* StateLine;//״̬��
	uint8_t* ContentType;//��������
	uint8_t* ContentLength;//���ݳ���
	uint8_t* ContentDisposition;//Ĭ���ļ���
	uint8_t* templine;
	uint8_t len_temp,i;//Ϊ�˷�����Զ�ȡ���м����
	mymemset(&HttpRespon, 0, sizeof(HttpRespon));
	
	//��ȡ״̬���������Խ��� ����ֱ�Ӳ�OK����һ��Ϣͷ�д���OK������Ҫ��״̬������OK
	mymemset(temp, 0, sizeof(temp));
	StateLine = (uint8_t*)strstr((char*)data, "HTTP");
	if(!StateLine)
	{
		printf("Response formatting error1 \r\n");
		goto http_return;
	}
	
	//�� StateLine ��ʼ�����ҵ���һ�����з�
	templine = (uint8_t *)strstr((char *)StateLine,"\r\n");
	if(!templine)
	{
		printf("Response formatting error2 \r\n");
		goto http_return;
	}
	mymemcpy(temp,StateLine,templine-StateLine);
	StateLine = (uint8_t *)strstr((char *)temp,"OK");
	if(!StateLine)
	{
		printf("http responed not ok! %s\r\n",temp);
		goto http_return;
	}
	HttpRespon.State = atoi((char *)&temp[8]);
	printf("http receive ,http responed ok!,state=%d\r\n",HttpRespon.State);
		
	if(HttpRespon.Method == POST)
	{
		goto http_return;//POST�յ��ظ�OK��Ͳ���Ҫ��������
	}
	
	
	mymemset(temp,0,sizeof(temp));
	ContentType = (uint8_t *)strstr((char *)data,"Content-Type");
	if(!ContentType)
	{
		printf("Response formatting error3 \r\n");
		goto http_return;
	}
	templine = (uint8_t *)strstr((char *)ContentType,"\r\n");
	if(!templine)
	{
		printf("Response formatting error4 \r\n");
		goto http_return;
	}
	mymemcpy(temp,ContentType,templine-ContentType);
	ContentType = (uint8_t *)strstr((char *)temp,"/");
	if(!ContentType)
	{
		printf("not find type\r\n");
		goto http_return;
	}
	ContentType++;//����/
	len_temp = strlen((char*)ContentType);
	HttpRespon.ContentType = (uint8_t *)mymalloc(SRAMIN,len_temp+1);//��һ��������������鿴
	memset(HttpRespon.ContentType,0, len_temp+1);
	memcpy(HttpRespon.ContentType,ContentType,len_temp);
	printf("ContentType:%s\r\n",HttpRespon.ContentType);
	
	memset(temp,0,sizeof(temp));
	ContentLength = (uint8_t *)strstr((char *)data,"Content-Length");
	if(!ContentLength)
	{
		printf("not find ! ContentLength\r\n");
		goto http_return;
	}
	//�� ContentLength ��ʼ�����ҵ���һ�����з�
	templine = (uint8_t *)strstr((char *)ContentLength,"\r\n");
	if(!templine)
	{
		printf("Response formatting error5 \r\n");
		goto http_return;
	}
	//��֪��Ϊʲô������memcpy���Ƶľͳ���
	mymemcpy(temp,ContentLength,templine-ContentLength);
	//memcpy(temp,ContentLength,templine-ContentLength);
	//��ֹ��ЩContent-Length�����ǿո� ð�� �ո� ���ȣ���forѭ���ҵ���һ����������atoi
	for (i = 0; i < templine - ContentLength; i++)
	{
		if (temp[i] >= '0' && temp[i] <= '9')
		{
			break;
		}
	}
	HttpRespon.DataLength = atoi((char *) &temp[i]);
	printf("DataLength=%d\r\n",HttpRespon.DataLength);
	
	mymemset(temp,0,sizeof(temp));
	ContentDisposition = (uint8_t *)strstr((char *)data,"Content-Disposition");
	if(!ContentDisposition)//û�и�Ĭ���ļ���
	{
		printf("ContentDisposition == NULL\r\n");
		Generate_Random_Name();
	}
	else//��������Ĭ���ļ���
	{
		//�� ContentLength ��ʼ�����ҵ���һ�����з�
		templine = (uint8_t *)strstr((char *)ContentDisposition,"\r\n");
		if(!templine)
		{
			printf("templine == NULL\r\n");
			Generate_Random_Name();
			
		}
		else
		{
			mymemcpy(temp,ContentDisposition,templine-ContentDisposition);
			//�ҵ��ļ���
			ContentDisposition = (uint8_t *)strstr((char *)&temp[0],"filename=\"");
			if(!ContentDisposition)
			{
				printf("filename == NULL\r\n");
				Generate_Random_Name();
				goto http_return;
			}
			ContentDisposition += 10;//���� filename="
			templine = (uint8_t *)strstr((char *)ContentDisposition,"\"");
			if(!templine)
			{
				printf("\" == NULL\r\n");
				Generate_Random_Name();
				goto http_return;
			}
			len_temp = templine - ContentDisposition;
			HttpRespon.ContentDisposition = (uint8_t*)mymalloc(SRAMIN,len_temp+1);
			mymemset(HttpRespon.ContentDisposition,0, len_temp+1);
			mymemcpy(HttpRespon.ContentDisposition,ContentDisposition, len_temp);
			printf("use default name:%s\r\n",HttpRespon.ContentDisposition);
		}
	}
	
	if(HttpRespon.DataLength)//�������ݲű���
	{
		templine = 	(uint8_t *)strstr((char *)data,"\r\n\r\n");
		if(!templine)
		{
			printf("Response formatting error6 \r\n");
			goto http_return;
		}
		
		templine += 4;
		i = Http_Save_Date(templine,HttpRespon.DataLength);
		if(i)
		{
			printf("data save err \r\n");
			goto http_return;
		}
        BackGroundCtrl.HttpRespone=1;
	}
http_return:
	myfree(SRAMIN,HttpRespon.ContentDisposition);
	myfree(SRAMIN,HttpRespon.ContentType);
}

/**
****************************************************************************************
@brief:    AnalysisList �����ļ��б�
@Input��   NULL
@Output��  NULL
@Return��  NULL
@Warning:  NULL   
@note:     ���� 2022-7-11
****************************************************************************************
 **/
 void AnalysisList(uint8_t *data)
{
    char* pData;
    char temp[32];
    int number,cnt,i;
    pData = (char *)data;

    printf("pData = %s", pData);
    memset(temp,0,sizeof(temp));
    sscanf(pData,"number:%d\r\n", &number);
    printf("number=%d\r\n", number);
    if(PictureList.number)
    {
        printf("clear last list\r\n");
        for( i=0;i<PictureList.number;i++)
    	{
    		myfree(SRAMIN,PictureList.Name[i]);
    	}
    	myfree(SRAMIN,PictureList.Name);
    	PictureList.number=0;
    }



    PictureList.Name=(uint8_t**)mymalloc(SRAMIN,number*sizeof(uint8_t*));
	PictureList.number = number;
    while(number)
    {
        pData = strstr(pData, "\r\n");
        pData += 2;
        memset(temp, 0, sizeof(temp));
        cnt = sscanf(pData, "%s\r\n", temp);
        if (!cnt)
        {
            break;
        }
		i=PictureList.number-number;
        PictureList.Name[i] =\
        mymalloc(SRAMIN, strlen((char *)temp));
        strcpy((char *)PictureList.Name[i],temp);
        printf("temp%d=%s\r\n", i, PictureList.Name[i]);
        number--;
    }
}
  /**
 ****************************************************************************************
 @brief:    FoundMax Ѱ�������е����ֵ
 @Input��   NULL
 @Output��  NULL
 @Return��  NULL
 @Warning:  NULL   
 @note:     ���� 2022-7-11
 ****************************************************************************************
  **/
uint32_t FoundMax(uint32_t *Array,uint32_t ArraySize)
{
    uint32_t max = 0,i;
    for(i=0; i<ArraySize; i++)
    {
        if(Array[i]>max)
        max = Array[i];
    } 
    return  max;
}
 /**
 ****************************************************************************************
 @brief:    Move_last_Picture ��֮ǰ��ͼƬ�ƶ���backup�ļ�����
 @Input��   NULL
 @Output��  NULL
 @Return��  NULL
 @Warning:  NULL   
 @note:     ���� 2022-7-11
 ****************************************************************************************
  **/
  void Move_last_Picture(pFile_Scan Filelist)
 {
    FRESULT res = FR_OK;
	FIL* pFile,*pFileBku;	//�ļ���Ϣ
	DIR* dir;  	//Ŀ¼
	uint8_t *BackupPath,*data,*path,*FileName;
    uint32_t num,MaxDataSize,br;
	if(!Filelist->file_num)
	{
        printf("not scan file\r\n");
        return;
    }
    /*******************************************
    ���´�����Ϊ�˼���ļ����Ƿ���ڣ�
    �����ھʹ����ļ���
    ***************************************/
   
    path = mymalloc(SRAMIN, 64);

    BackupPath = mymalloc(SRAMIN, 64);
    mymemset(BackupPath,0,sizeof(BackupPath));
    sprintf((char *)BackupPath,"%sbackup",SaveDir);
    printf("BackupPath=%s\r\n",BackupPath);
	dir=(DIR*)mymalloc(SRAMIN,sizeof(DIR));
	res=f_opendir(dir,(TCHAR *)BackupPath);
    if(res == FR_NO_PATH)
    {
        printf("creat path\r\n");
        res = f_mkdir((TCHAR *)BackupPath);
        if(res)
        {
            printf("mkdir err!\r\n");
            myfree(SRAMIN, BackupPath);
            myfree(SRAMIN, path);
            myfree(SRAMIN, dir);
            return;
        }
    }
	f_closedir(dir);

    /*******************************************
    ���´�����Ϊ���ƶ��ļ�
    ***************************************/
	pFile=(FIL*)mymalloc(SRAMIN,sizeof(FIL));
	if(!pFile)
	{
		
		printf("file struct malloc err!\r\n");
        myfree(SRAMIN, BackupPath);
        myfree(SRAMIN, path);
        myfree(SRAMIN, dir);
        myfree(SRAMIN, pFile);
        return;
	}
    pFileBku=(FIL*)mymalloc(SRAMIN,sizeof(FIL));
	if(!pFileBku)
	{
		
		printf("file backup struct malloc err!\r\n");
        myfree(SRAMIN, BackupPath);
        myfree(SRAMIN, path);
        myfree(SRAMIN, dir);
        myfree(SRAMIN, pFile);
        myfree(SRAMIN, pFileBku);
        return;
	}
    num = Filelist->file_num;
    MaxDataSize = FoundMax(Filelist->file_size,Filelist->file_num);
    data = mymalloc(SRAMEX,MaxDataSize+1);
    if(!data)
    {
        printf("malloc for data err!\r\n");
        myfree(SRAMIN, BackupPath);
        myfree(SRAMIN, path);
        myfree(SRAMIN, dir);
        myfree(SRAMIN, pFile);
        myfree(SRAMIN, pFileBku);
        myfree(SRAMEX, data);
        return;
    }
	while(num)
	{
        printf("move %s\r\n",Filelist->file_name[Filelist->file_num-num]);
        res = f_open(pFile, Filelist->file_name[Filelist->file_num-num], FA_READ);
        if(res)
        {
            printf("open %s failed\r\n",Filelist->file_name[Filelist->file_num-num]);
            continue;
        }
        mymemset(data,0,MaxDataSize+1);
        res = f_read(pFile, data, pFile->obj.objsize, &br);
        if(res)
        {
            printf("read %s failed\r\n",Filelist->file_name[Filelist->file_num-num]);
            continue;
        }
        f_close(pFile);
        mymemset(path,0,sizeof(path));
        FileName = (uint8_t *)strstr(Filelist->file_name[Filelist->file_num-num],".");
        while(FileName[0] != '/')
            FileName--;

        sprintf((char *)path,"%s%s",BackupPath,FileName);
        res = f_open(pFileBku, (TCHAR *)path, FA_WRITE|FA_CREATE_ALWAYS);
        if(res)
        {
            printf("open %s failed\r\n",path);
            continue;
        }
        res = f_write(pFileBku, data, pFile->obj.objsize, &br);
        if(res)
        {
            printf("write %s failed\r\n",path);
            continue;
        }
        f_close(pFileBku);

        //ɾ��Դ�ļ�
        res = f_unlink(Filelist->file_name[Filelist->file_num-num]);
        if(res)
        {
            printf("delete %s failed\r\n",Filelist->file_name[Filelist->file_num-num]);
            continue;
        }
		num--;
    }
	printf("move ok\r\n");
	myfree(SRAMIN, BackupPath);
	myfree(SRAMIN, path);
    myfree(SRAMIN, dir);
    myfree(SRAMIN, pFile);
    myfree(SRAMIN, pFileBku);
    myfree(SRAMEX, data);
    return;
}

/**
****************************************************************************************
@brief:    Updata_Picture ����ͼƬ״̬��
@Input��   NULL
@Output��  NULL
@Return��  NULL
@Warning:  NULL   
@note:     ���� 2022-7-10
****************************************************************************************
 **/
extern void test_ff(char* scan_dir,char* choose_file);
extern void test_ff_free(void);
void Updata_Picture(void)
{
    //�����ļ�״̬��
    static Updata_Pic_ENUM state=GET_LIST;
    //TimeoutCnt����
    static uint32_t TimeoutCnt,timeout;

    uint8_t *path,*data,*url;
    WM_HWIN hItem;
    UINT br;
    FIL fp;
    FRESULT fs;
    //��ֹ����ҵ���һ��Ҫ����������ͼƬ������ת״̬��
    if(!BackgroundFlag.UpdataPicture)
        return;
    if(state)
    {
        TimeoutCnt+=BKG_DELAY;
        if(TimeoutCnt>timeout)
        {
            printf("%s state timeout TimeoutCnt=%d timeout=%d\r\n",__func__,TimeoutCnt,timeout);
            TimeoutCnt=0;
            timeout=0;
            BackgroundFlag.UpdataPicture=0;
            return;
        }
    }
    switch(state)
    {
        case GET_LIST://��ȡ�ļ��б�
        {
            printf("GET_LIST\r\n");
            mymemset(SaveDir, 0, sizeof(SaveDir));
            sprintf((char *)SaveDir,"3:/download/");
            test_http_get();
            TimeoutCnt=0;
            timeout = BKG_DELAY*1000;//BKG_DELAYΪ10ms,���յȴ�10s
            state=WAIT_FOR_LIST;
        }break;
        case WAIT_FOR_LIST://�ȴ���̨�ظ�,�ظ������
        {
            if(BackGroundCtrl.HttpRespone)
            {
                printf("bkg respone start analysis\r\n");
       
                path = mymalloc(SRAMIN, 32);
                mymemset(path, 0, 32);
                sprintf((char *)path,"%s%s",SaveDir,UPDATA_PICTURE_PATH);
          
                fs = f_open(&fp,(TCHAR *)path,FA_READ);
                if(fs)
                {
                    printf("open %s failed:%d\r\n ",path,fs);
                    state=GET_LIST;
                    BackgroundFlag.UpdataPicture=0;
                    myfree(SRAMIN, path);
                    return;
                }
                //��Ϊ���ܻ��ֶ�����\r\n \0
                data = mymalloc(SRAMEX, fp.obj.objsize+3);
                
                if(!data )
                {
                    printf("malloc data failed\r\n ");
                    state=GET_LIST;
                    BackgroundFlag.UpdataPicture=0;
                    myfree(SRAMIN, path);
                    myfree(SRAMEX, data);
                    return;
                }
                mymemset(data,0,fp.obj.objsize+3);
                fs = f_read(&fp, data, fp.obj.objsize,&br);
                if(fs  )
                {
                    printf("read %s data failed:%d\r\n ",path,fs);
                    state=GET_LIST;
                    BackgroundFlag.UpdataPicture=0;
                    myfree(SRAMIN, path);
                    myfree(SRAMEX, data);
                    f_close(&fp);
                    return;
                }             
                data[fp.obj.objsize]=0x0d;
                data[fp.obj.objsize+1]=0x0a;
                
                printf("data:\r\n%s\r\n",data);
                AnalysisList(data);
                Move_last_Picture(&DownloadPicture);               
                f_close(&fp);
                PictureList.option = PictureList.number;
                
                hItem = WM_GetDialogItem(ViewHomework, GUI_ID_TEXT0);
        		TEXT_SetFont(hItem,&GUI_FontHZ16);
        		TEXT_SetTextColor(hItem,GUI_RED);
        		sprintf((char *)path,"��%d��/��%d������",\
                    PictureList.number-PictureList.option+1,PictureList.number);
        		TEXT_SetText(hItem,(char *)path);

                myfree(SRAMIN, path);
                myfree(SRAMEX, data);
                state=DOWNLOAD_PIC;             
                TimeoutCnt=0;
                timeout = BKG_DELAY*1000;//BKG_DELAYΪ10ms,���յȴ�10s;
                
            }
        }break;
        case DOWNLOAD_PIC://����ͼƬ
        {
            if(PictureList.option)
            {
                url = mymalloc(SRAMEX,64);
                mymemset(url, 0, sizeof(url));
                sprintf((char *)url,"%s/download/%s",Root_URL,PictureList.Name[PictureList.option-1]);
				printf("url = %s\r\n",url);
                http_get((char *)url);
				myfree(SRAMEX,url);
                state=WAIT_FOR_DOWNLOAD;             
                TimeoutCnt=0;
                timeout = BKG_DELAY*20000;//BKG_DELAYΪ10ms,���յȴ�200s;
            }
        }break;
        case WAIT_FOR_DOWNLOAD://����ͼƬ
        {
            if(BackGroundCtrl.HttpRespone)
            {
                printf("download ok\r\n");
				//��ʾ��Ϣ��ʾ
				hItem = WM_GetDialogItem(ViewHomework, GUI_ID_TEXT0);
				TEXT_SetFont(hItem,&GUI_FontHZ16);
				TEXT_SetTextColor(hItem,GUI_RED);
				
                //�ͷ��ڴ�
                myfree(SRAMIN, PictureList.Name[PictureList.option-1]);
                PictureList.option--;
                if(PictureList.option)//��������ͼƬ
                {
                    printf("download next %d\r\n",PictureList.option);     
                    //���������path
					path = mymalloc(SRAMIN, 32);
					mymemset(path, 0, 32);
					sprintf((char *)path,"��%d��/��%d������",\
						PictureList.number-PictureList.option+1,PictureList.number);
					TEXT_SetText(hItem,(char *)path);

					myfree(SRAMIN, path);
					
                    state=DOWNLOAD_PIC;             
                    TimeoutCnt=0;
                    timeout = BKG_DELAY*1000;//BKG_DELAYΪ10ms,���յȴ�10s;
                }
                else//����ͼƬ��ʾ�б�
                {
                    myfree(SRAMIN, PictureList.Name);
					path = mymalloc(SRAMIN, 32);
					mymemset(path, 0, 32);
					sprintf((char *)path,"�������!");
					TEXT_SetText(hItem,(char *)path);
					myfree(SRAMIN, path);
                    state=UPDATA_PIC_LIST;             
                    TimeoutCnt=0;
                    timeout = BKG_DELAY*1000;//BKG_DELAYΪ10ms,���յȴ�10s;
                    
                }
            }

        }break;
        case UPDATA_PIC_LIST://����ͼƬ��ʾ�б�
        {
            //�����֮ǰ�ļ�¼
            test_ff_free();
            test_ff("3:/download","jpg");
            PictureIndex=0;
			PaintPic=1;
			WM_InvalidateWindow(WM_Picture);//����ͼƬ
			TimeoutCnt=0;
            timeout = 0;//BKG_DELAYΪ10ms,���յȴ�10s;
            BackgroundFlag.UpdataPicture=0;
        }break;
    }
    
}
/**
****************************************************************************************
@brief:    MessageRxHandle �������ݽ���
@Input��   NULL
@Output��  NULL
@Return��  NULL
@Warning:  NULL   
@note:     ���� 2022-6-11
****************************************************************************************
 **/
void  MessageRxHandle(void)
{
	uint8_t * HttpRsp;
	uint32_t datalen;
	OS_ERR err;
	if(Usart3Data.USART3_RX_STA)//�յ����ݣ�����һ������
	{
		BackGroundCtrl.RxState = ((Usart3Data.USART3_RX_STA & RECEIVE_OK_MARK_U3)>>31);
		//�ȴ���ȫ�������
		if(BackGroundCtrl.RxState)
		{
			BackGroundCtrl.Message_rxLen = (Usart3Data.USART3_RX_STA & RECEIVE_LEN_MARK_U3);
			printf("Rx state: %d,Rx len:%d\r\n",BackGroundCtrl.RxState,BackGroundCtrl.Message_rxLen);
			printf("Rx data:%s\r\n",BackGroundCtrl.Message_RXBuffer);
			//��������յ�����
			//���󻥳��ź���,��ֹ���߳��������߳����޸�����
			OSMutexPend (&Usart3Data_RX_MUTEX,0,OS_OPT_PEND_BLOCKING,0,&err);
			//http����Ӧ������HTTP��ͷ������ҵ�
			HttpRsp = (uint8_t *)strstr((char *)BackGroundCtrl.Message_RXBuffer,"HTTP");
			if(HttpRsp)
			{
				datalen = BackGroundCtrl.Message_rxLen - (BackGroundCtrl.Message_RXBuffer - HttpRsp);
				HTTP_Handle(HttpRsp,datalen);
                
			}
            Usart3Data.USART3_RX_STA = 0;
            mymemset(BackGroundCtrl.Message_RXBuffer,0,BackGroundCtrl.MaxRxBufferLen);
			BackGroundCtrl.RxState = 0;
			BackGroundCtrl.Message_rxLen = 0;	
            //�ͷŻ����ź���
			OSMutexPost(&Usart3Data_RX_MUTEX,OS_OPT_POST_NONE,&err);
			
		}
	}

	
}
/**
****************************************************************************************
@brief:    MessageRxHandle �������ݽ���
@Input��   NULL
@Output��  NULL
@Return��  NULL
@Warning:  NULL   
@note:     ���� 2022-6-11
****************************************************************************************
 **/

void  BKG_Flag_Handle()
{
    if(BackgroundFlag.UpdataPicture)//����ͼƬ
    {
        Updata_Picture();
    }
    else if(BackgroundFlag.POST_send)//�ϴ�ͼƬ
    {

    }
}


 
 
 
 
 
