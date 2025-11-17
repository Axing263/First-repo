#include "Process.h"
u32 START_Time;
u32 WORK_Time;
u8 Ping_Interval;
extern u32 Time_Interval;
extern u16 Current_Set;
extern u16 Voltage_Set;
extern u16 Current_Alarm;
extern u16 Voltage_Alarm;
extern double Voltage,Current,Power;   //电压 电流 累计功耗
u8 Net_flag=0;
u8 Renew_flag = 0;
extern uint8_t  ML307R_TxBuf[1024];
extern _GPSData GPSData;
u16 Count_Interval;


extern uint8_t  ML307R_RxBuf[1024];       
extern uint8_t  ML307R_TxBuf[1024];
extern uint16_t ML307R_RxCount;      
extern uint16_t ML307R_ANum;   
extern uint8_t  ML307R_UartReceiving;
extern uint8_t  ML307R_UartDataFinishFlag;


extern uint32_t WIFI_TimCount; 
extern uint8_t  WIFI_RxBuf[1024];       
extern uint8_t  WIFI_TxBuf[1024];
extern uint16_t WIFI_RxCount;      
extern uint16_t WIFI_ANum;   
extern uint8_t  WIFI_UartReceiving;
extern uint8_t  WIFI_UartDataFinishFlag;



extern u8 run;
extern u32 water_mm;

uint8_t  HTTP_IP[50];
uint8_t  HTTP_PATH[50];
uint32_t  File_Size;


double Va,Vb,Vc,Ia,Ib,Ic,Es,Ep;
double wf,wlf;
double gf,glf,gp,gt;

u32 Dead_Time;
u32 Dead_Flag;
/*
W25Q64BV拥有32768页的可编程阵列(每页可写256字节数据)。
一次最大可编辑256字节。每页可以被分成16组扇区擦除,每个扇区分成128（32KB区擦除），
或256个（64KB区擦除）或者整个擦除（整个芯片擦除）。W25Q64BV有2048个可擦除扇区和128个可擦除区块。
最小4KB的扇区擦除拥有更好的灵活性在数据和参数存储上*/


//W25Q64 存储区域
//64Mbit
//8Mbyte
//32768Page
//每页256字节


void Init(void)
{
	Stm32_Clock_Init(336,16,2,7);//设置时钟,168Mhz
	__enable_irq();              //开启中断源
	delay_init(168);		         //初始化延时函数
	RS485_Init(42,9600);        //RS485初始化

	GPS_Init(42,115200);           //GPS初始化
	RTC_Init();		 			         //初始化RTC
	FML_TIME_Init();             //TIM3  1ms中断 
	ADC1_Init();                 //ADC_DMA初始化
	RC522_Init();
	W25QXX_Init();               //W25Q64存储器初始化  
  switch(run)
		{
			case MQTT:
			  ML307R_Init(84,115200);      //串口6初始化   
				break;
			case WIFI_MQTT:
				ESP8266_Init(84,115200);     //WIFI初始化	
				break;
		}	
  Read_Set();                  //读取设置参数
	IoT_Parameter_Init();
}

void GET_Data(void)
{
	ADC_Get();
}
 
