
/************************************************************
Copyright (C), 2010-2011, ��ͨ�������Դ�Ƽ����޹�˾
FileName:  display.c
Author:    BlackStone
Version :  V1.0
Date: 	   2013.12.24
Description: ��Ҫ�����봮������ͨѶ��������ʾ���������

History: // ��ʷ�޸ļ�¼
<author>    <time>  <version >     <desc>
 Stone   13/12/23      1.0     build this moudle
 ***********************************************************/


#include "includes.h"

/*************************************************
Function: // SendFrameHeaderCMD
Description: // �����봮����ͨѶ��֡ͷ
Input: // ��
Return: // ��
Others: //
*************************************************/
void SendFrameHeaderCMD(void)
{
    USART_SendData(USART1, 0XEE);
}

/*************************************************
Function: // SendFrameTailCMD
Description: // �����봮����ͨѶ��֡β
Input: // ��
Return: // ��
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
Description: // ���ñ�����ɫ
Input: // RGB ��ֵ
Return: // ��
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
Description: // ���ñ�����ɫ
Input: // RGB ��ֵ
Return: // ��
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
Description: // ����
Input: // ��
Return: // ��
Others: // 	����ǰ�����ñ�����ɫ���������ã�Ĭ��Ϊ
            ��ɫ
*************************************************/
void Clear(void)
{
    SendFrameHeaderCmd();
    Usart_SendData(USART1, 0x01);
    SendFrameTailCMD();
}


