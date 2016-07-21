/*****************************************
	电池充放电
*******************************************/

#include <string.h>
#include <math.h>
#include "main.h"



int charge(void);
int dis_charge(void);
int charge_timeout(double,double,int);
int dis_charge_timeout(double);
int delay_hour(void);
int show_inf(void);
int show_dis_time(void);


void record_time(char *);

int time_out = 0;
int rec = 0;

int record[100] = {0};




int battery(void)
{


	int i;
	printf("\n\r");
	Time_Display(RTC_GetCounter());
	if(show_inf())
		return 1;

//	printf("\r放电开始");
//	Time_Display(RTC_GetCounter());
//	printf("\n\r");
//
//	if(dis_charge())
//		return 1;
//
//	printf("放电结束");
//	Time_Display(RTC_GetCounter());
//	printf("\n\r放电时间为：%d分钟\n\r",record[0]);
//
//	if(delay_hour())
//		return 1;



	printf("\r不加修复器:充电 开始");
	Time_Display(RTC_GetCounter());
	printf("\n\r");

	if(charge())
		return 1;
	
       	printf("不加修复器:充电 结束");
	Time_Display(RTC_GetCounter());
	printf("\n\r");

	if(delay_hour())
		return 1;
	
	printf("\n\r不加修复器:放电 开始");
	Time_Display(RTC_GetCounter());
	printf("\n\r");
	

	if(dis_charge())
		return 1;

	printf("不加修复器:放电 结束");
	Time_Display(RTC_GetCounter());
	printf("\n\r放电时间为：%d分钟\n\r",record[0]);

	if(delay_hour())
		return 1;

	printf("\n\r带修复器充放%d次\n\r",conf->times);
	br_con_on();
	for(i = 0; i < conf->times; i++){
		

		printf("\n\r带修复器第%d次:充电 开始",i + 1);
		Time_Display(RTC_GetCounter());
		printf("\n\r");

		if(charge())
			return 1;

		printf("带修复器第%d次:充电 结束",i + 1);
		Time_Display(RTC_GetCounter());
		printf("\n\r");
		
							 
		if(delay_hour())
			return 1;

		printf("\n\r带修复器第%d次:放电 开始",i + 1);
		Time_Display(RTC_GetCounter());
		printf("\n\r");

		if(dis_charge())
			return 1;


		printf("带修复器第%d次:放电 结束",i + 1);
		Time_Display(RTC_GetCounter());
		printf("\n\r放电时间为：%d分钟\n\r",record[i+1]);

		if(i+1 < conf->times){
				

			if(delay_hour())
				return 1;
		}
		
	}

	br_con_off();

	if(show_dis_time())
		return 1;

	time_out = 0;
	rec = 0;

	memset(record, 0, sizeof(record));	
	
	return 0;


	
}

