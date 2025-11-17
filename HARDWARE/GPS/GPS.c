#include "GPS.h"
#include "delay.h"
#include "RS485.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>



//1.WGS-84：是国际标准，GPS坐标（Google Earth使用、或者GPS模块）；
//2.GCJ-02：中国坐标偏移标准，Google Map、高德、腾讯使用，又称为火星坐标；
//3.BD-09：百度坐标偏移标准，Baidu Map使用


/*
 pi: 圆周率。
 a: 卫星椭球坐标投影到平面地图坐标系的投影因子。
 ee: 椭球的偏心率。
 x_pi: 圆周率转换量。
*/
double pi = 3.14159265358979324;		
double a = 6378245.0;
double ee = 0.00669342162296594323;
double x_pi = 3.14159265358979324 * 3000.0 / 180.0;


_GPSData GPSData;
u8 GPS_Data[GPS_RX_BUF_NUM];
//初始化IO 串口3
//pclk1:PCLK1时钟频率(Mhz)
//bound:波特率 
void GPS_Init(u32 pclk1,u32 bound)
{  	 
	float temp;
	u16 mantissa;
	u16 fraction;	   
	temp=(float)(pclk1*1000000)/(bound*16);//得到USARTDIV@OVER8=0
	mantissa=temp;				 //得到整数部分
	fraction=(temp-mantissa)*16; //得到小数部分@OVER8=0 
  mantissa<<=4;
	mantissa+=fraction; 
	RCC->AHB1ENR|=1<<1;   	//使能PORTB口时钟  
	RCC->APB1ENR|=1<<18;  	//使能串口3时钟 
	GPIO_Set(GPIOB,PIN10|PIN11,GPIO_MODE_AF,GPIO_OTYPE_PP,GPIO_SPEED_50M,GPIO_PUPD_PU);//PB10,PB11,复用功能,上拉输出
 	GPIO_AF_Set(GPIOB,10,7);	//PB10,AF7
	GPIO_AF_Set(GPIOB,11,7);  //PB11,AF7  	   
	
	//波特率设置
 	USART3->BRR=mantissa; 	//波特率设置	 
	USART3->CR1&=~(1<<15); 	//设置OVER8=0 
	USART3->CR1|=1<<3;  	//串口发送使能 

	USART3->CR1|=1<<2;  	//串口接收使能
	USART3->CR1|=1<<5;    	//接收缓冲区非空中断使能	 
 // USART3->CR1|=1<<4;	  //开启串口总线空闲中断. 		
	MY_NVIC_Init(3,3,USART3_IRQn,2);//组2，最低优先级 

	USART3->CR1|=1<<13;  	//串口使能
	ClrStruct();
}



unsigned int DataIndex=0;
void USART3_IRQHandler(void)
{
	u8 Res;
	if(USART3->SR&(1<<5))	//接收到数据
	{	 
		Res=USART3->DR;  	
    if(Res=='$')                 //接收开头
		{
			DataIndex=0;
		}
	  GPS_Data[DataIndex++]=Res;   
		if(GPS_Data[0]=='$' && GPS_Data[4]=='M' && GPS_Data[5]=='C')  //判断是否是GNRMC
		{
			if(Res==0X0A)
			{
				memset(GPSData.GPS_Buff,0,GPS_Buff_Len);    //清空
				memcpy(GPSData.GPS_Buff,GPS_Data,DataIndex);
				GPSData.isGetData = TRUE;
				DataIndex = 0;
				memset(GPS_Data,0,255);
			}
		}
		if(DataIndex>=250)
		{
			DataIndex=0;
		}
	}
} 


