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
			sprintf((char *)p,"GET %s HTTP/1.0\r\n",url);
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
			sprintf((char *)p,"POST %s HTTP/1.0\r\n",url);
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
void MessageTx(uint8_t method,uint8_t *data,uint32_t datelen)
{
	
	printf("BackGround send:%d\r\n",BackGroundCtrl.Message_TxLen);
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
	MessageTx(GET,NULL,0);
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
	u16 PathLen;
	char *filepath;
	char result;
	int bread;
	FIL File;
	#if SYSTEM_SUPPORT_OS
		CPU_SR_ALLOC();
	#endif
	PathLen = strlen((char *)HttpRespon.ContentDisposition)+strlen("0:/download/")+1;//1是结束符\0
	filepath = mymalloc(SRAMIN,PathLen);	//申请内存
	mymemset(filepath,0,PathLen);
	sprintf(filepath,"%s%s","0:/download/",HttpRespon.ContentDisposition);
	printf("open %s\r\n",filepath);
	result = f_open(&File,(const TCHAR*)filepath,FA_WRITE|FA_CREATE_ALWAYS);	//打开文件

	if((result != FR_OK)) 	
	{
		printf("open %s  failed,result = %d\r\n",filepath,result);
		return 1;
	}	

	
	#if SYSTEM_SUPPORT_OS
		OS_CRITICAL_ENTER();		//临界区
	#endif
	//printf("start saving data\r\n");	
	result = f_write(&File,data,len,(UINT *)&bread); //读取数据
	if((result != FR_OK)) 	
	{
		printf("write %s  failed,result = %d\r\n",filepath,result);
		#if SYSTEM_SUPPORT_OS
		OS_CRITICAL_EXIT();	//退出临界区
		#endif
		return 1;
	}	

	
	#if SYSTEM_SUPPORT_OS
		OS_CRITICAL_EXIT();	//退出临界区
	#endif
	
	f_close(&File);			//关闭JPEGFile文件
	printf("save data ok !\r\n");
	return 0;
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
	//分割线
	mymemset(boundary,0,128);
	sprintf((char *)boundary,"--%s\r\n",BOUNDARY);
	BoundaryLen = strlen((char *)ContentDisposition);
	
	//数据类型
	DateLen = 0;
	pName = (uint8_t *)strrchr((char *)FileName,'.');
	pName++;	
	for(i=0; i<sizeof(HttpFileAttribute)/sizeof(Http_File_Attribute);i++)
	{
		if(!strcasecmp((char *)pName,(char *)HttpFileAttribute[i].ExName))
		{		
			mymemset(ContentType,0,128);
			sprintf((char *)ContentType,"Content-Type: %s\r\n",HttpFileAttribute[i].ContentType);
			ContentTypeLen = strlen((char *)ContentType);
			break;
		}
	}
	//默认文件名
	mymemset(ContentDisposition,0,128);
	sprintf((char *)ContentDisposition,"Content-Disposition: filename=\"%s\"\r\n\r\n",FileName);
	DispostionLen = strlen((char *)ContentDisposition);
	
	
	//结束信息
	mymemset(EndMsg,0,256);
	//                          分割线  结束信息  分割线
	sprintf((char *)EndMsg,"\r\n--%s\r\n%s\r\n--%s--\r\n",BOUNDARY,UPLOAD_END,BOUNDARY);
	EndMsgLen = strlen((char *)EndMsg);
	
	//数据长度= 分割线      + 数据类型       + 默认文件名    + 文件大小  + 结束符
	DateLen = BoundaryLen + ContentTypeLen + DispostionLen + FileLen + EndMsgLen;
	
	mymemset(ContentLenTXT,0,128);
	sprintf((char *)ContentLenTXT,"Content-Length: %d\r\n\r\n",DateLen);
	ContentTxtLen = strlen((char *)ContentLenTXT);
	
	//加入数据长度字段
	mymemcpy(&BackGroundCtrl.Message_TXBuffer[BackGroundCtrl.Message_TxLen],ContentLenTXT,ContentTxtLen);
	BackGroundCtrl.Message_TxLen  += ContentTxtLen;
	
	//加入分隔符
	mymemcpy(&BackGroundCtrl.Message_TXBuffer[BackGroundCtrl.Message_TxLen],boundary,BoundaryLen);
	BackGroundCtrl.Message_TxLen  += BoundaryLen;
	
	//加入文件属性
	mymemcpy(&BackGroundCtrl.Message_TXBuffer[BackGroundCtrl.Message_TxLen],ContentType,ContentTypeLen);
	BackGroundCtrl.Message_TxLen  += ContentTypeLen;
	
	//加入文件名
	mymemcpy(&BackGroundCtrl.Message_TXBuffer[BackGroundCtrl.Message_TxLen],ContentDisposition,DispostionLen);
	BackGroundCtrl.Message_TxLen  += DispostionLen;
	
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

	OSMutexPend (&Usart3Data_TX_MUTEX,0,OS_OPT_PEND_BLOCKING,0,&err);//请求互斥信号量,防止多线程在其他线程中修改发送区数据

	
	
	MessageTxInit(POST,url);
	
	PathLen = strlen((char *)FileName)+strlen("0:/update/")+1;//1是结束符\0
	filepath = mymalloc(SRAMIN,PathLen);	//申请内存
	
	mymemset(filepath,0,PathLen);
	sprintf(filepath,"0:/update/%s",FileName);
	
	
	fno = (FILINFO*)mymalloc(SRAMIN,sizeof(FILINFO));
	result = f_stat (filepath,fno);	
	if((result != FR_OK)) 	
	{
		printf("get %s state  failed,result = %d\r\n",filepath,result);
		goto post_return ;
	}	
	
	printf("open %s\r\n",filepath);
	result = f_open(&File,(const TCHAR*)filepath,FA_READ);	//打开文件

	if((result != FR_OK)) 	
	{
		printf("open %s  failed,result = %d\r\n",filepath,result);
		goto post_return ;
	}	

	

	pdata = mymalloc(SRAMEX,fno->fsize);
	//printf("start saving data\r\n");	
	result = f_read(&File,pdata,fno->fsize,(UINT *)&bread); //读取数据
	if((result != FR_OK)) 	
	{
		printf("write %s  failed,result = %d\r\n",filepath,result);

		goto post_return ; 
	}
	HTTP_Post_AddFileInfo(FileName,(uint8_t *)pdata,fno->fsize);	
	MessageTx(POST,NULL ,0);
post_return:
	f_close(&File);			//关闭文件
	
	myfree(SRAMIN,filepath);
	myfree(SRAMIN,fno);
	myfree(SRAMEX,pdata);
	OSMutexPost(&Usart3Data_TX_MUTEX,OS_OPT_POST_NONE,&err);//释放互斥信号量
		
	return ;
}
int makehttprotocol(HttpParam_t httpparam, char* httppack)
{
    char *header;  //需要异常判断大小
    char *startmsg;
    char *endmsg;
    char *request;
    char *http_boundary;
    int startboundarylen;
    int endboundarylen;

  //  int Content_Length;
    int headerlen;
   // int tolsize;
	int returnlen;
	
	returnlen = 0;
	header = mymalloc(SRAMEX,MAX_SIZE_HEARER);	//申请内存
	startmsg = mymalloc(SRAMEX,128);
	endmsg = mymalloc(SRAMEX,256);
	request = mymalloc(SRAMEX,256);
	http_boundary = mymalloc(SRAMEX,64);
	
    if (httpparam.method == GET)
    {

        if (httpparam.url[0] == 0) {
            sprintf(httppack, "GET / %s\r\n", HTTP1_1);
        }
        else {
            sprintf(httppack, "GET %s %s\r\n", httpparam.url, HTTP1_1);
        }

        //请求头： HTTP_DEFAULT_HEADER
        sprintf(httppack, "%s%s\r\n", httppack, HTTP_DEFAULT_HEADER);
        headerlen = sprintf(httppack, "%sHost: %s:%d\r\n", httppack, httpparam.host, httpparam.port);
        if (httpparam.content != NULL)
        {
            sprintf(httppack, "%sContent-Length: %d\r\n\r\n", httppack, httpparam.picparm.picsize);
        }
        else {
            sprintf(httppack, "%s\r\n", httppack);
        }
        returnlen = httpparam.picparm.picsize + headerlen;
		goto makehttprotocol_return;
    }
    else if (httpparam.method == POST)
    {
        if (httpparam.ispostpic == 0) //不上传图片，只是简单的post数据
        {
            if (httpparam.url[0] == NULL) {
                sprintf(httppack, "POST / %s\r\n", HTTP1_1);
            }
            else {
                sprintf(httppack, "POST %s %s\r\n", httpparam.url, HTTP1_1);
            }

            //请求头： HTTP_DEFAULT_HEADER
            sprintf(httppack, "%s%s\r\n", httppack, HTTP_DEFAULT_HEADER);
            headerlen = sprintf(httppack, "%sHost: %s:%d\r\n", httppack, httpparam.host, httpparam.port);
            if (httpparam.content != NULL)
            {
                sprintf(httppack, "%sContent-Length: %d\r\n\r\n", httppack, httpparam.picparm.picsize);
            }
            else {
                sprintf(httppack, "%s\r\n", httppack);
            }

            sprintf(httppack, "%s%s\r\n", httppack, httpparam.content);
            returnlen = httpparam.picparm.picsize + headerlen;
			goto makehttprotocol_return;

        }
        else if (httpparam.ispostpic == 1) //上传图片
        {

            long long int timestamp;
  
            timestamp = 112131233232;
            //1. 获取http_boundary
            sprintf((char *)http_boundary, "----------------------------%lld", timestamp);
            //2. start boundary
            startboundarylen = sprintf(startmsg, "--%s\r\n", http_boundary);
            //3. end boundart
            endboundarylen = sprintf(endmsg, "\r\n--%s\r\n"UPLOAD_END"\r\n--%s--\r\n", http_boundary,http_boundary);
            //4. 
			
            int requelen = sprintf(request, UPLOAD_REQUEST, httpparam.picparm.picname);
            int Content_Length = requelen + startboundarylen + httpparam.picparm.picsize + endboundarylen;
            int headerlen = sprintf(header, HTTP_POST_PIC_HEAD, httpparam.url, httpparam.host, http_boundary, Content_Length);
            int tolsize = Content_Length + strlen(HTTP_POST_PIC_HEAD) + 256; //加256防止溢出

            memcpy(httppack, header, headerlen);
            memcpy(httppack + headerlen, startmsg, startboundarylen);
            memcpy(httppack + headerlen + startboundarylen, request, requelen);
            memcpy(httppack + headerlen + startboundarylen + requelen, httpparam.content, httpparam.picparm.picsize);
            memcpy(httppack + headerlen + startboundarylen + requelen + httpparam.picparm.picsize, endmsg, endboundarylen);

            returnlen = tolsize;
			goto makehttprotocol_return;
        }

    }
makehttprotocol_return:
	 myfree(SRAMEX,header);	//申请内存
	 myfree(SRAMEX,startmsg);
	 myfree(SRAMEX,endmsg);
	 myfree(SRAMEX,request);
	 myfree(SRAMEX,http_boundary);
	
    return returnlen;
}

