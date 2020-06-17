
#if 1//ROOMA_GYROONLY

#ifndef _KEY_H
#define _KEY_H

//加密及激活相关

//激活
#define SCUR_BYTES_ADDR	0x08008630
#define SCUR_BYTES_DATA_OLD	0xffffffff//0x77ec99ff
#define SCUR_BYTES_DATA_NEW	0x00000000//0x3628185a
#define SCUR_BYTES_DATA_LENTH	1



#define KEY_WIFI_EN	0
#define KEY_MUTE_EN	1

#define KEY_WIFI_MUTE	KEY_WIFI_EN
//#define KEY_START_LONG_NO	1

#define KEY_LONG_TIME	0x80
#define KEY_WIFI_NO		0
#define KEY_DOCK_NO		1
#define KEY_SPOT_NO		2


#define KEY_CLEAN_NO	3

#define KEY_WIFI_L_NO			(KEY_WIFI_NO + KEY_LONG_TIME)
#define KEY_PAIR_ROMATE_L_NO	(KEY_CLEAN_NO + KEY_LONG_TIME)


#if 1

#ifndef ICLEAN_IRCTRL_TYPE
#define ICLEAN_IRCTRL_TYPE	0
#endif

//NEC红外遥控器

#define IRDA_DEV_ADDR_ENABLE	1
#define IRDA_DEV_ADDR	0xa1

#if ICLEAN_IRCTRL_TYPE == 2
#define IRDA_CODE_INVALID	0xff//无效码

#define IRDA_CODE_POWER	0x96//电源 d
#define IRDA_CODE_MOP	0x92//抹布,无此功能
#define IRDA_CODE_SPOT	0x99//集中清扫 d
#define IRDA_CODE_AUTO	0x9c//自动清扫 d
#define IRDA_CODE_WALLWALK	0x97//沿墙清扫 d
#define IRDA_CODE_UP	0x92//向上键
#define IRDA_CODE_LEFT	0x94//向左键
#define IRDA_CODE_RIGHT	0x91//向右键
#define IRDA_CODE_DOWN	0x60//向下键
#define IRDA_CODE_TIM	0x41//时间,无此功能
#define IRDA_CODE_DOCK	0x9a//回充

#define IRDA_CODE_SETTIM	0xa5//确认键,用来发送时间及预约设置,收到此键进入时间及预约设置

#define IRDA_CODE_FORCE	0x93//强力模式开关


#define IRDA_CODE_WIFI	0xe9 //无此功能

#define IRDA_CODE_SCUR	0xd9//IRDA_CODE_WIFI

#elif ICLEAN_IRCTRL_TYPE == 3//小圆机的新遥控器(白色长条),不带预约功能

#define IRDA_CODE_INVALID	0xff//无效码

#define IRDA_CODE_POWER	0x00//电源 d
#define IRDA_CODE_MOP	0x92//抹布,无此功能
#define IRDA_CODE_SPOT	0xe8//集中清扫 d
#define IRDA_CODE_AUTO	0x90//0x10//自动清扫 d
#define IRDA_CODE_UP	0x70//向上键
#define IRDA_CODE_LEFT	0xc0//向左键
#define IRDA_CODE_RIGHT	0x18//向右键
#define IRDA_CODE_DOWN	0x60//向下键
#define IRDA_CODE_TIM	0x41//时间,无此功能
#define IRDA_CODE_DOCK	0xf2//回充
#define IRDA_CODE_K2	0x12//最后一排最左,无此功能
#define IRDA_CODE_K3	IRDA_CODE_INVALID//最后一排中间(现在无码),无此功能
#define IRDA_CODE_K4	0x0a//最后一排右边

#define IRDA_CODE_WALLWALK	0xf0//0x10//0x90//沿墙清扫 d
#define IRDA_CODE_OK	0x90//OK 与沿边重码了,这个按键取消
#define IRDA_CODE_SCHDL	IRDA_CODE_INVALID//预约,无此功能
#define IRDA_CODE_MUTE	0x80//0xf0

#define IRDA_CODE_WIFI	0xe9 //无此功能

#define IRDA_CODE_SCUR	0xd9//IRDA_CODE_WIFI

#define IRDA_CODE_NAVI	0x10//弓字形清扫

#elif ICLEAN_IRCTRL_TYPE == 4//便宜货的遥控器,只有9个按键

#define IRDA_DEV_ADDR_ENABLE	1
#define IRDA_DEV_ADDR	0x00

