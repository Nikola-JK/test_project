
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
#define Tel_part1 BKP_DR10	//电话号码1第一部分
#define Tel_part2 BKP_DR11
#define Tel_part3 BKP_DR12
#define PW_part1 BKP_DR13		//密码第一部分,共十位
#define PW_part2 BKP_DR14	
#define PW_part3 BKP_DR15	
#define PW_part4 BKP_DR16	
#define PW_part5 BKP_DR17	
#define PW_part6 BKP_DR18	
#define PW_part7 BKP_DR19	
#define PW_part8 BKP_DR20	
#define PW_part9 BKP_DR21	
#define PW_part10 BKP_DR22

#define Tel2_part1 BKP_DR23	//电话号码2第一部分
#define Tel2_part2 BKP_DR24
#define Tel2_part3 BKP_DR25
#define Tel3_part1 BKP_DR26	//电话号码3第一部分
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
//    HAND_SHAKE_inside      = 0X00,		/*设备的握手协议*/
//    R_VERSION_inside       = 0X01,		/*读取设备固件版本*/
//    R_ADDR_inside          = 0X02,		/*读取设备地址*/
//    DETECT				  = 0X03,		/*模块开始测试电压内阻温度容量*/
//    R_B_V     		  = 0X04,		/*读取电池的电压*/
//    R_B_T     		  = 0X05,		/*读取电池的温度信息*/
//    R_B_C     		  = 0X06,		/*读取电池的容量值*/
//    R_B_VTC   		  = 0X07,		/*读取电池所有的VTRC*/


//} I_CODE_inside;

typedef enum {
    HAND_SHAKE      = 0X00,		/*设备的握手协议*/
    R_VERSION       = 0X01,		/*读取设备固件版本*/
    R_ADDR          = 0X02,		/*读取设备地址*/
    R_A_B_CONFIG    = 0X03,		/*读取电池组的配置信息*/
    R_ALL_B_V       = 0X04,		/*读取电池组的电压*/
    R_ALL_B_T       = 0X05,		/*读取电池组的温度信息*/
    R_ALL_B_C       = 0X06,		/*读取电池组的容量值*/
    R_ALL_B_VTC     = 0X07,		/*读取电池组所有的VTRC*/
    R_SINGLE_B_VTC  = 0X08,		/*读取单块电池的VTC*/
    R_T_LIMIT       = 0X09,		/*读取温度上限值*/
    R_R0_LIMIT      = 0X0A,		/*读取内阻上限值*/
    R_V_LIMIT       = 0X0B,		/*读取设备的电压上下限*/
    R_ALARM         = 0X0C,		/*读取设备的报警信息*/
    R_ALL_CONFIG    = 0X0D,		/*读取所有的配置信息*/
    R_ALL_R0        = 0X0E,		/*读取电池组的内阻值*/
    R_CURRENT       = 0X0F,		/*读取电池组电流*/
    R_POWER         = 0X10,		/*读取负载功率*/
    R_UNIQID		= 0X11,		/*获取设备ID号*/
    R_SC_INF        = 0X12,		/*设备上电自检信息*/

    W_ADDR          = 0X81,		/*更改设备地址*/
    W_R0_LIMIT      = 0X82,		/*更改电池组的内阻上限值*/
    W_T_LIMIT       = 0X83,		/*更改电池温度上限值*/
    W_V_LIMIT       = 0X84,		/*更改电池电压上限和下限*/
    W_B_CONFIG      = 0X85,		/*配置电池的相关信息，电池标准电压，电池总数，电池容量*/


    CAL_V = 0XC0,				    /*电压校准*/
    CAL_T = 0XC1,					/*温度校准*/
    CAL_R = 0XC2,					/*内阻校准*/
		CAL_C = 0XC3,					/*容量校准*/
		
		TEL_N = 0XC4,					/*电话号码*/
		R_unwork1 = 0XC5,					/*0xC5 内阻检测板失效*/
		R_unwork2 = 0XC6,					/*0xC6 内阻检测板恢复正常模式*/
		C_rated = 0XC7,					/*0xC7 电池额定容量*/
		
    POWER_DOWN_ALARM = 0XFD,		/*掉电报警*/
    CLEAR_ALARM      = 0XFE			/*清除当前报警信息*/

} I_CODE;
//extern void  com(unsigned char *com_buffer, unsigned char *DataBuffer);
extern void  com(unsigned char *com_buffer);
extern void  hand_shake(void);
extern void  r485_Sendout(uint8_t *p);

#endif
