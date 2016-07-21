/*
*********************************************************************************************************
*	                                  
*	ģ������ : NAND Flash����ģ��    
*	�ļ����� : bsp_nand.c
*	��    �� : V1.0
*	˵    �� : �ṩNAND Flash (HY27UF081G2A�� 8bit 128K�ֽ� ��ҳ)�ĵײ�ӿں�������������ԭ������ֹת�ء�
*	�޸ļ�¼ :
*		�汾��  ����       ����    ˵��
*		v1.0    2011-05-25 armfly  ST�̼��� V3.5.0�汾��
*
*	Copyright (C), 2010-2011, ���������� www.armfly.com
*
*********************************************************************************************************
*/
#include "string.h"
#include "stdio.h"
#include "bsp_nand.h"

/*
	�����IAR��KEIL�ı༭�����Ķ����뽫�༭������������Ϊ�����壨9��/��ţ���������TAB����Ϊ4��
	���򣬷��򴦳��ֲ���������⡣

	�������Ƶĵط���
	��1���ڲ���NAND Flashʱ�����������һ����ѭ�������Ӳ�������쳣����������������
 		while( GPIO_ReadInputDataBit(GPIOG, GPIO_Pin_6) == 0 )
 		
 	��2��û������ECCУ�鹦�ܡ�ECC���Լ��1����2��bit�������ֻ��1��bit����������޸����bit�����
 		����2��bit��������ܼ�ⲻ����
 		
 	��3������д�ļ�����ʱ���ᵼ���ؽ�LUT��Ŀǰ���ؽ�LUT�Ĵ���ִ��Ч�ʻ������ߣ��д����ơ�

	��Ӳ��˵����
	������STM32F103ZE-EK���������õ�NAND FlahsΪ����ʿ��HY27UF081G2A 
	��1��NAND Flash��Ƭѡ�ź����ӵ�CPU��FSMC_NCE2���������NAND Flash�ĵ�ַ�ռ�Ϊ 0x70000000����CPU������
		�ֲ��FSMC�½�)
	��2����FSMC�������ж�������豸����TFT��SRAM��CH374T��NOR������˱���ȷ�����������豸��Ƭѡ���ڽ�ֹ
		״̬�����򽫳������߳�ͻ���� ���μ����ļ���ʼ��FSMC GPIO�ĺ�����


	��NAND Flash �ṹ���塿
     ��������16x4�ֽڣ�ÿpage 1024�ֽڣ�ÿ512�ֽ�һ��������ÿ��������Ӧ16�Լ��ı�������

	 ÿ��PAGE���߼��ṹ��ǰ��512Bx4����������������16Bx4�Ǳ�����
	��������������������������������������������������������������������������������������������������������������������������������
	�� Main area  ���� Main area  ���� Main area  ����Main area   ���� Spare area ���� Spare area ���� Spare area ����Spare area  ��
	��            ����            ����            ����            ����            ����            ����            ����            ��
	��   512B     ����    512B    ����    512B    ����    512B    ����    16B     ����     16B    ����     16B    ����    512B    ��
	��������������������������������������������������������������������������������������������������������������������������������
	
	 ÿ16B�ı��������߼��ṹ����:(�����Ƽ���׼��
	������������������������������������������������������������������������������������������������������������������������������������������������������������
	��  BI  ����RESER ����LSN0����LSN1����LSN2����RESER ����RESER ����RESER ����ECC0����ECC1����ECC2����ECC0����S-ECC1����S-ECC0����RESER ����RESER ����RESER ��
	��      ���� VED  ����    ����    ����    ���� VED  ���� VED  ���� VED  ����    ����    ����    ����    ����      ����      ���� VED  ���� VED  ���� VED  ��
	������������������������������������������������������������������������������������������������������������������������������������������������������������
	
	K9F1G08U0A �� HY27UF081G2A �Ǽ��ݵġ�оƬ����ʱ�����̱�֤оƬ�ĵ�1�����Ǻÿ顣����ǻ��飬���ڸÿ�ĵ�1��PAGE�ĵ�1���ֽ�
	���ߵ�2��PAGE������1��PAGE�����޷����Ϊ0xFFʱ���ĵ�1���ֽ�д���0xFFֵ��������ֵ������ģ�����ֱ���ж��Ƿ����0xFF���ɡ�
	
	ע�⣺������Щ����˵NAND Flash���̵�Ĭ�������ǽ������Ƕ��ڵ�1��PAGE�ĵ�6���ֽڴ������˵���Ǵ��󡣻������ڵ�6���ֽڽ���Բ���С������512�ֽڣ���NAND Flash
	���������е�NAND Flash���������׼������ڸ���NAND Flashʱ���ޱ���ϸ�Ķ�оƬ�������ֲᡣ
	

	Ϊ�˱�����NAND Flash ����ֲFat�ļ�ϵͳ�����Ƕ�16B�ı������������·��䷽��:
	������������������������������������������������������������������������������������������������������������������������������������
	�� BI ����USED����LBN0����LBN1����ECC0����ECC1����ECC2����ECC3����ECC4����ECC5����S-ECC1����S-ECC0����RSVD����RSVD����RSVD����RSVD��
	��    ����    ����    ����    ����    ����    ����    ����    ����    ����    ����      ����      ����    ����    ����    ����    ��
	������������������������������������������������������������������������������������������������������������������������������������
    - BI : �����־(Bad Block Identifier)��ÿ��BLOCK�ĵ�1��PAGE���ߵ�2��PAGE�ĵ�1���ֽ�ָʾ�ÿ��Ƿ񻵿顣0xFF��ʾ�ÿ飬����0xFF��ʾ���顣
    - USED : �ÿ�ʹ�ñ�־��0xFF��ʾ���п飻0xFE��ʾ���ÿ顣
    - LBN0 LBN1 : �߼����(Logic Block No) ����0��ʼ���롣ֻ��ÿ��BLOCK�ĵ�1��PAGE��Ч������PAGE���ֶι̶�Ϊ0xFF FF
    - ECC0 ~ ECC6 : 512B����������ECCУ�� �����������ṩECC�㷨��256�ֽڶ�Ӧ3���ֽڵ�ECC)
    - S-ECC1 S-ECC0 : LSN0��LSN2��ECCУ��
    - RSVD : �����ֽڣ�Reserved

	��������� & ĥ��ƽ�⡿
	(1) �ڲ�ȫ������s_usLUT[]�����򱣴�������š�������������߼���ĵ�ַӳ�䡣
	(2) ��ʽ��ʱ����98%�ĺÿ����������ݴ洢��ʣ���2%���ڱ������������滻����
	(3) д������512B)ʱ�������������Ϊ�գ���ֱ��д�룬���ٲ���Ҫ�Ŀ������������Ч���NAND Flash�������Ͷ�д���ܡ�
	(4) д����ʱ������������ݲ�Ϊ�գ����ĩβ��ʼ����һ�����п��滻���ɿ飬�滻����д������ɺ󣬽��ɿ����������עΪ���У�֮���ؽ�LUT��
	(5) �鸴��ʱ���������NAND FlashӲ����Copy-Back���ܣ������Դҳ���ڴ���д��Ŀ��ҳ��������������߶�дЧ�ʡ�
	(6) ĥ��ƽ�⻹����ȱ�ݣ�Ч�����á�ECCУ����δʵ�֡�
		
*/

/* ����NAND Flash��������ַ���������Ӳ�������� */
#define Bank2_NAND_ADDR    ((uint32_t)0x70000000)
#define Bank_NAND_ADDR     Bank2_NAND_ADDR 

/* �������NAND Flash�õ�3���� */
#define NAND_CMD_AREA		*(__IO uint8_t *)(Bank_NAND_ADDR | CMD_AREA)
#define NAND_ADDR_AREA		*(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA)
#define NAND_DATA_AREA		*(__IO uint8_t *)(Bank_NAND_ADDR | DATA_AREA)

/* �߼����ӳ������ÿ�������2%���ڱ��������������ά������1024�� LUT = Look Up Table */
static uint16_t s_usLUT[NAND_BLOCK_COUNT]; 

