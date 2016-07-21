/*
*********************************************************************************************************
*	                                  
*	模块名称 : LED指示灯驱动模块    
*	文件名称 : bsp_led.h
*
*
*********************************************************************************************************
*/

#ifndef __BOARD_INIT_H
#define __BOARD_INIT_H

#define GPIO_PORT_KEY1	GPIOB
#define GPIO_PIN_KEY1	GPIO_Pin_8
#define GPIO_CLK_KEY1	RCC_APB2Periph_GPIOB

#define GPIO_PORT_KEY2	GPIOB
#define GPIO_PIN_KEY2	GPIO_Pin_9
#define GPIO_CLK_KEY2	RCC_APB2Periph_GPIOB

#define GPIO_PORT_BEEPER	GPIOE
#define GPIO_PIN_BEEPER	GPIO_Pin_0
#define GPIO_CLK_BEEPER	RCC_APB2Periph_GPIOE


#endif


