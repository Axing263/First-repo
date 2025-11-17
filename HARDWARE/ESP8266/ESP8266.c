#include "ESP8266.h"
#include "delay.h"
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "ML307R.h"
#include "Process.h"
#include "W25Q64.h"
#include "MQTT.h"

uint32_t WIFI_TimCount; 
uint8_t  WIFI_RxBuf[1024];       
uint8_t  WIFI_TxBuf[1024];
uint16_t WIFI_RxCount = 0;      
uint16_t WIFI_ANum = 1024;   
uint8_t  WIFI_UartReceiving = 0;
uint8_t  WIFI_UartDataFinishFlag = 0;


extern MQTT_CB   Aep_mqtt;        //创建一个用于连接电信AEP平台mqtt的结构体
//初始化IO 串口1
//pclk2:PCLK时钟频率(Mhz)
//bound:波特率 
void ESP8266_Init(u32 pclk1,u32 bound)
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
	RCC->APB2ENR|=1<<4;   	//使能串口1时钟 
	GPIO_Set(GPIOA,PIN9|PIN10,GPIO_MODE_AF,GPIO_OTYPE_PP,GPIO_SPEED_50M,GPIO_PUPD_PU);//PA9,PA10,复用功能,上拉输出
 	GPIO_AF_Set(GPIOA,9,7);	//PA9,AF7
	GPIO_AF_Set(GPIOA,10,7);  //PA10,AF7  	   
	
	GPIO_Set(GPIOA,PIN11|PIN12,GPIO_MODE_OUT,GPIO_OTYPE_PP,GPIO_SPEED_50M,GPIO_PUPD_PU);//PA11,PA12,推挽输出
	
	//波特率设置
 	USART1->BRR=mantissa; 	//波特率设置	 
	USART1->CR1&=~(1<<15); 	//设置OVER8=0 
	USART1->CR1|=1<<3;  	//串口发送使能 

	USART1->CR1|=1<<2;  	//串口接收使能
	USART1->CR1|=1<<5;    	//接收缓冲区非空中断使能	    	
	MY_NVIC_Init(0,2,USART1_IRQn,2);//组2，最低优先级 

	USART1->CR1|=1<<13;  	//串口使能
	WIFI_RST=WIFI_EN=0;
	delay_ms(200);
  WIFI_RST=WIFI_EN=1;
	WIFI_START();
}

//WIFI发送一个字节
void WIFI_Send_Byte(uint8_t Dat)
{
	USART1->DR=Dat; 
	while((USART1->SR&0X40)==0);//等待发送结束
}
//WIFI发送数组
void WIFI_Send_array(uint8_t *arr, uint16_t len)
{
	uint16_t i=0;
	for(i=0; i<len; i++)
	{
		WIFI_Send_Byte(arr[i]);
	}
}




void WIFI_ReceiveData(void)    
{
    if(WIFI_TimCount >= 50 && WIFI_UartReceiving)                                    
    {  
      WIFI_TimCount = 0;                                                             
      if(WIFI_ANum == WIFI_RxCount)                                                 
      {
        WIFI_UartDataFinishFlag = 1;                                                 
        WIFI_UartReceiving = 0;                                                   
      }
      else
      {
        WIFI_ANum = WIFI_RxCount;
      }
    }
}


extern HTTP_AT_Send_Flag         HTTP_Flag;
extern u8 OTA_buf[1024*50];
extern u32 OTA_Count;
void USART1_IRQHandler(void)
{
  u8 WIFI_DATA;
	if(USART1->SR&(1<<5))//接收到数据
	{	 
		WIFI_DATA=USART1->DR;  //读出数据清除中断标志		
		if(HTTP_Flag.f_AT_TCPSEND_Cm==1) //表述数据OTA
		{
			OTA_buf[OTA_Count++]=WIFI_DATA;
		}
	  WIFI_TimCount=0;
	  WIFI_UartReceiving = 1;    
    if (WIFI_RxCount < sizeof(WIFI_RxBuf)-5)  
    {
      WIFI_RxBuf[WIFI_RxCount++] = WIFI_DATA;
    }
		else 
		{
			WIFI_RxCount = 0;                       
		}	 
	} 
}



uint8_t WAckData(uint8_t *string)
{
    uint16_t i=0;
    for (i=0;i<1024; i++)
    {
      if (memcmp(string, &WIFI_RxBuf[i], strlen((const char *)string)) == 0) 
      {
        WIFI_RxCount=0;
        return 1;
      }
    }
    WIFI_RxCount=0;         
    return 0;
}

