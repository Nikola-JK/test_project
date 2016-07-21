
#include "com.h"
#include <stdio.h>
#include "stm32f10x.h"
#include "includes.h"
#include <stdlib.h>
#include "string.h"
#include "stdio.h"

#include "nrf905.h"
#include "app.h"

static void USART3_SendData(unsigned char ch);

static void USART2_SendData(unsigned char ch);

static void USART1_SendData(unsigned char ch);

static void hand_shake(void);
static uint16_t CRC16(unsigned char *puchMsg, unsigned int usDataLen);
static void return_version(void);
static void return_id(void);
void return_para_all(void);
void nrf905_return_para_all(void);
static void return_addr(uint8_t Code);
static void return_ack(uint8_t Code , uint8_t ACK);
static void return_cal(uint8_t, uint8_t, uint8_t);
static void return_all_voltage();

static void return_config();

static void return_all_temp();
static void return_all_res();
static void return_all_capacity();

static int readADD(void);

bool R_calibration_OK = TRUE;
FlagStatus V_calibration_OK = RESET;
bool R_unwork = FALSE;

extern OS_EVENT *TxMbox;
extern OS_EVENT *CalSem;
extern OS_EVENT *ParaMbox;
extern OS_EVENT *DetectSem;

const unsigned auchCRCHi[256] = {
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
    0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
    0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01,
    0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81,
    0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01,
    0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
    0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
    0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01,
    0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
    0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01,
    0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
    0x40
};
const unsigned auchCRCLo[256] = {
    0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06, 0x07, 0xC7, 0x05, 0xC5, 0xC4,
    0x04, 0xCC, 0x0C, 0x0D, 0xCD, 0x0F, 0xCF, 0xCE, 0x0E, 0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09,
    0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9, 0x1B, 0xDB, 0xDA, 0x1A, 0x1E, 0xDE, 0xDF, 0x1F, 0xDD,
    0x1D, 0x1C, 0xDC, 0x14, 0xD4, 0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3,
    0x11, 0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3, 0xF2, 0x32, 0x36, 0xF6, 0xF7,
    0x37, 0xF5, 0x35, 0x34, 0xF4, 0x3C, 0xFC, 0xFD, 0x3D, 0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A,
    0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38, 0x28, 0xE8, 0xE9, 0x29, 0xEB, 0x2B, 0x2A, 0xEA, 0xEE,
    0x2E, 0x2F, 0xEF, 0x2D, 0xED, 0xEC, 0x2C, 0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26,
    0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0, 0x60, 0x61, 0xA1, 0x63, 0xA3, 0xA2,
    0x62, 0x66, 0xA6, 0xA7, 0x67, 0xA5, 0x65, 0x64, 0xA4, 0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F,
    0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB, 0x69, 0xA9, 0xA8, 0x68, 0x78, 0xB8, 0xB9, 0x79, 0xBB,
    0x7B, 0x7A, 0xBA, 0xBE, 0x7E, 0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5,
    0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71, 0x70, 0xB0, 0x50, 0x90, 0x91,
    0x51, 0x93, 0x53, 0x52, 0x92, 0x96, 0x56, 0x57, 0x97, 0x55, 0x95, 0x94, 0x54, 0x9C, 0x5C,
    0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E, 0x5A, 0x9A, 0x9B, 0x5B, 0x99, 0x59, 0x58, 0x98, 0x88,
    0x48, 0x49, 0x89, 0x4B, 0x8B, 0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C,
    0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42, 0x43, 0x83, 0x41, 0x81, 0x80,
    0x40
};

/*******************************************************************************
函数名：CRC16
输  入: *puchMsg  ---指向被校验数组的指针
        usDataLen ---数据长度
返  回: CRC 计算结果
功能说明：
********************************************************************************/

uint16_t CRC16(unsigned char *puchMsg, unsigned int usDataLen)
{
    unsigned char uIndex ;                              //*把CRC表*/
    unsigned char uchCRCHi = 0xff ;                     //* 初始化高字节*/
    unsigned char uchCRCLo = 0xff ;                     //* 初始化低字节*/

    while(usDataLen--) {                                //*通过数据缓冲器*/
        uIndex = uchCRCHi ^ *puchMsg++ ;                  //*计算CRC */
        uchCRCHi = uchCRCLo ^ auchCRCHi[uIndex] ;
        uchCRCLo = auchCRCLo[uIndex] ;
    }

    return (uchCRCHi << 8 | uchCRCLo);
}

/*扫描拨码开关，编码地址*/
static int readADD(void)
{
	return 0x00;
}

/* 握手返回 */
static void hand_shake(void)
{
    USART2_SendData(0xEE);
    USART2_SendData((uint8_t)readADD());
    USART2_SendData(0x00);
    USART2_SendData(0x00);
    USART2_SendData(0x01);
    USART2_SendData(0x01);
    USART2_SendData(0xED);
}

/*************************************************
 Function: detect
 Description: 模块开始检测电压内阻温度容量
 Input:code--- R_ADD/W_ADD
 Return:
 Others:
 *************************************************/