void ClrStruct(void)
{
	GPSData.isGetData=FALSE;
	GPSData.isAnalyData=FALSE;
	GPSData.isUseFull=FALSE;
	memset(GPSData.UTCTime1,0,UTCTime_Buff_Len);
	memset(GPSData.UTCTime2,0,UTCTime_Buff_Len);
	memset(GPSData.GPS_Buff,0,GPS_Buff_Len);
	memset(GPSData.Latitude,0,Latitude_Len);
	memset(GPSData.N_S,0,N_S_Len);
	memset(GPSData.Longitude,0,Longitude_Len);
	memset(GPSData.E_W,0,E_W_Len);
	memset(GPSData.SPEED,0,V_Len);
}
//void UTC_to_ZoneTime( _UTCTimer* utc_time, int timezone, _UTCTimer* local_time)
//{
//	int year, month, day, hour;
//    int lastday;   
//    int lastlastday; 
// 
//    year    = utc_time->year; 
//    month   = utc_time->months;
//    day     = utc_time->day;
//    hour    = utc_time->hour + timezone; 
// 
//    if (1==month || 3==month || 5==month || 7==month || 8==month || 10==month || 12==month) {
//        lastday = 31;
//        lastlastday = 30;
//        if (3 == month) {
//            if ((0 == year%400) || ((0 == year%4) && (year%100 != 0))) { //if this is lunar year.
//                lastlastday = 29;
//            } else {
//                lastlastday = 28;
//            }
//        } else if ((1 == month) || (8 == month)) {
//            lastlastday = 31;
//        }
//    } else if (4==month || 6==month || 9==month || 11==month) {
//        lastday = 30;
//        lastlastday = 31;
//    } else {
//        lastlastday = 31;
//        if ((0 == year%400) || ((0 == year%4) && (year%100 != 0))) {
//            lastday = 29;
//        } else {
//            lastday = 28;
//        }
//    }
//    if (hour >= 24) {
//        hour -= 24;
//        day += 1; 
// 
//        if (day > lastday) {
//            day -= lastday;
//            month += 1;
// 
//            if (month > 12) {
//                month -= 12;
//                year += 1;
//            }
//        }
//    } else if (hour < 0) {
//        hour += 24;
//        day -= 1;
//        if (day < 1) {
//            day = lastlastday;
//            month -= 1;
//            if (month < 1) {
//                month = 12;
//                year -= 1;
//            }
//        }
//    }
//	local_time->year  = year;
//	local_time->months = month;
//	local_time->day  = day;
//	local_time->hour  = hour;
//	local_time->minute	 = utc_time->minute;
//	local_time->second	 = utc_time->second;
//}
//void Get_Time()
//{
//	int year = 0;
//		year = (GPSData.UTCTime2[4]-'0')*10+(GPSData.UTCTime2[5]-'0')+2000;
//		if((year >= 2024) && (year < 3000))
//		{
//			UTCTimer.year = year;
//			UTCTimer.months = (GPSData.UTCTime2[2]-'0')*10+(GPSData.UTCTime2[3]-'0');
//			UTCTimer.day = (GPSData.UTCTime2[0]-'0')*10+(GPSData.UTCTime2[1]-'0');
//			UTCTimer.hour = (GPSData.UTCTime1[0]-'0')*10+(GPSData.UTCTime1[1]-'0');
//			UTCTimer.minute = (GPSData.UTCTime1[2]-'0')*10+(GPSData.UTCTime1[3]-'0');
//			UTCTimer.second = (GPSData.UTCTime1[4]-'0')*10+(GPSData.UTCTime1[5]-'0');
//			UTC_to_ZoneTime(&UTCTimer,8,&UTCTimer);
//		}
//}
void GPS_Analysis(void)
{
	char *subString;
	char *subStringNext;
	char i = 0;
	if (GPSData.isGetData)
	{
		GPSData.isGetData = FALSE;
		//截取数据帧前六部分    							 |对地航速 对地航向  日期
		//$GNRMC,112536.000,A,2322.75023,N,11326.28605,E,|  0.00,   0.00,  100722,,,A*78
		//$GBRMC,000010.700,V,          , ,           , ,       ,       ,  050180,,,N,V*2F
		for (i = 0 ; i <= 9 ; i++)
		{
			if (i == 0)
			{
				if ((subString = strstr(GPSData.GPS_Buff, ",")) == NULL)//如果没有找到逗号
				{
					return;
				}
			}
			else
			{
				subString++;
				if ((subStringNext = strstr(subString, ",")) != NULL)
				{
					char usefulBuffer[2]; 
					switch(i)
					{
						case 1:
						{
							memcpy(GPSData.UTCTime1, subString, subStringNext - subString);
							break;	
						}
						case 2:
						{
							memcpy(usefulBuffer, subString, subStringNext - subString);//有效标志位
							if(usefulBuffer[0] == 'A')
								GPSData.isUseFull = TRUE;
							else if(usefulBuffer[0] == 'V')
								GPSData.isUseFull = FALSE;
							break;
						}	
						case 3:memcpy(GPSData.Latitude, subString, subStringNext - subString);break;	
						case 4:memcpy(GPSData.N_S, subString, subStringNext - subString);break;	
						case 5:memcpy(GPSData.Longitude, subString, subStringNext - subString);break;	
						case 6:memcpy(GPSData.E_W, subString, subStringNext - subString);break;	
						case 7:memcpy(GPSData.SPEED, subString,subStringNext - subString );break;	
						case 8:memcpy(GPSData.Course, subString,subStringNext - subString);break;
					//	case 9:{memcpy(GPSData.UTCTime2, subString,subStringNext - subString);Get_Time();break;}
						default:break;
					}
					subString = subStringNext;					
				}
			}
		}
//		RS485_Send_array((void *)GPSData.GPS_Buff,strlen(GPSData.GPS_Buff));
//		RS485_Send_array((void *)GPSData.UTCTime1,strlen(GPSData.UTCTime1));
//		RS485_Send_array((void *)"\r\n",2);
//		RS485_Send_array((void *)GPSData.UTCTime2,strlen(GPSData.UTCTime2));
//		RS485_Send_array((void *)"\r\n",2);
//		RS485_Send_array((void *)GPSData.Latitude,Latitude_Len);
//		RS485_Send_array((void *)"\r\n",2);
//		RS485_Send_array((void *)GPSData.N_S,N_S_Len);
//		RS485_Send_array((void *)"\r\n",2);
//		RS485_Send_array((void *)GPSData.Longitude,Longitude_Len);
//		RS485_Send_array((void *)"\r\n",2);
//		RS485_Send_array((void *)GPSData.E_W,E_W_Len);
//		RS485_Send_array((void *)"\r\n",2);
//		u8 txbuf[100];
//	  sprintf((char*)txbuf,(void *)"%02d-%02d-%02d %02d:%02d:%02d",UTCTimer.year,UTCTimer.months,UTCTimer.day,UTCTimer.hour,UTCTimer.minute,UTCTimer.second); 
//		RS485_Send_array((void *)txbuf,strlen((void *)txbuf));
//		RS485_Send_array((void *)"\r\n",2);
	
	  GPSData.wcgs84_lat=atof(GPSData.Latitude);
		GPSData.wcgs84_lon=atof(GPSData.Longitude);
	 
    GPSData.wcgs84_lon = (int)(GPSData.wcgs84_lon/100) + (GPSData.wcgs84_lon/100.0 - (int)(GPSData.wcgs84_lon/100)) *100.0 / 60.0;
    GPSData.wcgs84_lat = (int)(GPSData.wcgs84_lat/100) + (GPSData.wcgs84_lat/100.0 - (int)(GPSData.wcgs84_lat/100)) *100.0 / 60.0;
    

    memset((void *)GPSData.Latitude,0,Latitude_Len);
		memset((void *)GPSData.Longitude,0,Longitude_Len);
		if(MAP_Format==1)
		{
			sprintf(GPSData.Latitude,"%.6lf", GPSData.wcgs84_lat);
			sprintf(GPSData.Longitude,"%.6lf", GPSData.wcgs84_lon);
		}
	  else if(MAP_Format==2)
		{
		if(wgs2bd(GPSData.wcgs84_lat,GPSData.wcgs84_lon,&GPSData.bd_lat,&GPSData.bd_lon)==0)
			sprintf(GPSData.Latitude,"%.6lf", GPSData.bd_lat);
			sprintf(GPSData.Longitude,"%.6lf", GPSData.bd_lon);
		}
		else if(MAP_Format==3)
		{
		if(wgs2gcj(GPSData.wcgs84_lat,GPSData.wcgs84_lon,&GPSData.gcj_lat,&GPSData.gcj_lon)==0)
			sprintf(GPSData.Latitude,"%.6lf", GPSData.gcj_lat);
			sprintf(GPSData.Longitude,"%.6lf", GPSData.gcj_lon);
		}
//		RS485_Send_array((void *)GPSData.Longitude,Longitude_Len);
//		RS485_Send_array((void *)"\r\n",2); 
//		RS485_Send_array((void *)GPSData.Latitude,Latitude_Len);
//		RS485_Send_array((void *)"\r\n",2);
		GPSData.isAnalyData = TRUE;	
	}
}





