

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
*	函 数 名: MAL_Init
*	功能说明: 初始化存储设备（用于USB Mass Storage）
*	形    参：lun ： SCSI逻辑单元号，0表示SD卡，1表示NAND Flash
*	返 回 值: MAL_OK : 成功；MAL_FAIL : 失败
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
			if (NAND_Init() != NAND_OK)	/* 初始化NAND Flash 硬件设备（配置时钟、FSMC、GPIO并使能FSMC */
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
*	函 数 名: MAL_Write
*	功能说明: 写扇区
*	形    参：lun ： SCSI逻辑单元号，0表示SD卡，1表示NAND Flash
*			  Memory_Offset : 存储单元偏移地址
*			  Writebuff     ：待写入的数据缓冲区的指针
*			  Transfer_Length ：待写入的字节数， 不大于 512
*	返 回 值: MAL_OK : 成功；MAL_FAIL : 失败
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
*	函 数 名: MAL_Read
*	功能说明: 读扇区，支持多个扇区
*	形    参：lun ： SCSI逻辑单元号，
*			  Memory_Offset : 存储单元偏移地址
*			  Readbuff      ：存储读出的数据的缓冲区的指针
*			  Transfer_Length ：需要读出的字节数， 不大于 512
*	返 回 值: MAL_OK : 成功；MAL_FAIL : 失败
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
*	函 数 名: MAL_GetStatus
*	功能说明: 读取存储设备的状态信息
*	形    参：lun ： SCSI逻辑单元号，0表示SD卡，1表示NAND Flash
*	返 回 值: MAL_OK : 成功；MAL_FAIL : 失败
*********************************************************************************************************
*/
uint16_t MAL_GetStatus (uint8_t lun)
{
	uint32_t nand_id;			/* 保存NAND ID */
	uint16_t status = MAL_OK;
	
	switch (lun)
	{

		
		case MASS_NAND:
		{	
			nand_id = NAND_ReadID();	/* 读取NAND Flash的制造商ID和器件ID */

			/* 判断NAND_ID是否正确 */
			if ((nand_id == HY27UF081G2A) || (nand_id == K9F1G08U0A) || (nand_id == K9F1G08U0B))
			{
				/*  对于HY27UF081G2A, 1个区，每区1024块，每块64个页，每页2048字节
					 Mass块数量 Mass_Block_Count = 1024 * 64 * 1 = 64K 个
					 Mass块大小 Mass_Block_Size = 2048 字节
					 整个存储器大小 Mass_Memory_Size = 64K * 2048 = 128M 字节
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
