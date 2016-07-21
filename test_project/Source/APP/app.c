#include <includes.h>
#include "app.h"
#include "nrf905.h"
#include "sim900a.h"
#include "com.h"
#include "hmi.h"

#include "main.h"
#include "board_init.h"

//AppTaskStart && AppTaskT_VSample
bool FLAG_warning; //报警标志位

//AppTaskGSM && AppTaskT_VSample && AppTaskUserif
char AbnormalNumber[576] = {0};	//异常的电池编号3*4*48=576
char AN_former[576] = {0};	//前一次异常的电池编号3*4*48=576

//系统
void wtd(void);//看门狗

/* ----------------- APPLICATION GLOBALS ------------------ */
/***********__align(8) 规定任务堆栈8字节对齐，否则printf容易出现问题************/	//jkl:定义数组来作为任务堆栈
__align(8) static OS_STK AppTaskStartStk[APP_TASK_START_STK_SIZE];
__align(8) static OS_STK AppTaskUserIFStk[APP_TASK_USER_IF_STK_SIZE];
__align(8) static OS_STK AppTaskT_VSampleStk[APP_TASK_T_V_SAMPLE_STK_SIZE];
__align(8) static OS_STK AppTaskCom_485Stk[APP_TASK_COM_485_STK_SIZE];
__align(8) static OS_STK AppTaskInsideCom_485Stk[APP_TASK_COM_485_STK_SIZE];
__align(8) static OS_STK AppTaskGSMStk[APP_TASK_GSM_STK_SIZE];
__align(8) static OS_STK AppTaskNrf905Stk[APP_TASK_NRF905_STK_SIZE];
												 
OS_EVENT *TxMbox;
OS_EVENT *InsideTxMbox;//485内部通讯（收敛中心与检测模块）
OS_EVENT *DisMbox;
OS_EVENT *ParaMbox;
OS_EVENT *DataMbox;//检测模块传来的参数数据

OS_EVENT *CalSem;//打开任务AppTaskT_VSample
OS_EVENT *GSMSem;	//GSM任务内循环信号量
OS_EVENT *DetectSem;
OS_EVENT *SendSucceedSem;//短信发送成功信号量
OS_EVENT *Nrf905Sem;	//没插无线模块时一直挂起无线任务，以节约CPU资源。



/*
 *********************************************************************************************************
 *                                      LOCAL FUNCTION PROTOTYPES
 *********************************************************************************************************
 */

static void AppTaskCreate(void);
static void AppTaskStart(void *p_arg);
static void AppTaskUserif(void *pdata);
static void AppTaskT_VSample(void *pdata);
static void AppTaskCom_485(void *p_arg);
static void AppTaskInsideCom_485(void *p_arg);
static void AppTaskGSM(void *p_arg);
static void AppTaskNrf905(void *p_arg);

/*
 *********************************************************************************************************
 *                                                main()
 *
 * Description : This is the standard entry point for C code.  It is assumed that your code will call
 *               main() once you have performed all necessary initialization.
 *
 * Arguments   : none
 *
 * Returns     : none
 *********************************************************************************************************
 */


void MainTask(void);
	 
int main(void)
 {
    INT8U  err;

		
    /* Set the Vector Table base location at 0x08000000 */
    NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x00);
    //BSP_IntDisAll();                   /* Disable all interrupts until we are ready to accept them */
    OSInit();                          /* Initialize "uC/OS-II, The Real-Time Kernel"              */
				
    /* Create the start task */
    OSTaskCreateExt(AppTaskStart, (void *)0, (OS_STK *)&AppTaskStartStk[APP_TASK_START_STK_SIZE - 1], APP_TASK_START_PRIO, APP_TASK_START_PRIO, (OS_STK *)&AppTaskStartStk[0], APP_TASK_START_STK_SIZE, (void *)0, OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);
#if (OS_TASK_NAME_SIZE > 13)
    OSTaskNameSet(APP_TASK_START_PRIO, "Start Task", &err);