void Date_Pack(u8 mode)
{
	u8 Pub_Topic_Buf[50];
	u8 Time_Buf[100];
	u16 Send_Size;
	/*推送主题*/
	memset(Pub_Topic_Buf,0,50);
	//memcpy(Pub_Topic_Buf,"lkkj/lk4g-hj211/pub/welding-machine",strlen("lkkj/lk4g-hj211/pub/welding-machine"));
	memcpy(Pub_Topic_Buf,"lkkj/lk4g-hj211/pub/sn",strlen("lkkj/lk4g-hj211/pub/sn"));

	//strcat((char*)Pub_Topic_Buf,(char*)ML307R_Inf.ML307R_IMEI);
	//strcat((char*)Pub_Topic_Buf,(char*)"/property/p");
	/*获取时间*/
	RTC_Get_Time();
	RTC_Get_Date();
	memset((void *)Time_Buf,0,100);
	sprintf((char*)Time_Buf,(void *)"20%02d-%02d-%02d %02d:%02d:%02d",UTCTimer.year,UTCTimer.months,UTCTimer.day,UTCTimer.hour,UTCTimer.minute,UTCTimer.second);
	
	memset(ML307R_TxBuf,0,1024);
	memcpy(ML307R_TxBuf,"{\"sn\":\"",strlen("{\"sn\":\""));
	strcat((char*)ML307R_TxBuf,(char*)ML307R_Inf.ML307R_IMEI);
	strcat((char*)ML307R_TxBuf,"\",\"imsi\":\"");  
	strcat((char*)ML307R_TxBuf,(char*)ML307R_Inf.ML307R_IMSI);
	strcat((char*)ML307R_TxBuf,"\",\"t\":\""); 
	strcat((char*)ML307R_TxBuf,(char*)Time_Buf);
//	
//	/***********设备类型********************************/
//	//1-焊机
//	//2-电表
//	//3-水表
//	//4-气表
//	//5-水位仪
	strcat((char*)ML307R_TxBuf,"\",\"type\":\""); 
	strcat((char*)ML307R_TxBuf,(char*)"4");
	
	strcat((char*)ML307R_TxBuf,"\",\"data\":{\"rt\":\"");
	Send_Size=strlen((char*)ML307R_TxBuf);
  sprintf((char*)ML307R_TxBuf+Send_Size,"%d", START_Time);	
  strcat((char*)ML307R_TxBuf,"\",\"wt\":\"");	
	Send_Size=strlen((char*)ML307R_TxBuf);
	sprintf((char*)ML307R_TxBuf+Send_Size,"%d", WORK_Time);	
	
	/***************焊机参数******************************/
	strcat((char*)ML307R_TxBuf,"\",\"v\":\"");	
	Send_Size=strlen((char*)ML307R_TxBuf);
	sprintf((char*)ML307R_TxBuf+Send_Size,"%.1f", Voltage);	
	
	strcat((char*)ML307R_TxBuf,"\",\"c\":\"");
  Send_Size=strlen((char*)ML307R_TxBuf);
	sprintf((char*)ML307R_TxBuf+Send_Size,"%.1f", Current);	
	
	strcat((char*)ML307R_TxBuf,"\",\"p\":\"");
  Send_Size=strlen((char*)ML307R_TxBuf);
	sprintf((char*)ML307R_TxBuf+Send_Size,"%.4f", Power);	
	
	
	
	
	
	
	/**************水位仪参数******************************/		
//	strcat((char*)ML307R_TxBuf,"\",\"wl\":\"");
//	Send_Size=strlen((char*)ML307R_TxBuf);
//	sprintf((char*)ML307R_TxBuf+Send_Size,"%d", water_mm);	
  
	/**************电表参数********************************/
//	strcat((char*)ML307R_TxBuf,"\",\"a\":\"");
//	Send_Size=strlen((char*)ML307R_TxBuf);
//	sprintf((char*)ML307R_TxBuf+Send_Size,"%.1f", Va);	
//	
//	strcat((char*)ML307R_TxBuf,"\",\"b\":\"");
//	Send_Size=strlen((char*)ML307R_TxBuf);
//	sprintf((char*)ML307R_TxBuf+Send_Size,"%.1f", Vb);	
//	
//	strcat((char*)ML307R_TxBuf,"\",\"c\":\"");
//	Send_Size=strlen((char*)ML307R_TxBuf);
//	sprintf((char*)ML307R_TxBuf+Send_Size,"%.1f", Vc);	
//	
//	strcat((char*)ML307R_TxBuf,"\",\"ia\":\"");
//	Send_Size=strlen((char*)ML307R_TxBuf);
//	sprintf((char*)ML307R_TxBuf+Send_Size,"%.1f", Ia);	
//	
//	strcat((char*)ML307R_TxBuf,"\",\"ib\":\"");
//	Send_Size=strlen((char*)ML307R_TxBuf);
//	sprintf((char*)ML307R_TxBuf+Send_Size,"%.1f", Ia);	
//	
//	strcat((char*)ML307R_TxBuf,"\",\"ic\":\"");
//	Send_Size=strlen((char*)ML307R_TxBuf);
//	sprintf((char*)ML307R_TxBuf+Send_Size,"%.1f", Ic);	
//	
//	strcat((char*)ML307R_TxBuf,"\",\"s\":\"");
//	Send_Size=strlen((char*)ML307R_TxBuf);
//	sprintf((char*)ML307R_TxBuf+Send_Size,"%.1f", Es);	
//	
//	strcat((char*)ML307R_TxBuf,"\",\"p\":\"");
//	Send_Size=strlen((char*)ML307R_TxBuf);
//	sprintf((char*)ML307R_TxBuf+Send_Size,"%.1f", Ep);
	
	/**************水表参数********************************/
//	strcat((char*)ML307R_TxBuf,"\",\"wf\":\"");
//	Send_Size=strlen((char*)ML307R_TxBuf);
//	sprintf((char*)ML307R_TxBuf+Send_Size,"%.1f", wf);	
//	
//	strcat((char*)ML307R_TxBuf,"\",\"lf\":\"");
//	Send_Size=strlen((char*)ML307R_TxBuf);
//	sprintf((char*)ML307R_TxBuf+Send_Size,"%.1f", wlf);	
	
	
	/**************气表参数********************************/
//	strcat((char*)ML307R_TxBuf,"\",\"gf\":\"");
//	Send_Size=strlen((char*)ML307R_TxBuf);
//	sprintf((char*)ML307R_TxBuf+Send_Size,"%.1f", gf);	
//	
//	strcat((char*)ML307R_TxBuf,"\",\"lf\":\"");
//	Send_Size=strlen((char*)ML307R_TxBuf);
//	sprintf((char*)ML307R_TxBuf+Send_Size,"%.1f", glf);	
//	
//	strcat((char*)ML307R_TxBuf,"\",\"gt\":\"");
//	Send_Size=strlen((char*)ML307R_TxBuf);
//	sprintf((char*)ML307R_TxBuf+Send_Size,"%.1f", gt);	
//	
//	strcat((char*)ML307R_TxBuf,"\",\"gp\":\"");
//	Send_Size=strlen((char*)ML307R_TxBuf);
//	sprintf((char*)ML307R_TxBuf+Send_Size,"%.1f", gp);	


  strcat((char*)ML307R_TxBuf,"\",\"lat\":\"");
	strcat((char*)ML307R_TxBuf,GPSData.Latitude);
	strcat((char*)ML307R_TxBuf,"\",\"lon\":\"");
	strcat((char*)ML307R_TxBuf,GPSData.Longitude);
	strcat((char*)ML307R_TxBuf,"\",\"ver\":\"");
	strcat((char*)ML307R_TxBuf,(char *)Set_Type.ver);
	strcat((char*)ML307R_TxBuf,"\"}}");
 	MQTT_PublishQs0((void *)Pub_Topic_Buf,(void *)ML307R_TxBuf,strlen((void *)ML307R_TxBuf),mode);
}



