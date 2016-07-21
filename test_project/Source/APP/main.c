#include "main.h"
#include "board_init.h"
#include "test.h"

struct configure *conf;
struct time_now *t_now;


void FSmain(void)
{

	InitBoard();
	
	DispMenu();

	RTC_Check();


	
	InitUsbDev();
	
	struct_malloc(); 


 START:	

  printf("\n\r*******************************************\n\r");
	printf("\n\r1、按下按键1开始测试\n\r2、按下按键2拷贝数据到U盘\n\r");
	printf("\n\r*******************************************\n\r");
		

	while(1){
	
		//按键1 PG13   
		if(GPIO_ReadInputDataBit(GPIO_PORT_KEY1, GPIO_PIN_KEY1) == Bit_RESET){

			CH374_DISEN;//关闭CH374使能
//			user_configure();
			NVIC_Configuration();	
			
			if(get_conf("conf.txt")){		//get("conf.txt")成功返回0
				printf("get_conf erro \r\n");
				sys_error();
			}

			if(battery())
				sys_error();

			goto START;
			
		}

		//按键2  PG14  
		else if(GPIO_ReadInputDataBit(GPIO_PORT_KEY2, GPIO_PIN_KEY2) == Bit_RESET){	
		
			if(get_conf("conf.txt"))
				sys_error();
			copy_to_udisk();	//拷贝数据到U盘
			goto START;

		}
	OSTimeDlyHMSM(0,0,0,10);	//等待检测器检测完毕
	}
}

