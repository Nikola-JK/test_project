
/*************************************************
  Copyright (C),  2010-2011,  Haohua Tech. Ltd.
  File name:      botton.h
  Author:         BlackStone
  Version:        V1.0
  Date:           2012.12.12
  Description:
  Others:
  History:

    1. Date: 	     12.12.12
       Author: 	     BlackStone
       Modification: Build this moulde
    2. ...
*************************************************/
#ifndef _BOTTON__H
#define _BOTTON__H
/* 按键滤波时间50ms, 单位10ms
 只有连续检测到50ms状态不变才认为有效，包括弹起和按下两种事件
*/
#define BUTTON_FILTER_TIME 	5
#define BUTTON_LONG_TIME 	200		/* 持续1秒，认为长按事件 */

#define GPIOSET  GPIOA
#define GPIOUP   GPIOB
#define GPIODOWN GPIOB
#define GPIO_Pin_SET   GPIO_Pin_3
#define GPIO_Pin_UP    GPIO_Pin_0
#define GPIO_Pin_DOWN  GPIO_Pin_1

/*
	每个按键对应1个全局的结构体变量。
	其成员变量是实现滤波和多种按键状态所必须的
*/
typedef struct {
    /* 下面是一个函数指针，指向判断按键手否按下的函数 */
    uint8_t (*IsKeyDownFunc)(void);     /* 按键按下的判断函数,1表示按下 */

    uint8_t Count;			/* 滤波器计数器 */
    uint8_t FilterTime;		/* 滤波时间(最大255,表示2550ms) */
    uint16_t LongCount;		/* 长按计数器 */
    uint16_t LongTime;		/* 按键按下持续时间, 0表示不检测长按 */
    uint8_t  State;			/* 按键当前状态（按下还是弹起） */
    uint8_t KeyCodeUp;		/* 按键弹起的键值代码, 0表示不检测按键弹起 */
    uint8_t KeyCodeDown;	/* 按键按下的键值代码, 0表示不检测按键按下 */
    uint8_t KeyCodeLong;	/* 按键长按的键值代码, 0表示不检测长按 */
} BUTTON_T;

/* 定义键值代码
	推荐使用enum, 不用#define，原因：
	(1) 便于新增键值,方便调整顺序，使代码看起来舒服点
	(2)	编译器可帮我们避免键值重复。
*/
typedef enum {
    KEY_NONE = 0,			/* 0 表示按键事件 */
    KEY_DOWN_SET,
    KEY_DOWN_UP,
    KEY_DOWN_DOWN,
    KEY_UP_SET,
    KEY_UP_UP,
    KEY_UP_DOWN,
    KEY_DOWNLONG_SET,
    KEY_DOWNLONG_UP,
    KEY_DOWNLONG_DOWN
} KEY_ENUM;

/*
	按键FIFO用到变量
*/
#define KEY_FIFO_SIZE	5
typedef struct {
    uint8_t Buf[KEY_FIFO_SIZE];		/* 键值缓冲区 */
    uint8_t Read;	/* 缓冲区读指针 */
    uint8_t Write;	/* 缓冲区写指针 */
} KEY_FIFO_T;

void InitButtonVar(void);
void PutKey(uint8_t _KeyCode);
uint8_t GetKey(void);
void KeyPro(void);
void KeyPortInit(void);


#endif

