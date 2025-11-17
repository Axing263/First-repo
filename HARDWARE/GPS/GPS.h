#ifndef __GPS__H
#define __GPS__H
#include "sys.h"

//确定上传的坐标系
//1.WGS-84：是国际标准，GPS坐标（Google Earth使用、或者GPS模块）；
//2.BD-09：百度坐标偏移标准，Baidu Map使用
//3.GCJ-02：中国坐标偏移标准，Google Map、高德、腾讯使用，又称为火星坐标；

#define MAP_Format 1

#define GPS_RX_BUF_NUM 255
#define FALSE 0
#define TRUE 1

/**************解析内容************************/
//消息ID          $GNRMC
//定位点UTC时间   051008.000
//定位状态        A:定位 V:导航(A表示数据有限 V表示数据无效)
//纬度            3138.47508
//纬度方向        N
//经度            12022.05586
//经度方向        E
/***********************************************/
#define GPS_Buff_Len 80  //GPS需要解析数组的长度
#define UTCTime_Buff_Len 11 
#define Latitude_Len 11
#define N_S_Len 1
#define Longitude_Len 12
#define E_W_Len 1
#define V_Len 6

typedef struct GPSData
{
	char GPS_Buff[GPS_Buff_Len];
	char isGetData;    //是否获取到GPS数据
	char isAnalyData;  //是否完成解析
	char UTCTime1[UTCTime_Buff_Len];//UTC时间时分秒
	char UTCTime2[UTCTime_Buff_Len];//UTC时间日月年
	char Latitude[Latitude_Len];   	//纬度
	char N_S[N_S_Len];             	//N/S
	char Longitude[Longitude_Len]; 	//经度
	char E_W[E_W_Len];             	//E/W
	char SPEED[V_Len];            	//E/W
	char isUseFull;               	//定位信息是否有效 A/V
	char Course[5];               	//航向
	double wcgs84_lat;
	double wcgs84_lon;
	double gcj_lat;
	double gcj_lon;
	double bd_lat;
	double bd_lon;
}_GPSData;
extern _GPSData GPSdata; 



 
void GPS_Init(u32 pclk1,u32 bound);//GPS串口初始化
void ClrStruct(void);    //清除缓存
void GPS_Analysis(void); //GPS解析 
int wgs2bd(double lat, double lon, double* pLat, double* pLon); // WGS84=>BD09 地球坐标系=>百度坐标系 
int gcj2bd(double lat, double lon, double* pLat, double* pLon);	// GCJ02=>BD09 火星坐标系=>百度坐标系 
int bd2gcj(double lat, double lon, double* pLat, double* pLon);	// BD09=>GCJ02 百度坐标系=>火星坐标系 
int wgs2gcj(double lat, double lon, double* pLat, double* pLon);// WGS84=>GCJ02 地球坐标系=>火星坐标系
int gcj2wgs(double lat, double lon, double* pLat, double* pLon);// GCJ02=>WGS84 火星坐标系=>地球坐标系(粗略)
int bd2wgs(double lat, double lon, double* pLat, double* pLon);	// BD09=>WGS84 百度坐标系=>地球坐标系(粗略)
int gcj2wgs_Exactly(double lat, double lon, double* wgs_Lat, double* wgs_lon);// GCJ02=>WGS84 火星坐标系=>地球坐标系（精确）
int bd2wgs_Exactly(double lat, double lon, double* pLat, double* pLon);// BD09=>WGS84 百度坐标系=>地球坐标系(精确)
double *OffSet(double lat, double lon);	// 偏移量
double transformLat(double x, double y);// 纬度偏移量
double transformLon(double x, double y);// 经度偏移量


#endif
