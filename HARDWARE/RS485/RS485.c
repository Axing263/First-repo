#include "RS485.h"
#include "delay.h"



uint32_t RS485_TimCount; 
uint8_t  RS485_RxBuf[100];       
uint16_t RS485_RxCount = 0;      
uint16_t RS485_ANum = 1024;   
uint8_t  RS485_UartReceiving = 0;
uint8_t  RS485_UartDataFinishFlag = 0;




void RS485_Smode(void)
{
	RS485_SR=1;
}

void RS485_Rmode(void)
{
	RS485_SR=0;
}
//初始化IO 串口4
//pclk1:PCLK1时钟频率(Mhz)
//bound:波特率 
void RS485_Init(u32 pclk1,u32 bound)
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
	RCC->APB1ENR|=1<<19;  	//使能串口4时钟 
	GPIO_Set(GPIOA,PIN0|PIN1,GPIO_MODE_AF,GPIO_OTYPE_PP,GPIO_SPEED_2M,GPIO_PUPD_PU);//PA0,PA1,复用功能,上拉输出

 	GPIO_AF_Set(GPIOA,0,8);	 //PA0,AF8
	GPIO_AF_Set(GPIOA,1,8);  //PA1,AF8  	
  
  GPIO_Set(GPIOA,PIN2 ,GPIO_MODE_OUT,GPIO_OTYPE_PP,GPIO_SPEED_2M,GPIO_PUPD_PU);	//PA2推挽输出	
	//波特率设置
 	UART4->BRR=mantissa; 	//波特率设置	 
	
	
	UART4->CR1=0;
	
	UART4->CR1&=~(1<<15);  
	UART4->CR1|=1<<3;  	  //串口发送使能 
	UART4->CR1|=1<<2;  	//串口接收使能
//	
 // UART4->CR1|=1<<12; 
	//UART4->CR1|=1<<10;  	//使能奇偶校验
	 
	//UART4->CR1&=~(1<<9);  
	UART4->CR2&=(3<<12);
	UART4->CR1|=1<<5;   //接收缓冲区非空中断使能		
	MY_NVIC_Init(1,3,UART4_IRQn,2);//组2，最低优先级 
	UART4->CR1|=1<<13;  	//串口使能
	RS485_Rmode();
}




void RS485_ReceiveData(void)    
{
    if(RS485_TimCount >= 50 && RS485_UartReceiving)                                    
    {  
      RS485_TimCount = 0;                                                             
      if(RS485_ANum == RS485_RxCount)                                                 
      {
        RS485_UartDataFinishFlag = 1;                                                 
        RS485_UartReceiving = 0;                                                   
      }
      else
      {
        RS485_ANum = RS485_RxCount;
      }
    }
}


 

 



//RS485发送一个字节
void RS485_Send_Byte(uint8_t Dat)
{
	UART4->DR=Dat; 
	while((UART4->SR&0X40)==0);//等待发送结束
	
}
void UART4_IRQHandler(void)
{
	u8 UART4_DATA;
	if(UART4->SR&(1<<5))//接收到数据
	{	 
		UART4_DATA=UART4->DR;  //读出数据清除中断标志			
	  RS485_TimCount=0;
	  RS485_UartReceiving = 1;    
    if (RS485_RxCount < sizeof(RS485_RxBuf)-5)  
    {
      RS485_RxBuf[RS485_RxCount++] = UART4_DATA;
    }
	}  
}


  
//RS485发送数组
void RS485_Send_array(uint8_t *arr, uint16_t len)
{
	uint16_t i=0;
	RS485_Smode();
	for(i=0; i<len; i++)
	{
		RS485_Send_Byte(arr[i]);
		delay_us(100);
	}
	delay_us(100);
  RS485_Rmode();
}

//水位仪指令发送
void RS485_Send_Water_Level(void)
{
	u8 buf[]={0x01,0x03,0x00,0x04,0x00,0x01,0xC5,0xCB};
	RS485_Send_array(buf, 8);
}


u8 Ammeter_flag;
//电表
void RS485_Send_Ammeter(void)
{
	u8 buf_w[]={0x01,0x04,0x01,0x3A,0x00,0x02,0x50,0x3A};//查询电量
	u8 buf_v[]={0x01,0x04,0x01,0x00,0x00,0x0C,0xF1,0xF3};//查询电压
	u8 buf_p[]={0x01,0x04,0x01,0x1E,0x00,0x02,0x10,0x31};//查询功率
	if(Ammeter_flag==0)
	{
	RS485_Send_array(buf_v, 8);
	Ammeter_flag=1;
	}
	else if(Ammeter_flag==1)
	{
	RS485_Send_array(buf_w, 8);
	Ammeter_flag=2;
	}
	else 
	{
	RS485_Send_array(buf_p, 8);
	Ammeter_flag=0;
	}
}

