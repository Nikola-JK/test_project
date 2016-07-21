
/************************************************************
Copyright (C), 2010-2011, 电通厚光新能源科技有限公司
FileName:  display.c
Author:    BlackStone
Version :  V1.0
Date: 	   2013.12.24
Description: 主要用于与串口屏的通讯，控制显示界面的内容

History: // 历史修改记录
<author>    <time>  <version >     <desc>
 Stone   13/12/23      1.0     build this moudle
 ***********************************************************/


#include "includes.h"

/*************************************************
Function: // SendFrameHeaderCMD
Description: // 发送与串口屏通讯的帧头
Input: // 无
Return: // 无
Others: //
*************************************************/
void SendFrameHeaderCMD(void)
{
    USART_SendData(USART1, 0XEE);
}

/*************************************************
Function: // SendFrameTailCMD
Description: // 发送与串口屏通讯的帧尾
Input: // 无
Return: // 无
Others: //
*************************************************/
void SendFrameTailCMD(void)
{
    USART_SendData(USART1, 0XFF);
    USART_SendData(USART1, 0XFC);
    USART_SendData(USART1, 0XFF);
    USART_SendData(USART1, 0XFF);
}

/*************************************************
Function: // SetBColor
Description: // 设置背景颜色
Input: // RGB 码值
Return: // 无
Others: //
*************************************************/
void SetBColor(uint16_t BColor)
{
    SendFrameHeaderCmd();
    Usart_SendData(USART1, 0x42);
    Usart_SendData(USART1, BColor);
    SendFrameTailCMD();
}

/*************************************************
Function: // SetFColor
Description: // 设置背景颜色
Input: // RGB 码值
Return: // 无
Others: //
*************************************************/
void SetFColor(uint16_t FColor)
{
    SendFrameHeaderCmd();
    Usart_SendData(USART1, 0x41);
    Usart_SendData(USART1, FColor);
    SendFrameTailCMD();
}

/*************************************************
Function: // Clear
Description: // 清屏
Input: // 无
Return: // 无
Others: // 	清屏前先设置背景颜色，若不设置，默认为
            蓝色
*************************************************/
void Clear(void)
{
    SendFrameHeaderCmd();
    Usart_SendData(USART1, 0x01);
    SendFrameTailCMD();
}