uint16_t MQTTAckData(uint8_t *string)
{
    uint16_t i=0;
    for (i=0;i<1024; i++)
    {
      if (memcmp(string, &ML307R_RxBuf[i], strlen((const char *)string)) == 0) 
      {
        return i;
      }
    }     
    return 0;
}


uint16_t MQTT_WIFI_AckData(uint8_t *string)
{
    uint16_t i=0;
    for (i=0;i<1024; i++)
    {
      if (memcmp(string, &WIFI_RxBuf[i], strlen((const char *)string)) == 0) 
      {
        return i;
      }
    }     
    return 0;
}
void Copy(u16 num,u8 *buf)
{
	uint16_t i=0;
	while(ML307R_RxBuf[num]!='"')
	{
		buf[i]=ML307R_RxBuf[num];
		i++;
		num++;
	}
}

extern u8 HTTP_Step;
extern u8 WIFI_HTTP_Step;
u8 Finish_Flag;
u8 Change_Flag;
void REC_Analysis(void)
{
	u16 num;
	u16 i=0,j=0;
	u8 Buf[50];
	u8 Read_Buf[255];
	u8 Write_Buf[255];
	memset(Read_Buf,0,255);
	W25QXX_Read(Read_Buf,0,120);  
	memcpy(Set_Type.ver,Read_Buf,10);
	memcpy(Set_Type.interval,Read_Buf+10,4);
	memcpy(Set_Type.port,Read_Buf+18,10);
	memcpy(Set_Type.ip,Read_Buf+28,30);
	memcpy(Set_Type.votage_set,Read_Buf+109,2);	
	memcpy(Set_Type.current_set,Read_Buf+111,2);	
	memcpy(Set_Type.url,Read_Buf+58,50);
	memcpy(Set_Type.flag,Read_Buf+108,1);
	memcpy(Set_Type.flilesize,Read_Buf+14,4);	
	
	memcpy(Set_Type.current_alarm,Read_Buf+115,2);
	memcpy(Set_Type.votage_alarm,Read_Buf+113,2);	
	
	num=MQTTAckData((void *)"\"type\":");
	if(ML307R_RxBuf[num+8]=='1')
	{
		memset(Buf,0,50);
		num=MQTTAckData((void *)"\"url\":");
		Copy(num+7,Buf);
		if(num!=0)
		{
		memset(Set_Type.url,0,50);
		memcpy(Set_Type.url,(void *)Buf,strlen((void *)Buf));
		memset(HTTP_IP,0,50);	
		memset(HTTP_PATH,0,50);	
		num=MQTTAckData((void *)"//");
		while(ML307R_RxBuf[i+num+2]!='/')
		{
			HTTP_IP[i]=ML307R_RxBuf[num+2+i];
			i++;
		}
		while(ML307R_RxBuf[num+2+i]!='"')
		{
			HTTP_PATH[j]=ML307R_RxBuf[num+2+i];
			i++;
			j++;
		}
		}
		
		memset(Buf,0,50);
		num=MQTTAckData((void *)"\"filesize\":");
		Copy(num+12,Buf);
		if(num!=0)
		{
		memset(Set_Type.flilesize,0,4);
		Set_Type.flilesize[0]=atoi((void *)Buf)/256/256/256;
		Set_Type.flilesize[1]=atoi((void *)Buf)/256/256;
		Set_Type.flilesize[2]=atoi((void *)Buf)/256;
		Set_Type.flilesize[3]=atoi((void *)Buf)%256;
		File_Size=atoi((void *)Buf);
		}
		
		memset(Buf,0,50);
		num=MQTTAckData((void *)"\"ver\":");
		Copy(num+7,Buf);
		if(num!=0)
		{
			if(memcmp(Set_Type.ver,Buf,5)!=0)  //判断软件版本不一样
			{
			run=HTTP; 
			HTTP_Step=1;
			Net_flag=0;
		  memset(Set_Type.ver,0,10);
		  memcpy(Set_Type.ver,(void *)Buf,strlen((void *)Buf));
			ML307R_Init(84,115200);      //串口6初始化 
			}
		}
	}
	else
	{
		memset(Buf,0,50);
		num=MQTTAckData((void *)"\"v\":");
		Copy(num+5,Buf);
		if(num!=0)
		{
		memset(Set_Type.votage_set,0,2);
		Set_Type.votage_set[0]=atoi((void *)Buf)/256;
		Set_Type.votage_set[1]=atoi((void *)Buf)%256;
		Voltage_Set=atoi((void *)Buf);
			Change_Flag = 1;
		}
		
		memset(Buf,0,50);
		num=MQTTAckData((void *)"\"c\":");
		Copy(num+5,Buf);
		if(num!=0)
		{
		memset(Set_Type.current_set,0,2);
		Set_Type.current_set[0]=atoi((void *)Buf)/256;
		Set_Type.current_set[1]=atoi((void *)Buf)%256;
		Current_Set=atoi((void *)Buf);
			Change_Flag = 2;
		}
		
		memset(Buf,0,50);
		num=MQTTAckData((void *)"\"v_a\":");
		Copy(num+7,Buf);
		if(num!=0)
		{
		memset(Set_Type.votage_alarm,0,2);
		Set_Type.votage_alarm[0]=atoi((void *)Buf)/256;
		Set_Type.votage_alarm[1]=atoi((void *)Buf)%256;
		Voltage_Alarm=atoi((void *)Buf);
		}
		
		memset(Buf,0,50);
		num=MQTTAckData((void *)"\"c_a\":");
		Copy(num+7,Buf);
		if(num!=0)
		{
		memset(Set_Type.current_alarm,0,2);
		Set_Type.current_alarm[0]=atoi((void *)Buf)/256;
		Set_Type.current_alarm[1]=atoi((void *)Buf)%256;
		Current_Alarm=atoi((void *)Buf);
		}
		
		memset(Buf,0,50);
		num=MQTTAckData((void *)"\"interval\":");
		Copy(num+12,Buf);
		if(num!=0)
		{
		memset(Set_Type.interval,0,4);
		Set_Type.interval[0]=atoi((void *)Buf)/256/256/256;
		Set_Type.interval[1]=atoi((void *)Buf)/256/256;
		Set_Type.interval[2]=atoi((void *)Buf)/256;
		Set_Type.interval[3]=atoi((void *)Buf)%256;
		Time_Interval=atoi((void *)Buf);
			Change_Flag = 3;
		}
		
		memset(Buf,0,50);
		num=MQTTAckData((void *)"\"ip\":");
		Copy(num+6,Buf);
		if(num!=0)
		{
		run=REBOOT;
		memset(Set_Type.ip,0,30);
		memcpy(Set_Type.ip,(void *)Buf,strlen((void *)Buf));
		Renew_flag = 1;
		}
		
		memset(Buf,0,50);
		num=MQTTAckData((void *)"\"port\":");
		Copy(num+8,Buf);
		if(num!=0)
		{
		memset(Set_Type.port,0,10);
		memcpy(Set_Type.port,(void *)Buf,strlen((void *)Buf));
			
		}
		
	}
	memset(Write_Buf,0,120);
	memcpy(Write_Buf,Set_Type.ver,10);
	memcpy(Write_Buf+10,Set_Type.interval,4); 
	memcpy(Write_Buf+14,Set_Type.flilesize,4);
	memcpy(Write_Buf+18,Set_Type.port,10);
	memcpy(Write_Buf+28,Set_Type.ip,30);
	memcpy(Write_Buf+58,Set_Type.url,50);
	memcpy(Write_Buf+108,Set_Type.flag,1);
	memcpy(Write_Buf+109,Set_Type.votage_set,2);
	memcpy(Write_Buf+111,Set_Type.current_set,2);
	memcpy(Write_Buf+113,Set_Type.votage_alarm,2);
	memcpy(Write_Buf+115,Set_Type.current_alarm,2);
	W25QXX_Write(Write_Buf,0,120); 
	Finish_Flag = 1;
	Read_Set();
	MQTT_AckData(0);
	if(Renew_flag == 1)
	{
		Renew_flag = 0;
		run=REBOOT;
	}
	
}

