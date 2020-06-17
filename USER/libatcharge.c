
/***********************************************************************************************
14.8V 4节锂电池充电程序

同平台移植说明
1、PID调用周期为10ms
2、需要修改的函数或宏
	a 采样函数或采样通道
		采样通道:LIBAT_CHARGECURRENT_CHNO/LIBAT_CHARGEVOLT_CHNO/LIBAT_LIBATTEMPER_CHNO
		采样函数:LiBat_GetBatTemper()/LiBat_GetBatVolt()/LiBat_GetChargeCurrent()
	b 电机的电源开关/测距及对地红外的电源开关
		LIBAT_MOTOR_PWR_PORT/LIBAT_MOTOR_PWR_PIN/LIBAT_IRDIST_PWR_PORT/LIBAT_IRDIST_PWR_PIN/
		涉及函数 LiBat_TurnOnOtherPwr()
	c 充电PWM的TIM及其输出通道
		充电IO及PWM LIBAT_CHPWM_PORT/LIBAT_CHPWM_PIN/LIBAT_CHPWM_TIM/LIBAT_CHPWM_CH
3、调用
	初始化LiBat_HalInit()
	充电模式退出LiBat_ExitChargeMode()

	充电功能函数周期性调用 LiBat_CurrentPid()
				一般调用方式:LiBat_CurrentPid(500,10); //预调500mA PID周期内10个电流采样平均
***********************************************************************************************/


/***********************************************************************************************
底层驱动接口 INCLUDE
***********************************************************************************************/
#include "sys.h"
/*
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "system_conf.h"
#include "portmacro.h"
#include "detect.h"
#include "stm32f10x.h"

#include "detect.h"
#include "libatcharge.h"
*/
//#include "ADC.h"
//#include "pwms.h"
#include "TempTrans.h"
#include "test_mode.h"

/***********************************************************************************************
充电策略相关变量
***********************************************************************************************/
static int16_t PwmOutOld = 0;//上一次的PWM输出值(这本来是LiBat_CurrentPid的局部变量,但在掉座的
							//情况下,此变量需要清0,否则会发生过流,所以改作全局变量,在initial中清0)
/***********************************************************************************************
end 充电策略相关变量
***********************************************************************************************/
/***********************************************************************************************
end 底层驱动接口 INCLUDE
***********************************************************************************************/

/***********************************************************************************************
PID相关变量
***********************************************************************************************/
typedef struct _libat_pid{
	int16_t SetPoint;// 设定目标Desired value
	int16_t Proportion;// 比例常数Proportional Const
	int16_t Integral;// 积分常数Integral Const
	int16_t Derivative;// 微分常数Derivative Const
	int16_t LastError;// Error[t - 1]
	int16_t PrevError;// Error[t - 2]
	int16_t SumError;  // Sums of Errors
} LB_PID;


static LB_PID Pid_Current,Pid_Volt;
static int16_t ErrorBufferCurrent[16];
static int16_t ErrorBufferVolt[16];
static uint8_t ErrorBufferCurrentCt;//,ErrorBufferVoltCt;//差值缓存中最新差值所在的位置
//锂电池容量表
/*
int libat_cap[28]={ 	16600,16560,16480,16400,16320,16200,16120,
						15880,15720,15600,15480,15360,15240,15160,
						15080,15040,14960,14920,14880,14840,14760,
						14640,14600,14560,14520,14440,14360,14320};
*/

/*
	15200		75%以上  三格
	14200		50%以上	 两格
	13700		25%下	 闪
	*/
/*
	15200		66%以上  三格
	14200		31%以上	 两格
	14200		10%以上	 一格
	13500		10%下	 闪
	*/
#if CAPACITY_4400_VER
int libat_cap[54]={ 	16050,16000,15900,15850,15800,15750,15700,		//100-88
						15650,15600,15500,15400,15450,15400,15350,		//87-75
						15300,15100,15050,15000,14950,14890,14860,		//73-62
						14830,14750,14690,14630,14570,14510,14450,		//61-50
						14400,14370,14340,14310,14280,					//49-41
						14250,14150,14135,14115,14100,14050,13950,		//39-20
						13930,13910,13890,13850,13830,13800,13740,		//19-10
						13700,13600,13500,13400,13300,13200,13100};		///9-0

int libat_percent[54] ={100,98,96,94,92,90,88,
							87,85,83,81,79,77,75,
							73,71,69,67,65,63,62,
							61,60,58,56,54,52,50,
							49,47,45,43,41,
							39,33,32,31,30,25,20,
							19,18,17,15,14,12,10,
							 9, 8, 6, 5, 3, 1, 0 };

#else
int libat_cap[21]={			16600,	16410,	16220,	16030,	15840,	
								15650,	15460,	15270,	15080,	14890,	
								14700,	14510,	14320,	14130,	13940,	
								13750,	13560,	13370,	13180,	12990,	
								12800						};

int libat_percent[21] ={	100, 	95,		90,		85,		80,	
								75,		70,		65,		60,		55,	
								50,		45,		40,		35,		30,	
								25,		20,		15,		10,		5,	
								0							};	

#endif

#if CAPACITY_4400_VER
#define	LIBAT_CAP_NUB	54
#else
#define	LIBAT_CAP_NUB	49
#endif
#define BTY_FULL_VOLT		16600
#define BTY_EMPT_VOLT		12800

