/******************** (C) COPYRIGHT 2008 STMicroelectronics ********************
* File Name          : stm32f10x_it.c
* Author             : MCD Application Team
* Version            : V1.1
* Date               : 05/30/2008
* Description        : Main Interrupt Service Routines.
*                      This file provides template for all exceptions handler
*                      and peripherals interrupt service routine.
********************************************************************************
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH SOFTWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
* FOR MORE INFORMATION PLEASE CAREFULLY READ THE LICENSE AGREEMENT FILE LOCATED
* IN THE ROOT DIRECTORY OF THIS FIRMWARE PACKAGE.
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "includes.h"
#include "sim900a.h"
#include "com.h"
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define HMI_FRAME_HEADER 0XEE
#define HMI_FRAME_TAIL_1 0XFF

#define LED_RUN_OFF GPIO_ResetBits(GPIOE,GPIO_Pin_3)
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

static unsigned char receive_flag, com_counter, HMIFrameTailStep;
static unsigned char com_buffer[20];
//int A[20]= {1};	//占据RAM的200000ac-200000fb位，第三台巡检仪这块儿有问题
volatile bool GSMreceive_flag = TRUE;	
FlagStatus GSMsend_OK = RESET;
volatile bool GSM_send = FALSE;
volatile bool GSM_Stop = FALSE;
FlagStatus O_OK = RESET;
static unsigned char GSM_buffer[100];
volatile uint8_t GSM_counter = 0;
volatile uint8_t O_counter;
volatile uint8_t GSM_step = 0;	
volatile uint8_t GSM_step_original = 0;	
volatile uint8_t GSM_stepM = 1;
uint8_t GSM_step5_n = 0;
uint8_t GSM_step6_n = 0;
extern volatile bool GSM_first;
//unsigned char com_str=0;

extern OS_EVENT *TxMbox;
extern OS_EVENT *InsideTxMbox;
extern OS_EVENT *DisMbox;
extern OS_EVENT *GSMSem;


//static unsigned char COM_Buffer[500] = {0};
//static uint16_t Counter = 0;

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/*******************************************************************************
* Function Name  : NMIException
* Description    : This function handles NMI exception.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void NMIException(void)
{
	LED_RUN_OFF;
}

/*******************************************************************************
* Function Name  : HardFaultException
* Description    : This function handles Hard Fault exception.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void HardFaultException(void)
{
    /* Go to infinite loop when Hard Fault exception occurs */
//		if (CoreDebug->DHCSR & 1) {  //check C_DEBUGEN == 1 -> Debugger Connected  

//__breakpoint(0);  // halt program execution here         
//		}  

    while(1) {
				LED_RUN_OFF;
    }
}

/*******************************************************************************
* Function Name  : MemManageException
* Description    : This function handles Memory Manage exception.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void MemManageException(void)
{
    /* Go to infinite loop when Memory Manage exception occurs */
    while(1) {
				LED_RUN_OFF;
    }
}

/*******************************************************************************
* Function Name  : BusFaultException
* Description    : This function handles Bus Fault exception.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void BusFaultException(void)
{
    /* Go to infinite loop when Bus Fault exception occurs */
    while(1) {
				LED_RUN_OFF;
    }
}

/*******************************************************************************
* Function Name  : UsageFaultException
* Description    : This function handles Usage Fault exception.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void UsageFaultException(void)
{
    /* Go to infinite loop when Usage Fault exception occurs */
    while(1) {
				LED_RUN_OFF;
    }
}

/*******************************************************************************
* Function Name  : DebugMonitor
* Description    : This function handles Debug Monitor exception.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DebugMonitor(void)
{
}

/*******************************************************************************
* Function Name  : SVCHandler
* Description    : This function handles SVCall exception.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SVCHandler(void)
{
}

/*******************************************************************************
* Function Name  : PendSVC
* Description    : This function handles PendSVC exception.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void PendSVC(void)
{
}

/*******************************************************************************
* Function Name  : SysTickHandler
* Description    : This function handles SysTick Handler.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SysTickHandler(void)
{
    OS_CPU_SR  cpu_sr;
    OS_ENTER_CRITICAL();  //保存全局中断标志,关总中断/* Tell uC/OS-II that we are starting an ISR*/
    OSIntNesting++;
    OS_EXIT_CRITICAL();	  //恢复全局中断标志
    OSTimeTick();     /* Call uC/OS-II's OSTimeTick(),在os_core.c文件里定义,主要判断延时的任务是否计时到*/
    /* Reload IWDG counter */
		SysTick_ISR();
