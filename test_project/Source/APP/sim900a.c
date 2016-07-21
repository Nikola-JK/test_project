/**
	******************************************************************************
	* @file		sim900a.c 
	* @author	JKL
	* @version V0.1
	* @date		13-July-2015
	* @brief	 用SIM900A实现GMS短信收发功能(更改串口后需要更改		#define	SEND USART_SendData(UART5, 0x1A))	
	******************************************************************************
	* @attention
	*
	* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
	* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
	* TIME. AS A RESULT, JKL SHALL NOT BE HELD LIABLE FOR ANY
	* DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
	* FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
	* CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
	*
	* <h2><center>&copy; COPYRIGHT 2015 JKL/center></h2>
	******************************************************************************
	*/ 

/* Includes ------------------------------------------------------------------*/
#include "sim900a.h"
#include "includes.h"
#include "com.h"
/* Private typedef ----------------------------------------------------------*/
/* Private define -----------------------------------------------------------*/
#define	SEND USART_SendData(UART5, 0x1A)

/* Private macro ------------------------------------------------------------*/
/* Private variables --------------------------------------------------------*/
bool FLAG_Abnormal = FALSE;
volatile bool GSM_first;
static char PhoneNumber[] = "00310038003200330031003100390037003800350034";
static char GroupNumber[] = "0031";
extern char AbnormalNumber[];	//异常的电池编号3*4*48=576

/* Global variable used to store variable value in read sequence */
volatile extern FlagStatus GSMsend_OK;
//volatile bool GSM_send;
//volatile bool GSM_Stop;
//volatile bool GSMreceive_flag;
extern volatile uint8_t GSM_stepM;

extern OS_EVENT *GSMSem;
extern OS_EVENT *SendSucceedSem;
/*Virtual address define by the user: 0XFFFF value is prohibited */

/* Private function prototypes ---------------------------------------------*/

/* Private functions -------------------------------------------------------*/



/**
	* @brief	
	*
	* @param	None
	* @retval 
	*/