static void detect(void)
{
	OSSemPost(DetectSem);
}
/*
//内部通讯（收敛中心与检测模块）
void com_inside(unsigned char *com_buffer)
{
    unsigned char *ext_buffer;
    uint8_t i = 0, err = 0;

		uint16_t Rv = 0;
		uint16_t Ri = 0;
		uint16_t Rk = 0;
    uint16_t crc_receive, crc_result, data_len;
    unsigned char crc_data_buf[20];
		
		if(com_buffer[1] == 0X00)	//广播地址
		{
			switch(com_buffer[2]) 
				{
				    case R_ADDR_inside: 		//0x02 地址
                return_addr(R_ADDR);
                break;
								
						case DETECT: 		//0x03 检测各参数，因为检测需要时间所以先广播让所有的模块检测参数等待传送
                detect();
                break;
						
						default:
                break;
					}
		}
    else if(com_buffer[1] == ((uint8_t)readADD()) )//是此地址
		{ 
        switch(com_buffer[2]) 
				{
            case HAND_SHAKE_inside: 	//0x00 握手  
								hand_shake();
								break;
								

            case R_B_VTC:  //0x07 电池的 电压 内阻 容量 温度              
								nrf905_return_para_all();
								return_para_all();
                break;
								
            
            default:
                break;
        }
    }

}
*/


