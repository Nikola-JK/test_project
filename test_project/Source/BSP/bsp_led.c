/*
*********************************************************************************************************
*	                                  
*	ģ������ : LEDָʾ������ģ��
*	�ļ����� : bsp_led.c
*	��    �� : V1.0
*	˵    �� : ����LEDָʾ��
*
*********************************************************************************************************
*/

#include "stm32f10x.h"
#include <stdio.h>

#include "bsp_led.h"

/*
	ϵͳ����״̬LED1 : 	PG10 (���0����)
	���LED2 : 		PG9  (���0����)
	�ŵ�LED3 : 		PC12 (���0����)
	����LED4 : 		PC11 (���0����)
	USB�豸LED5:		PC10 (���0����)		
*/

#define GPIO_PORT_LED1	GPIOE
#define GPIO_PORT_LED2	GPIOE
#define GPIO_PORT_LED3	GPIOE

#define GPIO_PIN_LED1	GPIO_Pin_4
#define GPIO_PIN_LED2	GPIO_Pin_3
#define GPIO_PIN_LED3	GPIO_Pin_1

#define GPIO_CLK_LED1	RCC_APB2Periph_GPIOE
#define GPIO_CLK_LED2	RCC_APB2Periph_GPIOE
#define GPIO_CLK_LED3	RCC_APB2Periph_GPIOE

/*
*********************************************************************************************************
*	�� �� ��: bsp_InitLed
*	����˵��: ��ʼ��LEDָʾ��
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void bsp_InitLed(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

//#if 1	/* ���ú궨��ķ�ʽ��ʼ��GPIO���Ա����޸�GPIO���� */
	/* ��GPIOF��ʱ�� */
	RCC_APB2PeriphClockCmd(GPIO_CLK_LED1 | GPIO_CLK_LED2 | GPIO_CLK_LED3, ENABLE);

	/* �������е�LEDָʾ��GPIOΪ�������ģʽ */
	/* ���ڽ�GPIO����Ϊ���ʱ��GPIO����Ĵ�����ֵȱʡ��0����˻�����LED����
		�����Ҳ�ϣ���ģ�����ڸı�GPIOΪ���ǰ�����޸�����Ĵ�����ֵΪ1 */
	GPIO_SetBits(GPIO_PORT_LED1,  GPIO_PIN_LED1);
	GPIO_SetBits(GPIO_PORT_LED2,  GPIO_PIN_LED2);
	GPIO_SetBits(GPIO_PORT_LED3,  GPIO_PIN_LED3);


	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	

	GPIO_InitStructure.GPIO_Pin = GPIO_PIN_LED1;
	GPIO_Init(GPIO_PORT_LED1, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_PIN_LED2;
	GPIO_Init(GPIO_PORT_LED2, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_PIN_LED3;
	GPIO_Init(GPIO_PORT_LED3, &GPIO_InitStructure);





//#else
//	/* ��GPIOF��ʱ�� */
//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOF, ENABLE);
//
//	/* �������е�LEDָʾ��GPIOΪ�������ģʽ */
//	/* ���ڽ�GPIO����Ϊ���ʱ��GPIO����Ĵ�����ֵȱʡ��0����˻�����LED����
//		�����Ҳ�ϣ���ģ�����ڸı�GPIOΪ���ǰ�����޸�����Ĵ�����ֵΪ1 */
//	GPIO_SetBits(GPIOF,  GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9);
//	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9;
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
//	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//	GPIO_Init(GPIOF, &GPIO_InitStructure);
//#endif
}

/*
*********************************************************************************************************
*	�� �� ��: bsp_LedOn
*	����˵��: ����ָ����LEDָʾ�ơ�
*	��    �Σ�_no : ָʾ����ţ���Χ 1 - 5
*	�� �� ֵ: ��������
*********************************************************************************************************
*/
void bsp_LedOn(uint8_t _no)
{
	_no--;
	
	if (_no == 0)
	{
		GPIO_PORT_LED1->BRR = GPIO_PIN_LED1;
	}
	else if (_no == 1)
	{
		GPIO_PORT_LED2->BRR = GPIO_PIN_LED2;
	}
	else if (_no == 2)
	{
		GPIO_PORT_LED3->BRR = GPIO_PIN_LED3;
	}

}

/*
*********************************************************************************************************
*	�� �� ��: bsp_LedOff
*	����˵��: Ϩ��ָ����LEDָʾ�ơ�
*	��    �Σ�_no : ָʾ����ţ���Χ 1 - 4
*	�� �� ֵ: ��������
*********************************************************************************************************
*/
void bsp_LedOff(uint8_t _no)
{
	_no--;
	
	if (_no == 0)
	{
		GPIO_PORT_LED1->BSRR = GPIO_PIN_LED1;
	}
	else if (_no == 1)
	{
		GPIO_PORT_LED2->BSRR = GPIO_PIN_LED2;
	}
	else if (_no == 2)
	{
		GPIO_PORT_LED3->BSRR = GPIO_PIN_LED3;
	}

}

/*
*********************************************************************************************************
*	�� �� ��: bsp_LedToggle
*	����˵��: ��תָ����LEDָʾ�ơ�
*	��    �Σ�_no : ָʾ����ţ���Χ 1 - 4
*	�� �� ֵ: ��������
*********************************************************************************************************
*/
void bsp_LedToggle(uint8_t _no)
{
	_no--;
	
	if (_no == 0)
	{
		GPIO_PORT_LED1->ODR ^= GPIO_PIN_LED1;
	}
	else if (_no == 1)
	{
		GPIO_PORT_LED2->ODR ^= GPIO_PIN_LED2;
	}
	else if (_no == 2)
	{
		GPIO_PORT_LED3->ODR ^= GPIO_PIN_LED3;
	}

}




