#include "main.h"
#include <string.h>


void conf_file_name(void)
{

	sprintf(conf->file_name, "%d", conf->id);
	strcat(conf->file_name, ".txt");	
}

void led_all_on(void)
{

	bsp_LedOn(1);
	bsp_LedOn(2);
	bsp_LedOn(3);
}

void led_all_off(void)
{

	bsp_LedOff(1);
	bsp_LedOff(2);
	bsp_LedOff(3);
}

void beeper_on(void)
{
//	GPIO_SetBits(GPIOE, GPIO_Pin_0);
}
void beeper_off(void)
{
//	GPIO_ResetBits(GPIOE, GPIO_Pin_0);
}


void br_con_on(void)
{
	GPIO_ResetBits(GPIOA, GPIO_Pin_2);
}

void br_con_off(void)
{
	GPIO_SetBits(GPIOA, GPIO_Pin_2);

}

void load_con_on(void)
{
	GPIO_SetBits(GPIOA, GPIO_Pin_1);
	
}

void load_con_off(void)
{
	GPIO_ResetBits(GPIOA, GPIO_Pin_1);
}

void charge_con_on(void)
{
	GPIO_SetBits(GPIOB, GPIO_Pin_9);	
}
void charge_con_off(void)
{
	GPIO_ResetBits(GPIOB, GPIO_Pin_9);	
}

void sys_error(void)
{
	for(;;){
		led_all_on();
		beeper_on();
	
		delay_ms(400);
			
		beeper_off();
		led_all_off();

		delay_ms(400);
	}
}



//
//
//void itoa(int val, char *buf, int radix) 
//{       
//           
//	char *p;                                              
//	char *firstdig;                                    
//	char temp;                                           
//	int digval;  
//	                                  
//	p = buf;
//	                   
//	firstdig = p; 
//	                                         
//	do{                           
//		digval = (unsigned)(val % radix); 
//		                         
//		val /= radix;                                        
//		  
//		if   (digval > 9)                                   
//			*p++ = (char)(digval - 10 + 'a');                           
//		else                                   
//			*p++ = (char)(digval + '0');                              
//	}while(val > 0); 
//	                       
//	*p-- = '\0';   
//	                     
//	do{                           
//		temp = *p;                          
//		*p = *firstdig;                        
//		*firstdig = temp;               
//		--p;                           
//		++firstdig;                         
//	}while(firstdig < p);    
//
// }

