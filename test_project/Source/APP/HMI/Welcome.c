/**----------------------------文件信息---------------------------------
** 文件名称:   Welcome.c
** 修改时间:   2013-12-27 13:47:50
** 文件说明:   Welcome画面函数
**--------------------------------------------------------------------**/
/**----------------------------------------------------------------------
**                         使用必读
**每个page函数的驱动程序在hmi_driver.c文件中，用户可以复制添加到应用程序中
**--------------------------------------------------------------------**/
//#include  "DefineID.h"

void Welcome()
{
//设置视图背景色
    SetBcolor(31);
    GUI_CleanScreen();
//设置视图背景图片
    DisFull_Image(0, 0);
}
