/*************************************************
  Copyright (C),  2010-2013,  Haohua Tech. Ltd.
  File name:      P100.h
  Author:         BlackStone
  Version:        V1.0
  Date:           2013.1.8
  Description:
  Others:
  History:
    1. Date:
       Author:
       Modification:
    2. ...
*************************************************/

#ifndef __PT100_H_
#define __PT100_H_

#include    "stm32f10x.h"

#define PT100_RESIS_ZERO   (80.31)          /*-50��*/
#define PT100_RESIS_FULL   (157.33)         /*150��*/



typedef struct {
    uint16_t CalZero;        /*AD���У׼ֵ*/
    uint16_t CalFull;        /*AD����У׼ֵ*/
    uint16_t ADValue;
    float    ResValue;       /*��������ֵ*/
    float    k;              /*ADУ׼б��k*/
    long     caltemp;
} MEASURECHANEL;

float R2T(float Resis);

#endif