static uint16_t s_usValidDataBlockCount;	/* ��Ч�����ݿ���� */

static uint8_t s_ucTempBuf[NAND_PAGE_TOTAL_SIZE];	/* �󻺳�����2112�ֽ�. ���ڶ����Ƚ� */

static uint8_t NAND_BuildLUT(void);
static uint8_t FSMC_NAND_GetStatus(void);
static uint16_t NAND_FindFreeBlock (void);
static uint8_t NAND_MarkUsedBlock(uint32_t _ulBlockNo);
static void NAND_MarkBadBlock(uint32_t _ulBlockNo);
static uint16_t NAND_AddrToPhyBlockNo(uint32_t _ulMemAddr);
static uint8_t NAND_IsBufOk(uint8_t *_pBuf, uint32_t _ulLen, uint8_t _ucValue);
uint8_t NAND_WriteToNewBlock(uint32_t _ulPhyPageNo, uint8_t *_pWriteBuf, uint16_t _usOffset, uint16_t _usSize);
static uint8_t NAND_IsFreeBlock(uint32_t _ulBlockNo);

/*
*********************************************************************************************************
*	�� �� ��: FSMC_NAND_Init
*	����˵��: ����FSMC��GPIO����NAND Flash�ӿڡ�������������ڶ�дnand flashǰ������һ�Ρ�
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void FSMC_NAND_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure; 
	FSMC_NANDInitTypeDef FSMC_NANDInitStructure;
	FSMC_NAND_PCCARDTimingInitTypeDef  p;
	
	/* ʹ�� FSMC ʱ�� */
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_FSMC, ENABLE);
	
	/* ʹ��GPIOD��GPIOE��GPIOF��GPIOG ʱ�� */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOE | 
	                     RCC_APB2Periph_GPIOF | RCC_APB2Periph_GPIOG, ENABLE);
	
	/* GPIO ���� */
	/* ������CLE, ALE, D0-D3, NOE, NWE �� NCE2  NAND ��������Ϊ���ù��ܣ�������FSMC)  */
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_14 | GPIO_Pin_15 |  
	                             GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_4 | GPIO_Pin_5 | 
	                             GPIO_Pin_7;                                  
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOD, &GPIO_InitStructure); 
	
	/* ������ D4-D7 ��������Ϊ���ù��� */  
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10;
	GPIO_Init(GPIOE, &GPIO_InitStructure);
	
	/* NWAIT ��������. ������STM32F103ZE-EK������ȱʡδʹ��NWAIT������Ϊæ�źţ�ʹ�õ���INT2
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;   							 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(GPIOD, &GPIO_InitStructure); 
	*/
	
	/* INT2 ��������Ϊ�ڲ��������룬����æ�ź� */  
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;   
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;								 
	GPIO_Init(GPIOD, &GPIO_InitStructure);	//jkl
	
	/* FSMC ���� */
	p.FSMC_SetupTime = 0x1;
	p.FSMC_WaitSetupTime = 0x3;
	p.FSMC_HoldSetupTime = 0x2;
	p.FSMC_HiZSetupTime = 0x1;
	
	FSMC_NANDInitStructure.FSMC_Bank = FSMC_Bank2_NAND;							/* ����FSMC BANK �� */
	FSMC_NANDInitStructure.FSMC_Waitfeature = FSMC_Waitfeature_Enable;			/* ����ȴ�ʱ��ʹ�� */
	FSMC_NANDInitStructure.FSMC_MemoryDataWidth = FSMC_MemoryDataWidth_8b;		/* ���ݿ��� 8bit */
	FSMC_NANDInitStructure.FSMC_ECC = FSMC_ECC_Enable;							/* ECC������;�������ʹ�� */
	FSMC_NANDInitStructure.FSMC_ECCPageSize = FSMC_ECCPageSize_2048Bytes;		/* ECC ҳ���С */
	FSMC_NANDInitStructure.FSMC_TCLRSetupTime = 0x00;
	FSMC_NANDInitStructure.FSMC_TARSetupTime = 0x00;
	FSMC_NANDInitStructure.FSMC_CommonSpaceTimingStruct = &p;
	FSMC_NANDInitStructure.FSMC_AttributeSpaceTimingStruct = &p;
	
	FSMC_NANDInit(&FSMC_NANDInitStructure);
	
	/* FSMC NAND Bank ʹ�� */
	FSMC_NANDCmd(FSMC_Bank2_NAND, ENABLE);


		
}

/*
*********************************************************************************************************
*	�� �� ��: NAND_ReadID
*	����˵��: ��NAND Flash��ID��ID�洢���β�ָ���Ľṹ������С�
*	��    �Σ���
*	�� �� ֵ: 32bit��NAND Flash ID
*********************************************************************************************************
*/
uint32_t NAND_ReadID(void)
{
	uint32_t data = 0;	
	
	/* �������� Command to the command area */ 	
	NAND_CMD_AREA = 0x90;
	NAND_ADDR_AREA = 0x00;
	
	/* ˳���ȡNAND Flash��ID */	
	data = *(__IO uint32_t *)(Bank_NAND_ADDR | DATA_AREA);
	data =  ((data << 24) & 0xFF000000) | 
			((data << 8 ) & 0x00FF0000) | 
			((data >> 8 ) & 0x0000FF00) | 
			((data >> 24) & 0x000000FF) ;
	return data;
}

/*
*********************************************************************************************************
*	�� �� ��: FSMC_NAND_PageCopyBack
*	����˵��: ��һҳ���ݸ��Ƶ�����һ��ҳ��Դҳ��Ŀ��ҳ����ͬΪż��ҳ��ͬΪ����ҳ��
*	��    �Σ�- _ulSrcPageNo: Դҳ��
*             - _ulTarPageNo: Ŀ��ҳ��
*	�� �� ֵ: ִ�н����
*				- NAND_FAIL ��ʾʧ��
*				- NAND_OK ��ʾ�ɹ�
*
*	˵    ���������ֲ��Ƽ�����ҳ����֮ǰ����У��Դҳ��λУ�飬������ܻ����λ���󡣱�����δʵ�֡�
*
*********************************************************************************************************
*/
static uint8_t FSMC_NAND_PageCopyBack(uint32_t _ulSrcPageNo, uint32_t _ulTarPageNo)
{		
	NAND_CMD_AREA = NAND_CMD_COPYBACK_A;
	
	/* ����Դҳ��ַ �� ���� HY27UF081G2A
				  Bit7 Bit6 Bit5 Bit4 Bit3 Bit2 Bit1 Bit0
		��1�ֽڣ� A7   A6   A5   A4   A3   A2   A1   A0		(_usPageAddr ��bit7 - bit0)
		��2�ֽڣ� 0    0    0    0    A11  A10  A9   A8		(_usPageAddr ��bit11 - bit8, ��4bit������0)
		��3�ֽڣ� A19  A18  A17  A16  A15  A14  A13  A12
		��4�ֽڣ� A27  A26  A25  A24  A23  A22  A21  A20
	*/
	NAND_ADDR_AREA = 0;
	NAND_ADDR_AREA = 0;
	NAND_ADDR_AREA = _ulSrcPageNo;
	NAND_ADDR_AREA = (_ulSrcPageNo & 0xFF00) >> 8;

	NAND_CMD_AREA = NAND_CMD_COPYBACK_B;

	 /* ����ȴ���������������쳣, �˴�Ӧ���жϳ�ʱ */
	 while( GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_6) == 0 );		//jkl

	NAND_CMD_AREA = NAND_CMD_COPYBACK_C;

	/* ����Ŀ��ҳ��ַ �� ���� HY27UF081G2A
				  Bit7 Bit6 Bit5 Bit4 Bit3 Bit2 Bit1 Bit0
		��1�ֽڣ� A7   A6   A5   A4   A3   A2   A1   A0		(_usPageAddr ��bit7 - bit0)
		��2�ֽڣ� 0    0    0    0    A11  A10  A9   A8		(_usPageAddr ��bit11 - bit8, ��4bit������0)
		��3�ֽڣ� A19  A18  A17  A16  A15  A14  A13  A12
		��4�ֽڣ� A27  A26  A25  A24  A23  A22  A21  A20
	*/
	NAND_ADDR_AREA = 0;
	NAND_ADDR_AREA = 0;
	NAND_ADDR_AREA = _ulTarPageNo;
	NAND_ADDR_AREA = (_ulTarPageNo & 0xFF00) >> 8;

	NAND_CMD_AREA = NAND_CMD_COPYBACK_D;	 
	
	/* ������״̬ */	
	if (FSMC_NAND_GetStatus() == NAND_READY)
	{
		return NAND_OK;
	}
	return NAND_FAIL;
}