#endif
    OSStart();                                                  /* Start multitasking (i.e. give control to uC/OS-II)       */
}

 
/*
 *********************************************************************************************************
 *                                          STARTUP TASK
 *
 * Description : This is an example of a startup task.  As mentioned in the book's text, you MUST
 *               initialize the ticker only once multitasking has started.
 *
 * Arguments   : p_arg   is the argument passed to 'AppTaskStart()' by 'OSTaskCreate()'.
 *
 * Returns     : none
 *
 * Notes       : 1) The first line of code is used to prevent a compiler warning because 'p_arg' is not
 *                  used.  The compiler should not generate any code for this statement.
 *********************************************************************************************************
 */
 
static  void  AppTaskStart(void *p_arg)
{
		(void)p_arg;
		BSP_Init();                                    /* Initialize BSP functions   */
		#if (OS_TASK_STAT_EN > 0)
			OSStatInit();                                   /* Determine CPU                                  */
		#endif
		
		TxMbox = OSMboxCreate((void *)0);							 /*创建串口通讯邮箱*/
		InsideTxMbox = OSMboxCreate((void *)0);							 /*创建串口通讯邮箱*/		
		DisMbox = OSMboxCreate((void *)0);							 /*显示屏的串口通讯邮箱*/
		ParaMbox = OSMboxCreate((void *)0);	
		DataMbox = OSMboxCreate((void *)0);						/*检测模块传来的参数数据*/
		
		CalSem = OSSemCreate(1);
		GSMSem = OSSemCreate(0);
		DetectSem = OSSemCreate(0);
		SendSucceedSem = OSSemCreate(0);
		
		Nrf905Sem = OSSemCreate(0);
		
		AppTaskCreate();                                /* Create application tasks  */
		wtd();	//初始化并启动独立看门狗，喂狗在SysTickHandler里 
		
		printf("\r\n系统启动了！\r\n");
		 
		
		LED_RUN_ON;//运行指示灯。在Exception中关断
		
		
//test
		while(DEF_TRUE) 
		{			
				if(!FLAG_warning) {	//未报警时，运行校准灯指示，有报警时此灯一直亮
					if(KEY_CAL) {
						OSTimeDlyHMSM(0, 0, 0, 700);
						LED_RUN_ON;
						OSTimeDlyHMSM(0, 0, 0, 700);
						LED_RUN_OFF;
					}else {
						LED_RUN_ON;	
						OSTimeDlyHMSM(0, 0, 0, 200);			//切换任务			
					}
				}		
					if(GSM_send) {
						FLAG_warning = TRUE;
					}
					if(FLAG_warning) {
						LED_RUN_ON;		
						//报警灯闪烁,蜂鸣器响
						LED_WARNING_ON;
						OSTimeDlyHMSM(0, 0, 0, 200);
						LED_WARNING_OFF;
						OSTimeDlyHMSM(0, 0, 0, 200);
	
					}
					
					if(KEY_CLEAR == 0) {	//清除和恢复报警
						FLAG_warning = (bool)!FLAG_warning;
						if(FLAG_warning == TRUE) {
							//蜂鸣器响两声恢复报警
							beep(100);
							OSTimeDlyHMSM(0, 0, 0, 100);
							beep(100);
						}
						if(FLAG_warning == FALSE) {
							//蜂鸣器响一声清除报警
							GSM_send = FALSE;
							beep(100);
						}
					}
		}
}

/*
 *********************************************************************************************************
 *                                      CREATE APPLICATION TASKS
 *
 * Description:  This function creates the application tasks.
 *
 * Arguments  :  none
 *
 * Returns    :  none
 *********************************************************************************************************
 */

