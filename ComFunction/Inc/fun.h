/*************************************************************************************************** 
                                   xxxx公司
  
                  

文件:   fun.h
作者:   龙川
说明:   常见函数的实现
***************************************************************************************************/
#ifndef __FUN_H_
#define __FUN_H_
#include "stdint.h"

#define BCD_TO_HEX(n)  ((n>>4)*10 +(n&0xf))
#define HEX_TO_BCD(n)  (((n/10)<<4) + (n%10))


extern void printf_Hexbuffer(uint8_t *buffer,uint32_t len);
extern unsigned  char CheckSum(const unsigned  char *buffer,unsigned  int bufferLen);
extern int32_t StrFloatToInt(uint8_t *pSrc,uint8_t FloatPoint);
#endif







