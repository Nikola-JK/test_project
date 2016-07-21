/****************************************************************************

* �ļ���: sysytick.c
* ���ݼ���: ��ģ�����systick�жϵ����ã��Լ������ʱ����ʵ�֡�
*

*/

#include "stm32f10x.h"
#include <stdio.h>
#include <string.h>


#include "systick.h"
#include "button.h"		/* �����������ģ���ͷ�ļ� */

extern int UartHaveData;
extern char com_buff[20];
extern int jump = 0;
extern void AdcPro(void);
extern uint32_t tosecond(int16_t year,int16_t month,int16_t date,int16_t hour,int16_t min,int16_t sec);
/* �ڴ˶������ɸ������ʱ��ȫ�ֱ���
ע�⣬��������volatile����Ϊ����������жϺ���������ͬʱ
�����ʣ��п�����ɱ����������Ż���DelayMS����������
*/
#define TMR_COUNT	4		/* �����ʱ���ĸ��� */
SOFT_TMR g_Tmr[TMR_COUNT];

/* ȫ������ʱ�䣬��λ10ms������uIP����� */
__IO int32_t g_RunTime = 0;

/*******************************************************************************
	��������SoftTimerDec
	��  ��: ��ʱ������ָ��,ÿ��1ms��1
	��  ��:
	����˵����
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
	��������SysTick_ISR
	��  ��:
	��  ��:
	����˵����SysTick�жϷ������ÿ��1ms����1��
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

	/* ����Ĵ���ʵ�����а����ļ��
		�������ÿ��10msһ�ξ����ˣ�һ����40ms���˲�����Ϳ�����Ч���˵���е������ɵ�
		����������
	*/
	if (++s_count >= 10)
	{
		s_count = 0;

		g_RunTime++;	/* ȫ������ʱ��ÿ10ms��1 */	
		if (g_RunTime == 0x80000000)
		{
			g_RunTime = 0;
		}
		AdcPro();

		
		
//		KeyPro();		/* �ú����� button.c ��ʵ�� */
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


//        Uart1_PutString(com_buff,10);//�������ݣ������Ƿ�����

        UartHaveData=0;
	//memset(com_buff,0,sizeof(com_buff));

        USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);

     }
}

/*******************************************************************************
	��������DelayMS
	��  ��: �ӳٳ��ȣ���λ1 ms. �ӳپ���Ϊ����1ms
	��  ��:
	����˵������ʱ������ռ�������ʱ��0
*/
void delay_ms(uint32_t n)
{
	/* ���� n = 1 �������������� */
	if (n == 1)
	{
		n = 2;
	}
	g_Tmr[0].count = n;
	g_Tmr[0].flag = 0;

	/* while ѭ���������CPU����IDLE״̬���ѽ��͹��� */
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
	��������StartTimer
	��  ��: ��ʱ��ID (0 - 3)
	��  ��:
	����˵����
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
	��������StartTimer
	��  ��: ��ʱ��ID (0 - 3)
	��  ��: ���� 0 ��ʾ��ʱδ���� 1��ʾ��ʱ��
	����˵����
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
*	��������GetRunTime
*	��  ��: ��
*	��  ��: ��Ը�λ��ʼ��ϵͳ����ʱ�䣬��λ10ms
*	����˵����
*/
int32_t GetRunTime(void)
{
	int runtime; 

	__set_PRIMASK(1);  		/* ���ж� */
	
	runtime = g_RunTime;	/* ������Systick�жϱ���д����˹��жϽ��б��� */
		
	__set_PRIMASK(0);  		/* ���ж� */

	return runtime;
}