static  void  AppTaskCreate(void)
{
    INT8U  err;
      OSTaskCreateExt(AppTaskUserif,
    	             (void *)0,
    			     (OS_STK *)&AppTaskUserIFStk[APP_TASK_USER_IF_STK_SIZE-1],
    				 APP_TASK_USER_IF_PRIO,
    				 APP_TASK_USER_IF_PRIO,
    				 (OS_STK *)&AppTaskUserIFStk[0],
    				 APP_TASK_USER_IF_STK_SIZE,
    				 (void *)0,
    				 OS_TASK_OPT_STK_CHK|OS_TASK_OPT_STK_CLR);
#if (OS_TASK_NAME_SIZE > 10)
    OSTaskNameSet(APP_TASK_USER_IF_PRIO, "UserIF", &err);
#endif

    OSTaskCreateExt(AppTaskT_VSample,
                    (void *)0,
                    (OS_STK *)&AppTaskT_VSampleStk[APP_TASK_T_V_SAMPLE_STK_SIZE - 1],
                    APP_TASK_T_V_SAMPLE_PRIO,
                    APP_TASK_T_V_SAMPLE_PRIO,
                    (OS_STK *)&AppTaskT_VSampleStk[0],
                    APP_TASK_T_V_SAMPLE_STK_SIZE,
                    (void *)0,
                    OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);
#if (OS_TASK_NAME_SIZE > 10)
    OSTaskNameSet(APP_TASK_T_V_SAMPLE_PRIO, "T_VSample", &err);
#endif

    OSTaskCreateExt(AppTaskCom_485,
                    (void *)0,
                    (OS_STK *)&AppTaskCom_485Stk[APP_TASK_COM_485_STK_SIZE - 1],
                    APP_TASK_COM_485_PRIO,
                    APP_TASK_COM_485_PRIO,
                    (OS_STK *)&AppTaskCom_485Stk[0],
                    APP_TASK_COM_485_STK_SIZE,
                    (void *)0,
                    OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);
#if (OS_TASK_NAME_SIZE >12)
    OSTaskNameSet(APP_TASK_COM_485_PRIO, "Com_485", &err);
		
#endif

		 OSTaskCreateExt(AppTaskInsideCom_485,
                    (void *)0,
                    (OS_STK *)&AppTaskInsideCom_485Stk[APP_TASK_InsideCOM_485_STK_SIZE - 1],
                    APP_TASK_InsideCOM_485_PRIO,
                    APP_TASK_InsideCOM_485_PRIO,
                    (OS_STK *)&AppTaskInsideCom_485Stk[0],
                    APP_TASK_InsideCOM_485_STK_SIZE,
                    (void *)0,
                    OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);
#if (OS_TASK_NAME_SIZE >12)
    OSTaskNameSet(APP_TASK_InsideCOM_485_PRIO, "InsideCom_485", &err);
		
#endif

      OSTaskCreateExt(AppTaskGSM,
    	             (void *)0,
    			     (OS_STK *)&AppTaskGSMStk[APP_TASK_GSM_STK_SIZE-1],
    				 APP_TASK_GSM_PRIO,
    				 APP_TASK_GSM_PRIO,
    				 (OS_STK *)&AppTaskGSMStk[0],
    				 APP_TASK_GSM_STK_SIZE,
    				 (void *)0,
    				 OS_TASK_OPT_STK_CHK|OS_TASK_OPT_STK_CLR);
#if (OS_TASK_NAME_SIZE > 10)
    OSTaskNameSet(APP_TASK_GSM_PRIO, "GSM", &err);
#endif

      OSTaskCreateExt(AppTaskNrf905,
    	             (void *)0,
    			     (OS_STK *)&AppTaskNrf905Stk[APP_TASK_NRF905_STK_SIZE-1],
    				 APP_TASK_NRF905_PRIO,
    				 APP_TASK_NRF905_PRIO,
    				 (OS_STK *)&AppTaskNrf905Stk[0],
    				 APP_TASK_NRF905_STK_SIZE,
    				 (void *)0,
    				 OS_TASK_OPT_STK_CHK|OS_TASK_OPT_STK_CLR);
#if (OS_TASK_NAME_SIZE > 10)
    OSTaskNameSet(APP_TASK_NRF905_PRIO, "Nrf905", &err);
#endif
}

