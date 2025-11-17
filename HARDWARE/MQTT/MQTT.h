#ifndef __MQTT_H
#define __MQTT_H
#include "sys.h"
#include "ML307R.h"


#define  TOPIC_NUM    2                              //需要订阅的最大Topic数量
#define  TOPIC_SIZE   128                            //存放Topic字符串名称缓冲区长度

typedef struct{
	char ClientID[128];                        //存放客户端ID的缓冲区
	char Username[128];                        //存放用户名的缓冲区
	char Passward[128];                        //存放密码的缓冲区
	char ServerIP[128];                        //存放服务器IP或是域名
	char ServerPort[5];                           //存放服务器的端口号
	unsigned char Pack_buff[1024];              //发送报文的缓冲区
	int  MessageID;                            //记录报文标识符
	int  Fixed_len;                       	   //固定报头长度
	int  Variable_len;                         //可变报头长度
	int  Payload_len;                          //有效负荷长度
	char Stopic_Buff[TOPIC_NUM][TOPIC_SIZE];   //包含的是订阅的主题列表
	char cmdbuff[3000];                        //保存推送的数据中的命令数据部分
	int  streamId;                             //OTA升级时，保存streamId数据
	int  streamFileId;                         //OTA升级时，保存streamFileId
	int  streamSize;                           //OTA升级时，固件总的大小
}MQTT_CB;

#define MQTT_CB_LEN         sizeof(MQTT_CB)    //结构体长度 
	
int powdata(int x, int y);
void IoT_Parameter_Init(void);
void MQTT_ConectPack(void); 
void MQTT_PublishQs1(char *topic, char *data, int data_len);
void MQTT_DealPushdata_Qs0(unsigned char *redata, int data_len);
 


void MQTT_PingREQ(u8 mode);
void MQTT_PublishQs0(char *topic, char *data, int data_len,u8 mode);
void MQTT_Subscribe(char topicbuff[TOPIC_NUM][TOPIC_SIZE], int topicnum, unsigned char Qs,u8 mode);
		
#endif




