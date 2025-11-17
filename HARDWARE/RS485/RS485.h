#ifndef __RS485__H
#define __RS485__H
#include "sys.h"

#define RS485_SR PAout(2)    

void RS485_Smode(void);
void RS485_Rmode(void);
void RS485_Init(u32 pclk1,u32 bound);
void RS485_Send_Byte(uint8_t Dat);
void RS485_Send_array(uint8_t *arr, uint16_t len);

void RS485_Send_Water_Level(void);
void RS485_Send_Ammeter(void);
void RS485_Analysis(void);
void RS485_Analysis_Ammeter(void);
void RS485_Send_Water_meter(void);
void RS485_Analysis_Watermeter(void);

void RS485_Alarm_Analysis(void);
void RS485_Send_Alarm_Start(void);
void RS485_Send_Alarm_End(void);

void RS485_Send_MCB_Start(void);
void RS485_Send_MCB_End(void);
void MCB_Analysis(void);
void RS485_ReceiveData(void)   ;
#endif