//外部通信（收敛中心与上位机）
void com(unsigned char *com_buffer)
{
    unsigned char *ext_buffer;
    uint8_t i = 0, err = 0;
		uint16_t Rv = 0;
		uint16_t Ri = 0;
		uint16_t Rk = 0;
    uint16_t crc_receive, crc_result, data_len;
    unsigned char crc_data_buf[20];
		uint16_t Tel_temp;

    if((com_buffer[1] == ((uint8_t)BKP_ReadBackupRegister(ADD))) || (com_buffer[1] == 0X00)) { //是此地址或上位机广播查询地址
        switch(com_buffer[2]) {
            case HAND_SHAKE: 	//0x00 握手
                if(com_buffer[5] == 0x00) {
                    T_485_EN1;;
                    hand_shake();
                    R_485_EN1;
                    break;
                }

            case R_VERSION: 	//0x01 版本
                T_485_EN1;
                return_version();
                R_485_EN1;
                break;

            case R_ADDR: 		//0x02 地址
                T_485_EN1;
                return_addr(R_ADDR);
                R_485_EN1;
                break;

					case R_A_B_CONFIG:	//0x03 电池组的配置信息 电压 容量 电池数量
								T_485_EN1;
                return_config();
                R_485_EN1;
                break;

            case R_ALL_B_V:	//0x04 所有电池的电压
                T_485_EN1;
                return_all_voltage();
                R_485_EN1;
                break;

            case R_ALL_B_T:	//0x05 所有电池的温度
                T_485_EN1;
                return_all_temp();
                R_485_EN1;
                break;

            case R_ALL_B_C:	//0x06 所有电池的容量
                T_485_EN1;
                return_all_capacity();
                R_485_EN1;
                break;

            case R_ALL_B_VTC:  //0x07 所有电池的 电压 内阻 容量 温度
                T_485_EN1;
                return_para_all();
                R_485_EN1;
                break;

            case R_ALL_CONFIG: //0x0D 配置信息
                T_485_EN1;
//                return_all_config();
                R_485_EN1;
                break;

            case R_UNIQID: 	//0x11 设备ID
                T_485_EN1;
                return_id();
                R_485_EN1;
                break;

            case R_SC_INF:  	//0x12 自检信息
                T_485_EN1;
                return_ack(R_SC_INF, 0);
                R_485_EN1;
                break;

            case R_ALL_R0:		//0x0E 所有电池的内阻
                T_485_EN1;
                return_all_res();
                R_485_EN1;
                break;
#if 0
            case CAL_V:		//0xC0 电压校准
                OSSemPend(CalSem, 0, &err); //关闭任务AppTaskT_VSample
								
								/*指定通道校准*/
								if(com_buffer[4] != 0x00){	
									
										ChannelAllDisable();
										OSTimeDlyHMSM(0, 0, 0, 100);	
										Channel_Select(com_buffer[5]);
										OSTimeDlyHMSM(0, 0, 0, 100);
										
											/*通道N零点校准*/
										ext_buffer = (unsigned char *)OSMboxPend(TxMbox, 0, &err);

										if(err == OS_TIMEOUT) {
												break;

//										} else if(ext_buffer[5] == (i + 1)) {
										}else{
												ZERO = ParaCal(1, ext_buffer[5]);
												EE_WriteVariable(Channel1_V_ZERO_Addr + ext_buffer[5]-1, ZERO);

												Com_485TX_Enable;
												return_cal(ZERO >> 8, ZERO & 0x00ff, CAL_V);
												Com_485TX_Disable;
//												printf("  Vl= %d  channel= %d",ZERO , ext_buffer[5]);
										}															
										/*通道N满量程校准*/
										ext_buffer = (unsigned char *)OSMboxPend(TxMbox, 0, &err);

										if(err == OS_TIMEOUT) {
												break;

//										} else if(ext_buffer[5] == (i + 1)) 
										}else{
												FULL = ParaCal(1, i+1);
												EE_WriteVariable(Channel1_V_FULL_Addr + ext_buffer[5]-1, FULL);
												Com_485TX_Enable;
												return_cal(FULL >> 8, FULL & 0x00ff, CAL_V);
												Com_485TX_Disable;
//												printf("  Vh= %d",FULL);																									
										}
								/*全部按顺序校准*/		
								}else{
										for(i = 0; i < 8; i++) {
												ChannelAllDisable();
												OSTimeDlyHMSM(0, 0, 0, 100);	
												Channel_Select(i + 1);
												OSTimeDlyHMSM(0, 0, 0, 100);
												
												/*通道N零点校准*/
												ext_buffer = (unsigned char *)OSMboxPend(TxMbox, 0, &err);

												if(err == OS_TIMEOUT) {
														break;

//												} else if(ext_buffer[5] == (i + 1)) {
												}else{
														ZERO = ParaCal(1, i+1);
														EE_WriteVariable(Channel1_V_ZERO_Addr + i, ZERO);

														Com_485TX_Enable;
														return_cal(ZERO >> 8, ZERO & 0x00ff, CAL_V);
														Com_485TX_Disable;
//														printf("  Vl= %d",ZERO);												
												}
										}
									
										for(i = 0; i < 8; i++) {
												ChannelAllDisable();
												OSTimeDlyHMSM(0, 0, 0, 100);	
												Channel_Select(i + 1);
												OSTimeDlyHMSM(0, 0, 0, 100);

												/*通道N满量程校准*/
												ext_buffer = (unsigned char *)OSMboxPend(TxMbox, 0, &err);

												if(err == OS_TIMEOUT) {
														break;

		//										} else if(ext_buffer[5] == (i + 1)) 
												}else{
														FULL = ParaCal(1, i+1);
														EE_WriteVariable(Channel1_V_FULL_Addr + i, FULL);
														Com_485TX_Enable;
														return_cal(FULL >> 8, FULL & 0x00ff, CAL_V);
														Com_485TX_Disable;
//														printf("  Vh= %d",FULL);
												}
										}
								}		
                OSSemPost(CalSem);	//打开任务AppTaskT_VSample
                break;

            case CAL_T: 		//0xC1 温度校准
                OSSemPend(CalSem, 0, &err); //关闭任务AppTaskT_VSample

                for(i = 0; i < 8; i++) {
                    ChannelAllDisable();
                    OSTimeDlyHMSM(0, 0, 0, 100);
                    Channel_Select(i + 1);
                    OSTimeDlyHMSM(0, 0, 0, 100);
                    /*通道N零点校准*/
                    ext_buffer = (unsigned char *)OSMboxPend(TxMbox, 0, &err);

                    if(err == OS_TIMEOUT) {
                        //printf("\r\n Sorry , time up!\r\n");
                        break;

//                    } else if(ext_buffer[5] == (i + 1)) {
										}else{
                        ZERO = ParaCal(0, i+1);
                        EE_WriteVariable(Channel1_T_ZERO_Addr + i, ZERO);
                        Com_485TX_Enable;
                        return_cal(ZERO >> 8, ZERO & 0x00ff, CAL_T);
                        Com_485TX_Disable;
//												printf("  Tl= %d",ZERO);
                    }
								}
								
								for(i = 0; i < 8; i++) {
                    ChannelAllDisable();
                    OSTimeDlyHMSM(0, 0, 0, 100);
                    Channel_Select(i + 1);
                    OSTimeDlyHMSM(0, 0, 0, 100);
                    /*通道N满量程校准*/
                    ext_buffer = (unsigned char *)OSMboxPend(TxMbox, 0, &err);

                    if(err == OS_TIMEOUT) {
                        break;

//                    } else if(ext_buffer[5] == (i + 1)) {
										}else{
                        FULL = ParaCal(0, i+1);
                        EE_WriteVariable(Channel1_T_FULL_Addr + i, FULL);
                        Com_485TX_Enable;
                        return_cal(FULL >> 8, FULL & 0x00ff, CAL_T);
                        Com_485TX_Disable;
//												printf("  Th= %d",FULL);
                    }
                }
								OSSemPost(CalSem);	//打开任务AppTaskT_VSample
                break;
#endif
            case CAL_R: 	//0xC2 内阻校准
								R_calibration_OK = FALSE;		//禁止任务AppTaskT_VSample中关闭激励通道
								OSSemPend(CalSem, 0, &err); //关闭任务AppTaskT_VSample
								GPIO_ResetBits(GPIOC, GPIO_Pin_4);	//打开内阻板激励通道
//								OSTimeDlyHMSM(0, 0, 1, 0);
	
								/*指定通道校准*/
								if(com_buffer[4] == 0x01){	
									
//										ChannelAllDisable();
										OSTimeDlyHMSM(0, 0, 0, 100);	
										Channel_Select(com_buffer[5]);
										OSTimeDlyHMSM(0, 0, 0, 100);
										
											/*通道N校准*/
										ext_buffer = (unsigned char *)OSMboxPend(TxMbox, 0, &err);

										if(err == OS_TIMEOUT) {
												break;
										}else{
											if(R_unwork) {
												Rk = (uint16_t)( (ext_buffer[6]<<8)+ext_buffer[7] );
											}else {
												Rv = ParaCal(2, ext_buffer[5]);
												Ri = ParaCal(3, ext_buffer[5]);
												Rk = (uint16_t)( ( (ext_buffer[6]<<8)+ext_buffer[7] ) * ( (float)Ri / (float)Rv ) );
											}
//												EE_WriteVariable(Channel1_R_k_Addr + ext_buffer[5]-1, Rk);

												T_485_EN1;
												return_cal(Rk >> 8, Rk & 0x00ff, CAL_R);
												R_485_EN1;
//												printf(" R= %d Rv= %d Ri= %d   Rk= %d  Rchannel= %d \r\n",(ext_buffer[6]<<8)+ext_buffer[7] ,Rv,Ri, Rk ,ext_buffer[5]);
												GPIO_SetBits(GPIOA, GPIO_Pin_1);
												OSTimeDlyHMSM(0, 0, 0, 100);
												GPIO_ResetBits(GPIOA, GPIO_Pin_1);
												OSTimeDlyHMSM(0, 0, 0, 100);
										}															
									
								/*全部按顺序校准*/		
								}else if(com_buffer[4] == 0x00){
										for(i = 0; i < 36; i++) {
//												ChannelAllDisable();
												OSTimeDlyHMSM(0, 0, 0, 100);	
												Channel_Select(i + 1);
												OSTimeDlyHMSM(0, 0, 0, 100);
												
												/*通道N校准*/
												ext_buffer = (unsigned char *)OSMboxPend(TxMbox, 0, &err);

												if(err == OS_TIMEOUT) {
														break;

												}else{
														if(R_unwork) {
															Rk = (uint16_t)( (ext_buffer[5]<<8)+ext_buffer[6] );
														}else {
															Rv = ParaCal(2, i+1);
															Ri = ParaCal(3, i+1);
															Rk = (uint16_t)( ( (ext_buffer[5]<<8)+ext_buffer[6] ) * ( (float)Ri / (float)Rv ) );
														}
//															EE_WriteVariable(Channel1_R_k_Addr + i, Rk);

															T_485_EN1;
															return_cal(Rk >> 8, Rk & 0x00ff, CAL_R);
															R_485_EN1;
//															printf(" R= %d Rv= %d Ri= %d   Rk= %d  Rchannel= %d \r\n",(ext_buffer[5]<<8)+ext_buffer[6] ,Rv,Ri, Rk ,i);	
															GPIO_SetBits(GPIOA, GPIO_Pin_1);
															OSTimeDlyHMSM(0, 0, 0, 100);
															GPIO_ResetBits(GPIOA, GPIO_Pin_1);
															OSTimeDlyHMSM(0, 0, 0, 100);
											 }
										}

								}		
                OSSemPost(CalSem);	//打开任务AppTaskT_VSample
								GPIO_SetBits(GPIOC, GPIO_Pin_4);	//关闭内阻板激励通道
								R_calibration_OK = TRUE;	//允许任务AppTaskT_VSample中关闭激励通道
                break;
						case CAL_C: 	//0xC3 容量校准
									
//								printf("CAL_C :");

								Rk =  (com_buffer[5]<<8)+com_buffer[6] ;	
								EE_WriteVariable(R_Cal_C_Addr, Rk); //用内阻校准容量
//								printf("%d",Rk);
//								printf("\r\n");
								GPIO_SetBits(GPIOA, GPIO_Pin_1);
								OSTimeDlyHMSM(0, 0, 0, 100);
								GPIO_ResetBits(GPIOA, GPIO_Pin_1);
								OSTimeDlyHMSM(0, 0, 0, 100);
								
								T_485_EN1;
								return_cal(Rk >> 8, Rk & 0x00ff, CAL_R);
								R_485_EN1;

                break;
								
						case TEL_N: 	//0xC4 电话号码
									
//								printf("TelNumber :");
								
//									for(i = 0; i < 11; i++) {
//										Tel_temp = (uint16_t)com_buffer[5+i]-48;
//										EE_WriteVariable(PhoneNumber_Addr + i, Tel_temp);
////									printf("%d",Tel_temp);
//									}	
								Tel_temp = ((uint16_t)com_buffer[5]-48)*1000 + ((uint16_t)com_buffer[6]-48)*100 + \
								((uint16_t)com_buffer[7]-48)*10 + ((uint16_t)com_buffer[8]-48);
								BKP_WriteBackupRegister(Tel_part1,  Tel_temp);
								
								Tel_temp = ((uint16_t)com_buffer[9]-48)*1000 + ((uint16_t)com_buffer[10]-48)*100 + \
								((uint16_t)com_buffer[11]-48)*10 + ((uint16_t)com_buffer[12]-48);
								BKP_WriteBackupRegister(Tel_part2,  Tel_temp);
								
								Tel_temp = ((uint16_t)com_buffer[13]-48)*100 + ((uint16_t)com_buffer[14]-48)*10 + ((uint16_t)com_buffer[15]-48);
								BKP_WriteBackupRegister(Tel_part3,  Tel_temp);
								
//								printf("\r\n");
								GPIO_SetBits(GPIOA, GPIO_Pin_1);
								OSTimeDlyHMSM(0, 0, 0, 100);
								GPIO_ResetBits(GPIOA, GPIO_Pin_1);
								OSTimeDlyHMSM(0, 0, 0, 100);
								
								T_485_EN1;
								return_cal(Tel_temp >> 8, Tel_temp & 0x00ff, CAL_R);
								R_485_EN1;

                break;
								
						case R_unwork1: 	//0xC5 内阻检测板失效
									
//								printf("R_unwork!");
								R_unwork = TRUE;
								EE_WriteVariable(R_unwork_Addr, 0x01);//内阻检测板失效标志
//								printf("\r\n");
								GPIO_SetBits(GPIOA, GPIO_Pin_1);
								OSTimeDlyHMSM(0, 0, 0, 100);
								GPIO_ResetBits(GPIOA, GPIO_Pin_1);
								OSTimeDlyHMSM(0, 0, 0, 100);
								
//								Com_485TX_Enable;
//								return_cal(R_unwork >> 8, R_unwork & 0x00ff, CAL_R);
//								Com_485TX_Disable;

                break;	
								
						case R_unwork2: 	//0xC6 内阻检测板恢复正常模式
									
//								printf("R_unwork!");
								R_unwork = FALSE;
								EE_WriteVariable(R_unwork_Addr, 0x00);//内阻检测板失效标志
//								printf("\r\n");
								GPIO_SetBits(GPIOA, GPIO_Pin_1);
								OSTimeDlyHMSM(0, 0, 0, 100);
								GPIO_ResetBits(GPIOA, GPIO_Pin_1);
								OSTimeDlyHMSM(0, 0, 0, 100);
								
//								Com_485TX_Enable;
//								return_cal(R_unwork >> 8, R_unwork & 0x00ff, CAL_R);
//								Com_485TX_Disable;

                break;	
								
							case C_rated: 	//0xC7 电池额定容量
									
//								printf("C_rated :");
								Rk =  (com_buffer[5]<<8)+com_buffer[6] ;	
								EE_WriteVariable(C_rated_Addr, Rk); 
//								printf("%d",Rk);
//								printf("\r\n");
								GPIO_SetBits(GPIOA, GPIO_Pin_1);
								OSTimeDlyHMSM(0, 0, 0, 100);
								GPIO_ResetBits(GPIOA, GPIO_Pin_1);
								OSTimeDlyHMSM(0, 0, 0, 100);
								
								T_485_EN1;
								return_cal(Rk >> 8, Rk & 0x00ff, CAL_R);
								R_485_EN1;

                break;			

            case W_ADDR: 		//0x81  EE aa 81 00 01 XX CRC ED 更改设备地址
                /*****************CRC_CHK**********************************************/
                data_len = com_buffer[3];
                data_len = (data_len << 8) + com_buffer[4];

                for(i  = 1; i <= data_len; i++) {
                    crc_data_buf[i - 1] = com_buffer[i + 4];
                }

                crc_receive = com_buffer[data_len + 4 + 1];
                crc_receive = (crc_receive << 8) + com_buffer[data_len + 4 + 2];
                crc_result = CRC16(crc_data_buf, data_len);

                if(crc_receive == crc_result) {
                    BKP_WriteBackupRegister(ADD, (uint16_t)com_buffer[5]);

                    if(com_buffer[5] == (uint8_t)(BKP_ReadBackupRegister(ADD) & 0x00ff)) {
                        T_485_EN1;
                        return_addr(W_ADDR);
                        R_485_EN1;
                    }
                }
								
                break;

            case W_R0_LIMIT: 	//0x82 更改电池内阻上限
                /*****************CRC_CHK**********************************************/
                data_len = com_buffer[3];
                data_len = (data_len << 8) + com_buffer[4];

                for(i  = 1; i <= data_len; i++) {
                    crc_data_buf[i - 1] = com_buffer[i + 4];
                }

                crc_receive = com_buffer[data_len + 4 + 1];
                crc_receive = (crc_receive << 8) + com_buffer[data_len + 4 + 2];
                crc_result = CRC16(crc_data_buf, data_len);

                if(crc_result == crc_receive) {
                    BKP_WriteBackupRegister(r_TOP_BETTRY, ((uint16_t)(com_buffer[5] << 8) + com_buffer[6]));

                    if((BKP_ReadBackupRegister(r_TOP_BETTRY) >> 8 == com_buffer[5])
                            && ((uint8_t)(BKP_ReadBackupRegister(r_TOP_BETTRY) & 0x00ff) == com_buffer[6])) {
                        T_485_EN1;
                        return_ack(W_R0_LIMIT, 0);
                        R_485_EN1;
                    }
                }

                break;

            case W_T_LIMIT: 	//0x83 更改电池温度上限
                /*****************CRC_CHK**********************************************/
                data_len = com_buffer[3];
                data_len = (data_len << 8) + com_buffer[4];

                for(i  = 1; i <= data_len; i++) {
                    crc_data_buf[i - 1] = com_buffer[i + 4];
                }

                crc_receive = com_buffer[data_len + 4 + 1];
                crc_receive = (crc_receive << 8) + com_buffer[data_len + 4 + 2];
                crc_result = CRC16(crc_data_buf, data_len);

                if(crc_result == crc_receive) {
                    BKP_WriteBackupRegister(T_TOP_BETTRY, ((int16_t)(com_buffer[5] << 8) + com_buffer[6]));

                    if((BKP_ReadBackupRegister(T_TOP_BETTRY) >> 8 == com_buffer[5])
                            && ((uint8_t)(BKP_ReadBackupRegister(T_TOP_BETTRY) & 0x00ff) == com_buffer[6])) {
                        T_485_EN1;
                        return_ack(W_T_LIMIT, 0);
                        R_485_EN1;
                    }
                }

                break;

            case W_V_LIMIT: 	//0x84 更改电池电压上限和下限
                /*****************CRC_CHK**********************************************/
                data_len = com_buffer[3];
                data_len = (data_len << 8) + com_buffer[4];

                for(i  = 1; i <= data_len; i++) {
                    crc_data_buf[i - 1] = com_buffer[i + 4];
                }

                crc_receive = com_buffer[data_len + 4 + 1];
                crc_receive = (crc_receive << 8) + com_buffer[data_len + 4 + 2];
                crc_result = CRC16(crc_data_buf, data_len);

                if(crc_result == crc_receive) {
                    /*向BKP中写入数据*/
                    BKP_WriteBackupRegister(V_TOP_BETTRY, ((int16_t)(com_buffer[5] << 8) + com_buffer[6]));
                    BKP_WriteBackupRegister(V_BOTTOM_BETTRY, ((int16_t)(com_buffer[7] << 8) + com_buffer[8]));

                    /*读取数据验证*/
                    if((BKP_ReadBackupRegister(V_TOP_BETTRY) >> 8 == com_buffer[5])
                            && ((uint8_t)(BKP_ReadBackupRegister(V_TOP_BETTRY) & 0x00ff) == com_buffer[6])
                            && (BKP_ReadBackupRegister(V_BOTTOM_BETTRY) >> 8 == com_buffer[7]
                                && ((uint8_t)(BKP_ReadBackupRegister(V_BOTTOM_BETTRY) & 0x00ff)) == com_buffer[8])) {
                        /*发送信息*/
                        T_485_EN1;
                        return_ack(W_V_LIMIT, 0);
                        R_485_EN1;
                    }
                }

                break;

            case W_B_CONFIG: 	//0X85 配置所管理电池组的信息
                /*****************CRC_CHK**********************************************/
                data_len = com_buffer[3];
                data_len = (data_len << 8) + com_buffer[4];

                for(i  = 1; i <= data_len; i++) {
                    crc_data_buf[i - 1] = com_buffer[i + 4];
                }

                crc_receive = com_buffer[data_len + 4 + 1];
                crc_receive = (crc_receive << 8) + com_buffer[data_len + 4 + 2];
                crc_result = CRC16(crc_data_buf, data_len);

                if(crc_result == crc_receive) {
                    BKP_WriteBackupRegister(V_BETTRY, (uint16_t)(com_buffer[5]));
                    BKP_WriteBackupRegister(N_BETTRY, (uint16_t)(com_buffer[6]));
                    BKP_WriteBackupRegister(C_BETTRY, (com_buffer[7] << 8) + com_buffer[8]);

                    if(((uint8_t)BKP_ReadBackupRegister(V_BETTRY) == com_buffer[5])
                            && ((uint8_t)BKP_ReadBackupRegister(N_BETTRY) == com_buffer[6])
                            && (BKP_ReadBackupRegister(C_BETTRY) >> 8 == com_buffer[7])
                            && ((uint8_t)(BKP_ReadBackupRegister(C_BETTRY) & 0x00ff) == com_buffer[8])) {
                        T_485_EN1;
                        return_ack(W_B_CONFIG, 0);
                        R_485_EN1;
                    }
                }

                break;

            default:
                break;
        }
    }

}