//    IWDG_ReloadCounter();
	
    OSIntExit();  //在os_core.c文件里定义,如果有更高优先级的任务就绪了,则执行一次任务切换
}

/*******************************************************************************
* Function Name  : WWDG_IRQHandler
* Description    : This function handles WWDG interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void WWDG_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : PVD_IRQHandler
* Description    : This function handles PVD interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void PVD_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : TAMPER_IRQHandler
* Description    : This function handles Tamper interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TAMPER_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : RTC_IRQHandler
* Description    : This function handles RTC global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void RTC_IRQHandler(void)
{
    /* If counter is equal to 86339: one day was elapsed */
    if((RTC_GetCounter() / 3600 == 23) && (((RTC_GetCounter() % 3600) / 60) == 59) &&
            (((RTC_GetCounter() % 3600) % 60) == 59)) {        /* 23*3600 + 59*60 + 59 = 86339 */
        /* Wait until last write operation on RTC registers has finished */
        RTC_WaitForLastTask();
        /* Reset counter value */
        RTC_SetCounter(0x0);
        /* Wait until last write operation on RTC registers has finished */
        RTC_WaitForLastTask();
        /* Increment the date */
        // Date_Update();
    }

    /* Clear the RTC Second Interrupt pending bit */
    RTC_ClearITPendingBit(RTC_IT_SEC);
}

/*******************************************************************************
* Function Name  : FLASH_IRQHandler
* Description    : This function handles Flash interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void FLASH_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : RCC_IRQHandler
* Description    : This function handles RCC interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void RCC_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : EXTI0_IRQHandler
* Description    : This function handles External interrupt Line 0 request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void EXTI0_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : EXTI1_IRQHandler
* Description    : This function handles External interrupt Line 1 request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void EXTI1_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : EXTI2_IRQHandler
* Description    : This function handles External interrupt Line 2 request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void EXTI2_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : EXTI3_IRQHandler
* Description    : This function handles External interrupt Line 3 request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void EXTI3_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : EXTI4_IRQHandler
* Description    : This function handles External interrupt Line 4 request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void EXTI4_IRQHandler(void)
{
    OS_CPU_SR  cpu_sr;
    OS_ENTER_CRITICAL();   /* Tell uC/OS-II that we are starting an ISR*/
    OSIntNesting++;
    OS_EXIT_CRITICAL();

    /*
    if(LedTick&0x0001)
      {
      	GPIO_SetBits(GPIOC,GPIO_Pin_13);
      }
    else
      {
      	 GPIO_ResetBits(GPIOC,GPIO_Pin_13);
      } */
    if(EXTI_GetITStatus(EXTI_Line4) != RESET) {
        //OSTaskResume(APP_TASK_START_PRIO);
        /* Clear the EXTI Line 3 */
        EXTI_ClearITPendingBit(EXTI_Line4);
    }

    OSIntExit();  //在os_core.c文件里定义,如果有更高优先级的任务就绪了,则执行一次任务切换
}

