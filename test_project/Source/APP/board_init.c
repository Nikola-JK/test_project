
#include "main.h"
#include "board_init.h"

u32 Month_Days_Accu_C[13] = {0,31,59,90,120,151,181,212,243,273,304,334,365};
u32 Month_Days_Accu_L[13] = {0,31,60,91,121,152,182,213,244,274,305,335,366};


s32 Year_Secs_Accu[5]={0,31622400,63158400,94694400,126230400};

s32 Month_Secs_Accu_C[13] = { 0,2678400,5097600,7776000,10368000,13046400,15638400,18316800,
				20995200,23587200,26265600,28857600,31536000};

s32 Month_Secs_Accu_L[13] = {0,2678400,5184000,7862400,  10454400,13132800,15724800,18403200,
				21081600,23673600,26352000,28944000,31622400};

 void DispMenu(void)
{
	printf("\r\n*******************************************\r\n");
	printf("\n\r*******铅蓄电池自动充放电测试**************\r\n");


	
}

 void InitBoard(void)
{
	GPIO_Configuration();
	bsp_InitLed();
	USART_Configuration();
	SysTick_Config(SystemFrequency / 1000);
	
}

void RTC_Check(void)
{
	if (BKP_ReadBackupRegister(BKP_DR1) != 0xA5A5){

		printf("\r\n\n未设置时间");
			
		RTC_Configuration();
		
		Time_Adjust();
		
		BKP_WriteBackupRegister(BKP_DR1, 0xA5A5);

  	}else{
	 
		if (RCC_GetFlagStatus(RCC_FLAG_PORRST) != RESET){}
		
		else if (RCC_GetFlagStatus(RCC_FLAG_PINRST) != RESET){}
		
		RTC_WaitForSynchro();
		
		RTC_ITConfig(RTC_IT_SEC, ENABLE);
		
		RTC_WaitForLastTask();
	
  	}

}
				     

/*******************************************************************************
	函数名：GPIO_Configuration

	LED口线分配：
	系统运行状态LED1 : 	PG10 (输出0点亮)
	充电LED2 : 		PG9  (输出0点亮)
	放电LED3 : 		PC12 (输出0点亮)
	静置LED4 : 		PC11 (输出0点亮)
	USB设备LED5:		PC10 (输出0点亮)

*/

void GPIO_Configuration(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	/* 第1步：打开GPIOA GPIOB GPIOC GPIOD GPIOE的时钟
	  	  一次性全打开
	*/
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC
				| RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOE,ENABLE);
	
	bsp_InitLed();		//LED初始化
	
	
	/* 第2步：配置按键为浮动输入模式GPIOB(实际上CPUf复位后就是输入状态) 按键COPY  CLEAR 设置为输入 */
	GPIO_InitStructure.GPIO_Pin = GPIO_PIN_KEY1;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIO_PORT_KEY1, &GPIO_InitStructure);	/* PB8 PB9*/
	
	GPIO_InitStructure.GPIO_Pin = GPIO_PIN_KEY2;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIO_PORT_KEY2, &GPIO_InitStructure);	/* PB8 PB9*/
	
	//初始化beeper
	GPIO_InitStructure.GPIO_Pin = GPIO_PIN_BEEPER;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
	GPIO_Init(GPIO_PORT_BEEPER, &GPIO_InitStructure);
	
} 

