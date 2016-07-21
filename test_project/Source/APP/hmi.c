/**
  ******************************************************************************
  * @file    hmi.c 
  * @author  JKL
  * @version V0.1
  * @date    16-July-2015
  * @brief   �˻���������
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
#include "hmi.h"
#include "includes.h"
#include "hmi_driver.h"
#include "com.h"
#include "app.h"
/* Private typedef ----------------------------------------------------------*/
/* Private define -----------------------------------------------------------*/


/* Private macro ------------------------------------------------------------*/
/* Private variables --------------------------------------------------------*/


/* Global variable used to store variable value in read sequence */
extern bool R_unwork;//������ʧЧ��־λ
extern float ParaAll[4][48];	//48��ͨ����ÿ��4������

extern OS_EVENT *CalSem;//������AppTaskT_VSample

/*�����쳣��ؽ���*/
uint16_t AN_temp = 0,AN_num = 0;//AbnormalNumber
uint8_t AN_array[36] = {0};			//�쳣�������
volatile int8_t AN_channel = 0;//�쳣��ر��



/*��������������Ľ���*/
volatile bool FLAG_Password = FALSE;
volatile bool FLAG_Changeword = FALSE;
volatile bool FLAG_affirmword = FALSE;
unsigned char Npassword_buffer[10];	 //New passworld ������
unsigned char Affpassword_buffer[10];//affirm passworld ����ȷ��


/*��������У׼���溯��*/	
volatile uint16_t Rc = 0; //RУ׼ֵ
volatile bool FLAG_36channle = FALSE;
volatile bool FLAG_rCc = FALSE;	//������У׼����
volatile uint8_t Channel_i = 0;	//У׼����ȫ��У׼ͨ���ۼ�
volatile uint8_t channel = 1;		//У׼����ָ��ͨ��У׼�����У׼��ť��ˢ�½�����ʾͨ����
volatile bool FLAG_Cal_Over = TRUE;//����У׼���
extern bool R_calibration_OK;
volatile bool FLAG_CPassword = FALSE;//У׼Ȩ������



/*�����������ú����ñ������������*/	
volatile 	bool FLAG_B_V = FALSE;	//���õ�ѹ���ñ�־λ��������水ť��ˢ�±������ݣ�û����λ�Ĳ�����
volatile 	bool FLAG_B_C = FALSE;
volatile 	bool FLAG_B_N = FALSE;
volatile 	bool FLAG_Up_V = FALSE;
volatile	bool FLAG_Lo_V = FALSE;
volatile	bool FLAG_Up_T = FALSE;
volatile	bool FLAG_Up_R = FALSE;
volatile	bool FLAG_Up_C = FALSE;	
volatile 	bool FLAG_Tel = FALSE;
char B_V_buffer[8]= {0};					//���õ�ѹ���ݴ��ڴ����飬������水ť��ˢ�±�������
char B_C_buffer[8]= {0};
char B_N_buffer[8]= {0};
char Up_V_buffer[8]= {0};
char Lo_V_buffer[8]= {0};
char Up_T_buffer[8]= {0};
char Up_R_buffer[8]= {0};
char Lo_C_buffer[8]= {0};
char Tel_buffer[11]= {0};//���ü���ߵ绰���룬������水ť��ˢ�±�������	
uint8_t Telnum = 1;
volatile bool FLAG_Set_Refresh = TRUE;	//���ý�����±�׼����û������������һ���˳����ڽ��벻֪����û��ɹ���


volatile bool FLAG_Setif = TRUE;	//ˢ�²�����Ϣ��־λ

/*Virtual address define by the user: 0XFFFF value is prohibited */

/* Private function prototypes ---------------------------------------------*/
extern uint16_t  ParaCal(uint8_t p, uint8_t NumChannel);	//����У׼����

/* Private functions -------------------------------------------------------*/



/**
	* @brief	�쳣��ؽ���
	*
	* @param	com_buffer
	* @retval None
	*/
void Abnor_BatteryIF(unsigned char *com_buffer)
{
		char temp_buf[10] = {0};
		uint16_t Tel_temp;

		if(com_buffer[6] == 0X20) { //�·�
			AN_channel++;
			if(AN_channel > AN_num-1){
				AN_channel = 0;
			}
				AN_temp = AN_array[AN_channel];
				sprintf(temp_buf, "%02d", AN_array[AN_channel]);
				SetTextValue(26, 20, (unsigned char*)temp_buf); //���
				sprintf(temp_buf, "%.2f", ParaAll[0][AN_temp-1]);
				SetTextValue(26, 10, (unsigned char*)temp_buf);	//��ѹ
				sprintf(temp_buf, "%.3f", ParaAll[2][AN_temp-1]);
				SetTextValue(26, 11, (unsigned char*)temp_buf); //����
				sprintf(temp_buf, "%.2f", ParaAll[3][AN_temp-1]);
				SetTextValue(26, 12, (unsigned char*)temp_buf); //����
				sprintf(temp_buf, "%.2f", ParaAll[1][AN_temp-1]);
				SetTextValue(26, 13, (unsigned char*)temp_buf);	//�¶�
				Tel_temp = (uint16_t)ParaAll[3][AN_temp-1];
				if((ParaAll[3][AN_temp-1]-Tel_temp) >= 0.5){
							Tel_temp++;
						}
				sprintf(temp_buf, "%3d",Tel_temp );
				SetTextValue(26, 34, (unsigned char*)temp_buf); //�����ٷֱ�
				
				if(ParaAll[3][AN_temp-1]>80){			
					SetProgressValue(26,27,ParaAll[3][AN_temp-1]);
				}else if(ParaAll[3][AN_temp-1]>20){
								SetProgressValue(26,27,0);
								SetProgressValue(26,26,ParaAll[3][AN_temp-1]);
							}else{
								SetProgressValue(26,27,0);
								SetProgressValue(26,26,0);
								SetProgressValue(26,1,ParaAll[3][AN_temp-1]);
							}			
		}
		if(com_buffer[6] == 0X1F) { //�Ϸ�
			AN_channel--;
			if(AN_channel < 0){
				AN_channel = AN_num-1;
			}	
				AN_temp = AN_array[AN_channel];
				sprintf(temp_buf, "%02d", AN_array[AN_channel]);
				SetTextValue(26, 20, (unsigned char*)temp_buf); //���
				sprintf(temp_buf, "%.2f", ParaAll[0][AN_temp-1]);
				SetTextValue(26, 10, (unsigned char*)temp_buf);	//��ѹ
				sprintf(temp_buf, "%.3f", ParaAll[2][AN_temp-1]);
				SetTextValue(26, 11, (unsigned char*)temp_buf); //����
				sprintf(temp_buf, "%.2f", ParaAll[3][AN_temp-1]);
				SetTextValue(26, 12, (unsigned char*)temp_buf); //����
				sprintf(temp_buf, "%.2f", ParaAll[1][AN_temp-1]);
				SetTextValue(26, 13, (unsigned char*)temp_buf);	//�¶�
				Tel_temp = (uint16_t)ParaAll[3][AN_temp-1];
				if((ParaAll[3][AN_temp-1]-Tel_temp) >= 0.5){
							Tel_temp++;
						}
				sprintf(temp_buf, "%3d",Tel_temp );
				SetTextValue(26, 34, (unsigned char*)temp_buf); //�����ٷֱ�
				
				if(ParaAll[3][AN_temp-1]>80){			
					SetProgressValue(26,27,ParaAll[3][AN_temp-1]);
				}else if(ParaAll[3][AN_temp-1]>20){
								SetProgressValue(26,27,0);
								SetProgressValue(26,26,ParaAll[3][AN_temp-1]);
							}else{
								SetProgressValue(26,27,0);
								SetProgressValue(26,26,0);
								SetProgressValue(26,1,ParaAll[3][AN_temp-1]);
							}	
		}
}

/**
	* @brief	У׼Ȩ���������
	*
	* @param	com_buffer
	* @retval None
	*/
void Calib_PassWorldIF(unsigned char *com_buffer)
{
		unsigned char value_bufB[6] = "******"; 
		unsigned char value_Clear[1] = ""; 
		
	//Ȩ�����������
		if(com_buffer[6] == 0X03) { 
			if((com_buffer[8] == 0X68) && (com_buffer[9] == 0X6F) && (com_buffer[10] == 0X70) \
				&& (com_buffer[11] == 0X65) && (com_buffer[12] == 0X20) && (com_buffer[13] == 0X67) \
				&& (com_buffer[14] == 0X72) && (com_buffer[15] == 0X61) && (com_buffer[16] == 0X6E) && (com_buffer[17] == 0X64)) {	
				FLAG_CPassword = TRUE;
			}
			SetTextValue(24, 3, (unsigned char*)value_bufB);
		}
		//ȷ��
		if(com_buffer[6] == 0X08) {
				if(FLAG_CPassword){
						SetScreen(22);		//����У׼����
						FLAG_CPassword = FALSE;
				}else {
						SetScreen(25);		//Ȩ�������������
						SetTextValue(24, 3, (unsigned char*)value_Clear);
				}
		}
}