/*
*********************************************************************************************************
*	�� �� ��: FSMC_NAND_PageCopyBackEx
*	����˵��: ��һҳ���ݸ��Ƶ�����һ��ҳ,������Ŀ��ҳ�еĲ������ݡ�Դҳ��Ŀ��ҳ����ͬΪż��ҳ��ͬΪ����ҳ��
*	��    �Σ�- _ulSrcPageNo: Դҳ��
*             - _ulTarPageNo: Ŀ��ҳ��
*			  - _usOffset: ҳ��ƫ�Ƶ�ַ��pBuf�����ݽ�д�������ַ��ʼ��Ԫ
*			  - _pBuf: ���ݻ�����
*			  - _usSize: ���ݴ�С
*	�� �� ֵ: ִ�н����
*				- NAND_FAIL ��ʾʧ��
*				- NAND_OK ��ʾ�ɹ�
*
*	˵    ���������ֲ��Ƽ�����ҳ����֮ǰ����У��Դҳ��λУ�飬������ܻ����λ���󡣱�����δʵ�֡�
*
*********************************************************************************************************
*/
static uint8_t FSMC_NAND_PageCopyBackEx(uint32_t _ulSrcPageNo, uint32_t _ulTarPageNo, uint8_t *_pBuf, uint16_t _usOffset, uint16_t _usSize)
{
	uint16_t i;
		
	NAND_CMD_AREA = NAND_CMD_COPYBACK_A;
	
	/* ����Դҳ��ַ �� ���� HY27UF081G2A
				  Bit7 Bit6 Bit5 Bit4 Bit3 Bit2 Bit1 Bit0
		��1�ֽڣ� A7   A6   A5   A4   A3   A2   A1   A0		(_usPageAddr ��bit7 - bit0)
		��2�ֽڣ� 0    0    0    0    A11  A10  A9   A8		(_usPageAddr ��bit11 - bit8, ��4bit������0)
		��3�ֽڣ� A19  A18  A17  A16  A15  A14  A13  A12
		��4�ֽڣ� A27  A26  A25  A24  A23  A22  A21  A20
	*/
	NAND_ADDR_AREA = 0;
	NAND_ADDR_AREA = 0;
	NAND_ADDR_AREA = _ulSrcPageNo;
	NAND_ADDR_AREA = (_ulSrcPageNo & 0xFF00) >> 8;

	NAND_CMD_AREA = NAND_CMD_COPYBACK_B;

	 /* ����ȴ���������������쳣, �˴�Ӧ���жϳ�ʱ */
	 while( GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_6) == 0 );		//jkl

	NAND_CMD_AREA = NAND_CMD_COPYBACK_C;

	/* ����Ŀ��ҳ��ַ �� ���� HY27UF081G2A
				  Bit7 Bit6 Bit5 Bit4 Bit3 Bit2 Bit1 Bit0
		��1�ֽڣ� A7   A6   A5   A4   A3   A2   A1   A0		(_usPageAddr ��bit7 - bit0)
		��2�ֽڣ� 0    0    0    0    A11  A10  A9   A8		(_usPageAddr ��bit11 - bit8, ��4bit������0)
		��3�ֽڣ� A19  A18  A17  A16  A15  A14  A13  A12
		��4�ֽڣ� A27  A26  A25  A24  A23  A22  A21  A20
	*/
	NAND_ADDR_AREA = 0;
	NAND_ADDR_AREA = 0;
	NAND_ADDR_AREA = _ulTarPageNo;
	NAND_ADDR_AREA = (_ulTarPageNo & 0xFF00) >> 8;

	/* �м����������, Ҳ����ȴ� */
	
	NAND_CMD_AREA = NAND_CMD_COPYBACK_C;
	
	NAND_ADDR_AREA = _usOffset;
	NAND_ADDR_AREA = _usOffset >> 8;
	
	/* �������� */
	for(i = 0; i < _usSize; i++)
	{
		NAND_DATA_AREA = _pBuf[i];
	}
			
	NAND_CMD_AREA = NAND_CMD_COPYBACK_D;	 
	
	/* ������״̬ */	
	if (FSMC_NAND_GetStatus() == NAND_READY)
	{
		return NAND_OK;
	}
	return NAND_FAIL;
}

/*
*********************************************************************************************************
*	�� �� ��: FSMC_NAND_WritePage
*	����˵��: дһ��������NandFlashָ��ҳ���ָ��λ�ã�д������ݳ��Ȳ�����һҳ�Ĵ�С��
*	��    �Σ�- _pBuffer: ָ�������д���ݵĻ����� 
*             - _ulPageNo: ҳ�ţ����е�ҳͳһ���룬��ΧΪ��0 - 65535
*			  - _usAddrInPage : ҳ�ڵ�ַ����ΧΪ��0-2111
*             - _usByteCount: д����ֽڸ���
*	�� �� ֵ: ִ�н����
*				- NAND_FAIL ��ʾʧ��
*				- NAND_OK ��ʾ�ɹ�
*********************************************************************************************************
*/
static uint8_t FSMC_NAND_WritePage(uint8_t *_pBuffer, uint32_t _ulPageNo, uint16_t _usAddrInPage, uint16_t _usByteCount)
{
	uint16_t i;
		
	/* ����ҳд���� */
	NAND_CMD_AREA = NAND_CMD_WRITE0;
	
	/* ����ҳ�ڵ�ַ �� ���� HY27UF081G2A
				  Bit7 Bit6 Bit5 Bit4 Bit3 Bit2 Bit1 Bit0
		��1�ֽڣ� A7   A6   A5   A4   A3   A2   A1   A0		(_usPageAddr ��bit7 - bit0)
		��2�ֽڣ� 0    0    0    0    A11  A10  A9   A8		(_usPageAddr ��bit11 - bit8, ��4bit������0)
		��3�ֽڣ� A19  A18  A17  A16  A15  A14  A13  A12
		��4�ֽڣ� A27  A26  A25  A24  A23  A22  A21  A20
	*/
	NAND_ADDR_AREA = _usAddrInPage;
	NAND_ADDR_AREA = _usAddrInPage >> 8;
	NAND_ADDR_AREA = _ulPageNo;
	NAND_ADDR_AREA = (_ulPageNo & 0xFF00) >> 8;

	/* д���� */
	for(i = 0; i < _usByteCount; i++)
	{
		NAND_DATA_AREA = _pBuffer[i];
	}
	NAND_CMD_AREA = NAND_CMD_WRITE_TRUE1;
	
	/* ������״̬ */	
	if (FSMC_NAND_GetStatus() == NAND_READY)
	{
		return NAND_OK;
	}
	return NAND_FAIL;
}

