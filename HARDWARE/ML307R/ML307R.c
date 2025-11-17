#include "ML307R.h"
#include "RS485.h"
#include "RTC.h"
#include "MQTT.h"
#include "delay.h"
#include "W25Q64.h"
#include "Process.h"
uint32_t ML307R_TimCount; 
uint8_t  ML307R_RxBuf[1024];       
uint8_t  ML307R_TxBuf[1024];
uint16_t ML307R_RxCount = 0;      
uint16_t ML307R_ANum = 1024;   
uint8_t  ML307R_UartReceiving = 0;
uint8_t  ML307R_UartDataFinishFlag = 0;

u8 OTA_buf[1024*50];
u32 OTA_Count;
extern uint32_t  File_Size;


extern MQTT_CB   Aep_mqtt;        //创建一个用于连接电信AEP平台mqtt的结构体

extern u8 Net_flag; //链接服务器成功标志位
u8 AT[4] ="AT\r\n";//	查询是否回显
u8 ATE[6] ="ATE0\r\n";//关闭回显
u8 AT_CGSN[11] = "AT+CGSN=1\r\n";//获取IMEI号
u8 AT_CPIN[10] = "AT+CPIN?\r\n";//检查SIM卡状态
u8 AT_CIMI[9] = "AT+CIMI\r\n";//获取SIM卡的IMSI
u8 AT_CCID[10] = "AT+MCCID\r\n";//获取SIM卡ICCD号
u8 AT_CSQ[8]  = "AT+CSQ\r\n";//获取信号强度
u8 AT_CTZU[11]  = "AT+CTZU=1\r\n";//设置设备更新时间来源
u8 AT_CCLK[10] = "AT+CCLK?\r\n";//查询当前时间
u8 AT_MNTP[35] = "AT+MNTP=\"ntp.ntsc.ac.cn\",123,1,30\r\n";//查询当前NTP时间
u8 AT_CREG[11] = "AT+CEREG?\r\n";//查询网络注册状态 
u8 AT_CGATT[11] = "AT+CGATT?\r\n";//查询附网状态
u8 AT_MIPCLOSE[15]="AT+MIPCLOSE=0\r\n";//关闭socket通道
u8 AT_SETTCP[16]="AT+MIPMODE=0,1\r\n";//设置为透传模式
u8 AT_MIPTKA[24]="AT+MIPTKA=0,1,120,60,1\r\n";//设置keepalive
u8 AT_MIPOPEN[80];
//u8 AT_MIPOPEN[45]="AT+MIPOPEN=0,\"TCP\",\"cloud.xstars.tech\",1883\r\n";//IP+端口
//u8 AT_MIPOPEN[40]="AT+MIPOPEN=0,\"TCP\",\"123.56.8.160\",1883\r\n";//IP+端口
//u8 AT_MIPOPEN[57]="AT+MIPOPEN=0,\"TCP\",\"112.125.89.8\",42732\r\n";//IP+端口


 
ML307R_AT_Send_Flag         AT_Flag={0};
ML307R_AT_Send_Flag_OK      AT_OK_Flag={0};
ML307R_AT_Send_Count        AT_Count={0};
ML307R_AT_Error             AT_Error={0};
ML307R_Information          ML307R_Inf={0};
 

 
HTTP_AT_Send_Flag         HTTP_Flag={0};
HTTP_AT_Send_Flag_OK      HTTP_OK_Flag={0};
HTTP_AT_Send_Count        HTTP_Count={0};
HTTP_AT_Error             HTTP_Error={0};


void ML307R_Init(u32 pclk2,u32 bound)
{
	float temp;
	u16 mantissa;
	u16 fraction;	   
  
	temp=(float)(pclk2*1000000)/(bound*16);//得到USARTDIV@OVER8=0
	mantissa=temp;				 //得到整数部分
	fraction=(temp-mantissa)*16; //得到小数部分@OVER8=0 
  mantissa<<=4;
	mantissa+=fraction; 
	RCC->AHB1ENR|=1<<2;   //使能PORTC口时钟  
	RCC->AHB1ENR|=1<<0;   //使能PORTA口时钟  
	RCC->APB2ENR|=1<<5;  	//使能串口6时钟 
	GPIO_Set(GPIOA,PIN8,GPIO_MODE_OUT,GPIO_OTYPE_PP,GPIO_SPEED_50M,GPIO_PUPD_PU);//PA8设置 
	GPIO_Set(GPIOC,PIN6|PIN7,GPIO_MODE_AF,GPIO_OTYPE_PP,GPIO_SPEED_50M,GPIO_PUPD_PU);//PC6,PC7,复用功能,上拉输出
 	GPIO_AF_Set(GPIOC,6,8);//PC6,AF8
	GPIO_AF_Set(GPIOC,7,8);//PC7,AF8  	   
	//波特率设置
 	USART6->BRR=mantissa; 	//波特率设置	 
	USART6->CR1&=~(1<<15); 	//设置OVER8=0 
	USART6->CR1|=1<<3;  	//串口发送使能 
	//使能接收中断 
	USART6->CR1|=1<<2;  	//串口接收使能
	USART6->CR1|=1<<5;    	//接收缓冲区非空中断使能	    	
	MY_NVIC_Init(3,3,USART6_IRQn,2);//组2，最低优先级 
	USART6->CR1|=1<<13;  	//串口使能

	ML307R_RST=1;
	delay_ms(500);
	ML307R_RST=0;
	ML307R_START();
}


void ML307R_ReceiveData(void)    
{
    if(ML307R_TimCount >= 50 && ML307R_UartReceiving)                                    
    {  
      ML307R_TimCount = 0;                                                             
      if(ML307R_ANum == ML307R_RxCount)                                                 
      {
        ML307R_UartDataFinishFlag = 1;                                                 
        ML307R_UartReceiving = 0;                                                   
      }
      else
      {
        ML307R_ANum = ML307R_RxCount;
      }
    }
}


 

