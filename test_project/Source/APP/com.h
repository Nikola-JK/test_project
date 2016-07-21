
#ifndef com_H
#define com_H
#include "stm32f10x.h"
#include "math.h"

#define RX_SIZE 4
#define ADD             BKP_DR1
#define V_BETTRY        BKP_DR2
#define N_BETTRY        BKP_DR3
#define C_BETTRY        BKP_DR4
#define r_TOP_BETTRY    BKP_DR5
#define V_TOP_BETTRY    BKP_DR6
#define V_BOTTOM_BETTRY	BKP_DR7
#define T_TOP_BETTRY    BKP_DR8
#define C_BOTTOM_BETTRY BKP_DR9
#define Tel_part1 BKP_DR10	//�绰����1��һ����
#define Tel_part2 BKP_DR11
#define Tel_part3 BKP_DR12
#define PW_part1 BKP_DR13		//�����һ����,��ʮλ
#define PW_part2 BKP_DR14	
#define PW_part3 BKP_DR15	
#define PW_part4 BKP_DR16	
#define PW_part5 BKP_DR17	
#define PW_part6 BKP_DR18	
#define PW_part7 BKP_DR19	
#define PW_part8 BKP_DR20	
#define PW_part9 BKP_DR21	
#define PW_part10 BKP_DR22

#define Tel2_part1 BKP_DR23	//�绰����2��һ����
#define Tel2_part2 BKP_DR24
#define Tel2_part3 BKP_DR25
#define Tel3_part1 BKP_DR26	//�绰����3��һ����
#define Tel3_part2 BKP_DR27
#define Tel3_part3 BKP_DR28


typedef enum {
    NONE = 0,
    COM_FRAME_HEADER ,
    COM_FRAME_ADDR,
    COM_FRAME_I_CODE,
    COM_FRAME_D_LENGTH_HI,
    COM_FRAME_D_LENGTH_LO,
    COM_FRAME_D_CONTENT,
    COM_FRAME_CRC_HI,
    COM_FRAME_CRC_LO,
    COM_FRAME_TAIL
} PROTOCOL_NODE;

/*
*
*
*
*/

//typedef enum {
//    HAND_SHAKE_inside      = 0X00,		/*�豸������Э��*/
//    R_VERSION_inside       = 0X01,		/*��ȡ�豸�̼��汾*/
//    R_ADDR_inside          = 0X02,		/*��ȡ�豸��ַ*/
//    DETECT				  = 0X03,		/*ģ�鿪ʼ���Ե�ѹ�����¶�����*/
//    R_B_V     		  = 0X04,		/*��ȡ��صĵ�ѹ*/
//    R_B_T     		  = 0X05,		/*��ȡ��ص��¶���Ϣ*/
//    R_B_C     		  = 0X06,		/*��ȡ��ص�����ֵ*/
//    R_B_VTC   		  = 0X07,		/*��ȡ������е�VTRC*/


//} I_CODE_inside;

typedef enum {
    HAND_SHAKE      = 0X00,		/*�豸������Э��*/
    R_VERSION       = 0X01,		/*��ȡ�豸�̼��汾*/
    R_ADDR          = 0X02,		/*��ȡ�豸��ַ*/
    R_A_B_CONFIG    = 0X03,		/*��ȡ������������Ϣ*/
    R_ALL_B_V       = 0X04,		/*��ȡ�����ĵ�ѹ*/
    R_ALL_B_T       = 0X05,		/*��ȡ�������¶���Ϣ*/
    R_ALL_B_C       = 0X06,		/*��ȡ����������ֵ*/
    R_ALL_B_VTC     = 0X07,		/*��ȡ��������е�VTRC*/
    R_SINGLE_B_VTC  = 0X08,		/*��ȡ�����ص�VTC*/
    R_T_LIMIT       = 0X09,		/*��ȡ�¶�����ֵ*/
    R_R0_LIMIT      = 0X0A,		/*��ȡ��������ֵ*/
    R_V_LIMIT       = 0X0B,		/*��ȡ�豸�ĵ�ѹ������*/
    R_ALARM         = 0X0C,		/*��ȡ�豸�ı�����Ϣ*/
    R_ALL_CONFIG    = 0X0D,		/*��ȡ���е�������Ϣ*/
    R_ALL_R0        = 0X0E,		/*��ȡ����������ֵ*/
    R_CURRENT       = 0X0F,		/*��ȡ��������*/
    R_POWER         = 0X10,		/*��ȡ���ع���*/
    R_UNIQID		= 0X11,		/*��ȡ�豸ID��*/
    R_SC_INF        = 0X12,		/*�豸�ϵ��Լ���Ϣ*/

    W_ADDR          = 0X81,		/*�����豸��ַ*/
    W_R0_LIMIT      = 0X82,		/*���ĵ�������������ֵ*/
    W_T_LIMIT       = 0X83,		/*���ĵ���¶�����ֵ*/
    W_V_LIMIT       = 0X84,		/*���ĵ�ص�ѹ���޺�����*/
    W_B_CONFIG      = 0X85,		/*���õ�ص������Ϣ����ر�׼��ѹ������������������*/


    CAL_V = 0XC0,				    /*��ѹУ׼*/
    CAL_T = 0XC1,					/*�¶�У׼*/
    CAL_R = 0XC2,					/*����У׼*/
		CAL_C = 0XC3,					/*����У׼*/
		
		TEL_N = 0XC4,					/*�绰����*/
		R_unwork1 = 0XC5,					/*0xC5 �������ʧЧ*/
		R_unwork2 = 0XC6,					/*0xC6 �������ָ�����ģʽ*/
		C_rated = 0XC7,					/*0xC7 ��ض����*/
		
    POWER_DOWN_ALARM = 0XFD,		/*���籨��*/
    CLEAR_ALARM      = 0XFE			/*�����ǰ������Ϣ*/

} I_CODE;
//extern void  com(unsigned char *com_buffer, unsigned char *DataBuffer);
extern void  com(unsigned char *com_buffer);
extern void  hand_shake(void);
extern void  r485_Sendout(uint8_t *p);

#endif
