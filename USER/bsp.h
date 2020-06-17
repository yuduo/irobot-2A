/**
  *************************************************************************
  *@FileName
  *@Author
  *@Version
  *@Date
  *@History
  *@Dependence
  *@Description
  *************************************************************************
  *@CopyRight
  *************************************************************************
  *
	*/

#ifndef _MCUPERIPH_INIT_H
#define _MCUPERIPH_INIT_H

#define SYS_DEBUG			1
#define HW_TEST				0
//#define PID_DEBUG			0
#define EN_PID				1
#define IRDA_TEST			1
#define CALC_TIMER2_OUT		1
#define BALANC_CALC		0
#define MOTOR_DEBUG		1
#define EN_START_PWM		0
#define TEST_SENSES			0
#define TEST_PID				0
#define CALC_R_MOTOR_SP	1		//计算右边点击速度
#define CALC_L_MOTOR_SP	1		//计算左边电机速度
#define EN_LOAD_DTC		0		//是否计算负载过重


//管教定义
#define PIN_CHARGE		GPIO_Pin_11
#define PORT_CHARGE		GPIOD

#define PIN_AC		GPIO_Pin_13
#define PORT_AC		GPIOB             //PD0

#define PIN_BUM_DET		 GPIO_Pin_1
#define PORT_BUM_DET	 GPIOD
#define READ_BUM_DET()	GPIO_ReadInputDataBit( PIN_BUM_DET	, PORT_BUM_DET)

//PE13	左边刷电流检测（H/L电平检测）	输入	普通IO口
//-----------------------------------------------------------------modified by wonton2004 20161228
#if 0
#define PORT_SIDE_MOTOR_L		GPIOE
#define PIN_SIDE_MOTOR_L		0
#define READ_SIDE_MOTOR_L()		0//GPIO_ReadInputDataBit(PORT_SIDE_MOTOR_L, PIN_SIDE_MOTOR_L)
#else
#define PORT_SIDE_MOTOR_L		GPIOE
#define PIN_SIDE_MOTOR_L		GPIO_Pin_15
#define READ_SIDE_MOTOR_L()		GPIO_ReadInputDataBit(PORT_SIDE_MOTOR_L, PIN_SIDE_MOTOR_L)
#endif
#if DP_PANEL_EN >= 4
#define I_SMTL_CUR_MAX	1200
#endif
//---------------------------------------------------------------------------------------------end
//----------------------------------------------------------------------add by wonton2004 20170117
#if DP_PANEL_EN > 1
#define I_MID_VALUE_MAX	650
#define READ_MAIN_MOTOR()	I_MID_ADC()
#else
#define PORT_MAIN_MOTOR		GPIOB
#define PIN_MAIN_MOTOR		GPIO_Pin_12
#define READ_MAIN_MOTOR()		GPIO_ReadInputDataBit(PORT_MAIN_MOTOR, PIN_MAIN_MOTOR)
#endif
//---------------------------------------------------------------------------------------------end

//PE9	右边刷过流检测（H/L电平检测）	输入	普通IO口
////#define PORT_SIDE_MOTOR_R		GPIOE
////#define PIN_SIDE_MOTOR_R		GPIO_Pin_9
////#define READ_SIDE_MOTOR_R()		GPIO_ReadInputDataBit(PORT_SIDE_MOTOR_R, PIN_SIDE_MOTOR_R)
#if DP_PANEL_EN >= 4
#define I_SMTR_CUR_MAX	1200
#endif