void USART6_IRQHandler(void)
{
  u8 ML307R_DATA;
	if(USART6->SR&(1<<5))//接收到数据
	{	 
		ML307R_DATA=USART6->DR;  //读出数据清除中断标志		
		if(HTTP_Flag.f_AT_TCPSEND_Cm==1) //表述数据OTA
		{
			OTA_buf[OTA_Count++]=ML307R_DATA;
		}
	  ML307R_TimCount=0;
	  ML307R_UartReceiving = 1;    
    if (ML307R_RxCount < sizeof(ML307R_RxBuf)-5)  
    {
      ML307R_RxBuf[ML307R_RxCount++] = ML307R_DATA;
    }
		else 
		{
			ML307R_RxCount = 0;                       
		}	 
	} 
}


uint8_t GetAckData(uint8_t *string)
{
    uint16_t i=0;
    for (i=0;i<1024; i++)
    {
      if (memcmp(string, &ML307R_RxBuf[i], strlen((const char *)string)) == 0) 
      {
        ML307R_RxCount=0;
        return 1;
      }
    }
    ML307R_RxCount=0;         
    return 0;
}

void ML307R_START(void)
{
	memset(ML307R_TxBuf,0,1024);
	ML307R_RxCount=0;
	ML307R_UartDataFinishFlag=0;
	ML307R_TimCount=0;
	while(1)
	{
	  ML307R_ReceiveData();
		if(ML307R_UartDataFinishFlag==1)
		{
			ML307R_UartDataFinishFlag=0;
			if((GetAckData((void *)"+MATREADY"))|(GetAckData((void *)"OK\r\n")))
			{
			memset(ML307R_RxBuf, 0, 1024); 
			ML307R_TimCount=0;
			break;
			}
		}
		if(ML307R_TimCount>=2000)
		{
			ML307R_UartDataFinishFlag=0;
			ML307R_Send_array((void *)"AT\r\n",4); 
	    ML307R_TimCount=0;
		}
  }
}

void UTC_to_ZoneTime( _UTCTimer* utc_time, int timezone, _UTCTimer* local_time)
{
	int year, month, day, hour;
    int lastday;   
    int lastlastday; 
 
    year    = utc_time->year; 
    month   = utc_time->months;
    day     = utc_time->day;
    hour    = utc_time->hour + timezone; 
 
    if (1==month || 3==month || 5==month || 7==month || 8==month || 10==month || 12==month) {
        lastday = 31;
        lastlastday = 30;
        if (3 == month) {
            if ((0 == year%400) || ((0 == year%4) && (year%100 != 0))) { //if this is lunar year.
                lastlastday = 29;
            } else {
                lastlastday = 28;
            }
        } else if ((1 == month) || (8 == month)) {
            lastlastday = 31;
        }
    } else if (4==month || 6==month || 9==month || 11==month) {
        lastday = 30;
        lastlastday = 31;
    } else {
        lastlastday = 31;
        if ((0 == year%400) || ((0 == year%4) && (year%100 != 0))) {
            lastday = 29;
        } else {
            lastday = 28;
        }
    }
    if (hour >= 24) {
        hour -= 24;
        day += 1; 
 
        if (day > lastday) {
            day -= lastday;
            month += 1;
 
            if (month > 12) {
                month -= 12;
                year += 1;
            }
        }
    } else if (hour < 0) {
        hour += 24;
        day -= 1;
        if (day < 1) {
            day = lastlastday;
            month -= 1;
            if (month < 1) {
                month = 12;
                year -= 1;
            }
        }
    }
	local_time->year  = year;
	local_time->months = month;
	local_time->day  = day;
	local_time->hour  = hour;
	local_time->minute	 = utc_time->minute;
	local_time->second	 = utc_time->second;
}


//CAT1发送一个字节
void ML307R_Send_Byte(uint8_t Dat)
{
	USART6->DR=Dat; 
	while((USART6->SR&0X40)==0);//等待发送结束
}

 

//CAT1发送数组
void ML307R_Send_array(uint8_t *arr, uint16_t len)
{
	uint16_t i=0;
	for(i=0; i<len; i++)
	{
		ML307R_Send_Byte(arr[i]);
	}
}
void ML307R_Send_AT(void)//发送AT指令
{
	ML307R_RxCount=0;
		ML307R_Send_array(AT,4);
		AT_Count.AT_Ct +=1;
		AT_Flag.f_AT_Cm=1;	 
}

