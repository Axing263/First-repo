#ifndef __ML307R_H
#define __ML307R_H
#include "sys.h"
#include "W25Q64.h"
#define ML307R_RST PAout(8)


typedef struct AT_Send_Flag_Def
{
u8 f_AT_Cm ;            //查询是否回显
u8 f_ATE_Cm;            //关闭回显
u8 f_AT_CGSN_Cm  ;      //获取IMEI号
u8 f_AT_CPIN_Cm  ;      //检查SIM卡状态
u8 f_AT_CIMI_Cm  ;      //获取SIM卡的IMSI
u8 f_AT_CCID_Cm  ;      //获取SIM卡ICCD号
u8 f_AT_CSQ_Cm  ;       //获取信号强度
u8 f_AT_CTZU_Cm  ;      //设置时间更新来源
u8 f_AT_CCLK_Cm  ;      //查询当前时间
u8 f_AT_MNTP_Cm  ;      //查询NTP时间
u8 f_AT_CREG_Cm  ;      //查询网络注册状态
u8 f_AT_CGATT_Cm  ;     //查询附网状态	
u8 f_AT_CLOSE_Cm  ;     //关闭socket
u8 f_AT_MIPTKA_Cm ;     //设置keepalive
u8 f_AT_OPEN_Cm  ;      //关闭设置IP+端口	
u8 f_AT_STCP_Cm  ;      //设置TCP为透传模式	
u8 f_AT_SCNT_Cm  ;      //发送TCPIP数据数量
u8 f_AT_SEND_Cm  ;      //发送TCPIP数据
u8 f_AT_MQTT_Con_Cm  ;      //MQTT链接
u8 f_AT_MYSYSINFO_Cm  ; //查询网络制式
u8 f_AT_CGDCONT_Cm  ;   //设置电信APN
u8 f_AT_XGAUTH_Cm  ;    //设置电信身份认证参数
u8 f_AT_XIIC_Cm  ;      //建立PPP连接获取IP地址
u8 f_AT_CXIIC_Cm  ;     //查询PPP连接是否建立
u8 f_AT_TCPSETUP_Cm  ;  //建立 TCP 连接
u8 f_AT_IPSTATUS_Cm  ;  //查询TCP链路状态
u8 f_AT_TCPSEND_Cm ;    //TCP发送数据
u8 f_AT_TCPREAD_Cm ;    //读取TCP接收的数据，默认不需要查询只需要定时查询串口是否出现“+TCPRECV:”
}ML307R_AT_Send_Flag;    //AT发送标志位

typedef struct HTTP_Send_Flag_Def
{
u8 f_AT_Cm ;            //查询是否回显
u8 f_ATE_Cm;            //关闭回显
u8 f_AT_CGSN_Cm  ;      //获取IMEI号
u8 f_AT_CPIN_Cm  ;      //检查SIM卡状态
u8 f_AT_CIMI_Cm  ;      //获取SIM卡的IMSI
u8 f_AT_CCID_Cm  ;      //获取SIM卡ICCD号
u8 f_AT_CSQ_Cm  ;       //获取信号强度
u8 f_AT_CTZU_Cm  ;      //设置时间更新来源
u8 f_AT_CCLK_Cm  ;      //查询当前时间
u8 f_AT_MNTP_Cm  ;      //查询NTP时间
u8 f_AT_CREG_Cm  ;      //查询网络注册状态
u8 f_AT_CGATT_Cm  ;     //查询附网状态	
u8 f_AT_CLOSE_Cm  ;     //关闭socket
u8 f_AT_MIPTKA_Cm ;     //设置keepalive
u8 f_AT_OPEN_Cm  ;      //关闭设置IP+端口	
u8 f_AT_STCP_Cm  ;      //设置TCP为透传模式	
u8 f_AT_SCNT_Cm  ;      //发送TCPIP数据数量
u8 f_AT_SEND_Cm  ;      //发送TCPIP数据
u8 f_AT_MQTT_Con_Cm  ;      //MQTT链接
u8 f_AT_MYSYSINFO_Cm  ; //查询网络制式
u8 f_AT_CGDCONT_Cm  ;   //设置电信APN
u8 f_AT_XGAUTH_Cm  ;    //设置电信身份认证参数
u8 f_AT_XIIC_Cm  ;      //建立PPP连接获取IP地址
u8 f_AT_CXIIC_Cm  ;     //查询PPP连接是否建立
u8 f_AT_TCPSETUP_Cm  ;  //建立 TCP 连接
u8 f_AT_IPSTATUS_Cm  ;  //查询TCP链路状态
u8 f_AT_TCPSEND_Cm ;    //TCP发送数据
u8 f_AT_TCPREAD_Cm ;    //读取TCP接收的数据，默认不需要查询只需要定时查询串口是否出现“+TCPRECV:”
}HTTP_AT_Send_Flag;    //AT发送标志位