/*
*********************************************************************************************************
*	�� �� ��: FSMC_NAND_ReadPage
*	����˵��: ��NandFlashָ��ҳ���ָ��λ�ö�һ�����ݣ����������ݳ��Ȳ�����һҳ�Ĵ�С��
*	��    �Σ�- _pBuffer: ָ�������д���ݵĻ����� 
*             - _ulPageNo: ҳ�ţ����е�ҳͳһ���룬��ΧΪ��0 - 65535
*			  - _usAddrInPage : ҳ�ڵ�ַ����ΧΪ��0-2111
*             - _usByteCount: д����ֽڸ���
*	�� �� ֵ: ִ�н����
*				- NAND_FAIL ��ʾʧ��
*				- NAND_OK ��ʾ�ɹ�
*********************************************************************************************************
*/
static uint8_t FSMC_NAND_ReadPage(uint8_t *_pBuffer, uint32_t _ulPageNo, uint16_t _usAddrInPage, uint16_t _usByteCount)
{
	uint16_t i;

    /* ����ҳ������� */
    NAND_CMD_AREA = NAND_CMD_AREA_A;

	/* ����ҳ�ڵ�ַ �� ���� HY27UF081G2A
				  Bit7 Bit6 Bit5 Bit4 Bit3 Bit2 Bit1 Bit0
		��1�ֽڣ� A7   A6   A5   A4   A3   A2   A1   A0		(_usPageAddr ��bit7 - bit0)
		��2�ֽڣ� 0    0    0    0    A11  A10  A9   A8		(_usPageAddr ��bit11 - bit8, ��4bit������0)
		��3�ֽڣ� A19  A18  A17  A16  A15  A14  A13  A12
		��4�ֽڣ� A27  A26  A25  A24  A23  A22  A21  A20
	*/
	NAND_ADDR_AREA = _usAddrInPage;
	NAND_ADDR_AREA = _usAddrInPage >> 8;
	NAND_ADDR_AREA = _ulPageNo;
	NAND_ADDR_AREA = (_ulPageNo & 0xFF00) >> 8;

	NAND_CMD_AREA = NAND_CMD_AREA_TRUE1;
	    
	 /* ����ȴ���������������쳣, �˴�Ӧ���жϳ�ʱ */
	 while( GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_6) == 0 );		
	
	/* �����ݵ�������pBuffer */    
	for(i = 0; i < _usByteCount; i++)
	{
		_pBuffer[i] = NAND_DATA_AREA;
	}	

	return NAND_OK;
}

/*
*********************************************************************************************************
*	�� �� ��: FSMC_NAND_WriteSpare
*	����˵��: ��1��PAGE��Spare��д������
*	��    �Σ�- _pBuffer: ָ�������д���ݵĻ����� 
*             - _ulPageNo: ҳ�ţ����е�ҳͳһ���룬��ΧΪ��0 - 65535
*			  - _usAddrInSpare : ҳ�ڱ�������ƫ�Ƶ�ַ����ΧΪ��0-63
*             - _usByteCount: д����ֽڸ���
*	�� �� ֵ: ִ�н����
*				- NAND_FAIL ��ʾʧ��
*				- NAND_OK ��ʾ�ɹ�
*********************************************************************************************************
*/
static uint8_t FSMC_NAND_WriteSpare(uint8_t *_pBuffer, uint32_t _ulPageNo, uint16_t _usAddrInSpare, uint16_t _usByteCount)
{
	if (_usByteCount > NAND_SPARE_AREA_SIZE)
	{
		return NAND_FAIL;
	}
	
	return FSMC_NAND_WritePage(_pBuffer, _ulPageNo, NAND_PAGE_SIZE + _usAddrInSpare, _usByteCount);
}

/*
*********************************************************************************************************
*	�� �� ��: FSMC_NAND_ReadSpare
*	����˵��: ��1��PAGE��Spare��������
*	��    �Σ�- _pBuffer: ָ�������д���ݵĻ����� 
*             - _ulPageNo: ҳ�ţ����е�ҳͳһ���룬��ΧΪ��0 - 65535
*			  - _usAddrInSpare : ҳ�ڱ�������ƫ�Ƶ�ַ����ΧΪ��0-63
*             - _usByteCount: д����ֽڸ���
*	�� �� ֵ: ִ�н����
*				- NAND_FAIL ��ʾʧ��
*				- NAND_OK ��ʾ�ɹ�
*********************************************************************************************************
*/
static uint8_t FSMC_NAND_ReadSpare(uint8_t *_pBuffer, uint32_t _ulPageNo, uint16_t _usAddrInSpare, uint16_t _usByteCount)
{
	if (_usByteCount > NAND_SPARE_AREA_SIZE)
	{
		return NAND_FAIL;
	}
		
	return FSMC_NAND_ReadPage(_pBuffer, _ulPageNo, NAND_PAGE_SIZE + _usAddrInSpare, _usByteCount);
}

/*
*********************************************************************************************************
*	�� �� ��: FSMC_NAND_WriteData
*	����˵��: ��1��PAGE����������д������
*	��    �Σ�- _pBuffer: ָ�������д���ݵĻ����� 
*             - _ulPageNo: ҳ�ţ����е�ҳͳһ���룬��ΧΪ��0 - 65535
*			  - _usAddrInPage : ҳ����������ƫ�Ƶ�ַ����ΧΪ��0-2047
*             - _usByteCount: д����ֽڸ���
*	�� �� ֵ: ִ�н����
*				- NAND_FAIL ��ʾʧ��
*				- NAND_OK ��ʾ�ɹ�
*********************************************************************************************************
*/
static uint8_t FSMC_NAND_WriteData(uint8_t *_pBuffer, uint32_t _ulPageNo, uint16_t _usAddrInPage, uint16_t _usByteCount)
{
	if (_usByteCount > NAND_PAGE_SIZE)
	{
		return NAND_FAIL;
	}
	
	return FSMC_NAND_WritePage(_pBuffer, _ulPageNo, _usAddrInPage, _usByteCount);
}

/*
*********************************************************************************************************
*	�� �� ��: FSMC_NAND_ReadData
*	����˵��: ��1��PAGE�������ݵ�����
*	��    �Σ�- _pBuffer: ָ�������д���ݵĻ����� 
*             - _ulPageNo: ҳ�ţ����е�ҳͳһ���룬��ΧΪ��0 - 65535
*			  - _usAddrInPage : ҳ����������ƫ�Ƶ�ַ����ΧΪ��0-2047
*             - _usByteCount: д����ֽڸ���
*	�� �� ֵ: ִ�н����
*				- NAND_FAIL ��ʾʧ��
*				- NAND_OK ��ʾ�ɹ�
*********************************************************************************************************
*/
static uint8_t FSMC_NAND_ReadData(uint8_t *_pBuffer, uint32_t _ulPageNo, uint16_t _usAddrInPage, uint16_t _usByteCount)
{
	if (_usByteCount > NAND_PAGE_SIZE)
	{
		return NAND_FAIL;
	}
	
	return FSMC_NAND_ReadPage(_pBuffer, _ulPageNo, _usAddrInPage, _usByteCount);
}

/*
*********************************************************************************************************
*	�� �� ��: FSMC_NAND_EraseBlock
*	����˵��: ����NAND Flashһ���飨block��
*	��    �Σ�- _ulBlockNo: ��ţ���ΧΪ��0 - 1023
*	�� �� ֵ: NAND����״̬�������¼���ֵ��
*             - NAND_TIMEOUT_ERROR  : ��ʱ����
*             - NAND_READY          : �����ɹ�
*********************************************************************************************************
*/
static uint8_t FSMC_NAND_EraseBlock(uint32_t _ulBlockNo)
{
	/* ���Ͳ������� */
	NAND_CMD_AREA = NAND_CMD_ERASE0;
	
	_ulBlockNo <<= 6;	/* ���ת��Ϊҳ��� */
	NAND_ADDR_AREA = _ulBlockNo;
	NAND_ADDR_AREA = _ulBlockNo >> 8;
	
	NAND_CMD_AREA = NAND_CMD_ERASE1;
	
	return (FSMC_NAND_GetStatus());
}

/*
*********************************************************************************************************
*	�� �� ��: FSMC_NAND_Reset
*	����˵��: ��λNAND Flash
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static uint8_t FSMC_NAND_Reset(void)
{
	NAND_CMD_AREA = NAND_CMD_RESET;
	
		/* ������״̬ */	
	if (FSMC_NAND_GetStatus() == NAND_READY)
	{
		return NAND_OK;
	}
	
	return NAND_FAIL;
}