int get_libat_percent(int volt)
{
#if 1
		static uint8_t last=0,last_i[5]={0},change_i_c=0,first_in=1;
		uint8_t i;
		#if 0 //查表  0,5,10,15
		for(i=0;i<21;i++)
		{
			if(volt >=libat_cap[i])
				break;
		}
		#else//线性计算	(16600-12800)/100 = 38		
		if(volt>BTY_FULL_VOLT)
			volt = BTY_FULL_VOLT;
		if(volt<BTY_EMPT_VOLT)
			volt = BTY_EMPT_VOLT;
		i = (volt-BTY_EMPT_VOLT)/38;		
		#endif
		//log_printf("get_libat_percent:i:%d\r\n",i);
		if(first_in)
		{
			first_in = 0;
			change_i_c=0;
			last = i;
		}
		else
		{
			if(i != last)	//防止波动
			{		
				last_i[change_i_c] = i;
				if(change_i_c++>=3)
				{
					i = (last_i[0]+last_i[1]+last_i[2])/3;					
					change_i_c = 0;
					last=i;
					//log_printf("1i:%dlast(%d,%d,%d)(%d,%d,%d)per%d\r\n",i,last_i[0],last_i[1],last_i[2],libat_cap[last_i[0]],libat_cap[last_i[1]],libat_cap[last_i[2]],libat_percent[i]);
				}
				else
				{
					i = last;		//没到3次还取上一次的
					//log_printf("2i:%dlast(last%d)per%d\r\n",i,last,libat_percent[i]);
				}	
			}
			else
			{				
				change_i_c = 0;
				//log_printf("3i:%d,per%d\r\n",i,libat_percent[i]);
			}	
		}
		//log_printf("volt=%d,%d\r\n",volt,libat_percent[i]);
#if 0		
		i = i > 20 ? 20:i;		
		return libat_percent[i];	
#else
		return i;
#endif
#else
		int i;
		for(i=0;i<21;i++)
		{
			if(volt >=libat_cap[i])
				break;
		}
		//log_printf("volt=%d,%d\r\n",volt,libat_percent[i]);
		i = i > 20 ? 20:i;
		return libat_percent[i];
#endif	

}
/***********************************************************************************************
end PID相关变量
***********************************************************************************************/


/***********************************************************************************************
底层驱动接口
***********************************************************************************************/

//是否在充电座上1为检测到充电座,0为未检测到
static uint8_t LiBat_isOnDock(void)
{
	//if(NO_CHARGE_DETECTED!=dc_socket_detect())
	return 1;
	//	return 0;
}

//初始化充电PWM输出
//GPIO PD12 TIM4.1
//PWM最大值4095,不分频
//充电PWM初始化,由于TIMER与电机复用,但是如果把TIMER重新初始化,会发生过流且过很久才会进入正常充电甚至不会
//恢复的现象,所以在这里直接使用的原电机的TIMER设置,频率为18K,比充电PWM频率要低,但目前不影响使用
void LiBat_InitPwrChargePwm(void)
{

//	TIM_TimeBaseStructure.TIM_Period = 0xff;
//	TIM_TimeBaseStructure.TIM_Prescaler = 9;//36-1;
	GPIO_InitTypeDef GPIO_InitStructure;
	//TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;

 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1 | RCC_APB2Periph_GPIOE  | RCC_APB2Periph_AFIO, ENABLE);  //使能GPIO外设和AFIO复用功能模块时钟

	GPIO_PinRemapConfig(GPIO_FullRemap_TIM1, ENABLE); //Timer1完全重映射  TIM1_CH1->E9
#if 0
	TIM_Cmd(TIM1, DISABLE);  //使能TIM1
	TIM_DeInit(TIM1);//初始化TIM1寄存器
#endif
 	LIBAT_CHPWM_PORT->ODR &= ~LIBAT_CHPWM_PIN;
       //设置该引脚为复用输出功能,输出TIM1 CH1的PWM脉冲波形	GPIOE.9
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 ; //TIM1_CH1
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  //复用推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOE, &GPIO_InitStructure);//初始化GPIO

 	LIBAT_CHPWM_PORT->ODR &= ~LIBAT_CHPWM_PIN;
	// GPIO_SetBits(GPIOE,GPIO_Pin_9);
	// GPIO_SetBits(GPIOE,GPIO_Pin_14);
	// while(1);
 #if 1
 #if 0
       //初始化TIM1
	TIM_TimeBaseStructure.TIM_Period = 0xff; //设置在下一个更新事件装入活动的自动重装载寄存器周期的值
	TIM_TimeBaseStructure.TIM_Prescaler =9; //设置用来作为TIMx时钟频率除数的预分频值
	TIM_TimeBaseStructure.TIM_ClockDivision = 0; //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
	TIM_TimeBaseStructure.TIM_RepetitionCounter = 0x0;
	TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure); //根据TIM_TimeBaseInitStruct中指定的参数初始化TIMx的时间基数单位
#endif

	//初始化TIM1 Channel1 PWM模式
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1; //选择定时器模式:TIM脉冲宽度调制模式1
 	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; //比较输出使能
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;//TIM_OCPolarity_High; //输出极性:TIM输出比较极性高
	TIM_OCInitStructure.TIM_Pulse =1000;
	TIM_OC3Init(TIM1, &TIM_OCInitStructure);  //根据T指定的参数初始化外设TIM1 OC1

	TIM_OC3PreloadConfig(TIM1, TIM_OCPreload_Enable);  //使能TIM1在CCR1上的预装载寄存器

#if 0
 	TIM_ARRPreloadConfig(TIM1, ENABLE);                //使能TIMx在ARR上的预装载寄存器
	//TIM_ForcedOC3Config(TIM1, TIM_ForcedAction_InActive);
	TIM_Cmd(TIM1, ENABLE);  //使能TIM1
	TIM_CtrlPWMOutputs(TIM1, ENABLE);//设置PWM主输出
#endif
 	TIM1->CCR3 = 1000;
 	//TIM_ForcedOC3Config(TIM1, TIM_ForcedAction_Active);
	#endif
#if 0

	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;

	TIM_DeInit(TIM1);//初始化TIM1寄存器
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1 | RCC_APB2Periph_GPIOE  | RCC_APB2Periph_AFIO, ENABLE);  //使能GPIO外设和AFIO复用功能模块时钟

	GPIO_PinRemapConfig(GPIO_FullRemap_TIM1, ENABLE); //Timer1完全重映射  TIM1_CH1->E9

	 GPIO_PinAFConfig(GPIOE,GPIO_PinSource8,GPIO_AF_TIM1);

      //设置该引脚为复用输出功能,输出TIM1 CH1的PWM脉冲波形	GPIOE.9
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 ; //TIM1_CH1
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  //复用推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOE, &GPIO_InitStructure);//初始化GPIO

	// GPIO_SetBits(GPIOE,GPIO_Pin_9);
	// GPIO_SetBits(GPIOE,GPIO_Pin_14);
	// while(1);

       //初始化TIM1
	TIM_TimeBaseStructure.TIM_Period = 0xff; //设置在下一个更新事件装入活动的自动重装载寄存器周期的值
	TIM_TimeBaseStructure.TIM_Prescaler =9; //设置用来作为TIMx时钟频率除数的预分频值
	TIM_TimeBaseStructure.TIM_ClockDivision = 0; //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
	TIM_TimeBaseStructure.TIM_RepetitionCounter = 0x0;
	TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure); //根据TIM_TimeBaseInitStruct中指定的参数初始化TIMx的时间基数单位

	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Enable;
	TIM_OCInitStructure.TIM_Pulse = ccr1;
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
	TIM_OCInitStructure.TIM_OCNPolarity = TIM_OCNPolarity_Low;//输出同相，TIM_OCNPolarity_High时输出反相
	TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Set;
	TIM_OCInitStructure.TIM_OCNIdleState = TIM_OCNIdleState_Reset;

	TIM_OC1Init(TIM1,&TIM_OCInitStructure);

	TIM_OCInitStructure.TIM_Pulse = 100;
	TIM_OC1Init(TIM1,&TIM_OCInitStructure);
