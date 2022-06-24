/*************************************************************************************************** 
                                   xxxx公司
  
                  

文件:   MessageHandle.c 
作者:   龙川
说明:   协议报文处理——网络协议应用层
***************************************************************************************************/
#include "BackGround.h"
#include "MessageHandle.h"
#include "string.h"
#include "usart.h"
#include "malloc.h"
#include "common.h"
#include "usbh_app.h"
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

Http_Respon  HttpRespon;//http的响应头解析
File_Scan pbuf;//目录扫描结果存放
//文件拓展名和 ContentType-type之间的关系
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
  //检查参数
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

/* 确定文件名的首地址和尾地址 */

    line_start = strurl;
    line_end = (char *)strstr(line_start, "\r\n");
    if (line_end == NULL)
    {
        ret = 1;
        return ret;
    }

    /* 找到方法名 */
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
    /* 找到文件名所在的字符串 */
    start_temp++;
    end_temp = (char *)strstr(line_start, " HTTP");
    
    line_start = start_temp;
    line_end = end_temp;
    /*从字符串中提取文件名
    若为 /aaaa/bbb/ccc.txt提取为ccc.txt
    若为 ccc.txt 提取为ccc.txt
    若为 /aaaa/bbb/ 判断为无文件
    若为 /aaaa/bbb/ccc 判断为无文件

    下面开始不要动 line_end 调节 line_start指到倒数第一个/
    */
    end_temp = (char *)strstr(line_start, ".");
    /*为 /aaaa/bbb/ 或者 /aaaa/bbb/ccc 判断为无文件*/
    if(end_temp == NULL)
    {
        ret = 1;
        return ret;
    }
    //从 . 开始往前找 /
    while(end_temp-line_start)
    {
        end_temp--;//往前找一位
        if(end_temp[0] == '/')
        {
            line_start = end_temp;
        }
    }
    line_start++;
    //防止出现 /aa/bb/.的情况
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
@Input：  method 方法有GET POST
@Output：  NULL
@Return：  NULL
@Warning:  NULL   
@note:     http请求行和请求头部的处理
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
	//初始化
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
			//在发送之前还要表明发送内容的类型，数据长度、默认文件
		
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
@Input：   method 请求方法
		   data 数据
		   datalen 数据长度
@Output：  NULL
@Return：  NULL
@Warning:  NULL   
@note:     添加http请求数据，并发送
****************************************************************************************
 **/
void MessageTx(void)
{
	
	printf("BackGround send:%d\r\n data:\r\n%s\r\n",BackGroundCtrl.Message_TxLen,BackGroundCtrl.Message_TXBuffer);
	atk_8266_quit_trans();
	atk_8266_send_cmd("AT+CIPSEND","OK",200);         //开始透传  
	BackGroundCtrl.BackgroundSend(BackGroundCtrl.Message_TXBuffer,BackGroundCtrl.Message_TxLen);
	printf("BackGround send:ok\r\n");
	

}

void test_http_get()
{
	OS_ERR err;
	char url[] = "/HFS/test.jpg";
	OSMutexPend (&Usart3Data_TX_MUTEX,0,OS_OPT_PEND_BLOCKING,0,&err);//请求互斥信号量,防止多线程在其他线程中修改发送区数据
	MessageTxInit(GET,(uint8_t *)&url[0]);	
	MessageTx();
	OSMutexPost(&Usart3Data_TX_MUTEX,OS_OPT_POST_NONE,&err);//释放互斥信号量
	HttpRespon.Method = GET;//标记等待GET回复

}
 /**
****************************************************************************************
@brief:    Generate_Random_Name 生成随机名称
@Input：    NULL
@Output：   NULL
@Return：   NULL
@Warning:   NULL   
@note:     龙川 2022-6-10
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
@brief:    Http_Save_Date 保存http接受的数据
@Input：    NULL
@Output：   NULL
@Return：   NULL
@Warning:   NULL   
@note:     龙川 2022-6-11
****************************************************************************************
 **/

