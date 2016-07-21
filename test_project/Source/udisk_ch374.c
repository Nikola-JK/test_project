


#include "stm32f10x.h"


#include <stdio.h>
#include <string.h>

#include "bsp_timer.h"
#include "bsp_led.h"

#include "bsp_CH374.h"

#include "main.h"



static void StopIfError(uint8_t _ucErr);
static void flash_to_udisk(void);


void copy_to_udisk(void)
{
	uint32_t i;
	uint8_t ucErr;

	
	led_all_on();
	
	beeper_on();
	delay_ms(200);
	beeper_off();
					/* 初始化CH374相关的GPIO和FSMC */
	bsp_InitCH374();
						/* 检测CH374芯片ID */
	if (ch374_DetectOk())
		;
	//	printf("CH374T Detect Ok\r\n");
	else
		printf("CH374T Detect Failed\r\n");

	delay_ms(200); 				 /* 延时200毫秒 */	

						/* 初始化CH374程序库和CH374芯片,操作成功返回0 */ 
	StopIfError(CH374LibInit());
	Time_Display(RTC_GetCounter());

	while (1) 
	{
		/* 等待插入U盘 */
		printf( "\r\n*******************************************\r\n" );
		printf( "插入U盘\r\n" );

		while (1){
			

			delay_ms(10);  			/* 没必要频繁查询 */
			if (CH374DiskConnect() == ERR_SUCCESS)
				break;  		/* 查询方式: 检查磁盘是否连接并更新磁盘状态,返回成功说明连接 */
			
		}
				
		delay_ms(200);  				/* 延时,可选操作,有的USB存储器需要几十毫秒的延时 */
							/* 检查U盘是否准备好,有些U盘不需要这一步,但是某些U盘必须要执行这一步才能工作 */
		for (i = 0; i < 5; i ++ ){
							/* 有的U盘总是返回未准备好,不过可以被忽略 */
			delay_ms(100);
			if (CH374DiskReady( ) == ERR_SUCCESS)
				break;  		/* 查询磁盘是否准备好 */
			
		}
		
		if (CH374DiskStatus <= DISK_CONNECT)
			continue;  			/* 支持USB-HUB */
							/* 查询磁盘物理容量 */
		{
			uint32_t uiSize = 0;

			ucErr = ch374_DiskSize(&uiSize);
			
			printf( "U盘容量 = %u MB \r\n正在拷贝数据至U盘......\n\r", uiSize >> 11); 	/* 显示为以MB为单位的容量*/
			led_all_off();
			bsp_LedOn(5);
			StopIfError(ucErr);
		}

		flash_to_udisk();			/* 开始拷贝数据 */

		


		/* 等待U盘拔出 */
		printf( "操作完成，拔出U盘\r\n" );
		led_all_on();		
		while ( 1 ) {

			/* 采用查询方式检查磁盘是否连接并更新磁盘状态,返回成功说明连接 */
			/* 支持USB-HUB */
			delay_ms(10);  /* 没必要频繁查询 */
			if (CH374DiskConnect() != ERR_SUCCESS) 
				break;  
		
		}

		break;

	}

}


static void flash_to_udisk(void)
{
	FRESULT result;
	FATFS fs;
	FIL file;
	DIR DirInf;  
	uint32_t bw;

	uint8_t ucErr;

	char write_buf[1024];
	char file_name_buf[20];
	char tmp[10];

	memset(write_buf, 0, 1024);
	memset(file_name_buf, 0, 20);
	memset(tmp, 0, 10);

	strcat(file_name_buf, "/");
	sprintf(tmp, "%d", conf->id);
	strcat(file_name_buf, tmp);
	strcat(file_name_buf, ".TXT");

	result = f_mount(0, &fs);		
	if (result != FR_OK){
		printf("FileSystem Mounted Failed (%d)\r\n", result);
		
		return;
	}
	result = f_opendir(&DirInf, "/");
	if (result != FR_OK){ 
		printf("Open Root Directory Error (%d)\r\n", result);

		return;
	}

	result = f_open(&file, conf->file_name, FA_OPEN_ALWAYS  | FA_READ);	
	if(result !=FR_OK){
		printf("open error \n\r");
	 	return;
	}

		ucErr = ch374_CreateFile(file_name_buf);	/* 新建文件并打开,如果文件已经存在则先删除后再新建 */
		StopIfError(ucErr);
		ch374_ModifyFileDate(atoi(t_now->year), atoi(t_now->month), atoi(t_now->day));
		StopIfError(ucErr);

	  //file.fptr < file.fsize
	while(1){
		
		if(file.fsize - file.fptr < sizeof(write_buf)){
		
			result = f_read(&file, write_buf, file.fsize - file.fptr, &bw);	
			if(result !=FR_OK)
				printf("open error : %d \n\r",result);
			
			ucErr = ch374_WriteFile((uint8_t *)write_buf, sizeof(write_buf));	/* 写数据到U_DISK文件 */
			StopIfError(ucErr);

			memset(write_buf, 0, 1024);
			
			break;
		}else{
	      
			result = f_read(&file, write_buf, sizeof(write_buf), &bw);	//从flash中读到缓冲区	
			if(result !=FR_OK)
				printf("open error : %d \n\r",result);
			ucErr = ch374_WriteFile((uint8_t *)write_buf, sizeof(write_buf));	/* 写数据到U_DISK文件 */
			StopIfError(ucErr);

			memset(write_buf, 0, 1024);

		}

		

      }

	ucErr = ch374_CloseFile();
	StopIfError(ucErr);


	f_close(&file);
 	
	f_mount(0, NULL);	


}

/*
*********************************************************************************************************
*	函 数 名: StopIfError
*	功能说明: 检查操作状态,如果错误则显示错误代码并停机
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/

static void StopIfError(uint8_t _ucErr)
{
	if (_ucErr == ERR_SUCCESS) 
		return; 			 /* 操作成功 */
	
	printf( "Error: %02X\r\n", _ucErr );  /* 显示错误 */
}