/*
	TIM_OCInitStructure.TIM_Pulse = ccr3;
	TIM_OC3Init(TIM1,&TIM_OCInitStructure);

	TIM_OCInitStructure.TIM_Pulse = ccr4;
	TIM_OC4Init(TIM1,&TIM_OCInitStructure);
*/
	TIM_Cmd(TIM1,ENABLE);
	TIM_CtrlPWMOutputs(TIM1,ENABLE);
#endif


}

//设置充电PWM的输出值,0-4095
void LiBat_SetPwrChargePwm(uint16_t pwm)
{
	TIM1->CCR3 =  /*255 -*/ 1000 - (pwm << 2);
//	PRINTF("%d:%d\r\n",pwm,TIM1->CCR3);
/*
#if LIBAT_CHPWM_CH == 1
	TIM_SetCompare1(LIBAT_CHPWM_TIM, (pwm));
#elif LIBAT_CHPWM_CH == 2
	TIM_SetCompare2(LIBAT_CHPWM_TIM, (pwm));
#elif LIBAT_CHPWM_CH == 3
	TIM_SetCompare3(LIBAT_CHPWM_TIM, (pwm));
#elif LIBAT_CHPWM_CH == 4
	TIM_SetCompare4(LIBAT_CHPWM_TIM, (pwm));
#endif
*/
}
//设置充电PWM的输出值,0-4095
uint16_t LiBat_GetPwrChargePwm(void)
{
	return (1000 - TIM1->CCR3) >> 2;
//	PRINTF("%d:%d\r\n",pwm,TIM1->CCR3);
/*
#if LIBAT_CHPWM_CH == 1
	TIM_SetCompare1(LIBAT_CHPWM_TIM, (pwm));
#elif LIBAT_CHPWM_CH == 2
	TIM_SetCompare2(LIBAT_CHPWM_TIM, (pwm));
#elif LIBAT_CHPWM_CH == 3
	TIM_SetCompare3(LIBAT_CHPWM_TIM, (pwm));
#elif LIBAT_CHPWM_CH == 4
	TIM_SetCompare4(LIBAT_CHPWM_TIM, (pwm));
#endif
*/
}



/*----------------------------------------------------------------------------------------------
-- 初始化ADC,如果有其他地方已对此中相关模块进行初始化，则此函数留空
-- 充电电流采样PB0 ADC8
-- 充电电压采样PA7 ADC7
-- 电池温度采样PC5 ADC15
----------------------------------------------------------------------------------------------*/
void LiBat_InitAdc(void)
{
#if 0
	ADC_InitTypeDef ADC_InitStructure;
  	GPIO_InitTypeDef GPIO_InitStructure;

	//ADC Channel 7
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

	GPIO_Init(GPIOA, &GPIO_InitStructure);
	//ADC Channel 8
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_0;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	//ADC Channel 15
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_5;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	/* ADC1 configuration ------------------------------------------------------*/
	/* Clock On */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 | RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC, ENABLE);

	ADC_DeInit(ADC1);
	ADC_StructInit(&ADC_InitStructure);

	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_NbrOfChannel = 1;
	ADC_Init(ADC1, &ADC_InitStructure);

	// ADC1 regular channel0 configuration
	ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_1Cycles5);

	/* Enable ADC1 */
	ADC_Cmd(ADC1, ENABLE);

	/* Enable ADC1 DMA
	ADC_DMACmd(ADC1, ENABLE); */

	/* Enable ADC1 reset calibaration register */
	ADC_ResetCalibration(ADC1);
	/* Check the end of ADC1 reset calibration register */
	while(ADC_GetResetCalibrationStatus(ADC1));

	/* Start ADC1 calibaration */
	ADC_StartCalibration(ADC1);
	/* Check the end of ADC1 calibration */
	while(ADC_GetCalibrationStatus(ADC1));
#endif
}


/*----------------------------------------------------------------------------------------------
-- 对某一通道单独进行多次转换
-- Channel:通道编号,从0始
-- Channel: 转换通道号,ADC_Channel_xx 可参考 LIBAT_CHARGECURRENT_CH
											 LIBAT_CHARGEVOLT_CH
											 LIBAT_LIBATTEMPER_CH
-- Times: 转换次数,转换需要去掉一个最大值与一个最小值，故实际转换次数比Times多2
----------------------------------------------------------------------------------------------*/
uint16_t LiBat_ADCGetChannel(uint8_t Channel,uint8_t Times)
{
#if 1
	return 0;//getAdcValue(Channel + 1);
#else
	uint8_t i;
	uint16_t TempADC;
	uint16_t ADC_Channel_MAX;//通道转换中得到的最大值
	uint16_t ADC_Channel_MIN;//通道转换中得到的最小值
	uint32_t ADC_Channel_SUM;//通道转换结果的和

	if(!Times)
		return 0;
	ADC_Channel_SUM = 0;
	Times += 2;
	ADC_Channel_MAX = 0;
	ADC_Channel_MIN = 0x0fff;
	ADC_RegularChannelConfig(ADC1, Channel, 1, ADC_SampleTime_7Cycles5);
/*	ADC_SoftwareStartConvCmd(ADC1, ENABLE);
	while(ADC_TranseCt);*/
	for(i = 0;i < Times;i ++)
	{
		ADC_SoftwareStartConvCmd(ADC1, ENABLE);
		while(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET);
		TempADC = ADC_GetConversionValue(ADC1);
		if(TempADC > ADC_Channel_MAX)
			ADC_Channel_MAX = TempADC;
		if(TempADC < ADC_Channel_MIN)
			ADC_Channel_MIN = TempADC;
		ADC_Channel_SUM += TempADC;
	}
	ADC_Channel_SUM -= (ADC_Channel_MAX + ADC_Channel_MIN);
	ADC_Channel_SUM /= (Times - 2);

	return (uint16_t)ADC_Channel_SUM;
#endif
}


