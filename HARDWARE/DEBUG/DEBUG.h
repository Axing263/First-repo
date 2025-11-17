#ifndef __DEBUG_H
#define __DEBUG_H
#include "sys.h"

void DEBUG_Init(u32 pclk2,u32 bound);
void DEBUG_Send_Byte(uint8_t Dat);
void DEBUG_Send_array(uint8_t *arr, uint16_t len);
 
		 
#endif