void ML307R_Send_ATE(void)//关闭回显
{
	  ML307R_Send_array(ATE,6);
		AT_Count.ATE_Ct +=1;
		AT_Flag.f_ATE_Cm=1;	 
}

	void ML307R_Send_AT_CGSN(void)//查询IMEI
{
	  ML307R_Send_array(AT_CGSN,11);
		AT_Count.AT_CGSN_Ct +=1;
		AT_Flag.f_AT_CGSN_Cm=1;	 
}
void ML307R_Send_AT_CPIN(void)//检查SIM卡状态
{
	  ML307R_Send_array(AT_CPIN,10);
		AT_Count.AT_CPIN_Ct +=1;
		AT_Flag.f_AT_CPIN_Cm=1;	 
}
void ML307R_Send_AT_CIMI(void)//查询IMSI
{
	  ML307R_Send_array(AT_CIMI,9);
		AT_Count.AT_CIMI_Ct +=1;
		AT_Flag.f_AT_CIMI_Cm=1;	 
}
void ML307R_Send_AT_CCID(void)//查询CCID
{
	  ML307R_Send_array(AT_CCID,10);
		AT_Count.AT_CCID_Ct +=1;
		AT_Flag.f_AT_CCID_Cm=1;	 
}
void ML307R_Send_AT_CSQ(void)//查询CSQ
{
	  ML307R_Send_array(AT_CSQ,10);
		AT_Count.AT_CSQ_Ct +=1;
		AT_Flag.f_AT_CSQ_Cm=1;	 
}
void ML307R_Send_AT_CTZU(void)//设置时间更新来源
{
	  ML307R_Send_array(AT_CTZU,11);
		AT_Count.AT_CTZU_Ct +=1;
		AT_Flag.f_AT_CTZU_Cm=1;	 
}
void ML307R_Send_AT_CCLK(void)//查询时间
{
	  ML307R_Send_array(AT_CCLK,10);
		AT_Count.AT_CCLK_Ct +=1;
		AT_Flag.f_AT_CCLK_Cm=1;	 
}
void ML307R_Send_AT_MNTP(void)//查询NTP时间
{
	  ML307R_Send_array(AT_MNTP,35);
		AT_Count.AT_MNTP_Ct +=1;
		AT_Flag.f_AT_MNTP_Cm=1;	 
}
void ML307R_Send_AT_CEREG(void)//查询网络注册状态
{
	  ML307R_Send_array(AT_CREG,11);
		AT_Count.AT_CREG_Ct +=1;
		AT_Flag.f_AT_CREG_Cm=1;	 
}
void ML307R_Send_AT_CGATT(void)//查询附网状态
{
	  ML307R_Send_array(AT_CGATT,11);
		AT_Count.AT_CGATT_Ct +=1;
		AT_Flag.f_AT_CGATT_Cm=1;	 
}
void ML307R_Send_AT_CLOSE(void)//关闭SOCKET
{
	  ML307R_Send_array(AT_MIPCLOSE,15);
		AT_Count.AT_CLOSE_Ct +=1;
		AT_Flag.f_AT_CLOSE_Cm=1;	 
}
void ML307R_Send_AT_OPEN(void)//设置IP+端口
{
	  memset(AT_MIPOPEN,0,80);
	  memcpy(AT_MIPOPEN,"AT+MIPOPEN=0,\"TCP\",\"",strlen("AT+MIPOPEN=0,\"TCP\",\""));
	  strcat((char*)AT_MIPOPEN,(char*)Set_Type.ip);
	  strcat((char*)AT_MIPOPEN,(char*)"\",");
	  strcat((char*)AT_MIPOPEN,(char*)Set_Type.port);
	  strcat((char*)AT_MIPOPEN,(char*)"\r\n");
	//memset(AT_MIPOPEN,0,80);
	//RS485_Send_array((void *)AT_MIPOPEN,strlen((void *)AT_MIPOPEN));
	  ML307R_Send_array(AT_MIPOPEN,strlen((void *)AT_MIPOPEN));
		AT_Count.AT_OPEN_Ct +=1;
		AT_Flag.f_AT_OPEN_Cm=1;	 
}

void ML307R_Send_TCP_SET(void)//设置TCP为透传模式
{
	  ML307R_Send_array(AT_SETTCP,16);
		AT_Count.AT_STCP_Ct +=1;
		AT_Flag.f_AT_STCP_Cm=1;	 
}
void ML307R_Send_TCP_ALIVE(void)//设置keepalive
{
	  ML307R_Send_array(AT_MIPTKA,24);
		AT_Count.AT_MIPTKA_Ct +=1;
		AT_Flag.f_AT_MIPTKA_Cm=1;	 
}
 