//串口屏任务
uint16_t VirtAddVarTab[NumbOfVar] = {0};//EEPROM文件所用/*Virtual address define by the user: 0XFFFF value is prohibited */
static  void	AppTaskUserif(void *p_arg)
{
		INT8U err;
    unsigned char *ComBuffer;
		
		
//	 /*初始化数据备份区*/
//    /* Enable PWR and BKP clock */
//    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
//    /* Enable write access to Backup domain */
//    PWR_BackupAccessCmd(ENABLE);
//    /* Clear Tamper pin Event(TE) pending flag */
//    BKP_ClearFlag();
//    /* Unlock the Flash Program Erase controller */
//    FLASH_Unlock();
//    /* EEPROM Init */
//    EE_Init();


    while(DEF_TRUE) {

				ComBuffer = (unsigned char *)OSMboxPend(DisMbox, 0, &err);
        if(err == OS_ERR_NONE) {
            //DataBuffer = (unsigned char *)OSMboxPend(DataMbox,2,&err);
						Analysis(ComBuffer);
        }
        /*通讯超时*/
        else if(err == OS_ERR_TIMEOUT) {
        }
				
				/*参数信息输出*/
				if(FLAG_Setif) //设置界面时允许向下位机发送数据，若其它界面时也允许屏上容易出现乱码
				{ 
						Param_IF();//参数界面：电池电压、数量、电话等
				}
				FLAG_Setif =FALSE;
		}
}

//外部通信任务（收敛中心与上位机）
static  void  AppTaskCom_485(void *p_arg)
{
		unsigned char *ComBuffer;
		
    INT8U err;	

		R_485_EN1;//打开远程rs485接收。
		
    while(DEF_TRUE) {

				/*待添加标志事件以实现超时功能*/
        ComBuffer = (unsigned char *)OSMboxPend(TxMbox, 0, &err);
							
        if(err == OS_ERR_NONE) 
				{
						com(ComBuffer);
        }

        /*通讯超时*/
        else if(err == OS_ERR_TIMEOUT) {
        }
    }
}

//内部通讯任务（收敛中心与检测模块）
static  void  AppTaskInsideCom_485(void *p_arg)
{
		unsigned char *ComBuffer;
		
    INT8U err;	
		int i =0;
		uint8_t TempBuffer[12];
		R_485_EN2;//打开近程rs485接收。
		
    while(DEF_TRUE) {

				/*待添加标志事件以实现超时功能*/
        ComBuffer = (unsigned char *)OSMboxPend(InsideTxMbox, 0, &err);
							
        if(err == OS_ERR_NONE) 
				{
					
					/*通讯协议解析 协议格式
					 帧头（8bit）+从机地址（8bit）+指令代码（8bit）+数据长度（16bit）
					+数据内容+CRC16（8bit）+帧尾（8bit）*/

					for (i=0; i<12; i++)
					{
						TempBuffer[i] = ComBuffer[i+5];
					}
					OSMboxPost(DataMbox, (void *)TempBuffer); //将数据转给AppTaskT_VSample

        }

        /*通讯超时*/
        else if(err == OS_ERR_TIMEOUT) {
        }
    }
}

//射频905通信任务
bool FlAG_Nrf905_Send = FALSE;//nrf905发送标志位
extern bool FLAG_Nrf905_Used;	//标记无线模块有没有插着
uint8_t COM_SEND[6]={0xEE,0x01,0x07,0x00,0x00,0xED};//Channel_Select中改变此数组内容。
static void AppTaskNrf905(void *pdata)
{
		INT8U err;
		n95_Init_IO();										// 初始化nRF905引脚状态
		n95_Init_Dev();										// 初始化nRF905,并将其转换为接收状态
		
		Nrf905_Test();	//发一条信息，检测DR引脚电平会不会置高。由此来判断无线模块有没有插着。没有的话则一直挂起无线任务（即此任务），以节约CPU资源。
	  while (DEF_TRUE)
   {
				OSSemPend(Nrf905Sem, 0, &err);	//校准时关闭采集
				
				if(n95_Check_DR(n95_RX_Buff)){					// nrf905发送接收
										
						OSMboxPost(DataMbox, (void *)n95_RX_Buff); //将数据转给AppTaskT_VSample

				}
				
				if(FlAG_Nrf905_Send){					// nrf905发送开
										
						n95_Sendout(COM_SEND);		// nrf905无线发送		
						
						FlAG_Nrf905_Send = FALSE; // nrf905发送关

				}
				OSTimeDlyHMSM(0,0,0,10);
				
				OSSemPost(Nrf905Sem);	//进来之后能继续循环。

	 }

}


