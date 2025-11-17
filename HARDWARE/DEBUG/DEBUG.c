#include "DEBUG.h"
#include "USART_DEBUG.h"

USART_DEBUG DEBUG;
//标准库需要的支持函数                 
struct __FILE 
{ 
	int handle; 

}; 
FILE __stdout;       
//定义_sys_exit()以避免使用半主机模式    
void _sys_exit(int x) 
{ 
	x = x; 
} 
//重定义fputc函数 
int fputc(int ch, FILE *f)
{ 
	while((USART1->SR&0X40)==0){};//循环发送,直到发送完毕   
	USART1->DR = (u8) ch; 	 
	return ch;
}

uint8_t GetAckData(uint8_t *string)
{
    uint16_t i=0;
    for (i=0;i<1024; i++)
    {
      if (memcmp(string, &DEBUG.RxBuf[i], strlen((const char *)string)) == 0) 
      {
        DEBUG.RxCount=0;
        return 1;
      }
    }
    DEBUG.RxCount=0;         
    return 0;
}

void DEBUG_ReceiveData(void)    
{
    if(DEBUG.TimCount >= 5 && DEBUG.UartReceiving)                                    
    {  
      DEBUG.TimCount = 0;                                                             
      if(DEBUG.ANum == DEBUG.RxCount)                                                 
      {
        DEBUG.UartDataFinishFlag = 1;                                                 
        DEBUG.UartReceiving = 0;                                                   
      }
      else
      {
        DEBUG.ANum = DEBUG.RxCount;
      }
    }
}


void USART1_IRQHandler(void)
{
	static u8 DEBUG_DATA;
	if(USART1->SR&(1<<5))//接收到数据
	{	  
    //接收数据
		DEBUG_DATA = USART1->DR;  //读出数据清除中断标志
		
	  DEBUG.TimCount=0;
	  DEBUG.UartReceiving = 1;    
    if (DEBUG.RxCount < sizeof(DEBUG.RxBuf)-5)  
    {
      DEBUG.RxBuf[DEBUG.RxCount++] = DEBUG_DATA;
    }
		else 
		{
			DEBUG.RxCount = 0;                       
		}			
	} 
}
//初始化IO 串口1
//pclk2:PCLK2时钟频率(Mhz)
//bound:波特率 
void DEBUG_Init(u32 pclk2,u32 bound)
{  	 
	float temp;
	u16 mantissa;
	u16 fraction;	   
	
	temp=(float)(pclk2*1000000)/(bound*16);//得到USARTDIV@OVER8=0
	mantissa=temp;				 //得到整数部分
	fraction=(temp-mantissa)*16; //得到小数部分@OVER8=0 
    mantissa<<=4;
	mantissa+=fraction; 
	RCC->AHB1ENR|=1<<0;   	//使能PORTA口时钟  
	RCC->APB2ENR|=1<<4;  	 //使能串口1时钟 
	GPIO_Set(GPIOA,PIN9|PIN10,GPIO_MODE_AF,GPIO_OTYPE_PP,GPIO_SPEED_50M,GPIO_PUPD_PU);//PA9,PA10,复用功能,上拉输出
 	GPIO_AF_Set(GPIOA,9,7);	//PA9,AF7
	GPIO_AF_Set(GPIOA,10,7);//PA10,AF7  	   
	//波特率设置
 	USART1->BRR=mantissa; 	//波特率设置	 
	USART1->CR1&=~(1<<15); 	//设置OVER8=0 
	USART1->CR1|=1<<3;  	//串口发送使能 
	//使能接收中断 
	USART1->CR1|=1<<2;  	//串口接收使能
	USART1->CR1|=1<<5;    	//接收缓冲区非空中断使能	    	
	MY_NVIC_Init(3,3,USART1_IRQn,2);//组2，最低优先级 
	USART1->CR1|=1<<13;  	//串口使能
}


//DEBUG发送一个字节
void DEBUG_Send_Byte(uint8_t Dat)
{
	USART1->SR|=1<<6;
	USART1->DR=Dat; 
	while((USART1->SR&0X40)==0);//等待发送结束
}
//DEBUG发送数组
void DEBUG_Send_array(uint8_t *arr, uint16_t len)
{
	uint16_t i=0;
	for(i=0; i<len; i++)
	{
		DEBUG_Send_Byte(arr[i]);
	}
}





void Debug_Analyse(void)
{
	DEBUG_ReceiveData();
	if(DEBUG.UartDataFinishFlag==1)
	{
		DEBUG.UartDataFinishFlag=0;
		if(GetAckData((void *)"OK"))
		{
			memset(DEBUG.RxBuf, 0, 1024);	
			DEBUG.RxCount=0;
			DEBUG.TimCount=0;
			DEBUG.UartDataFinishFlag=0;
		}
	 }
}
 
					