/**
	* @brief	����У׼����
	*
	* @param	com_buffer
	* @retval None
	*/
void Param_CalibrationIF(unsigned char *com_buffer)
{
		uint8_t err; 
		volatile uint8_t i = 0;  
		uint16_t Rv = 0;
		uint16_t Ri = 0;
		uint16_t Rk = 0;
		unsigned char VC_buf[] = "��ѹУ׼ģʽ"; 
		unsigned char TC_buf[] = "�¶�У׼ģʽ";
		unsigned char CC_buf[] = "����У׼ģʽ";
		unsigned char RC_buf[] = "����У׼ģʽ";
		unsigned char CV_buf[] = "������У׼ֵ:";
		unsigned char Channel_buf[] = "ͨ����";
		unsigned char AD_buf[] = "����AD�룺";
		unsigned char Finish_buf[] = "����У׼���!";
		unsigned char value_Clear[1] = ""; 
		char R_buffer[8]= {0};
		char temp_buf[10] = {0};
	
		// ����
		if((com_buffer[6] == 0X09) || (com_buffer[6] == 0X16)) {
				SetButtonValue(22, 3, 0);			//�ر�����ѡ��	
				SetButtonValue(22, 4, 0);
				SetButtonValue(22, 5, 0);
				SetButtonValue(22, 6, 0);
				SetTextValue(22, 10, (unsigned char*)value_Clear);
				SetTextValue(22, 15, (unsigned char*)value_Clear);
				SetTextValue(22, 16, (unsigned char*)value_Clear);
				SetTextValue(22, 17, (unsigned char*)value_Clear);
				SetTextValue(22, 18, (unsigned char*)value_Clear);
				SetTextValue(22, 19, (unsigned char*)value_Clear);
				SetTextValue(22, 20, (unsigned char*)value_Clear);
				SetTextValue(22, 11, (unsigned char*)value_Clear);
				SetTextValue(22, 12, (unsigned char*)value_Clear);
				if(!FLAG_Cal_Over){
					FLAG_Cal_Over = TRUE;
					OSSemPost(CalSem);	//������AppTaskT_VSample
					R_calibration_OK = TRUE;	//��������AppTaskT_VSample�йرռ���ͨ��
				}	
		}
		// ��ѹУ׼ѡ��
		if((com_buffer[6] == 0X03) && (com_buffer[9] == 0X01)) { 
				SetButtonValue(22, 4, 0);//�ر�����ѡ��
				SetButtonValue(22, 5, 0);
				SetButtonValue(22, 6, 0);
				SetTextValue(22, 10, (unsigned char*)VC_buf);
				SetTextValue(22, 15, (unsigned char*)CV_buf);
				SetTextValue(22, 16, (unsigned char*)value_Clear);
				SetTextValue(22, 17, (unsigned char*)value_Clear);
				SetTextValue(22, 18, (unsigned char*)value_Clear);
				SetTextValue(22, 19, (unsigned char*)value_Clear);
				SetTextValue(22, 20, (unsigned char*)value_Clear);
				SetTextValue(22, 11, (unsigned char*)value_Clear);
				SetTextValue(22, 12, (unsigned char*)value_Clear);
				if(!FLAG_Cal_Over){
					FLAG_Cal_Over = TRUE;
					OSSemPost(CalSem);	//������AppTaskT_VSample
					R_calibration_OK = TRUE;	//��������AppTaskT_VSample�йرռ���ͨ��
				}	
		}
		// �¶�У׼ѡ��
		if((com_buffer[6] == 0X04) && (com_buffer[9] == 0X01)) { 
				SetButtonValue(22, 3, 0);//�ر�����ѡ��
				SetButtonValue(22, 5, 0);
				SetButtonValue(22, 6, 0);
				SetTextValue(22, 10, (unsigned char*)TC_buf);
				SetTextValue(22, 15, (unsigned char*)CV_buf);
				SetTextValue(22, 16, (unsigned char*)value_Clear);
				SetTextValue(22, 17, (unsigned char*)value_Clear);
				SetTextValue(22, 18, (unsigned char*)value_Clear);
				SetTextValue(22, 19, (unsigned char*)value_Clear);
				SetTextValue(22, 20, (unsigned char*)value_Clear);
				SetTextValue(22, 11, (unsigned char*)value_Clear);
				SetTextValue(22, 12, (unsigned char*)value_Clear);	
				if(!FLAG_Cal_Over){
					FLAG_Cal_Over = TRUE;
					OSSemPost(CalSem);	//������AppTaskT_VSample
					R_calibration_OK = TRUE;	//��������AppTaskT_VSample�йرռ���ͨ��
				}	
		}
		// ����У׼ѡ��
		if((com_buffer[6] == 0X06) && (com_buffer[9] == 0X01)) { 
				SetButtonValue(22, 3, 0);//�ر�����ѡ��
				SetButtonValue(22, 4, 0);
				SetButtonValue(22, 5, 0);
				SetTextValue(22, 10, (unsigned char*)CC_buf);
				SetTextValue(22, 15, (unsigned char*)CV_buf);
				SetTextValue(22, 16, (unsigned char*)value_Clear);
				SetTextValue(22, 17, (unsigned char*)value_Clear);
				SetTextValue(22, 18, (unsigned char*)value_Clear);
				SetTextValue(22, 19, (unsigned char*)value_Clear);
				SetTextValue(22, 20, (unsigned char*)value_Clear);
				SetTextValue(22, 11, (unsigned char*)value_Clear);
				SetTextValue(22, 12, (unsigned char*)value_Clear);		
				FLAG_rCc = TRUE;
				if(!FLAG_Cal_Over){
					FLAG_Cal_Over = TRUE;
					OSSemPost(CalSem);	//������AppTaskT_VSample
					R_calibration_OK = TRUE;	//��������AppTaskT_VSample�йرռ���ͨ��
				}		
		}
		// ����У׼ѡ��
		if((com_buffer[6] == 0X05) && (com_buffer[9] == 0X01)) { 
				SetButtonValue(22, 3, 0);//�ر�����ѡ��
				SetButtonValue(22, 4, 0);
				SetButtonValue(22, 6, 0);
				SetTextValue(22, 10, (unsigned char*)RC_buf);
				SetTextValue(22, 15, (unsigned char*)CV_buf);
				SetTextValue(22, 16, (unsigned char*)value_Clear);
				SetTextValue(22, 17, (unsigned char*)value_Clear);
				SetTextValue(22, 18, (unsigned char*)value_Clear);
				SetTextValue(22, 19, (unsigned char*)value_Clear);
				SetTextValue(22, 20, (unsigned char*)value_Clear);
				SetTextValue(22, 11, (unsigned char*)value_Clear);
				SetTextValue(22, 12, (unsigned char*)value_Clear);		
				FLAG_rCc = FALSE;
				
				FLAG_Cal_Over = FALSE;
				R_calibration_OK = FALSE;		//��ֹ����AppTaskT_VSample�йرռ���ͨ��
				OSSemPend(CalSem, 0, &err); //�ر�����AppTaskT_VSample
		}
		// ͨ������
		if((com_buffer[6] == 0X0B)) {	
				/*ָ��ͨ��У׼*/
				if(com_buffer[8] != 0x00){	
//						ChannelAllDisable();
						OSTimeDlyHMSM(0, 0, 0, 100);	
						channel = (com_buffer[8]-48)*10+(com_buffer[9]-48);
						Channel_Select(channel);
						OSTimeDlyHMSM(0, 0, 0, 100);
						SetTextValue(22, 16, (unsigned char*)Channel_buf);
						sprintf(temp_buf, "%d", channel);
						SetTextValue(22, 20, (unsigned char*)temp_buf);
						FLAG_36channle = FALSE;
				}else {
						FLAG_36channle = TRUE;
				}
		}
		//У׼ֵ����
		if((com_buffer[6] == 0X0C)) {		
				if( (com_buffer[9] == 0X2E) || (com_buffer[10] == 0X2E) || (com_buffer[11] == 0X2E) ){ //0X2E"."��ACSII�룬ȷ��С����λ��
					for(i=8;com_buffer[i] != 0X2E;i++){
						R_buffer[i-8] = com_buffer[i];
					}
					if(com_buffer[i+2] == 0x00){	//С�������һλ��
						R_buffer[i-8] = com_buffer[i+1];
						R_buffer[i-7] = 0x30;	//��һλ0�����湫ʽ��1000
						R_buffer[i-6] = 0x30;
						R_buffer[i-5] = 0x00;	//�ַ�����β��־
					}else{	//С���������λ��
						R_buffer[i-8] = com_buffer[i+1];
						R_buffer[i-7] = com_buffer[i+2];
						R_buffer[i-6] = 0x30;
						R_buffer[i-5] = 0x00;	//�ַ�����β��־
					}
				}else{	//��С���㣬����Ϊ����
					for(i=8;com_buffer[i] != 0X00;i++){
						R_buffer[i-8] = com_buffer[i];
						}
						R_buffer[i-8] = 0x30;	
						R_buffer[i-7] = 0x30;
						R_buffer[i-6] = 0x30;
						R_buffer[i-5] = 0x00;	//�ַ�����β��־					
					}	
					Rc = atoi(R_buffer);
		//						printf("Rc = %d \r\n",Rc);
		}
		//У׼��ť		
		if((com_buffer[6] == 0X0D)) {
			
			SetTextValue(22, 17, (unsigned char*)AD_buf);
			if(FLAG_rCc){
					EE_WriteVariable(R_Cal_C_Addr, Rc); //������У׼����	
					FLAG_rCc = FALSE;			
					sprintf(temp_buf, "%d", Rc);
					SetTextValue(22, 19, (unsigned char*)temp_buf);
					SetTextValue(22, 18, (unsigned char*)Finish_buf);						
					beep(100);
			}else if(!FLAG_36channle){					
						/*ͨ��NУ׼*/					
							if(R_unwork) {
							Rk = (uint16_t)Rc;
						}else {
							Rk = (uint16_t)( Rc / r_value[channel] );
						}
							EE_WriteVariable(Channel1_r_k_Addr + channel-1, Rk);

							sprintf(temp_buf, "%d", Rk);
							SetTextValue(22, 19, (unsigned char*)temp_buf);
							SetTextValue(22, 18, (unsigned char*)Finish_buf);
						
							//У׼������ˢ����ֵ
						  ParaAll[2][channel-1] = (float)Rc/1000;						
							Show((channel-1)/4);		
														
							beep(10);
							
							FLAG_Cal_Over = TRUE;
							OSSemPost(CalSem);	//������AppTaskT_VSample
							GPIO_SetBits(GPIOC, GPIO_Pin_4);	//�ر�����弤��ͨ��
							R_calibration_OK = TRUE;	//��������AppTaskT_VSample�йرռ���ͨ��						
				/*ȫ����˳��У׼*/		
				}else{
		//						printf("Channel_i = %d \r\n",Channel_i+1);
					if(Channel_i<36) {
//							ChannelAllDisable();
							OSTimeDlyHMSM(0, 0, 0, 100);	
							Channel_Select(Channel_i + 1);
							OSTimeDlyHMSM(0, 0, 1, 0);
							
							SetTextValue(22, 16, (unsigned char*)Channel_buf);
							sprintf(temp_buf, "%d", Channel_i+1);
							SetTextValue(22, 20, (unsigned char*)temp_buf);
													
							if(R_unwork) {
								Rk = (uint16_t)Rc ;
							}else {
								Rv = ParaCal(2, Channel_i+1);
								Ri = ParaCal(3, Channel_i+1);
								Rk = (uint16_t)( Rc * ( (float)Ri / (float)Rv ) );
							}
								EE_WriteVariable(Channel1_r_k_Addr + Channel_i, Rk);

								sprintf(temp_buf, "%d", Rk);
								SetTextValue(22, 19, (unsigned char*)temp_buf);
															
								beep(100);
								Channel_i++;
					}
				if(Channel_i == 36){
					Channel_i = 0;
					SetTextValue(22, 18, (unsigned char*)Finish_buf);
					
					FLAG_Cal_Over = TRUE;
					OSSemPost(CalSem);	//������AppTaskT_VSample
					R_calibration_OK = TRUE;	//��������AppTaskT_VSample�йرռ���ͨ��
				}				
			}		
			
		}

}
	

