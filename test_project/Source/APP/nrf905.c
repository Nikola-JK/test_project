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
  
				�ļ�:	nRF905.c
				����:	����1GHz��������ģ��nRF905��������
				��д:	NOVATE copyright 2012
				�汾:	2012-04-09 V1.2
--------------------------------------------------------------------------------------*/

#include "includes.h"
#include "nrf905.h"
GPIO_InitTypeDef GPIO_InitStructure;

#define RF_CH			220								// RF��Ƶͨ�� 0~511	(����Ƶ�� = 422.4MHz + RF_CH �� 0.1MHz )
#define RF_DATA_WIDTH	12								// RF���ݿ�� 1~32byte

uint8_t n95_RF_Addr[4]={0xBA,0xBA,0xBA,0xBA};		// TX,RX��ַ,�����и���
uint8_t n95_TF_Addr[4]={0xBA,0xBB,0xBC,0xBD};		// TX,RX��ַ,�����и���
uint8_t n95_RX_Buff[RF_DATA_WIDTH]={0};			// �������ݻ�����
uint8_t n95_TX_Buff[RF_DATA_WIDTH]={0};	// �������ݻ�����

bool FLAG_Nrf905_Used = FALSE;	//�������ģ����û�в���
extern OS_EVENT *Nrf905Sem;

void nRF905_GPIO_Configuration(void)
{ 
	// Enable GPIOA, GPIOE clocks */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOE, ENABLE);
	 
	//��ʼ��TXEN POW TRX CD DR CSN SCK MOSI
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15; 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz; 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 
	GPIO_Init(GPIOE, &GPIO_InitStructure); 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_5 | GPIO_Pin_4; 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz; 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 
	GPIO_Init(GPIOA, &GPIO_InitStructure); 	
	
	//��ʼ��MISO
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6; 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz; 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; 
	GPIO_Init(GPIOA, &GPIO_InitStructure);
}
// ����: n95_Init_IO()
// ����: ��ʼ��nRF905����״̬
// ����: ��
void n95_Init_IO(void)
{
	nRF905_GPIO_Configuration();
//	nPin_CD_L;										// CD    �õ�
	nPin_PWR_UP_H;									// PWR_UP�ø�,nRF905�����ϵ�ģʽ
	nPin_TRX_CE_L;									// TRX_CE�õ�,���������SPI����ģʽ
	nPin_SCK_L;            							// SCK   �õ�
	
		//��DR��AM��CD������Ϊ��������ģʽ
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11 | GPIO_Pin_12; 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz; 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; 
	GPIO_Init(GPIOE, &GPIO_InitStructure); 
}

// ����: n1P_SPI_WR_Byte()
// ����: ͨ��SPIдһ��byte��nRF24L01+,ͬʱ��nRF24L01+��ȡһ��byte
// ����: byte 	��д���ֽ�����
//		 return ��ȡ�����ֽ�����
uint8_t n95_SPI_WR_Byte(uint8_t byte)
{	
		uint8_t i;
//		OSTimeDlyHMSM(0,0,0,2);	//Ӳ������ʱͨѶ��������׳���
   	for(i=0; i<8; i++){          						// ѭ��8��

			if(byte & 0x80)
			{
				nPin_MOSI_H;  					// ��byte���λ�����MOSI
			}else
			{
				nPin_MOSI_L;
			}
   		byte <<= 1;             						// ��һλ��λ�����λ
   		nPin_SCK_H;               					// ����SCK��nRF24L01��MOSI����1λ���ݣ�ͬʱ��MISO���1λ����
   		byte |= nPin_MISO;       						// ��MISO��byte���λ
   		nPin_SCK_L;            						// SCK�õ�
   	}
    return(byte);           							// ���ض�����һ�ֽ�
}