void MQTT_AckData(u8 mode)
{
	u8 Ack_Topic_Buf[50];
	u16 Ack_Size;

	/*推送主题*/
	memset(Ack_Topic_Buf,0,50);
	memcpy(Ack_Topic_Buf,"lkkj/lk4g-hj211/pub/ack/sn",strlen("lkkj/lk4g-hj211/pub/ack/sn"));

	//strcat((char*)Pub_Topic_Buf,(char*)ML307R_Inf.ML307R_IMEI);
	//strcat((char*)Pub_Topic_Buf,(char*)"/property/p");
	
	memset(ML307R_TxBuf,0,1024);
	memcpy(ML307R_TxBuf,"{\"sn\":\"",strlen("{\"sn\":\""));
	strcat((char*)ML307R_TxBuf,(char*)ML307R_Inf.ML307R_IMEI);

	strcat((char*)ML307R_TxBuf,"\",\"mode\":\""); 
	Ack_Size=strlen((char*)ML307R_TxBuf);
	sprintf((char*)ML307R_TxBuf+Ack_Size,"%d", Change_Flag);
	//strcat((char*)ML307R_TxBuf,(char*)"3");
	
	strcat((char*)ML307R_TxBuf,"\",\"data\":{\"ff\":\"");
	Ack_Size=strlen((char*)ML307R_TxBuf);
  sprintf((char*)ML307R_TxBuf+Ack_Size,"%d", Finish_Flag);	
	strcat((char*)ML307R_TxBuf,"\"}}");
	
	MQTT_PublishQs0((void *)Ack_Topic_Buf,(void *)ML307R_TxBuf,strlen((void *)ML307R_TxBuf),mode);

}