void ML307R_MQTT_Connect(void)//设置MQTT链接
{
	  IoT_Parameter_Init();
		MQTT_ConectPack();
	  ML307R_Send_array(Aep_mqtt.Pack_buff,Aep_mqtt.Fixed_len+Aep_mqtt.Payload_len+Aep_mqtt.Variable_len);
		AT_Count.AT_MQTT_Con_Ct +=1;
		AT_Flag.f_AT_MQTT_Con_Cm=1;	 
}
 

 
u8 ML307R_Step=1;
extern u8 run;
void ML307R_Send_Command(void)
{
  switch(ML307R_Step)
	{
		case 1://AT验证
		{
				if((AT_Count.AT_Ct<200)&&(AT_Flag.f_AT_Cm ==0))     //发送次数少于60次，指令发送标志位为0时
						{ ML307R_Send_AT();}
				if(AT_Count.AT_Ct == 200)
					{AT_Error.f_AT=1;ML307R_Step=1;run=REBOOT;}
		}	                                                   break;
		case 2://关闭回显
		{
				if((AT_Count.ATE_Ct<60)&&(AT_Flag.f_ATE_Cm ==0))     //发送次数少于60次，指令发送标志位为0时
						{ ML307R_Send_ATE();}
				if(AT_Count.ATE_Ct == 60)
					{AT_Error.f_ATE=1;ML307R_Step=1;run=REBOOT;}
		}	                                                   break;																			
		case 3://获取IMEI号
		{
				if((AT_Count.AT_CGSN_Ct<60)&&(AT_Flag.f_AT_CGSN_Cm ==0))
						{ ML307R_Send_AT_CGSN();}
				if(AT_Count.AT_CGSN_Ct == 60)
					{AT_Error.f_AT_CGSN=1;ML307R_Step=1;run=REBOOT;}										
		}		                                                 break;	
		case 4://检查SIM卡状态
		{
				if((AT_Count.AT_CPIN_Ct<30)&&(AT_Flag.f_AT_CPIN_Cm ==0))
						{ ML307R_Send_AT_CPIN();}
				if(AT_Count.AT_CPIN_Ct == 30)
					{AT_Error.f_AT_CPIN=1;ML307R_Step=1;run=REBOOT;}										
		}	                                                   break;	
    case 5://获取SIM卡的IMSI
		{
				if((AT_Count.AT_CIMI_Ct<30)&&(AT_Flag.f_AT_CIMI_Cm ==0))
						{ ML307R_Send_AT_CIMI();}
				if(AT_Count.AT_CIMI_Ct ==30)
					{AT_Error.f_AT_CIMI=1;ML307R_Step=1;run=REBOOT;}
		}                                                    break;
	  case 6://获取SIM卡ICCD号
		{
				if((AT_Count.AT_CCID_Ct<30)&&(AT_Flag.f_AT_CCID_Cm ==0))
						{ ML307R_Send_AT_CCID();}
				if(AT_Count.AT_CCID_Ct == 30)
					{AT_Error.f_AT_CCID=1;ML307R_Step=1;run=REBOOT;}											
		}																									   break;	
    case 7://获取CSQ
		{
				if((AT_Count.AT_CSQ_Ct<60)&&(AT_Flag.f_AT_CSQ_Cm ==0))
						{ ML307R_Send_AT_CSQ();}
				if(AT_Count.AT_CSQ_Ct == 60)
					{AT_Error.f_AT_CSQ=1;ML307R_Step=1;run=REBOOT;}											
		}																									   break;
		case 8://获取网络注册状态
		{
				if((AT_Count.AT_CREG_Ct<60)&&(AT_Flag.f_AT_CREG_Cm ==0))
						{ ML307R_Send_AT_CEREG();}
				if(AT_Count.AT_CREG_Ct == 60)
					{AT_Error.f_AT_CREG=1;ML307R_Step=1;run=REBOOT;}											
		}		                                                 break;
		  case 9://查询附网状态
		{
				if((AT_Count.AT_CGATT_Ct<200)&&(AT_Flag.f_AT_CGATT_Cm ==0))
						{ ML307R_Send_AT_CGATT();}
				if(AT_Count.AT_CGATT_Ct == 200)
					{AT_Error.f_AT_CGATT=1;ML307R_Step=1;run=REBOOT;}											
		}																									   break;	
//		case 8://设置时间更新来源
//		{
//				if((AT_Count.AT_CTZU_Ct<30)&&(AT_Flag.f_AT_CTZU_Cm ==0))
//						{ ML307R_Send_AT_CTZU();}
//				if(AT_Count.AT_CTZU_Ct == 30)
//					{AT_Error.f_AT_CTZU=1;ML307R_Step++;}											
//		}																									   break;
////    case 10://获取NTP时间
////		{
////				if((AT_Count.AT_MNTP_Ct<30)&&(AT_Flag.f_AT_MNTP_Cm ==0))
////						{ ML307R_Send_AT_MNTP();}
////				if(AT_Count.AT_MNTP_Ct == 30)
////					{AT_Error.f_AT_MNTP=1;ML307R_Step++;}											
////		}				  break;	
    case 10://获取时间
		{
				if((AT_Count.AT_CCLK_Ct<30)&&(AT_Flag.f_AT_CCLK_Cm ==0))
						{ ML307R_Send_AT_CCLK();}
				if(AT_Count.AT_CCLK_Ct == 30)
					{AT_Error.f_AT_CCLK=1;ML307R_Step=1;run=REBOOT;}											
		}					break;																							   
    case 11://关闭SOCKET
		{
				if((AT_Count.AT_CLOSE_Ct<30)&&(AT_Flag.f_AT_CLOSE_Cm ==0))
						{ ML307R_Send_AT_CLOSE();}
				if(AT_Count.AT_CLOSE_Ct == 30)
					{AT_Error.f_AT_CLOSE=1;ML307R_Step=1;run=REBOOT;}											
		}																									   break;
		case 12://设置keepalive
		{
				if((AT_Count.AT_MIPTKA_Ct<30)&&(AT_Flag.f_AT_MIPTKA_Cm ==0))
						{ ML307R_Send_TCP_ALIVE();}
				if(AT_Count.AT_MIPTKA_Ct == 30)
					{AT_Error.f_AT_MIPTKA=1;ML307R_Step=1;run=REBOOT;}											
		}																									   break;	
    case 13://设置IP+端口
		{
				if((AT_Count.AT_OPEN_Ct<30)&&(AT_Flag.f_AT_OPEN_Cm ==0))
						{ 
						 
						// ML307R_Send_array("",strlen((void *)""));
						 ML307R_Send_AT_OPEN();}
				if(AT_Count.AT_OPEN_Ct == 30)
					{AT_Error.f_AT_OPEN=1;ML307R_Step=1;run=REBOOT;}											
		}																									   break;
		case 14://设置TCP为透传模式
		{
				if((AT_Count.AT_STCP_Ct<30)&&(AT_Flag.f_AT_STCP_Cm ==0))
						{ ML307R_Send_TCP_SET();}
				if(AT_Count.AT_STCP_Ct == 30)
					{AT_Error.f_AT_STCP=1;ML307R_Step=1;run=REBOOT;}											
		}																									   break;	
		case 15://设置MQTT CONNECT
		{
			  if((AT_Count.AT_MQTT_Con_Ct<30)&&(AT_Flag.f_AT_MQTT_Con_Cm ==0))
						{ ML307R_MQTT_Connect();}
				if(AT_Count.AT_MQTT_Con_Ct == 30)
					{AT_Error.f_AT_MQTT_Con=1;ML307R_Step=1;run=REBOOT;}      
		}                                                   break;	
		case 16:
			IoT_Parameter_Init();
			MQTT_Subscribe(Aep_mqtt.Stopic_Buff, 1, 0, 0);
		  ML307R_Step++;
			break;
		default:																								
			break;
	}
}
void Check_ML307R_AT_ReciveData(void)//检查AT接收到的数据
{
		 u16 i;
	   u8 mqtt_ack[2]={0x20,0x02};
		 ML307R_ReceiveData();
		 if(ML307R_UartDataFinishFlag==1)
			{
	 if( AT_Flag.f_AT_Cm == 1)
							{
									if(strstr((void *)ML307R_RxBuf, "OK")!= NULL)
									{ML307R_Step++;AT_Count.AT_Ct=0;}
									AT_Flag.f_AT_Cm=0;
							}
	 if( AT_Flag.f_ATE_Cm == 1)
							{
									if(strstr((void *)ML307R_RxBuf, "OK")!= NULL)
										{ ML307R_Step++;AT_Count.ATE_Ct=0;}
									AT_Flag.f_ATE_Cm=0;
							}
	 if(AT_Flag.f_AT_CGSN_Cm == 1)
						 {
							if(strstr((void *)ML307R_RxBuf, "+CGSN:")!= NULL)
							{
									 ML307R_Step++;AT_Count.AT_CGSN_Ct=0;
									 for(i=0;i<15;i++)
											ML307R_Inf.ML307R_IMEI[i] = ML307R_RxBuf[9+i];
							}
							AT_Flag.f_AT_CGSN_Cm=0;
						 }	
	 if(AT_Flag.f_AT_CPIN_Cm == 1)
							{
									if(strstr((void *)ML307R_RxBuf, "+CPIN: READY")!= NULL)
									{ML307R_Step++;AT_Count.AT_CPIN_Ct=0;	}	
									AT_Flag.f_AT_CPIN_Cm=0;
							}
		if(AT_Flag.f_AT_CIMI_Cm == 1)
							{
									if(strstr((void *)ML307R_RxBuf, "460")!= NULL)
									{
											ML307R_Step++;AT_Count.AT_CIMI_Ct=0;
											for(i=0;i<15;i++)
													ML307R_Inf.ML307R_IMSI[i] = ML307R_RxBuf[i+2];
									}
									AT_Flag.f_AT_CIMI_Cm=0;
							}
    if(AT_Flag.f_AT_CCID_Cm == 1)
							{
									if(strstr((void *)ML307R_RxBuf, "+MCCID:")!= NULL)
									{
											 ML307R_Step++; AT_Count.AT_CCID_Ct=0;
											 for(i=0;i<20;i++)
													ML307R_Inf.ML307R_ICCID[i] = ML307R_RxBuf[9+i];
									}
									AT_Flag.f_AT_CCID_Cm=0;
							}	
    if(AT_Flag.f_AT_CSQ_Cm == 1)
							{
									if(strstr((void *)ML307R_RxBuf, "+CSQ")!= NULL)
									{
											 ML307R_Step++; AT_Count.AT_CSQ_Ct=0;
											 for(i=0;i<2;i++)
													ML307R_Inf.ML307R_CSQ[i] = ML307R_RxBuf[8+i];
									}
									AT_Flag.f_AT_CSQ_Cm=0;
							}		
		 if(AT_Flag.f_AT_CTZU_Cm == 1)
							{
									if(strstr((void *)ML307R_RxBuf, "OK")!= NULL)
									{
											 ML307R_Step++; AT_Count.AT_CTZU_Ct=0;
									}
									AT_Flag.f_AT_CTZU_Cm=0;
							}		
     if(AT_Flag.f_AT_CCLK_Cm == 1)
							{
									if(strstr((void *)ML307R_RxBuf, "+CCLK")!= NULL)
									{
										if(strstr((void *)ML307R_RxBuf, "70/01/01")== NULL)
										{
											 ML307R_Step++; AT_Count.AT_CCLK_Ct=0;
										   UTCTimer.year=(ML307R_RxBuf[10]-0x30)*10+ML307R_RxBuf[11]-0x30;
										   UTCTimer.months=(ML307R_RxBuf[13]-0x30)*10+ML307R_RxBuf[14]-0x30;
										   UTCTimer.day=(ML307R_RxBuf[16]-0x30)*10+ML307R_RxBuf[17]-0x30;
										   UTCTimer.hour=(ML307R_RxBuf[19]-0x30)*10+ML307R_RxBuf[20]-0x30;
										   UTCTimer.minute=(ML307R_RxBuf[22]-0x30)*10+ML307R_RxBuf[23]-0x30;
										   UTCTimer.second=(ML307R_RxBuf[25]-0x30)*10+ML307R_RxBuf[26]-0x30;
                       UTC_to_ZoneTime(&UTCTimer,8,&UTCTimer); //时区转换
										   UTCTimer.week=RTC_Get_Week(UTCTimer.year,UTCTimer.months,UTCTimer.day);
										   RTC_Set_Date(UTCTimer.year,UTCTimer.months,UTCTimer.day,UTCTimer.week);
										   RTC_Set_Time(UTCTimer.hour,UTCTimer.minute,UTCTimer.second,0);
										}
									}
									AT_Flag.f_AT_CCLK_Cm=0;
							}	
			if(AT_Flag.f_AT_MNTP_Cm == 1)
							{
									if(strstr((void *)ML307R_RxBuf, "+MNTP")!= NULL)
									{
										 ML307R_Step++; AT_Count.AT_MNTP_Ct=0;
									}
									AT_Flag.f_AT_MNTP_Cm=0;
							}	
      if(AT_Flag.f_AT_CREG_Cm == 1)
							{
									if(strstr((void *)ML307R_RxBuf, "+CEREG: 0,1")!= NULL)
									{
											 ML307R_Step++; AT_Count.AT_CREG_Ct=0;
									}
									AT_Flag.f_AT_CREG_Cm=0;
							}		
      if(AT_Flag.f_AT_CGATT_Cm == 1)
							{
									if(strstr((void *)ML307R_RxBuf, "+CGATT: 1")!= NULL)
									{
											 ML307R_Step++; AT_Count.AT_CGATT_Ct=0;
									}
									AT_Flag.f_AT_CGATT_Cm=0;
							}
			if(AT_Flag.f_AT_CLOSE_Cm == 1)
							{
									if(strstr((void *)ML307R_RxBuf, "+CME ERROR: 551")!= NULL)//TCP未被使用
									{
											 ML307R_Step++; AT_Count.AT_CLOSE_Ct=0;
									}
									AT_Flag.f_AT_CLOSE_Cm=0;
							}	
			if(AT_Flag.f_AT_MIPTKA_Cm == 1)
							{
									if(strstr((void *)ML307R_RxBuf, "OK")!= NULL)//TCP未被使用
									{
											 ML307R_Step++; AT_Count.AT_MIPTKA_Ct=0;
									}
									AT_Flag.f_AT_MIPTKA_Cm=0;
							}	
			if(AT_Flag.f_AT_OPEN_Cm == 1)
							{
									if(strstr((void *)ML307R_RxBuf, "+MIPOPEN: 0,0")!= NULL)
									{
											 ML307R_Step++; AT_Count.AT_OPEN_Ct=0;
									}
									AT_Flag.f_AT_OPEN_Cm=0;
							}	
			if(AT_Flag.f_AT_STCP_Cm == 1)
							{
									if(strstr((void *)ML307R_RxBuf, "CONNECT")!= NULL)
									{
											 ML307R_Step++; AT_Count.AT_STCP_Ct=0;
										  
									}
									AT_Flag.f_AT_STCP_Cm=0;
							}	
			 if(AT_Flag.f_AT_MQTT_Con_Cm == 1)
							{
									if(strstr((void *)ML307R_RxBuf, (void *)mqtt_ack)!= NULL)
									{
											 ML307R_Step++; AT_Count.AT_MQTT_Con_Ct=0;
										   Net_flag=1;   //连接MQTT服务器成功
									}
									AT_Flag.f_AT_MQTT_Con_Cm=0;
							}	
       if(strstr((void *)ML307R_RxBuf,"+MIPURC: \"disconn\",0")!= NULL)
									{
											 ML307R_Step=1;
										   Net_flag=0; 
										   run=REBOOT;
									}
       ML307R_RxCount=0;
       ML307R_TimCount=0;
			 ML307R_UartDataFinishFlag=0;
			 memset(ML307R_RxBuf,0,1024);
			 memset(ML307R_TxBuf,0,1024);									
			}
}

