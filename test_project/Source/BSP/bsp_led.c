/*
*********************************************************************************************************
*	                                  
*	模块名称 : LED指示灯驱动模块
*	文件名称 : bsp_led.c
*	版    本 : V1.0
*	说    明 : 驱动LED指示灯
*
*********************************************************************************************************
*/

#include "stm32f10x.h"
#include <stdio.h>

#include "bsp_led.h"

/*
	系统运行状态LED1 : 	PG10 (输出0点亮)
	充电LED2 : 		PG9  (输出0点亮)
	放电LED3 : 		PC12 (输出0点亮)
	静置LED4 : 		PC11 (输出0点亮)
	USB设备LED5:		PC10 (输出0点亮)		
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
*	函 数 名: bsp_InitLed
*	功能说明: 初始化LED指示灯
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_InitLed(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

//#if 1	/* 采用宏定义的方式初始化GPIO，以便于修改GPIO口线 */
	/* 打开GPIOF的时钟 */
	RCC_APB2PeriphClockCmd(GPIO_CLK_LED1 | GPIO_CLK_LED2 | GPIO_CLK_LED3, ENABLE);

	/* 配置所有的LED指示灯GPIO为推挽输出模式 */
	/* 由于将GPIO设置为输出时，GPIO输出寄存器的值缺省是0，因此会驱动LED点亮
		这是我不希望的，因此在改变GPIO为输出前，先修改输出寄存器的值为1 */
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
//	/* 打开GPIOF的时钟 */
//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOF, ENABLE);
//
//	/* 配置所有的LED指示灯GPIO为推挽输出模式 */
//	/* 由于将GPIO设置为输出时，GPIO输出寄存器的值缺省是0，因此会驱动LED点亮
//		这是我不希望的，因此在改变GPIO为输出前，先修改输出寄存器的值为1 */
//	GPIO_SetBits(GPIOF,  GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9);
//	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9;
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
//	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//	GPIO_Init(GPIOF, &GPIO_InitStructure);
//#endif
}

/*
*********************************************************************************************************
*	函 数 名: bsp_LedOn
*	功能说明: 点亮指定的LED指示灯。
*	形    参：_no : 指示灯序号，范围 1 - 5
*	返 回 值: 按键代码
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
*	函 数 名: bsp_LedOff
*	功能说明: 熄灭指定的LED指示灯。
*	形    参：_no : 指示灯序号，范围 1 - 4
*	返 回 值: 按键代码
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
*	函 数 名: bsp_LedToggle
*	功能说明: 翻转指定的LED指示灯。
*	形    参：_no : 指示灯序号，范围 1 - 4
*	返 回 值: 按键代码
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




