/**----------------------------文件信息---------------------------------
** 文件名称:   Screen3.c
** 修改时间:   2013-12-27 13:47:50
** 文件说明:   Screen3画面函数
**--------------------------------------------------------------------**/
/**----------------------------------------------------------------------
**                         使用必读
**每个page函数的驱动程序在hmi_driver.c文件中，用户可以复制添加到应用程序中
**--------------------------------------------------------------------**/
//DefineID.h
void Screen3()
{
//设置视图背景色
    SetBcolor(31);
    GUI_CleanScreen();
//设置视图背景图片
    DisFull_Image(1, 0);
//图片显示
//图片名: 弹出.bmp
    DisArea_Image(78, 38, 7, 1);
}
