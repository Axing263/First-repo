#ifndef __DMA_H
#define	__DMA_H	   
#include "sys.h"	 
#define Usart1_DMA_Len 4096   //定义DMA缓冲区长度
#define BAUD_RATE    115200//波特率 
#define WATE_TIME    (((Usart1_DMA_Len*10000/BAUD_RATE)+10)*10)    //串口发送Usart1_DMA_Len长度字节所需要的时间，单位：ms
//----------------------------------------------------------------------------------------------
extern u32 GBK_OVER_Flag; 		//结束标志
extern u8 GBK_BUF_Flag;	 		//缓冲区标志
extern u8 Usart1_Rece_Buf0[];
extern u8 Usart1_Rece_Buf1[];
//----------------------------------------------------------------------------------------------------------
void MYDMA_Config(DMA_Stream_TypeDef *DMA_Streamx,u8 chx,u32 par,u32 mar0,u32 mar1,u16 ndtr);//DMAx的各通道配置
void MYDMA_REST_Enable(DMA_Stream_TypeDef *DMA_Streamx,u32 par,u32 mar,u16 ndtr);//重新开启一次DMA传输
#endif

