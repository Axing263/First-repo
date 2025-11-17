#ifndef USART_DEBUG__H
#define USART_DEBUG__H
#include "sys.h"


typedef struct
{
    uint32_t TimCount; 
    uint8_t  RxBuf[1024];       
    uint8_t  TxBuf[1024];
    uint16_t RxCount;      
    uint16_t ANum;   
    uint8_t  UartReceiving;
    uint8_t  UartDataFinishFlag;
} USART_DEBUG;


 
#endif