/*******************************************************************************
* Function Name  : DMA1_Channel1_IRQHandler
* Description    : This function handles DMA1 Channel 1 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DMAChannel1_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : DMA1_Channel2_IRQHandler
* Description    : This function handles DMA1 Channel 2 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DMAChannel2_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : DMA1_Channel3_IRQHandler
* Description    : This function handles DMA1 Channel 3 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DMAChannel3_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : DMA1_Channel4_IRQHandler
* Description    : This function handles DMA1 Channel 4 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DMAChannel4_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : DMA1_Channel5_IRQHandler
* Description    : This function handles DMA1 Channel 5 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DMAChannel5_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : DMA1_Channel6_IRQHandler
* Description    : This function handles DMA1 Channel 6 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DMAChannel6_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : DMA1_Channel7_IRQHandler
* Description    : This function handles DMA1 Channel 7 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DMAChannel7_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : ADC1_2_IRQHandler
* Description    : This function handles ADC1 and ADC2 global interrupts requests.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void ADC_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : USB_HP_CAN_TX_IRQHandler
* Description    : This function handles USB High Priority or CAN TX interrupts
*                  requests.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void USB_HP_CAN1_TX_IRQHandler(void)
{
	CTR_HP();
}

/*******************************************************************************
* Function Name  : USB_LP_CAN_RX0_IRQHandler
* Description    : This function handles USB Low Priority or CAN RX0 interrupts
*                  requests.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void USB_LP_CAN1_RX0_IRQHandler(void)
{
	USB_Istr();
}

/*******************************************************************************
* Function Name  : CAN_RX1_IRQHandler
* Description    : This function handles CAN RX1 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void CAN_RX1_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : CAN_SCE_IRQHandler
* Description    : This function handles CAN SCE interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void CAN_SCE_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : EXTI9_5_IRQHandler
* Description    : This function handles External lines 9 to 5 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void EXTI9_5_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : TIM1_BRK_IRQHandler
* Description    : This function handles TIM1 Break interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TIM1_BRK_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : TIM1_UP_IRQHandler
* Description    : This function handles TIM1 overflow and update interrupt
*                  request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TIM1_UP_IRQHandler(void)
{
    /* Clear the TIM1 Update pending bit */
}

/*******************************************************************************
* Function Name  : TIM1_TRG_COM_IRQHandler
* Description    : This function handles TIM1 Trigger and commutation interrupts
*                  requests.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TIM1_TRG_COM_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : TIM1_CC_IRQHandler
* Description    : This function handles TIM1 capture compare interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TIM1_CC_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : TIM2_IRQHandler
* Description    : This function handles TIM2 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TIM2_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : TIM3_IRQHandler
* Description    : This function handles TIM3 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TIM3_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : TIM4_IRQHandler
* Description    : This function handles TIM4 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TIM4_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : I2C1_EV_IRQHandler
* Description    : This function handles I2C1 Event interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void I2C1_EV_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : I2C1_ER_IRQHandler
* Description    : This function handles I2C1 Error interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void I2C1_ER_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : I2C2_EV_IRQHandler
* Description    : This function handles I2C2 Event interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void I2C2_EV_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : I2C2_ER_IRQHandler
* Description    : This function handles I2C2 Error interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void I2C2_ER_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : SPI1_IRQHandler
* Description    : This function handles SPI1 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SPI1_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : SPI2_IRQHandler
* Description    : This function handles SPI2 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SPI2_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : USART1_IRQHandler
* Description    : This function handles USART1 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/

extern char com_buff[20] = {0};
extern int UartHaveData = 0;
uint16_t com1_counter = 0;

void USART1_IRQHandler(void)
{
    char temp;
    OS_CPU_SR  cpu_sr;
    OS_ENTER_CRITICAL();  //保存全局中断标志,关总中断/* Tell uC/OS-II that we are starting an ISR*/
    OSIntNesting++;
    OS_EXIT_CRITICAL();	  //恢复全局中断标志
	
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
	 {
	 	
		com_buff[com_counter] = USART_ReceiveData(USART1);
 
		    com_counter++;
		    if(com_counter == 20 || 0x0D == com_buff[com_counter-1])//接受完毕条件，超过20个字符或回车
		    {
			     /* Disable the USART1 Receive interrupt */
			     USART_ITConfig(USART1, USART_IT_RXNE, DISABLE);
			     com_buff[com_counter] = '\0';
			     UartHaveData = 1;
			     com_counter = 0;
		    }
		 
 

      		USART_SendData(USART1,com_buff[com_counter-1]); 
	        USART_ClearITPendingBit(USART1, USART_IT_RXNE);
	 }
	