typedef struct AT_OK_Flag
{
u8 f_AT_OK;        //查询是否回显
u8 f_ATE_OK;       //设置回显
u8 f_AT_CGSN_OK;   //获取IMEI号
u8 f_AT_CPIN_OK;   //检查SIM卡状态
u8 f_AT_CIMI_OK;   //获取SIM卡的IMSI
u8 f_AT_CCID_OK;   //获取SIM卡ICCD号
u8 f_AT_CSQ_OK;    //获取信号强度
u8 f_AT_CTZU_OK;   //设置时间更新来源
u8 f_AT_CCLK_OK;   //查询当前时间
u8 f_AT_MNTP_OK;   //查询NTP时间
u8 f_AT_CREG_OK;   //查询网络注册状态
u8 f_AT_CGATT_OK;  //查询附网状态	
u8 f_AT_CLOSE_OK;  //关闭SOCKET
u8 f_AT_MIPTKA_OK;  //设置keepalive
u8 f_AT_OPEN_OK;   //设置IP+端口
u8 f_AT_STCP_OK;	 //设置TCP为透传模式	
u8 f_AT_SCNT_OK;   //发送TCPIP数据数量
u8 f_AT_SEND_OK;   //发送TCPIP数据
u8 f_AT_MQTT_Con_OK;   //MQTT连接
	
u8 f_AT_MYSYSINFO_OK;//查询网络制式
u8 f_AT_CGDCONT_OK;//设置电信APN
u8 f_AT_XGAUTH_OK;//设置电信身份认证参数
u8 f_AT_XIIC_OK; //建立PPP连接获取IP地址
u8 f_AT_CXIIC_OK;//查询PPP连接是否建立
u8 f_AT_TCPSETUP_OK;//建立 TCP 连接
u8 f_AT_IPSTATUS_OK;//查询TCP链路状态
u8 f_AT_TCPSEND_OK;//TCP发送数据
u8 f_AT_TCPREAD_OK;    //读取TCP接收的数据，默认不需要查询只需要定时查询串口是否出现“+TCPRECV:”
}ML307R_AT_Send_Flag_OK ;//AT发送完成标志位


typedef struct HTTP_OK_Flag
{
u8 f_AT_OK;        //查询是否回显
u8 f_ATE_OK;       //设置回显
u8 f_AT_CGSN_OK;   //获取IMEI号
u8 f_AT_CPIN_OK;   //检查SIM卡状态
u8 f_AT_CIMI_OK;   //获取SIM卡的IMSI
u8 f_AT_CCID_OK;   //获取SIM卡ICCD号
u8 f_AT_CSQ_OK;    //获取信号强度
u8 f_AT_CTZU_OK;   //设置时间更新来源
u8 f_AT_CCLK_OK;   //查询当前时间
u8 f_AT_MNTP_OK;   //查询NTP时间
u8 f_AT_CREG_OK;   //查询网络注册状态
u8 f_AT_CGATT_OK;  //查询附网状态	
u8 f_AT_CLOSE_OK;  //关闭SOCKET
u8 f_AT_MIPTKA_OK;  //设置keepalive
u8 f_AT_OPEN_OK;   //设置IP+端口
u8 f_AT_STCP_OK;	 //设置TCP为透传模式	
u8 f_AT_SCNT_OK;   //发送TCPIP数据数量
u8 f_AT_SEND_OK;   //发送TCPIP数据
u8 f_AT_MQTT_Con_OK;   //MQTT连接
	
u8 f_AT_MYSYSINFO_OK;//查询网络制式
u8 f_AT_CGDCONT_OK;//设置电信APN
u8 f_AT_XGAUTH_OK;//设置电信身份认证参数
u8 f_AT_XIIC_OK; //建立PPP连接获取IP地址
u8 f_AT_CXIIC_OK;//查询PPP连接是否建立
u8 f_AT_TCPSETUP_OK;//建立 TCP 连接
u8 f_AT_IPSTATUS_OK;//查询TCP链路状态
u8 f_AT_TCPSEND_OK;//TCP发送数据
u8 f_AT_TCPREAD_OK;    //读取TCP接收的数据，默认不需要查询只需要定时查询串口是否出现“+TCPRECV:”
}HTTP_AT_Send_Flag_OK ;//AT发送完成标志位