u8 outOfChina(double lat, double lon){
		if ((lon < 72.004 || lon > 137.8347)&&(lat < 0.8293 || lat > 55.8271))
				return 1;
		return 0;
 }

// WGS84=>BD09 地球坐标系=>百度坐标系 
int wgs2bd(double lat, double lon, double* pLat, double* pLon) {
   double lat_ = 0.0, lon_ = 0.0;
   wgs2gcj(lat, lon, &lat_, &lon_);
   gcj2bd(lat_, lon_,  pLat, pLon);
   return 0;
}
 
// GCJ02=>BD09 火星坐标系=>百度坐标系  
int gcj2bd(double lat, double lon, double* pLat, double* pLon) {
   double x = lon, y = lat;
   double z = sqrt(x * x + y * y) + 0.00002 * sin(y * x_pi);
   double theta = atan2(y, x) + 0.000003 * cos(x * x_pi);
   *pLon = z * cos(theta) + 0.0065;
   *pLat = z * sin(theta) + 0.006;
   return 0;
}
 
// BD09=>GCJ02 百度坐标系=>火星坐标系 
int bd2gcj(double lat, double lon, double* pLat, double* pLon) {
   double x = lon - 0.0065, y = lat - 0.006;
   double z = sqrt(x * x + y * y) - 0.00002 * sin(y * x_pi);
   double theta = atan2(y, x) - 0.000003 * cos(x * x_pi);
   *pLon = z * cos(theta);
   *pLat = z * sin(theta);
   return 0;
}
 