/***********************************************************
电池充电

**************************************************************/
int charge(void)
{

	FRESULT result;
	FATFS fs;
	FIL file;
	DIR DirInf;  
	uint32_t bw;


	char time_buf[TIME_MAX];
	char write_buf[WR_MAX];
	char tmp_buf[8];
	int i = 0;
	int min_count = 0;
	int adc_value = 0;
	int adc_tmp = 0;
	int flag = 0;
	int times = 0;





	double charge_time = 0;
	double vol;

/* 挂载文件系统，打开文件，定位文件指针 */
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
	
	result = f_open(&file, conf->file_name, FA_OPEN_ALWAYS | FA_READ | FA_WRITE);	
	if(result !=FR_OK){
		printf("open error, errornu:%d\n\r",result);
	 	goto ERROR;
	}

	if(f_lseek(&file,file.fsize) != FR_OK){
		printf("f_lseek error, errornu:%d\n\r",result);
		goto ERROR;
	}


	beeper_on();					   /* 蜂鸣器提示 */
	delay_ms(200);
	beeper_off();

	load_con_off();					/*负载通道关闭 */
	charge_con_on(); 				/*打开CHAR_CON充电通道 */

	bsp_LedOn(2);

	memset(time_buf,0,TIME_MAX);	  		/*清空数组，防止乱码*/
	memset(write_buf,0,WR_MAX);

	StartTimer(0,  100);  	
	adc_value = GetADC();
							
	do{
	       if(jump)
			goto JMP;	       	
		if(CheckTimer(0)) {
		
			vol = ADC_SAMPLING();	
			charge_time ++;
			min_count ++;
			
			StartTimer(0, 1000);


			adc_tmp = GetADC();


		  if((abs(adc_value - adc_tmp) > 3) || (times != 0)){
		     		
				if(abs(adc_value - adc_tmp) <= 3){
					times = 0;
				}
				else{
						times++;
					if(times >= 10){
						times = 0;
						flag = 0;
						adc_value = adc_tmp;	
					}
				}
		  }
		  else if((abs(adc_value - adc_tmp) <= 3) && (times == 0)){
			 
				flag ++;
			}
		
					
			if(min_count >= 60){	      		/* 每隔一分钟采样一次 */

				min_count = 0;
				record_time(time_buf);
				strcat(time_buf,",");						
	
				strcat(write_buf, time_buf);
				memset(time_buf,0,TIME_MAX);		
	
								
				sprintf(tmp_buf, "%.2f", vol);
				strcat(write_buf, tmp_buf);								      
				strcat(write_buf,",充电\n");
	
				if(i > 9){		     /* 10次采样将数据记录到flash中 */
					result = f_write(&file,write_buf,sizeof(write_buf),&bw);			
					if(result != FR_OK){
						printf("file write faild, errornu:%d\n\r",result);
						goto ERROR;
					}
					
					memset(write_buf,0,WR_MAX);
					i = 0;
				}
				i++;	
	
			}

		}
		   			
	}while(charge_timeout(charge_time/3600, vol, flag));   /* 充电时间 */

JMP:
	jump = 0;
							
	if(i != 0){			/* 最后将不足10次的数据记录到flash中 */

		result = f_write(&file,write_buf,(15 + TIME_MAX)*i, &bw);			
		if(result != FR_OK){
			printf("file write faild, errornu:%d\n\r",result);
			goto ERROR;
			
		}
		i = 0;		
	}
	memset(write_buf,0,WR_MAX);

	charge_con_off();	    /* 关闭CHAR_CON充电通道 */
	bsp_LedOff(2);

       /* 关闭文件，卸载文件系统 */
	f_close(&file);		      
	f_mount(0, NULL);
	return 0;
							
ERROR:			    	/* 错误处理 */
	bsp_LedOff(2);
	f_close(&file);
	f_mount(0, NULL);
	return 1;


}

/***********************************************
电池放电

***********************************************/
int dis_charge(void)
{

	FRESULT result;
	FATFS fs;
	FIL file;
	DIR DirInf;  
	uint32_t bw;


	char time_buf[TIME_MAX];
	char write_buf[WR_MAX];
	char tmp_buf[8];

	int dis_time = 0;
	int i = 0;
	int min_count = 0;
	
	double vol;


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
	
	result = f_open(&file, conf->file_name, FA_OPEN_ALWAYS | FA_READ | FA_WRITE);	
	if(result !=FR_OK){
		printf("open error, errornu:%d\n\r",result);
	 	goto ERROR;
	}

	if(f_lseek(&file,file.fsize) != FR_OK){
		printf("f_lseek error, errornu:%d\n\r",result);
		goto ERROR;
	}
	 	
	load_con_on();		//打开负载通道LOAD_CON
	bsp_LedOn(3); 
	
	beeper_on();
	delay_ms(200);
	beeper_off(); 
	
	memset(time_buf,0,TIME_MAX);
	memset(write_buf,0,WR_MAX);


	StartTimer(0,  1000);  	
//	Time_Display(RTC_GetCounter());
//	vol = ADC_SAMPLING();	

	do{
		if(jump)
			goto JMP;
		if(CheckTimer(0)) {	
			dis_time ++;
			StartTimer(0,  1000);
			min_count ++;
			vol = ADC_SAMPLING();

			if(min_count >=60){
				min_count = 0;
			
				memset(time_buf,0,TIME_MAX);	
				record_time(time_buf);
				strcat(time_buf,",");						
	
				strcat(write_buf, time_buf);
				memset(time_buf,0,TIME_MAX);		
	
								
				sprintf(tmp_buf, "%.2f", vol);	
				strcat(write_buf, tmp_buf);							      
				strcat(write_buf,",放电\n");
	
				if(i > 9){
					result = f_write(&file,write_buf,sizeof(write_buf),&bw);			
					if(result != FR_OK){
						printf("file write faild, errornu:%d\n\r",result);
						goto ERROR;
					}
					
					memset(write_buf,0,WR_MAX);
					i = 0;
				}
	
				i++;
			}
		}
	
	}while(dis_charge_timeout(vol));

JMP:
	record[rec]= dis_time/60;
	rec++;

	

	jump = 0;
	if(i != 0){

		result = f_write(&file,write_buf,35*i, &bw);			
		if(result != FR_OK){
			printf("file write faild, errornu:%d\n\r",result);
			goto ERROR;
			
		}
		i = 0;
	}
	memset(write_buf,0,WR_MAX);


	bsp_LedOff(3);							//放电结束

	load_con_off();
	
	f_close(&file);
	f_mount(0, NULL);
	return 0;						//关闭负载通道LOAD_CON

ERROR:
	bsp_LedOff(3);
	f_close(&file);
	f_mount(0, NULL);
	return 1;



}

