
/*************************************************
  Copyright (C),  2011-2012,  Haohua Tech. Ltd.
  File name:     PID.h
  Author:        BlackStone
  Version:       V1.0
  Date:          2012.11.9
  Description:
  Others:

  History:
    1. Date:
       Author:
       Modification:
    2. ...
*************************************************/
#ifndef __PID_H
#define __PID_H


#define INTEGRALLIMIT 8999
/*PID计算机结构体*/
typedef struct {
    int       PPoint; /*ProcessPoint*/
    int       SPoint; /*SetPoint*/
    long int  Integral;/**/
    float     P_Gain;
    float     I_Gain;
    float     D_Gain;
    int       DeadBand;
    int       LastError;
} PID;

typedef struct {
    char CurrentPIDState;
    char LastPIDState;
} PIDSTATE;

typedef enum {
    PIDIDLE = 0,
    PIDHEAT = 1,
    PIDREFR = 2
} STATE_PID;


extern float process_point, set_point, dead_band;
extern float p_gain, i_gain, d_gain;
extern long int display_value;

void PID_Init(PID *Pid, float ProcessPoint, float SetPoint);
void PID_Tune(PID *Pid, float P_Gain, float I_Gain, float D_gain, float DeadBand);
void PID_SetNewIntegral(PID *Pid, float NewIntegral);
void PID_Bumpless(PID *Pid);
long int PID_Calculate(PID *Pid, float ProcessPoint, float SetPoint);
void Init_PID(PID *Pid, float SetPoint, float ProcessPoint, float P_Gain, float I_Gain,
              float D_Gain, float DeadBand);

#endif