//PB13	是吸尘电机的过流检测（H/L电平检测）	输入	普通IO口
#define PORT_DUST_MOTOR_PWM	GPIOB
#define PIN_DUST_MOTOR_PWM		GPIO_Pin_2
#define PORT_DUST_MOTOR_SPD	GPIOC
#define PIN_DUST_MOTOR_SPD		GPIO_Pin_13
#if(1)
#define DUST_MOTOR_OFF()			{GPIO_ResetBits(PORT_DUST_MOTOR_PWM,PIN_DUST_MOTOR_PWM);GPIO_ResetBits(PORT_DUST_MOTOR_SPD,PIN_DUST_MOTOR_SPD);/*log_printf("DUST_MOTOR_OFF\r\n");*/}
#if DUST_MOTOR_SAME_2B
#define DUST_MOTOR_RANK1()		{GPIO_SetBits(PORT_DUST_MOTOR_PWM,PIN_DUST_MOTOR_PWM);GPIO_ResetBits(PORT_DUST_MOTOR_SPD,PIN_DUST_MOTOR_SPD);sys->dust_level = 1;}
#define DUST_MOTOR_RANK2()		{GPIO_ResetBits(PORT_DUST_MOTOR_PWM,PIN_DUST_MOTOR_PWM);GPIO_SetBits(PORT_DUST_MOTOR_SPD,PIN_DUST_MOTOR_SPD);sys->dust_level = 2;}
#else
#define DUST_MOTOR_RANK2()		{GPIO_SetBits(PORT_DUST_MOTOR_PWM,PIN_DUST_MOTOR_PWM);GPIO_ResetBits(PORT_DUST_MOTOR_SPD,PIN_DUST_MOTOR_SPD);sys->dust_level = 2;}
#define DUST_MOTOR_RANK1()		{GPIO_ResetBits(PORT_DUST_MOTOR_PWM,PIN_DUST_MOTOR_PWM);GPIO_SetBits(PORT_DUST_MOTOR_SPD,PIN_DUST_MOTOR_SPD);sys->dust_level = 1;}

#endif
#define DUST_MOTOR_RANK3()		{GPIO_SetBits(PORT_DUST_MOTOR_PWM,PIN_DUST_MOTOR_PWM);GPIO_SetBits(PORT_DUST_MOTOR_SPD,PIN_DUST_MOTOR_SPD);sys->dust_level = 3;}
#else
#define PORT_DUST_MOTOR			//GPIOB
#define PIN_DUST_MOTOR			//GPIO_Pin_13
#define READ_DUST_MOTOR()		GPIO_ReadInputDataBit(PORT_DUST_MOTOR, PIN_DUST_MOTOR)
#endif

//PE2	MOTOR电源开关（H/L电平出输）
#define PORT_MOTOR_POWER			GPIOE
#define PIN_MOTOR_POWER				GPIO_Pin_2

//micony201710
#define MOTOR_POWER_ON()			{GPIO_SetBits(PORT_MOTOR_POWER, PIN_MOTOR_POWER);sys->motor_power_off = 0;}
#define MOTOR_POWER_OFF()			{GPIO_ResetBits(PORT_MOTOR_POWER, PIN_MOTOR_POWER);sys->motor_power_off = 1;log_printf("MOTOR_POWER_OFF=%d\r\n",sys->motor_power_off);}
#define MOTOR_POWER_OFF_NPRI()			{GPIO_ResetBits(PORT_MOTOR_POWER, PIN_MOTOR_POWER);sys->motor_power_off = 1;}