/*************************************************
 Function: USART3_SendData
 Description: 串口3 发送数据函数
 Input: 	 要发送的字节 8bit
 Return:
 Others:
 *************************************************/
static void USART3_SendData(unsigned char ch)
{
    USART_SendData(USART3, ch);

    /* Loop until the end of transmission */
    while(USART_GetFlagStatus(USART3, USART_FLAG_TC) == RESET) {
    }
}

/*************************************************
 Function: USART2_SendData
 Description: 串口2 发送数据函数
 Input: 	 要发送的字节 8bit
 Return:
 Others:
 *************************************************/
static void USART2_SendData(unsigned char ch)
{
    USART_SendData(USART2, ch);

    /* Loop until the end of transmission */
    while(USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET) {
    }
}

/*************************************************
 Function: USART1_SendData
 Description: 串口1 发送数据函数
 Input: 	 要发送的字节 8bit
 Return:
 Others:
 *************************************************/
static void USART1_SendData(unsigned char ch)
{
    USART_SendData(USART1, ch);

    /* Loop until the end of transmission */
    while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET) {
    }
}
/*************************************************
 Function: return_version
 Description: 返回设备固件版本，版本号由3个字节组成，
              具体由相应的宏定义完成
 Input:
 Return:
 Others:
 *************************************************/