#if 0
    if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET) {
        temp =  USART_ReceiveData(USART1);

        if(receive_flag == 1) {
            com_buffer[com_counter] = temp;
            com_counter++;

            if((temp == HMI_FRAME_TAIL_1) && (!HMIFrameTailStep)) {
                HMIFrameTailStep = 1;
            }

            switch(HMIFrameTailStep) {
                case 1: {
                    HMIFrameTailStep = 2;
                    break;
                }

                case 2: {
                    if(temp == 0XFC) {
                        HMIFrameTailStep = 3;

                    } else {
                        if(temp == 0xff) {
                            HMIFrameTailStep = 2;

                        } else {
                            HMIFrameTailStep = 0;
                        }
                    }

                    break;
                }

                case 3: {
                    if(temp == 0XFF) {
                        HMIFrameTailStep = 4;

                    } else {
                        HMIFrameTailStep = 0;
                    }

                    break;
                }

                case 4: {
                    if(temp == 0xff) {
                        /*接收完成一条完整指令*/
                        receive_flag = 0;
                        com_counter = 0;
                        HMIFrameTailStep = 0;
												
                        OSMboxPost(DisMbox, (void *)com_buffer);
											
                    } else {
                        if(temp == 0xfc) {
                            HMIFrameTailStep = 3;

                        } else {
                            HMIFrameTailStep = 0;
                        }
                    }

                    break;
                }

                default:
                    HMIFrameTailStep = 0;
                    break;
            }
        }

        if(temp == HMI_FRAME_HEADER) {
            com_buffer[com_counter] = temp;
            com_counter++;
            receive_flag = 1;
        }
    }

#endif
    OSIntExit();  //在os_core.c文件里定义,如果有更高优先级的任务就绪了,则执行一次任务切换
}

/*******************************************************************************
* Function Name  : USART2_IRQHandler
* Description    : 用于外部485通信（收敛中心与上位机）
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/

#define FRAME_HEADER 0XEE
#define FRAME_TAIL   0XED

static unsigned char COM_485_Buffer[500] = {0};
static uint16_t Counter_485 = 0;
static uint16_t Datalen = 0;
static PROTOCOL_NODE ProtocolNode = NONE;

void USART2_IRQHandler(void)
{
 
    char temp;
    OS_CPU_SR  cpu_sr;
    OS_ENTER_CRITICAL();  //保存全局中断标志,关总中断/* Tell uC/OS-II that we are starting an ISR*/
    OSIntNesting++;
    OS_EXIT_CRITICAL();	  //恢复全局中断标志

    if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET) {
        temp =  USART_ReceiveData(USART2);

        /*通讯协议解析 协议格式
         帧头（8bit）+从机地址（8bit）+指令代码（8bit）+数据长度（16bit）
        +数据内容+CRC16（8bit）+帧尾（8bit）*/
        switch(ProtocolNode) {
            case NONE: {
                if(temp == FRAME_HEADER) {
                    memset(COM_485_Buffer, 0, 500);
                    Counter_485 = 0;
                    COM_485_Buffer[Counter_485] = temp;
                    Counter_485++;
                    ProtocolNode = COM_FRAME_ADDR;
                }

                break;
            }

//	    case COM_FRAME_HEADER:
//		{
//		  COM_485_Buffer[Counter_485] = temp;
//		  Counter_485++;
//
//		  ProtocolNode = COM_FRAME_ADDR;
//		 break;
//		}
            case COM_FRAME_ADDR: {
                COM_485_Buffer[Counter_485] = temp;
                Counter_485++;
                ProtocolNode = COM_FRAME_I_CODE;
                break;
            }

            case COM_FRAME_I_CODE: {
                COM_485_Buffer[Counter_485] = temp;
                Counter_485++;
                ProtocolNode = COM_FRAME_D_LENGTH_HI;
                break;
            }

            case COM_FRAME_D_LENGTH_HI: {
                COM_485_Buffer[Counter_485] = temp;
                Counter_485++;
                Datalen = temp;
                ProtocolNode = COM_FRAME_D_LENGTH_LO;
                break;
            }

            case COM_FRAME_D_LENGTH_LO: {
                COM_485_Buffer[Counter_485] = temp;
                Counter_485++;
                Datalen = (Datalen << 8) + temp;

                if(Datalen == 0) {
                    ProtocolNode = COM_FRAME_TAIL;

                } else if(Datalen > 500) {
                    ProtocolNode = NONE;

                } else {
                    ProtocolNode = COM_FRAME_D_CONTENT;
                }

                break;
            }

            case COM_FRAME_D_CONTENT: {
                COM_485_Buffer[Counter_485] = temp;
                Counter_485++;
                Datalen--;

                if(Datalen == 0) {
                    if(COM_485_Buffer[2] == HAND_SHAKE) {
                        ProtocolNode = COM_FRAME_TAIL;

                    } else {
                        ProtocolNode = COM_FRAME_CRC_HI;
                    }
                }

                break;
            }

            case COM_FRAME_CRC_HI: {
                COM_485_Buffer[Counter_485] = temp;
                Counter_485++;
                ProtocolNode = COM_FRAME_CRC_LO;
                break;
            }

            case COM_FRAME_CRC_LO: {
                COM_485_Buffer[Counter_485] = temp;
                Counter_485++;
                ProtocolNode = COM_FRAME_TAIL;
                break;
            }

            case COM_FRAME_TAIL: {
                COM_485_Buffer[Counter_485] = temp;
                Counter_485++;

                if(temp == FRAME_TAIL) {
                    /*接收完整指令 发送信息解析指令*/
                   OSMboxPost(TxMbox, (void *)COM_485_Buffer);
									
                } else {
                }

                ProtocolNode = NONE;
                break;
            }

            default:
                break;
        }
    }

    OSIntExit();  //在os_core.c文件里定义,如果有更高优先级的任务就绪了,则执行一次任务切换

}