void sim900a(void)
{
	uint8_t err;
	if(GSM_send) {
				if( strlen(AbnormalNumber) > 120) { //3*4*10=120十块儿异常电池
						GSM_first = FALSE;
						switch(GSM_stepM) {
							case 1:
								printf("AT+CMGF=1\r\n");
								OSTimeDlyHMSM(0, 0, 0, 50);
								break;
							
							case 2: 
								printf("AT+CSMP=17,167,2,25\r\n");
								OSTimeDlyHMSM(0, 0, 0, 50);
								break;
								
							case 3: 
								printf("AT+CSCS=\"UCS2\"\r\n");
								OSTimeDlyHMSM(0, 0, 0, 50);
								break;
							case 4:
								printf("AT+CMGS=\"");
								printf(PhoneNumber);
								printf("\"\r\n");
								OSTimeDlyHMSM(0, 0, 0, 50);
								break;
							case 5:		
								//尊敬的客户，您好！厚光UPS巡检监测系统提醒您：								
								printf("5C0A656C76845BA26237FF0C60A8597DFF01539A51490055005000535DE168C076D16D4B7CFB7EDF63D0919260A8FF1A"); 
								OSTimeDlyHMSM(0, 0, 0, 100);//发太快导致程序异常
								printf("7B2C");					//"第"
								printf(GroupNumber);		//”N“
								printf("7EC4");			//”组“
								OSTimeDlyHMSM(0, 0, 0, 50);
								printf("591A575775356C605F025E38FF0C656C8BF753CA65F668C067E5FF01");	//”多块电池异常，敬请及时检查！”
								OSTimeDlyHMSM(0, 0, 0, 100);
								SEND;
								GSM_stepM = 0;
								break;
								
							default:
								GSM_stepM = 0;
								break;
						}
				}else if( strlen(AbnormalNumber) > 36) { //3*4*3=36三块儿异常电池，大于三块电池异常分两条短信发送。
						switch(GSM_stepM) {
							case 1:
								printf("AT+CMGF=1\r\n");
								OSTimeDlyHMSM(0, 0, 0, 50);
								break;
							
							case 2: 
								printf("AT+CSMP=17,167,2,25\r\n");
								OSTimeDlyHMSM(0, 0, 0, 50);
								break;
								
							case 3: 
								printf("AT+CSCS=\"UCS2\"\r\n");
								OSTimeDlyHMSM(0, 0, 0, 50);
								break;
							case 4:
								printf("AT+CMGS=\"");
								printf(PhoneNumber);
								printf("\"\r\n");
								OSTimeDlyHMSM(0, 0, 0, 50);
								break;
							case 5:		
								if(GSM_first){
									//尊敬的客户，您好！厚光UPS巡检监测系统提醒您：								
									printf("5C0A656C76845BA26237FF0C60A8597DFF01539A51490055005000535DE168C076D16D4B7CFB7EDF63D0919260A8FF1A"); 
									OSTimeDlyHMSM(0, 0, 0, 100);//发太快导致程序异常
									printf("7B2C");					//"第"
									printf(GroupNumber);		//”N“
									printf("7EC47B2C");			//”组第“
									OSTimeDlyHMSM(0, 0, 0, 100);						
									SEND;
									GSM_stepM = 0;			
								}else {
									printf(AbnormalNumber);		//如“03、19……”（异常电阻编号）
									OSTimeDlyHMSM(0, 0, 0, 100);
									printf("53F775356C605F025E38FF0C656C8BF751736CE8FF01");	//”号电池异常，敬请关注！”
									OSTimeDlyHMSM(0, 0, 0, 100);
									SEND;
									GSM_stepM = 0;
									
								}
								break;
								
							default:
								break;
						}
						
				}else{
						GSM_first = FALSE;
						switch(GSM_stepM) {
							case 1:
								printf("AT+CMGF=1\r\n");
								OSTimeDlyHMSM(0, 0, 0, 50);
								break;
							
							case 2: 
								printf("AT+CSMP=17,167,2,25\r\n");
								OSTimeDlyHMSM(0, 0, 0, 50);
								break;
								
							case 3: 
								printf("AT+CSCS=\"UCS2\"\r\n");
								OSTimeDlyHMSM(0, 0, 0, 50);
								break;
							case 4:
								printf("AT+CMGS=\"");
								printf(PhoneNumber);
								printf("\"\r\n");
								OSTimeDlyHMSM(0, 0, 0, 50);
								break;
							case 5:		
								//尊敬的客户，您好！厚光UPS巡检监测系统提醒您：								
								printf("5C0A656C76845BA26237FF0C60A8597DFF01539A51490055005000535DE168C076D16D4B7CFB7EDF63D0919260A8FF1A"); 
								OSTimeDlyHMSM(0, 0, 0, 100);//发太快导致程序异常
								printf("7B2C");					//"第"
								printf(GroupNumber);		//”N“
								printf("7EC47B2C");			//”组第“
								OSTimeDlyHMSM(0, 0, 0, 50);
//								printf(AbnormalNumber);		//如“03、19……”（异常电阻编号）
								OSTimeDlyHMSM(0, 0, 0, 100);
								printf("53F775356C605F025E38FF0C656C8BF751736CE8FF01");	//”号电池异常，敬请关注！”
								OSTimeDlyHMSM(0, 0, 0, 100);
								SEND;	
								OSTimeDlyHMSM(0,0,1,0);
								GSM_stepM = 0;
								break;
								
							default:
								GSM_stepM = 0;
								break;
						}
				}
			}
			
			OSSemPost(GSMSem);		//是GSM任务内循环进行
			if(GSM_Stop){
				OSSemPend(GSMSem, 0, &err);	//停止GSM任务内的循环
				OSSemPost(SendSucceedSem);	//短信发送成功信号量
			}
	
			OSTimeDlyHMSM(0,0,0,500);//延时等待GSM模块相应，太快响应不过来容易出错。
}

