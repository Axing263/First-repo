#include "sys.h"
#include "dma.h"	
u32 GBK_OVER_Flag=0; //时间计数器
u8 GBK_BUF_Flag =2;	 //缓冲区标志
u8 Usart1_Rece_Buf0[Usart1_DMA_Len+1];
u8 Usart1_Rece_Buf1[Usart1_DMA_Len+1];
//--------------------------------------------------------------------------------------------
//DMAx的各通道配置
//这里的传输形式是固定的,这点要根据不同的情况来修改
//从存储器->外设模式/8位数据宽度/存储器增量模式
//DMA_Streamx:DMA数据流,DMA1_Stream0~7/DMA2_Stream0~7
//chx:DMA通道选择,范围:0~7
//par:外设地址
//mar0:存储器0地址
//mar1:存储器1地址
//ndtr:数据传输量  
void MYDMA_Config(DMA_Stream_TypeDef *DMA_Streamx,u8 chx,u32 par,u32 mar0,u32 mar1,u16 ndtr)
{ 
	DMA_TypeDef *DMAx;
	u8 streamx;
	
	USART1->CR3 |= 1<<6;			//使能串口1的DMA接收
	if((u32)DMA_Streamx>(u32)DMA2)//得到当前stream是属于DMA2还是DMA1
	{
		DMAx=DMA2;
		RCC->AHB1ENR|=RCC_AHB1ENR_DMA2EN;//DMA2时钟使能 
	}else 
	{
		DMAx=DMA1; 
 		RCC->AHB1ENR|=RCC_AHB1ENR_DMA1EN;//DMA1时钟使能 
	}
	while(DMA_Streamx->CR&0X01);	//等待DMA可配置 
	
	streamx=(((u32)DMA_Streamx-(u32)DMAx)-0X10)/0X18;		 //得到stream通道号
 	if(streamx>=6)DMAx->HIFCR|=0X3D<<(6*(streamx-6)+16);	 //清空之前该stream上的所有中断标志
	else if(streamx>=4)DMAx->HIFCR|=0X3D<<6*(streamx-4);     //清空之前该stream上的所有中断标志
	else if(streamx>=2)DMAx->LIFCR|=0X3D<<(6*(streamx-2)+16);//清空之前该stream上的所有中断标志
	else DMAx->LIFCR|=0X3D<<6*streamx;						 //清空之前该stream上的所有中断标志

	DMA_Streamx->CR=0;			    //先全部复位CR寄存器值 
	DMA_Streamx->PAR=par;		 	//DMA外设地址
	DMA_Streamx->M0AR=mar0; 		//DMA 存储器0地址
	DMA_Streamx->NDTR=ndtr;		 	//数据传输量
	
	DMA_Streamx->CR|=0<<6;			//外设到存储器模式
	DMA_Streamx->CR|=1<<8;			//注意：这里设置为循环模式，不然不能启动第二次传输
	DMA_Streamx->CR|=0<<9;			//外设非增量模式
	DMA_Streamx->CR|=1<<10;			//存储器增量模式
	DMA_Streamx->CR|=0<<11;			//外设数据长度:8位
	DMA_Streamx->CR|=0<<13;			//存储器数据长度:8位
	DMA_Streamx->CR|=1<<16;			//中等优先级
	DMA_Streamx->CR|=0<<21;			//外设突发单次传输
	DMA_Streamx->CR|=0<<23;			//存储器突发单次传输
	DMA_Streamx->CR|=(u32)chx<<25;  //通道选择
	DMA_Streamx->FCR=0<<2;			//FIFO 模式禁止
	DMA_Streamx->FCR=0X03;			//FIFO 阈值
	//使能DMA双缓冲模式
	DMA_Streamx->CR|=0<<19; 		//CT=0当前目标存储器为存储器0
	DMA_Streamx->M1AR=mar1;			//DMA 存储器1地址
	DMA_Streamx->CR|=1<<18;		    //DBM=1设置双缓存模式
	
	//DMA_Streamx->CR|=1<<0;		//EN=1开启DMA传输
	DMA_Streamx->CR|=1<<4;		//使能DMA传输完成中断TCIE使能
	DMAx->HIFCR|=1<<11; 		//清除TCIF5中断标志
	//DMA NVIC 配置
	MY_NVIC_Init(3,2,DMA2_Stream5_IRQn,2);//组2,抢占2,优先级2 DMA2_Stream5_IRQn中断通道
	DMA_Streamx->CR|=1<<0;		//EN=1开启DMA传输
} 
//重新开启一次DMA传输
//DMA_Streamx:DMA数据流,DMA1_Stream0~7/DMA2_Stream0~7 
//ndtr:数据传输量  
void MYDMA_REST_Enable(DMA_Stream_TypeDef *DMA_Streamx,u32 par,u32 mar,u16 ndtr)
{
	DMA_Streamx->CR&=~(1<<0); 	//关闭DMA传输 
	while(DMA_Streamx->CR&0X1);	//确保DMA可以被设置  
	DMA_Streamx->PAR=par;		//DMA外设地址
	DMA_Streamx->M0AR=mar; 		//DMA 存储器0地址
	DMA_Streamx->NDTR=ndtr;		//数据传输量
	DMA_Streamx->CR|=1<<0;		//开启DMA传输
}	  

/********************************************************************
*函数名称:void DMA2_Stream5_IRQHandler(void) 
*功    能:DMA2_Stream5_IRQHandler中断服务程序
*说    明:DMA2数据流5中断服务函数
*输入参数:无
*输出参数:无
*********************************************************************/  
void DMA2_Stream5_IRQHandler(void)                	
{
	if(DMA2->HISR&(1<<11))//判断TCIF5传输完成
	{
	   DMA2->HIFCR|=1<<11; //清除TCIF5中断标志
	   //***************数据接收完处理******************//
	   if(DMA2_Stream5->CR&(1<<19)) //得到CT当前目标存储器
	   {
			GBK_BUF_Flag=1; //存储器1
	   }
	   else
	   {
	   	    GBK_BUF_Flag=0; //存储器0
	   }
	   //**************************************************//
	}
} 



 

