/*******************************************************************************
* Function Name  : USART3_IRQHandler
* Description    : 用于内部485通讯（收敛中心与检测模块）
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/

static unsigned char InsideCOM_485_Buffer[500] = {0};
static uint16_t InsideCounter_485 = 0;
static uint16_t InsideDatalen = 0;
static PROTOCOL_NODE InsideProtocolNode = NONE;

void USART3_IRQHandler(void)
{
   
    char temp;
    OS_CPU_SR  cpu_sr;
    OS_ENTER_CRITICAL();  //保存全局中断标志,关总中断/* Tell uC/OS-II that we are starting an ISR*/
    OSIntNesting++;
    OS_EXIT_CRITICAL();	  //恢复全局中断标志

    if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET) {
        temp =  USART_ReceiveData(USART3);

        /*通讯协议解析 协议格式
         帧头（8bit）+从机地址（8bit）+指令代码（8bit）+数据长度（16bit）
        +数据内容+CRC16（8bit）+帧尾（8bit）*/
        switch(InsideProtocolNode) {
            case NONE: {
                if(temp == FRAME_HEADER) {
                    memset(InsideCOM_485_Buffer, 0, 500);
                    InsideCounter_485 = 0;
                    InsideCOM_485_Buffer[InsideCounter_485] = temp;
                    InsideCounter_485++;
                    InsideProtocolNode = COM_FRAME_ADDR;
                }

                break;
            }

//	    case COM_FRAME_HEADER:
//		{
//		  InsideCOM_485_Buffer[InsideCounter_485] = temp;
//		  InsideCounter_485++;
//
//		  InsideProtocolNode = COM_FRAME_ADDR;
//		 break;
//		}
            case COM_FRAME_ADDR: {
                InsideCOM_485_Buffer[InsideCounter_485] = temp;
                InsideCounter_485++;
                InsideProtocolNode = COM_FRAME_I_CODE;
                break;
            }

            case COM_FRAME_I_CODE: {
                InsideCOM_485_Buffer[InsideCounter_485] = temp;
                InsideCounter_485++;
                InsideProtocolNode = COM_FRAME_D_LENGTH_HI;
                break;
            }

            case COM_FRAME_D_LENGTH_HI: {
                InsideCOM_485_Buffer[InsideCounter_485] = temp;
                InsideCounter_485++;
                InsideDatalen = temp;
                InsideProtocolNode = COM_FRAME_D_LENGTH_LO;
                break;
            }

            case COM_FRAME_D_LENGTH_LO: {
                InsideCOM_485_Buffer[InsideCounter_485] = temp;
                InsideCounter_485++;
                InsideDatalen = (InsideDatalen << 8) + temp;

                if(InsideDatalen == 0) {
                    InsideProtocolNode = COM_FRAME_TAIL;

                } else if(InsideDatalen > 500) {
                    InsideProtocolNode = NONE;

                } else {
                    InsideProtocolNode = COM_FRAME_D_CONTENT;
                }

                break;
            }

            case COM_FRAME_D_CONTENT: {
                InsideCOM_485_Buffer[InsideCounter_485] = temp;
                InsideCounter_485++;
                InsideDatalen--;

                if(InsideDatalen == 0) {
                    if(InsideCOM_485_Buffer[2] == HAND_SHAKE) {
                        InsideProtocolNode = COM_FRAME_TAIL;

                    } else {
                        InsideProtocolNode = COM_FRAME_CRC_HI;
                    }
                }

                break;
            }

            case COM_FRAME_CRC_HI: {
                InsideCOM_485_Buffer[InsideCounter_485] = temp;
                InsideCounter_485++;
                InsideProtocolNode = COM_FRAME_CRC_LO;
                break;
            }

            case COM_FRAME_CRC_LO: {
                InsideCOM_485_Buffer[InsideCounter_485] = temp;
                InsideCounter_485++;
                InsideProtocolNode = COM_FRAME_TAIL;
                break;
            }

            case COM_FRAME_TAIL: {
                InsideCOM_485_Buffer[InsideCounter_485] = temp;
                InsideCounter_485++;

                if(temp == FRAME_TAIL) {
                    /*接收完整指令 发送信息解析指令*/
                    OSMboxPost(InsideTxMbox, (void *)InsideCOM_485_Buffer);
									
                } else {
                }

                InsideProtocolNode = NONE;
                break;
            }

            default:
                break;
        }
    }

    OSIntExit();  //在os_core.c文件里定义,如果有更高优先级的任务就绪了,则执行一次任务切换
}
	