void MQTT_Analysis(void)
{
	u16 num;
	u16 i=0,j=0;
	u8 Buf[50];
	u8 Read_Buf[255];
	u8 Write_Buf[255];
	memset(Read_Buf,0,255);
	W25QXX_Read(Read_Buf,0,120);  
	memcpy(Set_Type.ver,Read_Buf,10);
	memcpy(Set_Type.interval,Read_Buf+10,4);
	memcpy(Set_Type.port,Read_Buf+18,10);
	memcpy(Set_Type.ip,Read_Buf+28,30);
	memcpy(Set_Type.votage_set,Read_Buf+109,2);	
	memcpy(Set_Type.current_set,Read_Buf+111,2);	
	memcpy(Set_Type.url,Read_Buf+58,50);
	memcpy(Set_Type.flag,Read_Buf+108,1);
	memcpy(Set_Type.flilesize,Read_Buf+14,4);	
	
	memcpy(Set_Type.current_alarm,Read_Buf+115,2);
	memcpy(Set_Type.votage_alarm,Read_Buf+113,2);	
	
	num=MQTT_WIFI_AckData((void *)"\"type\":");
	if(WIFI_RxBuf[num+8]=='1')
	{
		memset(Buf,0,50);
		num=MQTTAckData((void *)"\"url\":");
		Copy(num+7,Buf);
		if(num!=0)
		{
		memset(Set_Type.url,0,50);
		memcpy(Set_Type.url,(void *)Buf,strlen((void *)Buf));
		memset(HTTP_IP,0,50);	
		memset(HTTP_PATH,0,50);	
		num=MQTTAckData((void *)"//");
		while(WIFI_RxBuf[i+num+2]!='/')
		{
			HTTP_IP[i]=WIFI_RxBuf[num+2+i];
			i++;
		}
		while(WIFI_RxBuf[num+2+i]!='"')
		{
			HTTP_PATH[j]=WIFI_RxBuf[num+2+i];
			i++;
			j++;
		}
		}
		
		memset(Buf,0,50);
		num=MQTTAckData((void *)"\"filesize\":");
		Copy(num+12,Buf);
		if(num!=0)
		{
		memset(Set_Type.flilesize,0,4);
		Set_Type.flilesize[0]=atoi((void *)Buf)/256/256/256;
		Set_Type.flilesize[1]=atoi((void *)Buf)/256/256;
		Set_Type.flilesize[2]=atoi((void *)Buf)/256;
		Set_Type.flilesize[3]=atoi((void *)Buf)%256;
		File_Size=atoi((void *)Buf);
		}
		
		memset(Buf,0,50);
		num=MQTTAckData((void *)"\"ver\":");
		Copy(num+7,Buf);
		if(num!=0)
		{
			if(memcmp(Set_Type.ver,Buf,5)!=0)  //判断软件版本不一样
			{
			run=WIFI_HTTP; 
			WIFI_HTTP_Step=1;
			Net_flag=0;
		  memset(Set_Type.ver,0,10);
		  memcpy(Set_Type.ver,(void *)Buf,strlen((void *)Buf));
			ESP8266_Init(84,115200);      //串口6初始化 
			}
		}
	}
	else
	{
		memset(Buf,0,50);
		num=MQTTAckData((void *)"\"v\":");
		Copy(num+5,Buf);
		if(num!=0)
		{
		memset(Set_Type.votage_set,0,2);
		Set_Type.votage_set[0]=atoi((void *)Buf)/256;
		Set_Type.votage_set[1]=atoi((void *)Buf)%256;
		Voltage_Set=atoi((void *)Buf);
		}
		
		memset(Buf,0,50);
		num=MQTTAckData((void *)"\"c\":");
		Copy(num+5,Buf);
		if(num!=0)
		{
		memset(Set_Type.current_set,0,2);
		Set_Type.current_set[0]=atoi((void *)Buf)/256;
		Set_Type.current_set[1]=atoi((void *)Buf)%256;
		Current_Set=atoi((void *)Buf);
		}
		
		memset(Buf,0,50);
		num=MQTTAckData((void *)"\"v_a\":");
		Copy(num+7,Buf);
		if(num!=0)
		{
		memset(Set_Type.votage_alarm,0,2);
		Set_Type.votage_alarm[0]=atoi((void *)Buf)/256;
		Set_Type.votage_alarm[1]=atoi((void *)Buf)%256;
		Voltage_Alarm=atoi((void *)Buf);
		}
		
		memset(Buf,0,50);
		num=MQTTAckData((void *)"\"c_a\":");
		Copy(num+7,Buf);
		if(num!=0)
		{
		memset(Set_Type.current_alarm,0,2);
		Set_Type.current_alarm[0]=atoi((void *)Buf)/256;
		Set_Type.current_alarm[1]=atoi((void *)Buf)%256;
		Current_Alarm=atoi((void *)Buf);
		}
		
		memset(Buf,0,50);
		num=MQTTAckData((void *)"\"interval\":");
		Copy(num+12,Buf);
		if(num!=0)
		{
		memset(Set_Type.interval,0,4);
		Set_Type.interval[0]=atoi((void *)Buf)/256/256/256;
		Set_Type.interval[1]=atoi((void *)Buf)/256/256;
		Set_Type.interval[2]=atoi((void *)Buf)/256;
		Set_Type.interval[3]=atoi((void *)Buf)%256;
		Time_Interval=atoi((void *)Buf);
		}
		
		memset(Buf,0,50);
		num=MQTTAckData((void *)"\"ip\":");
		Copy(num+6,Buf);
		if(num!=0)
		{
		run=REBOOT;
		memset(Set_Type.ip,0,30);
		memcpy(Set_Type.ip,(void *)Buf,strlen((void *)Buf));
		}
		
		memset(Buf,0,50);
		num=MQTTAckData((void *)"\"port\":");
		Copy(num+8,Buf);
		if(num!=0)
		{
		memset(Set_Type.port,0,10);
		memcpy(Set_Type.port,(void *)Buf,strlen((void *)Buf));
		}
	}
	memset(Write_Buf,0,120);
	memcpy(Write_Buf,Set_Type.ver,10);
	memcpy(Write_Buf+10,Set_Type.interval,4); 
	memcpy(Write_Buf+14,Set_Type.flilesize,4);
	memcpy(Write_Buf+18,Set_Type.port,10);
	memcpy(Write_Buf+28,Set_Type.ip,30);
	memcpy(Write_Buf+58,Set_Type.url,50);
	memcpy(Write_Buf+108,Set_Type.flag,1);
	memcpy(Write_Buf+109,Set_Type.votage_set,2);
	memcpy(Write_Buf+111,Set_Type.current_set,2);
	memcpy(Write_Buf+113,Set_Type.votage_alarm,2);
	memcpy(Write_Buf+115,Set_Type.current_alarm,2);
	W25QXX_Write(Write_Buf,0,120); 
	Read_Set();
}
void MQTT_Rec(u8 mode)
{
	if(mode==0)
	REC_Analysis();
	else if(mode==1)
	MQTT_Analysis();
}