/***********************************************************************************************
end 底层驱动接口
***********************************************************************************************/

/***********************************************************************************************
中间层函数
***********************************************************************************************/

/*----------------------------------------------------------------------------------------------
-- 将电压值换算为电压值的函数
-- adc: adc转换的值
-- adc_max: adc的量程最大值
-- voltref: 参考电压值,此值放大1000倍并取整数
-- 返回: 转换的电压值,单位mv
----------------------------------------------------------------------------------------------*/
/*
static uint32_t LiBat_Adc2Volt(uint16_t adc,uint16_t voltref)
{
	uint32_t volt;

	volt = (uint32_t)adc * voltref;
	volt = volt/LIBAT_ADCDIGI_MAX;
	return volt;
}
*/

/*----------------------------------------------------------------------------------------------
-- 将电压值换算为电阻值的函数,适用于两电阻串联，被测电阻位于下方的采样电路
-- adc: adc转换的值
-- adc_max: adc的量程最大值
-- resup: 上接电阻的阻值，单位为欧姆
-- 返回: 电阻阻值,单位:欧姆
----------------------------------------------------------------------------------------------*/
/*
static uint32_t LiBat_Adc2Res(uint16_t adc,uint32_t resup)
{
	uint32_t resistor;

	resistor = (uint32_t)adc * resup;
	resistor = resistor/(LIBAT_ADCDIGI_MAX - adc);
	return resistor;
}
*/

/*----------------------------------------------------------------------------------------------
-- 获取充电电流值,单位 mA
-- 返回: 采样电流值,单位:mA
----------------------------------------------------------------------------------------------*/
uint16_t LiBat_GetChargeCurrent(void)
{
	int32_t res;
#if 1
	res = I_CHARG_ADC()*2;
	res = (res*25 + 15)/31 - LIBAT_CHARGECURRENT_OFFSET;
	res = res<0 ? 0:res;
#else
	res = LiBat_ADCGetChannel(LIBAT_CHARGECURRENT_CH,8);
	res = LiBat_Adc2Volt((uint16_t)res,LIBAT_ADCVOLREF);//单位为mv
	res = res*1000*LIBAT_ADCCURRENT_GAIN_DIV/LIBAT_ADCCURRENT_GAIN_MULTI/LIBAT_ADCCURRENT_RES;//计算电流值,单位为mA
#endif
	return (uint16_t)res;
}

/*----------------------------------------------------------------------------------------------
-- 获取充电电池的电压,单位 mv
-- 返回: 采样电压值,单位:mv
----------------------------------------------------------------------------------------------*/
uint16_t LiBat_GetBatVolt(void)
{
	uint32_t res;
#if 1
	res = (V_CHARG_ADC()*807 + 45)/91;// 3.300V参考电压
	//res = (getAdcValue(8)*7205 + 410)/819;//3.275V 参考电压
#else
	res = LiBat_ADCGetChannel(LIBAT_CHARGEVOLT_CH,8);
	res = LiBat_Adc2Volt((uint16_t)res,LIBAT_ADCVOLREF);//单位为mv
	res = res*LIBAT_ADCVOLT_GAIN_DIV/LIBAT_ADCVOLT_GAIN_MULTI;//计算电流值,单位为mA
#endif
	return (uint16_t)res;
}


/*----------------------------------------------------------------------------------------------
-- 温度查表函数,返回温度值-10~120,*10
-- 温度表长:131*2byte,in flash
-- 温度表名:const uint16 temper[120]
-- T95 T40 为校温参数,缺少为950与400
----------------------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------------------------
-- 获取充电电池的温度,单位 摄氏度*10 ,范围-10~110,*10
-- 返回: 电池的温度值,单位:摄氏度*10 ,范围-10~110,*10
-- 温度表名:const u16 r20klist[]
--		 超出温度表最低值返回TEMPER_MIN - 1
--	     超出温度表最高值返回TEMPER_MAX + 1
--	     NTC未接返回1500
----------------------------------------------------------------------------------------------*/
int16_t LiBat_GetBatTemper(void)
{
#if 1
	return 300;//GetTemp(T_BAT_ADC());
	//return 250;//GetTemp(getAdcValue(LIBAT_LIBATTEMPER_CHNO));
#else
	uint32_t res;

	res = LiBat_ADCGetChannel(LIBAT_LIBATTEMPER_CH,8);
	if((res > 60000) || (res < 350))
		return 1500;
	res = LiBat_Adc2Res((uint16_t)res,LIBAT_NTCRESABOVE);//单位为欧姆
	return LiBat_Res2Temper((uint16_t)res,950,400);
#endif
}





/*----------------------------------------------------------------------------------------------
-- PID函数
-- p_pid		 : 对应的PID结构体数据,包含相关的系数和微积分项等
-- TgtPoint 	 : 要设置的目标值,可为电压、电流、温度等
-- nowpoint 	 : 当前的采样值,可为电压、电流、温度等
-- p_ErrorBuffer : 差值存放的数组,目前长度为16,记录16个周期的差值数据
-- ErrorBufferCt : 差值数据的最新差值下标
-- 返回			 : 控制输出量
----------------------------------------------------------------------------------------------*/
int32_t LiBat_GEPIDCalc(LB_PID *p_pid,int16_t TgtPoint, int16_t nowpoint, int16_t *p_ErrorBuffer,uint8_t *ErrorBufferCt)
{
	int16_t dError,Error;
//	int32_t pid_p,pid_i,pid_d;
	int32_t pid_out = 0;

	p_pid->SetPoint = TgtPoint;
	p_pid->PrevError = p_pid->LastError;
	p_pid->LastError = Error;
	dError = p_pid->LastError - p_pid->PrevError;//当前微分

	Error = (p_pid->SetPoint - nowpoint);
	*ErrorBufferCt ++;//移动差值缓存下标至后一个位置,因缓存为循环式,因此移动过后的位置为缓存中最早的一个差值
	*ErrorBufferCt &= 0x0f;
	p_pid->SumError -= p_ErrorBuffer[*ErrorBufferCt];
	p_pid->SumError += Error; // 积分
	p_ErrorBuffer[*ErrorBufferCt] = Error;

	pid_out += p_pid->Proportion * (int32_t)Error;// 比例项+ feedback
	pid_out += p_pid->Integral * p_pid->SumError ; // 积分项
	pid_out += p_pid->Derivative * dError ; // 微分项

	return pid_out;
}