/**
	* @brief	������Ľ���
	*
	* @param	com_buffer
	* @retval None
	*/
void PassW_ChangeIF(unsigned char *com_buffer)
{
		volatile uint8_t i = 0;  
		uint16_t Tel_temp;
		unsigned char value_bufB[6] = "******"; 
		unsigned char value_Clear[1] = ""; 
		
		//ԭʼ���룺Hope Grand
		if(com_buffer[6] == 0X0B) {   //ԭʼ���������
			if((com_buffer[8] == 0X48) && (com_buffer[9] == 0X6F) && (com_buffer[10] == 0X70) \
				&& (com_buffer[11] == 0X65) && (com_buffer[12] == 0X20) && (com_buffer[13] == 0X47) \
				&& (com_buffer[14] == 0X72) && (com_buffer[15] == 0X61) && (com_buffer[16] == 0X6E) && (com_buffer[17] == 0X64)) {
				FLAG_Changeword = TRUE;	
			}
				SetTextValue(18, 11, (unsigned char*)value_bufB);
		}
		//������
		if(com_buffer[6] == 0X0C) { //�����������
			for(i = 0;i < 10;i++) {
				Npassword_buffer[i] = com_buffer[8+i];
			}
			SetTextValue(18, 12, (unsigned char*)value_bufB);
		}
		//������ȷ��
		if(com_buffer[6] == 0X0D) {	//������ȷ�������
			for(i = 0;i < 10;i++) {
				Affpassword_buffer[i] = com_buffer[8+i];
			}
			if((Affpassword_buffer[0]==Npassword_buffer[0]) && (Affpassword_buffer[1]==Npassword_buffer[1]) && (Affpassword_buffer[2]==Npassword_buffer[2]) \
				 && (Affpassword_buffer[3]==Npassword_buffer[3]) && (Affpassword_buffer[4]==Npassword_buffer[4]) && (Affpassword_buffer[5]==Npassword_buffer[5])\
					&& (Affpassword_buffer[6]==Npassword_buffer[6]) && (Affpassword_buffer[7]==Npassword_buffer[7]) && (Affpassword_buffer[8]==Npassword_buffer[8])\
					 && (Affpassword_buffer[9]==Npassword_buffer[9])&& (Affpassword_buffer[9] !=0X00)) {	//��������������ȷ����ͬ,�ҵ�10λ���գ�����ʮλ����
				FLAG_affirmword = TRUE;				
			}
				SetTextValue(18, 13, (unsigned char*)value_bufB);
		}
		//ȷ��
		if((com_buffer[6] == 0X09) && (com_buffer[7] == 0X10) && (com_buffer[8] == 0X02) \
				&& (com_buffer[9] == 0X61)) {
				if(!FLAG_Changeword){
					SetScreen(20);		//ԭʼ�����������
				}else if(!FLAG_affirmword) {
								SetScreen(21);		//����ȷ���������
							}else {
								SetScreen(19);		//���������ɽ���
								FLAG_Changeword = FALSE;
								FLAG_affirmword = FALSE;
								SetTextValue(18, 11, (unsigned char*)value_Clear);
								SetTextValue(18, 12, (unsigned char*)value_Clear);
								SetTextValue(18, 13, (unsigned char*)value_Clear);
								//���뱣��
								PWR_BackupAccessCmd(ENABLE); //ȡ�����ݼĴ���д����
//									for(i = 0; i < 10; i++) {
//										Tel_temp = (uint16_t)Affpassword_buffer[i];
//										EE_WriteVariable(Password_Addr + i, Tel_temp);
//									}	
								Tel_temp = (uint16_t)Affpassword_buffer[0];
								BKP_WriteBackupRegister(PW_part1,  Tel_temp);
								Tel_temp = (uint16_t)Affpassword_buffer[1];
								BKP_WriteBackupRegister(PW_part2,  Tel_temp);	
								Tel_temp = (uint16_t)Affpassword_buffer[2];
								BKP_WriteBackupRegister(PW_part3,  Tel_temp);	
								Tel_temp = (uint16_t)Affpassword_buffer[3];
								BKP_WriteBackupRegister(PW_part4,  Tel_temp);	
								Tel_temp = (uint16_t)Affpassword_buffer[4];
								BKP_WriteBackupRegister(PW_part5,  Tel_temp);	
								Tel_temp = (uint16_t)Affpassword_buffer[5];
								BKP_WriteBackupRegister(PW_part6,  Tel_temp);	
								Tel_temp = (uint16_t)Affpassword_buffer[6];
								BKP_WriteBackupRegister(PW_part7,  Tel_temp);	
								Tel_temp = (uint16_t)Affpassword_buffer[7];
								BKP_WriteBackupRegister(PW_part8,  Tel_temp);	
								Tel_temp = (uint16_t)Affpassword_buffer[8];
								BKP_WriteBackupRegister(PW_part9,  Tel_temp);	
								Tel_temp = (uint16_t)Affpassword_buffer[9];
								BKP_WriteBackupRegister(PW_part10,  Tel_temp);	
								
								GPIO_SetBits(GPIOA, GPIO_Pin_1);
								OSTimeDlyHMSM(0, 0, 0, 100);
								GPIO_ResetBits(GPIOA, GPIO_Pin_1);
								OSTimeDlyHMSM(0, 0, 0, 100);
								PWR_BackupAccessCmd(DISABLE); //���ݼĴ���д����
							}
				SetTextValue(18, 11, (unsigned char*)value_Clear);
				SetTextValue(18, 12, (unsigned char*)value_Clear);
				SetTextValue(18, 13, (unsigned char*)value_Clear);
		}
			
}

