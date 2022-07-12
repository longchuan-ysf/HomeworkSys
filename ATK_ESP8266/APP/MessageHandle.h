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
	
	uint8_t Method;//�յ����ݵ����� �յ���POST������ GET�Ļظ� �����Լ�POST�Ļظ�
}Http_Respon;



typedef struct 
{
	uint8_t* ExName;//�ļ���ָ������
	uint8_t *ContentType;//�ļ���С����
}Http_File_Attribute;

typedef struct 
{
	uint8_t** Name;//�ļ���ָ������
	uint16_t number;//�ж���ͼƬ
}Picture_List;

typedef enum 
{
	GET_LIST=0,
    WAIT_FOR_LIST,
    DOWNLOAD_PIC,
    WAIT_FOR_DOWNLOAD,
    UPDATA_PIC_LIST,
    
}Updata_Pic_ENUM;


//http�汾
#define HTTP1_1 "HTTP/1.1"
#define HTTP2_0 "HTTP/2.0"
						
#define UPLOAD_END  "Content-Disposition: form-data; name=\"file\"; filename=\"\"\r\n"\
					"Content-Type: application/octet-stream\r\n\r\n"

					
#define BOUNDARY		"----WebKitFormBoundaryO2WAD3vh5VDNQCJf"


                

extern void MessageRxHandle(void);
extern void BKG_Flag_Handle(void);


#endif

