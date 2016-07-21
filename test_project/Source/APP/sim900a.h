#ifndef __SIM900A_H
#define __SIM900A_H
#include "stm32f10x.h"
extern volatile bool GSM_send;
extern volatile bool GSM_Stop;
extern volatile bool GSMreceive_flag;
extern volatile bool GSM_first;

extern void sim900a(void);
extern void Send_Message(uint8_t num);

#endif
