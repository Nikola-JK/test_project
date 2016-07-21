#include <includes.h>
#include "app.h"
#include "nrf905.h"
#include "sim900a.h"
#include "com.h"
#include "hmi.h"

#include "main.h"
#include "board_init.h"

//AppTaskStart && AppTaskT_VSample
bool FLAG_warning; //������־λ

//AppTaskGSM && AppTaskT_VSample && AppTaskUserif
char AbnormalNumber[576] = {0};	//�쳣�ĵ�ر��3*4*48=576
char AN_former[576] = {0};	//ǰһ���쳣�ĵ�ر��3*4*48=576

//ϵͳ
void wtd(void);//���Ź�

/* ----------------- APPLICATION GLOBALS ------------------ */
/***********__align(8) �涨�����ջ8�ֽڶ��룬����printf���׳�������************/	//jkl:������������Ϊ�����ջ
__align(8) static OS_STK AppTaskStartStk[APP_TASK_START_STK_SIZE];
__align(8) static OS_STK AppTaskUserIFStk[APP_TASK_USER_IF_STK_SIZE];
__align(8) static OS_STK AppTaskT_VSampleStk[APP_TASK_T_V_SAMPLE_STK_SIZE];
__align(8) static OS_STK AppTaskCom_485Stk[APP_TASK_COM_485_STK_SIZE];
__align(8) static OS_STK AppTaskInsideCom_485Stk[APP_TASK_COM_485_STK_SIZE];
__align(8) static OS_STK AppTaskGSMStk[APP_TASK_GSM_STK_SIZE];
__align(8) static OS_STK AppTaskNrf905Stk[APP_TASK_NRF905_STK_SIZE];
												 
OS_EVENT *TxMbox;
OS_EVENT *InsideTxMbox;//485�ڲ�ͨѶ��������������ģ�飩
OS_EVENT *DisMbox;
OS_EVENT *ParaMbox;
OS_EVENT *DataMbox;//���ģ�鴫���Ĳ�������

OS_EVENT *CalSem;//������AppTaskT_VSample
OS_EVENT *GSMSem;	//GSM������ѭ���ź���
OS_EVENT *DetectSem;
OS_EVENT *SendSucceedSem;//���ŷ��ͳɹ��ź���
OS_EVENT *Nrf905Sem;	//û������ģ��ʱһֱ�������������Խ�ԼCPU��Դ��



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
		
		TxMbox = OSMboxCreate((void *)0);							 /*��������ͨѶ����*/
		InsideTxMbox = OSMboxCreate((void *)0);							 /*��������ͨѶ����*/		
		DisMbox = OSMboxCreate((void *)0);							 /*��ʾ���Ĵ���ͨѶ����*/
		ParaMbox = OSMboxCreate((void *)0);	
		DataMbox = OSMboxCreate((void *)0);						/*���ģ�鴫���Ĳ�������*/
		
		CalSem = OSSemCreate(1);
		GSMSem = OSSemCreate(0);
		DetectSem = OSSemCreate(0);
		SendSucceedSem = OSSemCreate(0);
		
		Nrf905Sem = OSSemCreate(0);
		
		AppTaskCreate();                                /* Create application tasks  */
		wtd();	//��ʼ���������������Ź���ι����SysTickHandler�� 
		
		printf("\r\nϵͳ�����ˣ�\r\n");
		 
		
		LED_RUN_ON;//����ָʾ�ơ���Exception�йض�
		
		