/*
*********************************************************************************************************
*	�� �� ��: FSMC_NAND_ReadStatus
*	����˵��: ʹ��Read statuc �����NAND Flash�ڲ�״̬
*	��    �Σ�- Address: �������Ŀ��������ַ
*	�� �� ֵ: NAND����״̬�������¼���ֵ��
*             - NAND_BUSY: �ڲ���æ
*             - NAND_READY: �ڲ����У����Խ����²�����
*             - NAND_ERROR: ��ǰ������ִ��ʧ��
*********************************************************************************************************
*/
static uint8_t FSMC_NAND_ReadStatus(void)
{
	uint8_t ucData;
	uint8_t ucStatus = NAND_BUSY;
	
	/* ��״̬���� */
	NAND_CMD_AREA = NAND_CMD_STATUS;
	ucData = *(__IO uint8_t *)(Bank_NAND_ADDR);
	
	if((ucData & NAND_ERROR) == NAND_ERROR)
	{
		ucStatus = NAND_ERROR;
	} 
	else if((ucData & NAND_READY) == NAND_READY)
	{
		ucStatus = NAND_READY;
	}
	else
	{
		ucStatus = NAND_BUSY; 
	}
	
	return (ucStatus);
}

/*
*********************************************************************************************************
*	�� �� ��: FSMC_NAND_GetStatus
*	����˵��: ��ȡNAND Flash����״̬
*	��    �Σ�- Address: �������Ŀ��������ַ
*	�� �� ֵ: NAND����״̬�������¼���ֵ��
*             - NAND_TIMEOUT_ERROR  : ��ʱ����
*             - NAND_READY          : �����ɹ�
*********************************************************************************************************
*/
static uint8_t FSMC_NAND_GetStatus(void)
{
	uint32_t ulTimeout = 0x10000;
	uint8_t ucStatus = NAND_READY;
	
	ucStatus = FSMC_NAND_ReadStatus(); 
	
	/* �ȴ�NAND������������ʱ����˳� */
	while ((ucStatus != NAND_READY) &&( ulTimeout != 0x00))
	{
		ucStatus = FSMC_NAND_ReadStatus();
		ulTimeout--;      
	}
	
	if(ulTimeout == 0x00)
	{          
		ucStatus =  NAND_TIMEOUT_ERROR;      
	} 
	
	/* ���ز���״̬ */
	return (ucStatus);      
}

/*
*********************************************************************************************************
*	�� �� ��: NAND_Init
*	����˵��: ��ʼ��NAND Flash�ӿ�
*	��    �Σ���
*	�� �� ֵ: ִ�н����
*				- NAND_FAIL ��ʾʧ��
*				- NAND_OK ��ʾ�ɹ�
*********************************************************************************************************
*/
uint8_t NAND_Init(void)
{
	uint8_t Status;
	
	FSMC_NAND_Init();			/* ����FSMC��GPIO����NAND Flash�ӿ� */
	
	FSMC_NAND_Reset();			/* ͨ����λ���λNAND Flash����״̬ */
						
	Status = NAND_BuildLUT();	/* ����������� LUT = Look up table */
	return Status;
}

/*
*********************************************************************************************************
*	�� �� ��: NAND_WriteToNewBlock
*	����˵��: ���ɿ�����ݸ��Ƶ��¿飬�����µ����ݶ�д������¿�
*	��    �Σ�	_ulPhyPageNo : Դҳ��
*				_pWriteBuf �� ���ݻ�����
*				_usOffset �� ҳ��ƫ�Ƶ�ַ
*				_usSize �����ݳ��ȣ�������4�ֽڵ�������
*	�� �� ֵ: ִ�н����
*				- NAND_FAIL ��ʾʧ��
*				- NAND_OK ��ʾ�ɹ�
*********************************************************************************************************
*/
uint8_t NAND_WriteToNewBlock(uint32_t _ulPhyPageNo, uint8_t *_pWriteBuf, uint16_t _usOffset, uint16_t _usSize)
{
	uint16_t n, i;
	uint16_t usNewBlock;
	uint16_t ulSrcBlock;
	uint16_t usOffsetPageNo;

	ulSrcBlock = _ulPhyPageNo / NAND_BLOCK_SIZE;		/* ��������ҳ�ŷ��ƿ�� */
	usOffsetPageNo = _ulPhyPageNo % NAND_BLOCK_SIZE;	/* ��������ҳ�ż�������ҳ���ڿ���ƫ��ҳ�� */
	/* ����ѭ����Ŀ���Ǵ���Ŀ���Ϊ�������� */
	for (n = 0; n < 10; n++)
	{
		/* �������ȫ0xFF�� ����ҪѰ��һ�����п��ÿ飬����ҳ�ڵ�����ȫ���Ƶ��¿��У�Ȼ���������� */
		usNewBlock = NAND_FindFreeBlock();	/* �����һ��Block��ʼ����Ѱһ�����ÿ� */
		if (usNewBlock >= NAND_BLOCK_COUNT)
		{
			return NAND_FAIL;	/* ���ҿ��п�ʧ�� */
		}
		
		/* ʹ��page-copy���ܣ�����ǰ�飨usPBN��������ȫ�����Ƶ��¿飨usNewBlock�� */
		for (i = 0; i < NAND_BLOCK_SIZE; i++)
		{
			if (i == usOffsetPageNo)
			{
				/* ���д��������ڵ�ǰҳ������Ҫʹ�ô�������ݵ�Copy-Back���� */
				if (FSMC_NAND_PageCopyBackEx(ulSrcBlock * NAND_BLOCK_SIZE + i, usNewBlock * NAND_BLOCK_SIZE + i,
					_pWriteBuf, _usOffset, _usSize) == NAND_FAIL)
				{
					NAND_MarkBadBlock(usNewBlock);	/* ���¿���Ϊ���� */
					NAND_BuildLUT();				/* �ؽ�LUT�� */
					break;
				}
			}
			else
			{
				/* ʹ��NAND Flash �ṩ����ҳCopy-Back���ܣ�����������߲���Ч�� */
				if (FSMC_NAND_PageCopyBack(ulSrcBlock * NAND_BLOCK_SIZE + i, 
					usNewBlock * NAND_BLOCK_SIZE + i) == NAND_FAIL)
				{
					NAND_MarkBadBlock(usNewBlock);	/* ���¿���Ϊ���� */
					NAND_BuildLUT();				/* �ؽ�LUT�� */
					break;
				}
			}
		}
		/* Ŀ�����³ɹ� */
		if (i == NAND_BLOCK_SIZE)
		{
			/* ����¿�Ϊ���ÿ� */
			if (NAND_MarkUsedBlock(usNewBlock) == NAND_FAIL)
			{
				NAND_MarkBadBlock(usNewBlock);	/* ���¿���Ϊ���� */
				NAND_BuildLUT();				/* �ؽ�LUT�� */
				continue;
			}
			
			/* ����ԴBLOCK */
			if (FSMC_NAND_EraseBlock(ulSrcBlock) != NAND_READY)
			{
				NAND_MarkBadBlock(ulSrcBlock);	/* ��Դ����Ϊ���� */
				NAND_BuildLUT();				/* �ؽ�LUT�� */
				continue;
			}
			NAND_BuildLUT();				/* �ؽ�LUT�� */
			break;
		}
	}
	
	return NAND_OK;	/* д��ɹ� */
}

