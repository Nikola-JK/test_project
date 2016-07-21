

#ifndef __APP_H
#define __APP_H

typedef enum {
	
		Channel1_r_k_Addr = 0x01,
		
//		Channel1_T_k_Addr = 37,
//		
//		Channel1_r_k_Addr = 73,
//		
//		Channel1_C_k_Addr = 109,
		
		R_unwork_Addr = 37,
		
		R_Cal_C_Addr = 38,
		
		C_rated_Addr = 39,
				
} PARA_Addr;	//注意：实际所用地址个数必须写在eeprom.h的NumbOfVar。
							//36通道Channel1_R_k_Addr=36；NumbOfVar = 36+1+1+1 = 39;

/* Private define -----------------------------------------------------------*/
#define SET 1
#define RESET 0
//LED
#define LED_WARNING_ON GPIO_SetBits(GPIOE,GPIO_Pin_4)
#define LED_WARNING_OFF GPIO_ResetBits(GPIOE,GPIO_Pin_4)
#define LED_RUN_ON GPIO_SetBits(GPIOE,GPIO_Pin_3)
#define LED_RUN_OFF GPIO_ResetBits(GPIOE,GPIO_Pin_3)
//按键
#define KEY_CAL GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_8) //CAL/RUN
#define KEY_CLEAR GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_9) //CLEAR
//RS485
#define R_485_EN1 GPIO_SetBits(GPIOC,GPIO_Pin_2)		
#define T_485_EN1 GPIO_ResetBits(GPIOC,GPIO_Pin_2)
#define R_485_EN2 GPIO_SetBits(GPIOB,GPIO_Pin_1)
#define T_485_EN2 GPIO_ResetBits(GPIOB,GPIO_Pin_1)


extern float V_value[36];
extern float T_value[36];
extern float r_value[36];	
extern float C_value[36];	

extern void Channel_Select(uint8_t CHANNEL_NUM);//通道选择，即选择指定的电池进行操作（如校准，获取电压等）
#endif

