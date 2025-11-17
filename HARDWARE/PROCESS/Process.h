#ifndef __PROCESS_H
#define __PROCESS_H
#include "sys.h"
#include "delay.h" 
#include "RS485.h" 
#include "GPS.h"
#include "RTC.h"
#include "TIM.h"
#include "ADC.h"
#include "W25Q64.h"
#include "ML307R.h"
#include "MQTT.h"
#include "stdlib.h"
#include "string.h"
#include "ESP8266.h"
#include "RC522.h"
enum RUN
{
      MQTT=0,HTTP,REBOOT,WIFI_MQTT,WIFI_HTTP
};

void Init(void);        //底层初始化
void MQTT_Process(void);//MQTT上传数据
void MQTT_AckData(u8 mode);
void HTTP_Process(void);//HTTP数据更新
void WIFI_MQTT_Process(void);//WIFI MQTT上传数据
void WIFI_HTTP_Process(void);//WIFI HTTP上传数据
#endif