/*
*********************************************************************************************************
*	�� �� ��: NAND_Write
*	����˵��: дһ������
*	��    �Σ�	_MemAddr : �ڴ浥Ԫƫ�Ƶ�ַ
*				_pReadbuff ����Ŵ�д���ݵĻ�������ָ��
*				_usSize �����ݳ��ȣ�������4�ֽڵ�������
*	�� �� ֵ: ִ�н����
*				- NAND_FAIL ��ʾʧ��
*				- NAND_OK ��ʾ�ɹ�
*********************************************************************************************************
*/
uint8_t NAND_Write(uint32_t _ulMemAddr, uint32_t *_pWriteBuf, uint16_t _usSize)
{
	uint16_t usPBN;			/* ������� */
	uint32_t ulPhyPageNo;	/* ����ҳ�� */
	uint16_t usAddrInPage;	/* ҳ��ƫ�Ƶ�ַ */
	uint32_t ulTemp;

	/* ���ݳ��ȱ�����4�ֽ������� */
	if ((_usSize % 4) != 0)
	{
		return NAND_FAIL;
	}
	/* ���ݳ��Ȳ��ܳ���512�ֽ�(��ѭ Fat��ʽ) */
	if (_usSize > 512)
	{
		//return NAND_FAIL;	
	}

	usPBN = NAND_AddrToPhyBlockNo(_ulMemAddr);	/* ��ѯLUT������������ */

	ulTemp = _ulMemAddr % (NAND_BLOCK_SIZE * NAND_PAGE_SIZE);
	ulPhyPageNo = usPBN * NAND_BLOCK_SIZE + ulTemp / NAND_PAGE_SIZE;	/* ��������ҳ�� */
	usAddrInPage = ulTemp % NAND_PAGE_SIZE;	/* ����ҳ��ƫ�Ƶ�ַ */
	
	/* �������������ݣ��ж��Ƿ�ȫFF */
	if (FSMC_NAND_ReadData(s_ucTempBuf, ulPhyPageNo, usAddrInPage, _usSize) == NAND_FAIL)
	{
		return NAND_FAIL;	/* ��NAND Flashʧ�� */
	}
	/*�������ȫ0xFF, �����ֱ��д�룬������� */
	if (NAND_IsBufOk(s_ucTempBuf, _usSize, 0xFF))
	{
		if (FSMC_NAND_WriteData((uint8_t *)_pWriteBuf, ulPhyPageNo, usAddrInPage, _usSize) == NAND_FAIL)
		{
			/* ������д�뵽����һ���飨���п飩 */
			return NAND_WriteToNewBlock(ulPhyPageNo, (uint8_t *)_pWriteBuf, usAddrInPage, _usSize);
		}
		
		/* ��Ǹÿ����� */
		if (NAND_MarkUsedBlock(ulPhyPageNo) == NAND_FAIL)
		{
			/* ���ʧ�ܣ�������д�뵽����һ���飨���п飩 */
			return NAND_WriteToNewBlock(ulPhyPageNo, (uint8_t *)_pWriteBuf, usAddrInPage, _usSize);
		}	
		return NAND_OK;	/* д��ɹ� */
	}
	
	/* ������д�뵽����һ���飨���п飩 */
	return NAND_WriteToNewBlock(ulPhyPageNo, (uint8_t *)_pWriteBuf, usAddrInPage, _usSize);
}

/*
*********************************************************************************************************
*	�� �� ��: NAND_Read
*	����˵��: ��һ������
*	��    �Σ�	_MemAddr : �ڴ浥Ԫƫ�Ƶ�ַ
*				_pReadbuff ����Ŷ������ݵĻ�������ָ��
*				_usSize �����ݳ��ȣ�������4�ֽڵ�������
*	�� �� ֵ: ִ�н����
*				- NAND_FAIL ��ʾʧ��
*				- NAND_OK ��ʾ�ɹ�
*********************************************************************************************************
*/
uint8_t NAND_Read(uint32_t _ulMemAddr, uint32_t *_pReadBuf, uint16_t _usSize)
{
	uint16_t usPBN;			/* ������� */
	uint32_t ulPhyPageNo;	/* ����ҳ�� */
	uint16_t usAddrInPage;	/* ҳ��ƫ�Ƶ�ַ */
	uint32_t ulTemp;

	/* ���ݳ��ȱ�����4�ֽ������� */
	if ((_usSize % 4) != 0)
	{
		return NAND_FAIL;
	}

	usPBN = NAND_AddrToPhyBlockNo(_ulMemAddr);	/* ��ѯLUT������������ */
	if (usPBN >= NAND_BLOCK_COUNT)
	{
		/* û�и�ʽ����usPBN = 0xFFFF */
		return NAND_FAIL;
	}

	ulTemp = _ulMemAddr % (NAND_BLOCK_SIZE * NAND_PAGE_SIZE);
	ulPhyPageNo = usPBN * NAND_BLOCK_SIZE + ulTemp / NAND_PAGE_SIZE;	/* ��������ҳ�� */
	usAddrInPage = ulTemp % NAND_PAGE_SIZE;	/* ����ҳ��ƫ�Ƶ�ַ */
	
	if (FSMC_NAND_ReadData((uint8_t *)_pReadBuf, ulPhyPageNo, usAddrInPage, _usSize) == NAND_FAIL)
	{
		return NAND_FAIL;	/* ��NAND Flashʧ�� */
	}
	
	/* �ɹ� */
	return NAND_OK;
}

/*
*********************************************************************************************************
*	�� �� ��: NAND_BuildLUT
*	����˵��: ���ڴ��д������������
*	��    �Σ�ZoneNbr ������
*	�� �� ֵ: NAND_OK�� �ɹ��� 	NAND_FAIL��ʧ��
*********************************************************************************************************
*/
static uint8_t NAND_BuildLUT(void)
{
	uint16_t i;
	uint8_t buf[VALID_SPARE_SIZE];
	uint16_t usLBN;	/* �߼���� */
	
	/* */
	for (i = 0; i < NAND_BLOCK_COUNT; i++)
	{
		s_usLUT[i] = 0xFFFF;	/* �����Чֵ�������ؽ�LUT���ж�LUT�Ƿ���� */
	}
	for (i = 0; i < NAND_BLOCK_COUNT; i++)
	{
		/* ��ÿ����ĵ�1��PAGE��ƫ�Ƶ�ַΪLBN0_OFFSET������ */
		FSMC_NAND_ReadSpare(buf, i * NAND_BLOCK_SIZE, 0, VALID_SPARE_SIZE);
		
		/* ����Ǻÿ飬���¼LBN0 LBN1 */
		if (buf[BI_OFFSET] == 0xFF)	
		{
			usLBN = buf[LBN0_OFFSET] + buf[LBN1_OFFSET] * 256;	/* ����������߼���� */
			if (usLBN < NAND_BLOCK_COUNT)
			{
				/* ����Ѿ��Ǽǹ��ˣ����ж�Ϊ�쳣 */
				if (s_usLUT[usLBN] != 0xFFFF)
				{
					return NAND_FAIL;
				}

				s_usLUT[usLBN] = i;	/* ����LUT�� */
			}
		}
	}
	
	/* LUT������ϣ�����Ƿ���� */
	for (i = 0; i < NAND_BLOCK_COUNT; i++)
	{
		if (s_usLUT[i] >= NAND_BLOCK_COUNT)
		{
			s_usValidDataBlockCount = i;
			break;
		}
	}
	if (s_usValidDataBlockCount < 100)
	{
		/* ���� ������Ч�߼����С��100��������û�и�ʽ�� */
		return NAND_FAIL;	
	}
	for (; i < s_usValidDataBlockCount; i++)
	{
		if (s_usLUT[i] != 0xFFFF)
		{
			return NAND_FAIL;	/* ����LUT���߼���Ŵ�����Ծ���󣬿�����û�и�ʽ�� */
		}
	}
	
	/* �ؽ�LUT���� */
	return NAND_OK;
}

/*
*********************************************************************************************************
*	�� �� ��: NAND_AddrToPhyBlockNo
*	����˵��: �ڴ��߼���ַת��Ϊ�������
*	��    �Σ�_ulMemAddr���߼��ڴ��ַ
*	�� �� ֵ: ����ҳ�ţ� ����� 0xFFFFFFFF ���ʾ����
*********************************************************************************************************
*/
static uint16_t NAND_AddrToPhyBlockNo(uint32_t _ulMemAddr)
{
	uint16_t usLBN;		/* �߼���� */
	uint16_t usPBN;		/* ������� */
	
	usLBN = _ulMemAddr / (NAND_BLOCK_SIZE * NAND_PAGE_SIZE);	/* �����߼���� */
	/* ����߼���Ŵ�����Ч�����ݿ������̶�����0xFFFF, ���øú����Ĵ���Ӧ�ü������ִ��� */
	if (usLBN >= s_usValidDataBlockCount)
	{
		return 0xFFFF;
	}
	/* ��ѯLUT�������������� */
	usPBN = s_usLUT[usLBN];
	return usPBN;
}