//+CPIN: SIM REMOVED




extern uint8_t  HTTP_IP[50];
extern uint8_t  HTTP_PATH[50];
void HTTP_Send_AT(void)//发送AT指令
{
		ML307R_Send_array(AT,4);
		HTTP_Count.AT_Ct +=1;
	  HTTP_Flag.f_AT_Cm=1;	 
}

void HTTP_Send_ATE(void)//关闭回显
{
	  ML307R_Send_array(ATE,6);
		HTTP_Count.ATE_Ct +=1;
		HTTP_Flag.f_ATE_Cm=1;	 
}

void HTTP_Send_AT_CGSN(void)//查询IMEI
{
	  ML307R_Send_array(AT_CGSN,11);
		HTTP_Count.AT_CGSN_Ct +=1;
		HTTP_Flag.f_AT_CGSN_Cm=1;	 
}
void HTTP_Send_AT_CPIN(void)//检查SIM卡状态
{
	  ML307R_Send_array(AT_CPIN,10);
		HTTP_Count.AT_CPIN_Ct +=1;
		HTTP_Flag.f_AT_CPIN_Cm=1;	 
}
void HTTP_Send_AT_CIMI(void)//查询IMSI
{
	  ML307R_Send_array(AT_CIMI,9);
		HTTP_Count.AT_CIMI_Ct +=1;
		HTTP_Flag.f_AT_CIMI_Cm=1;	 
}
void HTTP_Send_AT_CCID(void)//查询CCID
{
	  ML307R_Send_array(AT_CCID,10);
		HTTP_Count.AT_CCID_Ct +=1;
		HTTP_Flag.f_AT_CCID_Cm=1;	 
}
void HTTP_Send_AT_CSQ(void)//查询CSQ
{
	  ML307R_Send_array(AT_CSQ,10);
		HTTP_Count.AT_CSQ_Ct +=1;
		HTTP_Flag.f_AT_CSQ_Cm=1;	 
}
 
