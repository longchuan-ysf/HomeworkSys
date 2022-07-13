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
	
	uint8_t Method;//收到数据的类型 收到的POST请求还是 GET的回复 或者自己POST的回复
}Http_Respon;



typedef struct 
{
	uint8_t* ExName;//文件名指针数组
	uint8_t *ContentType;//文件大小数组
}Http_File_Attribute;

typedef struct 
{
	uint8_t** Name;//文件名指针数组
	uint16_t number;//有多少图片
	uint16_t option;//当前正在操作的第几张照片
}Picture_List;

typedef enum 
{
	GET_LIST=0,
    WAIT_FOR_LIST,
    DOWNLOAD_PIC,
    WAIT_FOR_DOWNLOAD,
    UPDATA_PIC_LIST,
    
}Updata_Pic_ENUM;


//http版本
#define HTTP1_1 "HTTP/1.1"
#define HTTP2_0 "HTTP/2.0"
						
#define UPLOAD_END  "Content-Disposition: form-data; name=\"file\"; filename=\"\"\r\n"\
					"Content-Type: application/octet-stream\r\n\r\n"

					
#define BOUNDARY		"----WebKitFormBoundaryO2WAD3vh5VDNQCJf"


                

extern void MessageRxHandle(void);
extern void BKG_Flag_Handle(void);


#endif

