#ifndef __RS_232__H
#define __RS_232__H
#include "sys.h"

void RS232_Init(u32 pclk1,u32 bound);
void RS232_Send_array(uint8_t *arr, uint16_t len);

#endif




