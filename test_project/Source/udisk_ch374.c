


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
					/* ��ʼ��CH374��ص�GPIO��FSMC */
	bsp_InitCH374();
						/* ���CH374оƬID */
	if (ch374_DetectOk())
		;
	//	printf("CH374T Detect Ok\r\n");
	else
		printf("CH374T Detect Failed\r\n");

	delay_ms(200); 				 /* ��ʱ200���� */	

						/* ��ʼ��CH374������CH374оƬ,�����ɹ�����0 */ 
	StopIfError(CH374LibInit());
	Time_Display(RTC_GetCounter());

	while (1) 
	{
		/* �ȴ�����U�� */
		printf( "\r\n*******************************************\r\n" );
		printf( "����U��\r\n" );

		while (1){
			

			delay_ms(10);  			/* û��ҪƵ����ѯ */
			if (CH374DiskConnect() == ERR_SUCCESS)
				break;  		/* ��ѯ��ʽ: �������Ƿ����Ӳ����´���״̬,���سɹ�˵������ */
			
		}
				
		delay_ms(200);  				/* ��ʱ,��ѡ����,�е�USB�洢����Ҫ��ʮ�������ʱ */
							/* ���U���Ƿ�׼����,��ЩU�̲���Ҫ��һ��,����ĳЩU�̱���Ҫִ����һ�����ܹ��� */
		for (i = 0; i < 5; i ++ ){
							/* �е�U�����Ƿ���δ׼����,�������Ա����� */
			delay_ms(100);
			if (CH374DiskReady( ) == ERR_SUCCESS)
				break;  		/* ��ѯ�����Ƿ�׼���� */
			
		}
		
		if (CH374DiskStatus <= DISK_CONNECT)
			continue;  			/* ֧��USB-HUB */
							/* ��ѯ������������ */
		{
			uint32_t uiSize = 0;

			ucErr = ch374_DiskSize(&uiSize);
			
			printf( "U������ = %u MB \r\n���ڿ���������U��......\n\r", uiSize >> 11); 	/* ��ʾΪ��MBΪ��λ������*/
			led_all_off();
			bsp_LedOn(5);
			StopIfError(ucErr);
		}

		flash_to_udisk();			/* ��ʼ�������� */

		


		/* �ȴ�U�̰γ� */
		printf( "������ɣ��γ�U��\r\n" );
		led_all_on();		
		while ( 1 ) {

			/* ���ò�ѯ��ʽ�������Ƿ����Ӳ����´���״̬,���سɹ�˵������ */
			/* ֧��USB-HUB */
			delay_ms(10);  /* û��ҪƵ����ѯ */
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

		ucErr = ch374_CreateFile(file_name_buf);	/* �½��ļ�����,����ļ��Ѿ���������ɾ�������½� */
		StopIfError(ucErr);
		ch374_ModifyFileDate(atoi(t_now->year), atoi(t_now->month), atoi(t_now->day));
		StopIfError(ucErr);

	  //file.fptr < file.fsize
	while(1){
		
		if(file.fsize - file.fptr < sizeof(write_buf)){
		
			result = f_read(&file, write_buf, file.fsize - file.fptr, &bw);	
			if(result !=FR_OK)
				printf("open error : %d \n\r",result);
			
			ucErr = ch374_WriteFile((uint8_t *)write_buf, sizeof(write_buf));	/* д���ݵ�U_DISK�ļ� */
			StopIfError(ucErr);

			memset(write_buf, 0, 1024);
			
			break;
		}else{
	      
			result = f_read(&file, write_buf, sizeof(write_buf), &bw);	//��flash�ж���������	
			if(result !=FR_OK)
				printf("open error : %d \n\r",result);
			ucErr = ch374_WriteFile((uint8_t *)write_buf, sizeof(write_buf));	/* д���ݵ�U_DISK�ļ� */
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
*	�� �� ��: StopIfError
*	����˵��: ������״̬,�����������ʾ������벢ͣ��
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/

static void StopIfError(uint8_t _ucErr)
{
	if (_ucErr == ERR_SUCCESS) 
		return; 			 /* �����ɹ� */
	
	printf( "Error: %02X\r\n", _ucErr );  /* ��ʾ���� */
}