void MQTT_Process(void)
{
	GPS_Analysis();                                    //获取GPS信号
//	RS485_Analysis();                                  ///水位仪RS485数据  9600
//	RS485_Analysis_Ammeter();                            //电表数据分析  2400 偶校验
//  RS485_Analysis_Watermeter();
  //RS485_Alarm_Analysis();
	SI522_Analysis();
	MCB_Analysis();
	if (g_tTimeSign.bTic1msSign)                       /* 1ms */
	{
			g_tTimeSign.bTic1msSign = FALSE;
	}
	if (g_tTimeSign.bTic10msSign)                /* 10ms */
	{
			GET_Data();                              //电压  电流  功耗
			g_tTimeSign.bTic10msSign = FALSE;
	}
	if (g_tTimeSign.bTic20msSign)                /* 20ms */
	{
			g_tTimeSign.bTic20msSign = FALSE;
	}
	if (g_tTimeSign.bTic100msSign)               /* 100ms */
	{ 
			ML307R_Send_Command();
			Check_ML307R_AT_ReciveData();//检查AT接收到的数据
		 
	 
			g_tTimeSign.bTic100msSign = FALSE;
	}
	if (g_tTimeSign.bTic200msSign)                      /* 200ms */
	{	
		 g_tTimeSign.bTic200msSign = FALSE;					
	}
	if (g_tTimeSign.bTic500msSign)                     /* 500ms */
	{
		  Ping_Interval++;
		 if(Dead_Flag==1)
		 Dead_Time++;
		 
		 if(Dead_Time>200)
		 {
			 run=REBOOT;
		 }
		if(Ping_Interval==60)
		{
			Ping_Interval=0;
			if(Net_flag==1)
			{
			MQTT_PingREQ(0);
			if(Dead_Flag==0)
			Dead_Time=0;
			Dead_Flag=1;
			}
		}
		 g_tTimeSign.bTic500msSign = FALSE;	 
	}
	if (g_tTimeSign.bTic1secSign)                      /* 1sec */
	{ 
   //RS485_Send_Water_Level();	
   //RS485_Send_Ammeter();	
   //RS485_Send_Water_meter();		
	 START_Time++;
	 if((Voltage>Voltage_Set)&(Current>Current_Set))
	 WORK_Time++;
	 
	 Count_Interval++;
	 if(Count_Interval>=Time_Interval)
	 {
		 Count_Interval=0;
		 if(Net_flag==1)
		  Date_Pack(0);
	 }
	 
	 
	 
	 if((Voltage>Voltage_Alarm)||(Current>Current_Alarm))
	 {
		 RS485_Send_Alarm_Start();
	 }
	 else
	 {
		 RS485_Send_Alarm_End();
	 }
	
	 
	 g_tTimeSign.bTic1secSign = FALSE;
	}		
  if(Net_flag==1)
	{
		ML307R_ReceiveData();
		if(ML307R_UartDataFinishFlag==1)
			{
				if((ML307R_RxBuf[0]==0xD0)&(ML307R_RxBuf[1]==0X00))
			{
				Dead_Flag=0;
			}
			
			if(strstr((void *)&ML307R_RxBuf[10],(void *)"sub")!= NULL)
			{
				MQTT_Rec(0);
			}
       ML307R_RxCount=0;
			 ML307R_UartDataFinishFlag=0;
			 memset(ML307R_RxBuf,0,1024);
			 memset(ML307R_TxBuf,0,1024);					
			}
	}
}
extern uint32_t  File_Size;
extern u32 OTA_Count;
extern u8 OTA_buf[1024*50];
extern HTTP_AT_Send_Flag   HTTP_Flag;
u8 OTA_time;

