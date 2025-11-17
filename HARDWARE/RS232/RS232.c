#include "RS232.h"
//初始化IO 串口3
//pclk1:PCLK1时钟频率(Mhz)
//bound:波特率 
void RS232_Init(u32 pclk1,u32 bound)
{  	 
	float temp;
	u16 mantissa;
	u16 fraction;	   
	temp=(float)(pclk1*1000000)/(bound*16);//得到USARTDIV@OVER8=0
	mantissa=temp;				 //得到整数部分
	fraction=(temp-mantissa)*16; //得到小数部分@OVER8=0 
  mantissa<<=4;
	mantissa+=fraction; 
	RCC->AHB1ENR|=1<<0;   	//使能PORTA口时钟  
	RCC->APB1ENR|=1<<17;  	//使能串口17时钟 
	GPIO_Set(GPIOA,PIN2|PIN3,GPIO_MODE_AF,GPIO_OTYPE_PP,GPIO_SPEED_50M,GPIO_PUPD_PU);//PA2,PA3,复用功能,上拉输出
 	GPIO_AF_Set(GPIOA,2,7);	 //PA2,AF7
	GPIO_AF_Set(GPIOA,3,7);  //PA3,AF7  	   
	//波特率设置
 	USART2->BRR=mantissa; 	//波特率设置	 
	USART2->CR1&=~(1<<15); 	//设置OVER8=0 
	USART2->CR1|=1<<3;  	//串口发送使能 

	USART2->CR1|=1<<2;  	//串口接收使能
	USART2->CR1|=1<<5;    	//接收缓冲区非空中断使能	    	
	MY_NVIC_Init(2,3,USART2_IRQn,2);//组2，最低优先级 

	USART2->CR1|=1<<13;  	//串口使能
}


void USART2_IRQHandler(void)
{
	u8 res;	
	if(USART2->SR&(1<<5))//接收到数据
	{
	 res=USART2->DR;  //读出数据清除中断标志	 	
		USART2->DR=res; 
	 while((USART2->SR&0X40)==0);//等待发送结束						     
	} 
}



//RS232发送一个字节
void RS232_Send_Byte(uint8_t Dat)
{
	USART2->DR=Dat; 
	while((USART2->SR&0X40)==0);//等待发送结束
}
//RS232发送数组
void RS232_Send_array(uint8_t *arr, uint16_t len)
{
	uint16_t i=0;
	for(i=0; i<len; i++)
	{
		RS232_Send_Byte(arr[i]);
	}
}