void Send_Message(uint8_t num)
{
		char bkp_buf[2] = {0};
		uint16_t B_tmp = 0;
		char num_buf[3] = {0};
		
		switch(num)
		{
				case 1:{
						memset(PhoneNumber, 0, sizeof(PhoneNumber));//清空，防止溢出。
						bkp_buf[1] = (char)(BKP_ReadBackupRegister(Tel_part1) >> 8);
						bkp_buf[0] = (char)(BKP_ReadBackupRegister(Tel_part1) & 0x00ff);
						B_tmp = *(uint16_t*)bkp_buf;
						strcat(PhoneNumber, "003");
						sprintf(num_buf, "%d", B_tmp/1000);
						strcat(PhoneNumber, num_buf);
						strcat(PhoneNumber, "003");
						sprintf(num_buf, "%d", (B_tmp%1000)/100);
						strcat(PhoneNumber, num_buf);
						strcat(PhoneNumber, "003");
						sprintf(num_buf, "%d", (B_tmp%100)/10);
						strcat(PhoneNumber, num_buf);
						strcat(PhoneNumber, "003");
						sprintf(num_buf, "%d", B_tmp%10);
						strcat(PhoneNumber, num_buf);

						bkp_buf[1] = (char)(BKP_ReadBackupRegister(Tel_part2) >> 8);
						bkp_buf[0] = (char)(BKP_ReadBackupRegister(Tel_part2) & 0x00ff);
						B_tmp = *(uint16_t*)bkp_buf;
						strcat(PhoneNumber, "003");
						sprintf(num_buf, "%d", B_tmp/1000);
						strcat(PhoneNumber, num_buf);
						strcat(PhoneNumber, "003");
						sprintf(num_buf, "%d", (B_tmp%1000)/100);
						strcat(PhoneNumber, num_buf);
						strcat(PhoneNumber, "003");
						sprintf(num_buf, "%d", (B_tmp%100)/10);
						strcat(PhoneNumber, num_buf);
						strcat(PhoneNumber, "003");
						sprintf(num_buf, "%d", B_tmp%10);
						strcat(PhoneNumber, num_buf);

						bkp_buf[1] = (char)(BKP_ReadBackupRegister(Tel_part3) >> 8);
						bkp_buf[0] = (char)(BKP_ReadBackupRegister(Tel_part3) & 0x00ff);
						B_tmp = *(uint16_t*)bkp_buf;
						strcat(PhoneNumber, "003");
						sprintf(num_buf, "%d", B_tmp/100);
						strcat(PhoneNumber, num_buf);
						strcat(PhoneNumber, "003");
						sprintf(num_buf, "%d", (B_tmp%100)/10);
						strcat(PhoneNumber, num_buf);
						strcat(PhoneNumber, "003");
						sprintf(num_buf, "%d", B_tmp%10);
						strcat(PhoneNumber, num_buf);
						
						break;
				}
				
				case 2:{
						memset(PhoneNumber, 0, sizeof(PhoneNumber));//清空，防止溢出。
						bkp_buf[1] = (char)(BKP_ReadBackupRegister(Tel2_part1) >> 8);
						bkp_buf[0] = (char)(BKP_ReadBackupRegister(Tel2_part1) & 0x00ff);
						B_tmp = *(uint16_t*)bkp_buf;
						strcat(PhoneNumber, "003");
						sprintf(num_buf, "%d", B_tmp/1000);
						strcat(PhoneNumber, num_buf);
						strcat(PhoneNumber, "003");
						sprintf(num_buf, "%d", (B_tmp%1000)/100);
						strcat(PhoneNumber, num_buf);
						strcat(PhoneNumber, "003");
						sprintf(num_buf, "%d", (B_tmp%100)/10);
						strcat(PhoneNumber, num_buf);
						strcat(PhoneNumber, "003");
						sprintf(num_buf, "%d", B_tmp%10);
						strcat(PhoneNumber, num_buf);

						bkp_buf[1] = (char)(BKP_ReadBackupRegister(Tel2_part2) >> 8);
						bkp_buf[0] = (char)(BKP_ReadBackupRegister(Tel2_part2) & 0x00ff);
						B_tmp = *(uint16_t*)bkp_buf;
						strcat(PhoneNumber, "003");
						sprintf(num_buf, "%d", B_tmp/1000);
						strcat(PhoneNumber, num_buf);
						strcat(PhoneNumber, "003");
						sprintf(num_buf, "%d", (B_tmp%1000)/100);
						strcat(PhoneNumber, num_buf);
						strcat(PhoneNumber, "003");
						sprintf(num_buf, "%d", (B_tmp%100)/10);
						strcat(PhoneNumber, num_buf);
						strcat(PhoneNumber, "003");
						sprintf(num_buf, "%d", B_tmp%10);
						strcat(PhoneNumber, num_buf);

						bkp_buf[1] = (char)(BKP_ReadBackupRegister(Tel2_part3) >> 8);
						bkp_buf[0] = (char)(BKP_ReadBackupRegister(Tel2_part3) & 0x00ff);
						B_tmp = *(uint16_t*)bkp_buf;
						strcat(PhoneNumber, "003");
						sprintf(num_buf, "%d", B_tmp/100);
						strcat(PhoneNumber, num_buf);
						strcat(PhoneNumber, "003");
						sprintf(num_buf, "%d", (B_tmp%100)/10);
						strcat(PhoneNumber, num_buf);
						strcat(PhoneNumber, "003");
						sprintf(num_buf, "%d", B_tmp%10);
						strcat(PhoneNumber, num_buf);
						break;				
				}
				
				case 3:{
						memset(PhoneNumber, 0, sizeof(PhoneNumber));//清空，防止溢出。
						bkp_buf[1] = (char)(BKP_ReadBackupRegister(Tel3_part1) >> 8);
						bkp_buf[0] = (char)(BKP_ReadBackupRegister(Tel3_part1) & 0x00ff);
						B_tmp = *(uint16_t*)bkp_buf;
						strcat(PhoneNumber, "003");
						sprintf(num_buf, "%d", B_tmp/1000);
						strcat(PhoneNumber, num_buf);
						strcat(PhoneNumber, "003");
						sprintf(num_buf, "%d", (B_tmp%1000)/100);
						strcat(PhoneNumber, num_buf);
						strcat(PhoneNumber, "003");
						sprintf(num_buf, "%d", (B_tmp%100)/10);
						strcat(PhoneNumber, num_buf);
						strcat(PhoneNumber, "003");
						sprintf(num_buf, "%d", B_tmp%10);
						strcat(PhoneNumber, num_buf);

						bkp_buf[1] = (char)(BKP_ReadBackupRegister(Tel3_part2) >> 8);
						bkp_buf[0] = (char)(BKP_ReadBackupRegister(Tel3_part2) & 0x00ff);
						B_tmp = *(uint16_t*)bkp_buf;
						strcat(PhoneNumber, "003");
						sprintf(num_buf, "%d", B_tmp/1000);
						strcat(PhoneNumber, num_buf);
						strcat(PhoneNumber, "003");
						sprintf(num_buf, "%d", (B_tmp%1000)/100);
						strcat(PhoneNumber, num_buf);
						strcat(PhoneNumber, "003");
						sprintf(num_buf, "%d", (B_tmp%100)/10);
						strcat(PhoneNumber, num_buf);
						strcat(PhoneNumber, "003");
						sprintf(num_buf, "%d", B_tmp%10);
						strcat(PhoneNumber, num_buf);

						bkp_buf[1] = (char)(BKP_ReadBackupRegister(Tel3_part3) >> 8);
						bkp_buf[0] = (char)(BKP_ReadBackupRegister(Tel3_part3) & 0x00ff);
						B_tmp = *(uint16_t*)bkp_buf;
						strcat(PhoneNumber, "003");
						sprintf(num_buf, "%d", B_tmp/100);
						strcat(PhoneNumber, num_buf);
						strcat(PhoneNumber, "003");
						sprintf(num_buf, "%d", (B_tmp%100)/10);
						strcat(PhoneNumber, num_buf);
						strcat(PhoneNumber, "003");
						sprintf(num_buf, "%d", B_tmp%10);
						strcat(PhoneNumber, num_buf);
						break;				
				}
				default:
						break;
		}
		
		GSMreceive_flag	= TRUE; //打开接收			
		GSM_send = TRUE;	//发短信
		GSM_Stop = FALSE;
		GSM_first = TRUE;
		OSSemPost(GSMSem);	//发短信
		
}