void WIFI_START(void)
{
	memset(WIFI_TxBuf,0,1024);
	WIFI_RxCount=0;
	WIFI_UartDataFinishFlag=0;
	WIFI_TimCount=0;
	while(1)
	{
	  WIFI_ReceiveData();
		if(WIFI_UartDataFinishFlag==1)
		{
			WIFI_UartDataFinishFlag=0;
			if((WAckData((void *)"ready"))||(WAckData((void *)"OK\r\n")))
			{
			memset(WIFI_RxBuf, 0, 1024); 
			WIFI_TimCount=0;
			break;
			}
		}
		if(WIFI_TimCount>=2000)
		{
			WIFI_UartDataFinishFlag=0;
			WIFI_Send_array((void *)"AT\r\n",4); 
	    WIFI_TimCount=0;
		}
  }
}



WIFI_AT_Send_Flag         WIFI_Flag={0};
WIFI_AT_Send_Flag_OK      WIFI_OK_Flag={0};
WIFI_AT_Send_Count        WIFI_Count={0};
WIFI_AT_Error             WIFI_Error={0};

void WIFI_Send_AT(void)//发送AT指令
{
		WIFI_Send_array((void *)"AT\r\n",strlen("AT\r\n"));
		WIFI_Count.AT_Ct +=1;
		WIFI_Flag.f_AT_Cm=1;	 
}

void WIFI_Send_ATE(void)//回显
{
		WIFI_Send_array((void *)"ATE0\r\n",strlen("ATE0\r\n"));
		WIFI_Count.ATE_Ct +=1;
		WIFI_Flag.f_ATE_Cm=1;	  
}

void WIFI_Send_BLENAME(void)//设置BLE名
{
		WIFI_Send_array((void *)"AT+BLENAME=LK-IOT\r\n",strlen("AT+BLENAME=LK-IOT\r\n")); 
		WIFI_Count.BLENAME_Ct +=1;
		WIFI_Flag.f_BLENAME_Cm=1;	  
}

void WIFI_Send_BLEAUTH(void)//设置BLE密码
{
		WIFI_Send_array((void *)"AT+BLEAUTH=159521\r\n",strlen("AT+BLEAUTH=159521\r\n")); 
		WIFI_Count.BLENAME_Ct +=1;
		WIFI_Flag.f_BLENAME_Cm=1;	  
}


void WIFI_Send_BLEMODE(void)//设置蓝牙模式
{
		WIFI_Send_array((void *)"AT+BLEMODE=0\r\n",strlen("AT+BLEMODE=0\r\n"));
		WIFI_Count.BLENAME_Ct +=1;
		WIFI_Flag.f_BLENAME_Cm=1;	  
}

void WIFI_Send_BLEADVEN(void)//开启蓝牙
{
		WIFI_Send_array((void *)"AT+BLEADVEN=1\r\n",strlen("AT+BLEADVEN=1\r\n"));
		WIFI_Count.BLEADVEN_Ct +=1;
		WIFI_Flag.f_BLEADVEN_Cm=1;	  
}

void WIFI_Send_WMODE(void)//设置WIFI模式
{
		WIFI_Send_array((void *)"AT+WMODE=1,1\r\n",strlen("AT+WMODE=1,1\r\n"));
		WIFI_Count.WMODE_Ct +=1;
		WIFI_Flag.f_WMODE_Cm=1;	  
}

void WIFI_Send_WAUTOCONN(void)//设置WIFI SSID PWD
{
	  u8 send_buf[100];
	  memset(send_buf,0,100);
	  memcpy(send_buf,"AT+WJAP=",8);
	  strcat((char*)send_buf,(char*)Set_Type.ssid);
	  strcat((char*)send_buf,(char*)",");
	  strcat((char*)send_buf,(char*)Set_Type.pwd);
	  strcat((char*)send_buf,(char*)"\r\n");
		WIFI_Send_array((void *)send_buf,strlen((void *)send_buf));
	  WIFI_Count.WAUTOCONN_Ct +=1;
		WIFI_Flag.f_WAUTOCONN_Cm=1;	  
}

void WIFI_Send_SSNTP(void)//开启时间
{
		WIFI_Send_array((void *)"AT+SNTPTIMECFG=1,8\r\n",strlen("AT+SNTPTIMECFG=1,8\r\n"));
		WIFI_Count.SSNTP_Ct +=1;
		WIFI_Flag.f_SSNTP_Cm=1;	  
}