/*
*********************************************************************************************************
*	�� �� ��: NAND_FindFreeBlock
*	����˵��: �����һ���鿪ʼ������һ�����õĿ顣
*	��    �Σ�ZoneNbr ������
*	�� �� ֵ: ��ţ������0xFFFF��ʾʧ��
*********************************************************************************************************
*/
static uint16_t NAND_FindFreeBlock (void)
{
	uint16_t i;
	uint16_t n;

	n = NAND_BLOCK_COUNT - 1;
	for (i = 0; i < NAND_BLOCK_COUNT; i++)
	{
		if (NAND_IsFreeBlock(n))
		{
			return n;
		}
		n--;
	}
	return 0xFFFF;
}

/*
*********************************************************************************************************
*	�� �� ��: NAND_IsBufOk
*	����˵��: �ж��ڴ滺�����������Ƿ�ȫ��Ϊָ��ֵ
*	��    �Σ�- _pBuf : ���뻺����
*			  - _ulLen : ����������
*			  - __ucValue : ������ÿ����Ԫ����ȷ��ֵ
*	�� �� ֵ: 1 ��ȫ����ȷ�� 0 ������ȷ
*********************************************************************************************************
*/
static uint8_t NAND_IsBufOk(uint8_t *_pBuf, uint32_t _ulLen, uint8_t _ucValue)
{
	uint32_t i;
	
	for (i = 0; i < _ulLen; i++)
	{
		if (_pBuf[i] != _ucValue)
		{
			return 0;
		}
	}
	
	return 1;
}

/*
*********************************************************************************************************
*	�� �� ��: NAND_IsBadBlock
*	����˵��: ���ݻ����Ǽ��NAND Flashָ���Ŀ��Ƿ񻵿�
*	��    ��: _ulBlockNo ����� 0 - 1023 ������128M�ֽڣ�2K Page��NAND Flash����1024���飩
*	�� �� ֵ: 0 ���ÿ���ã� 1 ���ÿ��ǻ���
*********************************************************************************************************
*/
static uint8_t NAND_IsBadBlock(uint32_t _ulBlockNo)
{
	uint8_t ucFlag;
	
	/* ���NAND Flash����ǰ�Ѿ���עΪ�����ˣ������Ϊ�ǻ��� */
	FSMC_NAND_ReadSpare(&ucFlag, _ulBlockNo * NAND_BLOCK_SIZE, BI_OFFSET, 1);
	if (ucFlag != 0xFF)
	{
		return 1;		
	}

	FSMC_NAND_ReadSpare(&ucFlag, _ulBlockNo * NAND_BLOCK_SIZE + 1, BI_OFFSET, 1);
	if (ucFlag != 0xFF)
	{
		return 1;		
	}	
	return 0;	/* �Ǻÿ� */
}

/*
*********************************************************************************************************
*	�� �� ��: NAND_IsFreeBlock
*	����˵��: ���ݻ����Ǻ�USED��־����Ƿ���ÿ�
*	��    ��: _ulBlockNo ����� 0 - 1023 ������128M�ֽڣ�2K Page��NAND Flash����1024���飩
*	�� �� ֵ: 1 ���ÿ���ã� 0 ���ÿ��ǻ��������ռ��
*********************************************************************************************************
*/
static uint8_t NAND_IsFreeBlock(uint32_t _ulBlockNo)
{
	uint8_t ucFlag;

	/* ���NAND Flash����ǰ�Ѿ���עΪ�����ˣ������Ϊ�ǻ��� */
	if (NAND_IsBadBlock(_ulBlockNo))
	{
		return 0;
	}
	
	FSMC_NAND_ReadPage(&ucFlag, _ulBlockNo * NAND_BLOCK_SIZE, USED_OFFSET, 1);
	if (ucFlag == 0xFF)
	{
		return 1;	
	}
	return 0;
}
	
/*
*********************************************************************************************************
*	�� �� ��: NAND_ScanBlock
*	����˵��: ɨ�����NAND Flashָ���Ŀ�
*			��ɨ������㷨��
*			1) ��1���飨�������������ͱ����������������������Ƿ�ȫ0xFF, ��ȷ�Ļ��������ԸĿ飬����ÿ�
				�ǻ���,��������
*			2) ��ǰ��д��ȫ 0x00��Ȼ���ȡ��⣬��ȷ�Ļ��������ԸĿ飬�����˳�
*			3) �ظ��ڣ�2���������ѭ��������50�ζ�û�з���������ô�ÿ�����,�������أ�����ÿ��ǻ��飬
*				��������
*			��ע�⡿
*			1) �ú���������Ϻ󣬻�ɾ�������������ݣ�����Ϊȫ0xFF;
*			2) �ú������˲������������⣬Ҳ�Ա������������в��ԡ�
*			3) ��д����ѭ���������Ժ�ָ����#define BAD_BALOK_TEST_CYCLE 50
*	��    �Σ�_ulPageNo ��ҳ�� 0 - 65535 ������128M�ֽڣ�2K Page��NAND Flash����1024���飩
*	�� �� ֵ: NAND_OK ���ÿ���ã� NAND_FAIL ���ÿ��ǻ���
*********************************************************************************************************
*/
uint8_t NAND_ScanBlock(uint32_t _ulBlockNo)
{
	uint32_t i, k;
	uint32_t ulPageNo;
	
	#if 1	
	/* ���NAND Flash����ǰ�Ѿ���עΪ�����ˣ������Ϊ�ǻ��� */
	if (NAND_IsBadBlock(_ulBlockNo))
	{
		return NAND_FAIL;
	}
	#endif
	
	/* ����Ĵ��뽫ͨ��������������̵ķ�ʽ������NAND Flashÿ����Ŀɿ��� */
	memset(s_ucTempBuf, 0x00, NAND_PAGE_TOTAL_SIZE);
	for (i = 0; i < BAD_BALOK_TEST_CYCLE; i++)
	{
		/* ��1������������� */	
		if (FSMC_NAND_EraseBlock(_ulBlockNo) != NAND_READY)
		{
			return NAND_FAIL;
		}
		
		/* ��2������������ÿ��page�����ݣ����ж��Ƿ�ȫ0xFF */
		ulPageNo = _ulBlockNo * NAND_BLOCK_SIZE;	/* ����ÿ��1��ҳ��ҳ�� */
		for (k = 0; k < NAND_BLOCK_SIZE; k++)
		{
			/* ������ҳ���� */
			FSMC_NAND_ReadPage(s_ucTempBuf, ulPageNo, 0, NAND_PAGE_TOTAL_SIZE);

			/* �жϴ洢��Ԫ�ǲ���ȫ0xFF */
			if (NAND_IsBufOk(s_ucTempBuf, NAND_PAGE_TOTAL_SIZE, 0xFF) != NAND_OK)
			{
				return NAND_FAIL;
			}
			
			ulPageNo++;		/* ����д��һ��ҳ */
		}
		
		/* ��2����дȫ0���������ж��Ƿ�ȫ0 */
		ulPageNo = _ulBlockNo * NAND_BLOCK_SIZE;	/* ����ÿ��1��ҳ��ҳ�� */
		for (k = 0; k < NAND_BLOCK_SIZE; k++)
		{
			/* ���buf[]������Ϊȫ0,��д��NAND Flash */
			memset(s_ucTempBuf, 0x00, NAND_PAGE_TOTAL_SIZE);
			if (FSMC_NAND_WritePage(s_ucTempBuf, ulPageNo, 0, NAND_PAGE_TOTAL_SIZE) != NAND_OK)
			{
				return NAND_FAIL;
			}
			
			/* ������ҳ����, �жϴ洢��Ԫ�ǲ���ȫ0x00 */
			FSMC_NAND_ReadPage(s_ucTempBuf, ulPageNo, 0, NAND_PAGE_TOTAL_SIZE);
			if (NAND_IsBufOk(s_ucTempBuf, NAND_PAGE_TOTAL_SIZE, 0x00) != NAND_OK)
			{
				return NAND_FAIL;
			}
			
			ulPageNo++;		/* ����һ��ҳ */						
		}
	}
	
	/* ���һ�������������� */
	if (FSMC_NAND_EraseBlock(_ulBlockNo) != NAND_READY)
	{
		return NAND_FAIL;
	}
	ulPageNo = _ulBlockNo * NAND_BLOCK_SIZE;	/* ����ÿ��1��ҳ��ҳ�� */
	for (k = 0; k < NAND_BLOCK_SIZE; k++)
	{
		/* ������ҳ���� */
		FSMC_NAND_ReadPage(s_ucTempBuf, ulPageNo, 0, NAND_PAGE_TOTAL_SIZE);

		/* �жϴ洢��Ԫ�ǲ���ȫ0xFF */
		if (NAND_IsBufOk(s_ucTempBuf, NAND_PAGE_TOTAL_SIZE, 0xFF) != NAND_OK)
		{
			return NAND_FAIL;
		}
		
		ulPageNo++;		/* ����д��һ��ҳ */
	}	
	
	return NAND_OK;
}