// ����: n1P_Init_Dev()
// ����: ��ʼ��nRF905,������ת��Ϊ����״̬
// ����: ��
void n95_Init_Dev(void)
{
	uint8_t i=0;

	nPin_PWR_UP_H;									// PWR_UP�ø�,nRF905�����ϵ�ģʽ
	nPin_TRX_CE_L;									// TRX_CE�õ�,���������SPI����ģʽ
	nPin_CSN_L;										// CSN�õ�,   ����SPI����ģʽ��CSNΪ��ʱ��SPI�ӿڿ�ʼ�ȴ�һ��ָ��κ�һ����ָ�����CSN���ɸߵ��͵�ת����ʼ��
	
	n95_SPI_WR_Byte(nCMD_W_CONFIG);						// ��nRF905����"д���üĴ�������"
	n95_SPI_WR_Byte(RF_CH & 0xFF);						// RFͨ��bit7:0
	n95_SPI_WR_Byte(nRCD_AUTO_RETRAN_disanble			// �����Զ��ط� 
				  | nRCD_RX_RED_PWR_disanble			// ���õ͹���RXģʽ
				  | nRCD_PA_PWR_10dBm					// �������Ϊ10dBm
				  | nRCD_HFREQ_PLL_433MHz				// ������433Ƶ��
				  | (RF_CH>>8) );						// RFͨ��bit8

	n95_SPI_WR_Byte(nRCD_TX_AFW_4byte					// TX��ַ���Ϊ4byte
				  | nRCD_RX_AFW_4byte);					// RX��ַ���Ϊ4byte

	n95_SPI_WR_Byte(RF_DATA_WIDTH);						// RX���ݿ��
	n95_SPI_WR_Byte(RF_DATA_WIDTH);						// TX���ݿ��

	for(i=0; i<4; i++){
		n95_SPI_WR_Byte(n95_RF_Addr[i]);				// RX��ַ byte0~3
	}

	n95_SPI_WR_Byte(nRCD_CRC_MODE_16crc					// 16bitCRC
				  | nRCD_CRC_EN_enable					// ����CRC
				  |	nRCD_XOF_16MHz						// �ⲿ����Ƶ��Ϊ16MHz
				  |	nRCD_UP_CLK_EN_disanble				// �����ⲿʱ�����
				  | nRCD_UP_CLK_FREQ_4MHz);				// ʱ�����Ϊ4MHz

	nPin_CSN_H;										// CSN�ø�,   �˳�SPI����ģʽ
	nPin_TX_EN_L;										// TX_EN�õ� ,�������ģʽ
	nPin_TRX_CE_H;									// TRX_CE�ø�,���빤��ģʽ
}

// ����: n1P_Turn_TX()
// ����: ͨ��nRF905��������,���ݷ��ͽ����󷵻ؽ���ģʽ
// ����: p �������ݴ�ŵ�ַ
void n95_Sendout(uint8_t *p)
{
	uint8_t i=0;
	nPin_PWR_UP_H;									// PWR_UP�ø�,nRF905�����ϵ�ģʽ
	nPin_TRX_CE_L;									// TRX_CE�õ�,���������SPI����ģʽ
	nPin_TX_EN_H;										// TX_EN�ø� ,���뷢��ģʽ
	nPin_CSN_L;										// CSN�õ�,   ����SPI����ģʽ

	n95_SPI_WR_Byte(nCMD_W_TX_ADDRESS);					// ��nRF905д��"дTX��ַ"ָ��
	
	for(i=0; i<4; i++){
		n95_SPI_WR_Byte(n95_TF_Addr[i]);				// д��TX��ַ byte0~3,ע��˴�Ӧ��"nRCD_TX_AFW"������һ��
	}
	nPin_CSN_H;										// CSN�ø�,   �˳�SPI����ģʽ

	nPin_CSN_L;										// CSN�õ�,   ����SPI����ģʽ
	n95_SPI_WR_Byte(nCMD_W_TX_PAYLOAD);					// ��nRF905д��"дTX����"ָ��

	for(i=0; i<RF_DATA_WIDTH; i++){
		n95_SPI_WR_Byte(p[i]);							// д�����������
	}

	nPin_CSN_H;										// CSN�ø�,   �˳�SPI����ģʽ
	nPin_TRX_CE_H;									// TRX_CE�ø�,���뷢��ģʽ
	while(nPin_DR == 0);								// �ȴ�DR�ø�,�������
	nPin_TX_EN_L;										// TX_EN�õ� ,�������ģʽ
}