//GSM任务
static void AppTaskGSM(void *pdata)
{
    uint8_t err;
		
    while(DEF_TRUE) {
			
			OSSemPend(GSMSem, 0, &err);
			sim900a();
    }
} 

//testZigbee

//参数采集任务
static void ParaInit(uint16_t ChannelSlope[4][36]);
float ParaAll[4][48] = {0.0};	//48个通道，每个4个参数
void Channel_Select(uint8_t CHANNEL_NUM);//通道选择，即选择指定的电池进行操作（如校准，获取电压等）
uint16_t  ParaCal(uint8_t p, uint8_t NumChannel);	//参数校准函数
uint16_t r_Cal_C = 2800;
float V_value[36] = {0.0};
float T_value[36] = {0.0};
float r_value[36] = {0.0};	
float C_value[36] = {0.0};	
static void AppTaskT_VSample(void *pdata)
{

		INT8U err;

		uint32_t temp;
		uint16_t bettery_num;	//电池编号

		uint8_t Screen_id = 0;
		uint8_t	resend_time = 0;//重发次数
	  uint8_t n = 0;0.
		unsigned char *ComBuffer;
		uint16_t ChannelSlope[4][36] = {0}; //电压、温度、内阻和容量的校准参数，36通道

    ParaInit(ChannelSlope);//参数校准
#if 1		
  	FSmain();	
 
#endif
	
#if 0
    while(DEF_TRUE) 
		{
				n++;
			  if(n>10){ //没检查100次电压温度，检测1次内阻
					Channel_Select(0);//所有检测器开始测试
					OSTimeDlyHMSM(0,0,2,0);	//等待检测器检测完毕
					n = 0;
				}
				
				for(bettery_num = 1; bettery_num < 5; bettery_num++) //36块电池轮询
				{
						OSSemPend(CalSem, 0, &err);	//校准时关闭采集
						printf("num= %d  ",bettery_num);
RESEND:						
				//查询电池参数
						Channel_Select(bettery_num);
						
						
				//接收数据,若无数据则推上一次接收到的数据
						ComBuffer = (unsigned char *)OSMboxPend(DataMbox, 1000, &err);	
						
						if(err == OS_ERR_NONE) 
						{
								temp = ( ((uint32_t)ComBuffer[3])<<24 ) + ( ((uint32_t)ComBuffer[2])<<16 ) \
										+ ( ((uint32_t)ComBuffer[1])<<8 ) + (uint32_t)ComBuffer[0];
								V_value[bettery_num]=((float)temp)/1000;
																	
								temp = ( ((uint32_t)ComBuffer[7])<<24 ) + ( ((uint32_t)ComBuffer[6])<<16 ) \
										+ ( ((uint32_t)ComBuffer[5])<<8 ) + (uint32_t)ComBuffer[4];	
								T_value[bettery_num]=((float)temp)/1000;
																
								temp = ( ((uint32_t)ComBuffer[11])<<24 ) + ( ((uint32_t)ComBuffer[10])<<16 ) \
										+ ( ((uint32_t)ComBuffer[9])<<8 ) + (uint32_t)ComBuffer[8];
								r_value[bettery_num]=((float)temp)/1000;
																
								C_value[bettery_num]= 180 - 80 * r_value[bettery_num] *1000/ r_Cal_C;
									
						}
						//通讯超时，跳过此电池，进行下面的
						else if(err == OS_ERR_TIMEOUT) 
						{			
								printf("time out\r\n");
								resend_time++;
								if(resend_time < 5){
									goto	RESEND;			//检测器无响应，重新发送命令		
								}else{
									printf("检测器%d无响应！\r\n",bettery_num);
								}
								
//								OSSemPost(CalSem);  
//								continue;
						}else
						{
							printf("\r\nerr  =  %d\r\n",err);
						}
						
						resend_time = 0;//重发次数清零

				//参数校准
						//电压校准
						ParaAll[0][bettery_num-1] = V_value[bettery_num] * ChannelSlope[0][bettery_num-1] * 0.001;
						printf("V_value = ,%f,",ParaAll[0][bettery_num-1]);
						if((ParaAll[0][bettery_num-1] > 100) || (ParaAll[0][bettery_num-1] < 0)) 
						{
							ParaAll[0][bettery_num-1] = 0;							
						}
						
						
						//温度校准
						ParaAll[1][bettery_num-1] = T_value[bettery_num] * ChannelSlope[1][bettery_num-1] * 0.001;
						printf("T_value = ,%f,",ParaAll[1][bettery_num-1]);	
						if((ParaAll[1][bettery_num-1] > 100) || (ParaAll[1][bettery_num-1] < 0))
						{
							ParaAll[1][bettery_num-1] = 0;							
						}
						
						//内阻校准
						ParaAll[2][bettery_num-1] = r_value[bettery_num] * ChannelSlope[2][bettery_num-1] * 0.001;
						printf("r_value = ,%f,",ParaAll[2][bettery_num-1]);
						if((ParaAll[2][bettery_num-1] > 100) || (ParaAll[2][bettery_num-1] < 0))
						{
							ParaAll[2][bettery_num-1] = 0;							
						}
						
						//容量校准
						ParaAll[3][bettery_num-1] = 180 - 80 *ParaAll[2][bettery_num-1] *1000/ r_Cal_C * ChannelSlope[3][bettery_num-1] * 0.001;				
						printf("C_value = ,%f\r\n",ParaAll[3][bettery_num-1]);
						if(ParaAll[2][bettery_num-1]<0.5)
						{
								ParaAll[3][bettery_num-1] = 0;
						}
						else if(ParaAll[3][bettery_num-1] > 99.9) //容量不能超过99.9%
								{	
										ParaAll[3][bettery_num-1] = 99.9;
								}
								else if(ParaAll[3][bettery_num-1] < 0) 	//容量不能低于0
										{
												ParaAll[3][bettery_num-1] = 0;
										}
						
						OSSemPost(CalSem);  
				//推出显示
						if(3 ==((bettery_num-1)%4) )	//每测完四块电池送往显示屏一次，即整页显示
						{
							Screen_id = bettery_num/4-1;
							
							printf("Screen_id = %d",Screen_id);
							Show(Screen_id);
						}
					
				}
				

				//异常报警
				if (strcmp(AbnormalNumber,AN_former) != 0) //若本次异常数据和上次异常数据不同，刷新异常数据并发短信通知客户
				{
					FLAG_warning = TRUE;//报警标志控制报警指示灯。
					Abnormal_show();	//串口屏跳转到异常电池界面，并显示异常电池参数信息
					Send_Message(1);		//发短信通知运维人员电池异常信息。
					OSSemPend(SendSucceedSem, 3000, &err);	//等待号码1短信发送成功。3s超时跳过
					if(err == OS_ERR_NONE) 
					{
						printf("\r\nSendSucceedSem1 received!\r\n");
					}	
					else if(err == OS_ERR_TIMEOUT) 
					{
							OSSemPend(GSMSem, 0, &err);	//停止GSM任务内的循环
							printf("\r\nOS_ERR_TIMEOUT,Stop the GSM1 sending.\r\n");	
					}
					
					Send_Message(2);		//发短信给号码2
					OSSemPend(SendSucceedSem, 3000, &err);	//等待号码2短信发送成功。3s超时跳过
					if(err == OS_ERR_NONE) 
					{
						printf("\r\nSendSucceedSem2 received!\r\n");
					}	
					else if(err == OS_ERR_TIMEOUT) 
					{
							OSSemPend(GSMSem, 0, &err);	//停止GSM任务内的循环
							printf("\r\nOS_ERR_TIMEOUT,Stop the GSM2 sending.\r\n");	
					}
					
					Send_Message(3);		//发短信给号码3
					OSSemPend(SendSucceedSem, 3000, &err);	//等待号码3短信发送成功。3s超时跳过
					if(err == OS_ERR_NONE) 
					{
						printf("\r\nSendSucceedSem3 received!\r\n");
					}	
					else if(err == OS_ERR_TIMEOUT) 
					{
							OSSemPend(GSMSem, 0, &err);	//停止GSM任务内的循环
							printf("\r\nOS_ERR_TIMEOUT,Stop the GSM3 sending.\r\n");	
					}
				}
				memset(AbnormalNumber, 0, sizeof(AbnormalNumber));	//清空AbnormalNumber避免下次发错和内存溢出
			
		}

#endif
}
/*************************************************
Function: Channel_Select
Description: 选择通道，
Input:
Return:
Others:
 *************************************************/
