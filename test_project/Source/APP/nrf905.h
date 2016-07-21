/*--------------------------------------------------------------------------------------
      ####            #####       ##        ##     ###      ############################  
  ############    ############    ####    #####   #####     ############################                      
 #####     ####   ####     ####   ####    ####   #######         ####                                        
 ####      ####   ####     ####    ####  ####    #######         ####                      
 ####      ####   ####     ####    ####  ####   #########        ####      ############# 
 ####      ####   ####     ####     ########    #### ####        ####      #############          
 ####      ####   ####     ####     ########   ####   ####       ####      ####                   
 ####      ####   ####     ####      ######    ####   ####       ####      ####                            
 ####      ####    ##########         ####    ####     ####      ####        ########### 
  ##        ##         ###             ##      ##       ###       ##            ########
  
				文件:	nRF905.h
				描述:	低于1GHz无线数传模块nRF905驱动程序头文件
				编写:	NOVATE copyright 2011
				版本:	2011-08-24 V1.0
--------------------------------------------------------------------------------------*/

#ifndef _NRF905_SPI_DEF_
#define _NRF905_SPI_DEF_
#include "includes.h"

/////////nRF905的定义////////////

//sbit nPin_TX_EN	 = P0^0;	// TX/RX选择 (MCU output)  ////1为TX//0为RX//
//sbit nPin_TRX_CE = P0^1;	// TX/RX使能 (MCU output)  ////发送或接收时一直为1//////
//sbit nPin_PWR_UP = P0^2;	// 芯片上电  (MCU output)  ////发送或接收时一直为1//////
#define nPin_TX_EN_H  GPIO_SetBits(GPIOE,GPIO_Pin_14)
#define nPin_TX_EN_L  GPIO_ResetBits(GPIOE,GPIO_Pin_14)
#define nPin_TRX_CE_H  GPIO_SetBits(GPIOE,GPIO_Pin_13) 
#define nPin_TRX_CE_L  GPIO_ResetBits(GPIOE,GPIO_Pin_13) 
#define nPin_PWR_UP_H  GPIO_SetBits(GPIOE,GPIO_Pin_15)
#define nPin_PWR_UP_L  GPIO_ResetBits(GPIOE,GPIO_Pin_15) 

////////nRF905状态端口//////////////////
//sbit nPin_CD	 = P3^2;	// 载波检测  (MCU input)
//sbit nPin_AM	 = P3^7;	// 地址匹配  (MCU input)
//sbit nPin_DR	 = P0^3;	// TX/RX完成 (MCU input)
#define nPin_CD_H GPIO_SetBits(GPIOE,GPIO_Pin_12)
#define nPin_CD_L GPIO_ResetBits(GPIOE,GPIO_Pin_12)
//#define nPin_AM_H GPIO_SetBits(GPIOB,GPIO_Pin_8)
//#define nPin_AM_L GPIO_ResetBits(GPIOB,GPIO_Pin_8)
#define nPin_DR_H GPIO_SetBits(GPIOE,GPIO_Pin_11)
#define nPin_DR_L GPIO_ResetBits(GPIOE,GPIO_Pin_11)

#define nPin_CD GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_12)
#define nPin_DR GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_11)

/////////下面的4个管脚是控制SPI的//////SPI管脚定义	Define SPI pins//////////////////////
//sbit nPin_MISO	 = P0^4;	// 数据输出  (MCU input)
//sbit nPin_MOSI	 = P0^5;	// 数据输入  (MCU output)
//sbit nPin_SCK	 = P0^6;	// 时钟输入  (MCU output)
//sbit nPin_CSN	 = P0^7;	// 芯片选择  (MCU output)

#define nPin_MISO	GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_6)
#define nPin_MOSI_H  GPIO_SetBits(GPIOA,GPIO_Pin_7) 
#define nPin_MOSI_L  GPIO_ResetBits(GPIOA,GPIO_Pin_7) 
#define nPin_SCK_H  GPIO_SetBits(GPIOA,GPIO_Pin_5)
#define nPin_SCK_L  GPIO_ResetBits(GPIOA,GPIO_Pin_5) 
#define nPin_CSN_H  GPIO_SetBits(GPIOA,GPIO_Pin_4)
#define nPin_CSN_L  GPIO_ResetBits(GPIOA,GPIO_Pin_4) 



// 用户变量 
//extern uint8 code n95_RF_Addr[];		// TX,RX地址,可自行更改
//extern uint8 n95_RX_Buff[];				// 接收数据缓冲区
//extern uint8 n95_TX_Buff[];				// 发送数据缓冲区
extern uint8_t n95_RF_Addr[];		// TX,RX地址,可自行更改
extern uint8_t n95_RX_Buff[];				// 接收数据缓冲区
extern uint8_t n95_TX_Buff[];				// 发送数据缓冲区

