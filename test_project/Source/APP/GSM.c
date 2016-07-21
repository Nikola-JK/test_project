#include "GSM.h"
#include <stdio.h>
#include "stm32f10x.h"
#include "includes.h"

#if 0
//////////////////////////////////////////////////////////////////////////////////	

#define SRAMIN	0	//内部内存池
#define SRAMEX  1	//外部内存池

typedef unsigned short	WCHAR;

#define USART2_MAX_RECV_LEN		1024				//最大接收缓存字节数
#define USART2_MAX_SEND_LEN		1024				//最大发送缓存字节数
#define USART2_RX_EN 			1					//0,不接收;1,接收.

extern u8  USART2_RX_BUF[USART2_MAX_RECV_LEN]; 		//接收缓冲,最大USART2_MAX_RECV_LEN字节
extern u8  USART2_TX_BUF[USART2_MAX_SEND_LEN]; 		//发送缓冲,最大USART2_MAX_SEND_LEN字节
extern u16 USART2_RX_STA;   						//接收数据状态



//串口发送缓存区 	
__align(8) u8 USART2_TX_BUF[USART2_MAX_SEND_LEN]; 	//发送缓冲,最大USART2_MAX_SEND_LEN字节
//#ifdef USART2_RX_EN   								//如果使能了接收   	  
//串口接收缓存区 	
u8 USART2_RX_BUF[USART2_MAX_RECV_LEN]; 				//接收缓冲,最大USART2_MAX_RECV_LEN个字节.


//通过判断接收连续2个字符之间的时间差不大于10ms来决定是不是一次连续的数据.
//如果2个字符接收间隔超过10ms,则认为不是1次连续数据.也就是超过10ms没有接收到
//任何数据,则表示此次接收完毕.
//接收到的数据状态
//[15]:0,没有接收到数据;1,接收到了一批数据.
//[14:0]:接收到的数据长度
u16 USART2_RX_STA=0;   	 
 
 
/////////////////////////////////////////////////////////////////////////////////////////////////////////// 
//usmart支持部分

