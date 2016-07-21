

/************************************************************
  Copyright (C), 2010-2011, Haohua Tech. Co., Ltd.
  FileName:  botton.c
  Author:    BlackStone
  Version :  V1.0
  Date: 	 2012.12.12
  Description:     // ʵ�ְ����ļ�⣬��ʵ�ְ��£�����������ļ��
                      ���ҿ�ʵ�ּ�ֵ�Ļ���
					  ʹ�÷�������ʱ����KeyPro,ע������������˲�ʱ��
					  �Ĺ�ϵ
  History:         // ��ʷ�޸ļ�¼
      <author>      <time>    <version >   <desc>
      BlackStone    12.12.12     1.0     build this moudle
***********************************************************/


/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
#include <stdio.h>

#include "button.h"

static BUTTON_T BtnSET;		/* SET �� */
static BUTTON_T BtnUP;      /*UP ��*/
static BUTTON_T BtnDOWN;    /*DOWN ��*/


static KEY_FIFO_T s_Key;		/* ����FIFO����,�ṹ�� */


static uint8_t IsKeyDownSET(void)
{
    if(GPIO_ReadInputDataBit(GPIOSET, GPIO_Pin_SET) == Bit_SET) {
        return 0;
    }

    return 1;
}
static uint8_t IsKeyDownUP(void)
{
    if(GPIO_ReadInputDataBit(GPIOUP, GPIO_Pin_UP) == Bit_SET) {
        return 0;
    }

    return 1;
}
static uint8_t IsKeyDownDOWN(void)
{
    if(GPIO_ReadInputDataBit(GPIODOWN, GPIO_Pin_DOWN) == Bit_SET) {
        return 0;
    }

    return 1;
}

/*************************************************
  Function:       // InitButtonVar
  Description:    // ��ʼ���밴����صĲ���
  Input:          // ��
  Return:         // ��
  Others:         // ����˵��
*************************************************/

void InitButtonVar(void)
{
    /* �԰���FIFO��дָ������ */
    s_Key.Read = 0;
    s_Key.Write = 0;
    KeyPortInit();
    /* ��ʼ��USER�������� */
    BtnSET.IsKeyDownFunc = IsKeyDownSET;	/* �жϰ������µĺ��� */
    BtnSET.FilterTime = BUTTON_FILTER_TIME;	/* �����˲�ʱ�� */
    BtnSET.LongTime = BUTTON_LONG_TIME;		/* ����ʱ�� */
    BtnSET.Count = BtnSET.FilterTime / 2;	/* ����������Ϊ�˲�ʱ���һ�� */
    BtnSET.State = 0;						/* ����ȱʡ״̬��0Ϊδ���� */
    BtnSET.KeyCodeDown = KEY_DOWN_SET;     	/* �������µļ�ֵ���� */
    BtnSET.KeyCodeUp = KEY_UP_SET;		        	/* ��������ļ�ֵ���� */
    BtnSET.KeyCodeLong = KEY_DOWNLONG_SET;		            /* �������������µļ�ֵ���� */
    /* ��ʼ��DOWN�������� */
    BtnDOWN.IsKeyDownFunc = IsKeyDownDOWN;	    /* �жϰ������µĺ��� */
    BtnDOWN.FilterTime = BUTTON_FILTER_TIME;    	/* �����˲�ʱ�� */
    BtnDOWN.LongTime = BUTTON_LONG_TIME;    	/* ����ʱ��, 0��ʾ����� */
    BtnDOWN.Count = BtnDOWN.FilterTime / 2;	/* ����������Ϊ�˲�ʱ���һ�� */
    BtnDOWN.State = 0;							/* ����ȱʡ״̬��0Ϊδ���� */
    BtnDOWN.KeyCodeDown = KEY_DOWN_DOWN;           	/* �������µļ�ֵ���� */
    BtnDOWN.KeyCodeUp = KEY_UP_DOWN;						/* ��������ļ�ֵ���� */
    BtnDOWN.KeyCodeLong = KEY_DOWNLONG_DOWN;				    	/* �������������µļ�ֵ���� */
    /* ��ʼ��UP�������� */
    BtnUP.IsKeyDownFunc = IsKeyDownUP;	/* �жϰ������µĺ��� */
    BtnUP.FilterTime = BUTTON_FILTER_TIME;	/* �����˲�ʱ�� */
    BtnUP.LongTime = BUTTON_LONG_TIME;						/* ����ʱ�� */
    BtnUP.Count = BtnUP.FilterTime / 2;	/* ����������Ϊ�˲�ʱ���һ�� */
    BtnUP.State = 0;							/* ����ȱʡ״̬��0Ϊδ���� */
    BtnUP.KeyCodeUp = KEY_UP_UP;						/* ��������ļ�ֵ���룬0��ʾ����� */
    BtnUP.KeyCodeDown = KEY_DOWN_UP;		/* �������µļ�ֵ���� */
    BtnUP.KeyCodeLong = KEY_DOWNLONG_UP;					/* �������������µļ�ֵ���룬0��ʾ����� */
}

