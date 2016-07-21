/****************************************************************************

* 文件名: sysytick.c
* 内容简述: 本模块包含systick中断的设置，以及软件定时器的实现。
*

*/

#include "stm32f10x.h"
#include <stdio.h>
#include <string.h>


#include "systick.h"
#include "button.h"		/* 包含按键检测模块的头文件 */

extern int UartHaveData;
extern char com_buff[20];
extern int jump = 0;
extern void AdcPro(void);
extern uint32_t tosecond(int16_t year,int16_t month,int16_t date,int16_t hour,int16_t min,int16_t sec);
/* 在此定义若干个软件定时器全局变量
注意，必须增加volatile，因为这个变量在中断和主程序中同时
被访问，有可能造成编译器错误优化。DelayMS曾出过问题
*/
#define TMR_COUNT	4		/* 软件定时器的个数 */
SOFT_TMR g_Tmr[TMR_COUNT];

/* 全局运行时间，单位10ms，用于uIP，最大 */
__IO int32_t g_RunTime = 0;

/*******************************************************************************
	函数名：SoftTimerDec
	输  入: 定时器变量指针,每隔1ms减1
	输  出:
	功能说明：
*/
static void SoftTimerDec(SOFT_TMR *_tmr)
{
	if (_tmr->flag == 0)
	{
		if (_tmr->count > 0)
		{
			if (--_tmr->count == 0)
			{
				_tmr->flag = 1;
			}
		}
	}
}

/*******************************************************************************
	函数名：SysTick_ISR
	输  入:
	输  出:
	功能说明：SysTick中断服务程序，每隔1ms进入1次
*/
void SysTick_ISR(void)
{
	static uint8_t s_count = 0;
	uint8_t i;
	int setpos;
	uint32_t set_time;
	uint32_t tmp_year, tmp_month, tmp_day, tmp_hour,tmp_min,tmp_sec;

	for (i = 0; i < TMR_COUNT; i++)
	{
		SoftTimerDec(&g_Tmr[i]);
	}

	/* 下面的代码实现所有按键的检测
		按键检测每隔10ms一次就行了，一般做40ms的滤波处理就可以有效过滤掉机械动作造成的
		按键抖动。
	*/
	if (++s_count >= 10)
	{
		s_count = 0;

		g_RunTime++;	/* 全局运行时间每10ms增1 */	
		if (g_RunTime == 0x80000000)
		{
			g_RunTime = 0;
		}
		AdcPro();

		
		
//		KeyPro();		/* 该函数在 button.c 中实现 */
	}

	IWDG_ReloadCounter();


	if (UartHaveData)

     {

       setpos=0;
       if((com_buff[0] == 'j') && (com_buff[1] == 'u') && (com_buff[2] == 'm') && (com_buff[3] == 'p'))
       		jump = 1;

        //while (com_buff[setpos] != 't') setpos++;
	if(com_buff[setpos] == 't'){

			tmp_year = (com_buff[setpos+1]-'0')*1000 + (com_buff[setpos+2]-'0')*100 + (com_buff[setpos+3]-'0')*10 + (com_buff[setpos+4]-'0');
			tmp_month = (com_buff[setpos+5]-'0')*10 + (com_buff[setpos+6]-'0');
			tmp_day = (com_buff[setpos+7]-'0')*10 + (com_buff[setpos+8]-'0');
			tmp_hour = (com_buff[setpos+9]-'0')*10 + (com_buff[setpos+10]-'0');
			tmp_min = (com_buff[setpos+11]-'0')*10 + (com_buff[setpos+12]-'0');
			tmp_sec = (com_buff[setpos+13]-'0')*10 + (com_buff[setpos+14]-'0');
			
			set_time = tosecond(tmp_year, tmp_month,tmp_day,tmp_hour,tmp_min,tmp_sec);

			RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);	
			PWR_BackupAccessCmd(ENABLE);

			RTC_WaitForLastTask();
			RTC_SetCounter(set_time);
			RTC_WaitForLastTask();
			}


//        Uart1_PutString(com_buff,10);//返回数据，检验是否正常

        UartHaveData=0;
	//memset(com_buff,0,sizeof(com_buff));

        USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);

     }
}

/*******************************************************************************
	函数名：DelayMS
	输  入: 延迟长度，单位1 ms. 延迟精度为正负1ms
	输  出:
	功能说明：延时函数。占用软件定时器0
*/
void delay_ms(uint32_t n)
{
	/* 避免 n = 1 出现主程序死锁 */
	if (n == 1)
	{
		n = 2;
	}
	g_Tmr[0].count = n;
	g_Tmr[0].flag = 0;

	/* while 循环体最好让CPU进入IDLE状态，已降低功耗 */
	while (1)
	{
		CPU_IDLE();

		if (g_Tmr[0].flag == 1)
		{
			break;
		}
	}
}

/*******************************************************************************
	函数名：StartTimer
	输  入: 定时器ID (0 - 3)
	输  出:
	功能说明：
*/
void StartTimer(uint8_t _id, uint32_t _period)
{
	if (_id >= TMR_COUNT)
	{
		return;
	}

	g_Tmr[_id].count = _period;
	g_Tmr[_id].flag = 0;
}

/*******************************************************************************
	函数名：StartTimer
	输  入: 定时器ID (0 - 3)
	输  出: 返回 0 表示定时未到， 1表示定时到
	功能说明：
*/
uint8_t CheckTimer(uint8_t _id)
{
	if (_id >= TMR_COUNT)
	{
		return 0;
	}

	return g_Tmr[_id].flag;
}

/*******************************************************************************
*	函数名：GetRunTime
*	输  入: 无
*	返  回: 相对复位开始的系统运行时间，单位10ms
*	功能说明：
*/
int32_t GetRunTime(void)
{
	int runtime; 

	__set_PRIMASK(1);  		/* 关中断 */
	
	runtime = g_RunTime;	/* 由于在Systick中断被改写，因此关中断进行保护 */
		
	__set_PRIMASK(0);  		/* 开中断 */

	return runtime;
}