void Channel_Select(uint8_t CHANNEL_NUM)
{
		uint8_t temp[6]={0xEE,0x01,0x07,0x00,0x00,0xED};	//此数字定义成全局数组有问题，接收不到模块的数据？
		INT8U i = 0;
		
		if(CHANNEL_NUM == 0)
		{
				temp[2] = 0x03;//所有检测模块开始采集指令：EE 00 03 00 00 ED
		}
    if(CHANNEL_NUM > 48) 
		{
        /*无效的通道号*/
        //printf("invalid channel num!!\r\n");
    } 
		else 
		{
				temp[1] = CHANNEL_NUM;
				
				T_485_EN2;
				r485_Sendout(temp);		// rs485有线发送
				R_485_EN2;
				

				if(FLAG_Nrf905_Used)	//没有用无线模块就不执行内部代码，节约CPU资源。
				{
					for(i=1; i<3; i++)
					{
						COM_SEND[i] = temp[i];
					}
					FlAG_Nrf905_Send = TRUE;				
				}
       
    }
}

/*************************************************
Function: ParaCal
Description: 校准参数，结果为15位AD码值
Input: p---校准参数的代码 0 电压 1 温度 2内阻Rv 3内阻Ri
NumChannel---需要校准的通道号
Return: 校准后的数值
Others:
 *************************************************/