void WIFI_Send_SNTP(void)//设置时间
{
		WIFI_Send_array((void *)"AT+SNTPTIME?\r\n",strlen("AT+SNTPTIME?\r\n"));
		WIFI_Count.SNTP_Ct +=1;
		WIFI_Flag.f_SNTP_Cm=1;	  
}

void WIFI_Send_SOCKET(void)//建立TCPIP连接
{
	  u8 send_buf[100];
	  memset(send_buf,0,100);
	  memcpy(send_buf,"AT+SOCKET=4,",strlen("AT+SOCKET=4,"));
	  strcat((char*)send_buf,(char*)Set_Type.ip);
	  strcat((char*)send_buf,(char*)",");
	  strcat((char*)send_buf,(char*)Set_Type.port);
	  strcat((char*)send_buf,(char*)"\r\n");
		WIFI_Send_array((void *)send_buf,strlen((void *)send_buf));
	  WIFI_Count.SOCKET_Ct +=1;
		WIFI_Flag.f_SOCKET_Cm=1;	  
}


void WIFI_Send_SOCKETTT(void)//透传模式
{
		WIFI_Send_array((void *)"AT+SOCKETTT\r\n",strlen("AT+SOCKETTT\r\n"));
		WIFI_Count.SOCKETTT_Ct +=1;
		WIFI_Flag.f_SOCKETTT_Cm=1;	  
}


void WIFI_MQTT_Connect(void)//设置MQTT链接
{
	  IoT_Parameter_Init();
		MQTT_ConectPack();
	  WIFI_Send_array(Aep_mqtt.Pack_buff,Aep_mqtt.Fixed_len+Aep_mqtt.Payload_len+Aep_mqtt.Variable_len);
		WIFI_Count.MQTT_Ct +=1;
		WIFI_Flag.f_MQTT_Cm=1;	  
}

