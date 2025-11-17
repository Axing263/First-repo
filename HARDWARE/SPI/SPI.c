#include "spi.h"

//以下是SPI模块的初始化代码，配置成主机模式 						  
//SPI口初始化
//这里针是对SPI1的初始化
void SPI1_Init(void)
{	 
	u16 tempreg=0;
	RCC->AHB1ENR|=1<<0;    	//使能PORTA时钟	   
	RCC->APB2ENR|=1<<12;   	//SPI1时钟使能 
	GPIO_Set(GPIOA,7<<5,GPIO_MODE_AF,GPIO_OTYPE_PP,GPIO_SPEED_100M,GPIO_PUPD_PU);	//PA5~7复用功能输出	
  GPIO_AF_Set(GPIOA,5,5);	//PA5,AF5
 	GPIO_AF_Set(GPIOA,6,5);	//PA6,AF5
 	GPIO_AF_Set(GPIOA,7,5);	//PA7,AF5 

	//这里只针对SPI口初始化
	RCC->APB2RSTR|=1<<12;	//复位SPI1
	RCC->APB2RSTR&=~(1<<12);//停止复位SPI1
	tempreg|=0<<10;			//全双工模式	
	tempreg|=1<<9;			//软件nss管理
	tempreg|=1<<8;			 
	tempreg|=1<<2;			//SPI主机  
	tempreg|=0<<11;			//8位数据格式	
	tempreg|=1<<1;			//空闲模式下SCK为1 CPOL=1 
	tempreg|=1<<0;			//数据采样从第2个时间边沿开始,CPHA=1  
 	//对SPI1属于APB2的外设.时钟频率最大为84Mhz频率.
	tempreg|=7<<3;			//Fsck=Fpclk1/256
	tempreg|=0<<7;			//MSB First  
	tempreg|=1<<6;			//SPI启动 
	SPI1->CR1=tempreg; 		//设置CR1  
	SPI1->I2SCFGR&=~(1<<11);//选择SPI模式
	SPI1_ReadWriteByte(0xff);//启动传输		 
}   
//SPI1速度设置函数
//SpeedSet:0~7
//SPI速度=fAPB2/2^(SpeedSet+1)
//fAPB2时钟一般为84Mhz
void SPI1_SetSpeed(u8 SpeedSet)
{
	SpeedSet&=0X07;			//限制范围
	SPI1->CR1&=0XFFC7; 
	SPI1->CR1|=SpeedSet<<3;	//设置SPI1速度  
	SPI1->CR1|=1<<6; 		//SPI设备使能	  
} 
//SPI1 读写一个字节
//TxData:要写入的字节
//返回值:读取到的字节
u8 SPI1_ReadWriteByte(u8 TxData)
{		 			 
	while((SPI1->SR&1<<1)==0);		//等待发送区空 
	SPI1->DR=TxData;	 	  		//发送一个byte  
	while((SPI1->SR&1<<0)==0);		//等待接收完一个byte  
 	return SPI1->DR;          		//返回收到的数据				    
}






//以下是SPI模块的初始化代码，配置成主机模式 						  
//SPI口初始化
//这里针是对SPI2的初始化
void SPI2_Init(void)
{	 
	u16 tempreg=0;
	RCC->AHB1ENR|=1<<1;    	//使能PORTB时钟	   
	RCC->APB1ENR|=1<<14;   	//SPI2时钟使能 
	GPIO_Set(GPIOB,13<<5,GPIO_MODE_AF,GPIO_OTYPE_PP,GPIO_SPEED_100M,GPIO_PUPD_PU);	//PB13~15复用功能输出	
  GPIO_AF_Set(GPIOB,13,5);	//PB13,AF5
 	GPIO_AF_Set(GPIOB,14,5);	//PB14,AF5
 	GPIO_AF_Set(GPIOB,15,5);	//PB15,AF5 

	//这里只针对SPI口初始化
	RCC->APB1RSTR|=1<<14;	//复位SPI2
	RCC->APB1RSTR&=~(1<<14);//停止复位SPI2
	tempreg|=0<<10;			//全双工模式	
	tempreg|=1<<9;			//软件nss管理
	tempreg|=1<<8;			 
	tempreg|=1<<2;			//SPI主机  
	tempreg|=0<<11;			//8位数据格式	
	tempreg|=1<<1;			//空闲模式下SCK为1 CPOL=1 
	tempreg|=1<<0;			//数据采样从第2个时间边沿开始,CPHA=1  
 	//对SPI2属于APB1的外设.时钟频率最大为42Mhz频率.
	tempreg|=7<<3;			//Fsck=Fpclk1/256
	tempreg|=0<<7;			//MSB First  
	tempreg|=1<<6;			//SPI启动 
	SPI2->CR1=tempreg; 		//设置CR1  
	SPI2->I2SCFGR&=~(1<<11);//选择SPI模式
	SPI2_ReadWriteByte(0xff);//启动传输		 
}   
//SPI2速度设置函数
//SpeedSet:0~7
//SPI速度=fAPB1/2^(SpeedSet+1)
//fAPB1时钟一般为42Mhz
void SPI2_SetSpeed(u8 SpeedSet)
{
	SpeedSet&=0X07;			//限制范围
	SPI2->CR1&=0XFFC7; 
	SPI2->CR1|=SpeedSet<<3;	//设置SPI2速度  
	SPI2->CR1|=1<<6; 		//SPI设备使能	  
} 
//SPI2 读写一个字节
//TxData:要写入的字节
//返回值:读取到的字节
u8 SPI2_ReadWriteByte(u8 TxData)
{		 			 
	while((SPI2->SR&1<<1)==0);		//等待发送区空 
	SPI2->DR=TxData;	 	  		//发送一个byte  
	while((SPI2->SR&1<<0)==0);		//等待接收完一个byte  
 	return SPI2->DR;          		//返回收到的数据				    
}