uint32_t Time_Regulate(void)
{

	int16_t Tmp_Year, Tmp_Month, Tmp_Date, Tmp_HH =100, Tmp_MM =100, Tmp_SS =100;//否则初始化为0.则进不了时分秒设置

	u32 LeapY, ComY, TotSeconds, TotDays;
	
	printf("\r\n==============设置时间=======================");


	printf("\r\n  设置 年（0 - 2200）：");

	while(Tmp_Year <= 0 || Tmp_Year > 2200){

		Tmp_Year = USART_Scanf(2200);
	}
	printf(" %d", Tmp_Year);

	printf("\r\n  设置 月（1 - 12）：");  
	while(Tmp_Month <= 0 || Tmp_Month > 12){
	
		Tmp_Month = USART_Scanf(12);
	}
	printf(" %d", Tmp_Month);


	printf("\r\n  设置 日（1 - 31）：");
	while(Tmp_Date <= 0 || Tmp_Date > 31){
	
		Tmp_Date = USART_Scanf(31);
	}
	printf("%d", Tmp_Date);

	
	printf("\r\n  设置 小时（0 - 23）："); 
	while(Tmp_HH < 0 || Tmp_HH > 23){
	
		Tmp_HH = USART_Scanf(23);
	}
	printf("%d", Tmp_HH);
	

	printf("\r\n  设置 分钟（0 - 59）：");
	while(Tmp_MM < 0 || Tmp_MM > 59){
	
		Tmp_MM = USART_Scanf(59);
	}
	printf(" %d", Tmp_MM);

	
	printf("\r\n  设置 秒数（0 - 59）：");
	while(Tmp_SS < 0 || Tmp_SS > 59){
	
		Tmp_SS = USART_Scanf(59);
	}
	printf(" %d", Tmp_SS);
  

  
	/* change Year-Month-Data-Hour-Minute-Seconds into X(Second) to set RTC->CNTR */
	if(Tmp_Year==2000)
		LeapY = 0;
	else
		LeapY = (Tmp_Year - 2000 -1)/4 +1;
	
	ComY = (Tmp_Year - 2000)-(LeapY);
	
	if (Tmp_Year%4)
		//common year
		TotDays = LeapY*366 + ComY*365 + Month_Days_Accu_C[Tmp_Month-1] + (Tmp_Date-1); 
	else
		//leap year
		TotDays = LeapY*366 + ComY*365 + Month_Days_Accu_L[Tmp_Month-1] + (Tmp_Date-1); 
	
	TotSeconds = TotDays*SecsPerDay + (Tmp_HH*3600 + Tmp_MM*60 + Tmp_SS);
  

	
	/* Return the value to store in RTC counter register */
	
	return TotSeconds;
}

uint32_t tosecond(int16_t year,int16_t month,int16_t date,int16_t hour,int16_t min,int16_t sec)
{
	u32 LeapY, ComY, TotSeconds, TotDays;

       if(year==2000)
		LeapY = 0;
	else
		LeapY = (year - 2000 -1)/4 +1;
	
	ComY = (year - 2000)-(LeapY);
	
	if (year%4)
		//common year
		TotDays = LeapY*366 + ComY*365 + Month_Days_Accu_C[month-1] + (date-1); 
	else
		//leap year
		TotDays = LeapY*366 + ComY*365 + Month_Days_Accu_L[month-1] + (date-1); 
	
	TotSeconds = TotDays*SecsPerDay + (hour*3600 + min*60 + sec);

       return  TotSeconds;

}

uint32_t USART_Scanf(uint32_t value)
{
	
	u32 index = 0;
	u32 tmp[5];
	u32 Num;
	
	if(value >= 10000)
		Num = 5;  
	else if (value >= 1000)
		Num = 4;
	else if(value >= 100) 
		Num = 3;
	else if(value >= 10) 
		Num = 2;
	else
		Num = 1; 
	while(index < Num){
	
		/* Loop until RXNE = 1 */
		while(USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == RESET){}

		tmp[index++] = (USART_ReceiveData(USART1));

		if((tmp[index - 1] < 0x30) || (tmp[index - 1] > 0x39)){

			printf("\n\r************数值范围 0 -- %d，请重新输入：",value);
			index--;
		}
	}
	/* Calculate the Corresponding value */
	if(value >= 10000)
		index = ((tmp[0] - 0x30) * 10000) + ((tmp[1] - 0x30) * 1000) + ((tmp[2] - 0x30) * 100) + ((tmp[3] - 0x30)*10) + (tmp[4] - 0x30);
	else if(value >= 1000)
		index = ((tmp[0] - 0x30) * 1000) + ((tmp[1] - 0x30) * 100) + ((tmp[2] - 0x30) * 10) + (tmp[3] - 0x30); 
	else if(value >= 100)
		index = ((tmp[0] - 0x30) * 100) + ((tmp[1] - 0x30) * 10) + (tmp[2] - 0x30);
	else if(value >= 10)
		index = ((tmp[0] - 0x30) * 10) + (tmp[1] - 0x30);
	else
		index =  tmp[0] - 0x30;
	/* Checks */
	if(index > value){
	
		printf("\n\r***数值范围 0 -- %d，请重新输入：", value);
		return 0xFF;
	}
	return index;
}

