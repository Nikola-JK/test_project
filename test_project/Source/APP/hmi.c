/**
  ******************************************************************************
  * @file    hmi.c 
  * @author  JKL
  * @version V0.1
  * @date    16-July-2015
  * @brief   人机交互界面
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
extern bool R_unwork;//内阻检测失效标志位
extern float ParaAll[4][48];	//48个通道，每个4个参数

extern OS_EVENT *CalSem;//打开任务AppTaskT_VSample

/*用于异常电池界面*/
uint16_t AN_temp = 0,AN_num = 0;//AbnormalNumber
uint8_t AN_array[36] = {0};			//异常电池数组
volatile int8_t AN_channel = 0;//异常电池编号



/*以下用于密码更改界面*/
volatile bool FLAG_Password = FALSE;
volatile bool FLAG_Changeword = FALSE;
volatile bool FLAG_affirmword = FALSE;
unsigned char Npassword_buffer[10];	 //New passworld 新密码
unsigned char Affpassword_buffer[10];//affirm passworld 密码确认


/*以下用于校准界面函数*/	
volatile uint16_t Rc = 0; //R校准值
volatile bool FLAG_36channle = FALSE;
volatile bool FLAG_rCc = FALSE;	//用内阻校准容量
volatile uint8_t Channel_i = 0;	//校准界面全部校准通道累加
volatile uint8_t channel = 1;		//校准界面指定通道校准，点击校准按钮才刷新界面显示通道号
volatile bool FLAG_Cal_Over = TRUE;//内阻校准完成
extern bool R_calibration_OK;
volatile bool FLAG_CPassword = FALSE;//校准权限密码



/*以下用于设置和设置保存界面两函数*/	
volatile 	bool FLAG_B_V = FALSE;	//设置电压后置标志位，点击保存按钮再刷新保存数据，没有置位的不更新
volatile 	bool FLAG_B_C = FALSE;
volatile 	bool FLAG_B_N = FALSE;
volatile 	bool FLAG_Up_V = FALSE;
volatile	bool FLAG_Lo_V = FALSE;
volatile	bool FLAG_Up_T = FALSE;
volatile	bool FLAG_Up_R = FALSE;
volatile	bool FLAG_Up_C = FALSE;	
volatile 	bool FLAG_Tel = FALSE;
char B_V_buffer[8]= {0};					//设置电压后暂存于此数组，点击保存按钮再刷新保存数据
char B_C_buffer[8]= {0};
char B_N_buffer[8]= {0};
char Up_V_buffer[8]= {0};
char Lo_V_buffer[8]= {0};
char Up_T_buffer[8]= {0};
char Up_R_buffer[8]= {0};
char Lo_C_buffer[8]= {0};
char Tel_buffer[11]= {0};//设置监控者电话号码，点击保存按钮再刷新保存数据	
uint8_t Telnum = 1;
volatile bool FLAG_Set_Refresh = TRUE;	//设置界面更新标准，若没有则会出现设置一半退出后，在进入不知道设没设成功。


volatile bool FLAG_Setif = TRUE;	//刷新参数信息标志位

/*Virtual address define by the user: 0XFFFF value is prohibited */

/* Private function prototypes ---------------------------------------------*/
extern uint16_t  ParaCal(uint8_t p, uint8_t NumChannel);	//参数校准函数

/* Private functions -------------------------------------------------------*/



/**
	* @brief	异常电池界面
	*
	* @param	com_buffer
	* @retval None
	*/