uint8_t GetAckData_BLE_SSID(void)
{
    uint16_t i=0;
    for (i=0;i<500; i++)
    {
      if ((WIFI_RxBuf[i]=='s') &&(WIFI_RxBuf[i+1]=='s')&&(WIFI_RxBuf[i+2]=='i')&&(WIFI_RxBuf[i+3]=='d'))  
      {
        return i;
      }
    }        
    return 0;
}
uint8_t GetAckData_BLE_PWD(void)
{
    uint16_t i=0;
    for (i=0;i<200; i++)
    {
      if ((WIFI_RxBuf[i]=='p') &&(WIFI_RxBuf[i+1]=='w')&&(WIFI_RxBuf[i+2]=='d'))  
      {
        return i;
      }
    }        
    return 0;
}
uint8_t GetAckData_WIFI_TIME(void)
{
    uint16_t i=0;
    for (i=0;i<200; i++)
    {
      if ((WIFI_RxBuf[i]=='S') &&(WIFI_RxBuf[i+1]=='N')&&(WIFI_RxBuf[i+2]=='T')&&(WIFI_RxBuf[i+3]=='P'))  
      {
        return i;
      }
    }        
    return 0;
}
u32 WIFI_Step=1;
u8 WIFI_HTTP_Step=1;
extern u8 run;
void WIFI_Send_Command(void)
{
  switch(WIFI_Step)
	{
		case 1://AT验证
		{
				if((WIFI_Count.AT_Ct<200)&&(WIFI_Flag.f_AT_Cm ==0))     //发送次数少于200次，指令发送标志位为0时
						{ WIFI_Send_AT();}
				if(WIFI_Count.AT_Ct == 200)
					{WIFI_Error.f_AT=1;WIFI_Step=1;run=REBOOT;}
		}	                                                   break;
		case 2://关闭回显
		{
				if((WIFI_Count.ATE_Ct<60)&&(WIFI_Flag.f_ATE_Cm ==0))     //发送次数少于60次，指令发送标志位为0时
						{ WIFI_Send_ATE();}
				if(WIFI_Count.ATE_Ct == 60)
					  {WIFI_Error.f_ATE=1;WIFI_Step=1;run=REBOOT;}
		}	                                                   break;	
		case 3://BLE名设置
		{
				if((WIFI_Count.BLENAME_Ct<60)&&(WIFI_Flag.f_BLENAME_Cm ==0))     //发送次数少于60次，指令发送标志位为0时
						{ WIFI_Send_BLENAME();}
				if(WIFI_Count.BLENAME_Ct == 60)
					  {WIFI_Error.f_BLENAME=1;WIFI_Step=1;run=REBOOT;}
		}	                                                   break;	
		case 4://BLE设置密码
		{
				if((WIFI_Count.BLEAUTH_Ct<60)&&(WIFI_Flag.f_BLEAUTH_Cm ==0))     //发送次数少于60次，指令发送标志位为0时
						{ WIFI_Send_BLEAUTH();}
				if(WIFI_Count.BLEAUTH_Ct == 60)
					  {WIFI_Error.f_BLEAUTH=1;WIFI_Step=1;run=REBOOT;}
		}	                                                   break;	
		case 5://设置蓝牙模式
		{
				if((WIFI_Count.BLEMODE_Ct<60)&&(WIFI_Flag.f_BLEMODE_Cm ==0))     //发送次数少于60次，指令发送标志位为0时
						{ WIFI_Send_BLEMODE();}
				if(WIFI_Count.BLEMODE_Ct == 60)
					  {WIFI_Error.f_BLEMODE=1;WIFI_Step=1;run=REBOOT;}
		}	                                                   break;	
		case 6://开启蓝牙
		{
				if((WIFI_Count.BLEADVEN_Ct<60)&&(WIFI_Flag.f_BLEADVEN_Cm ==0))     //发送次数少于60次，指令发送标志位为0时
						{ WIFI_Send_BLEADVEN();}
				if(WIFI_Count.BLEADVEN_Ct == 60)
					  {WIFI_Error.f_BLEADVEN=1;WIFI_Step=1;run=REBOOT;}
		}	                                                   break;	
		case 7://设置WIFI模式
		{
				if((WIFI_Count.WMODE_Ct<60)&&(WIFI_Flag.f_WMODE_Cm ==0))     //发送次数少于60次，指令发送标志位为0时
						{ WIFI_Send_WMODE();}
				if(WIFI_Count.WMODE_Ct == 60)
					  {WIFI_Error.f_WMODE=1;WIFI_Step=1;run=REBOOT;}
		}	                                                   break;	
		case 8://设置SSID PWD
		{
				if((WIFI_Count.WAUTOCONN_Ct<60)&&(WIFI_Flag.f_WAUTOCONN_Cm ==0))     //发送次数少于60次，指令发送标志位为0时
						{ WIFI_Send_WAUTOCONN();delay_ms(5000);}
				if(WIFI_Count.WAUTOCONN_Ct == 60)
					  {WIFI_Error.f_WAUTOCONN=1;WIFI_Step=1;run=REBOOT;}
		}	                                                   break;	
		case 9://开启时间
		{
				if((WIFI_Count.SSNTP_Ct<60)&&(WIFI_Flag.f_SSNTP_Cm ==0))     //发送次数少于60次，指令发送标志位为0时
						{ WIFI_Send_SSNTP();delay_ms(5000);}
				if(WIFI_Count.SSNTP_Ct == 60)
					  {WIFI_Error.f_SSNTP=1;WIFI_Step=1;run=REBOOT;}
		}	                                                   break;	
		case 10://设置时间
		{
				if((WIFI_Count.SNTP_Ct<60)&&(WIFI_Flag.f_SNTP_Cm ==0))     //发送次数少于60次，指令发送标志位为0时
						{ WIFI_Send_SNTP();}
				if(WIFI_Count.SNTP_Ct == 60)
					  {WIFI_Error.f_SNTP=1;WIFI_Step=1;run=REBOOT;}
		}	                                                   break;	
		case 11://建立TCPIP连接
		{
				if((WIFI_Count.SOCKET_Ct<60)&&(WIFI_Flag.f_SOCKET_Cm ==0))     //发送次数少于60次，指令发送标志位为0时
						{ WIFI_Send_SOCKET();}
				if(WIFI_Count.SOCKET_Ct == 60)
					  {WIFI_Error.f_SOCKET=1;WIFI_Step=1;run=REBOOT;}
		}	                                                   break;	
		case 12://透传模式
		{
				if((WIFI_Count.SOCKETTT_Ct<60)&&(WIFI_Flag.f_SOCKETTT_Cm ==0))     //发送次数少于60次，指令发送标志位为0时
						{ WIFI_Send_SOCKETTT();}
				if(WIFI_Count.SOCKETTT_Ct == 60)
					  {WIFI_Error.f_SOCKETTT=1;WIFI_Step=1;run=REBOOT;}
		}	                                                   break;	
		case 13://MQTT设置
		{
				if((WIFI_Count.MQTT_Ct<60)&&(WIFI_Flag.f_MQTT_Cm ==0))     //发送次数少于60次，指令发送标志位为0时
						{ WIFI_MQTT_Connect();}
				if(WIFI_Count.MQTT_Ct == 60)
					  {WIFI_Error.f_MQTT=1;WIFI_Step=1;run=REBOOT;}
		}	                                                   break;	
		case 14:
			MQTT_Subscribe(Aep_mqtt.Stopic_Buff, 1, 0, 1);
		  WIFI_Step++;
		default:																								
			break;
  }
}