//水表
void RS485_Send_Water_meter(void)
{
	u8 buf[]={0x01,0x03,0x02,0x00,0x00,0x04,0x45,0xB1};
	RS485_Send_array(buf, 8);
}



//声光报警器
u8 Alarm_Flag;
void RS485_Send_Alarm_Start(void)
{
	u8 buf_start[]={0xFF,0x06,0x31,0x03,0x00,0x06,0xE2,0xEA};
	u8 buf_continue[]={0xFF,0x06,0x00,0x19,0x00,0x00,0x4D,0xD3};
	if(Alarm_Flag==0)
	{
	RS485_Send_array(buf_start, 8);
	}
	else if(Alarm_Flag==1)
	{
	RS485_Send_array(buf_continue, 8);
	}
}

void RS485_Send_Alarm_End(void)
{
	u8 buf_end[]={0xFF,0x06,0x00,0x19,0x00,0x01,0x8C,0x13};
  if((Alarm_Flag==2)||(Alarm_Flag==1))
	{
	RS485_Send_array(buf_end, 8);
	}
}

uint8_t GetAckData_Alarm(void)
{
    uint16_t i=0;
    for (i=0;i<100; i++)
    {
      if ((RS485_RxBuf[i]==0xFF) &&(RS485_RxBuf[i+1]==0x06))  
      {
        return i;
      }
    }        
    return 0;
}
void RS485_Alarm_Analysis(void)
{
	  RS485_ReceiveData();
		if(RS485_UartDataFinishFlag==1)
		{
			RS485_UartDataFinishFlag=0;
			u8 location=GetAckData_Alarm();
			if(Alarm_Flag==0)
			{
				if((RS485_RxBuf[location]==0xFF)&&(RS485_RxBuf[location+1]==0x06)&&(RS485_RxBuf[location+2]==0x31)&&(RS485_RxBuf[location+3]==0x03)&&(RS485_RxBuf[location+4]==0x00)&&(RS485_RxBuf[location+5]==0x06))
			   Alarm_Flag=1;
			}
			else if(Alarm_Flag==1)
			{
				if((RS485_RxBuf[location]==0xFF)&&(RS485_RxBuf[location+1]==0x06)&&(RS485_RxBuf[location+2]==0x00)&&(RS485_RxBuf[location+3]==0x19)&&(RS485_RxBuf[location+4]==0x00)&&(RS485_RxBuf[location+5]==0x00))
			   Alarm_Flag=2;
				if((RS485_RxBuf[location]==0xFF)&&(RS485_RxBuf[location+1]==0x06)&&(RS485_RxBuf[location+2]==0x00)&&(RS485_RxBuf[location+3]==0x19)&&(RS485_RxBuf[location+4]==0x00)&&(RS485_RxBuf[location+5]==0x01))
			   Alarm_Flag=0;
			}
			else if(Alarm_Flag==2)
			{
				if((RS485_RxBuf[location]==0xFF)&&(RS485_RxBuf[location+1]==0x06)&&(RS485_RxBuf[location+2]==0x00)&&(RS485_RxBuf[location+3]==0x19)&&(RS485_RxBuf[location+4]==0x00)&&(RS485_RxBuf[location+5]==0x01))
			   Alarm_Flag=0;
			}
			memset(RS485_RxBuf,0,100);
			RS485_RxCount=0;
			RS485_UartDataFinishFlag=0;
			RS485_TimCount=0;
		}
}

u8 MCB_Flag;
u8 MCB_Current_State = 0; // 初始化空开状态为关
void RS485_Send_MCB_Start(void)
{
	u8 mcb_start[]={0x68,0x00,0x00,0x00,0x00,0x00,0x00,0x68,0x1C,0x0E,0x34,0x35,0x34,0x39,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x35,0xD0,0x16};
		
	RS485_Send_array(mcb_start, 26);
}

void RS485_Send_MCB_End(void)
{
	u8 mcb_end[]={0x68,0x00,0x00,0x00,0x00,0x00,0x00,0x68,0x1C,0x0E,0x34,0x34,0x34,0x39,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x35,0xCF,0x16};
	
	RS485_Send_array(mcb_end, 26);		
}

void MCB_Analysis(void)
{
	if (MCB_Flag == 1) // 开命令
    {
        MCB_Flag = 0;
        RS485_Send_MCB_Start();
        // 可以在这里添加状态显示或其他开操作
    }
    else if (MCB_Flag == 2) // 关命令
    {
        MCB_Flag = 0;
        RS485_Send_MCB_End();
        // 可以在这里添加状态显示或其他关操作
    }
}