/*********************************************************
	充电时间控制，
	充电结束返回：0
	未结束返回：  1

**********************************************************/
static int charge_timeout(double time, double vol, int value)
{


//	if(vol >= conf->upper_voltage){  //上限电压控制
//		delay_ms(1000);
//		time_out++;
//	
//		if(time_out > 10){	
//			
//			time_out = 0;
//			return 0;
//		}
//		else
//			return 1;
//	}
	 if((time  >= conf->charge_time ) || (value >= 3600))
		return 0;
	else  
		return 1;
	     
}

 /*********************************************************
	放电时间控制，
	放电结束返回：0
	未结束返回：  1

**********************************************************/
int dis_charge_timeout(double vol)
{

	if(vol <= conf->lower_voltage){
		delay_ms(1000);
		time_out++;

		if(time_out > 10){
			
			time_out = 0;
			return 0;
		 	
		}
		else
			return 1;
	}	     
	else
		return 1;	

}


/* 充放电结束后放置一小时 */
int delay_hour(void)
{

	FRESULT result;
	FATFS fs;
	FIL file;
	DIR DirInf;  
	uint32_t bw;


	char time_buf[TIME_MAX];
	char write_buf[WR_MAX];
	char tmp_buf[8];

	int sec = 0;
	int i = 0;
	int min_count = 0;
	double vol;


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
	
	result = f_open(&file, conf->file_name, FA_OPEN_ALWAYS | FA_READ | FA_WRITE);	
	if(result !=FR_OK){
		printf("open error, errornu:%d\n\r",result);
	 	goto ERROR;
	}

	if(f_lseek(&file,file.fsize) != FR_OK){
		printf("f_lseek error, errornu:%d\n\r",result);
		goto ERROR;
	}

	
	beeper_on();
	delay_ms(200);
	beeper_off();

	memset(time_buf,0,TIME_MAX);
	memset(write_buf,0,WR_MAX);

	printf("\n\r静置开始");
	Time_Display(RTC_GetCounter());
	printf("\n\r");

	bsp_LedOn(4);

	StartTimer(0,  1000);
//	Time_Display(RTC_GetCounter());
			
	do{
		if(jump)
			goto JMP;	
		
		if(CheckTimer(0) ) {	
			StartTimer(0,  1000);
			sec++;
			min_count ++;

			vol = ADC_SAMPLING();

			if(min_count >= 60){
				min_count = 0;

				record_time(time_buf);
				strcat(time_buf,",");						
	
	
				strcat(write_buf, time_buf);
				memset(time_buf,0,TIME_MAX);		
	
								
				sprintf(tmp_buf, "%.2f", vol);								      
				strcat(write_buf, tmp_buf);
				strcat(write_buf,",静置\n");
	
				if(i >= 9){
					result = f_write(&file,write_buf,sizeof(write_buf),&bw);			
					if(result != FR_OK){
						printf("file write faild, errornu:%d\n\r",result);
						goto ERROR;
					}
					
					memset(write_buf,0,WR_MAX);
					i = 0;
				}
	
				i++;
			}
		}	

	}while(sec < 3600);  /* 静置时间（单位：秒） */

JMP:
	jump = 0;
	if(i != 0){

		result = f_write(&file,write_buf,(15 + TIME_MAX)*i, &bw);			
		if(result != FR_OK){
			printf("file write faild, errornu:%d\n\r",result);
			goto ERROR;
			
		}
		i = 0;
	}
	memset(write_buf,0,WR_MAX);


	printf("静置结束");
	Time_Display(RTC_GetCounter());
	printf("\n\r");

	bsp_LedOff(4);

	f_close(&file);
	f_mount(0, NULL);
	return 0;
	

ERROR:
	bsp_LedOff(4);
	f_close(&file);
	f_mount(0, NULL);
	return 1;
}