static void return_version(void)
{
    uint8_t Version[3] = {MAIN_VERSION, SUB_VERSION, R_VERSION};
    uint16_t CRC16_Result = 0;
    uint8_t CRC16_hi, CRC16_lo = 0;
    CRC16_Result = CRC16(Version, 3);
    CRC16_lo = (uint8_t)(CRC16_Result & 0x00ff);
    CRC16_hi = (uint8_t)(CRC16_Result >> 8);
    USART2_SendData(0xEE);
    USART2_SendData((uint8_t)readADD());
    USART2_SendData(0x01);
    USART2_SendData(0x00);
    USART2_SendData(0x03);
    USART2_SendData(Version[0]);
    USART2_SendData(Version[1]);
    USART2_SendData(Version[2]);
    USART2_SendData(CRC16_hi);
    USART2_SendData(CRC16_lo);
    USART2_SendData(0xED);
}
/*************************************************
Function: return_id
Description: 返回设备的唯一ID
             12个8位字节组成
Input:
Return:
Others:
*************************************************/
static void return_id(void)
{
    uint8_t *UinqID;
    uint16_t CRC16_Result = 0;
    uint8_t CRC16_hi, CRC16_lo = 0;
    uint8_t i = 0;
    UinqID = (uint8_t *)0x1FFFF7E8;
    CRC16_Result = CRC16(UinqID, 12);
    CRC16_lo = (uint8_t)(CRC16_Result & 0x00ff);
    CRC16_hi = (uint8_t)(CRC16_Result >> 8);
    USART2_SendData(0xEE);
    USART2_SendData((uint8_t)readADD());
    USART2_SendData(0x11);
    USART2_SendData(0x00);
    USART2_SendData(0x0C);

    for(i = 0; i < 12; i++) {
        USART2_SendData(UinqID[i]);
    }

    USART2_SendData(CRC16_hi);
    USART2_SendData(CRC16_lo);
    USART2_SendData(0xED);
}
/*************************************************
 Function: nrf905_return_para_all
 Description: 无线返回所有参数，数据格式，四字节浮点型
 电压 x	 x  x  x  x
 温度 x  x  x  x  x
 内阻 x  x  x  x  x
 容量 x  x  x  x  x
 Input:
 Return:
 Others:
 *************************************************/
