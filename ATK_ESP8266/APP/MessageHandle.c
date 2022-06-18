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

#define TEST_POST 0

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
File_Scan pbuf;//Ŀ¼ɨ�������
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
	char url[] = "/HFS/test.jpg";
	OSMutexPend (&Usart3Data_TX_MUTEX,0,OS_OPT_PEND_BLOCKING,0,&err);//���󻥳��ź���,��ֹ���߳��������߳����޸ķ���������
	MessageTxInit(GET,(uint8_t *)&url[0]);	
	MessageTx();
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
	u16 PathLen;
	char *filepath;
	char result;
	int bread;
	FIL File;
	#if SYSTEM_SUPPORT_OS
		CPU_SR_ALLOC();
	#endif
	PathLen = strlen((char *)HttpRespon.ContentDisposition)+strlen("0:/download/")+1;//1�ǽ�����\0
	filepath = mymalloc(SRAMIN,PathLen);	//�����ڴ�
	mymemset(filepath,0,PathLen);
	sprintf(filepath,"%s%s","0:/download/",HttpRespon.ContentDisposition);
	printf("open %s\r\n",filepath);
	result = f_open(&File,(const TCHAR*)filepath,FA_WRITE|FA_CREATE_ALWAYS);	//���ļ�

	if((result != FR_OK)) 	
	{
		printf("open %s  failed,result = %d\r\n",filepath,result);
		return 1;
	}	

	
	#if SYSTEM_SUPPORT_OS
		OS_CRITICAL_ENTER();		//�ٽ���
	#endif
	//printf("start saving data\r\n");	
	result = f_write(&File,data,len,(UINT *)&bread); 
	if((result != FR_OK)) 	
	{
		printf("write %s  failed,result = %d\r\n",filepath,result);
		#if SYSTEM_SUPPORT_OS
		OS_CRITICAL_EXIT();	//�˳��ٽ���
		#endif
		return 1;
	}	

	
	#if SYSTEM_SUPPORT_OS
		OS_CRITICAL_EXIT();	//�˳��ٽ���
	#endif
	
	f_close(&File);			//�ر�JPEGFile�ļ�
	printf("save data ok !\r\n");
	return 0;
}

//�ͷ��ڴ棬�ڶ�ȡ����Ϣ������ͷ�
void Dir_Scan_Free(File_Scan* pbuf)
{
	for(uint16_t i=0;i<pbuf->file_num;i++)
	{
		myfree(SRAMIN,pbuf->file_name[i]);
	}
	myfree(SRAMIN,pbuf->file_name);
	myfree(SRAMIN,pbuf->file_size);
	pbuf->file_num=0;
}
/*
ɨ��ָ��·���µ�ĳ�������ļ�,ɨ��һ�����͵��ļ�����Ҫ����һ��File_Scan
���͵Ľṹ��,�ڶ�����������Ϣ�󣬵���Dir_Scan_Free�ͷ��ڴ�
dir_path:�ļ���·��������:"0:/APP/ICON"
use_file:ɨ��ĺ�׺��,����:"txt",������Ϊ"*"����ɨ��Ŀ¼�µ����д��ں�׺�����ļ�;
scanfile:�ѷ����ڴ�ľ��
*/
FRESULT Dir_Scan(const char* dir_path,const char* use_file,File_Scan* scanfile)
{
	FRESULT res = FR_OK;
	FILINFO* FileInfo;	//�ļ���Ϣ
	DIR* dir;  			//Ŀ¼
	
	dir=(DIR*)mymalloc(SRAMIN,sizeof(DIR));
	
	res=f_opendir(dir,dir_path);
	printf("%s:open dir=%s,res=%d\r\n",__func__,dir_path,res);
	
	FileInfo=(FILINFO*)mymalloc(SRAMIN,sizeof(FILINFO));
	if(!FileInfo)
	{
		res = FR_NOT_ENOUGH_CORE; 
		printf("fileinfo malloc err!\r\n");
	}
	
	scanfile->file_name=(char**)mymalloc(SRAMIN,DIR_MAX_NUM*sizeof(char*));
	scanfile->file_size = (uint32_t *)mymalloc(SRAMIN,DIR_MAX_NUM*sizeof(uint32_t));
	
	if(!scanfile->file_name)
	{
		res = FR_NOT_ENOUGH_CORE; 
		printf("scanfile malloc err!\r\n");
	}
	
	scanfile->file_num=0;
	
	if(res==FR_OK)
	{
		while(1)
		{
			char* pstr;
			res = f_readdir(dir, FileInfo);     //��ȡĿ¼�µ�һ���ļ�
	        if ((res != FR_OK)||(FileInfo->fname[0]==0) ||(scanfile->file_num>DIR_MAX_NUM))
			{
				printf("f_readdir err %d\r\n",res);
				break;  						//������//��ĩβ��//�ﵽ����ȡ��Ŀ,�˳�
			}
			if(FileInfo->fattrib&AM_DIR)continue;//����Ŀ¼
			pstr=strrchr(FileInfo->fname,'.');	//�����������'.'��λ��
			if(pstr==NULL)continue;				//�޺�׺��������
			pstr++;								//ȥ��'.'
			
			if((strcasecmp(pstr,use_file)==0)||(use_file[0]=='*'))
			{

				scanfile->file_name[scanfile->file_num]=(char*)mymalloc(SRAMIN,strlen(FileInfo->fname)+1);
				mymemset(scanfile->file_name[scanfile->file_num],0,strlen(FileInfo->fname)+1);
				strcpy(scanfile->file_name[scanfile->file_num],FileInfo->fname);
				scanfile->file_size[scanfile->file_num] = FileInfo->fsize;
				
				scanfile->file_num++;
			}			
		}
	}
	f_closedir(dir);
	myfree(SRAMIN,dir);
	myfree(SRAMIN,FileInfo);
	return res;
}


