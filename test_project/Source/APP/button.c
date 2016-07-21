

/************************************************************
  Copyright (C), 2010-2011, Haohua Tech. Co., Ltd.
  FileName:  botton.c
  Author:    BlackStone
  Version :  V1.0
  Date: 	 2012.12.12
  Description:     // 实现按键的检测，可实现按下，长按，弹起的检测
                      并且可实现键值的缓冲
					  使用方法：定时调用KeyPro,注意调用周期与滤波时间
					  的关系
  History:         // 历史修改记录
      <author>      <time>    <version >   <desc>
      BlackStone    12.12.12     1.0     build this moudle
***********************************************************/


/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
#include <stdio.h>

#include "button.h"

static BUTTON_T BtnSET;		/* SET 键 */
static BUTTON_T BtnUP;      /*UP 键*/
static BUTTON_T BtnDOWN;    /*DOWN 键*/


static KEY_FIFO_T s_Key;		/* 按键FIFO变量,结构体 */


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
  Description:    // 初始化与按键相关的参数
  Input:          // 无
  Return:         // 无
  Others:         // 其它说明
*************************************************/

void InitButtonVar(void)
{
    /* 对按键FIFO读写指针清零 */
    s_Key.Read = 0;
    s_Key.Write = 0;
    KeyPortInit();
    /* 初始化USER按键变量 */
    BtnSET.IsKeyDownFunc = IsKeyDownSET;	/* 判断按键按下的函数 */
    BtnSET.FilterTime = BUTTON_FILTER_TIME;	/* 按键滤波时间 */
    BtnSET.LongTime = BUTTON_LONG_TIME;		/* 长按时间 */
    BtnSET.Count = BtnSET.FilterTime / 2;	/* 计数器设置为滤波时间的一半 */
    BtnSET.State = 0;						/* 按键缺省状态，0为未按下 */
    BtnSET.KeyCodeDown = KEY_DOWN_SET;     	/* 按键按下的键值代码 */
    BtnSET.KeyCodeUp = KEY_UP_SET;		        	/* 按键弹起的键值代码 */
    BtnSET.KeyCodeLong = KEY_DOWNLONG_SET;		            /* 按键被持续按下的键值代码 */
    /* 初始化DOWN按键变量 */
    BtnDOWN.IsKeyDownFunc = IsKeyDownDOWN;	    /* 判断按键按下的函数 */
    BtnDOWN.FilterTime = BUTTON_FILTER_TIME;    	/* 按键滤波时间 */
    BtnDOWN.LongTime = BUTTON_LONG_TIME;    	/* 长按时间, 0表示不检测 */
    BtnDOWN.Count = BtnDOWN.FilterTime / 2;	/* 计数器设置为滤波时间的一半 */
    BtnDOWN.State = 0;							/* 按键缺省状态，0为未按下 */
    BtnDOWN.KeyCodeDown = KEY_DOWN_DOWN;           	/* 按键按下的键值代码 */
    BtnDOWN.KeyCodeUp = KEY_UP_DOWN;						/* 按键弹起的键值代码 */
    BtnDOWN.KeyCodeLong = KEY_DOWNLONG_DOWN;				    	/* 按键被持续按下的键值代码 */
    /* 初始化UP按键变量 */
    BtnUP.IsKeyDownFunc = IsKeyDownUP;	/* 判断按键按下的函数 */
    BtnUP.FilterTime = BUTTON_FILTER_TIME;	/* 按键滤波时间 */
    BtnUP.LongTime = BUTTON_LONG_TIME;						/* 长按时间 */
    BtnUP.Count = BtnUP.FilterTime / 2;	/* 计数器设置为滤波时间的一半 */
    BtnUP.State = 0;							/* 按键缺省状态，0为未按下 */
    BtnUP.KeyCodeUp = KEY_UP_UP;						/* 按键弹起的键值代码，0表示不检测 */
    BtnUP.KeyCodeDown = KEY_DOWN_UP;		/* 按键按下的键值代码 */
    BtnUP.KeyCodeLong = KEY_DOWNLONG_UP;					/* 按键被持续按下的键值代码，0表示不检测 */
}

/*************************************************
  Function:       // PutKey
  Description:    // 将一个按键值压入按键FIFO区
  Input:          // 按键值
  Return:         // 无
  Others:         // 其它说明
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
  Description:    // 从按键FIFO获取按键值
  Input:          // 无
  Return:         // 按键值
  Others:         // 其它说明
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
  Description:    // 检测按键值，包括按下，长按，弹起
  Input:          // 需要检测的按键
  Return:         // 检测的按键值
  Others:         // 其它说明
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

                /* 发送按钮按下的消息 */
                if(_pBtn->KeyCodeDown > 0) {
                    /* 键值放入按键FIFO */
                    PutKey(_pBtn->KeyCodeDown);
                }
            }

            if(_pBtn->LongTime > 0) {
                if(_pBtn->LongCount < _pBtn->LongTime) {
                    /* 发送按钮持续按下的消息 */
                    if(++_pBtn->LongCount == _pBtn->LongTime) {
                        /* 键值放入按键FIFO */
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

                /* 发送按钮弹起的消息 */
                if(_pBtn->KeyCodeUp > 0) {
                    /* 键值放入按键FIFO */
                    PutKey(_pBtn->KeyCodeUp);
                }
            }
        }

        _pBtn->LongCount = 0;
    }
}

/*************************************************
  Function:       // KeyPro
  Description:    // 检测所有的按键，需定时调用
  Input:          // 无
  Return:         // 无
  Others:         // 其它说明
*************************************************/
void KeyPro(void)
{
    DetectButton(&BtnSET);	/* USER 键 */
    DetectButton(&BtnUP);	/* TAMPER 键 */
    DetectButton(&BtnDOWN);	/* WAKEUP 键 */
}

/*************************************************
  Function:       // KeyPro
  Description:    // 检测所有的按键，需定时调用
  Input:          // 无
  Return:         // 无
  Others:         // 其它说明
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