// WGS84=>GCJ02 地球坐标系=>火星坐标系
int wgs2gcj(double lat, double lon, double* pLat, double* pLon) {
    if (outOfChina(lat,lon))
    {
        *pLat = lat;
        *pLon = lon;
        return 0;
    }
   double dLat = transformLat(lon - 105.0, lat - 35.0);
   double dLon = transformLon(lon - 105.0, lat - 35.0);
   double radLat = lat / 180.0 * pi;
   double magic = sin(radLat);
   magic = 1 - ee * magic * magic;
   double sqrtMagic = sqrt(magic);
   dLat = (dLat * 180.0) / ((a * (1 - ee)) / (magic * sqrtMagic) * pi);
   dLon = (dLon * 180.0) / (a / sqrtMagic * cos(radLat) * pi);
   *pLat = lat + dLat;
   *pLon = lon + dLon;
   return 0;
}
 
// GCJ02=>WGS84 火星坐标系=>地球坐标系(粗略)
int gcj2wgs(double lat, double lon, double* pLat, double* pLon) {
    if (outOfChina(lat,lon))
    {
        *pLat = lat;
        *pLon = lon;
        return 0;
    }
	double *offset;
	offset = OffSet(lat,lon);
	*pLat = lat - offset[0];
 	*pLon = lon - offset[1];
	return 0;
}
 