uint32_t HTTPAckData(void)
{
    uint16_t i=0;
    for (i=0;i<1000; i++)
    {
      if (memcmp("\r\n\r\n", &OTA_buf[i], 4) == 0) 
      {
        return i+4;
      }
    }     
    return 0;
}



void HTTP_Process(void)
{
	u32 NUM,a,b,i;
	if (g_tTimeSign.bTic100msSign)               /* 100ms */
	{ 
			HTTP_Send_Command();
			Check_HTTP_AT_ReciveData();//检查AT接收到的数据
			g_tTimeSign.bTic100msSign = FALSE;
	}
	if (g_tTimeSign.bTic1secSign)               /* 100ms */
	{ 
		  if(HTTP_Flag.f_AT_TCPSEND_Cm==1)
				OTA_time++;
			g_tTimeSign.bTic1secSign = FALSE;
	}
	 if(OTA_time>3)
	 {
		   if((OTA_buf[0]==0x00)&&(OTA_buf[1]==0x00)&&(OTA_buf[2]==0x00)&&(OTA_buf[3]==0x00)&&(OTA_buf[4]==0x00))
			{
				run=REBOOT;
			}
			else
			{
		  NUM=HTTPAckData();
		  a=File_Size/32768;
			b=File_Size%32768;
			for(i=0;i<a;i++)
			{
      W25QXX_Write(&OTA_buf[NUM+i*32768],4096+i*32768,32768);
			}
			W25QXX_Write(&OTA_buf[NUM+i*32768],4096+i*32768,b);
			
			Set_Type.flag[0]=0x31;	
			W25QXX_Write(Set_Type.flag,108,1);  
			run=REBOOT;
		  }
	 }
}



