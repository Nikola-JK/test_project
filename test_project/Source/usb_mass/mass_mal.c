

#include "stm32f10x.h"
#include "stdio.h"
#include "bsp_nand.h"
#include "mass_mal.h"

uint32_t Mass_Memory_Size[1];
uint32_t Mass_Block_Size[1];
uint32_t Mass_Block_Count[1];
//__IO uint32_t Status = 0;

uint32_t Max_Lun = 0;	

/*
*********************************************************************************************************
*	�� �� ��: MAL_Init
*	����˵��: ��ʼ���洢�豸������USB Mass Storage��
*	��    �Σ�lun �� SCSI�߼���Ԫ�ţ�0��ʾSD����1��ʾNAND Flash
*	�� �� ֵ: MAL_OK : �ɹ���MAL_FAIL : ʧ��
*********************************************************************************************************
*/
uint16_t MAL_Init(uint8_t lun)
{
	uint16_t status = MAL_OK;
	int i = 0;
	
	switch (lun)
	{

		
		case MASS_NAND:
ERROR:
			if (NAND_Init() != NAND_OK)	/* ��ʼ��NAND Flash Ӳ���豸������ʱ�ӡ�FSMC��GPIO��ʹ��FSMC */
			{
				mass_printf_err("NAND_Init() fail : file %s on line %d\r\n", __FILE__, __LINE__);
					printf("NAND_Init() Error! \r\n");
					i++;
					if(i < 10)
						goto ERROR;			
				status = MAL_FAIL;
			}
			else
			{
				mass_printf_ok("NAND_Init() Ok\r\n");
				status = MAL_OK;
			}
			break;
		
		default:
			break;
	}
	
	return status;
}

/*
*********************************************************************************************************
*	�� �� ��: MAL_Write
*	����˵��: д����
*	��    �Σ�lun �� SCSI�߼���Ԫ�ţ�0��ʾSD����1��ʾNAND Flash
*			  Memory_Offset : �洢��Ԫƫ�Ƶ�ַ
*			  Writebuff     ����д������ݻ�������ָ��
*			  Transfer_Length ����д����ֽ����� ������ 512
*	�� �� ֵ: MAL_OK : �ɹ���MAL_FAIL : ʧ��
*********************************************************************************************************
*/
uint16_t MAL_Write(uint8_t lun, uint32_t Memory_Offset, uint32_t *Writebuff, uint16_t Transfer_Length)
{
	uint16_t status = MAL_OK;
		
	switch (lun)
	{
		
		case MASS_NAND:
			if (NAND_Write(Memory_Offset, Writebuff, Transfer_Length) != NAND_OK)
			{
				mass_printf_err("NAND_Write(0x%X, ,0x%X) Fail\r\n", Memory_Offset, Transfer_Length);
				status = MAL_FAIL;
			}
			else
			{
				mass_printf_ok("NAND_Write(0x%X, ,0x%X) Ok\r\n", Memory_Offset, Transfer_Length);			
				status = MAL_OK;
			}
			break;
		
		default:
			break;
	}
	return status;
}

/*
*********************************************************************************************************
*	�� �� ��: MAL_Read
*	����˵��: ��������֧�ֶ������
*	��    �Σ�lun �� SCSI�߼���Ԫ�ţ�
*			  Memory_Offset : �洢��Ԫƫ�Ƶ�ַ
*			  Readbuff      ���洢���������ݵĻ�������ָ��
*			  Transfer_Length ����Ҫ�������ֽ����� ������ 512
*	�� �� ֵ: MAL_OK : �ɹ���MAL_FAIL : ʧ��
*********************************************************************************************************
*/
uint16_t MAL_Read(uint8_t lun, uint32_t Memory_Offset, uint32_t *Readbuff, uint16_t Transfer_Length)
{
	uint16_t status = MAL_OK;
	
	switch (lun)
	{

	
		case MASS_NAND:
			if (NAND_Read(Memory_Offset, Readbuff, Transfer_Length) != NAND_OK)
			{
				mass_printf_err("NAND_Read(0x%X, ,0x%X) Fail\r\n", Memory_Offset, Transfer_Length);
				status = MAL_FAIL;
			}
			else
			{
				mass_printf_ok("NAND_Read(0x%X, ,0x%X) Ok\r\n", Memory_Offset, Transfer_Length);			
				status = MAL_OK;
			}			
			break;
			
		default:
			break;
	}
	return status;
}

/*
*********************************************************************************************************
*	�� �� ��: MAL_GetStatus
*	����˵��: ��ȡ�洢�豸��״̬��Ϣ
*	��    �Σ�lun �� SCSI�߼���Ԫ�ţ�0��ʾSD����1��ʾNAND Flash
*	�� �� ֵ: MAL_OK : �ɹ���MAL_FAIL : ʧ��
*********************************************************************************************************
*/
uint16_t MAL_GetStatus (uint8_t lun)
{
	uint32_t nand_id;			/* ����NAND ID */
	uint16_t status = MAL_OK;
	
	switch (lun)
	{

		
		case MASS_NAND:
		{	
			nand_id = NAND_ReadID();	/* ��ȡNAND Flash��������ID������ID */

			/* �ж�NAND_ID�Ƿ���ȷ */
			if ((nand_id == HY27UF081G2A) || (nand_id == K9F1G08U0A) || (nand_id == K9F1G08U0B))
			{
				/*  ����HY27UF081G2A, 1������ÿ��1024�飬ÿ��64��ҳ��ÿҳ2048�ֽ�
					 Mass������ Mass_Block_Count = 1024 * 64 * 1 = 64K ��
					 Mass���С Mass_Block_Size = 2048 �ֽ�
					 �����洢����С Mass_Memory_Size = 64K * 2048 = 128M �ֽ�
				*/
			#if 1
				//Mass_Block_Count[0] = 256 * 1024;
				Mass_Block_Count[0] = 250 * 1024;
				Mass_Block_Size[0]  = 512;
				Mass_Memory_Size[0] = (Mass_Block_Count[0] * Mass_Block_Size[0]);
			#else
				Mass_Block_Count[0] = NAND_ZONE_SIZE * NAND_BLOCK_SIZE * NAND_MAX_ZONE;
				Mass_Block_Size[0]  = NAND_PAGE_SIZE;
				Mass_Memory_Size[0] = (Mass_Block_Count[0] * Mass_Block_Size[0]);
			#endif
				mass_printf_ok("MAL_GetStatus(MASS_NAND) Ok. Memory Size = %uMB\r\n", Mass_Memory_Size[1]/(1024*1024));
				status = MAL_OK;
			}
			else
			{
				mass_printf_err("MAL_GetStatus(MASS_NAND) NAND_ReadID() Fail\r\n");					
				status = MAL_FAIL;
			}			
			break;
		}
			
		default:
			break;			
	}
	return status;
}
