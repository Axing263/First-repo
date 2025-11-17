#include "ADC.h"
#include "stm32f4xx.h"
#include "math.h"

//电压输入PC1
//电压输入DC0-120V   分压电阻 110K，10K   分后电压DC0-10V
//GP9301  DC0-10V  转  0%-100%
//GP8101S 0%-100%  转  0-10V
//分压电阻 1K 2K

//电压输入PC0
//电压输入 DC1-5V   或者4-20mA  
//GP9303   DC0-5V  转  0%-100%
//GP8101S  0%-100%  转  0-10V
//分压电阻 10K 20K



u16 ADC_BUF[20];               //DMA目的地址
u16 ADC1_ValueTable_PC0[10];   //电流采集
u16 ADC1_ValueTable_PC1[10];   //电压采集

u16 ADC_PC0,ADC_PC1;
double Voltage,Current,Power,Power_s;   //电压 电流 累计功耗
//ADC排序滤波
void ADC_Filter(void)
{
	u16 j,i;

	   for(i=0;i<10;i++)
		{
		ADC1_ValueTable_PC0[i]=ADC_BUF[2*i];
		ADC1_ValueTable_PC1[i]=ADC_BUF[2*i+1];
		}
    // 排序
    for ( i = 0; i < 10; i++)
    {
        for ( j = 0; j < 10 - i - 1; j++)
        {
            if (ADC1_ValueTable_PC0[j] > ADC1_ValueTable_PC0[j + 1])
            {
                uint16_t temp = ADC1_ValueTable_PC0[j];
                ADC1_ValueTable_PC0[j] = ADC1_ValueTable_PC0[j + 1];
                ADC1_ValueTable_PC0[j + 1] = temp;
            }
        }
    }
    for ( i = 0; i < 10; i++)
    {
        for ( j = 0; j < 10 - i - 1; j++)
        {
            if (ADC1_ValueTable_PC1[j] > ADC1_ValueTable_PC1[j + 1])
            {
                uint16_t temp = ADC1_ValueTable_PC1[j];
                ADC1_ValueTable_PC1[j] = ADC1_ValueTable_PC1[j + 1];
                ADC1_ValueTable_PC1[j + 1] = temp;
            }
        }
    }
 
		ADC_PC0=(ADC1_ValueTable_PC0[3]+ADC1_ValueTable_PC0[4]+ADC1_ValueTable_PC0[5]+ADC1_ValueTable_PC0[6])/4;
		ADC_PC1=(ADC1_ValueTable_PC1[3]+ADC1_ValueTable_PC1[4]+ADC1_ValueTable_PC1[5]+ADC1_ValueTable_PC1[6])/4;		
    Voltage=fabs(ADC_PC1*40*3.3/4095.0);  //ADC_PC1*3.3/4095.0/10*30/2/10*210
		Current=ADC_PC0*800.0/4095.0;   // ADC_PC0*3.3/4095.0/3.3*800
		if(Current<3.0)
			Current=0.0;
		Power_s=Power_s+Voltage*Current/100.0;  //W.h
		Power=Power_s/3600.0;
}
void ADC1_Init(void) 
{
  /************************开启时钟************************/
	
	RCC->APB2ENR |= 1<<8;						//ADC1时钟使能
	RCC->AHB1ENR |= 1<<2;						//GPIOC时钟使能
	RCC->AHB1ENR |= 1<<22;					//使能DMA2时钟
	
	
	/************************配置GPIO************************/
	/*****ADC不属于复用功能，将GPIO口设置成模拟输入即可*****/
	
	GPIO_Set(GPIOC,PIN0|PIN1,GPIO_MODE_AIN,0,0,GPIO_PUPD_NONE);
	
	
	/************************ ADC时钟复位 ************************/
	
	RCC->APB2RSTR |= 1<<8;		
	RCC->APB2RSTR &= ~(1<<8);	//ADC1时钟复位
	
	

	/************************配置 DMA ************************/	
	/*******************DMA2 数据流0 通道0 *******************/	
	
	while(DMA2_Stream0->CR & 0x01);		//等待数据流可配置
	
	DMA2->LIFCR |= 0x3D<<0;						//清除数据流0的中断标志

	DMA2_Stream0->PAR = (uint32_t)&ADC1->DR;				
	DMA2_Stream0->M0AR = (uint32_t)&ADC_BUF;
	DMA2_Stream0->NDTR = 20;
	DMA2_Stream0->CR = 0;							//清零CR寄存器
	DMA2_Stream0->CR |= 0<<6;					//数据传输方向 从外设到存储器
	DMA2_Stream0->CR |= 1<<8;					//循环模式
	DMA2_Stream0->CR |= 0<<9;					//外设地址指针固定
	DMA2_Stream0->CR |= 1<<10;				//存储器增量模式
	DMA2_Stream0->CR |= 1<<11;				//外设数据长度：16位
	DMA2_Stream0->CR |= 1<<13;				//存储器数据长度：16位
	DMA2_Stream0->CR |= 2<<16;				//高优先级
	DMA2_Stream0->CR |= 0<<21;				//外设突发单次传输
	DMA2_Stream0->CR |= 0<<23;				//存储器突发单次传输
	DMA2_Stream0->CR |= 0<<25;				//数据流通道设置为通道0
	
	DMA2_Stream0->CR |= 1<<0;					//使能DMA
	DMA2_Stream0->FCR = 0X21;					//FIFO控制寄存器
	/*********************ADC1 通道1 PC1 *********************/


	ADC->CCR |= 0<<0;							//单个ADC，独立模式
	ADC->CCR |= 15<<8;						//两次采样之间的间隔
	ADC->CCR |= 0<<14;						//单个ADC禁止该位，多个ADC使能该位
	ADC->CCR |= 3<<16;						//预分频系数为8，84MHz/8=10.5MHz

	
	ADC1->CR1 |= 0;								//清零ADC1的CR1寄存器
	ADC1->CR2 |= 0;								//清零ADC1的CR2寄存器
	
	ADC1->CR1 |= 1<<8;						//扫描模式
	ADC1->CR1 &= ~(1<<24);				//12位分辨率
	ADC1->CR2 |= 1<<1;		 				//连续转换
	ADC1->CR2 |= 1<<8;						//使能DMA模式
	ADC1->CR2 |= 1<<9;
	ADC1->CR2 &= ~(1<<11); 				//右对齐方式
	ADC1->CR2 |= 0<<28;						//软件触发

	ADC1->SQR1 &=~(0xF<<20);
	ADC1->SQR1 |= 1<<20;					//两个规则在序列中，0表示一次转换
		
	ADC1->SQR3 &= 0xFFFFE0E0;			//设置转换序列
	ADC1->SQR3 |= 10<<0;				  //ADC1的通道10
	ADC1->SQR3 |= 11<<5;					//ADC1的通道11
	
	ADC1->SMPR2 &=~(7<<3*1);
	ADC1->SMPR1 |= 0XFFFFFFFF;				//采样时间设置为480个周期
	
	ADC1->SMPR2 &=~(7<<3*0);
	ADC1->SMPR2 |= 0XFFFFFFFF;				//采样时间设置为480个周期

	ADC1->CR2 |= 1<<0 ;						//开启ADC1	
	/************************软件启动 ADC ************************/	
	
	ADC1->CR2 |= 1<<30;						//启动规则转换通道
}


void ADC_Get(void)  //10ms调用一次 涉及到累计功耗问题
{
	ADC_Filter();
}