//将收到的AT指令应答数据返回给电脑串口
//mode:0,不清零USART2_RX_STA;
//     1,清零USART2_RX_STA;
void sim_at_response(u8 mode)
{
	if(USART2_RX_STA&0X8000)		//接收到一次数据了
	{ 
		USART2_RX_BUF[USART2_RX_STA&0X7FFF]=0;//添加结束符
		printf("%s",USART2_RX_BUF);	//发送到串口
		if(mode)USART2_RX_STA=0;
	} 
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////// 
//ATK-SIM900A 各项测试(拨号测试、短信测试、GPRS测试)共用代码

//sim900a发送命令后,检测接收到的应答
//str:期待的应答结果
//返回值:0,没有得到期待的应答结果
//    其他,期待应答结果的位置(str的位置)
u8* sim900a_check_cmd(u8 *str)
{
	char *strx=0;
	if(USART2_RX_STA&0X8000)		//接收到一次数据了
	{ 
		USART2_RX_BUF[USART2_RX_STA&0X7FFF]=0;//添加结束符
		strx=strstr((const char*)USART2_RX_BUF,(const char*)str);
	} 
	return (u8*)strx;
}
//向sim900a发送命令
//cmd:发送的命令字符串(不需要添加回车了),当cmd<0XFF的时候,发送数字(比如发送0X1A),大于的时候发送字符串.
//ack:期待的应答结果,如果为空,则表示不需要等待应答
//waittime:等待时间(单位:10ms)
//返回值:0,发送成功(得到了期待的应答结果)
//       1,发送失败
u8 sim900a_send_cmd(u8 *cmd,u8 *ack,u16 waittime)
{
	u8 res=0; 
	USART2_RX_STA=0;
	if((u32)cmd<=0XFF)
	{
		while(DMA1_Channel7->CNDTR!=0);	//等待通道7传输完成   
		USART2->DR=(u32)cmd;
	}else u2_printf("%s\r\n",cmd);//发送命令
	if(ack&&waittime)		//需要等待应答
	{
		while(--waittime)	//等待倒计时
		{
			delay_ms(10);
			if(USART2_RX_STA&0X8000)//接收到期待的应答结果
			{
				if(sim900a_check_cmd(ack))break;//得到有效数据 
				USART2_RX_STA=0;
			} 
		}
		if(waittime==0)res=1; 
	}
	return res;
} 
//将1个字符转换为16进制数字
//chr:字符,0~9/A~F/a~F
//返回值:chr对应的16进制数值
u8 sim900a_chr2hex(u8 chr)
{
	if(chr>='0'&&chr<='9')return chr-'0';
	if(chr>='A'&&chr<='F')return (chr-'A'+10);
	if(chr>='a'&&chr<='f')return (chr-'a'+10); 
	return 0;
}
//将1个16进制数字转换为字符
//hex:16进制数字,0~15;
//返回值:字符
u8 sim900a_hex2chr(u8 hex)
{
	if(hex<=9)return hex+'0';
	if(hex>=10&&hex<=15)return (hex-10+'A'); 
	return '0';
}
//unicode gbk 转换函数
//src:输入字符串
//dst:输出(uni2gbk时为gbk内码,gbk2uni时,为unicode字符串)
//mode:0,unicode到gbk转换;
//     1,gbk到unicode转换;
void sim900a_unigbk_exchange(u8 *src,u8 *dst,u8 mode)
{
	u16 temp; 
	u8 buf[2];
	if(mode)//gbk 2 unicode
	{
		while(*src!=0)
		{
			if(*src<0X81)	//非汉字
			{
				temp=(u16)ff_convert((WCHAR)*src,1);
				src++;
			}else 			//汉字,占2个字节
			{
				buf[1]=*src++;
				buf[0]=*src++; 
				temp=(u16)ff_convert((WCHAR)*(u16*)buf,1); 
			}
			*dst++=sim900a_hex2chr((temp>>12)&0X0F);
			*dst++=sim900a_hex2chr((temp>>8)&0X0F);
			*dst++=sim900a_hex2chr((temp>>4)&0X0F);
			*dst++=sim900a_hex2chr(temp&0X0F);
		}
	}else	//unicode 2 gbk
	{ 
		while(*src!=0)
		{
			buf[1]=sim900a_chr2hex(*src++)*16;
			buf[1]+=sim900a_chr2hex(*src++);
			buf[0]=sim900a_chr2hex(*src++)*16;
			buf[0]+=sim900a_chr2hex(*src++);
 			temp=(u16)ff_convert((WCHAR)*(u16*)buf,0);
			if(temp<0X80){*dst=temp;dst++;}
			else {*(u16*)dst=swap16(temp);dst+=2;}
		} 
	}
	*dst=0;//添加结束符
} 

//测试短信发送内容(70个字[UCS2的时候,1个字符/数字都算1个字])
const u8* sim900a_test_msg="您好，这是一条测试短信，由SIM900A GSM模块发送，谢谢支持！";
//SIM900A发短信测试 
void sim900a_sms_send_test(void)
{
	u8 *p,*p1,*p2,*p3,*p4;
	u8 phonebuf[20] = "18501287068"; 		//号码缓存
	u8 pohnenumlen=0;		//号码长度,最大15个数 
	u8 smssendsta=0;		//短信发送状态,0,等待发送;1,发送失败;2,发送成功 
//	p=mymalloc(SRAMIN,100);	//申请100个字节的内存,用于存放电话号码的unicode字符串
//	p1=mymalloc(SRAMIN,300);//申请300个字节的内存,用于存放短信的unicode字符串
//	p2=mymalloc(SRAMIN,100);//申请100个字节的内存 存放：AT+CMGS=p1 

	while(1)
	{
					sim900a_unigbk_exchange(phonebuf,p,1);				//将电话号码转换为unicode字符串
					sim900a_unigbk_exchange((u8*)sim900a_test_msg,p1,1);//将短信内容转换为unicode字符串.
					
					sprintf((char*)p3,"AT+CSCS=\"%s\"","GSM");	//GSM字符集
					sprintf((char*)p3,"AT+CMGF=1");	//文本模式
					sprintf((char*)p2,"AT+CMGS=\"%s\"",p);
					
		    	sim900a_send_cmd(p3,"",200);	//设置GSM字符集
					sim900a_send_cmd(p4,"OK",200);		//设置文本模式
					if(sim900a_send_cmd(p2,">",200)==0)					//发送短信命令+电话号码
					{ 	
						sim900a_send_cmd(p1,"",200);	//发送短信内容到GSM模块 						
//						u2_printf("%s",p1);		 						//发送短信内容到GSM模块 
 						if(sim900a_send_cmd((u8*)0X1A,"+CMGS:",1000)==0)smssendsta=2;//发送结束符,等待发送完成(最长等待10秒钟,因为短信长了的话,等待时间会长一些)
					}  
//					if(smssendsta==1)Show_Str(30+40,70,170,90,"发送失败",16,0);	//显示状态
//					else Show_Str(30+40,70,170,90,"发送成功",16,0);				//显示状态	
//					USART2_RX_STA=0;
	
	
//		if(USART2_RX_STA&0X8000)sim_at_response(1);//检查从GSM模块接收到的数据 
//	
//	myfree(SRAMIN,p);
//	myfree(SRAMIN,p1);
//	myfree(SRAMIN,p2); 
//	
	}
} 

#endif