/************************************************************
  Copyright (C), 2010-2013, Haohua Tech. Co., Ltd.
  FileName:    pt100.c
  Author:      BlackStone
  Version :    V1.0
  Date: 	   2013.1.7
  Description:     pt100测温
  History:         // 历史修改记录
      <author>      <time>   <version >   <desc>
      BlackStone    13.1.7     1.0     build this moudle
***********************************************************/
#include "includes.h"


extern char Channel;
/*************************************************
  Function:       R2T
  Description:    对PT100进行阻值到温度的转换
  Input:          Resis---测量的电阻值
  Return:         对应的温度值
  Others:
*************************************************/

float R2T(float Resis)
{
    float t = 0.0;
    float x = 0.0;
    float y = 0.0;
    float z = 0.0;
    float r = 0.0;

    /*判断是否在该种热电阻的温度范围之内*/
    if((Resis < 39.5) || (Resis > 390.6)) {
//        switch (Channel)
//		{
//		  case 0:
//		  {
//		    printf("Channel 0 TempSensor error！\r\n");
//		    break;
//		  }
//		  case 1:
//		  {
//		    printf("Channel 1 TempSensor error！\r\n");
//		    break;
//		  }
//		  case 2:
//		  {
//		    printf("Channel 2 TempSensor error！\r\n");
//		    break;
//		  }
//		}
    } else {
        if((Resis >= 100.0) && (Resis <= 390.6)) {
            z = 1.0 - Resis / 100.0;
            x = (3.9083 / 1e3) * (3.9083 / 1e3) - 4.0 * (-5.775 / 1e7) * z;
            x = sqrt(x);
            t = (x - (3.9083 / 1e3)) / 2.0 / (-5.775 / 1e7);
        }

        if((Resis < 100.0) && (Resis >= 39.5)) {
            x = 0.0;
            y = -100.0;
            z = -201.0;
            r = 100.0 * (1.0 + (3.9083 / 1e3) * y + (-5.775 / 1e7) * y * y + (-4.183 / 1e12) * (y - 100.0) * y * y * y);

            while(((r - Resis) >= 0.001) || ((r - Resis) <= -0.001)) {
                if(Resis > r) {
                    x = x;
                    z = y;
                    y = (x + y) / 2.0;

                } else {
                    x = y;
                    z = z;
                    y = (y + z) / 2.0;
                }

                r = 100.0 * (1.0 + (3.9083 / 1e3) * y + (-5.775 / 1e7) * y * y + (-4.183 / 1e12) * (y - 100.0) * y * y * y);
            }

            t = y;
        }
    }

    return(t);
}

