#include "main.h"


#include <stdlib.h>
#include <string.h>


extern struct configure *conf;

#define MAX_BUF 50


void struct_malloc(void)
{

	conf = (struct configure *)malloc(sizeof(struct configure));
	
	if(conf == NULL){
	
		printf("configure malloc error!\n\r");
		return;
	}
	
	
	t_now = (struct time_now *)malloc(sizeof(struct time_now));
	if(t_now == NULL){
	
		printf("malloc time_now error\n\r");
		return;
	}	
}

//void variable_init(void)
//{
//	UartHaveData = 0;
//	jump = 0;
//}


/**********************************************************************
�û��ֶ����������Ϣ 

************************************************************************/

void user_configure(void)
{

	char tmp[100];
		
	conf->charge_time = 0xff;
	conf->id =  0xfffff;
	conf->lower_voltage = 0xfffff;
	conf->volume = 0xfff;
	
	
	printf("\n\r���õ�ص���ز���\n\r");
	
	
	printf("\n\r���õ��ID(5λ����):");
	while(conf->id == 0xfffff || conf->id == 0){
	        
	        conf->id = USART_Scanf(99999);
	        }
	
	printf(" %d",conf->id);
	
	
	
	printf("\n\r���õ�صĶ����(��λ��AH):");
	while(conf->volume == 0xfff || conf->volume == 0){
	        
	        conf->volume = USART_Scanf(999);
	        }
	
	printf(" %d",conf->volume);
	
	

	
	printf("\n\r���ó��Ŵ�����");
	while(conf->times == 0xf || conf->times == 0 ){
	        
	        conf->times= USART_Scanf(9);
	        }
	printf(" %d",conf->times);
	
	
	printf("\n\r���ó��ʱ��(��λ��Сʱ):");

	scanf("%s",tmp);
	while(atof(tmp) > 100 || atof(tmp) < 0){
	
		printf("\n\r�����������ֵ��");
		scanf("%s",tmp);
	
	}
	conf->charge_time = atof(tmp);
	printf(" %g",conf->charge_time);
	
		
	printf("\n\r�ŵ����޵�ѹ(��λ��V):");

	memset(tmp, 0, 100);
	scanf("%s",tmp);
	while(atof(tmp) > 100 || atof(tmp) < 0){
	
		printf("\n\r�����������ֵ��");
		scanf("%s",tmp);
	
	}
	conf->lower_voltage = atof(tmp);
	printf(" %g\n\r\n\r",conf->lower_voltage);

	conf_file_name();


}

/**********************************************************************
ʵ�������ļ��е���Ϣ���ú���ʵ�ֶ�ȡ�����ļ�

************************************************************************/

int get_conf(char *profile)
{
	char buf[MAX_BUF];
	char conf_name[MAX_BUF];
	char conf_date[MAX_BUF];

	uint32_t i = 0;
	uint32_t j = 0;

	FRESULT result;
	FATFS fs;
	FIL file;
	DIR DirInf;  



	memset(buf, 0, MAX_BUF);
	memset(conf_name, 0, MAX_BUF);
	memset(conf_date, 0, MAX_BUF);

	result = f_mount(0, &fs);		
	if (result != FR_OK){
		printf("FileSystem Mounted Failed (%d)\r\n", result);
		goto ERROR;
	}

	result = f_opendir(&DirInf, "/");
	if (result != FR_OK){ 
		printf("Open Root Directory Error (%d)\r\n", result);
		goto ERROR;
	}

	result = f_open(&file, profile, FA_OPEN_ALWAYS  | FA_READ);	
	if(result !=FR_OK){
		printf("open error \n\r");
	 	goto ERROR;
	}
	
	while( !f_eof(&file) && f_gets(buf,MAX_BUF, &file) != NULL){


		while((buf[i] != '=') && (buf[i] != '\n') && (buf[i] != '\0')){
		
			conf_name[j] = buf[i];
			
			i++;
			j++;
		}
		conf_name[j] = '\0';

		
		i++;
		j = 0;
		while((buf[i] != '\n') && (buf[i] != '\0')){
			
			conf_date[j] = buf[i];
			
			i++;
			j++;
		}
		
		
		i = j = 0;

		
		if(strcmp(conf_name, "lower_voltage") == 0){
		
			conf->lower_voltage = atof(conf_date);
			
			printf("lower_voltage=%f \r\n",conf->lower_voltage);
		}
		else if(strcmp(conf_name, "upper_voltage") == 0){
		
			conf->upper_voltage = atof(conf_date);	
			printf("upper_voltage=%f \r\n",conf->upper_voltage);			
		}
		else if(strcmp(conf_name, "charge_time") == 0){
			
			conf->charge_time = atof(conf_date);
			printf("charge_time=%f \r\n",conf->charge_time);		
		}
		else if(strcmp(conf_name, "times") == 0){
		
			conf->times = atoi(conf_date);
			printf("times=%d \r\n",conf->times);	
		}
		else if(strcmp(conf_name, "id") == 0){
			
			conf->id = atoi(conf_date);
			printf("id=%d \r\n",conf->id);	
		}
		else if(strcmp(conf_name, "volume") == 0){
		
			conf->volume = atoi(conf_date);
			printf("volume=%d \r\n",conf->volume);	
		}
		else
			continue;


		conf_file_name(); 

	}

	
       	f_close(&file);
	f_mount(0, NULL);
	return 0;
ERROR:

	f_close(&file);
	f_mount(0, NULL);
	return 1;
	
	printf("ERROR");

}