/**
	* @brief	�����������
	*
	* @param	com_buffer
	* @retval None
	*/
void PassW_InputIF(unsigned char *com_buffer)
{
		char bkp_buf[2] = {0};
		uint16_t Password[10] = {0};
		unsigned char value_bufB[6] = "******"; 
		unsigned char value_Clear[1] = ""; 
		
				//��ȡ����
//		 for(i = 0; i < 10; i++) {
//        EE_ReadVariable(Password_Addr + i, Password + i);
////        printf("%d",Password[i]);
//        /*���δ����У׼����������Ĭ��ֵ*/
//        if(Password[i] == 0X00) {
//           Password[i] = 0X30;
//        }
//			}		 
				bkp_buf[1] = (char)(BKP_ReadBackupRegister(PW_part1) >> 8);
        bkp_buf[0] = (char)(BKP_ReadBackupRegister(PW_part1) & 0x00ff);
        Password[0] = *(uint16*)bkp_buf;
				bkp_buf[1] = (char)(BKP_ReadBackupRegister(PW_part2) >> 8);
        bkp_buf[0] = (char)(BKP_ReadBackupRegister(PW_part2) & 0x00ff);
        Password[1] = *(uint16*)bkp_buf;
				bkp_buf[1] = (char)(BKP_ReadBackupRegister(PW_part3) >> 8);
        bkp_buf[0] = (char)(BKP_ReadBackupRegister(PW_part3) & 0x00ff);
        Password[2] = *(uint16*)bkp_buf;
				bkp_buf[1] = (char)(BKP_ReadBackupRegister(PW_part4) >> 8);
        bkp_buf[0] = (char)(BKP_ReadBackupRegister(PW_part4) & 0x00ff);
        Password[3] = *(uint16*)bkp_buf;
				bkp_buf[1] = (char)(BKP_ReadBackupRegister(PW_part5) >> 8);
        bkp_buf[0] = (char)(BKP_ReadBackupRegister(PW_part5) & 0x00ff);
        Password[4] = *(uint16*)bkp_buf;
				bkp_buf[1] = (char)(BKP_ReadBackupRegister(PW_part6) >> 8);
        bkp_buf[0] = (char)(BKP_ReadBackupRegister(PW_part6) & 0x00ff);
        Password[5] = *(uint16*)bkp_buf;
				bkp_buf[1] = (char)(BKP_ReadBackupRegister(PW_part7) >> 8);
        bkp_buf[0] = (char)(BKP_ReadBackupRegister(PW_part7) & 0x00ff);
        Password[6] = *(uint16*)bkp_buf;
				bkp_buf[1] = (char)(BKP_ReadBackupRegister(PW_part8) >> 8);
        bkp_buf[0] = (char)(BKP_ReadBackupRegister(PW_part8) & 0x00ff);
        Password[7] = *(uint16*)bkp_buf;
				bkp_buf[1] = (char)(BKP_ReadBackupRegister(PW_part9) >> 8);
        bkp_buf[0] = (char)(BKP_ReadBackupRegister(PW_part9) & 0x00ff);
        Password[8] = *(uint16*)bkp_buf;
				bkp_buf[1] = (char)(BKP_ReadBackupRegister(PW_part10) >> 8);
        bkp_buf[0] = (char)(BKP_ReadBackupRegister(PW_part10) & 0x00ff);
        Password[9] = *(uint16*)bkp_buf;
				 
				
			if(com_buffer[6] == 0X03) { //�����
				if((com_buffer[8] == Password[0]) && (com_buffer[9] == Password[1]) && (com_buffer[10] == Password[2]) \
					&& (com_buffer[11] == Password[3]) && (com_buffer[12] == Password[4]) && (com_buffer[13] == Password[5]) \
					&& (com_buffer[14] == Password[6]) && (com_buffer[15] == Password[7]) && (com_buffer[16] == Password[8]) && (com_buffer[17] == Password[9])) {	
						FLAG_Password = TRUE;
					}
					SetTextValue(16, 3, (unsigned char*)value_bufB);
			}
			if(com_buffer[6] == 0X08){
				if((com_buffer[6] == 0X08) && (com_buffer[7] == 0X10) && (com_buffer[8] == 0X02) \
						&& (com_buffer[9] == 0X61) && FLAG_Password) {
						FLAG_Password = FALSE;
						SetScreen(1);		//���ý���
						SetTextValue(16, 3, (unsigned char*)value_Clear);	
					}else {
						SetScreen(17);	//����������
						SetTextValue(16, 3, (unsigned char*)value_Clear);	
					}
				}
			if(com_buffer[6] == 0X0A || com_buffer[6] == 0X0C || com_buffer[6] == 0X05){
				SetTextValue(16, 3, (unsigned char*)value_Clear);	
			}
}

/**
	* @brief	���ý���
	*
	* @param	com_buffer
	* @retval None
	*/