/*----------------------------------------------------------------------------------------------
-- PID相关变量初始化
-- p_pid		 : 对应的PID结构体数据,包含相关的系数和微积分项等
-- TgtPoint 	 : 要设置的目标值,可为电压、电流、温度等
-- nowpoint 	 : 当前的采样值,可为电压、电流、温度等
-- p_ErrorBuffer : 差值存放的数组,目前长度为16,记录16个周期的差值数据
-- ErrorBufferCt : 差值数据的最新差值下标
-- 返回			 : 控制输出量
----------------------------------------------------------------------------------------------*/
static uint8_t n_chs_status;//LiBat_CurrentPid的充电状态
void LiBat_InitPID(void)
{
	memset((uint8_t*)ErrorBufferCurrent,0,sizeof(ErrorBufferCurrent));
	memset((uint8_t*)ErrorBufferVolt,0,sizeof(ErrorBufferVolt));
	memset((uint8_t*)&Pid_Current,0,sizeof(&Pid_Current));
	memset((uint8_t*)&Pid_Volt,0,sizeof(&Pid_Volt));
	Pid_Volt.Proportion = LIBAT_VOLT_KP_DEF;
	Pid_Volt.Integral = LIBAT_VOLT_KI_DEF;
	Pid_Volt.Derivative= LIBAT_VOLT_KD_DEF;
	ErrorBufferCurrentCt = 0;//ErrorBufferVoltCt = 0;

	Pid_Current.Proportion = LIBAT_CURRENT_KP_DEF;
	Pid_Current.Integral = LIBAT_CURRENT_KI_DEF;
	Pid_Current.Derivative= LIBAT_CURRENT_KD_DEF;

	PwmOutOld = 0;

	n_chs_status = LB_CHS_NORMAL;

}
/***********************************************************************************************
end 中间层函数
***********************************************************************************************/


/*----------------------------------------------------------------------------------------------
-- 充电管理函数,每隔10ms调用,另,PID周期为100ms
-- Current 	: 要设置的电流值,单位:mA
-- Cycles	: 电流取平均值的个数 由于采样电流有波动,采取取值N个周期取平均值作为本PID周期的电流值使用
--			  此值目前取10
-- 返回		: 参照enum __LiBat_PID_Errors
--			  0 执行成功
-- 			  1 为充电错误,一般为超出温度范围导致
--			  2 为电已充满,处于待机状态
----------------------------------------------------------------------------------------------*/
#define LIBAT_PWMVALUE_WAVE_MAX	2//PID输出的PWM值每周期的变化幅度

uint8_t LiBat_chgerr_retry4test = 0;//如果充电启动失败,需要不停地重试,此为测试模式下的计数器,超过3次重试
									//就认为充电连接失败