/*******************************************************************************
* Function Name  : EXTI15_10_IRQHandler
* Description    : This function handles External lines 15 to 10 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void EXTI15_10_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : RTCAlarm_IRQHandler
* Description    : This function handles RTC Alarm interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void RTCAlarm_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : USBWakeUp_IRQHandler
* Description    : This function handles USB WakeUp interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void USBWakeUp_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : TIM8_BRK_IRQHandler
* Description    : This function handles TIM8 Break interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TIM8_BRK_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : TIM8_UP_IRQHandler
* Description    : This function handles TIM8 overflow and update interrupt
*                  request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TIM8_UP_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : TIM8_TRG_COM_IRQHandler
* Description    : This function handles TIM8 Trigger and commutation interrupts
*                  requests.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TIM8_TRG_COM_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : TIM8_CC_IRQHandler
* Description    : This function handles TIM8 capture compare interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TIM8_CC_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : ADC3_IRQHandler
* Description    : This function handles ADC3 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void ADC3_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : FSMC_IRQHandler
* Description    : This function handles FSMC global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void FSMC_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : SDIO_IRQHandler
* Description    : This function handles SDIO global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SDIO_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : TIM5_IRQHandler
* Description    : This function handles TIM5 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TIM5_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : SPI3_IRQHandler
* Description    : This function handles SPI3 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SPI3_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : UART4_IRQHandler
* Description    : This function handles UART4 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void UART4_IRQHandler(void)
{
		char temp;
		
    OS_CPU_SR  cpu_sr;
    OS_ENTER_CRITICAL();  //保存全局中断标志,关总中断/* Tell uC/OS-II that we are starting an ISR*/
    OSIntNesting++;
    OS_EXIT_CRITICAL();	  //恢复全局中断标志
		
    if(USART_GetITStatus(UART4, USART_IT_RXNE) != RESET) 
		{
			temp =  USART_ReceiveData(UART4);
		}
		
		 OSIntExit();  //在os_core.c文件里定义,如果有更高优先级的任务就绪了,则执行一次任务切换
}