#define IRDA_CODE_INVALID	0xff//无效码

#define IRDA_CODE_POWER	0x70//电源 d
#define IRDA_CODE_SPOT	0x50//集中清扫 d
#define IRDA_CODE_AUTO	0x60//0x10//自动清扫 d
#define IRDA_CODE_UP	0x80//向上键
#define IRDA_CODE_LEFT	0x00//向左键
#define IRDA_CODE_RIGHT	0x40//向右键
#define IRDA_CODE_DOWN	0xA0//向下键

//#define IRDA_CODE_WALLWALK	0x90//0x10//0x90//沿墙清扫 d
#define IRDA_CODE_DOCK	0x90//0x10//0x90//沿墙清扫 d
#define IRDA_CODE_MUTE	0xf0//0xf0

#else
#define IRDA_CODE_INVALID	0xff//无效码

#define IRDA_CODE_POWER	0x00//电源 d
#define IRDA_CODE_MOP	0x92//抹布,无此功能
#define IRDA_CODE_SPOT	0xe8//集中清扫 d
#define IRDA_CODE_AUTO	0x10//自动清扫 d
#define IRDA_CODE_UP	0x70//向上键
#define IRDA_CODE_LEFT	0xc0//向左键
#define IRDA_CODE_RIGHT	0x18//向右键
#define IRDA_CODE_DOWN	0x60//向下键
#define IRDA_CODE_TIM	0x41//时间,无此功能
#define IRDA_CODE_DOCK	0xf2//回充
#define IRDA_CODE_K2	0x12//最后一排最左,无此功能
#define IRDA_CODE_K3	IRDA_CODE_INVALID//最后一排中间(现在无码),无此功能
#define IRDA_CODE_K4	0x0a//最后一排右边

#define IRDA_CODE_SCHDL	0xf0//预约 与静音键同位,菲果有遥控器预约功能,屏在机器上
#define IRDA_CODE_WALLWALK	0x90//沿墙清扫 d,与OK键重码了,因为需要设置预约时间时此键无效,其他时间功能正常
#define IRDA_CODE_OK	0x50//OK 与沿边重码了,这个按键取消
#define IRDA_CODE_MUTE	0x80


#define IRDA_CODE_WIFI	0xe9 //无此功能

#define IRDA_CODE_SCUR	0xd9//IRDA_CODE_WIFI
#endif

#else
#define IRDA_CODE_AUTO	0xe0
#define IRDA_CODE_CHARGE	0xe1
#define IRDA_CODE_MUTE	0xe2
#define IRDA_CODE_SETTIME	0xe3
#define IRDA_CODE_SETSCH	0xe4
#define IRDA_CODE_POWER	0xe5
#define IRDA_CODE_WALLWALK	0xe6
#define IRDA_CODE_SPOT	0xe7
#define IRDA_CODE_WIFI	0xe8
#define IRDA_CODE_SCUR	0xd9//IRDA_CODE_WIFI
#endif
/*
PD12	TO-触摸IC（开始）
PE0	TO-触摸IC（模式）
PE1	TO-触摸IC（回充）
*/
#if 1
//KEY3
#define KEY_DOCK_PORT	GPIOA
#define KEY_DOCK_PIN		GPIO_Pin_15
//KEY4
#define KEY_CLEAN_PORT	GPIOA
#define KEY_CLEAN_PIN	GPIO_Pin_8

//KEY1
#define KEY_WIFI_PORT	GPIOA
#define KEY_WIFI_PIN		GPIO_Pin_11
//KEY2
#define KEY_SPOT_PORT	GPIOA
#define KEY_SPOT_PIN		GPIO_Pin_12
/*

#define KEY_PALY_PORT 	 GPIOB
#define KEY_PALY_PIN	 GPIO_Pin_14

#define KEY_POWER_PORT 	 GPIOA
#define KEY_POWER_PIN	 GPIO_Pin_15
*/
//#define KEY_WIFI_PORT  GPIOA
//#define KEY_WIFI_PIN	 GPIO_Pin_8



#else
#define KEY_START_PORT 	 GPIOB
#define KEY_START_PIN	 GPIO_Pin_14

#define KEY_WIFI_PORT 	 GPIOA
#define KEY_WIFI_PIN	 GPIO_Pin_15

#define KEY_CHARGE_PORT 	 GPIOA
#define KEY_CHARGE_PIN	 GPIO_Pin_8

