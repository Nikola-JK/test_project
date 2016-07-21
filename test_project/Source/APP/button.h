
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
/* �����˲�ʱ��50ms, ��λ10ms
 ֻ��������⵽50ms״̬�������Ϊ��Ч����������Ͱ��������¼�
*/
#define BUTTON_FILTER_TIME 	5
#define BUTTON_LONG_TIME 	200		/* ����1�룬��Ϊ�����¼� */

#define GPIOSET  GPIOA
#define GPIOUP   GPIOB
#define GPIODOWN GPIOB
#define GPIO_Pin_SET   GPIO_Pin_3
#define GPIO_Pin_UP    GPIO_Pin_0
#define GPIO_Pin_DOWN  GPIO_Pin_1

/*
	ÿ��������Ӧ1��ȫ�ֵĽṹ�������
	���Ա������ʵ���˲��Ͷ��ְ���״̬�������
*/
typedef struct {
    /* ������һ������ָ�룬ָ���жϰ����ַ��µĺ��� */
    uint8_t (*IsKeyDownFunc)(void);     /* �������µ��жϺ���,1��ʾ���� */

    uint8_t Count;			/* �˲��������� */
    uint8_t FilterTime;		/* �˲�ʱ��(���255,��ʾ2550ms) */
    uint16_t LongCount;		/* ���������� */
    uint16_t LongTime;		/* �������³���ʱ��, 0��ʾ����ⳤ�� */
    uint8_t  State;			/* ������ǰ״̬�����»��ǵ��� */
    uint8_t KeyCodeUp;		/* ��������ļ�ֵ����, 0��ʾ����ⰴ������ */
    uint8_t KeyCodeDown;	/* �������µļ�ֵ����, 0��ʾ����ⰴ������ */
    uint8_t KeyCodeLong;	/* ���������ļ�ֵ����, 0��ʾ����ⳤ�� */
} BUTTON_T;

/* �����ֵ����
	�Ƽ�ʹ��enum, ����#define��ԭ��
	(1) ����������ֵ,�������˳��ʹ���뿴���������
	(2)	�������ɰ����Ǳ����ֵ�ظ���
*/
typedef enum {
    KEY_NONE = 0,			/* 0 ��ʾ�����¼� */
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
	����FIFO�õ�����
*/
#define KEY_FIFO_SIZE	5
typedef struct {
    uint8_t Buf[KEY_FIFO_SIZE];		/* ��ֵ������ */
    uint8_t Read;	/* ��������ָ�� */
    uint8_t Write;	/* ������дָ�� */
} KEY_FIFO_T;

void InitButtonVar(void);
void PutKey(uint8_t _KeyCode);
uint8_t GetKey(void);
void KeyPro(void);
void KeyPortInit(void);


#endif