extern u8 Net_flag; //链接服务器成功标志位

void Check_WIFI_AT_ReciveData(void)//检查AT接收到的数据
{
	   u16 i,t;
	   u8 mqtt_ack[2]={0x20,0x02};
		 WIFI_ReceiveData();
		 if(WIFI_UartDataFinishFlag==1)
			{
			 if( WIFI_Flag.f_AT_Cm == 1)
							{
								if(strstr((void *)WIFI_RxBuf, "OK")!= NULL)
								{WIFI_Step++;WIFI_Count.AT_Ct=0;}
								WIFI_Flag.f_AT_Cm=0;
							}
			 if( WIFI_Flag.f_ATE_Cm == 1)
							{
									if(strstr((void *)WIFI_RxBuf, "OK")!= NULL)
									{ WIFI_Step++;WIFI_Count.ATE_Ct=0;}
									WIFI_Flag.f_ATE_Cm=0;
							}
			 if( WIFI_Flag.f_BLENAME_Cm == 1)
							{
									if(strstr((void *)WIFI_RxBuf, "OK")!= NULL)
									{ WIFI_Step++;WIFI_Count.BLENAME_Ct=0;}
									WIFI_Flag.f_BLENAME_Cm=0;
							}
			 if( WIFI_Flag.f_BLEAUTH_Cm == 1)
							{
									if(strstr((void *)WIFI_RxBuf, "OK")!= NULL)
									{ WIFI_Step++;WIFI_Count.BLEAUTH_Ct=0;}
									WIFI_Flag.f_BLEAUTH_Cm=0;
							}
			 if( WIFI_Flag.f_BLEMODE_Cm == 1)
							{
									if(strstr((void *)WIFI_RxBuf, "OK")!= NULL)
									{ WIFI_Step++;WIFI_Count.BLEMODE_Ct=0;}
									WIFI_Flag.f_BLEMODE_Cm=0;
							}
			 if( WIFI_Flag.f_BLEADVEN_Cm == 1)
							{
									if(strstr((void *)WIFI_RxBuf, "OK")!= NULL)
									{ WIFI_Step++;WIFI_Count.BLEADVEN_Ct=0;}
									WIFI_Flag.f_BLEADVEN_Cm=0;
							}
			 if( WIFI_Flag.f_WMODE_Cm == 1)
							{
									if(strstr((void *)WIFI_RxBuf, "OK")!= NULL)
									{ WIFI_Step++;WIFI_Count.WMODE_Ct=0;}
									WIFI_Flag.f_WMODE_Cm=0;
							}
			 if( WIFI_Flag.f_WAUTOCONN_Cm == 1)
							{
									if(strstr((void *)WIFI_RxBuf, "WIFI_GOT_IP")!= NULL)
									{ WIFI_Step++;WIFI_Count.WAUTOCONN_Ct=0;}
									WIFI_Flag.f_WAUTOCONN_Cm=0;
							}
			 if( WIFI_Flag.f_SSNTP_Cm == 1)
							{
									if(strstr((void *)WIFI_RxBuf, "SNTP_SYNC_DONE")!= NULL)
									{ WIFI_Step++;WIFI_Count.SSNTP_Ct=0;}
									WIFI_Flag.f_SSNTP_Cm=0;
							}
			 if( WIFI_Flag.f_SNTP_Cm == 1)
							{
									if(strstr((void *)WIFI_RxBuf, "+SNTPTIME")!= NULL)
									{ 
										i=GetAckData_WIFI_TIME();
										 UTCTimer.year=(WIFI_RxBuf[i+31]-0x30)*10+WIFI_RxBuf[i+32]-0x30;
										 UTCTimer.day=(WIFI_RxBuf[i+17]-0x30)*10+WIFI_RxBuf[i+18]-0x30;
										 UTCTimer.hour=(WIFI_RxBuf[i+20]-0x30)*10+WIFI_RxBuf[i+21]-0x30;
										 UTCTimer.minute=(WIFI_RxBuf[i+23]-0x30)*10+WIFI_RxBuf[i+24]-0x30;
										 UTCTimer.second=(WIFI_RxBuf[i+26]-0x30)*10+WIFI_RxBuf[i+27]-0x30;
										 if(strstr((void *)WIFI_RxBuf,"Jan")!= NULL)
										   UTCTimer.months=1;
										 if(strstr((void *)WIFI_RxBuf,"Feb")!= NULL)
										   UTCTimer.months=2;
										 if(strstr((void *)WIFI_RxBuf,"Mar")!= NULL)
										   UTCTimer.months=3;
										 if(strstr((void *)WIFI_RxBuf,"Apr")!= NULL)
										   UTCTimer.months=4;
										 if(strstr((void *)WIFI_RxBuf,"May")!= NULL)
										   UTCTimer.months=5;
										 if(strstr((void *)WIFI_RxBuf,"Jun")!= NULL)
										   UTCTimer.months=6;
										 if(strstr((void *)WIFI_RxBuf,"Jul")!= NULL)
										   UTCTimer.months=7;
										 if(strstr((void *)WIFI_RxBuf,"Aug")!= NULL)
										   UTCTimer.months=8;
										 if(strstr((void *)WIFI_RxBuf,"Sep")!= NULL)
										   UTCTimer.months=9;
										 if(strstr((void *)WIFI_RxBuf,"Oct")!= NULL)
										   UTCTimer.months=10;
										 if(strstr((void *)WIFI_RxBuf,"Nov")!= NULL)
										   UTCTimer.months=11;
										 if(strstr((void *)WIFI_RxBuf,"Dec")!= NULL)
										   UTCTimer.months=12;
   										 UTCTimer.week=RTC_Get_Week(UTCTimer.year,UTCTimer.months,UTCTimer.day);
										   RTC_Set_Date(UTCTimer.year,UTCTimer.months,UTCTimer.day,UTCTimer.week);
										   RTC_Set_Time(UTCTimer.hour,UTCTimer.minute,UTCTimer.second,0);
										WIFI_Step++;
										WIFI_Count.SNTP_Ct=0;
									}
									WIFI_Flag.f_SNTP_Cm=0;
							}
			 if( WIFI_Flag.f_SOCKET_Cm == 1)
							{
									if(strstr((void *)WIFI_RxBuf, "connect success")!= NULL)
									{ WIFI_Step++;WIFI_Count.SOCKET_Ct=0;}
									WIFI_Flag.f_SOCKET_Cm=0;
							}	
       if( WIFI_Flag.f_SOCKETTT_Cm == 1)
							{
									if(strstr((void *)WIFI_RxBuf, ">")!= NULL)
									{ WIFI_Step++;WIFI_Count.SOCKETTT_Ct=0;}
									WIFI_Flag.f_SOCKETTT_Cm=0;
							}									
							
			 if(WIFI_Flag.f_MQTT_Cm  == 1)
							{
									if(strstr((void *)WIFI_RxBuf, (void *)mqtt_ack)!= NULL)
									{
										   WIFI_Step++;WIFI_Count.MQTT_Ct=0;
										   Net_flag=1;   //连接MQTT服务器成功
									}
									WIFI_Flag.f_MQTT_Cm=0;
							}					
							
			 if((strstr((void *)WIFI_RxBuf,"ssid")!= NULL)&&(strstr((void *)WIFI_RxBuf,"pwd")!= NULL))  //{"ssid":"LX_123456","pwd":"lx123456"}
							{
								  memset(Set_Type.ssid,0,20);
								  memset(Set_Type.pwd,0,20);
									i=GetAckData_BLE_SSID();
								  if(i!=0)
									{
									i=i+7;
									t=0;
									while(WIFI_RxBuf[i+t]!='\"')
									{
                  Set_Type.ssid[t]=WIFI_RxBuf[i+t];		
									t++;
									}										
									}
                  i=GetAckData_BLE_PWD();		
                  if(i!=0)
									{
									i=i+6;
									t=0;
									while(WIFI_RxBuf[i+t]!='\"')
									{
                  Set_Type.pwd[t]=WIFI_RxBuf[i+t];		
									t++;
									}										
									}		
								W25QXX_Write(Set_Type.ssid,121,20); 	
								W25QXX_Write(Set_Type.pwd,141,20); 	
                WIFI_Step=7;	WIFI_Count.WMODE_Ct=0;									
							}	
			 if(strstr((void *)WIFI_RxBuf,"WIFI_DISCONNECT")!= NULL)  //{"ssid":"LX_123456","pwd":"lx123456"}
							{
									 
							}	
			 WIFI_RxCount=0;
			 WIFI_TimCount=0;
			 WIFI_UartDataFinishFlag=0;
			 memset(WIFI_RxBuf,0,1024);
			 memset(WIFI_TxBuf,0,1024);									
			}
}