uint16_t ParaCal(uint8_t p, uint8_t NumChannel)
{
    uint8_t i = 0;
    uint8_t j = 0;
    uint32_t SUM0 = 0;
    uint32_t SUM1 = 0;
		uint32_t SUM2 = 0;
    uint32_t SUM3 = 0;
    uint32_t Avg_V = 0;
    uint32_t Avg_T = 0;
		uint32_t Avg_Rv = 0;
		uint32_t Avg_Ri = 0;

//    	 uint16_t Database = 0;
//    	 uint8_t  Overcounter = 0;
    /*调用3次滤波函数，更新基准值*/
    //	 DataFilter(&Database,ADC1ConvertedValue+NumChannel,3,&Overcounter);
    //	 DataFilter(&Database,ADC1ConvertedValue+NumChannel,3,&Overcounter);
    //	 DataFilter(&Database,ADC1ConvertedValue+NumChannel,3,&Overcounter);
    //printf("*******************%d*****\r\n",NumChannel);

    while(1) {
        OSTimeDlyHMSM(0, 0, 0, 1);
        //SUM0 += DataFilter(&Database,ADC1ConvertedValue+NumChannel,3,&Overcounter);
				switch(p) {
					case 0:	//温度
//						SUM0 += ADC1ConvertedValue[0];
						i++;
						if(i >= 64) {
								i = 0;
								Avg_T  += (uint16_t)(SUM0 >> 3);
								SUM0 = 0;
								j++;
								if(j >= 10) {
									return (uint16_t)(Avg_T / 10);
								}
						}
						break;
						
					case 1: //电压	
//						SUM1 += ADC1ConvertedValue[1];
						i++;
						if(i >= 64) {
								i = 0;
								Avg_V  += (uint16_t)(SUM1 >> 3);
								SUM1 = 0;
								j++;
								if(j >= 10) {								
									return (uint16_t)(Avg_V / 10);
										}
						}
						break;
						
					case 2: //内阻Rv
//						SUM2 += ADC1ConvertedValue[2];
						i++;
						if(i >= 64) {
								i = 0;
								Avg_Rv  += (uint16_t)(SUM2 >> 3);
								SUM2 = 0;
								j++;
								if(j >= 10) {								
									return (uint16_t)(Avg_Rv/10);
										}
						}
						break;
				
					case 3: //内阻Ri
//						SUM3 += ADC1ConvertedValue[3];
						i++;
						if(i >= 64) {
								i = 0;
								Avg_Ri  += (uint16_t)(SUM3 >> 3);
								SUM3 = 0;
								j++;
								if(j >= 10) {								
									return (uint16_t)(Avg_Ri/10);
										}
						}

						break;
					default:
						break;
			}
    }
}