void HTTP_Send_AT_CCLK(void)//查询时间
{
	  ML307R_Send_array(AT_CCLK,10);
		HTTP_Count.AT_CCLK_Ct +=1;
		HTTP_Flag.f_AT_CCLK_Cm=1;	 
}
 
void HTTP_Send_AT_CEREG(void)//查询网络注册状态
{
	  ML307R_Send_array(AT_CREG,11);
		HTTP_Count.AT_CREG_Ct +=1;
		HTTP_Flag.f_AT_CREG_Cm=1;	 
}
void HTTP_Send_AT_CGATT(void)//查询附网状态
{
	  ML307R_Send_array(AT_CGATT,11);
		HTTP_Count.AT_CGATT_Ct +=1;
		HTTP_Flag.f_AT_CGATT_Cm=1;	 
}
void HTTP_Send_AT_CLOSE(void)//关闭SOCKET
{
	  ML307R_Send_array(AT_MIPCLOSE,15);
		HTTP_Count.AT_CLOSE_Ct +=1;
		HTTP_Flag.f_AT_CLOSE_Cm=1;	 
}

 
void HTTP_Send_AT_OPEN(void)//设置IP+端口
{
	  memset(AT_MIPOPEN,0,80);
	  memcpy(AT_MIPOPEN,"AT+MIPOPEN=0,\"TCP\",\"",strlen("AT+MIPOPEN=0,\"TCP\",\""));
	  strcat((char*)AT_MIPOPEN,(char*)HTTP_IP);
	  strcat((char*)AT_MIPOPEN,(char*)"\",");
	  strcat((char*)AT_MIPOPEN,(char*)"80");
	  strcat((char*)AT_MIPOPEN,(char*)"\r\n");
	  ML307R_Send_array(AT_MIPOPEN,strlen((void *)AT_MIPOPEN));
		HTTP_Count.AT_OPEN_Ct +=1;
		HTTP_Flag.f_AT_OPEN_Cm=1;	 
}