void Set_IF(unsigned char *com_buffer)
{
		volatile uint8_t i = 0;  
				
			FLAG_Setif = TRUE; //���ý����������ˢ�����ý���
			
			/*****************���ü���ߵ绰����**************/
			if(com_buffer[6] == 0X13){
				if(com_buffer[8] == 0X2E){
					R_unwork = TRUE;
					EE_WriteVariable(R_unwork_Addr, 0x01);//�������ʧЧ��־
					beep(200);
				}else if(com_buffer[8] == 0X30){
					R_unwork = FALSE;
					EE_WriteVariable(R_unwork_Addr, 0x00);//�������ʧЧ��־
					beep(200);
				}else if(com_buffer[8] == 0X32){	//���õڶ����绰����
					Telnum = 2;
				}else if(com_buffer[8] == 0X33){	//���õ������绰����
					Telnum = 3;
				}else{
					for(i = 0; i < 11; i++) {
						Tel_buffer[i] = com_buffer[i+8];
					}	
					FLAG_Tel = TRUE;
				}	
			}

      /***********SET V_BETTRY*****************/
			if(com_buffer[6] == 0X17){ 
					for(i=8;com_buffer[i] != 0X00;i++){
						B_V_buffer[i-8] = com_buffer[i];
					}
					B_V_buffer[i-8] = 0x00;				
					FLAG_B_V = TRUE;				
			}	
			/***********SET C_BETTRY*****************/
			if(com_buffer[6] == 0X32){ 
					for(i=8;com_buffer[i] != 0X00;i++){
						B_C_buffer[i-8] = com_buffer[i];
					}
					B_C_buffer[i-8] = 0x00;	
					FLAG_B_C = TRUE;				
			}	
			/***********SET N_BETTRY*****************/
			if(com_buffer[6] == 0X36){ 
					for(i=8;com_buffer[i] != 0X00;i++){
						B_N_buffer[i-8] = com_buffer[i];
					}
					B_N_buffer[i-8] = 0x00;						
					FLAG_B_N = TRUE;				
			}	

			/***********SET UPPER_VOLTAGE*****************/
			//����������ΪС��������λ��������λ
			if(com_buffer[6] == 0X0C){ 
        if( (com_buffer[9] == 0X2E) || (com_buffer[10] == 0X2E) || (com_buffer[11] == 0X2E) ){ //0X2E"."��ACSII�룬ȷ��С����λ��
					for(i=8;com_buffer[i] != 0X2E;i++){
						Up_V_buffer[i-8] = com_buffer[i];
					}
					if(com_buffer[i+2] == 0x00){	//С�������һλ��
						Up_V_buffer[i-8] = com_buffer[i+1];
						Up_V_buffer[i-7] = 0x30;	//��һλ0�����湫ʽ��100
						Up_V_buffer[i-6] = 0x00;	//�ַ�����β��־
					}else{	//С���������λ��
            Up_V_buffer[i-8] = com_buffer[i+1];
						Up_V_buffer[i-7] = com_buffer[i+2];
						Up_V_buffer[i-6] = 0x00;
					}
				}else{	//��С���㣬����Ϊ����
				  for(i=8;com_buffer[i] != 0X00;i++){
			  	Up_V_buffer[i-8] = com_buffer[i];
				  }
					Up_V_buffer[i-8] = 0x30;	
					Up_V_buffer[i-7] = 0x30;
					Up_V_buffer[i-6] = 0x00;					
				}	
				FLAG_Up_V = TRUE;				
			}			
			/***********SET LOWER_VOLTAGE*****************/
			if(com_buffer[6] == 0X02){
        if( (com_buffer[9] == 0X2E) || (com_buffer[10] == 0X2E) || (com_buffer[11] == 0X2E) ){				
					for(i=8;com_buffer[i] != 0X2E;i++){
						Lo_V_buffer[i-8] = com_buffer[i];
					}
					if(com_buffer[i+2] == 0x00){
						Lo_V_buffer[i-8] = com_buffer[i+1];
						Lo_V_buffer[i-7] = 0x30;
						Lo_V_buffer[i-6] = 0x00;
					}
					else{
						Lo_V_buffer[i-8] = com_buffer[i+1];
						Lo_V_buffer[i-7] = com_buffer[i+2];
						Lo_V_buffer[i-6] = 0x00;
				  }
				}
				else{
					for(i=8;com_buffer[i] != 0X00;i++){
			  	Lo_V_buffer[i-8] = com_buffer[i];
				  }
					Lo_V_buffer[i-8] = 0x30;
					Lo_V_buffer[i-7] = 0x30;						
					Lo_V_buffer[i-6] = 0x00;		
				}
				FLAG_Lo_V = TRUE;
			}
			/***********SET UPPER_TEMP*****************/	
			if(com_buffer[6] == 0X04){
				if( (com_buffer[9] == 0X2E) || (com_buffer[10] == 0X2E) || (com_buffer[11] == 0X2E) ){
					for(i=8;com_buffer[i] != 0X2E;i++){
						Up_T_buffer[i-8] = com_buffer[i];
					}
					if(com_buffer[i+2] ==0x00){
						Up_T_buffer[i-8] = com_buffer[i+1];
						Up_T_buffer[i-7] = 0x30;
						Up_T_buffer[i-6] = 0x00;
					}
					else{
						Up_T_buffer[i-8] = com_buffer[i+1];
						Up_T_buffer[i-7] = com_buffer[i+2];
						Up_T_buffer[i-6] = 0x00;
					}
				}
				else{
					for(i=8;com_buffer[i] != 0X00;i++){
			  	Up_T_buffer[i-8] = com_buffer[i];
				  }
					Up_T_buffer[i-8] = 0x30;	
					Up_T_buffer[i-7] = 0x30;	
					Up_T_buffer[i-6] = 0x00;	
				}
				FLAG_Up_T = TRUE;
			}	
			/***********SET UPPER_RES*****************/	
			if(com_buffer[6] == 0X06){
				if( (com_buffer[9] == 0X2E) || (com_buffer[10] == 0X2E) || (com_buffer[11] == 0X2E) ){
					for(i=8;com_buffer[i] != 0X2E;i++){
						Up_R_buffer[i-8] = com_buffer[i];
					}
					if(com_buffer[i+2] ==0x00){
						Up_R_buffer[i-8] = com_buffer[i+1];
						Up_R_buffer[i-7] = 0x30;
						Up_R_buffer[i-6] = 0x00;
					}
					else{
						Up_R_buffer[i-8] = com_buffer[i+1];
						Up_R_buffer[i-7] = com_buffer[i+2];
						Up_R_buffer[i-6] = 0x00;
					}
				}
				else{
					for(i=8;com_buffer[i] != 0X00;i++){
			  	Up_R_buffer[i-8] = com_buffer[i];
				  }
					Up_R_buffer[i-8] = 0x30;	
					Up_R_buffer[i-7] = 0x30;
					Up_R_buffer[i-6] = 0x00;	
				}
				FLAG_Up_R = TRUE;
			}
			/***********SET LOWER CAPACITY*****************/
			if(com_buffer[6] == 0X08){
//				for(i=8;com_buffer[i] != 0X00;i++){
//		  		Lo_C_buffer[i-8] = com_buffer[i];
//				}
//				Lo_C_buffer[i-8] = 0x00;
					if( (com_buffer[9] == 0X2E) || (com_buffer[10] == 0X2E) || (com_buffer[11] == 0X2E) ){
					for(i=8;com_buffer[i] != 0X2E;i++){
						Lo_C_buffer[i-8] = com_buffer[i];
					}
					if(com_buffer[i+2] ==0x00){
						Lo_C_buffer[i-8] = com_buffer[i+1];
						Lo_C_buffer[i-7] = 0x30;
						Lo_C_buffer[i-6] = 0x00;
					}
					else{
						Lo_C_buffer[i-8] = com_buffer[i+1];
						Lo_C_buffer[i-7] = com_buffer[i+2];
						Lo_C_buffer[i-6] = 0x00;
					}
				}
				else{
					for(i=8;com_buffer[i] != 0X00;i++){
			  	Lo_C_buffer[i-8] = com_buffer[i];
				  }
					Lo_C_buffer[i-8] = 0x30;	
					Lo_C_buffer[i-7] = 0x30;
					Lo_C_buffer[i-6] = 0x00;	
				}
				FLAG_Up_C = TRUE;
			}
}

/**
	* @brief	�������ñ���
	*
	* @param	com_buffer
	* @retval None
	*/
void RingSet_Save_IF(unsigned char *com_buffer)
{
		uint16_t Tel_temp;
		uint16_t Up_V;
		uint16_t Lo_V;
		uint16_t Up_T;
		uint16_t Up_R;
		uint16_t Up_C;
		
		FLAG_Setif = TRUE; //���ý����������ˢ�����ý���
		
		PWR_BackupAccessCmd(ENABLE); //ȡ�����ݼĴ���д����
		/*ֻ��д���ı������*/	
		if(FLAG_Tel){
			switch(Telnum)	//��Telnum���绰���룬һ��3��
			{
					case 1:{
						Tel_temp = ((uint16_t)Tel_buffer[0]-48)*1000 + ((uint16_t)Tel_buffer[1]-48)*100 + \
						((uint16_t)Tel_buffer[2]-48)*10 + ((uint16_t)Tel_buffer[3]-48);
						BKP_WriteBackupRegister(Tel_part1,  Tel_temp);
						
						Tel_temp = ((uint16_t)Tel_buffer[4]-48)*1000 + ((uint16_t)Tel_buffer[5]-48)*100 + \
						((uint16_t)Tel_buffer[6]-48)*10 + ((uint16_t)Tel_buffer[7]-48);
						BKP_WriteBackupRegister(Tel_part2,  Tel_temp);
						
						Tel_temp = ((uint16_t)Tel_buffer[8]-48)*100 + ((uint16_t)Tel_buffer[9]-48)*10 + ((uint16_t)Tel_buffer[10]-48);
						BKP_WriteBackupRegister(Tel_part3,  Tel_temp);
						
						break;
					}
					case 2:{
						Tel_temp = ((uint16_t)Tel_buffer[0]-48)*1000 + ((uint16_t)Tel_buffer[1]-48)*100 + \
						((uint16_t)Tel_buffer[2]-48)*10 + ((uint16_t)Tel_buffer[3]-48);
						BKP_WriteBackupRegister(Tel2_part1,  Tel_temp);
						
						Tel_temp = ((uint16_t)Tel_buffer[4]-48)*1000 + ((uint16_t)Tel_buffer[5]-48)*100 + \
						((uint16_t)Tel_buffer[6]-48)*10 + ((uint16_t)Tel_buffer[7]-48);
						BKP_WriteBackupRegister(Tel2_part2,  Tel_temp);
						
						Tel_temp = ((uint16_t)Tel_buffer[8]-48)*100 + ((uint16_t)Tel_buffer[9]-48)*10 + ((uint16_t)Tel_buffer[10]-48);
						BKP_WriteBackupRegister(Tel2_part3,  Tel_temp);
						Telnum = 1;	//�ص�Ĭ��״̬�绰����1
						
						break;
					}
					case 3:{
						Tel_temp = ((uint16_t)Tel_buffer[0]-48)*1000 + ((uint16_t)Tel_buffer[1]-48)*100 + \
						((uint16_t)Tel_buffer[2]-48)*10 + ((uint16_t)Tel_buffer[3]-48);
						BKP_WriteBackupRegister(Tel3_part1,  Tel_temp);
						
						Tel_temp = ((uint16_t)Tel_buffer[4]-48)*1000 + ((uint16_t)Tel_buffer[5]-48)*100 + \
						((uint16_t)Tel_buffer[6]-48)*10 + ((uint16_t)Tel_buffer[7]-48);
						BKP_WriteBackupRegister(Tel3_part2,  Tel_temp);
						
						Tel_temp = ((uint16_t)Tel_buffer[8]-48)*100 + ((uint16_t)Tel_buffer[9]-48)*10 + ((uint16_t)Tel_buffer[10]-48);
						BKP_WriteBackupRegister(Tel3_part3,  Tel_temp);
						Telnum = 1;	//�ص�Ĭ��״̬�绰����1
						
						break;
					}
					default :
						break;
			}
			FLAG_Set_Refresh = TRUE;
			FLAG_Tel = FALSE;
			
		}
						
		if(FLAG_B_V){
			Up_V = atoi(B_V_buffer);
			BKP_WriteBackupRegister(V_BETTRY,  Up_V);
			FLAG_Set_Refresh = TRUE;
			FLAG_B_V = FALSE;
		}
		if(FLAG_B_C){
			Up_V = atoi(B_C_buffer);
			BKP_WriteBackupRegister(C_BETTRY,  Up_V);
			FLAG_Set_Refresh = TRUE;
			FLAG_B_C = FALSE;
		}
		if(FLAG_B_N){
			Up_V = atoi(B_N_buffer);
			BKP_WriteBackupRegister(N_BETTRY,  Up_V);
			FLAG_Set_Refresh = TRUE;
			FLAG_B_N = FALSE;
		}
		if(FLAG_Up_V){
			Up_V = atoi(Up_V_buffer);
			BKP_WriteBackupRegister(V_TOP_BETTRY,  Up_V);
			FLAG_Set_Refresh = TRUE;
			FLAG_Up_V = FALSE;
		}
		if(FLAG_Lo_V){
			Lo_V = atoi(Lo_V_buffer);
			BKP_WriteBackupRegister(V_BOTTOM_BETTRY, Lo_V);
			FLAG_Set_Refresh = TRUE;
			FLAG_Lo_V = FALSE;
		}
		if(FLAG_Up_T){
			Up_T = atoi(Up_T_buffer);
			BKP_WriteBackupRegister(T_TOP_BETTRY,  Up_T);
			FLAG_Set_Refresh = TRUE;
			FLAG_Up_T = FALSE;
		}
		if(FLAG_Up_R){				
			Up_R = atoi(Up_R_buffer);
			BKP_WriteBackupRegister(r_TOP_BETTRY,  Up_R);
			FLAG_Set_Refresh = TRUE;
			FLAG_Up_R = FALSE;
		}
		if(FLAG_Up_C){
			Up_C = atoi(Lo_C_buffer);			
			BKP_WriteBackupRegister(C_BOTTOM_BETTRY,  Up_C);
			FLAG_Set_Refresh = TRUE;
			FLAG_Up_C = FALSE;
		}
		
		GPIO_SetBits(GPIOA, GPIO_Pin_1);
		OSTimeDlyHMSM(0, 0, 0, 100);
		GPIO_ResetBits(GPIOA, GPIO_Pin_1);
		OSTimeDlyHMSM(0, 0, 0, 100);
		PWR_BackupAccessCmd(DISABLE); //���ݼĴ���д����
}

