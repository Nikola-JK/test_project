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
  
				文件:	nRF905.c
				描述:	低于1GHz无线数传模块nRF905驱动程序
				编写:	NOVATE copyright 2012
				版本:	2012-04-09 V1.2
--------------------------------------------------------------------------------------*/

#include "includes.h"
#include "nrf905.h"
GPIO_InitTypeDef GPIO_InitStructure;

#define RF_CH			220								// RF射频通道 0~511	(工作频率 = 422.4MHz + RF_CH × 0.1MHz )
#define RF_DATA_WIDTH	12								// RF数据宽度 1~32byte

uint8_t n95_RF_Addr[4]={0xBA,0xBA,0xBA,0xBA};		// TX,RX地址,可自行更改
uint8_t n95_TF_Addr[4]={0xBA,0xBB,0xBC,0xBD};		// TX,RX地址,可自行更改
uint8_t n95_RX_Buff[RF_DATA_WIDTH]={0};			// 接收数据缓冲区
uint8_t n95_TX_Buff[RF_DATA_WIDTH]={0};	// 发送数据缓冲区

bool FLAG_Nrf905_Used = FALSE;	//标记无线模块有没有插着
extern OS_EVENT *Nrf905Sem;

void nRF905_GPIO_Configuration(void)
{ 
	// Enable GPIOA, GPIOE clocks */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOE, ENABLE);
	 
	//初始化TXEN POW TRX CD DR CSN SCK MOSI
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15; 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz; 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 
	GPIO_Init(GPIOE, &GPIO_InitStructure); 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_5 | GPIO_Pin_4; 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz; 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 
	GPIO_Init(GPIOA, &GPIO_InitStructure); 	
	
	//初始化MISO
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6; 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz; 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; 
	GPIO_Init(GPIOA, &GPIO_InitStructure);
}
// 函数: n95_Init_IO()
// 描述: 初始化nRF905引脚状态
// 参数: 无
void n95_Init_IO(void)
{
	nRF905_GPIO_Configuration();
//	nPin_CD_L;										// CD    置低
	nPin_PWR_UP_H;									// PWR_UP置高,nRF905进入上电模式
	nPin_TRX_CE_L;									// TRX_CE置低,进入待机和SPI操作模式
	nPin_SCK_L;            							// SCK   置低
	
		//将DR、AM、CD引脚设为浮空输入模式
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11 | GPIO_Pin_12; 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz; 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; 
	GPIO_Init(GPIOE, &GPIO_InitStructure); 
}

// 函数: n1P_SPI_WR_Byte()
// 描述: 通过SPI写一个byte到nRF24L01+,同时从nRF24L01+读取一个byte
// 参数: byte 	待写入字节数据
//		 return 读取到的字节数据
uint8_t n95_SPI_WR_Byte(uint8_t byte)
{	
		uint8_t i;
//		OSTimeDlyHMSM(0,0,0,2);	//硬件不好时通讯慢点否则易出错
   	for(i=0; i<8; i++){          						// 循环8次

			if(byte & 0x80)
			{
				nPin_MOSI_H;  					// 将byte最高位输出到MOSI
			}else
			{
				nPin_MOSI_L;
			}
   		byte <<= 1;             						// 低一位移位到最高位
   		nPin_SCK_H;               					// 拉高SCK，nRF24L01从MOSI读入1位数据，同时从MISO输出1位数据
   		byte |= nPin_MISO;       						// 读MISO到byte最低位
   		nPin_SCK_L;            						// SCK置低
   	}
    return(byte);           							// 返回读出的一字节
}

// 函数: n1P_Init_Dev()
// 描述: 初始化nRF905,并将其转换为接收状态
// 参数: 无
void n95_Init_Dev(void)
{
	uint8_t i=0;

	nPin_PWR_UP_H;									// PWR_UP置高,nRF905进入上电模式
	nPin_TRX_CE_L;									// TRX_CE置低,进入待机和SPI操作模式
	nPin_CSN_L;										// CSN置低,   进入SPI操作模式。CSN为低时，SPI接口开始等待一条指令。任何一条新指令均由CSN的由高到低的转换开始。
	
	n95_SPI_WR_Byte(nCMD_W_CONFIG);						// 向nRF905发送"写配置寄存器命令"
	n95_SPI_WR_Byte(RF_CH & 0xFF);						// RF通道bit7:0
	n95_SPI_WR_Byte(nRCD_AUTO_RETRAN_disanble			// 禁用自动重发 
				  | nRCD_RX_RED_PWR_disanble			// 禁用低功耗RX模式
				  | nRCD_PA_PWR_10dBm					// 输出功率为10dBm
				  | nRCD_HFREQ_PLL_433MHz				// 工作在433频段
				  | (RF_CH>>8) );						// RF通道bit8

	n95_SPI_WR_Byte(nRCD_TX_AFW_4byte					// TX地址宽度为4byte
				  | nRCD_RX_AFW_4byte);					// RX地址宽度为4byte

	n95_SPI_WR_Byte(RF_DATA_WIDTH);						// RX数据宽度
	n95_SPI_WR_Byte(RF_DATA_WIDTH);						// TX数据宽度

	for(i=0; i<4; i++){
		n95_SPI_WR_Byte(n95_RF_Addr[i]);				// RX地址 byte0~3
	}

	n95_SPI_WR_Byte(nRCD_CRC_MODE_16crc					// 16bitCRC
				  | nRCD_CRC_EN_enable					// 启用CRC
				  |	nRCD_XOF_16MHz						// 外部晶振频率为16MHz
				  |	nRCD_UP_CLK_EN_disanble				// 禁用外部时钟输出
				  | nRCD_UP_CLK_FREQ_4MHz);				// 时钟输出为4MHz

	nPin_CSN_H;										// CSN置高,   退出SPI操作模式
	nPin_TX_EN_L;										// TX_EN置低 ,进入接收模式
	nPin_TRX_CE_H;									// TRX_CE置高,进入工作模式
}