const u8 CRC16HiTable[]=
{ 
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
	0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
	0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
	0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
	0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
	0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
	0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
	0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
	0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
	0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
	0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
	0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
	0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
	0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
	0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
	0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
	0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
	0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
	0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
	0x80, 0x41, 0x00, 0xC1, 0x81, 0x40   
};

//CRC16校验低位字节表
const u8 CRC16LoTable[]=
{
	0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06,
	0x07, 0xC7, 0x05, 0xC5, 0xC4, 0x04, 0xCC, 0x0C, 0x0D, 0xCD,
	0x0F, 0xCF, 0xCE, 0x0E, 0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09,
	0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9, 0x1B, 0xDB, 0xDA, 0x1A,
	0x1E, 0xDE, 0xDF, 0x1F, 0xDD, 0x1D, 0x1C, 0xDC, 0x14, 0xD4,
	0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3,
	0x11, 0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3,
	0xF2, 0x32, 0x36, 0xF6, 0xF7, 0x37, 0xF5, 0x35, 0x34, 0xF4,
	0x3C, 0xFC, 0xFD, 0x3D, 0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A,
	0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38, 0x28, 0xE8, 0xE9, 0x29,
	0xEB, 0x2B, 0x2A, 0xEA, 0xEE, 0x2E, 0x2F, 0xEF, 0x2D, 0xED,
	0xEC, 0x2C, 0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26,
	0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0, 0x60,
	0x61, 0xA1, 0x63, 0xA3, 0xA2, 0x62, 0x66, 0xA6, 0xA7, 0x67,
	0xA5, 0x65, 0x64, 0xA4, 0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F,
	0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB, 0x69, 0xA9, 0xA8, 0x68,
	0x78, 0xB8, 0xB9, 0x79, 0xBB, 0x7B, 0x7A, 0xBA, 0xBE, 0x7E,
	0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5,
	0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71,
	0x70, 0xB0, 0x50, 0x90, 0x91, 0x51, 0x93, 0x53, 0x52, 0x92,
	0x96, 0x56, 0x57, 0x97, 0x55, 0x95, 0x94, 0x54, 0x9C, 0x5C,
	0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E, 0x5A, 0x9A, 0x9B, 0x5B,
	0x99, 0x59, 0x58, 0x98, 0x88, 0x48, 0x49, 0x89, 0x4B, 0x8B,
	0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C,
	0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42,
	0x43, 0x83, 0x41, 0x81, 0x80, 0x40     
};
//使用CRC16校验
//计算方式： 
//buf:待校验缓冲区首地址
//len:要校验的长度
//返回值：CRC16校验值（高字节在前，低字节在后）  
u16 mc_check_crc16(u8 *buf,u16 len)
{
	u8 index;
	u16 check16=0;
	u8 crc_low=0XFF;
	u8 crc_high=0XFF;
	while(len--)
	{
		index=crc_high^(*buf++);
		crc_high=crc_low^CRC16HiTable[index];
		crc_low=CRC16LoTable[index];
	}
	check16 +=crc_high;
	check16 <<= 8;
	check16+=crc_low;
	return check16;
}
u32 water_mm;
void RS485_Analysis(void)
{
	  u16 crc_check;
	  u8 crc_high,crc_low;
	  RS485_ReceiveData();

		if(RS485_UartDataFinishFlag==1)
		{
			RS485_UartDataFinishFlag=0;
			crc_check=mc_check_crc16(RS485_RxBuf,5);
			crc_low=crc_check&0xFF;
			crc_high=(crc_check>>8)&0xFF;
			if((crc_high==RS485_RxBuf[5])&&(crc_low==RS485_RxBuf[6]))
			{
				water_mm=RS485_RxBuf[3]*256+RS485_RxBuf[4];
			}
			memset(RS485_RxBuf,0,100);
			RS485_RxCount=0;
			RS485_UartDataFinishFlag=0;
			RS485_TimCount=0;
		}
}




uint8_t GetAckData_Watermeter(void)
{
    uint16_t i=0;
    for (i=0;i<100; i++)
    {
      if ((RS485_RxBuf[i]==0x01) &&(RS485_RxBuf[i+1]==0x03))  
      {
        return i;
      }
    }        
    return 0;
}


