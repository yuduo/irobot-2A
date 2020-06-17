
#ifndef _H_LIBAT_
#define _H_LIBAT_


#define LIBAT_LOG_OUT	1
/********************************************************************************************************
ADC相关
********************************************************************************************************/
//ADC的量程最大值
#define LIBAT_ADCDIGI_MAX	4095

//各采样通道,这个在这里没有用到

#if 0
#define LIBAT_CHARGECURRENT_CH	ADC_Channel_8//充电电流
#define LIBAT_CHARGEVOLT_CH		ADC_Channel_7//充电电压
#define LIBAT_LIBATTEMPER_CH	ADC_Channel_15//电池温度
#else
#define LIBAT_CHARGECURRENT_CHNO	9//充电电流
#define LIBAT_CHARGEVOLT_CHNO		8//充电电压
#define LIBAT_LIBATTEMPER_CHNO	7//电池温度
#endif

//参考电压值：放大1000倍，单位为mv,参考电压一般不会超过10V,即10000mv
#define LIBAT_ADCVOLREF	3250
//电流采样转换为电压后的电压放大倍数
#define LIBAT_ADCCURRENT_GAIN_MULTI	10//分子
#define LIBAT_ADCCURRENT_GAIN_DIV	1//分母
//电流采样电阻阻值,放大1000倍,单位为毫欧
#define LIBAT_ADCCURRENT_RES	100
//电压放大倍数
#define LIBAT_ADCVOLT_GAIN_MULTI	1//分子
#define LIBAT_ADCVOLT_GAIN_DIV	11//分母

//NTC的上端分压电阻值,单位欧姆,此中使用为10K 1%
#define LIBAT_NTCRESABOVE	10000
/********************************************************************************************************
end ADC相关
********************************************************************************************************/

/********************************************************************************************************
PID相关
********************************************************************************************************/
//PID的缺省系数
#define LIBAT_CURRENT_KP_DEF	350
#define LIBAT_CURRENT_KI_DEF	0//600
#define LIBAT_CURRENT_KD_DEF	50

//这里没有用到恒压充电,只是在恒流充电时做了限压
#define LIBAT_VOLT_KP_DEF	1
#define LIBAT_VOLT_KI_DEF	12
#define LIBAT_VOLT_KD_DEF	16
/********************************************************************************************************
end PID相关
********************************************************************************************************/

/********************************************************************************************************
控制IO相关
********************************************************************************************************/
//电机电源开关
#define LIBAT_MOTOR_PWR_PORT	MT_PWR_PORT
#define LIBAT_MOTOR_PWR_PIN		MT_PWR_PIN

//测距及跌落红外开关
#define LIBAT_IRDIST_PWR_PORT	IR_DIST_PWR_PORT
#define LIBAT_IRDIST_PWR_PIN	IR_DIST_PWR_PIN

//电池充电IO
#define LIBAT_CHPWM_PORT	GPIOE
#define LIBAT_CHPWM_PIN		GPIO_Pin_13

#define LIBAT_CHPWM_TIM		TIM1
#define LIBAT_CHPWM_CH		3
/********************************************************************************************************
end 控制IO相关
********************************************************************************************************/

/********************************************************************************************************
充电策略相关
********************************************************************************************************/

//恒流充电电流
#define LIBAT_CHARGECURRENT_SET	500

//恒流充电的最大限压单位:mv
//#define LIBAT_CHARGEVOLT_MAX	16500//16500
#define LIBAT_CHARGEVOLT_MAX	25100//16500

#define LIBAT_CHARGECURRENT_OFFSET	12
//充电电压最大值
#define LIBAT_CHARGEVOLT_SET

//PWM的输出最大值
#define LIBAT_PWM_MAX	255 // 4095->255


//达到最大限压后,如果充电电流持续小于此电流值,认为电池已充满
//充电结束电流
#define LIBAT_CHARGESTOP_CURRENT	100

//恒压充电时,当充电电流小于LIBAT_CHARGECURRENT_END连续计数大于LIBAT_CHARGESTOP_TIMES次则结束充电
#define LIBAT_CHARGESTOP_TIMES	1000

//在非充电状态下(前提是已接充电座(器)),如果电池已充满并断开,那么每隔一段时间需要去检测电池电压,
//如果充电电流大于LIBAT_CHARGECURRENT_SET,则恢复充电,此时间间隔单位为100us
#define LIBAT_CHARGERESTART_DELAY	6000
//充电完成后发现电压下降,再恢复充电时的电压门限值
#define LIBAT_CHARGERESTART_VOLT	24800//16000
/********************************************************************************************************
end 充电策略相关
********************************************************************************************************/
//PID计算的返回值
enum __LiBat_PID_Errors
{
	LB_ERROR_NONE = 0,
	LB_ERR_CURROVER,//电流超限(这个未使用)
	LB_ERR_VOLTOVER,//电压超限
	LB_ERR_TEMPOVER,//温度超限
	LB_ERR_NOBAT,//电池不存在
	LB_ERR_RETRY//电池充电连接失败,只用在测试模式时,充电启动失败重试次数超过3次,返回此错误
};

#if 1
enum __LiBat_ChargeStatus
{
	LB_CHS_NOCHARGE = 0,//未开始充电或充电已停止
	LB_CHS_NORMAL,//正常充电,此状态包括恒流充电及恒压充电(其实就是加了限压控制的PID过程)
	LB_CHS_STANDBY,//待充模式,此中每隔一段时间检测电池电压以备恢复充电
	LB_CHS_ERROR//充电工作异常,比如电流失控,当进入此状态,pwm置0(即关闭),直至下一次重新进入充电状态
};

#endif

void LiBat_SetPwrChargePwm(uint16_t pwm);
#if 1
uint8_t LiBat_CurrentPid(uint16_t Current,uint8_t Cycles);
#else
uint8_t LiBat_CurrentPid(uint16_t Current);
#endif
uint8_t LiBat_VoltPid(uint16_t Volt, uint16_t NowVolt);
void LiBat_HalInit(void);

uint16_t LiBat_GetChargeCurrent(void);
uint16_t LiBat_GetBatVolt(void);

extern volatile uint16_t LiBat_VoltValue;
char libat_charge_task(void);
void LiBat_ExitChargeMode(void);

int get_state_libat_percent(uint8_t	sState,int volt);

int get_libat_percent(int volt);

int16_t LiBat_GetBatTemper(void);


#endif
