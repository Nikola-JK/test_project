
/************************************************************
  Copyright (C), 2011-2012, Haohua Tech. Co., Ltd.
  FileName:  PID.C
  Author:    BlackStone
  Version :  V1.0
  Date:      2012.11.8
  Description:   ���ļ���Ҫ�ṩPID�㷨���漰����һЩ����
  Version:   ���汾Ϊһ��Ӧ�õĳ��ΰ汾���Ժ��д���һ������
  History:
      <author>    <time>    <version >   <desc>
      BlackStone  12/11/8     1.0     build this moudle
***********************************************************/

#include "PID.h"


#define INTEGRALLIMIT 8999
#define IntegralBand  5.0

/*************************************************
  Function:       PID_Init()
  Description:    ��ʼ��PID����ĸ���������ڼ���PID֮ǰ����
  Input:          Pid--->�����PID�ṹ��
                  ProcessPoint--->���ƹ����еĵ�ǰ��
                  SetPoint--->�趨��Ŀ����Ƶ�
  Return:         // ��
  Others:         // �ڼ���PID֮ǰ����
*************************************************/

void PID_Init(PID *Pid, float ProcessPoint, float SetPoint)
{
    Pid->PPoint = ProcessPoint;
    Pid->SPoint = SetPoint;
}


/*************************************************
  Function:       PID_Tune()
  Description:    PID�Ĳ�������
  Input:          Pid     --->�����PID�ṹ��
                  P_Gain  --->����ϵ��
                  I_Gain  --->����ϵ��
                  D_Gain  --->΢��ϵ��
                  DeadBand--->������������
  Return:         // ��
  Others:         //
*************************************************/
void PID_Tune(PID *Pid, float P_Gain, float I_Gain, float D_Gain, float DeadBand, float Offset)
{
    Pid->P_Gain    = P_Gain;
    Pid->I_Gain    = I_Gain;
    Pid->D_Gain    = D_Gain;
    Pid->DeadBand  = DeadBand;
    Pid->Offset    = Offset;
// Pid->LastError = 0;
}

/*************************************************
  Function:       PID_SetNewIntegral()
  Description:    �趨һ���µļ���֮
  Input:          Pid        --->�����PID�ṹ��
                  NewIntegral--->�趨���µĻ���

  Return:         // ��
  Others:         // ��PID�л���Խ��ʱʹ��
*************************************************/
void PID_SetNewIntegral(PID *Pid, float NewIntegral)
{
    Pid->Integral = NewIntegral;
}

/*************************************************
  Function:       PID_Bumpless()
  Description:    ��PID���ƹ����������趨Ŀ��ֵ�ı仯����������ĳ��ԭ����ͣ����
                  �ٴ�����PID����ʱ��PID�ļ�����ܻ�����쳣
  Input:          Pid--->�����PID�ṹ��
  Return:         // ��
  Others:         //
*************************************************/
void PID_Bumpless(PID *Pid)
{
    Pid->LastError = Pid->SPoint - Pid->PPoint;
}

/*************************************************
  Function:       PID_Calculate()
  Description:    ����PID�����
  Input:          Pid--->�����PID�ṹ��
  Return:         // ��
  Others:         // �̶����ڵĵ��ü��㣬�����������úͻ���ֵ�������ù���
                    Ϊ�˱������ʱ��������»���Ӱ����������˻���������
                    �����ƹ��̴ﵽ����������ʱ���������������
*************************************************/


float PID_Calculate(PID *Pid, float ProcessPoint, float SetPoint)
{
    float Result  = 0;   //PID������
    float Pterm   = 0;  //PID�����ı�����
    float Iterm   = 0;  //PID�����Ļ�����
    float Dterm   = 0;  //PID������΢����
    float CurrentError = 0;  //�������
    Pid->PPoint = ProcessPoint;
    Pid->SPoint = SetPoint;
    CurrentError = Pid->SPoint - Pid->PPoint;

    if(CurrentError >= 5.0) {
        Result = 8999;
        return Result;
    }

    /*���Ʒ�Χ������֮��*/
    if((CurrentError >= (Pid->DeadBand)) || (CurrentError < (-(Pid->DeadBand)))) {
        /*���������*/
        Pterm = (Pid->P_Gain) * CurrentError;
        /*����΢����*/
        Dterm = Pid->D_Gain * (CurrentError - Pid->LastError);

        /*���������ھ���Ŀ���趨ֵ��ĳ�������ڿ�ʼ���֣���С�������Ӱ��*/
        if((CurrentError > IntegralBand) || (CurrentError < (-(IntegralBand)))) {
            Iterm = Pid->Integral = 0;
            Result = Pterm + Iterm + Dterm;

        } else {
            Pid->Integral += Pid->I_Gain * CurrentError;

            //Iterm =	Pid->Integral;
            if(Pid->Integral > INTEGRALLIMIT) {
                Pid->Integral = INTEGRALLIMIT;
                Iterm = Pid->Integral;

            } else {
                Iterm = Pid->Integral;
            }

            Result = Pterm + Iterm + Dterm + Pid->Offset;
        }
    }

    /*������֮�ڵ�PID���㣬��������������΢���ֱ�ӷ��ػ��֣����˲��Ƽ�*/
    else {
        Result = Iterm;
    }

    /*����LastError*/
    Pid->LastError = CurrentError;
    return(Result);
}

/*************************************************
  Function:       Init_PID()
  Description:    ��ʼ��PID�ĸ������
  Input:          Pid--->�����PID�ṹ��
  Return:         // ��
  Others:         // ��ʼ��PID�ĸ��������ע�����ֵ����
*************************************************/
void Init_PID(PID *Pid, float SetPoint, float ProcessPoint, float P_Gain, float I_Gain,
              float D_Gain, float DeadBand)
{
    PID_Init(Pid, ProcessPoint, SetPoint);
    PID_Tune(Pid, P_Gain, I_Gain, D_Gain, DeadBand, 0);
    PID_SetNewIntegral(Pid, 0.0);
    //PID_Bumpless(Pid);
}