void test_ff_free(void)
{
	Dir_Scan_Free(&pbuf);
}
void test_ff(char* scan_dir,char* choose_file)
{
	Dir_Scan(scan_dir,choose_file,&pbuf);
	for(uint8_t i=0;i<pbuf.file_num;i++)
	{
		printf("\r\n%s",scan_dir);
		printf("/%s",pbuf.file_name[i]);
	}
	test_ff_free();
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
	u16 PathLen;
	char *filepath;
	char *pdata;
	char result;
	int bread;
	OS_ERR err;
	FIL File;
	FILINFO* fno;

	#if SYSTEM_SUPPORT_OS
		CPU_SR_ALLOC();
	#endif

	OSMutexPend (&Usart3Data_TX_MUTEX,0,OS_OPT_PEND_BLOCKING,0,&err);//���󻥳��ź���,��ֹ���߳��������߳����޸ķ���������

	
	#if TEST_POST==0
	MessageTxInit(POST,url);
	#endif
    
	PathLen = strlen((char *)FileName)+strlen("0:/update/")+1;//1�ǽ�����\0
	filepath = mymalloc(SRAMIN,PathLen);	//�����ڴ�
	
	mymemset(filepath,0,PathLen);
	sprintf(filepath,"0:/update/%s",FileName);
	
	
	fno = (FILINFO*)mymalloc(SRAMIN,sizeof(FILINFO));
	result = f_stat (filepath,fno);	
	if((result != FR_OK)) 	
	{
		printf("get %s state  failed,result = %d\r\n",filepath,result);
		goto post_return ;
	}	
	
	printf("f_stat ok,filesize=%lld open %s\r\n",fno->fsize, filepath);
	result = f_open(&File,(const TCHAR*)filepath,FA_READ);	//���ļ�

	if((result != FR_OK)) 	
	{
		printf("open %s  failed,result = %d\r\n",filepath,result);
		goto post_return ;
	}	

	

	pdata = mymalloc(SRAMEX,fno->fsize);
	printf("open %s OK start read\r\n",filepath);
	result = f_read(&File,pdata,fno->fsize,(UINT *)&bread); //��ȡ����
	if((result != FR_OK)) 	
	{
		printf("write %s  failed,result = %d\r\n",filepath,result);

		goto post_return ; 
	}
	HTTP_Post_AddFileInfo(FileName,(uint8_t *)pdata,fno->fsize);	
	MessageTx();
post_return:
	f_close(&File);			//�ر��ļ�
	
	myfree(SRAMIN,filepath);
	myfree(SRAMIN,fno);
	myfree(SRAMEX,pdata);
	OSMutexPost(&Usart3Data_TX_MUTEX,OS_OPT_POST_NONE,&err);//�ͷŻ����ź���
		
	return ;
}

void test_http_post()
{
	HttpRespon.Method = POST;//��ǵȴ�GET�ظ�
	OS_ERR err;
	OSMutexPend (&Usart3Data_TX_MUTEX,0,OS_OPT_PEND_BLOCKING,0,&err);//���󻥳��ź���,��ֹ���߳��������߳����޸ķ���������
	Http_Post_Date("zhongli.jpg","/HFS/");

	OSMutexPost(&Usart3Data_TX_MUTEX,OS_OPT_POST_NONE,&err);//�ͷŻ����ź���	
  
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
	}
http_return:
	myfree(SRAMIN,HttpRespon.ContentDisposition);
	myfree(SRAMIN,HttpRespon.ContentType);
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
			OSMutexPend (&Usart3Data_RX_MUTEX,0,OS_OPT_PEND_BLOCKING,0,&err);//���󻥳��ź���,��ֹ���߳��������߳����޸ķ���������
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
			OSMutexPost(&Usart3Data_RX_MUTEX,OS_OPT_POST_NONE,&err);//�ͷŻ����ź���
			
		}
	}

	
}
 

 
 
 
 
 