WIFI_AT_HTTP_Flag         HTTP_WIFI_Flag={0};
WIFI_AT_HTTP_Flag_OK      HTTP_WIFI_OK_Flag={0};
WIFI_AT_HTTP_Count        HTTP_WIFI_Count={0};
WIFI_HTTP_Error           HTTP_WIFI_Error={0};

extern uint8_t  HTTP_IP[50];
extern uint8_t  HTTP_PATH[50];
extern u8 HTTP_Send_Buf[200];
void WIFI_HTTP_Send_TCP_SEND(void)
{
	  memset(HTTP_Send_Buf,0,200);
	  memcpy(HTTP_Send_Buf,"GET ",4);
	  strcat((char*)HTTP_Send_Buf,(char*)HTTP_PATH);
	  strcat((char*)HTTP_Send_Buf,(char*)" HTTP/1.1\r\n");
	  strcat((char*)HTTP_Send_Buf,(char*)"Host:");
	  strcat((char*)HTTP_Send_Buf,(char*)HTTP_IP);
	  strcat((char*)HTTP_Send_Buf,(char*)"\r\nConnection:keep-alive\r\n\r\n");
	  ML307R_Send_array(HTTP_Send_Buf,strlen((void *)HTTP_Send_Buf));
		HTTP_WIFI_Count.HTTP_Ct +=1;
		HTTP_WIFI_Flag.f_HTTP_Cm=1;	 
}