uint8_t Http_Save_Date(uint8_t *data,uint32_t len)
{
	u16 PathLen,timeout;
	uint8_t *filepath,*DataBuff;
    pUSBH_WR_MSG pMsgWR;

	PathLen = strlen((char *)HttpRespon.ContentDisposition)+strlen("3:/download/")+1;//1是结束符\0
	filepath = USBH_Malloc_Path(PathLen);	//申请内存
	sprintf((char *)filepath,"%s%s","3:/download/",HttpRespon.ContentDisposition);
    DataBuff = USBH_Malloc_WriteBuf(len);
    mymemcpy(DataBuff, data, len);
    pMsgWR = USBH_ApplyFor_WR(filepath,DataBuff,len,FA_WRITE|FA_CREATE_ALWAYS);
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
        if(timeout>10*1000)//10s的超时判断
        {
		    return 1;
        }
		delay_ms(10);//延时10ms
	}
}

//释放内存，在读取完信息后进行释放
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
扫描指定路径下的某种类型文件,扫描一种类型的文件就需要定义一个File_Scan
类型的结构体,在读出包含的信息后，调用Dir_Scan_Free释放内存
dir_path:文件夹路径，形如:"0:/APP/ICON"
use_file:扫描的后缀名,形如:"txt",若输入为"*"，则扫描目录下的所有存在后缀名的文件;
scanfile:已分配内存的句柄
*/
FRESULT Dir_Scan(const char* dir_path,const char* use_file,File_Scan* scanfile)
{
	FRESULT res = FR_OK;
	FILINFO* FileInfo;	//文件信息
	DIR* dir;  			//目录
	
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
			res = f_readdir(dir, FileInfo);     //读取目录下的一个文件
	        if ((res != FR_OK)||(FileInfo->fname[0]==0) ||(scanfile->file_num>DIR_MAX_NUM))
			{
				printf("f_readdir err %d\r\n",res);
				break;  						//错误了//到末尾了//达到最大读取数目,退出
			}
			if(FileInfo->fattrib&AM_DIR)continue;//跳过目录
			pstr=strrchr(FileInfo->fname,'.');	//从右向左查找'.'的位置
			if(pstr==NULL)continue;				//无后缀名的跳过
			pstr++;								//去掉'.'
			
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
@brief:    HTTP_Post_AddFileInfo 向发送数据添加必要文件属性描述
@Input：    文件名
@Output：   NULL
@Return：   NULL
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
	//添加boundary和必要的信息
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
	//开始分割线
	mymemset(boundary,0,128);
	sprintf((char *)boundary,"--%s\r\n",BOUNDARY);
	BoundaryLen = strlen((char *)boundary);
	
	//数据类型
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
	//默认文件名
	mymemset(ContentDisposition,0,128);
	sprintf((char *)ContentDisposition,"Content-Disposition: form-data; name=\"file\"; filename=\"%s\"\r\n",FileName);
	DispostionLen = strlen((char *)ContentDisposition);
	
	
	//结束信息
	mymemset(EndMsg,0,256);
	//                          分割线  结束信息  分割线
	sprintf((char *)EndMsg,"\r\n--%s\r\n%s\r\n--%s--\r\n",BOUNDARY,UPLOAD_END,BOUNDARY);
	EndMsgLen = strlen((char *)EndMsg);
	
	//数据长度= 分割线      + 数据类型       + 默认文件名    + 文件大小  + 结束符
	DateLen = BoundaryLen + ContentTypeLen + DispostionLen + FileLen + EndMsgLen;
	//DateLen = FileLen;
	mymemset(ContentLenTXT,0,128);
	sprintf((char *)ContentLenTXT,"Content-Length: %d\r\n\r\n",DateLen);
	ContentTxtLen = strlen((char *)ContentLenTXT);
	
	//加入数据长度字段
	mymemcpy(&BackGroundCtrl.Message_TXBuffer[BackGroundCtrl.Message_TxLen],ContentLenTXT,ContentTxtLen);
	BackGroundCtrl.Message_TxLen  += ContentTxtLen;
	
	//加入分隔符
	mymemcpy(&BackGroundCtrl.Message_TXBuffer[BackGroundCtrl.Message_TxLen],boundary,BoundaryLen);
	BackGroundCtrl.Message_TxLen  += BoundaryLen;
	
	//加入文件名
	mymemcpy(&BackGroundCtrl.Message_TXBuffer[BackGroundCtrl.Message_TxLen],ContentDisposition,DispostionLen);
	BackGroundCtrl.Message_TxLen  += DispostionLen;
	
	//加入文件属性
	mymemcpy(&BackGroundCtrl.Message_TXBuffer[BackGroundCtrl.Message_TxLen],ContentType,ContentTypeLen);
	BackGroundCtrl.Message_TxLen  += ContentTypeLen;
	
	
	//加入数据
	mymemcpy(&BackGroundCtrl.Message_TXBuffer[BackGroundCtrl.Message_TxLen],FileDate,FileLen);
	BackGroundCtrl.Message_TxLen  += FileLen;
	
	//加入结束信息
	mymemcpy(&BackGroundCtrl.Message_TXBuffer[BackGroundCtrl.Message_TxLen],EndMsg,EndMsgLen);
	BackGroundCtrl.Message_TxLen  += EndMsgLen;
	
	myfree(SRAMIN,boundary);
	myfree(SRAMIN,ContentType);
	myfree(SRAMIN,ContentDisposition);
	myfree(SRAMIN,EndMsg);
	myfree(SRAMIN,ContentLenTXT);
    #else
    //初始化
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
@brief:    Http_Post_Date 向主机提交数据
@Input：    NULL
@Output：   NULL
@Return：   NULL
@Warning:   NULL   
@note:    这个函数一定不能和 MessageRxHandle 放在同一个线程，因为每次发送都要等待回复才能进行下一次发送
****************************************************************************************
 **/

void Http_Post_Date(uint8_t *FileName,uint8_t *url)
{

	OS_ERR err;    
    u16 PathLen,timeout;
	uint8_t *filepath;
    pUSBH_WR_MSG pMsgRD;


	OSMutexPend (&Usart3Data_TX_MUTEX,0,OS_OPT_PEND_BLOCKING,0,&err);//请求互斥信号量,防止多线程在其他线程中修改发送区数据

	
	#if TEST_POST==0
	MessageTxInit(POST,url);
	#endif
	PathLen = strlen((char *)FileName)+strlen("3:/update/")+1;//1是结束符\0
	filepath = USBH_Malloc_Path(PathLen);	//申请内存
	sprintf((char *)filepath,"3:/update/%s",FileName);
    pMsgRD = USBH_ApplyFor_WR(filepath,0,RD_ALL_DATA,FA_READ);
    timeout = 0;
	while(1)
	{
        timeout++;
		if(pMsgRD->result==FR_OK)
		{
			printf("read ok\r\n");
			break;
		}
        if(timeout>10*1000)//10s的超时判断
        {
            printf("timeout!!!!!!!\r\n");
		   goto post_return ; 
        }
		delay_ms(10);//延时10ms
	}	
	HTTP_Post_AddFileInfo(FileName,pMsgRD->data,pMsgRD->bread);	
    USBH_WR_MsgFree(pMsgRD);
	MessageTx();
post_return:
	myfree(SRAMIN,filepath);
	OSMutexPost(&Usart3Data_TX_MUTEX,OS_OPT_POST_NONE,&err);//释放互斥信号量	
	return ;
}

void test_http_post()
{
	HttpRespon.Method = POST;//标记等待GET回复
	OS_ERR err;
	OSMutexPend (&Usart3Data_TX_MUTEX,0,OS_OPT_PEND_BLOCKING,0,&err);//请求互斥信号量,防止多线程在其他线程中修改发送区数据
	Http_Post_Date("zhongli.jpg","/HFS/");

	OSMutexPost(&Usart3Data_TX_MUTEX,OS_OPT_POST_NONE,&err);//释放互斥信号量	
  
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
    result = f_open(&File,(const TCHAR*)"0:/update/zhongli.jpg",FA_READ);	//打开文件

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
@brief:    HTTP_Handle 解析http响应
@Input：    data 数据
           Len  数据长度
@Output：   NULL
@Return：   NULL
@Warning:  NULL   
@note:     龙川 2022-6-10
****************************************************************************************
 **/
void HTTP_Handle(uint8_t *data,uint16_t len)
{
	static uint8_t temp[128];
	uint8_t* StateLine;//状态栏
	uint8_t* ContentType;//数据类型
	uint8_t* ContentLength;//数据长度
	uint8_t* ContentDisposition;//默认文件名
	uint8_t* templine;
	uint8_t len_temp,i;//为了方便调试而取的中间变量
	mymemset(&HttpRespon, 0, sizeof(HttpRespon));
	
	//获取状态栏的数据以解析 不能直接查OK，万一消息头中存在OK，而是要在状态栏里找OK
	mymemset(temp, 0, sizeof(temp));
	StateLine = (uint8_t*)strstr((char*)data, "HTTP");
	if(!StateLine)
	{
		printf("Response formatting error1 \r\n");
		goto http_return;
	}
	
	//从 StateLine 开始往下找的以一个换行符
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
		goto http_return;//POST收到回复OK后就不需要关心数据
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
	ContentType++;//跳过/
	len_temp = strlen((char*)ContentType);
	HttpRespon.ContentType = (uint8_t *)mymalloc(SRAMIN,len_temp+1);//多一个结束符，方便查看
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
	//从 ContentLength 开始往下找的以一个换行符
	templine = (uint8_t *)strstr((char *)ContentLength,"\r\n");
	if(!templine)
	{
		printf("Response formatting error5 \r\n");
		goto http_return;
	}
	//不知道为什么这里用memcpy复制的就出错
	mymemcpy(temp,ContentLength,templine-ContentLength);
	//memcpy(temp,ContentLength,templine-ContentLength);
	//防止有些Content-Length后面是空格 冒号 空格 长度，先for循环找到第一个数字再用atoi
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
	if(!ContentDisposition)//没有给默认文件名
	{
		printf("ContentDisposition == NULL\r\n");
		Generate_Random_Name();
	}
	else//主机给出默认文件名
	{
		//从 ContentLength 开始往下找的以一个换行符
		templine = (uint8_t *)strstr((char *)ContentDisposition,"\r\n");
		if(!templine)
		{
			printf("templine == NULL\r\n");
			Generate_Random_Name();
			
		}
		else
		{
			mymemcpy(temp,ContentDisposition,templine-ContentDisposition);
			//找到文件名
			ContentDisposition = (uint8_t *)strstr((char *)&temp[0],"filename=\"");
			if(!ContentDisposition)
			{
				printf("filename == NULL\r\n");
				Generate_Random_Name();
				goto http_return;
			}
			ContentDisposition += 10;//跳过 filename="
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
	
	if(HttpRespon.DataLength)//存在数据才保存
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
@brief:    MessageRxHandle 接收数据解析
@Input：   NULL
@Output：  NULL
@Return：  NULL
@Warning:  NULL   
@note:     龙川 2022-6-11
****************************************************************************************
 **/
void  MessageRxHandle(void)
{
	uint8_t * HttpRsp;
	uint32_t datalen;
	OS_ERR err;
	if(Usart3Data.USART3_RX_STA)//收到数据，但不一定收完
	{
		BackGroundCtrl.RxState = ((Usart3Data.USART3_RX_STA & RECEIVE_OK_MARK_U3)>>31);
		//等待完全接受完毕
		if(BackGroundCtrl.RxState)
		{
			BackGroundCtrl.Message_rxLen = (Usart3Data.USART3_RX_STA & RECEIVE_LEN_MARK_U3);
			printf("Rx state: %d,Rx len:%d\r\n",BackGroundCtrl.RxState,BackGroundCtrl.Message_rxLen);
			printf("Rx data:%s\r\n",BackGroundCtrl.Message_RXBuffer);
			//处理完接收的数据
			OSMutexPend (&Usart3Data_RX_MUTEX,0,OS_OPT_PEND_BLOCKING,0,&err);//请求互斥信号量,防止多线程在其他线程中修改发送区数据
			//http的响应都是以HTTP开头，如果找到
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
			OSMutexPost(&Usart3Data_RX_MUTEX,OS_OPT_POST_NONE,&err);//释放互斥信号量
			
		}
	}

	
}
 

 
 
 
 
 