/*
*********************************************************************************************************
*	�� �� ��: NAND_MarkUsedBlock
*	����˵��: ���NAND Flashָ���Ŀ�Ϊ���ÿ�
*	��    ��: _ulBlockNo ����� 0 - 1023 ������128M�ֽڣ�2K Page��NAND Flash����1024���飩
*	�� �� ֵ: NAND_OK:��ǳɹ��� NAND_FAIL�����ʧ�ܣ��ϼ�����Ӧ�ý��л��鴦����
*********************************************************************************************************
*/
static uint8_t NAND_MarkUsedBlock(uint32_t _ulBlockNo)
{								   
	uint32_t ulPageNo;
	uint8_t ucFlag;
	
	/* �����ĵ�1��ҳ�� */
	ulPageNo = _ulBlockNo * NAND_BLOCK_SIZE;	/* ����ÿ��1��ҳ��ҳ�� */
	
	/* ���ڵ�1��page�������ĵ�6���ֽ�д���0xFF���ݱ�ʾ���� */
	ucFlag = NAND_USED_BLOCK_FLAG;
	if (FSMC_NAND_WriteSpare(&ucFlag, ulPageNo, USED_OFFSET, 1) == NAND_FAIL)
	{
		/* ������ʧ�ܣ�����Ҫ��ע�����Ϊ���� */
		return NAND_FAIL;
	}
	return NAND_OK;
}

/*
*********************************************************************************************************
*	�� �� ��: NAND_MarkBadBlock
*	����˵��: ���NAND Flashָ���Ŀ�Ϊ����
*	��    ��: _ulBlockNo ����� 0 - 1023 ������128M�ֽڣ�2K Page��NAND Flash����1024���飩
*	�� �� ֵ: �̶�NAND_OK
*********************************************************************************************************
*/
static void NAND_MarkBadBlock(uint32_t _ulBlockNo)
{								   
	uint32_t ulPageNo;
	uint8_t ucFlag;
	
	/* �����ĵ�1��ҳ�� */
	ulPageNo = _ulBlockNo * NAND_BLOCK_SIZE;	/* ����ÿ��1��ҳ��ҳ�� */
	
	/* ���ڵ�1��page�������ĵ�6���ֽ�д���0xFF���ݱ�ʾ���� */
	ucFlag = NAND_BAD_BLOCK_FLAG;
	if (FSMC_NAND_WriteSpare(&ucFlag, ulPageNo, BI_OFFSET, 1) == NAND_FAIL)
	{
		/* �����1��ҳ���ʧ�ܣ����ڵ�2��ҳ��� */
		FSMC_NAND_WriteSpare(&ucFlag, ulPageNo + 1, BI_OFFSET, 1);
	}
}

/*
*********************************************************************************************************
*	�� �� ��: NAND_Format
*	����˵��: NAND Flash��ʽ�����������е����ݣ��ؽ�LUT
*	��    �Σ���
*	�� �� ֵ: NAND_OK : �ɹ��� NAND_Fail ��ʧ�ܣ�һ���ǻ����������ർ�£�
*********************************************************************************************************
*/
uint8_t NAND_Format(void)
{
	uint16_t i, n;
	uint16_t usGoodBlockCount;

	/* ����ÿ���� */
	usGoodBlockCount = 0;
	for (i = 0; i < NAND_BLOCK_COUNT; i++)
	{
		/* ����Ǻÿ飬����� */
		if (!NAND_IsBadBlock(i))
		{
			FSMC_NAND_EraseBlock(i);
			usGoodBlockCount++;
		}
	}
	
	/* ����ÿ����������100����NAND Flash���� */
	if (usGoodBlockCount < 100)
	{
		return NAND_FAIL;
	}

	usGoodBlockCount = (usGoodBlockCount * 98) / 100;	/* 98%�ĺÿ����ڴ洢���� */
		
	/* ��������һ�� */
	n = 0; /* ͳ���ѱ�ע�ĺÿ� */
	for (i = 0; i < NAND_BLOCK_COUNT; i++)
	{
		if (!NAND_IsBadBlock(i))
		{
			/* ����Ǻÿ飬���ڸÿ�ĵ�1��PAGE��LBN0 LBN1��д��nֵ (ǰ���Ѿ�ִ���˿������ */
			FSMC_NAND_WriteSpare((uint8_t *)&n, i * NAND_BLOCK_SIZE, LBN0_OFFSET, 2);		
			n++;

			/* ���㲢д��ÿ��������ECCֵ ����ʱδ����*/

			if (n == usGoodBlockCount)
			{
				break;
			}
		}
	}

	NAND_BuildLUT();	/* ��ʼ��LUT�� */
	return NAND_OK;
}

/*
*********************************************************************************************************
*	�� �� ��: NAND_FormatCapacity
*	����˵��: NAND Flash��ʽ�������Ч����
*	��    �Σ���
*	�� �� ֵ: NAND_OK : �ɹ��� NAND_Fail ��ʧ�ܣ�һ���ǻ����������ർ�£�
*********************************************************************************************************
*/
uint32_t NAND_FormatCapacity(void)
{
	uint16_t usCount;
	
	/* �������ڴ洢���ݵ����ݿ��������������Ч������98%������ */
	usCount = (s_usValidDataBlockCount * DATA_BLOCK_PERCENT) / 100;
	
	return (usCount * NAND_BLOCK_SIZE * NAND_PAGE_SIZE);
}
	
/*
*********************************************************************************************************
*	�� �� ��: NAND_DispBadBlockInfo
*	����˵��: ͨ�����ڴ�ӡ��NAND Flash�Ļ�����Ϣ
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void NAND_DispBadBlockInfo(void)
{
	uint32_t id;
	uint32_t i;
	uint32_t n;
	
	FSMC_NAND_Init();	/* ��ʼ��FSMC */
	
	id = NAND_ReadID();
	
	printf("NAND Flash ID = 0x%04X, Type = ", id);
	if (id == HY27UF081G2A)
	{
		printf("HY27UF081G2A\r\n  1024 Blocks, 64 pages per block, 2048 + 64 bytes per page\r\n");
	}
	else if (id == K9F1G08U0A)
	{
		printf("K9F1G08U0A\r\n  1024 Blocks, 64 pages per block, 2048 + 64 bytes per page\r\n");
	}
	else if (id == K9F1G08U0B)
	{
		printf("K9F1G08U0B\r\n  1024 Blocks, 64 pages per block, 2048 + 64 bytes per page\r\n");
	}
	else
	{
		printf("unkonow\r\n");
		return;
	}
	
	printf("Block Info :\r\n");
	n = 0;	/* ����ͳ�� */
	for (i = 0; i < NAND_BLOCK_COUNT; i++)
	{
		if (NAND_IsBadBlock(i))
		{
			printf("*");
			n++;
		}
		else
		{
			printf("0");
		}
		
		if (((i + 1) % 8) == 0)
		{
			printf(" ");
		}
		
		if (((i + 1) % 64) == 0)
		{
			printf("\r\n");
		}
	}
	printf("Bad Block Count = %d\r\n", n);
}