#define LED_START_PORT	GPIOA
#define LED_START_PIN	GPIO_Pin_11

#define LED_CHARGE_PORT	GPIOA
#define LED_CHARGE_PIN	GPIO_Pin_12
#endif
#define KEY_RD_WIFI()		(((KEY_WIFI_PORT->IDR) & KEY_WIFI_PIN) != KEY_WIFI_PIN)
#define KEY_RD_SPOT()	(((KEY_SPOT_PORT->IDR) & KEY_SPOT_PIN) != KEY_SPOT_PIN)
#define KEY_RD_DOCK()	(((KEY_DOCK_PORT->IDR) & KEY_DOCK_PIN) != KEY_DOCK_PIN)
#define KEY_RD_CLEAN()	(((KEY_CLEAN_PORT->IDR) & KEY_CLEAN_PIN) != KEY_CLEAN_PIN)
////#if (1 == SIMPLIFY)
////#define KEY_RD_SPOT()	(((KEY_CLEAN_PORT->IDR) & KEY_CLEAN_PIN) != KEY_CLEAN_PIN)
////#endif


#if(1)
#define LED1_OFF()		 GPIO_ResetBits(GPIOB,GPIO_Pin_14)     //0
#define LED1_ON()		 GPIO_SetBits(GPIOB,GPIO_Pin_14)      //spot
#define LED2_OFF()		 GPIO_ResetBits(GPIOD,GPIO_Pin_12)
#define LED2_ON()		 GPIO_SetBits(GPIOD,GPIO_Pin_12)		//wifi
#define LED3_OFF()		 GPIO_ResetBits(GPIOD,GPIO_Pin_3)
#define LED3_ON()		 GPIO_SetBits(GPIOD,GPIO_Pin_3)		//clean
#define LED4_OFF()		 GPIO_ResetBits(GPIOD,GPIO_Pin_4)
#define LED4_ON()		 GPIO_SetBits(GPIOD,GPIO_Pin_4)		//dock

#define AUTO_LAMP_ON()	GPIO_SetBits(GPIOE,GPIO_Pin_3)	//光敏电阻
#define AUTO_LAMP_OFF()	GPIO_ResetBits(GPIOE,GPIO_Pin_3)
#else
#if DP_PANEL_EN > 1
#define LED1_OFF()		 GPIO_SetBits(GPIOC,GPIO_Pin_10)
#define LED1_ON()		 GPIO_ResetBits(GPIOC,GPIO_Pin_10)
#define LED2_OFF()		 GPIO_SetBits(GPIOC,GPIO_Pin_11)
#define LED2_ON()		 GPIO_ResetBits(GPIOC,GPIO_Pin_11)
#else
#define LED1_OFF()		 GPIO_SetBits(GPIOA,GPIO_Pin_11)
#define LED1_ON()		 GPIO_ResetBits(GPIOA,GPIO_Pin_11)
#define LED2_OFF()		 GPIO_SetBits(GPIOA,GPIO_Pin_12)
#define LED2_ON()		 GPIO_ResetBits(GPIOA,GPIO_Pin_12)
#endif
#endif

#define LED_RUN_BLU_STATE_OFF				0		//红蓝全灭			关机
#define LED_RUN_BLU_STATE_TWINKLE			1		//红蓝交替闪烁		告警
#define LED_RUN_STATE_LONG_BRIGHT			2		//红灯长亮			运行
#define LED_BLUE_STATE_TWINKLE				3		//蓝灯长亮短灭闪烁		充电
#define LED_BLUE_STATE_FAST_TWINKLE			4		//蓝灯快闪			WIFI smart
#define LED_BLUE_STATE_SLOW_TWINKLE			5		//蓝灯慢闪			WIFI ap
#define LED_BLUE_STATE_LONG_BRIGHT			6		//蓝灯长亮			欠电压
#define LED_BLUE_STATE_OFF					7		//蓝灯长灭			无告警


void all_key_led_off(void);

void key_io_init(void);
//uint8_t led_state_out(uint8_t sys_state,uint8_t v_bat_percent);
uint8_t proc_key_task(uint8_t sys_state);
uint8_t proc_irctrl_task(uint8_t sys_state);
void proc_led_task(uint8_t sys_state,uint8_t v_bat_percent);

uint32_t kct_scur_getbytes(void);
uint8_t get_irda_key(void);
uint8_t get_irda_rep(void);
void ramote_pair_mode(void);
void proc_wifiled_task(void);
#endif
#endif