uint8_t LiBat_PwmCurr;
uint16_t LiBat_CurrNow;
uint8_t LiBat_CurrentPid(uint16_t Current,uint8_t Cycles)
{
#if 1

	//PWM充电的充电电流是一个变化的值,需要进行多次周期性采样再取平均值
	static uint8_t ct = 0;//多次采样的计数器
	static uint32_t SumCur = 0, SumVolt = 0; //多次电流采样的和
	static uint16_t CuteCurrentCt = 0xffff;//小电流充电的计数器,当充电电压达到限压并小于结束电流时,
											//此值被清0并开始计数,当大于LIBAT_CHARGESTOP_TIMES时停止充电
	static uint16_t Standby2ChkVoltCt = 0xffff;//待机时检测电压的计数器
	//static int16_t PwmOutOld = 0;//上一次的PWM输出值
	int32_t PwmOut;//当前计算或处理得到的PWM输出值
	int16_t Temper;//电池温度采样值
#if LIBAT_LOG_OUT
	static uint8_t logoutct = 0;
#endif
	uint16_t n_rd_pwm;//读取当前的PWM的值,调试用

	static uint16_t n_cherror_timer = 0;

	uint8_t res = LB_ERROR_NONE;

	SumVolt += LiBat_GetBatVolt();//获取电池电压
	SumCur += LiBat_GetChargeCurrent();
	ct ++;
	if(ct < Cycles)
	{
		return LB_ERROR_NONE;
	}
	ct = 0;
	SumCur /= Cycles;
	SumVolt /= Cycles;

	Temper =LiBat_GetBatTemper();

	sys->sys_charge_cur = SumCur;//2018-10-09 jzz
	if(sys->sState == SYS_TEST_BD)//测试模式时,记录充电电流
	{
		BDTST_SET_CURR(SumCur);
	}

	if( Temper > 500 && Temper < 1500)//温度异常，停止充电                    Temper < 0 ||  去掉了零下过温保护  2018 12 10 am
	{
		LiBat_SetPwrChargePwm(0);
		if(Temper == 1500)//NTC故障或未连接视为电池不存在
		{
#if LIBAT_LOG_OUT
			PRINTF("Invalid Battery\r\n");
#endif
			res = LB_ERR_TEMPOVER;
		}
		else
		{
#if LIBAT_LOG_OUT
			PRINTF("Temperature over limit %d\r\n",Temper);
#endif
			res = LB_ERR_NOBAT;
		}
		PwmOutOld = 0;
		n_chs_status = LB_CHS_NOCHARGE;
	}
	else
	{
		if(n_chs_status != LB_CHS_ERROR && SumCur < LIBAT_CHARGESTOP_CURRENT && CuteCurrentCt < 0xfff0)
		{

			CuteCurrentCt ++;

			if(CuteCurrentCt > LIBAT_CHARGESTOP_TIMES)
			{
				CuteCurrentCt = 0xfff1;
				n_chs_status = LB_CHS_STANDBY;
				PwmOutOld = 0;
				LiBat_SetPwrChargePwm(0);
#if LIBAT_LOG_OUT
				PRINTF("LiBat Charge over!\r\n");
#endif
			}
		}
		else if(n_chs_status != LB_CHS_ERROR && SumCur > (LIBAT_CHARGECURRENT_SET * 1.5))		///过流   再加上过压 16.8V
		{
			n_chs_status = LB_CHS_ERROR;
			PwmOut=PwmOutOld = 0;
			//TIM_ForcedOC3Config(TIM1, TIM_ForcedAction_InActive);
			LiBat_SetPwrChargePwm(0);
			if(sys->sState == SYS_CHARGE)
				log_printf("^^^^^LB_CHS_ERROR--%d^^^^^^\r\n",SumCur);
			#if 1
			#if 1
			GPIO_InitTypeDef GPIO_InitStructure;
			LiBat_ExitChargeMode();
		       //设置该引脚为复用输出功能,输出TIM1 CH1的PWM脉冲波形	GPIOE.9
			GPIO_InitStructure.GPIO_Pin = LIBAT_CHPWM_PIN ; //TIM1_CH1
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;  //复用推挽输出
			GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
			GPIO_Init(LIBAT_CHPWM_PORT, &GPIO_InitStructure);//初始化GPIO
			#endif
			LIBAT_CHPWM_PORT->ODR &= ~LIBAT_CHPWM_PIN;
			//TIM_Cmd(TIM1, DISABLE);  //使能TIM1

			//delay_ms(200);

			//GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  //复用推挽输出
			//GPIO_Init(LIBAT_CHPWM_PORT, &GPIO_InitStructure);//初始化GPIO
			#endif
			n_cherror_timer = 0;
		}
	}
	if(LiBat_isOnDock() == 0)//由于PWM为渐变的原因,当充电器（座）未连接时,需要将PwmOutOld置0,以防再接入时电流超限
		PwmOutOld = 0;
	switch(n_chs_status)
	{
		case LB_CHS_NORMAL:
			if(SumVolt > LIBAT_CHARGEVOLT_MAX)
			{
				if(sys->sState == SYS_CHARGE)
					log_printf("volt overload\r\n");
				PwmOutOld = PwmOutOld > 1 ? (PwmOutOld - 1):0;
				PwmOut = PwmOutOld;
				res = LB_ERR_VOLTOVER;
				if(CuteCurrentCt == 0xffff)
				{
				//	CuteCurrentCt = 0;
#if LIBAT_LOG_OUT
				PRINTF("LiBat CuteCurrent!\r\n");
#endif
				}
				//LiBat_SetPwrChargePwm(0);
			}
			else
			{
				PwmOut = LiBat_GEPIDCalc(&Pid_Current,Current,SumCur,ErrorBufferCurrent,&ErrorBufferCurrentCt)/200;
				LiBat_CurrNow = SumCur;
				//PRINTF("LB_PID:%d,%d\r\n",PwmOut,PwmOutOld);
				PwmOut += 127;
				if(PwmOut > LIBAT_PWM_MAX)PwmOut = LIBAT_PWM_MAX;
				if(PwmOut < 0)PwmOut = 0;
				if(PwmOut - PwmOutOld > LIBAT_PWMVALUE_WAVE_MAX)
				{
					PwmOut = PwmOutOld + LIBAT_PWMVALUE_WAVE_MAX;
					if(PwmOut > LIBAT_PWM_MAX)PwmOut = LIBAT_PWM_MAX;
					//log_printf("s1\r\n");
				}
				else if(PwmOut - PwmOutOld < -LIBAT_PWMVALUE_WAVE_MAX)
				{
					PwmOut = PwmOutOld - LIBAT_PWMVALUE_WAVE_MAX;
					if(PwmOut < 0)PwmOut = 0;
					//log_printf("s2\r\n");
				}
				PwmOutOld = PwmOut;
			}
			LiBat_PwmCurr = PwmOut & 0xff;
			//if(PwmOut > 255)
			//	PwmOut =254;
		//	PRINTF("LB_PID2:%d,%d\r\n",PwmOut,PwmOutOld);
			LiBat_SetPwrChargePwm((uint16_t)PwmOut);
			break;
		case LB_CHS_STANDBY:
			if(Standby2ChkVoltCt == 0xffff)
				Standby2ChkVoltCt = 0;
			else
			{
				Standby2ChkVoltCt ++;
				if(Standby2ChkVoltCt > LIBAT_CHARGERESTART_DELAY)
				{
					Standby2ChkVoltCt = 0;
					if(SumVolt < LIBAT_CHARGERESTART_VOLT)
					{
						Standby2ChkVoltCt = 0xffff;
						CuteCurrentCt = 0xffff;
						n_chs_status = LB_CHS_NORMAL;
						if(sys->sState == SYS_CHARGE)
							log_printf("LiBat Charge restart!\r\n");
					}
				}
			}
			PwmOut=PwmOutOld = 0;
			LiBat_SetPwrChargePwm(0);
			break;
		case LB_CHS_ERROR:
			PwmOut=PwmOutOld = 0;
			if(n_cherror_timer ++ > 50)
			{
				n_cherror_timer = 0;
				n_chs_status = LB_CHS_NORMAL;
				LiBat_InitPwrChargePwm();

				//TIM_ForcedOC3Config(TIM1, TIM_ForcedAction_Active);
				if(sys->sState == SYS_CHARGE)
					log_printf("return to charge...\r\n");
				else if(sys->sState == SYS_TEST_BD)
				{
					if(LiBat_chgerr_retry4test ++ > 2)
					{
						res = LB_ERR_RETRY;
					}
				}
			}
			if(LiBat_GetPwrChargePwm() != 0)
				LiBat_SetPwrChargePwm(0);
			break;
		case LB_CHS_NOCHARGE:
			if((Temper > 5) || (Temper < 360))
				n_chs_status = LB_CHS_NORMAL;
			PwmOut = 0;
			LiBat_SetPwrChargePwm(PwmOut);
		default:
			break;
	}
#if LIBAT_LOG_OUT
	if(logoutct > 5)
	{
		logoutct = 0;
		//PwmOut = 0;
		if(sys->sState == SYS_CHARGE)
		{
			www_idleintel_com();
			n_rd_pwm = LiBat_GetPwrChargePwm();
			log_printf("volt=%d,cur=%d,tmp=%d,pwm=%d-%d(%d,%d,%d,%d)%f\r\n",SumVolt,SumCur,Temper,PwmOut,n_rd_pwm,res,CuteCurrentCt,(GPIOE->IDR & GPIO_Pin_13) != 0,n_chs_status,sys->angle);
		}
	}
	else
		logoutct ++;
#endif
	//求和的变量清0
	SumVolt = 0;
	SumCur = 0;
	return res;
#else
	uint16_t NowCurrent;
	int32_t PwmOut;
	int16_t Temper;
	Temper = LiBat_GetBatTemper();
	if((Temper < 0) || (Temper > 400))
	{
		LiBat_SetPwrChargePwm(0);
		return -1;
	}
	NowCurrent = LiBat_GetChargeCurrent();
	PwmOut = LiBat_GEPIDCalc(&Pid_Current,Current,NowCurrent,ErrorBufferCurrent,&ErrorBufferCurrentCt)/8;
	if(PwmOut > LIBAT_PWM_MAX)PwmOut = LIBAT_PWM_MAX;
	if(PwmOut < 0)PwmOut = 0;
	LiBat_PwmCurr = PwmOut & 0xff;
	LiBat_SetPwrChargePwm((uint16_t)PwmOut);
	return 0;
#endif
}

