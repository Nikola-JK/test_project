#ifndef __HMI_H
#define __HMI_H
#include "stm32f10x.h"
extern void Analysis(unsigned char *com_buffer);

extern void Param_IF(void);
extern void Show(uint8_t Screen_id);
extern void Abnormal_show(void);

extern volatile bool FLAG_Setif;
#endif
