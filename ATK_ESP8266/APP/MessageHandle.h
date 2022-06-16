/***************************************************************************************************
                                   xxxxx��˾


�ļ�:   Message.h
����:
˵��:   ��̨���Ĵ���
***************************************************************************************************/
#ifndef __MESSSAGEHANDLE_H__                     //���ذ���.
#define __MESSSAGEHANDLE_H__
/**************************************************************************************************/

/***************************************************************************************************
����:   ͷ�ļ�.
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
	200 OK                        //�ͻ�������ɹ�
	400 Bad Request               //�ͻ����������﷨���󣬲��ܱ������������
	401 Unauthorized              //����δ����Ȩ�����״̬��������WWW-Authenticate��ͷ��һ��ʹ�� 
	403 Forbidden                 //�������յ����󣬵��Ǿܾ��ṩ����
	404 Not Found                 //������Դ�����ڣ�eg�������˴����URL
	500 Internal Server Error     //��������������Ԥ�ڵĴ���
	503 Server Unavailable        //��������ǰ���ܴ���ͻ��˵�����һ��ʱ�����ָܻ�����
	*/
    uint16_t State;//״̬��
	uint8_t *ContentType;//��������
	uint8_t *ContentDisposition;//Ĭ���ļ���
	uint32_t DataLength;//���ݳ���
	
	uint8_t Method;//�յ���Ӧ������ 0:GET;1:POST
}Http_Respon;

//����Ч�ʿ���ʹ�ú궨��ָ��Ŀ¼�¿�ɨ�������ļ�����
#define DIR_MAX_NUM				255
typedef struct 
{
	char** file_name;//�ļ���ָ������
	uint32_t *file_size;//�ļ���С����
	uint32_t file_num;//��Ч���ļ�������
}File_Scan;

typedef struct 
{
	uint8_t* ExName;//�ļ���ָ������
	uint8_t *ContentType;//�ļ���С����
}Http_File_Attribute;
typedef struct 
{
    char picname[32];
    unsigned int picsize;
}PicParm_t;

#define MAX_SIZE_HEARER 2048

//http�汾
#define HTTP1_1 "HTTP/1.1"
#define HTTP2_0 "HTTP/2.0"


//hear��ͨͷ
#define HTTP_DEFAULT_HEADER         "User-Agent: Mozilla/5.0 (Windows NT 6.1; WOW64; rv:63.0) Gecko/20100101 Firefox/63.0\r\n"   \
				                    "Connection: keep-alive\r\n" \
                                    "Cache-Control: no-cache\r\n"           \
                                    "Content-Type: application/json\r\n"   \
                                    "Accept: */*"


//http�ϴ��ļ�����Ҫ��ͷ//"enctype: multipart/form-data\r\n"
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
    char url[128]; //����������������url�д���
    int  ispostpic; //�Ƿ���Ҫ�ϴ�ͼƬ
    int  port;
    int  method;
    int  timeouts;
    int  timeoutus;
    char *content; //��Ҫ����ռ䣬�������
    PicParm_t picparm; //ͼƬ����
}HttpParam_t;

extern void  MessageRxHandle(void);


#endif