typedef struct AT_Send_Count
{
u16  AT_Ct;//	查询是否回显
u16  ATE_Ct;//设置回显
u16  AT_CGSN_Ct;//获取IMEI号
u16  AT_CPIN_Ct;//检查SIM卡状态
u16  AT_CIMI_Ct;//获取SIM卡的IMSI
u16  AT_CCID_Ct;//获取SIM卡ICCD号
u16  AT_CSQ_Ct;//获取信号强度
u16  AT_CTZU_Ct;//设置时间更新来源
u16  AT_CCLK_Ct;//查询当前时间
u16  AT_MNTP_Ct;//查询NTP时间
u16  AT_CREG_Ct;//查询网络注册状态
u16  AT_CGATT_Ct;//查询附网状态	
u16  AT_CLOSE_Ct;//关闭SOCKET
u16  AT_MIPTKA_Ct;//设置keepalive
u16  AT_OPEN_Ct; //设置IP+端口
u16  AT_STCP_Ct; //设置TCP为透传模式	
u16  AT_SCNT_Ct;	//发送TCPIP数据数量
u16  AT_SEND_Ct;	//发送TCPIP数据
u16  AT_MQTT_Con_Ct;	//MQTT连接
	
u16  AT_MYSYSINFO_Ct;//查询网络制式
u16  AT_CGDCONT_Ct;//设置电信APN
u16  AT_XGAUTH_Ct;//设置电信身份认证参数
u16  AT_XIIC_Ct; //建立PPP连接获取IP地址
u16  AT_CXIIC_Ct;//查询PPP连接是否建立
u16  AT_TCPSETUP_Ct;//建立 TCP 连接
u16  AT_IPSTATUS_Ct;//查询TCP链路状态
u16  AT_TCPSEND_Ct;//TCP发送数据
u16  AT_TCPREAD_Ct;//读取TCP接收的数据，默认不需要查询只需要定时查询串口是否出现“+TCPRECV:”
}ML307R_AT_Send_Count ;

typedef struct HTTP_Send_Count
{
u16  AT_Ct;//	查询是否回显
u16  ATE_Ct;//设置回显
u16  AT_CGSN_Ct;//获取IMEI号
u16  AT_CPIN_Ct;//检查SIM卡状态
u16  AT_CIMI_Ct;//获取SIM卡的IMSI
u16  AT_CCID_Ct;//获取SIM卡ICCD号
u16  AT_CSQ_Ct;//获取信号强度
u16  AT_CTZU_Ct;//设置时间更新来源
u16  AT_CCLK_Ct;//查询当前时间
u16  AT_MNTP_Ct;//查询NTP时间
u16  AT_CREG_Ct;//查询网络注册状态
u16  AT_CGATT_Ct;//查询附网状态	
u16  AT_CLOSE_Ct;//关闭SOCKET
u16  AT_MIPTKA_Ct;//设置keepalive
u16  AT_OPEN_Ct; //设置IP+端口
u16  AT_STCP_Ct; //设置TCP为透传模式	
u16  AT_SCNT_Ct;	//发送TCPIP数据数量
u16  AT_SEND_Ct;	//发送TCPIP数据
u16  AT_MQTT_Con_Ct;	//MQTT连接
	
u16  AT_MYSYSINFO_Ct;//查询网络制式
u16  AT_CGDCONT_Ct;//设置电信APN
u16  AT_XGAUTH_Ct;//设置电信身份认证参数
u16  AT_XIIC_Ct; //建立PPP连接获取IP地址
u16  AT_CXIIC_Ct;//查询PPP连接是否建立
u16  AT_TCPSETUP_Ct;//建立 TCP 连接
u16  AT_IPSTATUS_Ct;//查询TCP链路状态
u16  AT_TCPSEND_Ct;//TCP发送数据
u16  AT_TCPREAD_Ct;//读取TCP接收的数据，默认不需要查询只需要定时查询串口是否出现“+TCPRECV:”
}HTTP_AT_Send_Count ;