// 函数: n1P_Turn_TX()
// 描述: 通过nRF905发送数据,数据发送结束后返回接收模式
// 参数: p 发送数据存放地址
void n95_Sendout(uint8_t *p)
{
	uint8_t i=0;
	nPin_PWR_UP_H;									// PWR_UP置高,nRF905进入上电模式
	nPin_TRX_CE_L;									// TRX_CE置低,进入待机和SPI操作模式
	nPin_TX_EN_H;										// TX_EN置高 ,进入发送模式
	nPin_CSN_L;										// CSN置低,   进入SPI操作模式

	n95_SPI_WR_Byte(nCMD_W_TX_ADDRESS);					// 向nRF905写入"写TX地址"指令
	
	for(i=0; i<4; i++){
		n95_SPI_WR_Byte(n95_TF_Addr[i]);				// 写入TX地址 byte0~3,注意此处应与"nRCD_TX_AFW"的配置一致
	}
	nPin_CSN_H;										// CSN置高,   退出SPI操作模式

	nPin_CSN_L;										// CSN置低,   进入SPI操作模式
	n95_SPI_WR_Byte(nCMD_W_TX_PAYLOAD);					// 向nRF905写入"写TX数据"指令

	for(i=0; i<RF_DATA_WIDTH; i++){
		n95_SPI_WR_Byte(p[i]);							// 写入待发送数据
	}

	nPin_CSN_H;										// CSN置高,   退出SPI操作模式
	nPin_TRX_CE_H;									// TRX_CE置高,进入发送模式
	while(nPin_DR == 0);								// 等待DR置高,发送完成
	nPin_TX_EN_L;										// TX_EN置低 ,进入接收模式
}

// 函数: Nrf905_Test()，由n95_Sendout()改编而来。jkl
// 描述: 测试905存不存在，并置相关的标志位。
// 参数: 无
void Nrf905_Test(void)
{
		uint16_t i=0;
		uint8_t p[6]={0x00,0x00,0x00,0x00,0x00,0x00};	//无实际意义，只是发送一数据，检测905模块存不存在。
		uint8_t n95_Test_Addr[4]={0xBB,0xBB,0xBB,0xBB};		// 其他地址，不和发送接收地址相同即可。

		nPin_PWR_UP_H;									// PWR_UP置高,nRF905进入上电模式
		nPin_TRX_CE_L;									// TRX_CE置低,进入待机和SPI操作模式
		nPin_TX_EN_H;										// TX_EN置高 ,进入发送模式
		nPin_CSN_L;										// CSN置低,   进入SPI操作模式

		n95_SPI_WR_Byte(nCMD_W_TX_ADDRESS);					// 向nRF905写入"写TX地址"指令
		
		for(i=0; i<4; i++){
			n95_SPI_WR_Byte(n95_Test_Addr[i]);				// 写入TX地址 byte0~3,注意此处应与"nRCD_TX_AFW"的配置一致
		}
		nPin_CSN_H;										// CSN置高,   退出SPI操作模式

		nPin_CSN_L;										// CSN置低,   进入SPI操作模式
		n95_SPI_WR_Byte(nCMD_W_TX_PAYLOAD);					// 向nRF905写入"写TX数据"指令

		for(i=0; i<RF_DATA_WIDTH; i++){
			n95_SPI_WR_Byte(p[i]);							// 写入待发送数据
		}

		nPin_CSN_H;										// CSN置高,   退出SPI操作模式
		nPin_TRX_CE_H;									// TRX_CE置高,进入发送模式
//		while(nPin_DR == 0);								// 等待DR置高,发送完成
		for(i=0; i<50; i++)
		{
				if(nPin_DR == 1)//数据发送完成或接收到新数据都会硬件DR置1
				{
						FLAG_Nrf905_Used = TRUE;	//有905模块
						OSSemPost(Nrf905Sem);			//打开905功能
				}
				OSTimeDlyHMSM(0,0,0,5);
		}
		nPin_TX_EN_L;										// TX_EN置低 ,进入接收模式
}


// 函数: n1P_Check_DR()
// 描述: 检查nRF905是否接收到数据,如接收到数据将数据存入接收缓冲区,并返回成功标志
// 参数: p		接收数据存放地址
//		 return 接收成功标志,为1时表明数据接收成功
uint8_t n95_Check_DR(uint8_t *p)
{	uint8_t i=0;

	if(nPin_DR == 1){	///数据发送完成或接收到新数据都会硬件DR置1//
		nPin_TRX_CE_L;								// TRX_CE置低,进入待机模式
		nPin_CSN_L;									// CSN置低,   进入SPI操作模式

		n95_SPI_WR_Byte(nCMD_R_RX_PAYLOAD);				// 向nRF905写入"读取RXFIFO"指令

		for(i=0; i<RF_DATA_WIDTH; i++){
			p[i] = 	n95_SPI_WR_Byte(0);					// 读取接收数据
		}

		nPin_CSN_H;									// CSN置高,   退出SPI操作模式
		nPin_TRX_CE_H;								// TRX_CE置高,进入工作模式
		return(1);										// 返回接收成功标志
	}
	return(0);											// 返回未接收到数据标志
}