int http(HttpParam_t httpparam, char* content, int len)
{
    int rc = 0;
    char* httppack;
    httppack = (char *)mymalloc(SRAMEX,len + 512); //假设http头长度有512,http上传的最终头
    mymemset(httppack, 0, sizeof(len + 512));


   

    rc = makehttprotocol(httpparam, httppack);
    if (rc < 0)
    {
        myfree(SRAMEX,httppack);
        return rc;
    }
	mymemcpy(BackGroundCtrl.Message_TXBuffer,httppack,rc);
	BackGroundCtrl.Message_TxLen = rc;
	printf("BackGround send:%d\r\n",BackGroundCtrl.Message_TxLen);
	atk_8266_quit_trans();
	atk_8266_send_cmd("AT+CIPSEND","OK",200);         //开始透传  
	BackGroundCtrl.BackgroundSend(BackGroundCtrl.Message_TXBuffer,BackGroundCtrl.Message_TxLen);
	printf("BackGround send:ok\r\n");
    myfree(SRAMEX,httppack);
    return 0;
}
void test_http_post()
{
#if 1
	HttpRespon.Method = POST;//标记等待GET回复
	OS_ERR err;
	OSMutexPend (&Usart3Data_TX_MUTEX,0,OS_OPT_PEND_BLOCKING,0,&err);//请求互斥信号量,防止多线程在其他线程中修改发送区数据
	Http_Post_Date("zhongli.jpg","/HFS/");

	OSMutexPost(&Usart3Data_TX_MUTEX,OS_OPT_POST_NONE,&err);//释放互斥信号量
#else

	HttpParam_t httpparam;
	int PathLen,re;
	char * pdata;
	char *filepath;
	FRESULT result;
	UINT bread;
    memset(httpparam.host, 0, sizeof(httpparam.host));
    memset(httpparam.url, 0, sizeof(httpparam.url));
    strcpy(httpparam.host, "192.168.86.101");
    strcpy(httpparam.url, "/HFS/"); //后面的id是我们项目所需要的参数
    strcpy(httpparam.picparm.picname, "zhongli.jpg");
    httpparam.ispostpic = 1;
    httpparam.port = 80;
    httpparam.method = POST;
    httpparam.timeouts = 1;
    httpparam.timeoutus = 0;
	
	OS_ERR err;
	FIL File , File1;
	FILINFO* fno;

	#if SYSTEM_SUPPORT_OS
		CPU_SR_ALLOC();
	#endif

	OSMutexPend (&Usart3Data_TX_MUTEX,0,OS_OPT_PEND_BLOCKING,0,&err);//请求互斥信号量,防止多线程在其他线程中修改发送区数据
	#if SYSTEM_SUPPORT_OS
	OS_CRITICAL_ENTER();		//临界区
	#endif
	HttpRespon.Method = POST;//标记等待POST回复
	
	PathLen = strlen((char *)httpparam.picparm.picname)+strlen("0:/update/")+1;//1是结束符\0
	filepath = mymalloc(SRAMIN,PathLen);	//申请内存
	
	mymemset(filepath,0,PathLen);
	sprintf(filepath,"0:/update/%s",httpparam.picparm.picname);
	
	
	fno = (FILINFO*)mymalloc(SRAMIN,sizeof(FILINFO));
	result = f_stat (filepath,fno);	
	if((result != FR_OK)) 	
	{
		printf("get %s state  failed,result = %d\r\n",filepath,result);
		goto testpost_return ;
	}
	httpparam.picparm.picsize=fno->fsize;

	printf("get the size is: %d\r\n", httpparam.picparm.picsize);
	printf("open %s\r\n",filepath);
	result = f_open(&File,(const TCHAR*)filepath,FA_READ);	//打开文件

	if((result != FR_OK)) 	
	{
		printf("open %s  failed,result = %d\r\n",filepath,result);
		goto testpost_return ;
	}	
	
	printf("open %s OK,start read\r\n",filepath);

	httpparam.content = mymalloc(SRAMEX,fno->fsize);
	memset(httpparam.content, 0, sizeof(httpparam.picparm.picsize));
	
	result = f_read(&File,httpparam.content,fno->fsize,(UINT *)&bread); //读取数据
	if((result != FR_OK)) 	
	{
		printf("write %s  failed,result = %d\r\n",filepath,result);

		goto testpost_return ; 
	}
	result = f_open(&File1,(const TCHAR*)"0:/update/backup.jpg",FA_WRITE|FA_CREATE_ALWAYS);	//打开文件
	
	result = f_write(&File1,httpparam.content,fno->fsize,(UINT *)&bread); //读取数据
	
	printf("read %s OK,start make http\r\n",filepath);
    re = http(httpparam, httpparam.content, httpparam.picparm.picsize);

	testpost_return:
	f_close(&File);			//关闭文件
	f_close(&File1);			//关闭文件
	#if SYSTEM_SUPPORT_OS
		OS_CRITICAL_EXIT();	//退出临界区
	#endif
	myfree(SRAMIN,filepath);
	myfree(SRAMIN,fno);
	myfree(SRAMEX,pdata);
	myfree(SRAMEX,httpparam.content);
	OSMutexPost(&Usart3Data_TX_MUTEX,OS_OPT_POST_NONE,&err);//释放互斥信号量
#endif
	
	
    
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
 

 
 
 
 
 