extern double wf,wlf;  
void RS485_Analysis_Watermeter(void)
{
	  u16 crc_check;
	  u8 crc_high,crc_low;
	  u8 location;
	
	
	  RS485_ReceiveData();

		if(RS485_UartDataFinishFlag==1)
		{
			RS485_UartDataFinishFlag=0;
			location=GetAckData_Watermeter();
			
			if((RS485_RxBuf[location]==0x01)&&(RS485_RxBuf[location+1]==0x03)&&(RS485_RxBuf[location+2]==0x08))
			{
			crc_check=mc_check_crc16(&RS485_RxBuf[location],11);
			crc_low=crc_check&0xFF;
			crc_high=(crc_check>>8)&0xFF;
			if((crc_high==RS485_RxBuf[location+11])&&(crc_low==RS485_RxBuf[location+12]))
			{
				wf=(double)(RS485_RxBuf[location+9]*256+RS485_RxBuf[location+10])*0.001;
				wlf=(double)(RS485_RxBuf[location+3]*256+RS485_RxBuf[location+4]);
			}
		  }
			memset(RS485_RxBuf,0,100);
			RS485_RxCount=0;
			RS485_UartDataFinishFlag=0;
			RS485_TimCount=0;
		}
}


extern double Va,Vb,Vc,Ia,Ib,Ic,Es,Ep;



uint8_t GetAckData_Ammeter(void)
{
    uint16_t i=0;
    for (i=0;i<100; i++)
    {
      if ((RS485_RxBuf[i]==0x01) &&(RS485_RxBuf[i+1]==0x04))  
      {
        return i;
      }
    }        
    return 0;
}


void RS485_Analysis_Ammeter(void)
{
	  u16 crc_check;
	  u8 crc_high,crc_low;
	  RS485_ReceiveData();
	  u8 location;

		if(RS485_UartDataFinishFlag==1)
		{
 			RS485_UartDataFinishFlag=0;
			
			
			
			location=GetAckData_Ammeter();
			if((RS485_RxBuf[location]==0x01)&&(RS485_RxBuf[location+1]==0x04)&&(RS485_RxBuf[location+2]==0x04)&&(Ammeter_flag==2))
			{
				crc_check=mc_check_crc16(&RS485_RxBuf[location],7);
				crc_low=crc_check&0xFF;
				crc_high=(crc_check>>8)&0xFF;
				if((crc_high==RS485_RxBuf[location+7])&&(crc_low==RS485_RxBuf[location+8]))
				{
					Ep=(double)(RS485_RxBuf[location+3]*256*256*256+RS485_RxBuf[location+4]*256*256+RS485_RxBuf[location+5]*256+RS485_RxBuf[location+6])*0.01;
				}
			}
			else if((RS485_RxBuf[location]==0x01)&&(RS485_RxBuf[location+1]==0x04)&&(RS485_RxBuf[location+2]==0x04))
			{
				crc_check=mc_check_crc16(&RS485_RxBuf[location],7);
				crc_low=crc_check&0xFF;
				crc_high=(crc_check>>8)&0xFF;
				if((crc_high==RS485_RxBuf[location+7])&&(crc_low==RS485_RxBuf[location+8]))
				{
					Es=(double)(RS485_RxBuf[location+3]*256*256*256+RS485_RxBuf[location+4]*256*256+RS485_RxBuf[location+5]*256+RS485_RxBuf[location+6]);
				}
			}
			
			else if((RS485_RxBuf[location]==0x01)&&(RS485_RxBuf[location+1]==0x04)&&(RS485_RxBuf[location+2]==0x18))
			{
				crc_check=mc_check_crc16(&RS485_RxBuf[location],27);
				crc_low=crc_check&0xFF;
				crc_high=(crc_check>>8)&0xFF;
				if((crc_high==RS485_RxBuf[location+27])&&(crc_low==RS485_RxBuf[location+28]))
				{
					Va=(double)(RS485_RxBuf[location+3]*256*256*256+RS485_RxBuf[location+4]*256*256+RS485_RxBuf[location+5]*256+RS485_RxBuf[location+6])*0.1;
					Vb=(double)(RS485_RxBuf[location+7]*256*256*256+RS485_RxBuf[location+8]*256*256+RS485_RxBuf[location+9]*256+RS485_RxBuf[location+10])*0.1;
					Vc=(double)(RS485_RxBuf[location+11]*256*256*256+RS485_RxBuf[location+12]*256*256+RS485_RxBuf[location+13]*256+RS485_RxBuf[location+14])*0.1;
					Ia=(double)(RS485_RxBuf[location+15]*256*256*256+RS485_RxBuf[location+16]*256*256+RS485_RxBuf[location+17]*256+RS485_RxBuf[location+18])*0.01;
					Ib=(double)(RS485_RxBuf[location+19]*256*256*256+RS485_RxBuf[location+20]*256*256+RS485_RxBuf[location+21]*256+RS485_RxBuf[location+22])*0.01;
					Ic=(double)(RS485_RxBuf[location+23]*256*256*256+RS485_RxBuf[location+24]*256*256+RS485_RxBuf[location+25]*256+RS485_RxBuf[location+26])*0.01;
				}
			}
			memset(RS485_RxBuf,0,100);
			RS485_RxCount=0;
			RS485_UartDataFinishFlag=0;
			RS485_TimCount=0;
		}
}



