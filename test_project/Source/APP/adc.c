#include "main.h"


extern struct configure *conf;


uint16_t g_AdcValue;

void AdcPro(void)
{
	static uint16_t buf[SAMP_COUNT];
	static uint8_t write;
	uint32_t sum;
	uint8_t i;

	buf[write] = ADC_GetConversionValue(ADC1);

	if (++write >= SAMP_COUNT)
		write = 0;
	
	sum = 0;
	for (i = 0; i < SAMP_COUNT; i++){

		sum += buf[i];
	}

	g_AdcValue = sum / SAMP_COUNT;	/* ADC����ֵ�����ɴβ���ֵƽ�� */

	ADC_SoftwareStartConvCmd(ADC1, ENABLE);	/* ��������´�ADCת�� */
}

/*ADC����*/
uint16_t GetADC(void)
{
	uint16_t ret;

	/* �����ٽ����������ر��ж� */
	__set_PRIMASK(1);  /* ���ж� */

	ret = g_AdcValue;

	__set_PRIMASK(0);  /* ���ж� */

	return ret;
}



double ADC_SAMPLING(void)
{
	uint16_t value;
	
	double tmp;

	if (RTC_GetCounter() == 0x00015180){

		RTC_SetCounter(0x0);
		RTC_WaitForLastTask();	
	}
									

	CPU_IDLE();
	Time_Display(RTC_GetCounter());

	value = GetADC();
	
		if(value < 2047)
			tmp = (value * 26 - 2691);
		else if(value < 2400)
			tmp = (value * 26 - 2430);
		else 
			tmp = (value * 26 - 2510);
		
	
	if(tmp <= 0)
		tmp = 0;

	printf("��ѹ:%.2fV\r",tmp / 1000);
	

	return (tmp / 1000);
	
}