uint8_t LiBat_GetPwmCurr(void)
{
	return LiBat_PwmCurr;
}

uint16_t LiBat_GetCurrNow(void)
{
	return LiBat_CurrNow;
}

/*----------------------------------------------------------------------------------------------
-- 恒压PID函数
-- Volt 	: 要设置的电压值,单位:mv
-- NowVolt  : 当前的电压值,如果未知则写入0xffff,单位:mv
-- 返回		: 执行成功返回0
-- 			  失败返回-1,一般为超出温度范围导致
----------------------------------------------------------------------------------------------*/
//volatile uint16_t LiBat_VoltValue;
/*
uint8_t LiBat_VoltPid(uint16_t Volt, uint16_t NowVolt)
{
	int32_t PwmOut;
	int16_t Temper;
	Temper = LiBat_GetBatTemper();
	if((Temper < 0) || (Temper > 400))
	{
		LiBat_SetPwrChargePwm(0);
		return -1;
	}
	NowVolt = (NowVolt == 0xffff)?LiBat_GetBatVolt():NowVolt;
	//LiBat_VoltValue = NowVolt;
	//PwmOut = LiBat_GEPIDCalc(&Pid_Volt,Volt,NowVolt,ErrorBufferVolt,&ErrorBufferVoltCt)/500;
	PwmOut = LiBat_GEPIDCalc(&Pid_Volt,Volt,NowVolt,ErrorBufferVolt,&ErrorBufferVoltCt)/4000;
	if(PwmOut > LIBAT_PWM_MAX)PwmOut = LIBAT_PWM_MAX;
	if(PwmOut < 0)PwmOut = 0;
	LiBat_SetPwrChargePwm((uint16_t)PwmOut);
	return 0;
}
*/

/*----------------------------------------------------------------------------------------------
-- 关闭除遥控接收红外以外的所有外设电源,包括电机电源/测距及跌落红外电源
-- OnOff 1 On 0 Off
----------------------------------------------------------------------------------------------*/
void LiBat_TurnOnOtherPwr(uint8_t OnOff)
{
/*
	if(!OnOff)
	{
		GPIO_ResetBits(LIBAT_IRDIST_PWR_PORT, LIBAT_IRDIST_PWR_PIN);//关闭测距红外的电源
		GPIO_ResetBits(LIBAT_MOTOR_PWR_PORT, LIBAT_MOTOR_PWR_PIN);//关闭所有电机的电源
	}
	else
	{
		GPIO_SetBits(LIBAT_IRDIST_PWR_PORT, LIBAT_IRDIST_PWR_PIN);//打开测距红外的电源
		GPIO_SetBits(LIBAT_MOTOR_PWR_PORT, LIBAT_MOTOR_PWR_PIN);//打开所有电机的电源
	}
	*/
}


/*----------------------------------------------------------------------------------------------
-- 充电模块驱动初始化
----------------------------------------------------------------------------------------------*/
void LiBat_HalInit(void)
{


	LiBat_TurnOnOtherPwr(0);
	LiBat_InitPwrChargePwm();
	//LiBat_InitAdc();
	LiBat_InitPID();
}


/*----------------------------------------------------------------------------------------------
-- 充电模块退出
-- 充电完成或者机器人工作模式改变后,由于部分MCU片上外设复用的关系,需要重新初始化,以恢复现场
-- 这里主要是对TIM3重新初始化一下,在这之前首先要将充电关闭
----------------------------------------------------------------------------------------------*/
void LiBat_ExitChargeMode(void)
{
#if 0
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;
	//充电PWM口置为GPIO模式,并置低
	GPIO_InitStructure.GPIO_Pin =  LIBAT_CHPWM_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

	GPIO_Init(LIBAT_CHPWM_PORT, &GPIO_InitStructure);
	GPIO_ResetBits(LIBAT_CHPWM_PORT, LIBAT_CHPWM_PIN);

	TIM_Cmd(LIBAT_CHPWM_TIM, DISABLE);
	/* Time base configuration */
	TIM_TimeBaseStructure.TIM_Period = 0xff;
	TIM_TimeBaseStructure.TIM_Prescaler = 15;
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;//TIM_CKD_DIV2;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

	TIM_TimeBaseInit(LIBAT_CHPWM_TIM, &TIM_TimeBaseStructure);

	/* PWM1 Mode configuration: Channel1 */
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_Pulse = 0;
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low;

	//TIM_OC1Init(LIBAT_CHPWM_TIM, &TIM_OCInitStructure);
	//TIM_OC1PreloadConfig(LIBAT_CHPWM_TIM, TIM_OCPreload_Enable);

	//TIM_SetCompare1(TIM3, (0x7f));

	//TIM_OC2Init(LIBAT_CHPWM_TIM, &TIM_OCInitStructure);
	//TIM_OC2PreloadConfig(LIBAT_CHPWM_TIM, TIM_OCPreload_Enable);

	//TIM_SetCompare2(TIM3, (0x7f));

	TIM_OC3Init(LIBAT_CHPWM_TIM, &TIM_OCInitStructure);
	TIM_OC3PreloadConfig(LIBAT_CHPWM_TIM, TIM_OCPreload_Enable);

	/* TIM3 enable counter */
	TIM_Cmd(LIBAT_CHPWM_TIM, ENABLE);
	TIM_CtrlPWMOutputs(LIBAT_CHPWM_TIM, ENABLE);
#endif
	TIM1->CCR3 = 1000;
	LiBat_TurnOnOtherPwr(1);
}


