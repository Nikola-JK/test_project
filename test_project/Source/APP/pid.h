
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


/*PID计算机结构体*/
typedef struct {
    float     PPoint; /*ProcessPoint*/
    float     SPoint; /*SetPoint*/
    float     Integral;/**/
    float     P_Gain;
    float     I_Gain;
    float     D_Gain;
    float     DeadBand;
    float     LastError;
    float     Offset;
} PID;

typedef struct {
    char CurrentPIDState;
    char LastPIDState;
} PIDSTATE;

typedef enum {
    PIDIDLE = 0,
    PIDHEAT = 1,
    PIDREFR = 2,
    PIDHEATING = 3,
    PIDREFRING = 4
} STATE_PID;

enum {
    TUNENONE = 0,
    INC_01 = 1,
    INC_1,
    INC_10,
    DEC_01,
    DEC_1,
    DEC_10
};

enum {
    TUNE_100 = 1,
    TUNE_10  = 2,
    TUNE_1   = 3
};


void PID_Init(PID *Pid, float ProcessPoint, float SetPoint);
void PID_Tune(PID *Pid, float P_Gain, float I_Gain, float D_Gain, float DeadBand, float Offset);
void PID_SetNewIntegral(PID *Pid, float NewIntegral);
void PID_Bumpless(PID *Pid);
float PID_Calculate(PID *Pid, float ProcessPoint, float SetPoint);
void Init_PID(PID *Pid, float SetPoint, float ProcessPoint, float P_Gain, float I_Gain,
              float D_Gain, float DeadBand);

#endif