uint8_t *pre_p_ParaAll;

void nrf905_return_para_all(void)
{
	 uint8_t err = 0;
	 uint8_t *p_ParaAll;
	 p_ParaAll = (uint8_t *)OSMboxPend(ParaMbox, 2, &err);	
	 if(err == OS_ERR_NONE) 
		{
				pre_p_ParaAll = p_ParaAll;
		}
		/*通讯超时*/
		else if(err == OS_ERR_TIMEOUT) 
		{
				p_ParaAll = pre_p_ParaAll;
		}
	 n95_Sendout(p_ParaAll);					// 无线发送
}
/*************************************************
 Function: return_para_all
 Description: 返回所有参数，数据格式，四字节浮点型
 电压 x	 x  x  x  x
 温度 x  x  x  x  x
 内阻 x  x  x  x  x
 容量 x  x  x  x  x
 Input:
 Return:
 Others:
 *************************************************/
uint8_t *pre_p_ParaAll;

void return_para_all(void)
{
    //float *paraall;
    uint8_t err = 0;
    uint16_t i = 0;
    uint8_t *p_ParaAll;
    uint16_t CRC16_Result = 0;
    uint8_t CRC16_hi, CRC16_lo = 0;
		
    p_ParaAll = (uint8_t *)OSMboxPend(ParaMbox, 2, &err);	
		
		if(err == OS_ERR_NONE) 
		{
				pre_p_ParaAll = p_ParaAll;
		}
		/*通讯超时*/
		else if(err == OS_ERR_TIMEOUT) 
		{
				p_ParaAll = pre_p_ParaAll;
		}
   
    CRC16_Result = CRC16(p_ParaAll, 12);
    CRC16_lo = (uint8_t)(CRC16_Result & 0x00ff);
    CRC16_hi = (uint8_t)(CRC16_Result >> 8);
		
    USART2_SendData(0xEE);
    USART2_SendData((uint8_t)readADD());
    USART2_SendData(0x07);
    USART2_SendData(0x00);
    USART2_SendData(0x0C);
    for(i = 0; i < 12; i++) 
		{
        USART2_SendData(p_ParaAll[i]);
    }
    USART2_SendData(CRC16_hi);
    USART2_SendData(CRC16_lo);
    USART2_SendData(0xED);

}