//void NVIC_Configuration(void)
//{
//  NVIC_InitTypeDef NVIC_InitStructure;
//
//  /* 2 bits for Preemption Priority and 2 bits for Sub Priority */
//  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
//
//  NVIC_InitStructure.NVIC_IRQChannel = WWDG_IRQn;
//  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
//  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
//  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
//
//  NVIC_Init(&NVIC_InitStructure);
//}




void NVIC_Configuration(void)
{
NVIC_InitTypeDef NVIC_InitStructure;

#ifdef  VECT_TAB_RAM  
  /* Set the Vector Table base location at 0x20000000 */ 
  NVIC_SetVectorTable(NVIC_VectTab_RAM, 0x0); 
#else  /* VECT_TAB_FLASH  */ 
  /* Set the Vector Table base location at 0x08000000 */ 
  NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0);   
#endif

/* Configure the NVIC Preemption Priority Bits */  
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0); 
/* Enable the USART1 Interrupt */ 
  NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn; 
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0; 
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; 
  NVIC_Init(&NVIC_InitStructure);
 }




void RTC_Configuration(void)
{
	/* Enable PWR and BKP clocks */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
	
	/* Allow access to BKP Domain */
	PWR_BackupAccessCmd(ENABLE);
	
	/* Reset Backup Domain */
	BKP_DeInit();
	
	/* Enable LSE */
	RCC_LSEConfig(RCC_LSE_ON);
	/* Wait till LSE is ready */
	while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET)
	{}
	
	/* Select LSE as RTC Clock Source */
	RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
	
	/* Enable RTC Clock */
	RCC_RTCCLKCmd(ENABLE);
	
	/* Wait for RTC registers synchronization */
	RTC_WaitForSynchro();
	
	/* Wait until last write operation on RTC registers has finished */
	RTC_WaitForLastTask();
	
	/* Enable the RTC Second */
	RTC_ITConfig(RTC_IT_SEC, ENABLE);
	
	/* Wait until last write operation on RTC registers has finished */
	RTC_WaitForLastTask();
	
	/* Set RTC prescaler: set RTC period to 1sec */
	RTC_SetPrescaler(32767); /* RTC period = RTCCLK/RTC_PR = (32.768 KHz)/(32767+1) */
	
	/* Wait until last write operation on RTC registers has finished */
	RTC_WaitForLastTask();
}


void Time_Adjust(void)
{
	/* Wait until last write operation on RTC registers has finished */
	RTC_WaitForLastTask();
	/* Change the current time */
	RTC_SetCounter(Time_Regulate());
	/* Wait until last write operation on RTC registers has finished */
	RTC_WaitForLastTask();
}