void WIFI_HTTP_Process(void)
{
	u32 NUM,a,b,i;
	if (g_tTimeSign.bTic100msSign)               /* 100ms */
	{ 
			WIFI_HTTP_Send_Command();
			Check_WIFI_HTTP_AT_ReciveData();//检查AT接收到的数据
			g_tTimeSign.bTic100msSign = FALSE;
	}
	if (g_tTimeSign.bTic1secSign)               /* 100ms */
	{ 
		  if(HTTP_Flag.f_AT_TCPSEND_Cm==1)
				OTA_time++;
			g_tTimeSign.bTic1secSign = FALSE;
	}
	 if(OTA_time>3)
	 {
		   if((OTA_buf[0]==0x00)&&(OTA_buf[1]==0x00)&&(OTA_buf[2]==0x00)&&(OTA_buf[3]==0x00)&&(OTA_buf[4]==0x00))
			{
				run=REBOOT;
			}
			else
			{
		  NUM=HTTPAckData();
		  a=File_Size/32768;
			b=File_Size%32768;
			for(i=0;i<a;i++)
			{
      W25QXX_Write(&OTA_buf[NUM+i*32768],4096+i*32768,32768);
			}
			W25QXX_Write(&OTA_buf[NUM+i*32768],4096+i*32768,b);
			
			Set_Type.flag[0]=0x31;	
			W25QXX_Write(Set_Type.flag,108,1);  
			run=REBOOT;
		  }
	 }
}



void WIFI_MQTT_Process(void)
{
	GPS_Analysis();                                    //获取GPS信号
//	RS485_Analysis();                                  ///水位仪RS485数据  9600
//	RS485_Analysis_Ammeter();                            //电表数据分析  2400 偶校验
//  RS485_Analysis_Watermeter();
  RS485_Alarm_Analysis();
	if (g_tTimeSign.bTic1msSign)                       /* 1ms */
	{
			g_tTimeSign.bTic1msSign = FALSE;
	}
	if (g_tTimeSign.bTic10msSign)                /* 10ms */
	{
			GET_Data();                              //电压  电流  功耗
			g_tTimeSign.bTic10msSign = FALSE;
	}
	if (g_tTimeSign.bTic20msSign)                /* 20ms */
	{
			g_tTimeSign.bTic20msSign = FALSE;
	}
	if (g_tTimeSign.bTic100msSign)               /* 100ms */
	{ 
			WIFI_Send_Command();
			Check_WIFI_AT_ReciveData();//检查AT接收到的数据
			g_tTimeSign.bTic100msSign = FALSE;
	}
	if (g_tTimeSign.bTic200msSign)                      /* 200ms */
	{	
		 g_tTimeSign.bTic200msSign = FALSE;					
	}
	if (g_tTimeSign.bTic500msSign)                     /* 500ms */
	{
		  Ping_Interval++;
		 if(Dead_Flag==1)
		 Dead_Time++;
		 
		 if(Dead_Time>200)
		 {
			 run=REBOOT;
		 }
		if(Ping_Interval==60)
		{
			Ping_Interval=0;
			if(Net_flag==1)
			{
			MQTT_PingREQ(1);
			if(Dead_Flag==0)
			Dead_Time=0;
			Dead_Flag=1;
			}
		}
		 g_tTimeSign.bTic500msSign = FALSE;	 
	}
	if (g_tTimeSign.bTic1secSign)                      /* 1sec */
	{ 
   //RS485_Send_Water_Level();	
   //RS485_Send_Ammeter();	
   //RS485_Send_Water_meter();		
	 START_Time++;
	 if((Voltage>Voltage_Set)&(Current>Current_Set))
	 WORK_Time++;
	 
	 Count_Interval++;
	 if(Count_Interval>=Time_Interval)
	 {
		 Count_Interval=0;
		 if(Net_flag==1)
		  Date_Pack(1);
	 }
	 if((Voltage>Voltage_Alarm)||(Current>Current_Alarm))
	 {
		 RS485_Send_Alarm_Start();
	 }
	 else
	 {
		 RS485_Send_Alarm_End();
	 }
	 g_tTimeSign.bTic1secSign = FALSE;
	}		
  if(Net_flag==1)
	{
		WIFI_ReceiveData();
		if(WIFI_UartDataFinishFlag==1)
			{
				if((WIFI_RxBuf[0]==0xD0)&(WIFI_RxBuf[1]==0X00))
			{
				Dead_Flag=0;
			}
			
			if(strstr((void *)&WIFI_RxBuf[10],(void *)"sub")!= NULL)
			{
				MQTT_Rec(1);
			}
       WIFI_RxCount=0;
			 WIFI_UartDataFinishFlag=0;
			 memset(WIFI_RxBuf,0,1024);
			 memset(WIFI_TxBuf,0,1024);					
			}
	}
}