void Abnor_BatteryIF(unsigned char *com_buffer)
{
		char temp_buf[10] = {0};
		uint16_t Tel_temp;

		if(com_buffer[6] == 0X20) { //下翻
			AN_channel++;
			if(AN_channel > AN_num-1){
				AN_channel = 0;
			}
				AN_temp = AN_array[AN_channel];
				sprintf(temp_buf, "%02d", AN_array[AN_channel]);
				SetTextValue(26, 20, (unsigned char*)temp_buf); //编号
				sprintf(temp_buf, "%.2f", ParaAll[0][AN_temp-1]);
				SetTextValue(26, 10, (unsigned char*)temp_buf);	//电压
				sprintf(temp_buf, "%.3f", ParaAll[2][AN_temp-1]);
				SetTextValue(26, 11, (unsigned char*)temp_buf); //内阻
				sprintf(temp_buf, "%.2f", ParaAll[3][AN_temp-1]);
				SetTextValue(26, 12, (unsigned char*)temp_buf); //容量
				sprintf(temp_buf, "%.2f", ParaAll[1][AN_temp-1]);
				SetTextValue(26, 13, (unsigned char*)temp_buf);	//温度
				Tel_temp = (uint16_t)ParaAll[3][AN_temp-1];
				if((ParaAll[3][AN_temp-1]-Tel_temp) >= 0.5){
							Tel_temp++;
						}
				sprintf(temp_buf, "%3d",Tel_temp );
				SetTextValue(26, 34, (unsigned char*)temp_buf); //容量百分比
				
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
		if(com_buffer[6] == 0X1F) { //上翻
			AN_channel--;
			if(AN_channel < 0){
				AN_channel = AN_num-1;
			}	
				AN_temp = AN_array[AN_channel];
				sprintf(temp_buf, "%02d", AN_array[AN_channel]);
				SetTextValue(26, 20, (unsigned char*)temp_buf); //编号
				sprintf(temp_buf, "%.2f", ParaAll[0][AN_temp-1]);
				SetTextValue(26, 10, (unsigned char*)temp_buf);	//电压
				sprintf(temp_buf, "%.3f", ParaAll[2][AN_temp-1]);
				SetTextValue(26, 11, (unsigned char*)temp_buf); //内阻
				sprintf(temp_buf, "%.2f", ParaAll[3][AN_temp-1]);
				SetTextValue(26, 12, (unsigned char*)temp_buf); //容量
				sprintf(temp_buf, "%.2f", ParaAll[1][AN_temp-1]);
				SetTextValue(26, 13, (unsigned char*)temp_buf);	//温度
				Tel_temp = (uint16_t)ParaAll[3][AN_temp-1];
				if((ParaAll[3][AN_temp-1]-Tel_temp) >= 0.5){
							Tel_temp++;
						}
				sprintf(temp_buf, "%3d",Tel_temp );
				SetTextValue(26, 34, (unsigned char*)temp_buf); //容量百分比
				
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
	* @brief	校准权限密码界面
	*
	* @param	com_buffer
	* @retval None
	*/
void Calib_PassWorldIF(unsigned char *com_buffer)
{
		unsigned char value_bufB[6] = "******"; 
		unsigned char value_Clear[1] = ""; 
		
	//权限密码输入框
		if(com_buffer[6] == 0X03) { 
			if((com_buffer[8] == 0X68) && (com_buffer[9] == 0X6F) && (com_buffer[10] == 0X70) \
				&& (com_buffer[11] == 0X65) && (com_buffer[12] == 0X20) && (com_buffer[13] == 0X67) \
				&& (com_buffer[14] == 0X72) && (com_buffer[15] == 0X61) && (com_buffer[16] == 0X6E) && (com_buffer[17] == 0X64)) {	
				FLAG_CPassword = TRUE;
			}
			SetTextValue(24, 3, (unsigned char*)value_bufB);
		}
		//确认
		if(com_buffer[6] == 0X08) {
				if(FLAG_CPassword){
						SetScreen(22);		//参数校准界面
						FLAG_CPassword = FALSE;
				}else {
						SetScreen(25);		//权限密码有误界面
						SetTextValue(24, 3, (unsigned char*)value_Clear);
				}
		}
}

/**
	* @brief	参数校准界面
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
		unsigned char VC_buf[] = "电压校准模式"; 
		unsigned char TC_buf[] = "温度校准模式";
		unsigned char CC_buf[] = "容量校准模式";
		unsigned char RC_buf[] = "内阻校准模式";
		unsigned char CV_buf[] = "请输入校准值:";
		unsigned char Channel_buf[] = "通道：";
		unsigned char AD_buf[] = "返回AD码：";
		unsigned char Finish_buf[] = "此项校准完成!";
		unsigned char value_Clear[1] = ""; 
		char R_buffer[8]= {0};
		char temp_buf[10] = {0};
	
		// 返回
		if((com_buffer[6] == 0X09) || (com_buffer[6] == 0X16)) {
				SetButtonValue(22, 3, 0);			//关闭其他选项	
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
					OSSemPost(CalSem);	//打开任务AppTaskT_VSample
					R_calibration_OK = TRUE;	//允许任务AppTaskT_VSample中关闭激励通道
				}	
		}
		// 电压校准选项
		if((com_buffer[6] == 0X03) && (com_buffer[9] == 0X01)) { 
				SetButtonValue(22, 4, 0);//关闭其他选项
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
					OSSemPost(CalSem);	//打开任务AppTaskT_VSample
					R_calibration_OK = TRUE;	//允许任务AppTaskT_VSample中关闭激励通道
				}	
		}
		// 温度校准选项
		if((com_buffer[6] == 0X04) && (com_buffer[9] == 0X01)) { 
				SetButtonValue(22, 3, 0);//关闭其他选项
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
					OSSemPost(CalSem);	//打开任务AppTaskT_VSample
					R_calibration_OK = TRUE;	//允许任务AppTaskT_VSample中关闭激励通道
				}	
		}
		// 容量校准选项
		if((com_buffer[6] == 0X06) && (com_buffer[9] == 0X01)) { 
				SetButtonValue(22, 3, 0);//关闭其他选项
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
					OSSemPost(CalSem);	//打开任务AppTaskT_VSample
					R_calibration_OK = TRUE;	//允许任务AppTaskT_VSample中关闭激励通道
				}		
		}
		// 内阻校准选项
		if((com_buffer[6] == 0X05) && (com_buffer[9] == 0X01)) { 
				SetButtonValue(22, 3, 0);//关闭其他选项
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
				R_calibration_OK = FALSE;		//禁止任务AppTaskT_VSample中关闭激励通道
				OSSemPend(CalSem, 0, &err); //关闭任务AppTaskT_VSample
		}
		// 通道输入
		if((com_buffer[6] == 0X0B)) {	
				/*指定通道校准*/
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
		//校准值输入
		if((com_buffer[6] == 0X0C)) {		
				if( (com_buffer[9] == 0X2E) || (com_buffer[10] == 0X2E) || (com_buffer[11] == 0X2E) ){ //0X2E"."的ACSII码，确定小数点位置
					for(i=8;com_buffer[i] != 0X2E;i++){
						R_buffer[i-8] = com_buffer[i];
					}
					if(com_buffer[i+2] == 0x00){	//小数点后有一位数
						R_buffer[i-8] = com_buffer[i+1];
						R_buffer[i-7] = 0x30;	//补一位0，后面公式除1000
						R_buffer[i-6] = 0x30;
						R_buffer[i-5] = 0x00;	//字符串结尾标志
					}else{	//小数点后有两位数
						R_buffer[i-8] = com_buffer[i+1];
						R_buffer[i-7] = com_buffer[i+2];
						R_buffer[i-6] = 0x30;
						R_buffer[i-5] = 0x00;	//字符串结尾标志
					}
				}else{	//无小数点，输入为整数
					for(i=8;com_buffer[i] != 0X00;i++){
						R_buffer[i-8] = com_buffer[i];
						}
						R_buffer[i-8] = 0x30;	
						R_buffer[i-7] = 0x30;
						R_buffer[i-6] = 0x30;
						R_buffer[i-5] = 0x00;	//字符串结尾标志					
					}	
					Rc = atoi(R_buffer);
		//						printf("Rc = %d \r\n",Rc);
		}
		//校准按钮		
		if((com_buffer[6] == 0X0D)) {
			
			SetTextValue(22, 17, (unsigned char*)AD_buf);
			if(FLAG_rCc){
					EE_WriteVariable(R_Cal_C_Addr, Rc); //用内阻校准容量	
					FLAG_rCc = FALSE;			
					sprintf(temp_buf, "%d", Rc);
					SetTextValue(22, 19, (unsigned char*)temp_buf);
					SetTextValue(22, 18, (unsigned char*)Finish_buf);						
					beep(100);
			}else if(!FLAG_36channle){					
						/*通道N校准*/					
							if(R_unwork) {
							Rk = (uint16_t)Rc;
						}else {
							Rk = (uint16_t)( Rc / r_value[channel] );
						}
							EE_WriteVariable(Channel1_r_k_Addr + channel-1, Rk);

							sprintf(temp_buf, "%d", Rk);
							SetTextValue(22, 19, (unsigned char*)temp_buf);
							SetTextValue(22, 18, (unsigned char*)Finish_buf);
						
							//校准后立马刷新新值
						  ParaAll[2][channel-1] = (float)Rc/1000;						
							Show((channel-1)/4);		
														
							beep(10);
							
							FLAG_Cal_Over = TRUE;
							OSSemPost(CalSem);	//打开任务AppTaskT_VSample
							GPIO_SetBits(GPIOC, GPIO_Pin_4);	//关闭内阻板激励通道
							R_calibration_OK = TRUE;	//允许任务AppTaskT_VSample中关闭激励通道						
				/*全部按顺序校准*/		
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
					OSSemPost(CalSem);	//打开任务AppTaskT_VSample
					R_calibration_OK = TRUE;	//允许任务AppTaskT_VSample中关闭激励通道
				}				
			}		
			
		}

}
	

/**
	* @brief	密码更改界面
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
		
		//原始密码：Hope Grand
		if(com_buffer[6] == 0X0B) {   //原始密码输入框
			if((com_buffer[8] == 0X48) && (com_buffer[9] == 0X6F) && (com_buffer[10] == 0X70) \
				&& (com_buffer[11] == 0X65) && (com_buffer[12] == 0X20) && (com_buffer[13] == 0X47) \
				&& (com_buffer[14] == 0X72) && (com_buffer[15] == 0X61) && (com_buffer[16] == 0X6E) && (com_buffer[17] == 0X64)) {
				FLAG_Changeword = TRUE;	
			}
				SetTextValue(18, 11, (unsigned char*)value_bufB);
		}
		//新密码
		if(com_buffer[6] == 0X0C) { //新密码输入框
			for(i = 0;i < 10;i++) {
				Npassword_buffer[i] = com_buffer[8+i];
			}
			SetTextValue(18, 12, (unsigned char*)value_bufB);
		}
		//新密码确认
		if(com_buffer[6] == 0X0D) {	//新密码确认输入框
			for(i = 0;i < 10;i++) {
				Affpassword_buffer[i] = com_buffer[8+i];
			}
			if((Affpassword_buffer[0]==Npassword_buffer[0]) && (Affpassword_buffer[1]==Npassword_buffer[1]) && (Affpassword_buffer[2]==Npassword_buffer[2]) \
				 && (Affpassword_buffer[3]==Npassword_buffer[3]) && (Affpassword_buffer[4]==Npassword_buffer[4]) && (Affpassword_buffer[5]==Npassword_buffer[5])\
					&& (Affpassword_buffer[6]==Npassword_buffer[6]) && (Affpassword_buffer[7]==Npassword_buffer[7]) && (Affpassword_buffer[8]==Npassword_buffer[8])\
					 && (Affpassword_buffer[9]==Npassword_buffer[9])&& (Affpassword_buffer[9] !=0X00)) {	//新密码与新密码确认相同,且第10位不空，即是十位密码
				FLAG_affirmword = TRUE;				
			}
				SetTextValue(18, 13, (unsigned char*)value_bufB);
		}
		//确定
		if((com_buffer[6] == 0X09) && (com_buffer[7] == 0X10) && (com_buffer[8] == 0X02) \
				&& (com_buffer[9] == 0X61)) {
				if(!FLAG_Changeword){
					SetScreen(20);		//原始密码有误界面
				}else if(!FLAG_affirmword) {
								SetScreen(21);		//密码确认有误界面
							}else {
								SetScreen(19);		//密码更改完成界面
								FLAG_Changeword = FALSE;
								FLAG_affirmword = FALSE;
								SetTextValue(18, 11, (unsigned char*)value_Clear);
								SetTextValue(18, 12, (unsigned char*)value_Clear);
								SetTextValue(18, 13, (unsigned char*)value_Clear);
								//密码保存
								PWR_BackupAccessCmd(ENABLE); //取消备份寄存器写保护
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
								PWR_BackupAccessCmd(DISABLE); //备份寄存器写保护
							}
				SetTextValue(18, 11, (unsigned char*)value_Clear);
				SetTextValue(18, 12, (unsigned char*)value_Clear);
				SetTextValue(18, 13, (unsigned char*)value_Clear);
		}
			
}

/**
	* @brief	密码输入界面
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
		
				//读取密码
//		 for(i = 0; i < 10; i++) {
//        EE_ReadVariable(Password_Addr + i, Password + i);
////        printf("%d",Password[i]);
//        /*如果未保存校准参数，调用默认值*/
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
				 
				
			if(com_buffer[6] == 0X03) { //输入框
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
						SetScreen(1);		//设置界面
						SetTextValue(16, 3, (unsigned char*)value_Clear);	
					}else {
						SetScreen(17);	//密码错误界面
						SetTextValue(16, 3, (unsigned char*)value_Clear);	
					}
				}
			if(com_buffer[6] == 0X0A || com_buffer[6] == 0X0C || com_buffer[6] == 0X05){
				SetTextValue(16, 3, (unsigned char*)value_Clear);	
			}
}