void WIFI_HTTP_Send_Command(void)
{
  switch(WIFI_HTTP_Step)
	{
		case 1://AT验证
		{
				if((HTTP_WIFI_Count.AT_Ct<200)&&(HTTP_WIFI_Flag.f_AT_Cm ==0))     //发送次数少于200次，指令发送标志位为0时
						{ WIFI_Send_AT();}
				if(HTTP_WIFI_Count.AT_Ct == 200)
					{HTTP_WIFI_Error.f_AT=1;WIFI_HTTP_Step=1;run=REBOOT;}
		}	                                                   break;
		case 2://关闭回显
		{
				if((HTTP_WIFI_Count.ATE_Ct<60)&&(HTTP_WIFI_Flag.f_ATE_Cm ==0))     //发送次数少于60次，指令发送标志位为0时
						{ WIFI_Send_ATE();}
				if(HTTP_WIFI_Count.ATE_Ct == 60)
					  {HTTP_WIFI_Error.f_ATE=1;WIFI_HTTP_Step=1;run=REBOOT;}
		}	                                                   break;	
		case 3://设置WIFI模式
		{
				if((HTTP_WIFI_Count.WMODE_Ct<60)&&(HTTP_WIFI_Flag.f_WMODE_Cm ==0))     //发送次数少于60次，指令发送标志位为0时
						{ WIFI_Send_WMODE();}
				if(HTTP_WIFI_Count.WMODE_Ct == 60)
					  {HTTP_WIFI_Error.f_WMODE=1;WIFI_HTTP_Step=1;run=REBOOT;}
		}	                                                   break;	
		case 4://设置SSID PWD
		{
				if((HTTP_WIFI_Count.WAUTOCONN_Ct<60)&&(HTTP_WIFI_Flag.f_WAUTOCONN_Cm ==0))     //发送次数少于60次，指令发送标志位为0时
						{ WIFI_Send_WAUTOCONN();delay_ms(5000);}
				if(HTTP_WIFI_Count.WAUTOCONN_Ct == 60)
					  {HTTP_WIFI_Error.f_WAUTOCONN=1;WIFI_HTTP_Step=1;run=REBOOT;}
		}	                                                   break;	
		case 5://建立TCPIP连接
		{
				if((HTTP_WIFI_Count.SOCKET_Ct<60)&&(HTTP_WIFI_Flag.f_SOCKET_Cm ==0))     //发送次数少于60次，指令发送标志位为0时
						{ WIFI_Send_SOCKET();}
				if(HTTP_WIFI_Count.SOCKET_Ct == 60)
					  {HTTP_WIFI_Error.f_SOCKET=1;WIFI_HTTP_Step=1;run=REBOOT;}
		}	                                                   break;	
		case 6://透传模式
		{
				if((HTTP_WIFI_Count.SOCKETTT_Ct<60)&&(HTTP_WIFI_Flag.f_SOCKETTT_Cm ==0))     //发送次数少于60次，指令发送标志位为0时
						{ WIFI_Send_SOCKETTT();}
				if(HTTP_WIFI_Count.SOCKETTT_Ct == 60)
					  {HTTP_WIFI_Error.f_SOCKETTT=1;WIFI_HTTP_Step=1;run=REBOOT;}
		}	                                                   break;	
		case 7://HTTP设置
		{
				if((HTTP_WIFI_Count.HTTP_Ct<60)&&(HTTP_WIFI_Flag.f_HTTP_Cm ==0))     //发送次数少于60次，指令发送标志位为0时
						{ WIFI_HTTP_Send_TCP_SEND();}
				if(HTTP_WIFI_Count.HTTP_Ct == 60)
					  {HTTP_WIFI_Error.f_HTTP=1;WIFI_HTTP_Step=1;run=REBOOT;}
		}	                                                   break;	
		case 8:
			 
		default:																								
			break;
  }
}