char libat_charge_task(void)
{
		//static int cc=0;
	   	if(sys->sState != SYS_CHARGE)
	  	{
	  		//GPIO_Config();
	   		return 0;
	   	}
		//导航回去从充电桩出来

	  sys->auto_lamp_sta=0;
	   if(TIM5->CNT >=10000)
	   {
	   		TIM5->CNT =0;
	   		
	  	 	////LiBat_CurrentPid(LIBAT_CHARGECURRENT_SET,10);
	  	 	if((0 == CHARGE_DONE_DETECT()) && ((sys->charge_sta & CHARG_DONE) != CHARG_DONE ) )
	  	 	{
				sys->charge_sta |= CHARG_DONE;	//充电完成标志,提供给显控显示led	
				log_printf("[libat_charge_task]charge done,full and go=%d\r\n",sys->full2go);
	  	 	}
	  	 	if((sys->charge_sta & CHARG_DONE) == CHARG_DONE)
	  	 	{				
				if(sys->full2go && ((sys->charge_sta & DOCK_CHARGE) == DOCK_CHARGE))
				{
					log_printf("now cleanning....\r\n");
					sys->sState = SYS_NAVIGAT;
					sys->gey_key_clear_err = 1;		
					ny3p_play(VOICE_M_START);
					LED3_ON();
					return 1;
				}
	  	 	}
	   }

	   if(    (!EXTERAL_AC_DETECT()) && (!DOCK_DETECT())   )
	   {
	   		log_printf("no charge... sta=%x,%d,%d\r\n",sys->charge_sta,EXTERAL_AC_DETECT(),DOCK_DETECT());
	   		{
				int i=0;
				for(i=0;i<200;i++)
				{
					delay_ms(5);
					if(EXTERAL_AC_DETECT() || DOCK_DETECT())
						break;
					//log_printf("doc sta=(%d,%d)\r\n",EXTERAL_AC_DETECT() , DOCK_DETECT());
				}
				if( i>=190)
				{
					ny3p_play(VOICE_DIDI);
					/*
					if(sys->charge_sta & DOCK_CHARGE)		//座子充电进来
					{

					}
					*/
			   		if(sys->charge_sta & AC_CHARGE)
			   		{
			   			sys->sState = SYS_IDLE;
			   			//ny3p_play(VOICE_CHARGE_M);
			   		}
			   		else
			   		{
						log_printf("init to dock sta\r\n");
						//micony201809
						 LED4_ON();
						reset_angle();
						///2018-10-18 jzz
						timer1_init();					//时钟1，控制两个边刷
						timer2_init();					//控制中扫电机和吸尘电机
						MOTOR_POWER_ON();
						MOTOR_CTRL_SIDE(DOCK_SIDE_PWM_L,DOCK_SIDE_PWM_R);
						///
						//sys->angle = 0;
						//MOTOR_POWER_ON();
						//MOTOR_CTRL1(0,0,0,0);
						//MOTOR_CTRL_SIDE(DOCK_SIDE_PWM_L,DOCK_SIDE_PWM_R);
						motor_run(GO_STOP,0,0,0);
						//reset_gyro(1);
						usart_init_info();

						//delay(100);
						init_charge(SYS_IDLE);
						//delay(100);
						delay_sensor(100);
						if(sys->sState == SYS_IDLE)///2018-05-24 jzz
							return 0;
						sys->sState = SYS_DOCK;
						sys->wifiAreaNoCalc_en = 1;
#if WIFICONFIG
		init_stream_clean_id();
#endif
					//	navigat_init(5);

						log_printf("go back...\r\n");
						motor_back_off(80);
						if(sys->sState == SYS_IDLE)///2018-05-24 jzz
							return 0;
							//delay_ms(300);
						//micony201809
						//MOTOR_POWER_ON();
						//MOTOR_CTRL1(0,0,0,0);
						//delay(1000);
						//reset_angle();
						get_gyro_ofs();
						get_gyro_org();
						www_idleintel_com();
						log_printf("sys angle=%3.1f\r\n",sys->angle);
						motor_run(GO_STOP,0,0,0);
						//delay(200);
						//ny3p_play(VOICE_DIDI);
#if DUST_BOX_FULL_CHECK
	dust_full_check_pro(2);		//清扫结束更新尘满累积时间
#endif
#if STRAINER_ALARM_CHECK//滤网
	strainer_alarm_check_pro(2);
#endif
#if SIDE_RUSH_ALARM_CHECK
	side_rush_alarm_check_pro(2);
#endif
#if MAIN_RUSH_ALARM_CHECK
	main_rush_alarm_check_pro(2);
#endif

					//	if(sys->angle < 45 || sys->angle > 270)
						turn_to_deg(180);

						//robot_turn_deg(GO_RIGTH,DEG_TURN_PWM,80);
						//robot_turn_deg(GO_RIGTH,DEG_TURN_PWM,80);
						if(sys->sState == SYS_IDLE)///2018-05-24 jzz
							return 0;
						navigat->angle = 180;

						log_printf("turn to 180,go...\r\n");
						//micony201809
						motor_go_forwark(5000,NO_SIDE_NEAR,NULL);  //  20200311    hw=2000
						if(sys->sState == SYS_IDLE)		///2018-05-24	jzz
						 	return 0;
						//timer1_init();					//时钟1，控制两个边刷
						//timer2_init();					//控制中扫电机和吸尘电机
						//ny3p_play(VOICE_M_DOCK);
						sys->sState = SYS_DOCK;
						//pwifistream->wifiAreaNoCalc = 1;
						//MOTOR_POWER_ON();

						//MOTOR_CTRL(DOCK_SIDE_PWM,DOCK_SIDE_PWM,DOCK_DUST_PWM,DOCK_MID_PWM);
						//init_charge(SYS_IDLE);
						//init_sys_sta(SYS_DOCK);
			   		}

			sys->wifistatechange = 1;
					return 1;
				}
	   		}
	   }
	return 0;

}