/**
	* @brief	��������������������
	*
	* @param	com_buffer
	* @retval 
	*/
	void Analysis(unsigned char *com_buffer)
{
	/*��ȡ���ý���Ĵ��ذ�ť�����������ֲ�*/
	/*�쳣��ؽ���*/
	if((com_buffer[1] == 0XB1) && (com_buffer[2] == 0X11) && (com_buffer[4] == 0X1A)) 
	{
			Abnor_BatteryIF(com_buffer);
	}
	
	/*У׼Ȩ���������*/
	if((com_buffer[1] == 0XB1) && (com_buffer[2] == 0X11) && (com_buffer[4] == 0X18)) 
	{
			Calib_PassWorldIF(com_buffer);
	}
	
	/*����У׼����*/
	if((com_buffer[1] == 0XB1) && (com_buffer[2] == 0X11) && (com_buffer[4] == 0X16)) 
	{
			Param_CalibrationIF(com_buffer);
	}
	/*������Ľ���*/
	if((com_buffer[1] == 0XB1) && (com_buffer[2] == 0X11) && (com_buffer[4] == 0X12)) 
	{
			PassW_ChangeIF(com_buffer);
	}
	
	/*�����������*/
	if((com_buffer[1] == 0XB1) && (com_buffer[2] == 0X11) && (com_buffer[4] == 0X10)) 
	{
			PassW_InputIF(com_buffer);
	}
	
	/*���ý���*/
  if( (com_buffer[1] == 0XB1) && (com_buffer[2] == 0X11) && (com_buffer[4] == 0X01) )
	{
			Set_IF(com_buffer);
  }
	
	//��������ȡ��B1110001000A100000FFFCFFFF
	if((com_buffer[1] == 0XB1) && (com_buffer[2] == 0X11) && (com_buffer[4] == 0X01) && (com_buffer[6] == 0X0A))
	{	
			FLAG_Set_Refresh = TRUE;
	}
	//�������ñ���
	if((com_buffer[1] == 0XB1) && (com_buffer[2] == 0X11) && (com_buffer[4] == 0X02) && (com_buffer[6] == 0X01))
	{			
			RingSet_Save_IF(com_buffer);
	}
						
}



/**
	* @brief	ˢ�²�����Ϣ
	*
	* @param	None
	* @retval 
	*/