/*******************************************************************************
* Function Name  : UART5_IRQHandler
* Description    : This function handles UART5 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void UART5_IRQHandler(void)
{
    char temp;
    OS_CPU_SR  cpu_sr;
    OS_ENTER_CRITICAL();  //保存全局中断标志,关总中断/* Tell uC/OS-II that we are starting an ISR*/
    OSIntNesting++;
    OS_EXIT_CRITICAL();	  //恢复全局中断标志

    if(USART_GetITStatus(UART5, USART_IT_RXNE) != RESET) {
      temp =  USART_ReceiveData(UART5);
	
			if(GSMreceive_flag) {
					GSM_buffer[GSM_counter] = temp;
					GSM_counter++;

					if(temp == 0x4f) { //"O"的ACSII码
						O_OK = SET;
						O_counter = GSM_counter;
						OSIntExit();
						return;
					}
					if(O_OK) {
						if(GSM_buffer[O_counter] == 0x4b) { //"K"的ACSII码
							switch(GSM_step){
								case 0: 
									GSM_step = 1;
									break;
								case 1:
									GSM_step = 2;
									break;
								case 2: 
									GSM_step = 3;
									break;
								default:
									break;
							}
							O_OK = RESET;
						}
					}
					
					if(temp == 0x3e) { //">"的ACSII码
							GSM_step = 4;
						}

					if(temp == 0x2b) { //"+"的ACSII码
						GSM_step_original = GSM_step;	
						GSM_step = 5;
						GSM_counter = 1;
						OSIntExit();
						return;
					}
					/*GSM返回ERROR*/
					if(temp == 0x52) { //"R"的ACSII码
						GSM_step_original = GSM_step;	
						GSM_step = 6;
						GSM_counter = 1;
						OSIntExit();
						return;
					}
					
					switch(GSM_step){
						
						 case 1: {				
										GSM_counter = 0;
										GSM_stepM = 2;							
								break;		
						}
						
						case 2: {
										GSM_counter = 0;
										GSM_stepM = 3;
							break;
						}
						
						case 3: {
										GSM_counter = 0;
										GSM_stepM = 4;
							break;
						}
						
						case 4: {	
										GSM_stepM = 5;
							break;
						}
						
						case 5: {								
								/*+CMGS:*/
								GSM_step5_n++;
								if(GSM_step5_n == 5){
									GSM_step5_n = 0;
									if(GSM_buffer[1] == 0x43 && GSM_buffer[2] == 0x4d &&	GSM_buffer[3] == 0x47 && GSM_buffer[4] == 0x53 && GSM_buffer[5] == 0x3a) {	//"CMGS:"的ASCII码
										GSMsend_OK = SET;						
										GSM_stepM = 2;
										GSM_step = 1;
										if(!GSM_first){
											GSMreceive_flag = FALSE;	//停止接受
											GSM_send = FALSE;	//停止发送
											GSM_Stop = TRUE; 
										}
										GSM_first = FALSE; //短信分两条发时，发完第一条后，其标志位置0												
								 }else{
									 GSM_step = GSM_step_original;
								 }
							 }
							break;
						}	
						case 6: {	
							/*ERROR*/
							GSM_step6_n++;
							if(GSM_step6_n == 3){
								GSM_step6_n = 0;	
//								if( GSM_buffer[1] == 0x52 &&	GSM_buffer[2] == 0x4f && GSM_buffer[3] == 0x52) {	//"ERROR"的“ROR”的ASCII码					
										GSM_stepM = 2;
										GSM_step = 1;
//								 }else{
//									 GSM_step = GSM_step_original;
//								 }
							
							}
							break;
						}
						default:
							GSM_step = 0;
							break;
						
					}
				
			}
			
		}
    OSIntExit();  //在os_core.c文件里定义,如果有更高优先级的任务就绪了,则执行一次任务切换

}

/*******************************************************************************
* Function Name  : TIM6_IRQHandler
* Description    : This function handles TIM6 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TIM6_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : TIM7_IRQHandler
* Description    : This function handles TIM7 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TIM7_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : DMA2_Channel1_IRQHandler
* Description    : This function handles DMA2 Channel 1 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DMA2_Channel1_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : DMA2_Channel2_IRQHandler
* Description    : This function handles DMA2 Channel 2 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DMA2_Channel2_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : DMA2_Channel3_IRQHandler
* Description    : This function handles DMA2 Channel 3 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DMA2_Channel3_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : DMA2_Channel4_5_IRQHandler
* Description    : This function handles DMA2 Channel 4 and DMA2 Channel 5
*                  interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DMA2_Channel4_5_IRQHandler(void)
{
}

/******************* (C) COPYRIGHT 2008 STMicroelectronics *****END OF FILE****/
