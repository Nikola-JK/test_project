#ifndef __MAIN_H
#define __MAIN_H

#include <stdio.h>
#include <stdlib.h>

#include "stm32f10x_flash.h"
#include "stm32f10x.h"
#include "systick.h"
#include "usart_printf.h"
#include "usb_hw.h"
#include "usb_pwr.h"
#include "bsp_nand.h"
#include "bsp_led.h"
#include "ff.h"
#include "diskio.h"


void FSmain(void);

void InitBoard(void);
void DispMenu(void);
void ADC_Configuration(void);
void GPIO_Configuration(void);
void NVIC_Configuration(void);
void wtd(void);
void RTC_Configuration(void);
void Time_Display(uint32_t);
void RTC_Check(void);
void Time_Adjust(void);
void user_configure(void);
void usb_disk_init(void);
int battery(void);
void AdcPro(void);
void itoa(int val, char *buf, int radix);
void copy_to_udisk(void);
void charge_con_off(void);
void charge_con_on(void);
void load_con_off(void);
void load_con_on(void);
void br_con_off(void);
void br_con_on(void);
int get_conf(char *profile);
void struct_malloc(void);
void conf_file_name(void);
void led_all_on(void);
void led_all_off(void);
void ftoa(double db ,char *db_str);
void sys_error(void);



uint32_t tosecond(int16_t year,int16_t month,int16_t date,int16_t hour,int16_t min,int16_t sec);

uint32_t Time_Regulate(void);
uint32_t USART_Scanf(uint32_t value);
double ADC_SAMPLING(void);
uint16_t GetADC(void);
int Ymd2Wday(uint32_t , uint32_t, uint32_t);


#define SecsPerDay (3600*24)
#define SAMP_COUNT	20

#define SecsPerComYear  3153600		//(365*3600*24)
#define SecsPerLeapYear 31622400	//(366*3600*24)
#define SecsPerFourYear 126230400	//((365*3600*24)*3+(366*3600*24))
#define SecsPerDay      (3600*24)
#define	WR_MAX		350		//写缓冲大小
#define TIME_MAX	20

#define CH374_EN 			GPIO_ResetBits(GPIOB, GPIO_Pin_6)
#define CH374_DISEN 	GPIO_SetBits(GPIOB, GPIO_Pin_6)


struct configure{

	double lower_voltage;		//放电下限电压
	double upper_voltage;		//充电上限电压
        double charge_time ;		//充电时间
	uint32_t times;			//充放次数
        uint32_t id;			//电池ID
	uint32_t volume;		//电池额定容量
	char file_name[20];
	
};

struct time_now{

	char  year[5];
	char  month[5];
	char  day[5];
	char  hour[5];
	char  min[5];
	char  sec[5];

};


extern struct time_now *t_now;

extern struct configure *conf;

extern char com_buff[20];
extern int UartHaveData;
extern int jump;
	


#endif