void Param_IF(void)
{
    char cfg_buf[11] = {0};
    char bkp_buf[2] = {0};
		uint16_t B_tmp = 0;
		char num_buf[5] = {0};
    float upper_voltage_tmp = 0.0;
    float lower_voltage_tmp = 0.0;
    float res_tmp = 0.0;
    float temp_tmp = 0.0;
		float capacity_tmp = 0.0;
		
//����Ļ���
			//��Ļ�������ʾ��ǰ��������Ϣ
			/***********SET Tel*****************/
			memset(cfg_buf, 0, sizeof(cfg_buf));
//				for(i = 0; i < 11; i++) {
//					EE_ReadVariable(PhoneNumber_Addr + i, Pn_temp);
//					sprintf(num_buf, "%d", Pn_temp[0]);
//					strcat(cfg_buf, num_buf);
//				}
			bkp_buf[1] = (char)(BKP_ReadBackupRegister(Tel_part1) >> 8);
			bkp_buf[0] = (char)(BKP_ReadBackupRegister(Tel_part1) & 0x00ff);
			B_tmp = *(uint16*)bkp_buf;
			sprintf(num_buf, "%d", B_tmp);
			strcat(cfg_buf, num_buf);
			bkp_buf[1] = (char)(BKP_ReadBackupRegister(Tel_part2) >> 8);
			bkp_buf[0] = (char)(BKP_ReadBackupRegister(Tel_part2) & 0x00ff);
			B_tmp = *(uint16*)bkp_buf;
			sprintf(num_buf, "%d", B_tmp);
			strcat(cfg_buf, num_buf);
			bkp_buf[1] = (char)(BKP_ReadBackupRegister(Tel_part3) >> 8);
			bkp_buf[0] = (char)(BKP_ReadBackupRegister(Tel_part3) & 0x00ff);
			B_tmp = *(uint16*)bkp_buf;
			sprintf(num_buf, "%d", B_tmp);
			strcat(cfg_buf, num_buf);

			if(FLAG_Set_Refresh){	
				SetTextValue(1, 19, (unsigned char*)cfg_buf);
				SetTextValue(15, 7, (unsigned char*)cfg_buf);
			}
			/***********SET V_BETTRY*****************/
			bkp_buf[1] = (char)(BKP_ReadBackupRegister(V_BETTRY) >> 8);
			bkp_buf[0] = (char)(BKP_ReadBackupRegister(V_BETTRY) & 0x00ff);
			B_tmp = *(uint16*)bkp_buf;
			sprintf(cfg_buf, "%d", B_tmp);
			if(FLAG_Set_Refresh){	
				SetTextValue(1, 23, (unsigned char*)cfg_buf);
				SetTextValue(15, 11, (unsigned char*)cfg_buf);
			}
			/***********SET C_BETTRY*****************/
			bkp_buf[1] = (char)(BKP_ReadBackupRegister(C_BETTRY) >> 8);
			bkp_buf[0] = (char)(BKP_ReadBackupRegister(C_BETTRY) & 0x00ff);
			B_tmp = *(uint16*)bkp_buf;
			sprintf(cfg_buf, "%d", B_tmp);
			if(FLAG_Set_Refresh){	
				SetTextValue(1, 50, (unsigned char*)cfg_buf);
				SetTextValue(15, 30, (unsigned char*)cfg_buf);
			}
			/***********SET N_BETTRY*****************/
			bkp_buf[1] = (char)(BKP_ReadBackupRegister(N_BETTRY) >> 8);
			bkp_buf[0] = (char)(BKP_ReadBackupRegister(N_BETTRY) & 0x00ff);
			B_tmp = *(uint16*)bkp_buf;
			sprintf(cfg_buf, "%d", B_tmp);
			if(FLAG_Set_Refresh){				
				SetTextValue(1, 54, (unsigned char*)cfg_buf);
				SetTextValue(15, 39, (unsigned char*)cfg_buf);
			}
			/***********SET UPPER_VOLTAGE*****************/
			bkp_buf[1] = (char)(BKP_ReadBackupRegister(V_TOP_BETTRY) >> 8);
			bkp_buf[0] = (char)(BKP_ReadBackupRegister(V_TOP_BETTRY) & 0x00ff);
			upper_voltage_tmp = *(uint16*)bkp_buf;
			upper_voltage_tmp /= 100.0;
			sprintf(cfg_buf, "%.2f", upper_voltage_tmp);
			SetTextValue(1, 11, (unsigned char*)cfg_buf);
			SetTextValue(15, 23, (unsigned char*)cfg_buf);
			//����ȡ������ʱˢ��������
			if(FLAG_Set_Refresh){
					SetTextValue(1, 12, (unsigned char*)cfg_buf);
			}
			/***********SET LOWER_VOLTAGE*****************/
			bkp_buf[1] = (char)(BKP_ReadBackupRegister(V_BOTTOM_BETTRY) >> 8);
			bkp_buf[0] = (char)(BKP_ReadBackupRegister(V_BOTTOM_BETTRY) & 0x00ff);
			lower_voltage_tmp = *(uint16*)bkp_buf;
			lower_voltage_tmp /= 100.0;
			sprintf(cfg_buf, "%.2f", lower_voltage_tmp);
			SetTextValue(1, 1, (unsigned char*)cfg_buf);
			SetTextValue(15, 38, (unsigned char*)cfg_buf);
							
			if(FLAG_Set_Refresh){
					SetTextValue(1, 2, (unsigned char*)cfg_buf);
			}
			/***********SET UPPER_TEMP*****************/
			bkp_buf[1] = (char)(BKP_ReadBackupRegister(T_TOP_BETTRY) >> 8);
			bkp_buf[0] = (char)(BKP_ReadBackupRegister(T_TOP_BETTRY) & 0x00ff);
			temp_tmp = *(uint16*)bkp_buf;
			temp_tmp /= 100.0;
			sprintf(cfg_buf, "%.2f", temp_tmp);
			SetTextValue(1, 3, (unsigned char*)cfg_buf);
			SetTextValue(15, 44, (unsigned char*)cfg_buf);				
			if(FLAG_Set_Refresh){
					SetTextValue(1, 4, (unsigned char*)cfg_buf);
			}
			/***********SET UPPER_RES*****************/
			bkp_buf[1] = (char)(BKP_ReadBackupRegister(r_TOP_BETTRY) >> 8);
			bkp_buf[0] = (char)(BKP_ReadBackupRegister(r_TOP_BETTRY) & 0x00ff);
			res_tmp = *(uint16*)bkp_buf;
			res_tmp /= 100.0;
			sprintf(cfg_buf, "%.2f", res_tmp);
			
			SetTextValue(1, 5, (unsigned char*)cfg_buf);
			SetTextValue(15, 40, (unsigned char*)cfg_buf);
			if(FLAG_Set_Refresh){
					SetTextValue(1, 6, (unsigned char*)cfg_buf);
			}
			/***********SET LOWER CAPACITY*****************/
			bkp_buf[1] = (char)(BKP_ReadBackupRegister(C_BOTTOM_BETTRY) >> 8);
			bkp_buf[0] = (char)(BKP_ReadBackupRegister(C_BOTTOM_BETTRY) & 0x00ff);
//        capacity_tmp = *(uint16*)bkp_buf;
//        sprintf(cfg_buf, "%d", capacity_tmp);
			capacity_tmp = *(uint16*)bkp_buf;
			capacity_tmp /= 100.0;
//        sprintf(cfg_buf, "%d", capacity_tmp);
			sprintf(cfg_buf, "%.2f", capacity_tmp);
			SetTextValue(1, 7, (unsigned char*)cfg_buf);
			SetTextValue(15, 42, (unsigned char*)cfg_buf);				
			if(FLAG_Set_Refresh){
					SetTextValue(1, 8, (unsigned char*)cfg_buf);
					FLAG_Set_Refresh = FALSE; //ˢ��һ���λ
			}
			
}
/*************************************************
Function: Show()
Description: ��ҳ�Ƹ�������������ʾ���������쳣�����Ϣ
Input: 
Return: 
Others:
 *************************************************/
extern char AbnormalNumber[576];	//�쳣�ĵ�ر��3*4*48=576
extern bool FLAG_Abnormal;