// 用户函数
void n95_Init_IO(void);
void n95_Init_Dev(void);
void n95_Sendout(uint8_t*);
uint8_t n95_Check_DR(uint8_t*);

void Nrf905_Test(void);


// SPI命令字 SPI Commands
#define nCMD_W_CONFIG			0x00	// 写配置寄存器
#define nCMD_R_CONFIG 			0x10	// 读配置寄存器
#define nCMD_W_TX_PAYLOAD 		0x20	// 写TXFIFO
#define nCMD_R_TX_PAYLOAD 		0x21	// 读TXFIFO
#define nCMD_W_TX_ADDRESS 		0x22	// 写TX地址
#define nCMD_R_TX_ADDRESS 		0x23	// 读TX地址
#define nCMD_R_RX_PAYLOAD 		0x24	// 读RXFIFO
#define nCMD_CHANNEL_CONFIG 	0x80	// 频率设置

// 寄存器配置
// byte1
#define nRCD_AUTO_RETRAN_disanble			0<<5	// 禁用自动重发
#define nRCD_AUTO_RETRAN_enanble			1<<5	// 启用自动重发

#define nRCD_RX_RED_PWR_disanble			0<<4	// 禁用低功耗RX模式
#define nRCD_RX_RED_PWR_enanble				1<<4	// 启用低功耗RX模式

#define nRCD_PA_PWR_n10dBm			(0<<3)|(0<<2)	// 输出功率为-10dBm
#define nRCD_PA_PWR_n2dBm			(0<<3)|(1<<2)	// 输出功率为-2dBm
#define nRCD_PA_PWR_6dBm			(1<<3)|(0<<2)	// 输出功率为6dBm
#define nRCD_PA_PWR_10dBm			(1<<3)|(1<<2)	// 输出功率为10dBm

#define nRCD_HFREQ_PLL_433MHz				0<<1	// 工作在433频段
#define nRCD_HFREQ_PLL_868_915MHz			1<<1	// 工作在868,915频段

// byte2
#define nRCD_TX_AFW_1byte	 (0<<6)|(0<<5)|(1<<4)	// TX地址宽度为1byte
#define nRCD_TX_AFW_2byte	 (0<<6)|(1<<5)|(0<<4)	// TX地址宽度为2byte
#define nRCD_TX_AFW_3byte	 (0<<6)|(1<<5)|(1<<4)	// TX地址宽度为3byte
#define nRCD_TX_AFW_4byte	 (1<<6)|(0<<5)|(0<<4)	// TX地址宽度为4byte

#define nRCD_RX_AFW_1byte	 	  (0<<2)|(0<<1)|1	// RX地址宽度为1byte
#define nRCD_RX_AFW_2byte	 	  (0<<2)|(1<<1)|0	// RX地址宽度为2byte
#define nRCD_RX_AFW_3byte	 	  (0<<2)|(1<<1)|1	// RX地址宽度为3byte
#define nRCD_RX_AFW_4byte	 	  (1<<2)|(0<<1)|0	// RX地址宽度为4byte

// byte9
#define nRCD_CRC_MODE_8crc					0<<7	// 8bitCRC
#define nRCD_CRC_MODE_16crc					1<<7	// 16bitCRC

#define nRCD_CRC_EN_disanble				0<<6	// 禁用CRC
#define nRCD_CRC_EN_enable					1<<6	// 启用CRC

#define nRCD_XOF_4MHz	 	 (0<<5)|(0<<4)|(0<<3)	// 外部晶振频率为4MHz
#define nRCD_XOF_8MHz	 	 (0<<5)|(0<<4)|(1<<3)	// 外部晶振频率为8MHz
#define nRCD_XOF_12MHz	 	 (0<<5)|(1<<4)|(0<<3)	// 外部晶振频率为12MHz
#define nRCD_XOF_16MHz	 	 (0<<5)|(1<<4)|(1<<3)	// 外部晶振频率为16MHz
#define nRCD_XOF_20MHz	 	 (1<<5)|(0<<4)|(0<<3)	// 外部晶振频率为20MHz

#define nRCD_UP_CLK_EN_disanble				0<<2	// 禁用外部时钟输出
#define nRCD_UP_CLK_EN_enable				1<<2	// 启用外部时钟输出

#define nRCD_UP_CLK_FREQ_4MHz			(0<<1)|0	// 时钟输出为4MHz
#define nRCD_UP_CLK_FREQ_2MHz			(0<<1)|1	// 时钟输出为2MHz
#define nRCD_UP_CLK_FREQ_1MHz			(1<<1)|0	// 时钟输出为1MHz
#define nRCD_UP_CLK_FREQ_500kHz			(1<<1)|1	// 时钟输出为500kHz

#endif