/*************************************************
  Function:       // PutKey
  Description:    // ��һ������ֵѹ�밴��FIFO��
  Input:          // ����ֵ
  Return:         // ��
  Others:         // ����˵��
*************************************************/
void PutKey(uint8_t _KeyCode)
{
    s_Key.Buf[s_Key.Write] = _KeyCode;

    if(++s_Key.Write  >= KEY_FIFO_SIZE) {
        s_Key.Write = 0;
    }
}

/*************************************************
  Function:       // GetKey
  Description:    // �Ӱ���FIFO��ȡ����ֵ
  Input:          // ��
  Return:         // ����ֵ
  Others:         // ����˵��
*************************************************/
uint8_t GetKey(void)
{
    uint8_t ret;

    if(s_Key.Read == s_Key.Write) {
        return KEY_NONE;

    } else {
        ret = s_Key.Buf[s_Key.Read];

        if(++s_Key.Read >= KEY_FIFO_SIZE) {
            s_Key.Read = 0;
        }

        return ret;
    }
}

/*************************************************
  Function:       // DetectButton
  Description:    // ��ⰴ��ֵ���������£�����������
  Input:          // ��Ҫ���İ���
  Return:         // ���İ���ֵ
  Others:         // ����˵��
*************************************************/
static void DetectButton(BUTTON_T *_pBtn)
{
    if(_pBtn->IsKeyDownFunc()) {
        if(_pBtn->Count < _pBtn->FilterTime) {
            _pBtn->Count = _pBtn->FilterTime;

        } else if(_pBtn->Count < 2 * _pBtn->FilterTime) {
            _pBtn->Count++;

        } else {
            if(_pBtn->State == 0) {
                _pBtn->State = 1;

                /* ���Ͱ�ť���µ���Ϣ */
                if(_pBtn->KeyCodeDown > 0) {
                    /* ��ֵ���밴��FIFO */
                    PutKey(_pBtn->KeyCodeDown);
                }
            }

            if(_pBtn->LongTime > 0) {
                if(_pBtn->LongCount < _pBtn->LongTime) {
                    /* ���Ͱ�ť�������µ���Ϣ */
                    if(++_pBtn->LongCount == _pBtn->LongTime) {
                        /* ��ֵ���밴��FIFO */
                        PutKey(_pBtn->KeyCodeLong);
                    }
                }
            }
        }

    } else {
        if(_pBtn->Count > _pBtn->FilterTime) {
            _pBtn->Count = _pBtn->FilterTime;

        } else if(_pBtn->Count != 0) {
            _pBtn->Count--;

        } else {
            if(_pBtn->State == 1) {
                _pBtn->State = 0;

                /* ���Ͱ�ť�������Ϣ */
                if(_pBtn->KeyCodeUp > 0) {
                    /* ��ֵ���밴��FIFO */
                    PutKey(_pBtn->KeyCodeUp);
                }
            }
        }

        _pBtn->LongCount = 0;
    }
}

/*************************************************
  Function:       // KeyPro
  Description:    // ������еİ������趨ʱ����
  Input:          // ��
  Return:         // ��
  Others:         // ����˵��
*************************************************/
void KeyPro(void)
{
    DetectButton(&BtnSET);	/* USER �� */
    DetectButton(&BtnUP);	/* TAMPER �� */
    DetectButton(&BtnDOWN);	/* WAKEUP �� */
}

/*************************************************
  Function:       // KeyPro
  Description:    // ������еİ������趨ʱ����
  Input:          // ��
  Return:         // ��
  Others:         // ����˵��
*************************************************/
void KeyPortInit(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOC
                           | RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOF | RCC_APB2Periph_GPIOG,
                           ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_SET;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOSET, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_UP;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOUP, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_DOWN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIODOWN, &GPIO_InitStructure);
}

