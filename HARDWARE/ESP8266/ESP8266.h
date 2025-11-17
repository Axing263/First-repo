#ifndef __ESP8266__H
#define __ESP8266__H
#include "sys.h"



#define WIFI_RST PAout(12)    
#define WIFI_EN  PAout(11)
void ESP8266_Init(u32 pclk1,u32 bound);
void WIFI_Send_array(uint8_t *arr, uint16_t len);
void WIFI_ReceiveData(void);
void WIFI_START(void);   //是否有WIFI
void WIFI_Send_Command(void);
void Check_WIFI_AT_ReciveData(void);

void WIFI_HTTP_Send_Command(void);
void Check_WIFI_HTTP_AT_ReciveData(void);//检查AT接收到的数据



typedef struct WIFI_Send_Flag_Def
{
u8 f_AT_Cm ;            //查询是否回显
u8 f_ATE_Cm;            //关闭回显
u8 f_BLENAME_Cm;        //设置BLE名
u8 f_BLEAUTH_Cm; 	      //设置蓝牙连接密码
u8 f_BLEMODE_Cm; 		    //设置蓝牙模式
u8 f_BLEADVEN_Cm; 		  //开启蓝牙
u8 f_WMODE_Cm; 		      //设置WIFI模式	
u8 f_WAUTOCONN_Cm; 		  //设置SSID PWD
u8 f_SSNTP_Cm; 		      //开启时间
u8 f_SNTP_Cm; 		      //设置时间
u8 f_SOCKET_Cm; 		    //建立TCPIP连接
u8 f_SOCKETTT_Cm; 		  //透传模式
u8 f_MQTT_Cm; 		      //MQTT连接
}WIFI_AT_Send_Flag;    //AT发送标志位



typedef struct WIFI_OK_Flag
{
u8 f_AT_OK;        //查询是否回显
u8 f_ATE_OK;       //设置回显
u8 f_BLENAME_OK;   //设置BLE名
u8 f_BLEAUTH_OK;   //设置蓝牙连接密码
u8 f_BLEMODE_OK;   //设置蓝牙模式
u8 f_BLEADVEN_OK;  //开启蓝牙
u8 f_WMODE_OK;     //设置WIFI模式
u8 f_WAUTOCONN_OK; //设置SSID PWD
u8 f_SSNTP_OK;      //开启时间
u8 f_SNTP_OK;      //设置时间
u8 f_SOCKET_OK;    //建立TCPIP连接
u8 f_SOCKETTT_OK;  //透传模式
u8 f_MQTT_OK;      //MQTT连接
}WIFI_AT_Send_Flag_OK ;//AT发送完成标志位


typedef struct WIFI_Send_Count
{
u16  AT_Ct;//	查询是否回显
u16  ATE_Ct;//设置回显
u16  BLENAME_Ct;//设置BLE名
u16  BLEAUTH_Ct;	//设置蓝牙连接密码
u16  BLEMODE_Ct;	//设置蓝牙模式
u16  BLEADVEN_Ct;	//开启蓝牙
u16  WMODE_Ct;	//设置WIFI模式
u16  WAUTOCONN_Ct;//设置SSID PWD
u16  SSNTP_Ct;//开启时间
u16  SNTP_Ct;//设置时间
u16  SOCKET_Ct;//建立TCPIP连接
u16  SOCKETTT_Ct;//透传模式
u16  MQTT_Ct;//MQTT连接
}WIFI_AT_Send_Count ;


typedef struct WIFI_Error
{
u16  f_AT;//	查询是否回显
u16  f_ATE ;//设置回显
u16  f_BLENAME;//设置BLE名
u16  f_BLEAUTH;	//设置蓝牙连接密码
u16  f_BLEMODE;	//设置蓝牙模式
u16  f_BLEADVEN;//开启蓝牙
u16  f_WMODE;//设置WIFI模式
u16  f_WAUTOCONN; //设置SSID PWD
u16  f_SSNTP; //开启时间
u16  f_SNTP; //设置时间
u16  f_SOCKET;   //建立TCPIP连接
u16  f_SOCKETTT; //透传模式
u16  f_MQTT;     //MQTT连接
}WIFI_AT_Error; 





typedef struct WIFI_HTTP_Flag_Def
{
u8 f_AT_Cm ;            //查询是否回显
u8 f_ATE_Cm;            //关闭回显
u8 f_WMODE_Cm; 		      //设置WIFI模式	
u8 f_WAUTOCONN_Cm; 		  //设置SSID PWD
u8 f_SOCKET_Cm; 		    //建立TCPIP连接
u8 f_SOCKETTT_Cm; 		  //透传模式
u8 f_HTTP_Cm; 		      //HTTP连接
}WIFI_AT_HTTP_Flag;    //AT发送标志位



typedef struct WIFI_HTTP_OK_Flag
{
u8 f_AT_OK;        //查询是否回显
u8 f_ATE_OK;       //设置回显
u8 f_WMODE_OK;     //设置WIFI模式
u8 f_WAUTOCONN_OK; //设置SSID PWD
u8 f_SOCKET_OK;    //建立TCPIP连接
u8 f_SOCKETTT_OK;  //透传模式
u8 f_HTTP_OK;      //HTTP连接
}WIFI_AT_HTTP_Flag_OK ;//AT发送完成标志位


typedef struct WIFI_HTTP_Count
{
u16  AT_Ct;//	查询是否回显
u16  ATE_Ct;//设置回显
u16  WMODE_Ct;	//设置WIFI模式
u16  WAUTOCONN_Ct;//设置SSID PWD
u16  SOCKET_Ct;//建立TCPIP连接
u16  SOCKETTT_Ct;//透传模式
u16  HTTP_Ct;//HTTP连接
}WIFI_AT_HTTP_Count ;


typedef struct WIFI_HTTP_Error
{
u16  f_AT;//	查询是否回显
u16  f_ATE ;//设置回显
u16  f_WMODE;//设置WIFI模式
u16  f_WAUTOCONN; //设置SSID PWD
u16  f_SOCKET;   //建立TCPIP连接
u16  f_SOCKETTT; //透传模式
u16  f_HTTP;     //HTTP连接
}WIFI_HTTP_Error; 
#endif