void Check_WIFI_HTTP_AT_ReciveData(void)//检查AT接收到的数据
{
		 WIFI_ReceiveData();
		 if(WIFI_UartDataFinishFlag==1)
			{
			 if( HTTP_WIFI_Flag.f_AT_Cm == 1)
							{
								if(strstr((void *)WIFI_RxBuf, "OK")!= NULL)
								{WIFI_HTTP_Step++;HTTP_WIFI_Count.AT_Ct=0;}
								HTTP_WIFI_Flag.f_AT_Cm=0;
							}
			 if( HTTP_WIFI_Flag.f_ATE_Cm == 1)
							{
									if(strstr((void *)WIFI_RxBuf, "OK")!= NULL)
									{ WIFI_HTTP_Step++;HTTP_WIFI_Count.ATE_Ct=0;}
									HTTP_WIFI_Flag.f_ATE_Cm=0;
							}
			 if( HTTP_WIFI_Flag.f_WMODE_Cm == 1)
							{
									if(strstr((void *)WIFI_RxBuf, "OK")!= NULL)
									{ WIFI_HTTP_Step++;HTTP_WIFI_Count.WMODE_Ct=0;}
									HTTP_WIFI_Flag.f_WMODE_Cm=0;
							}
			 if( HTTP_WIFI_Flag.f_WAUTOCONN_Cm == 1)
							{
									if(strstr((void *)WIFI_RxBuf, "WIFI_GOT_IP")!= NULL)
									{ WIFI_HTTP_Step++;HTTP_WIFI_Count.WAUTOCONN_Ct=0;}
									HTTP_WIFI_Flag.f_WAUTOCONN_Cm=0;
							}
			 if( HTTP_WIFI_Flag.f_SOCKET_Cm == 1)
							{
									if(strstr((void *)WIFI_RxBuf, "connect success")!= NULL)
									{ WIFI_HTTP_Step++;HTTP_WIFI_Count.SOCKET_Ct=0;}
									HTTP_WIFI_Flag.f_SOCKET_Cm=0;
							}	
       if( HTTP_WIFI_Flag.f_SOCKETTT_Cm == 1)
							{
									if(strstr((void *)WIFI_RxBuf, ">")!= NULL)
									{ WIFI_HTTP_Step++;HTTP_WIFI_Count.SOCKETTT_Ct=0;}
									HTTP_WIFI_Flag.f_SOCKETTT_Cm=0;
							}									
					
									
			 if(strstr((void *)WIFI_RxBuf,"WIFI_DISCONNECT")!= NULL)  //{"ssid":"LX_123456","pwd":"lx123456"}
							{
									 
							}	
			 WIFI_RxCount=0;
			 WIFI_TimCount=0;
			 WIFI_UartDataFinishFlag=0;
			 memset(WIFI_RxBuf,0,1024);
			 memset(WIFI_TxBuf,0,1024);									
			}
}
