typedef struct ML307R_Error
{
u16  f_AT;//	查询是否回显
u16  f_ATE ;
u16  f_AT_CGSN ;//获取IMEI号
u16  f_AT_CPIN ;//检查SIM卡状态
u16  f_AT_CIMI ;//获取SIM卡的IMSI
u16  f_AT_CCID ;//获取SIM卡ICCD号
u16  f_AT_CSQ  ;//获取信号强度
u16  f_AT_CTZU ;//设置时间更新来源
u16  f_AT_CCLK ;//查询当前时间	
u16  f_AT_MNTP ;//NTP更新时间
u16  f_AT_CREG ;//查询网络注册状态
u16  f_AT_CGATT ;//查询附网状态	
u16  f_AT_CLOSE ;//关闭SOCKET
u16  f_AT_MIPTKA ;//设置keepalive
u16  f_AT_OPEN ;//设置IP+端口
u16  f_AT_STCP ;//设置TCP为透传模式
u16  f_AT_SCNT ;//发送TCPIP数据数量
u16  f_AT_SEND ;//发送TCPIP数据
u16  f_AT_MQTT_Con ;//MQTT链接
	
u16  f_AT_MYSYSINFO ;//查询网络制式
u16  f_AT_CGDCONT ;//设置电信APN
u16  f_AT_XGAUTH ;//设置电信身份认证参数
u16  f_AT_XIIC ; //建立PPP连接获取IP地址
u16  f_AT_CXIIC ;//查询PPP连接是否建立
u16  f_AT_TCPSETUP ;//建立 TCP 连接
u16  f_AT_IPSTATUS ;//查询TCP链路状态
u16  f_AT_TCPSEND ;//TCP发送数据
u16  f_AT_TCPREAD ;//读取TCP接收的数据，默认不需要查询只需要定时查询串口是否出现“+TCPRECV:”
}ML307R_AT_Error; 

typedef struct HTTP_Error
{
u16  f_AT;//	查询是否回显
u16  f_ATE ;
u16  f_AT_CGSN ;//获取IMEI号
u16  f_AT_CPIN ;//检查SIM卡状态
u16  f_AT_CIMI ;//获取SIM卡的IMSI
u16  f_AT_CCID ;//获取SIM卡ICCD号
u16  f_AT_CSQ  ;//获取信号强度
u16  f_AT_CTZU ;//设置时间更新来源
u16  f_AT_CCLK ;//查询当前时间	
u16  f_AT_MNTP ;//NTP更新时间
u16  f_AT_CREG ;//查询网络注册状态
u16  f_AT_CGATT ;//查询附网状态	
u16  f_AT_CLOSE ;//关闭SOCKET
u16  f_AT_MIPTKA ;//设置keepalive
u16  f_AT_OPEN ;//设置IP+端口
u16  f_AT_STCP ;//设置TCP为透传模式
u16  f_AT_SCNT ;//发送TCPIP数据数量
u16  f_AT_SEND ;//发送TCPIP数据
u16  f_AT_MQTT_Con ;//MQTT链接
	
u16  f_AT_MYSYSINFO ;//查询网络制式
u16  f_AT_CGDCONT ;//设置电信APN
u16  f_AT_XGAUTH ;//设置电信身份认证参数
u16  f_AT_XIIC ; //建立PPP连接获取IP地址
u16  f_AT_CXIIC ;//查询PPP连接是否建立
u16  f_AT_TCPSETUP ;//建立 TCP 连接
u16  f_AT_IPSTATUS ;//查询TCP链路状态
u16  f_AT_TCPSEND ;//TCP发送数据
u16  f_AT_TCPREAD ;//读取TCP接收的数据，默认不需要查询只需要定时查询串口是否出现“+TCPRECV:”
}HTTP_AT_Error; 

typedef struct ML307R_Power
{
		u8 f_PowerStart;
		u8 f_Reset;
	  u8 f_PowerStartComplet;
	  u8 f_ResetCompelet;
	  u16 PowerStartCnt;
	  u16 ResetCnt;
}ML307R_PoerPara;

typedef struct ML307R_Information_Card_IMEI
{
		u8 ML307R_IMEI[20]  ;//模块IMEI号
		u8 ML307R_IMSI[30]  ;//SIM卡IMSI号
		u8 ML307R_ICCID[30] ;//SIM卡ICCID号 
	  u8 ML307R_CSQ[5] ;  //信号值 
}ML307R_Information;
extern ML307R_Information  ML307R_Inf;  
 

void ML307R_START(void);
void ACK_Sever_Cmd(void);
void ML307R_Send_Command(void);
void HTTP_Send_Command(void);
void Check_HTTP_AT_ReciveData(void);//检查AT接收到的数据
void Check_ML307R_AT_ReciveData(void);//检查AT接收到的数据
void ML307R_ProcessSeverData(void);//处理服务器下发的命令
void ML307R_Init(u32 pclk2,u32 bound);
void ML307R_Send_Byte(uint8_t Dat);
void ML307R_Send_array(uint8_t *arr, uint16_t len);//CAT1发送数组
void ML307R_ReceiveData(void);
uint8_t GetAckData(uint8_t *string);
#endif
