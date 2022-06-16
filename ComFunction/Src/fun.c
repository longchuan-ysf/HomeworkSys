/*************************************************************************************************** 
                                   xxxx公司
  
                  

文件:   fun.c 
作者:   龙川
说明:   常见函数的实现
***************************************************************************************************/
#include "fun.h"
#include "stdio.h"
#include "string.h"
 /**
****************************************************************************************
@brief:    printf_Hexbuffer 
@Input：   *data 发送数据Buffer指针
		   Len  需要取得数据长度
@Output：  NULL
@Return    NULL
@Warning:  NULL
@note:     龙川2019-4-15
****************************************************************************************
 **/
 
 void printf_Hexbuffer(uint8_t *buffer,uint32_t len)
 {
	uint32_t  i;
	uint8_t   data[16];
	for(i=0;i<len;i++)
	 {
		sprintf((char  *)&data,"0x%02x ",buffer[i]);
		printf("%s",data);
		if(((i+1)%16 == 0))
		{
			printf("\r\n");
		}
	 }
	 printf("\r\n");
 }

  /**
****************************************************************************************
@brief:    CheckSum 计算校验和 
@Input：    *data 数据
	       bufferLen  数据长度
@Output：  NULL
@Return    NULL
@Warning:  NULL
@note:     龙川2019-4-15
****************************************************************************************
 **/
 unsigned  char CheckSum(const unsigned  char *buffer,unsigned  int bufferLen)
{
	unsigned  int Sum;
	unsigned  int i;
	
	if(!buffer)
	{
		return 0;
	}
	Sum = 0;
	for(i=0; i<bufferLen; i++)
	{
		Sum = Sum + *buffer;
		buffer ++;
	}
	
	return Sum&0xff;
}



/**
****************************************************************************************
@brief:   StrFloatToInt
@Input：   *pSrc 字符串指针
			FloatPoint 浮点位置
@Output： NULL
@Return   NULL
@Warning: NULL
@note:  屏幕中将定点小数转换成整数 
		比如 
		StrFloatToInt((uint8_t *)"1.23" ,2) = 123//正数整数位大于1 要求浮点数等于给定浮点数
		StrFloatToInt((uint8_t *)"1.2345" ,2) = 123//正数整数位大于1 要求浮点数小于给定浮点数
		StrFloatToInt((uint8_t *)"1.2" ,2) = 120//正数整数位大于1 要求浮点数大于于给定浮点数

		StrFloatToInt((uint8_t *)"0.23" ,2) = 23//正数整数位小于1 要求浮点数等于给定浮点数
		StrFloatToInt((uint8_t *)"0.2345" ,2) = 23
		StrFloatToInt((uint8_t *)"0.2" ,2) = 20

		StrFloatToInt((uint8_t *)"-1.23" ,2) = -123//负数
		StrFloatToInt((uint8_t *)"-1.2345" ,2) = -123
		StrFloatToInt((uint8_t *)"-1.2" ,2) = -120
		
		StrFloatToInt((uint8_t *)"-0.23" ,2) = -23
		StrFloatToInt((uint8_t *)"-0.2345" ,2) = -23
		StrFloatToInt((uint8_t *)"-0.2" ,2) = -20
****************************************************************************************
 **/

int32_t StrFloatToInt(uint8_t *pSrc,uint8_t FloatPoint)
{
	int32_t ret,i,len,FloatCnt,neg,deci;
	ret = 0;
	len = strlen((char *)pSrc);
	FloatCnt = 0;//小数位数计数
	neg = 0;//负数标记
	deci = 0;//小数标记
	for(i=0; i<len; i++)
	{
		if((*pSrc>=0x30)&&(*pSrc<=0x39))
		{
			ret = ret*10;
			ret += (*pSrc-0x30);		
			if(deci)//开始计数小数个数
			{	
				FloatCnt++;				
				if(FloatCnt>=FloatPoint)//等于指定的小数位数，退出，不管后面的数
				{
					break;
				}
				
			}
			
		}
		else if(*pSrc == '.')//小数点
		{
			if(!deci)
			{
				deci = 1;
				
			}
			else//字符串中第二次出现小数点，直接退出
			{
				break;
			}
			//如果输入的是1..234   那输出就是100后面几个0取决于FloatPoint
		}
		else if((*pSrc == '-')&&(i==0))
		{
			neg = 1;
		}
		else
		{
				break;
		}
		pSrc++;
	}
	//字符串的小数位不够指定的小数位，补0
	if(FloatCnt<FloatPoint)
	{
		for(i=0; i<FloatPoint-FloatCnt; i++)
		{
			ret=ret*10;
		}
	}
	if(neg)
		ret =-ret;
	return ret;
}