/**
	* @brief	设置界面
	*
	* @param	com_buffer
	* @retval None
	*/
void Set_IF(unsigned char *com_buffer)
{
		volatile uint8_t i = 0;  
				
			FLAG_Setif = TRUE; //设置界面进入允许刷新设置界面
			
			/*****************设置监控者电话号码**************/
			if(com_buffer[6] == 0X13){
				if(com_buffer[8] == 0X2E){
					R_unwork = TRUE;
					EE_WriteVariable(R_unwork_Addr, 0x01);//内阻检测板失效标志
					beep(200);
				}else if(com_buffer[8] == 0X30){
					R_unwork = FALSE;
					EE_WriteVariable(R_unwork_Addr, 0x00);//内阻检测板失效标志
					beep(200);
				}else if(com_buffer[8] == 0X32){	//设置第二个电话号码
					Telnum = 2;
				}else if(com_buffer[8] == 0X33){	//设置第三个电话号码
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
			//若输入数据为小数则整数位不大于三位
			if(com_buffer[6] == 0X0C){ 
        if( (com_buffer[9] == 0X2E) || (com_buffer[10] == 0X2E) || (com_buffer[11] == 0X2E) ){ //0X2E"."的ACSII码，确定小数点位置
					for(i=8;com_buffer[i] != 0X2E;i++){
						Up_V_buffer[i-8] = com_buffer[i];
					}
					if(com_buffer[i+2] == 0x00){	//小数点后有一位数
						Up_V_buffer[i-8] = com_buffer[i+1];
						Up_V_buffer[i-7] = 0x30;	//补一位0，后面公式除100
						Up_V_buffer[i-6] = 0x00;	//字符串结尾标志
					}else{	//小数点后有两位数
            Up_V_buffer[i-8] = com_buffer[i+1];
						Up_V_buffer[i-7] = com_buffer[i+2];
						Up_V_buffer[i-6] = 0x00;
					}
				}else{	//无小数点，输入为整数
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
	* @brief	报警设置保存
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
		
		FLAG_Setif = TRUE; //设置界面进入允许刷新设置界面
		
		PWR_BackupAccessCmd(ENABLE); //取消备份寄存器写保护
		/*只重写被改变的数据*/	
		if(FLAG_Tel){
			switch(Telnum)	//第Telnum个电话号码，一共3个
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
						Telnum = 1;	//回到默认状态电话号码1
						
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
						Telnum = 1;	//回到默认状态电话号码1
						
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
		PWR_BackupAccessCmd(DISABLE); //备份寄存器写保护
}

/**
	* @brief	解析串口屏传来的数据
	*
	* @param	com_buffer
	* @retval 
	*/
	void Analysis(unsigned char *com_buffer)
{
	/*读取设置界面的触控按钮，见串口屏手册*/
	/*异常电池界面*/
	if((com_buffer[1] == 0XB1) && (com_buffer[2] == 0X11) && (com_buffer[4] == 0X1A)) 
	{
			Abnor_BatteryIF(com_buffer);
	}
	
	/*校准权限密码界面*/
	if((com_buffer[1] == 0XB1) && (com_buffer[2] == 0X11) && (com_buffer[4] == 0X18)) 
	{
			Calib_PassWorldIF(com_buffer);
	}
	
	/*参数校准界面*/
	if((com_buffer[1] == 0XB1) && (com_buffer[2] == 0X11) && (com_buffer[4] == 0X16)) 
	{
			Param_CalibrationIF(com_buffer);
	}
	/*密码更改界面*/
	if((com_buffer[1] == 0XB1) && (com_buffer[2] == 0X11) && (com_buffer[4] == 0X12)) 
	{
			PassW_ChangeIF(com_buffer);
	}
	
	/*密码输入界面*/
	if((com_buffer[1] == 0XB1) && (com_buffer[2] == 0X11) && (com_buffer[4] == 0X10)) 
	{
			PassW_InputIF(com_buffer);
	}
	
	/*设置界面*/
  if( (com_buffer[1] == 0XB1) && (com_buffer[2] == 0X11) && (com_buffer[4] == 0X01) )
	{
			Set_IF(com_buffer);
  }
	
	//报警设置取消B1110001000A100000FFFCFFFF
	if((com_buffer[1] == 0XB1) && (com_buffer[2] == 0X11) && (com_buffer[4] == 0X01) && (com_buffer[6] == 0X0A))
	{	
			FLAG_Set_Refresh = TRUE;
	}
	//报警设置保存
	if((com_buffer[1] == 0XB1) && (com_buffer[2] == 0X11) && (com_buffer[4] == 0X02) && (com_buffer[6] == 0X01))
	{			
			RingSet_Save_IF(com_buffer);
	}
						
}



/**
	* @brief	刷新参数信息
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
		
//向屏幕输出
			//屏幕设置项，显示当前的配置信息
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
			//按“取消“键时刷新设置栏
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
					FLAG_Set_Refresh = FALSE; //刷新一遍后复位
			}
			
}
/*************************************************
Function: Show()
Description: 整页推给串口屏进行显示，并储存异常电池信息
Input: 
Return: 
Others:
 *************************************************/
extern char AbnormalNumber[576];	//异常的电池编号3*4*48=576
extern bool FLAG_Abnormal;

void Show(uint8_t Screen_id)
{
			char value_buf[10] = {0}; 
			char status[14] = {0};	//电池状态框
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
							strcat(status, "0");//不足两位前面加0，例如03
					}

					sprintf(num_buf, "%d", 4 * Screen_id + 1);
					strcat(status, num_buf);
					strcat(status, " ");
					
					/**GSM异常电池编号**/
					strcat(AbnormalNumber, "003");
					sprintf(num_buf, "%d", (4 * Screen_id + 1)/10); //十位
					strcat(AbnormalNumber, num_buf);
					strcat(AbnormalNumber, "003");
					sprintf(num_buf, "%d", (4 * Screen_id + 1)%10);	//个位
					strcat(AbnormalNumber, num_buf);								
					strcat(AbnormalNumber, "0020");	//" "（空格）的Unicode码 
			}

			if(ParaAll[0][4 * Screen_id + 1] >= upper_voltage_tmp || ParaAll[0][4 * Screen_id + 1] < lower_voltage_tmp || ParaAll[1][4 * Screen_id + 1] >= temp_tmp \
				|| ParaAll[2][4 * Screen_id + 1] >= res_tmp || ParaAll[3][4 * Screen_id+1] < capacity_tmp) {
					if(4 * Screen_id + 2 < 10) {
							strcat(status, "0");
					}

					sprintf(num_buf, "%d", 4 * Screen_id + 2);
					strcat(status, num_buf);
					strcat(status, " ");
					
					/**GSM异常电池编号**/
					strcat(AbnormalNumber, "003");
					sprintf(num_buf, "%d", (4 * Screen_id + 2)/10); //十位
					strcat(AbnormalNumber, num_buf);
					strcat(AbnormalNumber, "003");
					sprintf(num_buf, "%d", (4 * Screen_id + 2)%10);	//个位
					strcat(AbnormalNumber, num_buf);								
					strcat(AbnormalNumber, "0020");	//" "（空格）的Unicode码 
			}

			if(ParaAll[0][4 * Screen_id + 2] >= upper_voltage_tmp || ParaAll[0][4 * Screen_id + 2] < lower_voltage_tmp || ParaAll[1][4 * Screen_id + 2] >= temp_tmp \
				|| ParaAll[2][4 * Screen_id + 2] >= res_tmp || ParaAll[3][4 * Screen_id+2] < capacity_tmp) {
					if(4 * Screen_id + 3 < 10) {
							strcat(status, "0");
					}

					sprintf(num_buf, "%d", 4 * Screen_id + 3);
					strcat(status, num_buf);
					strcat(status, " ");
					
					/**GSM异常电池编号**/
					strcat(AbnormalNumber, "003");
					sprintf(num_buf, "%d", (4 * Screen_id + 3)/10); //十位
					strcat(AbnormalNumber, num_buf);
					strcat(AbnormalNumber, "003");
					sprintf(num_buf, "%d", (4 * Screen_id + 3)%10);	//个位
					strcat(AbnormalNumber, num_buf);								
					strcat(AbnormalNumber, "0020");	//" "（空格）的Unicode码  
			}
			
			if(ParaAll[0][4 * Screen_id + 3] >= upper_voltage_tmp || ParaAll[0][4 * Screen_id + 3] < lower_voltage_tmp || ParaAll[1][4 * Screen_id + 3] >= temp_tmp \
				|| ParaAll[2][4 * Screen_id + 3] >= res_tmp || ParaAll[3][4 * Screen_id+3] < capacity_tmp) {
					if(4 * Screen_id + 4 < 10) {
							strcat(status, "0");
					}

					sprintf(num_buf, "%d", 4 * Screen_id + 4);
					strcat(status, num_buf);
				//  strcat(status, " ");
					
					/**GSM异常电池编号**/
					strcat(AbnormalNumber, "003");
					sprintf(num_buf, "%d", (4 * Screen_id + 4)/10); //十位
					strcat(AbnormalNumber, num_buf);
					strcat(AbnormalNumber, "003");
					sprintf(num_buf, "%d", (4 * Screen_id + 4)%10);	//个位
					strcat(AbnormalNumber, num_buf);								
					strcat(AbnormalNumber, "0020");	//" "（空格）的Unicode码 
			}						

			if(strlen(status) == 0) {
					strcat(status, "正常");
					strcat(status, " ");

			} else {
					strcat(status, "异常");	
					FLAG_Abnormal = TRUE;
			}

			//设置状态框的显示内容,参数正常全部正常时显示 “正常”，有一个或多个不正常是显示 “XX 异常”
			SetTextValue(Screen_id + 3, 21, (unsigned char *)status);
			memset(status, 0, sizeof(status));	//清空status避免下次发错和内存溢出
}

/*************************************************
Function:  Abnormal_show()
Description: 跳转到异常电池界面
Input: 
Return: 
Others:
 *************************************************/
extern char AN_former[576];	//前一次异常的电池编号3*4*10=120 
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
				
				//异常电池界面 
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
				SetTextValue(26, 20, (unsigned char*)value_buf); //编号
				sprintf(value_buf, "%.2f", ParaAll[0][AN_temp-1]);
				SetTextValue(26, 10, (unsigned char*)value_buf);	//电压
				sprintf(value_buf, "%.3f", ParaAll[2][AN_temp-1]);
				SetTextValue(26, 11, (unsigned char*)value_buf); //内阻
				sprintf(value_buf, "%.2f", ParaAll[3][AN_temp-1]);
				SetTextValue(26, 12, (unsigned char*)value_buf); //容量
				sprintf(value_buf, "%.2f", ParaAll[1][AN_temp-1]);
				SetTextValue(26, 13, (unsigned char*)value_buf);	//温度
				
				//容量四舍五入成整数
				B_tmp = (uint16_t)ParaAll[3][AN_temp-1];
				if((ParaAll[3][AN_temp-1]-B_tmp) >= 0.5){
					B_tmp++;
				}
				sprintf(value_buf, "%3d",B_tmp );
				SetTextValue(26, 34, (unsigned char*)value_buf); //容量百分比
				
				//电池容量不同，显示不同颜色
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
				SetButtonValue(22, 3, 0);			//关闭其他选项	
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
				
				//蜂鸣器响
//				beep(100);//蜂鸣器响100ms

			
}