// ����: Nrf905_Test()����n95_Sendout()�ı������jkl
// ����: ����905�治���ڣ�������صı�־λ��
// ����: ��
void Nrf905_Test(void)
{
		uint16_t i=0;
		uint8_t p[6]={0x00,0x00,0x00,0x00,0x00,0x00};	//��ʵ�����壬ֻ�Ƿ���һ���ݣ����905ģ��治���ڡ�
		uint8_t n95_Test_Addr[4]={0xBB,0xBB,0xBB,0xBB};		// ������ַ�����ͷ��ͽ��յ�ַ��ͬ���ɡ�

		nPin_PWR_UP_H;									// PWR_UP�ø�,nRF905�����ϵ�ģʽ
		nPin_TRX_CE_L;									// TRX_CE�õ�,���������SPI����ģʽ
		nPin_TX_EN_H;										// TX_EN�ø� ,���뷢��ģʽ
		nPin_CSN_L;										// CSN�õ�,   ����SPI����ģʽ

		n95_SPI_WR_Byte(nCMD_W_TX_ADDRESS);					// ��nRF905д��"дTX��ַ"ָ��
		
		for(i=0; i<4; i++){
			n95_SPI_WR_Byte(n95_Test_Addr[i]);				// д��TX��ַ byte0~3,ע��˴�Ӧ��"nRCD_TX_AFW"������һ��
		}
		nPin_CSN_H;										// CSN�ø�,   �˳�SPI����ģʽ

		nPin_CSN_L;										// CSN�õ�,   ����SPI����ģʽ
		n95_SPI_WR_Byte(nCMD_W_TX_PAYLOAD);					// ��nRF905д��"дTX����"ָ��

		for(i=0; i<RF_DATA_WIDTH; i++){
			n95_SPI_WR_Byte(p[i]);							// д�����������
		}

		nPin_CSN_H;										// CSN�ø�,   �˳�SPI����ģʽ
		nPin_TRX_CE_H;									// TRX_CE�ø�,���뷢��ģʽ
//		while(nPin_DR == 0);								// �ȴ�DR�ø�,�������
		for(i=0; i<50; i++)
		{
				if(nPin_DR == 1)//���ݷ�����ɻ���յ������ݶ���Ӳ��DR��1
				{
						FLAG_Nrf905_Used = TRUE;	//��905ģ��
						OSSemPost(Nrf905Sem);			//��905����
				}
				OSTimeDlyHMSM(0,0,0,5);
		}
		nPin_TX_EN_L;										// TX_EN�õ� ,�������ģʽ
}


// ����: n1P_Check_DR()
// ����: ���nRF905�Ƿ���յ�����,����յ����ݽ����ݴ�����ջ�����,�����سɹ���־
// ����: p		�������ݴ�ŵ�ַ
//		 return ���ճɹ���־,Ϊ1ʱ�������ݽ��ճɹ�
uint8_t n95_Check_DR(uint8_t *p)
{	uint8_t i=0;

	if(nPin_DR == 1){	///���ݷ�����ɻ���յ������ݶ���Ӳ��DR��1//
		nPin_TRX_CE_L;								// TRX_CE�õ�,�������ģʽ
		nPin_CSN_L;									// CSN�õ�,   ����SPI����ģʽ

		n95_SPI_WR_Byte(nCMD_R_RX_PAYLOAD);				// ��nRF905д��"��ȡRXFIFO"ָ��

		for(i=0; i<RF_DATA_WIDTH; i++){
			p[i] = 	n95_SPI_WR_Byte(0);					// ��ȡ��������
		}

		nPin_CSN_H;									// CSN�ø�,   �˳�SPI����ģʽ
		nPin_TRX_CE_H;								// TRX_CE�ø�,���빤��ģʽ
		return(1);										// ���ؽ��ճɹ���־
	}
	return(0);											// ����δ���յ����ݱ�־
}