#define TEST_MOTOR_POWER_ON()			{GPIO_SetBits(PORT_MOTOR_POWER, PIN_MOTOR_POWER);sys->motor_power_off = 0;}
#define TEST_MOTOR_POWER_OFF()			{GPIO_ResetBits(PORT_MOTOR_POWER, PIN_MOTOR_POWER);sys->motor_power_off = 1;}
#define TEST_MOTOR_POWER_OFF_NPRI()		{GPIO_ResetBits(PORT_MOTOR_POWER, PIN_MOTOR_POWER);sys->motor_power_off = 1;}
#define TEST_MOTOR_CTRL(L,R,M,D)			{TEST_MOTOR_POWER_ON(); SET_DST_MOTER(D);delay_ms(5);	SET_LSIDE_MOTER(L); delay_ms(10);	SET_RSIDE_MOTER(R); delay_ms(10);SET_MID_MOTER(M);}
#if(1)
#define MOTOR_CTRL1(L,R,M,D)			{MOTOR_POWER_OFF(); delay_ms(2);MOTOR_POWER_ON(); delay_ms(4);	SET_LSIDE_MOTER(999);	SET_RSIDE_MOTER(999);	SET_MID_MOTER(0);	DUST_MOTOR_OFF();log_printf("MOTOR_POWER_OFF 12\r\n");}/*SET_LSIDE_MOTER(999);	SET_RSIDE_MOTER(999);	SET_MID_MOTER(999);	SET_DST_MOTER(999);}	*/
#define MOTOR_CTRL(L,R,M,D)			    {MOTOR_POWER_OFF(); delay_ms(10);DUST_MOTOR_RANK1();delay_ms(5);MOTOR_POWER_ON(); delay_ms(4);	SET_LSIDE_MOTER(L); delay_ms(10);	SET_RSIDE_MOTER(R); delay_ms(10);SET_MID_MOTER(M);log_printf("MOTOR_POWER_OFF 11\r\n");}
#define MOTOR_CTRL2(L,R,M,D)			{MOTOR_POWER_OFF(); delay_ms(10);DUST_MOTOR_RANK1();delay_ms(5);MOTOR_POWER_ON(); delay_ms(4);	SET_LSIDE_MOTER(L); delay_ms(5);	SET_RSIDE_MOTER(R); delay_ms(5);SET_MID_MOTER(M);log_printf("MOTOR_POWER_OFF 10\r\n");}
#define MOTOR_CTRL_SIDE(L,R)			{MOTOR_POWER_OFF(); delay_ms(10);MOTOR_POWER_ON(); delay_ms(4);	SET_LSIDE_MOTER(L);	SET_RSIDE_MOTER(R);	SET_MID_MOTER(0);	DUST_MOTOR_OFF();log_printf("MOTOR_POWER_OFF 9\r\n");}
#else
//#if 1
#define MOTOR_CTRL1(L,R,M,D)			{MOTOR_POWER_OFF(); delay_ms(2);MOTOR_POWER_ON(); delay_ms(4);	SET_LSIDE_MOTER(999);	SET_RSIDE_MOTER(999);	SET_MID_MOTER(0);	SET_DST_MOTER(0);}/*SET_LSIDE_MOTER(999);	SET_RSIDE_MOTER(999);	SET_MID_MOTER(999);	SET_DST_MOTER(999);}	*/
//#else
#define MOTOR_CTRL(L,R,M,D)			    {MOTOR_POWER_OFF(); delay_ms(10);SET_DST_MOTER(D);delay_ms(5);MOTOR_POWER_ON(); delay_ms(4);	SET_LSIDE_MOTER(L); delay_ms(10);	SET_RSIDE_MOTER(R); delay_ms(10);SET_MID_MOTER(M);}
#define MOTOR_CTRL2(L,R,M,D)			{MOTOR_POWER_OFF(); delay_ms(10);SET_DST_MOTER(D);delay_ms(5);MOTOR_POWER_ON(); delay_ms(4);	SET_LSIDE_MOTER(L); delay_ms(5);	SET_RSIDE_MOTER(R); delay_ms(5);SET_MID_MOTER(M);}
#define MOTOR_CTRL_SIDE(L,R)			{MOTOR_POWER_OFF(); delay_ms(10);MOTOR_POWER_ON(); delay_ms(4);	SET_LSIDE_MOTER(L);	SET_RSIDE_MOTER(R);	SET_MID_MOTER(0);	SET_DST_MOTER(0);}
#endif
//#endif
//#define STOP_ALL_MOTOR()			{ /*SET_DST_MOTER(0);*/MOTOR_POWER_OFF();	/*SET_LSIDE_MOTER(1000);	SET_RSIDE_MOTER(1000);*/	SET_MID_MOTER(0);	delay_ms(4);	SET_DST_MOTER(0); log_printf("MOTOR_POWER_OFF 8\r\n"); }
#define STOP_ALL_MOTOR()			{ MOTOR_POWER_OFF();	SET_LSIDE_MOTER(1000);	SET_RSIDE_MOTER(1000);		SET_MID_MOTER(0);delay_ms(4);SET_DST_MOTER(0);	 log_printf("MOTOR_POWER_OFF 8\r\n"); }

#define PORT_VOID_POWER				GPIOE
#define PIN_VOID_POWER				GPIO_Pin_11

//#define VOID_POWER_ON()				GPIO_SetBits(PORT_VOID_POWER, PIN_VOID_POWER)//GPIO_ResetBits(PORT_VOID_POWER, PIN_VOID_POWER)
#define VOID_POWER_ON()				//GPIO_ResetBits(PORT_VOID_POWER, PIN_VOID_POWER)
#define VOID_POWER_OFF()			//GPIO_SetBits(PORT_VOID_POWER, PIN_VOID_POWER)

//================对地远近红外==========================
#define PORT_NEAR_LAN_IR			GPIOE
#define PIN_NEAR_LAN_IR				GPIO_Pin_12

#define PORT_FAR_LAN_IR				GPIOE
#define PIN_FAR_LAN_IR				GPIO_Pin_6

