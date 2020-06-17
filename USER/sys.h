
#ifndef _SYS_H_
#define _SYS_H_

#define VER_ROMMAT			0	//芮魅
#define VER_KESMAN			1	//克林斯曼
#define VER_SMALLBLACKMACH	2	//小黑鸡
#define VER_BIGWHITEMACH	3	//大白鸡
#define VER_WHITEBLUEMACH	4	//白蓝鸡


#define SYS_VERSION			20191126.01f//"V2.10-20190307"

//版本控制
/*
[V2.10-20190307]
修改标志为 micony20190307

1、2019-03-07 为安抚董情绪，驱车去上海，在上海办公室修改版本，硬件版本为惠州100套前撞凹进版本，机器为惠州发货机器。
2、回充参数，修改为 9,13,12,否则回充大弧线。奇怪的是，之前的参数是9 8 9 ，居然说是成功的，到上海发现不成功，不知何故
	cfg->dock_delay = 9;
	cfg->dock_l_count =13;	
	cfg->dock_r_count = 12; 	//存储至FLASH
3、中线两红外要减1000，虽然前撞贴过海绵，但还是要减1000，否则延边有问题
	
	#define IR_D_VALUE 1000      //1000

4、is_unclear_side函数修改，避免在上海会议室重复的去会议室凳子那个区域，不知道有没有其他隐患

	if(sys->t_navi_work > 1200)
	{
		max_c_ucln =	12;
		max_c_cln	=	3;
	}else
	{
		max_c_ucln	= 4;		//最大未清扫条数
		max_c_cln	= 0;		//最大清扫条数 micony20190307 上海办公室测试，发现从一个房间掉下来，就不会去清扫了。从董的办公室掉到外边的办公室，就不会去扫了。
	}
	*c_unclean		= c_ucln;
	*c_clean		= c_cln;

5、地图沿边计算前方是否扫描增加。否则导致在边界那里反复清扫。
		//micony20190307 于上海办公室,再往Y方向，连续连个点，都不是障碍点，才是未清扫点
		if(point_sta(m_edge->tx[len],m_edge->ty[len]+y_dir) != OBST_STA && point_sta(m_edge->tx[len-1],m_edge->ty[len-1]+y_dir) != OBST_STA &&
			point_sta(m_edge->tx[len],m_edge->ty[len]+y_dir*2) != OBST_STA && point_sta(m_edge->tx[len-1],m_edge->ty[len-1]+y_dir*2) != OBST_STA)
		{
			log_printf("CLR_FRONT,near(%d,%d,)\r\n",m_edge->tx[len],m_edge->ty[len]);
			return 2;
		}
6、很多沿边函数没加 navigat->near.pid->c_lost_flag = 0;			//失去墙的标志
	导致碰撞后，没有沿边起来。
7、前撞中间的值修改为上线1200,BORD_FRONT_100 编译开关
#if BORD_FRONT_100
			cfg->midle_adc   	= 1200;
#else
			cfg->midle_adc   	= 2300;
#endif	

*/


#define SYS_VER					VER_ROMMAT
#define VER_BORD_2				0	//第二版本
#define HOOVER					1	//胡佛版本
#define ICLEAN_HW_ULTRASONIC	0	//超声波地检
#define SIMPLIFY				0	//胡佛客户测试版
#define ENABLE_VOICE			0	//是否有语音

#define SINGLE_LED_DOCK			0	//0:双灯1:单灯
#include "stm32f10x.h"
#include "stm32f10x_rtc.h"
#include "stm32f10x_bkp.h"
#include "stm32f10x_pwr.h"
#include    <stdarg.h>
#include    <stdlib.h>
#include    <string.h>		//20190422 WY add
#include    <stdio.h>		//20190423 WY add
#include "ee_iic.h"

#include "usart.h"
#include "bootloader.h"

#include "pid.h"
#include "gyro.h"
#include "navigation.h"					  
#include "bsp.h"
#include "string.h"
#include "SysTick.h"
#include "motor.h"
#include "iclean.h"
#include "ringbuffer.h"


#include "dispprot.h"
#include "mcu_api.h"
#include "wifi.h"
#include "protocol.h"
#include "system.h"
#include "cfg.h"
#include "sensers.h"
#include "task_rx.h"
#include "key.h"

#include "adc.h"

#include "i2c.h"

#include "math.h"
#include "randclean.h"

#include "charge.h"
#include "libatcharge.h"
#include "ny3p.h"

#include "laser.h"
#include "spi.h"

#include "rtc_lib.h"
#include "key.h"

#include "map.h"
#include "action.h"
#include "targets.h"
#include "calibra.h"
#include "route.h"
#include "test_mode.h"
#include "stdio.h"
#include "mpu6500.h"

#define TRUE	0x01
#define FALSE	0x00

#define MAX(a,b) ((a) > (b) ? (a) : (b)) 
#define MIN(a,b) ((a) < (b) ? (a) : (b)) 

#define disable_irq()  {__ASM  volatile ("cpsid i");}
#define enable_irq()   {__ASM  volatile ("cpsie i");}
float disfloat(float x,float y);
void init_sys_sta(uint8_t sta);
uint32_t disxy_32(uint32_t x,uint32_t y);

#endif

