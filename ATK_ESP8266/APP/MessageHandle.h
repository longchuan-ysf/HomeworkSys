/***************************************************************************************************
                                   xxxxx公司


文件:   Message.h
作者:
说明:   后台报文处理
***************************************************************************************************/
#ifndef __MESSSAGEHANDLE_H__                     //防重包含.
#define __MESSSAGEHANDLE_H__
/**************************************************************************************************/

/***************************************************************************************************
包含:   头文件.
***************************************************************************************************/
#include "stdint.h"
#define GET  0
#define POST 1
typedef struct http_parser_urt
{
    char filename[32];
    char method; /* 0:GET;1:POST */
}Http_URL;

typedef struct 
{
	/*
	200 OK                        //客户端请求成功
	400 Bad Request               //客户端请求有语法错误，不能被服务器所理解
	401 Unauthorized              //请求未经授权，这个状态代码必须和WWW-Authenticate报头域一起使用 
	403 Forbidden                 //服务器收到请求，但是拒绝提供服务
	404 Not Found                 //请求资源不存在，eg：输入了错误的URL
	500 Internal Server Error     //服务器发生不可预期的错误
	503 Server Unavailable        //服务器当前不能处理客户端的请求，一段时间后可能恢复正常
	*/
    uint16_t State;//状态栏
	uint8_t *ContentType;//数据类型
	uint8_t *ContentDisposition;//默认文件名
	uint32_t DataLength;//数据长度
	
	uint8_t Method;//收到响应的类型 0:GET;1:POST
}Http_Respon;

//出于效率考虑使用宏定义指定目录下可扫描的最大文件数量
#define DIR_MAX_NUM				255
typedef struct 
{
	char** file_name;//文件名指针数组
	uint32_t *file_size;//文件大小数组
	uint32_t file_num;//有效的文件名个数
}File_Scan;

typedef struct 
{
	uint8_t* ExName;//文件名指针数组
	uint8_t *ContentType;//文件大小数组
}Http_File_Attribute;
typedef struct 
{
    char picname[32];
    unsigned int picsize;
}PicParm_t;

#define MAX_SIZE_HEARER 2048

//http版本
#define HTTP1_1 "HTTP/1.1"
#define HTTP2_0 "HTTP/2.0"


//hear普通头
#define HTTP_DEFAULT_HEADER         "User-Agent: Mozilla/5.0 (Windows NT 6.1; WOW64; rv:63.0) Gecko/20100101 Firefox/63.0\r\n"   \
				                    "Connection: keep-alive\r\n" \
                                    "Cache-Control: no-cache\r\n"           \
                                    "Content-Type: application/json\r\n"   \
                                    "Accept: */*"


//http上传文件所需要的头//"enctype: multipart/form-data\r\n"
#define HTTP_POST_PIC_HEAD 	"POST %s HTTP/1.1\r\n"\
                            "User-Agent: PostmanRuntime/7.24.1\r\n"\
                            "Accept: */*\r\n"\
                            "Host: %s\r\n"\
                            "Accept-Encoding: gzip, deflate, br\r\n" \
                            "Connection: keep-alive\r\n" \
                            "Content-Type: multipart/form-data; boundary=%s\r\n"\
                            "Content-Length: %d\r\n\r\n"\

					
					
#define UPLOAD_REQUEST	"Content-Disposition: form-data; name=\"file\"; filename=\"%s\"\r\n" \
                        "Content-Type: image/jpeg\r\n\r\n"
						
#define UPLOAD_END		"Content-Disposition: form-data; name=\"file\"; filename=\"\"\r\n" \
                        "Content-Type: application/octet-stream\r\n\r\n"
					
#define BOUNDARY		"----WebKitFormBoundarysB16cMSV2A6aSwsK"
                        
typedef struct
{
    char host[16];
    char url[128]; //包含参数，参数从url中传入
    int  ispostpic; //是否需要上传图片
    int  port;
    int  method;
    int  timeouts;
    int  timeoutus;
    char *content; //需要申请空间，存放数据
    PicParm_t picparm; //图片参数
}HttpParam_t;

extern void  MessageRxHandle(void);


#endif

