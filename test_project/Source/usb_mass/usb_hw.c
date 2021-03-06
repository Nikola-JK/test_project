/*
*********************************************************************************************************
*	                                  
*	模块名称 : USB Mass例程相关的硬件配置。
*	文件名称 : usb_hw.c
*	版    本 : V1.0
*	说    明 : USB硬件设备驱动接口
*
*********************************************************************************************************
*/

#include "stm32f10x.h"
#include "bsp_led.h"
#include "usb_hw.h"
#include "usb_lib.h"
#include "usb_desc.h"
#include "usb_pwr.h"
#include "usb_lib.h"
#include "mass_mal.h"

static void IntToUnicode (uint32_t value , uint8_t *pbuf , uint8_t len);
static void USB_Disconnect_Config(void);
static void Set_USBClock(void);
static void USB_Interrupts_Config(void);

/*
*********************************************************************************************************
*	函 数 名: MAL_Config
*	功能说明: MAL层配置
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void MAL_Config(void)
{
	MAL_Init(0);
//	MAL_Init(1);
}

/*
*********************************************************************************************************
*	函 数 名: InitUsbDev
*	功能说明: 初始化USB相关的硬件设备
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void InitUsbDev(void)
{	
	
	USB_Disconnect_Config();/* 配置控制USB连接和断开的GPIO */
  
	MAL_Config();			/* MAL configuration */
	
	Set_USBClock();			/* 设置USB时钟 */
	
	USB_Interrupts_Config();/* 配置USB中断 */
	
	USB_Init();				/* 初始化USB设备, 此函数会调用UsbConnect()连接USB设备到USB总线 */

}

/*
*********************************************************************************************************
*	函 数 名: Led_RW_ON
*	功能说明: 点亮读写指示灯
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void Led_RW_ON(void)
{
	bsp_LedOn(3);
}

/*
*********************************************************************************************************
*	函 数 名: Led_RW_ON
*	功能说明: 关闭读写指示灯
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void Led_RW_OFF(void)
{
	bsp_LedOff(3);
}

/*
*********************************************************************************************************
*	函 数 名: USB_Configured_LED
*	功能说明: 点亮USB设备连接成功指示灯
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void USB_Configured_LED(void)
{
	bsp_LedOn(1);
}

/*
*********************************************************************************************************
*	函 数 名: USB_NotConfigured_LED
*	功能说明: 关闭USB设备连接成功指示灯
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void USB_NotConfigured_LED(void)
{
	bsp_LedOff(1);
}

/*
*********************************************************************************************************
*	函 数 名: Get_SerialNum
*	功能说明: 创建串口号和描述符
*	形    参：无
*	返 回 值: 错误代码(无需处理)
*********************************************************************************************************
*/
void Get_SerialNum(void)
{
	uint32_t Device_Serial0, Device_Serial1, Device_Serial2;
	
	Device_Serial0 = *(__IO uint32_t*)(0x1FFFF7E8);
	Device_Serial1 = *(__IO uint32_t*)(0x1FFFF7EC);
	Device_Serial2 = *(__IO uint32_t*)(0x1FFFF7F0);
	
	Device_Serial0 += Device_Serial2;
	
	if (Device_Serial0 != 0)
	{
		IntToUnicode (Device_Serial0, &MASS_StringSerial[2] , 8);
		IntToUnicode (Device_Serial1, &MASS_StringSerial[18], 4);
	}
}

/*
*********************************************************************************************************
*	函 数 名: USB_Cable_Config
*	功能说明: 软件控制USB电缆连接或断开
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void USB_Cable_Config (uint8_t NewState)
{
	/* 安富莱开发板使用PB14控制USB数据线上上拉电阻。PB12=0是USB电缆连接到总线 */
	if (NewState != DISABLE)
	{
		GPIO_ResetBits(GPIOA, GPIO_Pin_8);	
	}
	else
	{
		GPIO_SetBits(GPIOA, GPIO_Pin_8);
	}
}

/*
*********************************************************************************************************
*	函 数 名: Enter_LowPowerMode
*	功能说明: 当进入挂起模式时关闭系统时钟和电源
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void Enter_LowPowerMode(void)
{
	/* 设置设备状态为“挂起” */
	bDeviceState = SUSPENDED;
}

/*
*********************************************************************************************************
*	函 数 名: Leave_LowPowerMode
*	功能说明: 当退出挂起模式时恢复系统时钟和电源
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void Leave_LowPowerMode(void)
{	
	DEVICE_INFO *pInfo = &Device_Info;
	
	/* 更新设备状态 */
	if (pInfo->Current_Configuration != 0)
	{
		/* 设备连接到主机并配置成功 */
		bDeviceState = CONFIGURED;
	}
	else
	{
		bDeviceState = ATTACHED;
	}
}

/*
*********************************************************************************************************
*	函 数 名: USB_Disconnect_Config
*	功能说明: 配置控制USB连接和断开的GPIO
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void USB_Disconnect_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
		
	/* 使能USB disconnect GPIO时钟 */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

	USB_Cable_Config(DISABLE);	/* 断开USB总线的上拉电阻 */ 
		
	/* 配置PB12为推挽输出，PB12 = 0时，USB设备连接，PB12=1时，USB设备断开 */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);  
}

/*
*********************************************************************************************************
*	函 数 名: Set_USBClock
*	功能说明: 配置USB输入时钟（48MHz）
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void Set_USBClock(void)
{
	/* 选择USBCLK时钟源 */
	RCC_USBCLKConfig(RCC_USBCLKSource_PLLCLK_1Div5);
	
	/* 使能USB时钟 */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USB, ENABLE);
}

/*
*********************************************************************************************************
*	函 数 名: USB_Interrupts_Config
*	功能说明: 配置USB中断
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void USB_Interrupts_Config(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
	
	NVIC_InitStructure.NVIC_IRQChannel = USB_LP_CAN1_RX0_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	NVIC_InitStructure.NVIC_IRQChannel = USB_HP_CAN1_TX_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

//  	/* 配置SDIO中断 */
//  	{		  
//		NVIC_InitStructure.NVIC_IRQChannel = SDIO_IRQn;
//		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
//		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
//		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
//		NVIC_Init(&NVIC_InitStructure);
//	}
}

/*
*********************************************************************************************************
*	函 数 名: IntToUnicode
*	功能说明: 将一个32位的HEX值转换为Unicode编码格式的字符串
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void IntToUnicode (uint32_t value , uint8_t *pbuf , uint8_t len)
{
	uint8_t idx = 0;
	
	for( idx = 0 ; idx < len ; idx ++)
	{
		if( ((value >> 28)) < 0xA )
		{
			pbuf[ 2* idx] = (value >> 28) + '0';
		}
		else
		{
			pbuf[2* idx] = (value >> 28) + 'A' - 10; 
		}
		
		value = value << 4;
		
		pbuf[ 2* idx + 1] = 0;
	}
}