void HTTP_Send_TCP_SET(void)//设置TCP为透传模式
{
	  ML307R_Send_array(AT_SETTCP,16);
		HTTP_Count.AT_STCP_Ct +=1;
		HTTP_Flag.f_AT_STCP_Cm=1;	 
}
void HTTP_Send_TCP_ALIVE(void)//设置keepalive
{
	  ML307R_Send_array(AT_MIPTKA,24);
		HTTP_Count.AT_MIPTKA_Ct +=1;
		HTTP_Flag.f_AT_MIPTKA_Cm=1;	 
}
u8 HTTP_Send_Buf[200];
void HTTP_Send_TCP_SEND(void)
{
	  memset(HTTP_Send_Buf,0,200);
	  memcpy(HTTP_Send_Buf,"GET ",4);
	  strcat((char*)HTTP_Send_Buf,(char*)HTTP_PATH);
	  strcat((char*)HTTP_Send_Buf,(char*)" HTTP/1.1\r\n");
	  strcat((char*)HTTP_Send_Buf,(char*)"Host:");
	  strcat((char*)HTTP_Send_Buf,(char*)HTTP_IP);
	  strcat((char*)HTTP_Send_Buf,(char*)"\r\nConnection:keep-alive\r\n\r\n");
	  ML307R_Send_array(HTTP_Send_Buf,strlen((void *)HTTP_Send_Buf));
		HTTP_Count.AT_TCPSEND_Ct +=1;
		HTTP_Flag.f_AT_TCPSEND_Cm=1;	 
}
u8 HTTP_Step=1;
void HTTP_Send_Command(void)
{
//	RS485_Send_Byte(ML307R_Step/10+0x30);
//	RS485_Send_Byte(ML307R_Step%10+0x30);
  switch(HTTP_Step)
	{
		case 1://AT验证
		{
				if((HTTP_Count.AT_Ct<60)&&(HTTP_Flag.f_AT_Cm ==0))     //发送次数少于60次，指令发送标志位为0时
						{ HTTP_Send_AT();}
				if(HTTP_Count.AT_Ct == 60)
					{HTTP_Error.f_AT=1;HTTP_Step=1;run=REBOOT;}
		}	                                                   break;
		case 2://关闭回显
		{
				if((HTTP_Count.ATE_Ct<60)&&(HTTP_Flag.f_ATE_Cm ==0))     //发送次数少于60次，指令发送标志位为0时
						{ HTTP_Send_ATE();}
				if(HTTP_Count.ATE_Ct == 60)
					{HTTP_Error.f_ATE=1;HTTP_Step=1;run=REBOOT;}
		}	                                                   break;																			
		case 3://检查SIM卡状态
		{
				if((HTTP_Count.AT_CPIN_Ct<30)&&(HTTP_Flag.f_AT_CPIN_Cm ==0))
						{ HTTP_Send_AT_CPIN();}
				if(HTTP_Count.AT_CPIN_Ct == 30)
					{HTTP_Error.f_AT_CPIN=1;HTTP_Step=1;run=REBOOT;}										
		}	                                                   break;	
    case 4://获取CSQ
		{
				if((HTTP_Count.AT_CSQ_Ct<30)&&(HTTP_Flag.f_AT_CSQ_Cm ==0))
						{ HTTP_Send_AT_CSQ();}
				if(HTTP_Count.AT_CSQ_Ct == 30)
					{HTTP_Error.f_AT_CSQ=1;HTTP_Step=1;run=REBOOT;}											
		}																									   break;
		case 5://获取网络注册状态
		{
				if((HTTP_Count.AT_CREG_Ct<30)&&(HTTP_Flag.f_AT_CREG_Cm ==0))
						{ HTTP_Send_AT_CEREG();}
				if(HTTP_Count.AT_CREG_Ct == 30)
					{HTTP_Error.f_AT_CREG=1;HTTP_Step=1;run=REBOOT;}											
		}		
		  case 6://查询附网状态
		{
				if((HTTP_Count.AT_CGATT_Ct<30)&&(HTTP_Flag.f_AT_CGATT_Cm ==0))
						{ HTTP_Send_AT_CGATT();}
				if(HTTP_Count.AT_CGATT_Ct == 30)
					{HTTP_Error.f_AT_CGATT=1;HTTP_Step=1;run=REBOOT;}											
		}																									   break;																							   
    case 7://关闭SOCKET
		{
				if((HTTP_Count.AT_CLOSE_Ct<30)&&(HTTP_Flag.f_AT_CLOSE_Cm ==0))
						{ HTTP_Send_AT_CLOSE();}
				if(HTTP_Count.AT_CLOSE_Ct == 30)
					{HTTP_Error.f_AT_CLOSE=1;HTTP_Step=1;run=REBOOT;}											
		}																									   break;
		case 8://设置keepalive
		{
				if((HTTP_Count.AT_MIPTKA_Ct<30)&&(HTTP_Flag.f_AT_MIPTKA_Cm ==0))
						{ HTTP_Send_TCP_ALIVE();}
				if(HTTP_Count.AT_MIPTKA_Ct == 30)
					{HTTP_Error.f_AT_MIPTKA=1;HTTP_Step=1;run=REBOOT;}											
		}																									   break;	
    case 9://设置IP+端口
		{
				if((HTTP_Count.AT_OPEN_Ct<30)&&(HTTP_Flag.f_AT_OPEN_Cm ==0))
						{ HTTP_Send_AT_OPEN();}
				if(HTTP_Count.AT_OPEN_Ct == 30)
					{HTTP_Error.f_AT_OPEN=1;HTTP_Step=1;run=REBOOT;}											
		}																									   break;
		case 10://设置TCP为透传模式
		{
				if((HTTP_Count.AT_STCP_Ct<30)&&(HTTP_Flag.f_AT_STCP_Cm ==0))
						{ HTTP_Send_TCP_SET();}
				if(HTTP_Count.AT_STCP_Ct == 30)
					{HTTP_Error.f_AT_STCP=1;HTTP_Step=1;run=REBOOT;}											
		}																									   break;	
		case 11://发送HTTP
		{
				if((HTTP_Count.AT_TCPSEND_Ct<30)&&(HTTP_Flag.f_AT_TCPSEND_Cm ==0))
						{ HTTP_Send_TCP_SEND();}
				if(HTTP_Count.AT_TCPSEND_Ct == 30)
					{HTTP_Error.f_AT_TCPSEND=1;HTTP_Step++;}											
		}																									   break;	
		default:																								
			break;
	}
}