void Time_Display(uint32_t TimeVar)
{	
	u32 TY = 0, TM = 1, TD = 0;
	s32 Num4Y,NumY, OffSec, Off4Y = 0;
	u32 i;
	//s32 NumDay, OffDay;
	s32 NumDay;
	
	u32 THH = 0, TMM = 0, TSS = 0;
	
	
	{
		Num4Y = TimeVar/SecsPerFourYear;
		OffSec = TimeVar%SecsPerFourYear;
		
		i=1;
		while(OffSec > Year_Secs_Accu[i++])
			Off4Y++;
		
		/* Numer of Complete Year */
		NumY = Num4Y*4 + Off4Y;
		/* 2000,2001,...~2000+NumY-1 complete year before, so this year is 2000+NumY*/
		TY = 2000+NumY;
		
		OffSec = OffSec - Year_Secs_Accu[i-2];
		
		/* Month (TBD with OffSec)*/
		i=0;
		if(TY%4){	
			// common year
			while(OffSec > Month_Secs_Accu_C[i++]);
			TM = i-1;
			OffSec = OffSec - Month_Secs_Accu_C[i-2];
		}else{
		
			// leap year
			while(OffSec > Month_Secs_Accu_L[i++]);
			TM = i-1;
			OffSec = OffSec - Month_Secs_Accu_L[i-2];
		}
		
		/* Date (TBD with OffSec) */
		NumDay = OffSec/SecsPerDay;
		OffSec = OffSec%SecsPerDay;
		TD = NumDay+1;
		
		/* Compute  hours */
		THH = OffSec/3600;
		/* Compute minutes */
		TMM = (OffSec % 3600)/60;
		/* Compute seconds */
		TSS = (OffSec % 3600)% 60;
	}

//		{
//			static uint8_t pos = 0;
//	
//			if (pos == 0)
//				printf("|");
//			else if (pos == 1)
//				printf("/");
//			else if (pos == 2)
//				printf("-");
//			else if (pos == 3)
//				printf("\\");	
//	
//			if (++pos >= 4)
//				pos = 0;
//
//
//		}
		
	
	 printf("时间:%0.2d-%0.2d-%0.2d %0.2d:%0.2d:%0.2d				",TY, TM, TD,THH, TMM, TSS);

	//保存当前时间

	sprintf(t_now->year, "%d", TY);
	sprintf(t_now->month, "%d", TM);
	sprintf(t_now->day, "%d", TD);
	sprintf(t_now->hour, "%d", THH);
	sprintf(t_now->min, "%d", TMM);
	sprintf(t_now->sec, "%d", TSS);

}


//int Ymd2Wday(uint32_t year, uint32_t month, uint32_t days) //年月日 to 星期
//{ 
//	static int mdays[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30 }; 
//
//	int i, y = year - 1; 
//	for (i=0; i<month; ++i) days += mdays[i]; 
//
//	if (month>2) { 
//		
//		// Increment date if this is a leap year after February 
//
//		if (((year%400)==0) || ((year&3)==0 && (year%100))) 
//			++days; 
//	} 
//	return (y+y/4-y/100+y/400+days)%7; 
//} 
//


void ADC_Configuration(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	ADC_InitTypeDef ADC_InitStructure;
	__IO uint16_t ADCConvertedValue;

    /* 使能 ADC1 and GPIOC clock  */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 | RCC_APB2Periph_GPIOC, ENABLE);

	/* 配置PC-为模拟输入(ADC Channel14) */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	/* 配置ADC1, 不用DMA, 用软件自己触发 */
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
	ADC_InitStructure.ADC_ScanConvMode = ENABLE;
	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_NbrOfChannel = 1;
	ADC_Init(ADC1, &ADC_InitStructure);

	/* ADC1 regular channel0 configuration */
	ADC_RegularChannelConfig(ADC1, ADC_Channel_13, 1, ADC_SampleTime_55Cycles5);

	/* Enable ADC1 DMA */
	ADC_DMACmd(ADC1, ENABLE);

	/* Enable ADC1 */
	ADC_Cmd(ADC1, ENABLE);

	/* Enable ADC1 reset calibaration register */
	ADC_ResetCalibration(ADC1);
	/* Check the end of ADC1 reset calibration register */
	while(ADC_GetResetCalibrationStatus(ADC1));

	/* Start ADC1 calibaration */
	ADC_StartCalibration(ADC1);
	/* Check the end of ADC1 calibration */
	while(ADC_GetCalibrationStatus(ADC1));

	/* Start ADC1 Software Conversion */
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);
}