#define NEAR_LAN_ON()				//GPIO_SetBits(PORT_NEAR_LAN_IR, PIN_NEAR_LAN_IR)
#define NEAR_LAN_OFF()				//GPIO_ResetBits(PORT_NEAR_LAN_IR, PIN_NEAR_LAN_IR)

#define FAR_LAN_ON()				//GPIO_SetBits(PORT_FAR_LAN_IR, PIN_FAR_LAN_IR)
#define FAR_LAN_OFF()				//GPIO_ResetBits(PORT_FAR_LAN_IR, PIN_FAR_LAN_IR)





//----------------------------------------------------------------------add by wonton2004 20170115
//各检测杂项,读取信号均返回1时为信号被触发

//虚拟墙,低电平为虚拟墙
#define	PIN_VWALL_DET	GPIO_Pin_7
#define PORT_VWALL_DET	GPIOD
#define READ_VWALL_DET()	((PORT_VWALL_DET->IDR & PIN_VWALL_DET) == 0)

//尘盒检测,低电平为尘盒接入
#define PIN_DUSTBOX_DET	GPIO_Pin_5
#define PORT_DUSTBOX_DET	GPIOB
#define READ_DUSTBOX_DET()	((PORT_DUSTBOX_DET->IDR & PIN_DUSTBOX_DET) != 0)

//灰尘检测(灰尘大的时候红外被挡住,输出高电平)
////#define PIN_DUST_DET	GPIO_Pin_6
////#define PORT_DUST_DET	GPIOC
#define READ_DUST_DET()	0////((PORT_DUST_DET->IDR & PIN_DUST_DET) != 0)

//水箱(抹布)检测，高电平为抹布接入
#define PIN_MOP_DET		//GPIO_Pin_4
#define PORT_MOP_DET	//GPIOD
#define READ_MOP_DET()	0//((PORT_MOP_DET->IDR & PIN_MOP_DET) != 0)

//吸嘴切换检测(低电平时切换为吸嘴,中扫关闭),低电平为吸嘴接入
#define PIN_AIRTAKE_DET	GPIO_Pin_12
#define PORT_AIRTAKE_DET	GPIOD
#define READ_AIRTAKE_DET()	((PORT_AIRTAKE_DET->IDR & PIN_AIRTAKE_DET) == 0)

//电源开关是否打开的检测管脚
#define PIN_PSW			GPIO_Pin_10
#define PORT_PSW		GPIOD
#define READ_PSW_DET()	(0)//((PORT_PSW->IDR & PIN_PSW) == 0)




//--------------------------------------------------------------------------------------------end



/*
PE12	高门槛选择（H/L电平出输）
PE6	低门槛选择（H/L电平出输）
*/
#define PORT_IRD_MK_H				GPIOE
#define PIN_IRD_MK_H				GPIO_Pin_12

#define PORT_IRD_MK_L				GPIOE
#define PIN_IRD_MK_L				GPIO_Pin_6

#define SELECT_MK_H()				{GPIO_SetBits(PORT_IRD_MK_H, PIN_IRD_MK_H);GPIO_ResetBits(PORT_IRD_MK_L, PIN_IRD_MK_L);}
#define SELECT_MK_L()				{GPIO_SetBits(PORT_IRD_MK_L, PIN_IRD_MK_L);GPIO_ResetBits(PORT_IRD_MK_H, PIN_IRD_MK_H);}


//测距红外的电源开关脚
#if DP_PANEL_EN > 1
#define PORT_FIRPWR_MID				GPIOA
#define PIN_FIRPWR_MID				GPIO_Pin_12
#define PORT_FIRPWR_L1R1				GPIOA
#define PIN_FIRPWR_L1R1				GPIO_Pin_11
#elif DP_PANEL_EN == 0
#define PORT_FIRPWR_MID				GPIOC
#define PIN_FIRPWR_MID				GPIO_Pin_11
#define PORT_FIRPWR_L1R1				GPIOC
#define PIN_FIRPWR_L1R1				GPIO_Pin_12
#else
#define PORT_FIRPWR_MID				GPIOC
#define PIN_FIRPWR_MID				0
#define PORT_FIRPWR_L1R1				GPIOC
#define PIN_FIRPWR_L1R1				0
#endif

#if FRONT_IR_VERSION_1
#define FIRPWR_MID_ON()				GPIO_SetBits(PORT_FIRPWR_MID, PIN_FIRPWR_MID)
#define FIRPWR_MID_OFF()				GPIO_ResetBits(PORT_FIRPWR_MID, PIN_FIRPWR_MID)