static void  ParaInit(uint16_t ChannelSlope[4][36])
{
    uint16_t i = 0;
		uint16_t temp[2];
    // EE_WriteVariable(Channel1_V_FULL_Addr+k,k*100+200);
    /*参数初始化 48通道*/
    for(i = 0; i < 36; i++) {
        /*电压*/
//        EE_ReadVariable(Channel1_V_FULL_Addr + i, ChannelSlope[0] + 2 * i);
//        EE_ReadVariable(Channel1_V_ZERO_Addr + i, ChannelSlope[0] + 2 * i + 1);

//        /*如果未保存校准参数，调用默认值*/
//        if(ChannelSlope[0][2 * i] == 0X00) {
            ChannelSlope[0][i] = 1000;
//        }

        /*温度*/
//        EE_ReadVariable(Channel1_T_FULL_Addr + i, ChannelSlope[1] + 2 * i);
//        EE_ReadVariable(Channel1_T_ZERO_Addr + i, ChannelSlope[1] + 2 * i + 1);

//        if(ChannelSlope[1][2 * i] == 0X00) {
            ChannelSlope[1][i] = 1000;
//        }
				
				/*内阻*/
        EE_ReadVariable(Channel1_r_k_Addr + i, ChannelSlope[2] + i);

        if(ChannelSlope[2][i] == 0X00) {
            ChannelSlope[2][i] = 1000;
        }
			
				/*容量*/
        EE_ReadVariable(R_Cal_C_Addr + i, temp);
				r_Cal_C = temp[0];

//        if(ChannelSlope[3][i] == 0X00) {
            ChannelSlope[3][i] = 1000;
//        }
			
    }

}



void wtd(void)
{
	if (RCC_GetFlagStatus(RCC_FLAG_IWDGRST) != RESET)
	
		RCC_ClearFlag();
	
	
	
	/* NVIC configuration */
//	NVIC_Configuration();
	
	/* WWDG configuration */
	/* Enable WWDG clock */
//	RCC_APB1PeriphClockCmd(RCC_APB1Periph_WWDG, ENABLE);
	
	  IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
	
	  /* IWDG counter clock: 40KHz(LSI) / 32 = 1.25 KHz */
	  IWDG_SetPrescaler(IWDG_Prescaler_64);
	
	  /* Set counter reload value to 349 */
	  IWDG_SetReload(0xfff);
	
	  /* Reload IWDG counter */
	  IWDG_ReloadCounter();
	
	  /* Enable IWDG (the LSI oscillator will be enabled by hardware) */
	  IWDG_Enable();
}