// GCJ02=>WGS84 火星坐标系=>地球坐标系（精确）
int gcj2wgs_Exactly(double gcjlat, double gcjlon, double* wgs_Lat, double* wgs_lon) {
    if (outOfChina(gcjlat,gcjlon))
    {
        *wgs_Lat = gcjlat;
        *wgs_lon = gcjlon;
        return 0;
    }
	double initDelta = 0.01;
	double threshold = 0.000000001;
	double dLat = initDelta, dLon = initDelta;
	double mLat = gcjlat - dLat, mLon = gcjlon - dLon;
	double pLat = gcjlat + dLat, pLon = gcjlon + dLon;
	double wgsLat = 0.0, wgslon = 0.0, i = 0.0 ,newgcjlat = 0.0,newgcjlon = 0.0;
 
	while (1) {
		wgsLat = (mLat + pLat) / 2;
		wgslon = (mLon + pLon) / 2;		
		wgs2gcj(wgsLat,wgslon,&newgcjlat,&newgcjlon);
		dLon = newgcjlon - gcjlon;
		dLat = newgcjlat - gcjlat;
		if ((fabs(dLat) < threshold) && (fabs(dLon) < threshold))
			break;
							
		if (dLat > 0)
			pLat = wgsLat;
		else
			mLat = wgsLat;
		if (dLon > 0)
			pLon = wgslon;
		else
			mLon = wgslon;
 
		if (++i > 10000)
			break;
	}
	*wgs_Lat = wgsLat;
	*wgs_lon = wgslon;
	return 0;
}
 
// BD09=>WGS84 百度坐标系=>地球坐标系(粗略)
int bd2wgs(double lat, double lon, double* pLat, double* pLon) {
	double lat_ = 0.0, lon_ = 0.0;
 	bd2gcj(lat, lon, &lat_, &lon_);
 	gcj2wgs(lat_, lon_,  pLat, pLon);
 	return 0;
}
 
// BD09=>WGS84 百度坐标系=>地球坐标系(精确)
int bd2wgs_Exactly(double lat, double lon, double* pLat, double* pLon) {
	double lat_ = 0.0, lon_ = 0.0;
 	bd2gcj(lat, lon, &lat_, &lon_);
 	gcj2wgs_Exactly(lat_, lon_,  pLat, pLon);
 	return 0;
}
 
 
 
// 偏移量
double *OffSet(double lat, double lon) {
        double Latlon[2] = {0.0,0.0};
				double *p;
        double dLat = transformLat(lon - 105.0, lat - 35.0);
        double dLon = transformLon(lon - 105.0, lat - 35.0);
        double radLat = lat / 180.0 * pi;
        double magic = sin(radLat);
        magic = 1 - ee * magic * magic;
        double sqrtMagic = sqrt(magic);
        dLat =
				(dLat * 180.0) / ((a * (1 - ee)) / (magic * sqrtMagic) * pi);
        dLon = (dLon * 180.0) / (a / sqrtMagic * cos(radLat) * pi);
				Latlon[0] = dLat;
				Latlon[1] = dLon;
				p=Latlon;
		return p;
	}
 
// 纬度偏移量
double transformLat(double x, double y) {
       double ret = 0.0;
       ret = -100.0 + 2.0 * x + 3.0 * y + 0.2 * y * y + 0.1 * x * y + 0.2 * sqrt(fabs(x));
       ret += (20.0 * sin(6.0 * x * pi) + 20.0 * sin(2.0 * x * pi)) * 2.0 / 3.0;
       ret += (20.0 * sin(y * pi) + 40.0 * sin(y / 3.0 * pi)) * 2.0 / 3.0;
       ret += (160.0 * sin(y / 12.0 * pi) + 320 * sin(y * pi  / 30.0)) * 2.0 / 3.0;
       return ret;
}
 
// 经度偏移量
double transformLon(double x, double y) {
       double ret = 0.0;
       ret = 300.0 + x + 2.0 * y + 0.1 * x * x + 0.1 * x * y + 0.1 * sqrt(fabs(x));
       ret += (20.0 * sin(6.0 * x * pi) + 20.0 * sin(2.0 * x * pi)) * 2.0 / 3.0;
       ret += (20.0 * sin(x * pi) + 40.0 * sin(x / 3.0 * pi)) * 2.0 / 3.0;
       ret += (150.0 * sin(x / 12.0 * pi) + 300.0 * sin(x / 30.0 * pi)) * 2.0 / 3.0;
       return ret;
}