/* 记录电池ID，容量等信息，记录在文件头部 */
int show_inf(void)
{
	FRESULT result;
	FATFS fs;
	FIL file;
	DIR DirInf;  
	uint32_t bw;

	char inf_buf[150];
	char tmp_buf[10];


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
	
	result = f_open(&file, conf->file_name, FA_OPEN_ALWAYS | FA_READ | FA_WRITE);	
	if(result !=FR_OK){
		printf("open error, errornu:%d\n\r",result);
	 	goto ERROR;
	}

	if(f_lseek(&file,file.fsize) != FR_OK){
		printf("f_lseek error, errornu:%d\n\r",result);
		goto ERROR;
	}

	

	memset(inf_buf,0,sizeof(inf_buf));

	strcat(inf_buf, "你妹的铅酸蓄电池修复器充放电测试数据\n");
	memset(tmp_buf,0,sizeof(tmp_buf));
	strcat(inf_buf,"电池ID,,");
	sprintf(tmp_buf, "%d", conf->id);
	strcat(inf_buf,tmp_buf);
	strcat(inf_buf,"\n");
	memset(tmp_buf,0,sizeof(tmp_buf));


	strcat(inf_buf,"电池额定容量,,");
	sprintf(tmp_buf, "%d", conf->volume);
	strcat(inf_buf,tmp_buf);
	strcat(inf_buf,"AH\n");
	memset(tmp_buf,0,sizeof(tmp_buf));



	strcat(inf_buf,"电池充电时间,,");	
	sprintf(tmp_buf, "%.1f", conf->charge_time);
	strcat(inf_buf,tmp_buf);
	strcat(inf_buf,"小时\n");
	memset(tmp_buf,0,sizeof(tmp_buf));

	strcat(inf_buf,"充电上限电压,,");	
	sprintf(tmp_buf, "%.1f", conf->upper_voltage);
	strcat(inf_buf,tmp_buf);
	strcat(inf_buf,"V\n");
	memset(tmp_buf,0,sizeof(tmp_buf));


       	strcat(inf_buf,"放电下限电压,,");
	sprintf(tmp_buf, "%.1f",conf->lower_voltage);

	strcat(inf_buf,tmp_buf);
	strcat(inf_buf,"V\n\n");
	memset(tmp_buf,0,sizeof(tmp_buf));

	
	result = f_write(&file, inf_buf, sizeof(inf_buf), &bw);	
	if (result != FR_OK){
	       	printf("File Write Failed, errornu:%d\n\r",result);
		goto ERROR;
	}
	memset(inf_buf,0,sizeof(inf_buf));

	f_close(&file);
	f_mount(0, NULL);
	return 0;

ERROR:
	f_close(&file);
	f_mount(0, NULL);
	return 1;

}

/* 记录当前时间 */
void record_time(char buf[])
{
	char *p; 
	p = buf;

	strcat(p,t_now->year);
	strcat(p,"-");
	strcat(p,t_now->month);
	strcat(p,"-");
	strcat(p,t_now->day);
	strcat(p," ");
	strcat(p,t_now->hour);
	strcat(p,":");
	strcat(p,t_now->min);
	strcat(p,":");
	strcat(p,t_now->sec);


}

/* 将每次的放电时间记录在文件尾部*/
int show_dis_time()	  
{
	FRESULT result;
	FATFS fs;
	FIL file;
	DIR DirInf;  
	uint32_t bw;

	char inf_buf[35];
	char tmp[20];
	int total;


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
	
	result = f_open(&file, conf->file_name, FA_OPEN_ALWAYS | FA_READ | FA_WRITE);	
	if(result !=FR_OK){
		printf("open error, errornu:%d\n\r",result);
	 	goto ERROR;
	}

	if(f_lseek(&file,file.fsize) != FR_OK){
		printf("f_lseek error, errornu:%d\n\r",result);
		goto ERROR;
	}

	memset(inf_buf, 0, sizeof(inf_buf));
	strcat(inf_buf,"不带修复器放电");
	sprintf(tmp, "%d" ,record[0]);	
	strcat(inf_buf, tmp);
	strcat(inf_buf,"分钟\n");

	result = f_write(&file, inf_buf, sizeof(inf_buf), &bw);	
	if (result != FR_OK){
	       	printf("File Write Failed, errornu:%d\n\r",result);
		goto ERROR;
	}

	memset(inf_buf, 0, sizeof(inf_buf));


	for(total = 1; total <= conf->times; total++){
	
		strcat(inf_buf,"带修复器第");
		sprintf(tmp, "%d" ,total);
		strcat(inf_buf, tmp);
		memset(tmp, 0, sizeof(tmp));
		strcat(inf_buf, "次放电");
		sprintf(tmp, "%d" ,record[total]);		
		strcat(inf_buf, tmp);
		strcat(inf_buf,"分钟\n");

		result = f_write(&file, inf_buf, sizeof(inf_buf), &bw);	
		if (result != FR_OK){
		       	printf("File Write Failed, errornu:%d\n\r",result);
			goto ERROR;
		}
		memset(inf_buf, 0, sizeof(inf_buf));

	}

	f_close(&file);
	f_mount(0, NULL);
	return 0;
ERROR:
	f_close(&file);
	f_mount(0, NULL);
	return 1;

	
	
		

}




