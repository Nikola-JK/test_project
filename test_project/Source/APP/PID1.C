
/************************************************************
  Copyright (C), 2011-2012, Haohua Tech. Co., Ltd.
  FileName:  PID.C
  Author:    BlackStone
  Version :  V1.0
  Date:      2012.11.8
  Description:   本文件主要提供PID算法中涉及到的一些函数
  Version:   本版本为一般应用的初次版本，以后有待进一步升级
  History:
      <author>    <time>    <version >   <desc>
      BlackStone  12/11/8     1.0     build this moudle
***********************************************************/
#include "includes.h"

float DeadBand     = 0;
float IntegralBand = 8.0;

/*************************************************
  Function:       PID_Init()
  Description:    初始化PID计算的各项参数，在计算PID之前调用
  Input:          Pid--->计算的PID结构体
                  ProcessPoint--->控制过程中的当前点
                  SetPoint--->设定的目标控制点
  Return:         // 无
  Others:         // 在计算PID之前调用
*************************************************/

void PID_Init(PID *Pid, float ProcessPoint, float SetPoint)
{
    Pid->PPoint = (int)ProcessPoint * 100;
    Pid->SPoint = (int)SetPoint * 100;
}


/*************************************************
  Function:       PID_Tune()
  Description:    PID的参数调整
  Input:          Pid     --->计算的PID结构体
                  P_Gain  --->比例系数
                  I_Gain  --->积分系数
                  D_Gain  --->微分系数
                  DeadBand--->控制死区设置
  Return:         // 无
  Others:         //
*************************************************/
void PID_Tune(PID *Pid, float P_Gain, float I_Gain, float D_Gain, float DeadBand)
{
    Pid->P_Gain    = (int)(P_Gain * 100);
    Pid->I_Gain    = (int)(I_Gain * 100);
    Pid->D_Gain    = (int)(D_Gain * 100);
    Pid->DeadBand  = (int)(DeadBand * 100);
    Pid->LastError = 0;
}

/*************************************************
  Function:       PID_SetNewIntegral()
  Description:    设定一个新的几分之
  Input:          Pid        --->计算的PID结构体
                  NewIntegral--->设定的新的积分

  Return:         // 无
  Others:         // 在PID中积分越界时使用
*************************************************/
void PID_SetNewIntegral(PID *Pid, float NewIntegral)
{
    Pid->Integral = (int)NewIntegral * 100;
}

/*************************************************
  Function:       PID_Bumpless()
  Description:    在PID控制过程中由于设定目标值的变化，或者由于某种原因暂停控制
                  再次启动PID控制时，PID的计算可能会出现异常
  Input:          Pid--->计算的PID结构体
  Return:         // 无
  Others:         //
*************************************************/
void PID_Bumpless(PID *Pid)
{
    Pid->LastError = Pid->SPoint - Pid->PPoint;
}

/*************************************************
  Function:       PID_Calculate()
  Description:    计算PID结果。
  Input:          Pid--->计算的PID结构体
  Return:         // 无
  Others:         // 固定周期的调用计算，具有死区设置和积分值上限设置功能
                    为了避免积分时间过长导致积分影响过大，设置了积分区域，在
                    当控制过程达到积分区域内时，积分项才起作用
*************************************************/

long int Pterm   = 0;  //PID计算后的比例项
long int Iterm   = 0;  //PID计算后的积分项
long int Dterm   = 0;  //PID计算后的微分项
int CurrentError = 0;  //本次误差
uint32_t Result  = 0;  //PID计算结果
long int PID_Calculate(PID *Pid, float ProcessPoint, float SetPoint)
{
    Pid->PPoint = (int)(ProcessPoint * 100);
    Pid->SPoint = (int)(SetPoint * 100);
    CurrentError = Pid->SPoint - Pid->PPoint;

    /*控制范围在死区之外*/
    if(CurrentError > (Pid->DeadBand * 100)) {
        /*计算比例项*/
        Pterm = Pid->P_Gain * CurrentError;
        /*计算微分项*/
        Dterm = Pid->D_Gain * (CurrentError - Pid->LastError);

        /*计算积分项，在距离目标设定值的某个区间内开始积分，减小积分项的影响*/
        if(CurrentError > IntegralBand * 100) {
            Iterm = Pid->Integral = 0;
            Result = Pterm + Iterm + Dterm;

        } else {
            Iterm = Pid->Integral += Pid->I_Gain * CurrentError;

            if(Pid->Integral > INTEGRALLIMIT) {
                Iterm = Pid->Integral = INTEGRALLIMIT;

            } else {
                Iterm = Pid->Integral;
            }

            Result = Pterm + Iterm + Dterm;
        }
    }

    /*在死区之内的PID计算，不计算其比例项和微分项，直接返回积分，个人不推荐*/
    else {
        Result = Iterm;
    }

    /**********更新LastError********************/
    Pid->LastError = CurrentError;
    return(Result);
}

/*************************************************
  Function:       Init_PID()
  Description:    初始化PID的各项参数
  Input:          Pid--->计算的PID结构体
  Return:         // 无
  Others:         // 初始化PID的各项参数，注意积分值清零
*************************************************/
void Init_PID(PID *Pid, float SetPoint, float ProcessPoint, float P_Gain, float I_Gain,
              float D_Gain, float DeadBand)
{
    PID_Init(Pid, ProcessPoint, SetPoint);
    PID_Tune(Pid, P_Gain, I_Gain, D_Gain, DeadBand);
    PID_SetNewIntegral(Pid, 0.0);
    PID_Bumpless(Pid);
}