void Check_HTTP_AT_ReciveData(void)//检查AT接收到的数据
{
		 u16 i;
		 ML307R_ReceiveData();
		 if(ML307R_UartDataFinishFlag==1)
			{
	 if( HTTP_Flag.f_AT_Cm == 1)
							{
									if(strstr((void *)ML307R_RxBuf, "OK")!= NULL)
									{HTTP_Step++;HTTP_Count.AT_Ct=0;}
									HTTP_Flag.f_AT_Cm=0;
							}
	 if( HTTP_Flag.f_ATE_Cm == 1)
							{
									if(strstr((void *)ML307R_RxBuf, "OK")!= NULL)
										{ HTTP_Step++;HTTP_Count.ATE_Ct=0;}
									HTTP_Flag.f_ATE_Cm=0;
							}
	 
	 if(HTTP_Flag.f_AT_CPIN_Cm == 1)
							{
									if(strstr((void *)ML307R_RxBuf, "+CPIN: READY")!= NULL)
									{HTTP_Step++;HTTP_Count.AT_CPIN_Ct=0;	}	
									HTTP_Flag.f_AT_CPIN_Cm=0;
							}
    if(HTTP_Flag.f_AT_CSQ_Cm == 1)
							{
									if(strstr((void *)ML307R_RxBuf, "+CSQ")!= NULL)
									{
											 HTTP_Step++; HTTP_Count.AT_CSQ_Ct=0;
											 for(i=0;i<2;i++)
													ML307R_Inf.ML307R_CSQ[i] = ML307R_RxBuf[8+i];
									}
									HTTP_Flag.f_AT_CSQ_Cm=0;
							}		
      if(HTTP_Flag.f_AT_CREG_Cm == 1)
							{
									if(strstr((void *)ML307R_RxBuf, "+CEREG: 0,1")!= NULL)
									{
											 HTTP_Step++; HTTP_Count.AT_CREG_Ct=0;
									}
									HTTP_Flag.f_AT_CREG_Cm=0;
							}		
      if(HTTP_Flag.f_AT_CGATT_Cm == 1)
							{
									if(strstr((void *)ML307R_RxBuf, "+CGATT: 1")!= NULL)
									{
											 HTTP_Step++; HTTP_Count.AT_CGATT_Ct=0;
									}
									HTTP_Flag.f_AT_CGATT_Cm=0;
							}
			if(HTTP_Flag.f_AT_CLOSE_Cm == 1)
							{
									if(strstr((void *)ML307R_RxBuf, "+CME ERROR: 551")!= NULL)//TCP未被使用
									{
											 HTTP_Step++; HTTP_Count.AT_CLOSE_Ct=0;
									}
									HTTP_Flag.f_AT_CLOSE_Cm=0;
							}	
							
			if(HTTP_Flag.f_AT_MIPTKA_Cm == 1)
							{
									if(strstr((void *)ML307R_RxBuf, "OK")!= NULL)//TCP未被使用
									{
											 HTTP_Step++; HTTP_Count.AT_MIPTKA_Ct=0;
									}
									HTTP_Flag.f_AT_MIPTKA_Cm=0;
							}	
			if(HTTP_Flag.f_AT_OPEN_Cm == 1)
							{
									if(strstr((void *)ML307R_RxBuf, "+MIPOPEN: 0,0")!= NULL)
									{
											 HTTP_Step++; HTTP_Count.AT_OPEN_Ct=0;
									}
									HTTP_Flag.f_AT_OPEN_Cm=0;
							}	
			if(HTTP_Flag.f_AT_STCP_Cm == 1)
							{
									if(strstr((void *)ML307R_RxBuf, "CONNECT")!= NULL)
									{
											 HTTP_Step++; HTTP_Count.AT_STCP_Ct=0;
										  
									}
									HTTP_Flag.f_AT_STCP_Cm=0;
							}	
       if(strstr((void *)ML307R_RxBuf,"+MIPURC: \"disconn\",0")!= NULL)
									{
											 HTTP_Step=1;
										   Net_flag=0; 
									}
       ML307R_RxCount=0;
       ML307R_TimCount=0;
			 ML307R_UartDataFinishFlag=0;
			 memset(ML307R_RxBuf,0,1024);
			 memset(ML307R_TxBuf,0,1024);									
			}
}