//test
		while(DEF_TRUE) 
		{			
				if(!FLAG_warning) {	//δ����ʱ������У׼��ָʾ���б���ʱ�˵�һֱ��
					if(KEY_CAL) {
						OSTimeDlyHMSM(0, 0, 0, 700);
						LED_RUN_ON;
						OSTimeDlyHMSM(0, 0, 0, 700);
						LED_RUN_OFF;
					}else {
						LED_RUN_ON;	
						OSTimeDlyHMSM(0, 0, 0, 200);			//�л�����			
					}
				}		
					if(GSM_send) {
						FLAG_warning = TRUE;
					}
					if(FLAG_warning) {
						LED_RUN_ON;		
						//��������˸,��������
						LED_WARNING_ON;
						OSTimeDlyHMSM(0, 0, 0, 200);
						LED_WARNING_OFF;
						OSTimeDlyHMSM(0, 0, 0, 200);
	
					}
					
					if(KEY_CLEAR == 0) {	//����ͻָ�����
						FLAG_warning = (bool)!FLAG_warning;
						if(FLAG_warning == TRUE) {
							//�������������ָ�����
							beep(100);
							OSTimeDlyHMSM(0, 0, 0, 100);
							beep(100);
						}
						if(FLAG_warning == FALSE) {
							//��������һ���������
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

//����������
uint16_t VirtAddVarTab[NumbOfVar] = {0};//EEPROM�ļ�����/*Virtual address define by the user: 0XFFFF value is prohibited */
static  void	AppTaskUserif(void *p_arg)
{
		INT8U err;
    unsigned char *ComBuffer;
		
		
//	 /*��ʼ�����ݱ�����*/
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
        /*ͨѶ��ʱ*/
        else if(err == OS_ERR_TIMEOUT) {
        }
				
				/*������Ϣ���*/
				if(FLAG_Setif) //���ý���ʱ��������λ���������ݣ�����������ʱҲ�����������׳�������
				{ 
						Param_IF();//�������棺��ص�ѹ���������绰��
				}
				FLAG_Setif =FALSE;
		}
}

//�ⲿͨ������������������λ����
static  void  AppTaskCom_485(void *p_arg)
{
		unsigned char *ComBuffer;
		
    INT8U err;	

		R_485_EN1;//��Զ��rs485���ա�
		
    while(DEF_TRUE) {

				/*����ӱ�־�¼���ʵ�ֳ�ʱ����*/
        ComBuffer = (unsigned char *)OSMboxPend(TxMbox, 0, &err);
							
        if(err == OS_ERR_NONE) 
				{
						com(ComBuffer);
        }

        /*ͨѶ��ʱ*/
        else if(err == OS_ERR_TIMEOUT) {
        }
    }
}

//�ڲ�ͨѶ����������������ģ�飩
static  void  AppTaskInsideCom_485(void *p_arg)
{
		unsigned char *ComBuffer;
		
    INT8U err;	
		int i =0;
		uint8_t TempBuffer[12];
		R_485_EN2;//�򿪽���rs485���ա�
		
    while(DEF_TRUE) {

				/*����ӱ�־�¼���ʵ�ֳ�ʱ����*/
        ComBuffer = (unsigned char *)OSMboxPend(InsideTxMbox, 0, &err);
							
        if(err == OS_ERR_NONE) 
				{
					
					/*ͨѶЭ����� Э���ʽ
					 ֡ͷ��8bit��+�ӻ���ַ��8bit��+ָ����루8bit��+���ݳ��ȣ�16bit��
					+��������+CRC16��8bit��+֡β��8bit��*/

					for (i=0; i<12; i++)
					{
						TempBuffer[i] = ComBuffer[i+5];
					}
					OSMboxPost(DataMbox, (void *)TempBuffer); //������ת��AppTaskT_VSample

        }

        /*ͨѶ��ʱ*/
        else if(err == OS_ERR_TIMEOUT) {
        }
    }
}

//��Ƶ905ͨ������
bool FlAG_Nrf905_Send = FALSE;//nrf905���ͱ�־λ
extern bool FLAG_Nrf905_Used;	//�������ģ����û�в���
uint8_t COM_SEND[6]={0xEE,0x01,0x07,0x00,0x00,0xED};//Channel_Select�иı���������ݡ�
static void AppTaskNrf905(void *pdata)
{
		INT8U err;
		n95_Init_IO();										// ��ʼ��nRF905����״̬
		n95_Init_Dev();										// ��ʼ��nRF905,������ת��Ϊ����״̬
		
		Nrf905_Test();	//��һ����Ϣ�����DR���ŵ�ƽ�᲻���øߡ��ɴ����ж�����ģ����û�в��š�û�еĻ���һֱ�����������񣨼������񣩣��Խ�ԼCPU��Դ��
	  while (DEF_TRUE)
   {
				OSSemPend(Nrf905Sem, 0, &err);	//У׼ʱ�رղɼ�
				
				if(n95_Check_DR(n95_RX_Buff)){					// nrf905���ͽ���
										
						OSMboxPost(DataMbox, (void *)n95_RX_Buff); //������ת��AppTaskT_VSample

				}
				
				if(FlAG_Nrf905_Send){					// nrf905���Ϳ�
										
						n95_Sendout(COM_SEND);		// nrf905���߷���		
						
						FlAG_Nrf905_Send = FALSE; // nrf905���͹�

				}
				OSTimeDlyHMSM(0,0,0,10);
				
				OSSemPost(Nrf905Sem);	//����֮���ܼ���ѭ����

	 }

}


//GSM����
static void AppTaskGSM(void *pdata)
{
    uint8_t err;
		
    while(DEF_TRUE) {
			
			OSSemPend(GSMSem, 0, &err);
			sim900a();
    }
} 

//testZigbee

//�����ɼ�����
static void ParaInit(uint16_t ChannelSlope[4][36]);
float ParaAll[4][48] = {0.0};	//48��ͨ����ÿ��4������
void Channel_Select(uint8_t CHANNEL_NUM);//ͨ��ѡ�񣬼�ѡ��ָ���ĵ�ؽ��в�������У׼����ȡ��ѹ�ȣ�
uint16_t  ParaCal(uint8_t p, uint8_t NumChannel);	//����У׼����
uint16_t r_Cal_C = 2800;
float V_value[36] = {0.0};
float T_value[36] = {0.0};
float r_value[36] = {0.0};	
float C_value[36] = {0.0};	
static void AppTaskT_VSample(void *pdata)
{

		INT8U err;

		uint32_t temp;
		uint16_t bettery_num;	//��ر��

		uint8_t Screen_id = 0;
		uint8_t	resend_time = 0;//�ط�����
	  uint8_t n = 0;0.
		unsigned char *ComBuffer;
		uint16_t ChannelSlope[4][36] = {0}; //��ѹ���¶ȡ������������У׼������36ͨ��

    ParaInit(ChannelSlope);//����У׼
#if 1		
  	FSmain();	
 
#endif
	
#if 0
    while(DEF_TRUE) 
		{
				n++;
			  if(n>10){ //û���100�ε�ѹ�¶ȣ����1������
					Channel_Select(0);//���м������ʼ����
					OSTimeDlyHMSM(0,0,2,0);	//�ȴ������������
					n = 0;
				}
				
				for(bettery_num = 1; bettery_num < 5; bettery_num++) //36������ѯ
				{
						OSSemPend(CalSem, 0, &err);	//У׼ʱ�رղɼ�
						printf("num= %d  ",bettery_num);
RESEND:						
				//��ѯ��ز���
						Channel_Select(bettery_num);
						
						
				//��������,��������������һ�ν��յ�������
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
						//ͨѶ��ʱ�������˵�أ����������
						else if(err == OS_ERR_TIMEOUT) 
						{			
								printf("time out\r\n");
								resend_time++;
								if(resend_time < 5){
									goto	RESEND;			//���������Ӧ�����·�������		
								}else{
									printf("�����%d����Ӧ��\r\n",bettery_num);
								}
								
//								OSSemPost(CalSem);  
//								continue;
						}else
						{
							printf("\r\nerr  =  %d\r\n",err);
						}
						
						resend_time = 0;//�ط���������

				//����У׼
						//��ѹУ׼
						ParaAll[0][bettery_num-1] = V_value[bettery_num] * ChannelSlope[0][bettery_num-1] * 0.001;
						printf("V_value = ,%f,",ParaAll[0][bettery_num-1]);
						if((ParaAll[0][bettery_num-1] > 100) || (ParaAll[0][bettery_num-1] < 0)) 
						{
							ParaAll[0][bettery_num-1] = 0;							
						}
						
						
						//�¶�У׼
						ParaAll[1][bettery_num-1] = T_value[bettery_num] * ChannelSlope[1][bettery_num-1] * 0.001;
						printf("T_value = ,%f,",ParaAll[1][bettery_num-1]);	
						if((ParaAll[1][bettery_num-1] > 100) || (ParaAll[1][bettery_num-1] < 0))
						{
							ParaAll[1][bettery_num-1] = 0;							
						}
						
						//����У׼
						ParaAll[2][bettery_num-1] = r_value[bettery_num] * ChannelSlope[2][bettery_num-1] * 0.001;
						printf("r_value = ,%f,",ParaAll[2][bettery_num-1]);
						if((ParaAll[2][bettery_num-1] > 100) || (ParaAll[2][bettery_num-1] < 0))
						{
							ParaAll[2][bettery_num-1] = 0;							
						}
						
						//����У׼
						ParaAll[3][bettery_num-1] = 180 - 80 *ParaAll[2][bettery_num-1] *1000/ r_Cal_C * ChannelSlope[3][bettery_num-1] * 0.001;				
						printf("C_value = ,%f\r\n",ParaAll[3][bettery_num-1]);
						if(ParaAll[2][bettery_num-1]<0.5)
						{
								ParaAll[3][bettery_num-1] = 0;
						}
						else if(ParaAll[3][bettery_num-1] > 99.9) //�������ܳ���99.9%
								{	
										ParaAll[3][bettery_num-1] = 99.9;
								}
								else if(ParaAll[3][bettery_num-1] < 0) 	//�������ܵ���0
										{
												ParaAll[3][bettery_num-1] = 0;
										}
						
						OSSemPost(CalSem);  
				//�Ƴ���ʾ
						if(3 ==((bettery_num-1)%4) )	//ÿ�����Ŀ���������ʾ��һ�Σ�����ҳ��ʾ
						{
							Screen_id = bettery_num/4-1;
							
							printf("Screen_id = %d",Screen_id);
							Show(Screen_id);
						}
					
				}
				

				//�쳣����
				if (strcmp(AbnormalNumber,AN_former) != 0) //�������쳣���ݺ��ϴ��쳣���ݲ�ͬ��ˢ���쳣���ݲ�������֪ͨ�ͻ�
				{
					FLAG_warning = TRUE;//������־���Ʊ���ָʾ�ơ�
					Abnormal_show();	//��������ת���쳣��ؽ��棬����ʾ�쳣��ز�����Ϣ
					Send_Message(1);		//������֪ͨ��ά��Ա����쳣��Ϣ��
					OSSemPend(SendSucceedSem, 3000, &err);	//�ȴ�����1���ŷ��ͳɹ���3s��ʱ����
					if(err == OS_ERR_NONE) 
					{
						printf("\r\nSendSucceedSem1 received!\r\n");
					}	
					else if(err == OS_ERR_TIMEOUT) 
					{
							OSSemPend(GSMSem, 0, &err);	//ֹͣGSM�����ڵ�ѭ��
							printf("\r\nOS_ERR_TIMEOUT,Stop the GSM1 sending.\r\n");	
					}
					
					Send_Message(2);		//�����Ÿ�����2
					OSSemPend(SendSucceedSem, 3000, &err);	//�ȴ�����2���ŷ��ͳɹ���3s��ʱ����
					if(err == OS_ERR_NONE) 
					{
						printf("\r\nSendSucceedSem2 received!\r\n");
					}	
					else if(err == OS_ERR_TIMEOUT) 
					{
							OSSemPend(GSMSem, 0, &err);	//ֹͣGSM�����ڵ�ѭ��
							printf("\r\nOS_ERR_TIMEOUT,Stop the GSM2 sending.\r\n");	
					}
					
					Send_Message(3);		//�����Ÿ�����3
					OSSemPend(SendSucceedSem, 3000, &err);	//�ȴ�����3���ŷ��ͳɹ���3s��ʱ����
					if(err == OS_ERR_NONE) 
					{
						printf("\r\nSendSucceedSem3 received!\r\n");
					}	
					else if(err == OS_ERR_TIMEOUT) 
					{
							OSSemPend(GSMSem, 0, &err);	//ֹͣGSM�����ڵ�ѭ��
							printf("\r\nOS_ERR_TIMEOUT,Stop the GSM3 sending.\r\n");	
					}
				}
				memset(AbnormalNumber, 0, sizeof(AbnormalNumber));	//���AbnormalNumber�����´η�����ڴ����
			
		}

#endif
}
/*************************************************
Function: Channel_Select
Description: ѡ��ͨ����
Input:
Return:
Others:
 *************************************************/
void Channel_Select(uint8_t CHANNEL_NUM)
{
		uint8_t temp[6]={0xEE,0x01,0x07,0x00,0x00,0xED};	//�����ֶ����ȫ�����������⣬���ղ���ģ������ݣ�
		INT8U i = 0;
		
		if(CHANNEL_NUM == 0)
		{
				temp[2] = 0x03;//���м��ģ�鿪ʼ�ɼ�ָ�EE 00 03 00 00 ED
		}
    if(CHANNEL_NUM > 48) 
		{
        /*��Ч��ͨ����*/
        //printf("invalid channel num!!\r\n");
    } 
		else 
		{
				temp[1] = CHANNEL_NUM;
				
				T_485_EN2;
				r485_Sendout(temp);		// rs485���߷���
				R_485_EN2;
				

				if(FLAG_Nrf905_Used)	//û��������ģ��Ͳ�ִ���ڲ����룬��ԼCPU��Դ��
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
Description: У׼���������Ϊ15λAD��ֵ
Input: p---У׼�����Ĵ��� 0 ��ѹ 1 �¶� 2����Rv 3����Ri
NumChannel---��ҪУ׼��ͨ����
Return: У׼�����ֵ
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
    /*����3���˲����������»�׼ֵ*/
    //	 DataFilter(&Database,ADC1ConvertedValue+NumChannel,3,&Overcounter);
    //	 DataFilter(&Database,ADC1ConvertedValue+NumChannel,3,&Overcounter);
    //	 DataFilter(&Database,ADC1ConvertedValue+NumChannel,3,&Overcounter);
    //printf("*******************%d*****\r\n",NumChannel);

    while(1) {
        OSTimeDlyHMSM(0, 0, 0, 1);
        //SUM0 += DataFilter(&Database,ADC1ConvertedValue+NumChannel,3,&Overcounter);
				switch(p) {
					case 0:	//�¶�
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
						
					case 1: //��ѹ	
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
						
					case 2: //����Rv
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
				
					case 3: //����Ri
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
    /*������ʼ�� 48ͨ��*/
    for(i = 0; i < 36; i++) {
        /*��ѹ*/
//        EE_ReadVariable(Channel1_V_FULL_Addr + i, ChannelSlope[0] + 2 * i);
//        EE_ReadVariable(Channel1_V_ZERO_Addr + i, ChannelSlope[0] + 2 * i + 1);

//        /*���δ����У׼����������Ĭ��ֵ*/
//        if(ChannelSlope[0][2 * i] == 0X00) {
            ChannelSlope[0][i] = 1000;
//        }

        /*�¶�*/
//        EE_ReadVariable(Channel1_T_FULL_Addr + i, ChannelSlope[1] + 2 * i);
//        EE_ReadVariable(Channel1_T_ZERO_Addr + i, ChannelSlope[1] + 2 * i + 1);

//        if(ChannelSlope[1][2 * i] == 0X00) {
            ChannelSlope[1][i] = 1000;
//        }
				
				/*����*/
        EE_ReadVariable(Channel1_r_k_Addr + i, ChannelSlope[2] + i);

        if(ChannelSlope[2][i] == 0X00) {
            ChannelSlope[2][i] = 1000;
        }
			
				/*����*/
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