/*************************************************
 Function: r485_Sendout()
 Description: 用rs485发送信息
 Input:
 Return:
 Others:
 *************************************************/

void r485_Sendout(uint8_t *p)
{
    int i = 0;
//    uint16_t CRC16_Result = 0;
//    uint8_t CRC16_hi, CRC16_lo = 0;
//		uint8_t Length = sizeof(p)/sizeof(uint8_t);
		 
//    CRC16_Result = CRC16(p, Length);
//    CRC16_lo = (uint8_t)(CRC16_Result & 0x00ff);
//    CRC16_hi = (uint8_t)(CRC16_Result >> 8);
    i = -1;
		do
		{
				i++;
        USART3_SendData(p[i]);
    }while(p[i]!=0xED);
//    USART3_SendData(CRC16_hi);
//    USART3_SendData(CRC16_lo);
//    USART3_SendData(0xED);

}


/* 返回所有电池当前的电压 */
static void return_all_voltage()
{
    uint8_t error = 0;
    uint16_t i = 0;
    uint8_t *buf;
    uint8_t voltage[72];
    uint16_t crc16_result = 0;
//    buf = (uint8_t *)OSMboxPend(ParaMbox, 0, &error);
//    memcpy(voltage, buf, 96);
    crc16_result = CRC16(voltage, 72);
    USART2_SendData(0xEE);
    USART2_SendData((uint8_t)readADD());
    USART2_SendData(0x04);
    USART2_SendData(0x00);
    USART2_SendData(0x48);

    for(i = 0; i < 72; i++) {
//        USART2_SendData(voltage[i]);
				USART2_SendData(0x00);
    }

    USART2_SendData(crc16_result >> 8);
    USART2_SendData(crc16_result & 0x00ff);
    USART2_SendData(0xED);
}
/* 返回所有电池当前的电压 */
static void return_config()
{
    uint8_t error = 0;
    uint16_t i = 0;
    uint8_t *buf;
    uint8_t voltage[4];
    uint16_t crc16_result = 0;
//    buf = (uint8_t *)OSMboxPend(ParaMbox, 0, &error);

//    memcpy(voltage, buf, 4);
		voltage[0] =0x0C;
		voltage[1] =0x24;
		voltage[2] =0x00;
		voltage[3] =0x64;

    crc16_result = CRC16(voltage,4);
    USART2_SendData(0xEE);
    USART2_SendData((uint8_t)readADD());
    USART2_SendData(0x03);
    USART2_SendData(0x00);
    USART2_SendData(0x04);

    for(i = 0; i < 4; i++) {
        USART2_SendData(voltage[i]);
    }

    USART2_SendData(crc16_result >> 8);
    USART2_SendData(crc16_result & 0x00ff);
    USART2_SendData(0xED);
}