#define FIRPWR_L1R1_ON()				GPIO_SetBits(PORT_FIRPWR_L1R1, PIN_FIRPWR_L1R1)
#define FIRPWR_L1R1_OFF()			GPIO_ResetBits(PORT_FIRPWR_L1R1, PIN_FIRPWR_L1R1)
#else
#define FIRPWR_MID_OFF()				GPIO_SetBits(PORT_FIRPWR_MID, PIN_FIRPWR_MID)
#define FIRPWR_MID_ON()				GPIO_ResetBits(PORT_FIRPWR_MID, PIN_FIRPWR_MID)

#define FIRPWR_L1R1_OFF()			GPIO_SetBits(PORT_FIRPWR_L1R1, PIN_FIRPWR_L1R1)
#define FIRPWR_L1R1_ON()				GPIO_ResetBits(PORT_FIRPWR_L1R1, PIN_FIRPWR_L1R1)
#endif
/*
	PB2	左后轮凌空检测（H/L电平检测）	输入
	PB12	右后轮电机凌空检测（H/L电平检测）	输入

*/
#if(1 == VER_BORD_2)
///#define PORT_LEFT_MOTOR_LEAVE				GPIOB
//#define PIN_LEFT_MOTOR_LEAVE				GPIO_Pin_2

//#define PORT_RIGHT_MOTOR_LEAVE				GPIOB
//#define PIN_RIGHT_MOTOR_LEAVE				GPIO_Pin_12

#else
#define PORT_LEFT_MOTOR_LEAVE				GPIOE
#define PIN_LEFT_MOTOR_LEAVE				GPIO_Pin_10

#if DP_PANEL_EN > 1
#define PORT_RIGHT_MOTOR_LEAVE				GPIOB
#define PIN_RIGHT_MOTOR_LEAVE				GPIO_Pin_12
#else
#define PORT_RIGHT_MOTOR_LEAVE				GPIOB//GPIOC
#define PIN_RIGHT_MOTOR_LEAVE				GPIO_Pin_12//GPIO_Pin_4
#endif

#endif

#define PORT_MA5601_POWER				GPIOD
#define PIN_MA5601_POWER				GPIO_Pin_15


//前撞红外的控制线
#define PORT_IR_CTRL						GPIOC
#define PIN_IR_CTRL						GPIO_Pin_9

//前撞右侧红外的控制线
#define PORT_IR_CTRL2						//GPIOE//GPIOE					//陀螺仪版本为PE3,摄像头版本为PC9
#define PIN_IR_CTRL2						//GPIO_Pin_3//GPIO_Pin_3		//




//后轮凌空检测
#define LEFT_MOTOR_LEAVE()					GPIO_ReadInputDataBit(PORT_LEFT_MOTOR_LEAVE, PIN_LEFT_MOTOR_LEAVE)
#define RIGHT_MOTOR_LEAVE()					GPIO_ReadInputDataBit(PORT_RIGHT_MOTOR_LEAVE, PIN_RIGHT_MOTOR_LEAVE)


//PB12	中扫电机的过流检测（H/L电平检测）	ADC	ADC12_IN9
/*
#define PORT_MID_MOTOR			GPIOB
#define PIN_MID_MOTOR			GPIO_Pin_12
#define READ_MID_MOTOR()		GPIO_ReadInputDataBit(PORT_MID_MOTOR, PIN_MID_MOTOR)

*/


#define EXTERAL_AC_DETECT()	GPIO_ReadInputDataBit(PORT_AC, PIN_AC)
#define DOCK_DETECT()		GPIO_ReadInputDataBit(PORT_CHARGE, PIN_CHARGE)

//IC 正在充电检测
#define CHARGE_ING_PIN						GPIO_Pin_6
#define CHARGE_ING_PORT						GPIOC
#define CHARGE_ING_DETECT()					GPIO_ReadInputDataBit(CHARGE_ING_PORT, CHARGE_ING_PIN)

//IC 充电完成检测
#define CHARGE_DONE_PIN						GPIO_Pin_7
#define CHARGE_DONE_PORT					GPIOC
#define CHARGE_DONE_DETECT()				GPIO_ReadInputDataBit(CHARGE_DONE_PORT, CHARGE_DONE_PIN)


void Periph_Init(void);
void Delayus(int t);
void TargetSysReset(void);
void RTC_Init(void);


#endif