void Show(uint8_t Screen_id)
{
			char value_buf[10] = {0}; 
			char status[14] = {0};	//���״̬��
			char num_buf[3] = {0};
		
			float upper_voltage_tmp = 0.0;
			float lower_voltage_tmp = 0.0;
			float res_tmp = 0.0;
			float temp_tmp = 0.0;
			float capacity_tmp = 0.0;
		
			/***********SET_VOLTAGE*****************/
			sprintf(value_buf, "%.2f", ParaAll[0][4 * Screen_id]);
			SetTextValue(Screen_id + 3, 5, (unsigned char*)value_buf);
			sprintf(value_buf, "%.2f", ParaAll[0][4 * Screen_id + 1]);
			SetTextValue(Screen_id + 3, 6, (unsigned char*)value_buf);
			sprintf(value_buf, "%.2f", ParaAll[0][4 * Screen_id + 2]);
			SetTextValue(Screen_id + 3, 7, (unsigned char*)value_buf); 
			sprintf(value_buf, "%.2f", ParaAll[0][4 * Screen_id + 3]);
			SetTextValue(Screen_id + 3, 8, (unsigned char*)value_buf); 						
			/***********SET_RES********************/
			sprintf(value_buf, "%.3f", ParaAll[2][4 * Screen_id]);
			SetTextValue(Screen_id + 3, 9, (unsigned char*)value_buf);
			sprintf(value_buf, "%.3f", ParaAll[2][4 * Screen_id + 1]);
			SetTextValue(Screen_id + 3, 10, (unsigned char*)value_buf);
			sprintf(value_buf, "%.3f", ParaAll[2][4 * Screen_id + 2]);
			SetTextValue(Screen_id + 3, 11, (unsigned char*)value_buf);
			sprintf(value_buf, "%.3f", ParaAll[2][4 * Screen_id + 3]);
			SetTextValue(Screen_id + 3, 12, (unsigned char*)value_buf);						
			/***********SET_CAPACITY*****************/
			sprintf(value_buf, "%.2f", ParaAll[3][4 * Screen_id]);
			SetTextValue(Screen_id + 3, 13, (unsigned char*)value_buf);
			sprintf(value_buf, "%.2f", ParaAll[3][4 * Screen_id + 1]);
			SetTextValue(Screen_id + 3, 14, (unsigned char*)value_buf);
			sprintf(value_buf, "%.2f", ParaAll[3][4 * Screen_id + 2]);
			SetTextValue(Screen_id + 3, 15, (unsigned char*)value_buf);
			sprintf(value_buf, "%.2f", ParaAll[3][4 * Screen_id + 3]);
			SetTextValue(Screen_id + 3, 16, (unsigned char*)value_buf);
			/***********SET_TEMP*****************/
			sprintf(value_buf, "%.1f", ParaAll[1][4 * Screen_id]);
			SetTextValue(Screen_id + 3, 17, (unsigned char*)value_buf);
			sprintf(value_buf, "%.1f", ParaAll[1][4 * Screen_id + 1]);
			SetTextValue(Screen_id + 3, 18, (unsigned char*)value_buf);
			sprintf(value_buf, "%.1f", ParaAll[1][4 * Screen_id + 2]);
			SetTextValue(Screen_id + 3, 19, (unsigned char*)value_buf);
			sprintf(value_buf, "%.1f", ParaAll[1][4 * Screen_id + 3]);
			SetTextValue(Screen_id + 3, 20, (unsigned char*)value_buf);
			/***********SET_STATUS*****************/	
			memset(status, 0, sizeof(status));
			upper_voltage_tmp = (float)BKP_ReadBackupRegister(V_TOP_BETTRY);
			upper_voltage_tmp /= 100;
			lower_voltage_tmp = (float)BKP_ReadBackupRegister(V_BOTTOM_BETTRY);
			lower_voltage_tmp /= 100;
			res_tmp = (float)BKP_ReadBackupRegister(r_TOP_BETTRY);
			res_tmp /= 100;
			temp_tmp = (float)BKP_ReadBackupRegister(T_TOP_BETTRY);
			temp_tmp /= 100;
			capacity_tmp = (float)BKP_ReadBackupRegister(C_BOTTOM_BETTRY);
			capacity_tmp /= 100;

			if(ParaAll[0][4 * Screen_id] >= upper_voltage_tmp || ParaAll[0][4 * Screen_id] < lower_voltage_tmp || ParaAll[1][4 * Screen_id] >= temp_tmp \
				|| ParaAll[2][4 * Screen_id] >= res_tmp || ParaAll[3][4 * Screen_id] < capacity_tmp) {
					if(4 * Screen_id + 1 < 10) {
							strcat(status, "0");//������λǰ���0������03
					}

					sprintf(num_buf, "%d", 4 * Screen_id + 1);
					strcat(status, num_buf);
					strcat(status, " ");
					
					/**GSM�쳣��ر��**/
					strcat(AbnormalNumber, "003");
					sprintf(num_buf, "%d", (4 * Screen_id + 1)/10); //ʮλ
					strcat(AbnormalNumber, num_buf);
					strcat(AbnormalNumber, "003");
					sprintf(num_buf, "%d", (4 * Screen_id + 1)%10);	//��λ
					strcat(AbnormalNumber, num_buf);								
					strcat(AbnormalNumber, "0020");	//" "���ո񣩵�Unicode�� 
			}

			if(ParaAll[0][4 * Screen_id + 1] >= upper_voltage_tmp || ParaAll[0][4 * Screen_id + 1] < lower_voltage_tmp || ParaAll[1][4 * Screen_id + 1] >= temp_tmp \
				|| ParaAll[2][4 * Screen_id + 1] >= res_tmp || ParaAll[3][4 * Screen_id+1] < capacity_tmp) {
					if(4 * Screen_id + 2 < 10) {
							strcat(status, "0");
					}

					sprintf(num_buf, "%d", 4 * Screen_id + 2);
					strcat(status, num_buf);
					strcat(status, " ");
					
					/**GSM�쳣��ر��**/
					strcat(AbnormalNumber, "003");
					sprintf(num_buf, "%d", (4 * Screen_id + 2)/10); //ʮλ
					strcat(AbnormalNumber, num_buf);
					strcat(AbnormalNumber, "003");
					sprintf(num_buf, "%d", (4 * Screen_id + 2)%10);	//��λ
					strcat(AbnormalNumber, num_buf);								
					strcat(AbnormalNumber, "0020");	//" "���ո񣩵�Unicode�� 
			}

			if(ParaAll[0][4 * Screen_id + 2] >= upper_voltage_tmp || ParaAll[0][4 * Screen_id + 2] < lower_voltage_tmp || ParaAll[1][4 * Screen_id + 2] >= temp_tmp \
				|| ParaAll[2][4 * Screen_id + 2] >= res_tmp || ParaAll[3][4 * Screen_id+2] < capacity_tmp) {
					if(4 * Screen_id + 3 < 10) {
							strcat(status, "0");
					}

					sprintf(num_buf, "%d", 4 * Screen_id + 3);
					strcat(status, num_buf);
					strcat(status, " ");
					
					/**GSM�쳣��ر��**/
					strcat(AbnormalNumber, "003");
					sprintf(num_buf, "%d", (4 * Screen_id + 3)/10); //ʮλ
					strcat(AbnormalNumber, num_buf);
					strcat(AbnormalNumber, "003");
					sprintf(num_buf, "%d", (4 * Screen_id + 3)%10);	//��λ
					strcat(AbnormalNumber, num_buf);								
					strcat(AbnormalNumber, "0020");	//" "���ո񣩵�Unicode��  
			}
			
			if(ParaAll[0][4 * Screen_id + 3] >= upper_voltage_tmp || ParaAll[0][4 * Screen_id + 3] < lower_voltage_tmp || ParaAll[1][4 * Screen_id + 3] >= temp_tmp \
				|| ParaAll[2][4 * Screen_id + 3] >= res_tmp || ParaAll[3][4 * Screen_id+3] < capacity_tmp) {
					if(4 * Screen_id + 4 < 10) {
							strcat(status, "0");
					}

					sprintf(num_buf, "%d", 4 * Screen_id + 4);
					strcat(status, num_buf);
				//  strcat(status, " ");
					
					/**GSM�쳣��ر��**/
					strcat(AbnormalNumber, "003");
					sprintf(num_buf, "%d", (4 * Screen_id + 4)/10); //ʮλ
					strcat(AbnormalNumber, num_buf);
					strcat(AbnormalNumber, "003");
					sprintf(num_buf, "%d", (4 * Screen_id + 4)%10);	//��λ
					strcat(AbnormalNumber, num_buf);								
					strcat(AbnormalNumber, "0020");	//" "���ո񣩵�Unicode�� 
			}						

			if(strlen(status) == 0) {
					strcat(status, "����");
					strcat(status, " ");

			} else {
					strcat(status, "�쳣");	
					FLAG_Abnormal = TRUE;
			}

			//����״̬�����ʾ����,��������ȫ������ʱ��ʾ ������������һ����������������ʾ ��XX �쳣��
			SetTextValue(Screen_id + 3, 21, (unsigned char *)status);
			memset(status, 0, sizeof(status));	//���status�����´η�����ڴ����
}

/*************************************************
Function:  Abnormal_show()
Description: ��ת���쳣��ؽ���
Input: 
Return: 
Others:
 *************************************************/
extern char AN_former[576];	//ǰһ���쳣�ĵ�ر��3*4*10=120 
char AN_one[2] = {0};

void Abnormal_show(void)	
{
    char value_buf[10] = {0}; 
		uint16_t B_tmp = 0;
		unsigned char value_Clear[1] = ""; 
		int i = 0;


		FLAG_Abnormal = FALSE;
	
		
			
				memset(AN_former, 0, sizeof(AN_former));
				strcpy(AN_former,AbnormalNumber);
				
				//�쳣��ؽ��� 
				SetScreen(26);		
				AN_num = strlen(AN_former)/12;
				for(i = 0;i < AN_num;i++){
					AN_one[0] = AN_former[3+12*i];
					AN_one[1] = AN_former[7+12*i];
					AN_temp = atoi(AN_one);
					AN_array[i] = AN_temp;
				}
				AN_temp = AN_array[0];
				sprintf(value_buf, "%02d", AN_array[0]);
				SetTextValue(26, 20, (unsigned char*)value_buf); //���
				sprintf(value_buf, "%.2f", ParaAll[0][AN_temp-1]);
				SetTextValue(26, 10, (unsigned char*)value_buf);	//��ѹ
				sprintf(value_buf, "%.3f", ParaAll[2][AN_temp-1]);
				SetTextValue(26, 11, (unsigned char*)value_buf); //����
				sprintf(value_buf, "%.2f", ParaAll[3][AN_temp-1]);
				SetTextValue(26, 12, (unsigned char*)value_buf); //����
				sprintf(value_buf, "%.2f", ParaAll[1][AN_temp-1]);
				SetTextValue(26, 13, (unsigned char*)value_buf);	//�¶�
				
				//�����������������
				B_tmp = (uint16_t)ParaAll[3][AN_temp-1];
				if((ParaAll[3][AN_temp-1]-B_tmp) >= 0.5){
					B_tmp++;
				}
				sprintf(value_buf, "%3d",B_tmp );
				SetTextValue(26, 34, (unsigned char*)value_buf); //�����ٷֱ�
				
				//���������ͬ����ʾ��ͬ��ɫ
				if(ParaAll[3][AN_temp-1]>80){		
					SetProgressValue(26,27,ParaAll[3][AN_temp-1]);
				}else if(ParaAll[3][AN_temp-1]>20){
								SetProgressValue(26,27,0);
								SetProgressValue(26,26,ParaAll[3][AN_temp-1]);
							}else{
								SetProgressValue(26,27,0);
								SetProgressValue(26,26,0);
								SetProgressValue(26,1,ParaAll[3][AN_temp-1]);
							}
				SetButtonValue(22, 3, 0);			//�ر�����ѡ��	
				SetButtonValue(22, 4, 0);
				SetButtonValue(22, 5, 0);
				SetButtonValue(22, 6, 0);
				SetTextValue(22, 10, (unsigned char*)value_Clear);
				SetTextValue(22, 15, (unsigned char*)value_Clear);
				SetTextValue(22, 16, (unsigned char*)value_Clear);
				SetTextValue(22, 17, (unsigned char*)value_Clear);
				SetTextValue(22, 18, (unsigned char*)value_Clear);
				SetTextValue(22, 19, (unsigned char*)value_Clear);
				SetTextValue(22, 20, (unsigned char*)value_Clear);
				SetTextValue(22, 11, (unsigned char*)value_Clear);
				SetTextValue(22, 12, (unsigned char*)value_Clear);
				SetTextValue(24, 3, (unsigned char*)value_Clear);
				SetTextValue(18, 11, (unsigned char*)value_Clear);
				SetTextValue(18, 12, (unsigned char*)value_Clear);
				SetTextValue(18, 13, (unsigned char*)value_Clear);
				SetTextValue(16, 3, (unsigned char*)value_Clear);	
				
				//��������
//				beep(100);//��������100ms

			
}
