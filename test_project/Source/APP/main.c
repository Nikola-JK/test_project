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
	printf("\n\r1�����°���1��ʼ����\n\r2�����°���2�������ݵ�U��\n\r");
	printf("\n\r*******************************************\n\r");
		

	while(1){
	
		//����1 PG13   
		if(GPIO_ReadInputDataBit(GPIO_PORT_KEY1, GPIO_PIN_KEY1) == Bit_RESET){

			CH374_DISEN;//�ر�CH374ʹ��
//			user_configure();
			NVIC_Configuration();	
			
			if(get_conf("conf.txt")){		//get("conf.txt")�ɹ�����0
				printf("get_conf erro \r\n");
				sys_error();
			}

			if(battery())
				sys_error();

			goto START;
			
		}

		//����2  PG14  
		else if(GPIO_ReadInputDataBit(GPIO_PORT_KEY2, GPIO_PIN_KEY2) == Bit_RESET){	
		
			if(get_conf("conf.txt"))
				sys_error();
			copy_to_udisk();	//�������ݵ�U��
			goto START;

		}
	OSTimeDlyHMSM(0,0,0,10);	//�ȴ������������
	}
}