/* 返回所有电池当前的温度 */
static void return_all_temp()
{
    uint8_t error = 0;
    uint16_t i = 0;
    uint8_t *buf;
    uint8_t temp[96];
    uint16_t crc16_result = 0;
//    buf = (uint8_t *)OSMboxPend(ParaMbox, 0, &error);
//    memcpy(temp, buf + 96, 96);
    crc16_result = CRC16(temp, 96);
    USART2_SendData(0xEE);
    USART2_SendData((uint8_t)readADD());
    USART2_SendData(0x05);
    USART2_SendData(0x00);
    USART2_SendData(0x40);

    for(i = 0; i < 96; i++) {
//        USART2_SendData(temp[i]);
				USART2_SendData(0x00);
    }

    USART2_SendData(crc16_result >> 8);
    USART2_SendData(crc16_result & 0x00ff);
    USART2_SendData(0xED);
}


/* 返回所有电池当前的内阻 */
static void return_all_res()
{
    uint8_t error = 0;
    uint16_t i = 0;
    uint8_t *buf;
    uint8_t temp[96];
    uint16_t crc16_result = 0;
    buf = (uint8_t *)OSMboxPend(ParaMbox, 0, &error);
    memcpy(temp, buf + 96 * 2, 96);
    crc16_result = CRC16(temp, 96);
    USART2_SendData(0xEE);
    USART2_SendData((uint8_t)readADD());
    USART2_SendData(0x0e);
    USART2_SendData(0x00);
    USART2_SendData(0x40);

    for(i = 0; i < 96; i++) {
        USART2_SendData(temp[i]);
    }

    USART2_SendData(crc16_result >> 8);
    USART2_SendData(crc16_result & 0x00ff);
    USART2_SendData(0xED);
}



/* 返回所有电池当前容量 */
static void return_all_capacity()
{
    uint8_t error = 0;
    uint16_t i = 0;
    uint8_t *buf;
    uint8_t capacity[96];
    uint16_t crc16_result = 0;
    buf = (uint8_t *)OSMboxPend(ParaMbox, 0, &error);
    memcpy(capacity, buf + 96 * 3, 96);
    crc16_result = CRC16(capacity, 96);
    USART2_SendData(0xEE);
    USART2_SendData((uint8_t)readADD());
    USART2_SendData(0x0e);
    USART2_SendData(0x00);
    USART2_SendData(0x40);

    for(i = 0; i < 96; i++) {
        USART2_SendData(capacity[i]);
    }

    USART2_SendData(crc16_result >> 8);
    USART2_SendData(crc16_result & 0x00ff);
    USART2_SendData(0xED);
}


/*************************************************
 Function: return_addr
 Description: 返回从机设备的地址，该函数会被读地址和更改地址指令
 Input:code--- R_ADD/W_ADD
 Return:
 Others:
 *************************************************/
static void return_addr(uint8_t Code)
{
    uint16_t CRC16_Result = 0;
    uint8_t CRC16_hi, CRC16_lo = 0;
    uint8_t add;
    add = (uint8_t)readADD();
    CRC16_Result = CRC16(&add, 1);
    CRC16_lo = (uint8_t)(CRC16_Result & 0x00ff);
    CRC16_hi = (uint8_t)(CRC16_Result >> 8);
    USART2_SendData(0xEE);
    USART2_SendData(add);
    USART2_SendData(Code);
    USART2_SendData(0x00);
    USART2_SendData(0x01);
    USART2_SendData(add);
    USART2_SendData(CRC16_hi);
    USART2_SendData(CRC16_lo);
    USART2_SendData(0xED);
}

/*************************************************
 Function: return_ack
 Description: 返回指令执行的情况
 Input:  code---指令代码
         ACK ---返回代码 00 01
 Return:
 Others:
 *************************************************/
static void return_ack(uint8_t Code , uint8_t ACK)
{
    uint16_t CRC16_Result = 0;
    uint8_t CRC16_hi, CRC16_lo = 0;
    CRC16_Result = CRC16(&ACK, 1);
    CRC16_lo = (uint8_t)(CRC16_Result & 0x00ff);
    CRC16_hi = (uint8_t)(CRC16_Result >> 8);
    USART2_SendData(0xEE);
    USART2_SendData((uint8_t)readADD());
    USART2_SendData(Code);
    USART2_SendData(0x00);
    USART2_SendData(0x01);
    USART2_SendData(ACK);
    USART2_SendData(CRC16_hi);
    USART2_SendData(CRC16_lo);
    USART2_SendData(0xED);
}


/* 返回校准结果 */
static void return_cal(uint8_t hi_8, uint8_t low_8, uint8_t flag)
{
    char data[2];
    uint16_t crc_result = 0;
    data[0] = hi_8;
    data[1] = low_8;
    crc_result = CRC16((unsigned char *)data, 2);
    USART2_SendData(0xEE);
    USART2_SendData((uint8_t)readADD());	//address

    switch(flag) {
        case CAL_V:
            USART2_SendData(0xc0);
            break;

        case CAL_T:
            USART2_SendData(0xc1);
            break;

        case CAL_R:
            USART2_SendData(0xc2);

        default:
            break;
    }

    USART2_SendData(0x00);
    USART2_SendData(0x02);
    USART2_SendData(hi_8);
    USART2_SendData(low_8);
    USART2_SendData(crc_result >> 8);
    USART2_SendData(crc_result & 0x00ff);
    USART2_SendData(0xED);
}

