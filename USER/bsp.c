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

//#include "includes.h"
#include "stm32f10x.h"
#include "stm32f10x_conf.h"
#include "stm32f10x_it.h"

#include "stm32f10x_pwr.h"
#include "stm32f10x_rtc.h"
#include "stm32f10x_bkp.h"


#include "sys.h"
//#include "BLDC.h"

extern uint32_t bldcSpeed;

#if ROOMA_GYROONLY

void RTC_Init(void);
#endif

void  RCC_Config  (void);
void  GPIO_Config (void);
void  NVIC_Config(void);
void  EXTI_Config (void);
void EXTI_cfg(void);
/**
  *@Function
  *@Description
  *@Calls
  *@Call By
  *@Param	xxx
  *		@arg
  *		@arg
  *@Param	xxx
  *		@arg
  *		@arg
  *@Reture
  *@Others
  **/
void Periph_Init(void)
{
	//GPIO_InitTypeDef GPIO_InitStructure;
  /*********时钟使能*************/
  RCC_Config();
	/*

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_14; //TIM1_CH1
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;//GPIO_Mode_AF_PP;  //复用推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOE, &GPIO_InitStructure);//初始化GPIO

	 GPIO_SetBits(GPIOE,GPIO_Pin_9);
	while(1);
*/

  /********中断优先级设置********/
  NVIC_Config();

  EXTI_cfg();  //外部中断，测试过，可用!


  /********外部中断设置**********/
  /********外部GPIO设置**********/


   GPIO_Config();
   senser_gpio_config();
#if ROOMA_GYROONLY
	//#if DP_PANEL_EN == 0 || DP_PANEL_EN == 3
   key_io_init();
   //#endif
	////RTC_Init();
#endif
	I2C1_GPIO_Init();
	//i2c_io_init();

  // USART2_Config();
 //  ADC_Config();
//   MCPWM1_Config();
}
void EXTI_cfg()
{
       EXTI_InitTypeDef EXTI_InitStructure;
       //清空中断标志
 	 /*
		PD1	左后轮码盘出输A相
		PE4	右后码盘输出A相
		PD13 前轮码盘B相
		PE11 滚刷码盘
 	 */
       EXTI_ClearITPendingBit(EXTI_Line1);////EXTI_ClearITPendingBit(EXTI_Line2);
       EXTI_ClearITPendingBit(EXTI_Line4);
       //EXTI_ClearITPendingBit(EXTI_Line13);
	EXTI_ClearITPendingBit(EXTI_Line11);	//wy190615 PE11滚刷码盘

       GPIO_EXTILineConfig(GPIO_PortSourceGPIOD, GPIO_PinSource1);////GPIO_EXTILineConfig(GPIO_PortSourceGPIOD, GPIO_PinSource2);
       GPIO_EXTILineConfig(GPIO_PortSourceGPIOE, GPIO_PinSource4);
       //GPIO_EXTILineConfig(GPIO_PortSourceGPIOD, GPIO_PinSource13);
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOE, GPIO_PinSource11);//wy190615 PE11滚刷码盘

  	//EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;

       EXTI_InitStructure.EXTI_Line = EXTI_Line1; //选择中断线路2 3 5
       EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt; //设置为中断请求，非事件请求
       EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;//s_Falling; //设置中断触发方式为上下降沿触发
       EXTI_InitStructure.EXTI_LineCmd = ENABLE;                                          //外部中断使能
       EXTI_Init(&EXTI_InitStructure);

       EXTI_InitStructure.EXTI_Line = EXTI_Line4; //选择中断线路2 3 5
       EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt; //设置为中断请求，非事件请求
       EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;//s_Falling; //设置中断触发方式为上下降沿触发
       EXTI_InitStructure.EXTI_LineCmd = ENABLE;                                          //外部中断使能
       EXTI_Init(&EXTI_InitStructure);
/*
        EXTI_InitStructure.EXTI_Line = EXTI_Line13; //选择中断线路2 3 5
       EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt; //设置为中断请求，非事件请求
       EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;//s_Falling; //设置中断触发方式为上下降沿触发
       EXTI_InitStructure.EXTI_LineCmd = ENABLE;                                          //外部中断使能
       EXTI_Init(&EXTI_InitStructure);
*/
	//wy190615 PE11滚刷码盘
	EXTI_InitStructure.EXTI_Line = EXTI_Line11;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);


#if 0
/*
	回冲红外接收
	PC7		中间IR（回充用来接收信号）		中断输入/捕获输入	INT7/TIM3_CH2
	PB12		左边IR（回充用来接收信号的）	中断输入/捕获输入	INT12	能否改成PC6?这样可以用TIM3的捕获输入CH1
	PC8		右边IR（回充用来接收信号)		中断输入/捕获输入	INT8/TIM3_CH3
	PC9		360°IR接收					中断输入/捕获输入	INT9/TIM3_CH4
*/
	   EXTI_ClearITPendingBit(EXTI_Line8);
       EXTI_ClearITPendingBit(EXTI_Line7);
       EXTI_ClearITPendingBit(EXTI_Line9);
       EXTI_ClearITPendingBit(EXTI_Line12);

       GPIO_EXTILineConfig(GPIO_PortSourceGPIOE, GPIO_PinSource7);
       GPIO_EXTILineConfig(GPIO_PortSourceGPIOD, GPIO_PinSource8);
       GPIO_EXTILineConfig(GPIO_PortSourceGPIOE, GPIO_PinSource9);
       GPIO_EXTILineConfig(GPIO_PortSourceGPIOD, GPIO_PinSource12);

	   EXTI_InitStructure.EXTI_Line = EXTI_Line7 | EXTI_Line8 | EXTI_Line9| EXTI_Line12; //选择中断线路2 3 5
       EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt; //设置为中断请求，非事件请求
       EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;//s_Falling; //设置中断触发方式为上下降沿触发
       EXTI_InitStructure.EXTI_LineCmd = ENABLE;                                          //外部中断使能
       EXTI_Init(&EXTI_InitStructure);
#endif

}

/**
  *@Function
  *@Description
  *@Calls
  *@Call By
  *@Param	xxx
  *		@arg
  *		@arg
  *@Param	xxx
  *		@arg
  *		@arg
  *@Reture
  *@Others
  **/
void RCC_Config(void)
{
	SystemInit();

	/* 使能GPIO(A~C)|ADC1|USART1 时钟 */
	RCC_APB2PeriphClockCmd(   RCC_APB2Periph_GPIOA
							| RCC_APB2Periph_GPIOB
			 	 	 	 	| RCC_APB2Periph_GPIOC
			 	 	 	 	| RCC_APB2Periph_GPIOD
			 	 	 	 	| RCC_APB2Periph_GPIOE
							| RCC_APB2Periph_AFIO
							| RCC_APB2Periph_TIM1
							| RCC_APB2Periph_ADC1
							| RCC_APB2Periph_TIM8
							| RCC_APB2Periph_USART1
 							, ENABLE );
//	 RCC_APB1PeriphClockCmd(  RCC_APB1Periph_USART2
//	 						, ENABLE );
/******************CAN时钟使能**************************/
//	RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE);
 	RCC_APB1PeriphClockCmd(   RCC_APB1Periph_TIM3
							| RCC_APB1Periph_USART2
							| RCC_APB1Periph_TIM4
							| RCC_APB1Periph_TIM5
							| RCC_APB1Periph_TIM2
							| RCC_APB1Periph_UART4
							| RCC_APB1Periph_PWR
							| RCC_APB1Periph_BKP
							, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1,ENABLE);



 TIM_ARRPreloadConfig(TIM3, ENABLE);
#if DUST_BOX_FULL_CHECK

	 /* Enable PWR(电源控制） and BKP clock */
		 //RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);

		 PWR_DeInit();
	 /* Enable write access to Backup domain */
		 PWR_BackupAccessCmd(ENABLE);
	 /* Clear Tamper pin Event(TE) pending flag */
		 BKP_ClearFlag();
#endif




}





/***************************************************
* Descirbe  :
* Input     :
* Output    :
* Attention :
* author 	:
***************************************************/
void  GPIO_Config(void)
{

    GPIO_InitTypeDef GPIO_InitStructure;

	//4路外部中断
	/*
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1| GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4| GPIO_Pin_10| GPIO_Pin_11| GPIO_Pin_12| GPIO_Pin_13| GPIO_Pin_14|GPIO_Pin_15; //选择引脚2 3 5
	GPIO_InitStructure.GPIO_Mode =GPIO_Mode_IN_FLOATING; //GPIO_Mode_IPU;// 选择输入模式为浮空输入 GPIO_Mode_IPU;//
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;          //输出频率最大50MHz
	GPIO_Init(GPIOB,&GPIO_InitStructure);
	*/
	/*
		PD14	左后轮码盘出输A相
		PD13	右后码盘输出A相
 	 */
       GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13  |GPIO_Pin_14;
       GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
       GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
       GPIO_Init(GPIOD,&GPIO_InitStructure);

       //PD1 PD2	左后轮码盘出输A相	中断输入	INT1
       GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_0;   //  GPIO_Pin_2 | GPIO_Pin_1;
       GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
       GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
       GPIO_Init(GPIOD,&GPIO_InitStructure);

       //10	PE4 PE5 码盘输出A相	中断输入	INT4
       GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5;
       GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
       GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
       GPIO_Init(GPIOE,&GPIO_InitStructure);

		//wy190615 滚刷码盘
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOE,&GPIO_InitStructure);

		//PB15 PE7 PC8 左中右回充IR检测
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15 ;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOB,&GPIO_InitStructure);

		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
		GPIO_Init(GPIOC, &GPIO_InitStructure);

		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
		GPIO_Init(GPIOE, &GPIO_InitStructure);

		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
		GPIO_Init(GPIOE, &GPIO_InitStructure);

		//PE6 PE12 碰撞红外检测
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_12;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOE,&GPIO_InitStructure);


		//PD7 虚拟墙
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOD,&GPIO_InitStructure);

		//PE9 前照灯
		/*GPIO_InitStructure.GPIO_Pin =   GPIO_Pin_9;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOE, &GPIO_InitStructure);
		*/
		//PC9 前撞红外灯
		GPIO_InitStructure.GPIO_Pin =   GPIO_Pin_9;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOC, &GPIO_InitStructure);

		//PD11 充电座到位检测
		GPIO_InitStructure.GPIO_Pin = PIN_CHARGE;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(PORT_CHARGE,&GPIO_InitStructure);

		//PD0 AC检测
		GPIO_InitStructure.GPIO_Pin = PIN_AC;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(PORT_AC,&GPIO_InitStructure);

		//PE11 电源保持
		/*GPIO_InitStructure.GPIO_Pin =   PIN_VOID_POWER;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(PORT_VOID_POWER, &GPIO_InitStructure);*/
/*#if DP_PANEL_EN < 4
		GPIO_InitStructure.GPIO_Pin = PIN_SIDE_MOTOR_L;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(PORT_SIDE_MOTOR_L, &GPIO_InitStructure);


		GPIO_InitStructure.GPIO_Pin = PIN_SIDE_MOTOR_R;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(PORT_SIDE_MOTOR_R, &GPIO_InitStructure);
#endif
*/
/*
		GPIO_InitStructure.GPIO_Pin = PIN_DUST_MOTOR;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(PORT_DUST_MOTOR, &GPIO_InitStructure);
*/
		//电机的电流控制
		GPIO_InitStructure.GPIO_Pin = PIN_MOTOR_POWER;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(PORT_MOTOR_POWER, &GPIO_InitStructure);

		//风机控制
		GPIO_InitStructure.GPIO_Pin = PIN_DUST_MOTOR_PWM;
		GPIO_Init(PORT_DUST_MOTOR_PWM, &GPIO_InitStructure);

		GPIO_InitStructure.GPIO_Pin = PIN_DUST_MOTOR_SPD;
		GPIO_Init(PORT_DUST_MOTOR_SPD, &GPIO_InitStructure);

//IC正在充电检测引脚ok
	GPIO_InitStructure.GPIO_Pin = CHARGE_ING_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(CHARGE_ING_PORT,&GPIO_InitStructure);

//IC充电完成检测引脚ok
	GPIO_InitStructure.GPIO_Pin = CHARGE_DONE_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(CHARGE_DONE_PORT,&GPIO_InitStructure);

}
/***************************************************
* Descirbe  :
* Input     :
* Output    :
* Attention :
* author 	:
***************************************************/
void NVIC_Config(void)
{
	   NVIC_InitTypeDef NVIC_InitStructure; 					 //复位NVIC寄存器为默认值


       NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);           //设置中断优先级组

/*
	   NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;		//开TIM2中断
       NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
       NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
       NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
       NVIC_Init(&NVIC_InitStructure);

	   NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;		//开TIM2中断
       NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
       NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
       NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
       NVIC_Init(&NVIC_InitStructure);

*/
		NVIC_InitStructure.NVIC_IRQChannel = TIM6_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&NVIC_InitStructure);

		NVIC_InitStructure.NVIC_IRQChannel = TIM7_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 4;
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&NVIC_InitStructure);


		NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&NVIC_InitStructure);

/**/


		NVIC_InitStructure.NVIC_IRQChannel = UART4_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&NVIC_InitStructure);



		//	外部中断

		//码盘
        NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);                          //选择中断分组2


		NVIC_InitStructure.NVIC_IRQChannel = EXTI1_IRQn;     //选择中断通道2
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; //抢占式中断优先级设置为0
        NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;        //响应式中断优先级设置为0
        NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;                                   //使能中断
        NVIC_Init(&NVIC_InitStructure);


        NVIC_InitStructure.NVIC_IRQChannel = EXTI4_IRQn;            //选择中断通道3
        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1; //抢占式中断优先级设置为1
        NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;  //响应式中断优先级设置为1
        NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;                                   //使能中断
        NVIC_Init(&NVIC_InitStructure);
 		 /*
		//红外接收
		NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x3;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x3;
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&NVIC_InitStructure);
       */
		NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x04;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x04;
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&NVIC_InitStructure);


		 NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);

		NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&NVIC_InitStructure);

		NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&NVIC_InitStructure);
}
/***************************************************
* Descirbe  :
* Input     :
* Output    :
* Attention :
* author 	:
***************************************************/

void Delayus(int t)
{
   while(t--)
   {
   }
}
/*
void timer2_cfg(void)
{

	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;

  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
   TIM_InternalClockConfig(TIM2);

	TIM_TimeBaseStructure.TIM_Period = 0xFFFF; //设置自动装载寄存器//最大测量65000us =65ms
	//分频计数 72分频，主时钟为72M，分频后，就是1M,则，一个时钟，就是1us，也就是计数器加1，就是一us
	TIM_TimeBaseStructure.TIM_Prescaler = (720000-1);

	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; //选择向上计数
	TIM_TimeBaseStructure.TIM_RepetitionCounter =0;
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
	TIM_Cmd(TIM2, ENABLE); //是能定时器
  //
 TIM_TimeBaseInitTypeDef   TIM_TimeBaseStructure;

 RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

 TIM_DeInit(TIM2);

 TIM_TimeBaseStructure.TIM_Period=0xFFFF;		 //ARR的值
 TIM_TimeBaseStructure.TIM_Prescaler=0;
 TIM_TimeBaseStructure.TIM_ClockDivision=TIM_CKD_DIV1; //采样分频
 TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up; //向上计数模式
 TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
 TIM_PrescalerConfig(TIM2,71999,TIM_PSCReloadMode_Immediate);//时钟分频系数72000，所以定时器时钟为1K
 TIM_ARRPreloadConfig(TIM2, DISABLE);//禁止ARR预装载缓冲器
 TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE);

 TIM_Cmd(TIM2, ENABLE);	//开启时钟	  //
}

 */

void TargetSysReset(void)
{
 __set_FAULTMASK(1);
 SCB->AIRCR = 0x05FA0000 | (u32)0x04;
 while(1);
}

#if ROOMA_GYROONLY
void soft_delay_ms(uint32_t ms_num)
{
	uint32_t jisuan = 0;

	for(uint32_t ii = 0; ii < ms_num; ii++)
	{
		jisuan = 72000;
		while(jisuan--);
	}
}

/*******************************************************************************
* Function Name  : RTC_Configuration
* Description    : 来重新配置RTC和BKP，仅在检测到后备寄存器数据丢失时使用
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void RTC_RCCCfg(void)
{
//	uint32_t rtc_count = 0;
	//启用PWR和BKP的时钟（from APB1）
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);

	//后备域解锁
	PWR_BackupAccessCmd(ENABLE);

	//备份寄存器模块复位
	BKP_DeInit();
#if 1
	//外部32.768K其哟偶那个
	RCC_LSEConfig(RCC_LSE_ON);
	//等待稳定
	while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET);//i ++;
	//log_printf("[RTC_RCCCfg] 4 %ld\r\n",i);

	//RTC时钟源配置成LSE（外部32.768K）
	RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);//这里使用LSI作RTC时钟源，因此工程用不到RTC,只用到后备RAM

	//RTC开启
	RCC_RTCCLKCmd(ENABLE);
	//开启后需要等待APB1时钟与RTC时钟同步，才能读写寄存器
	RTC_WaitForSynchro();
	//读写寄存器前，要确定上一个操作已经结束
	RTC_WaitForLastTask();
	//设置RTC分频器，使RTC时钟为1Hz
	//RTC period = RTCCLK/RTC_PR = (32.768 KHz)/(32767+1)
	RTC_SetPrescaler(32767);
#else
	//外部32.768K其哟偶那个
	//RCC_LSEConfig(RCC_LSE_ON);
	RCC_LSICmd(ENABLE);
	//等待稳定
	while (RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET);//i ++;
	//log_printf("[RTC_RCCCfg] 4 %ld\r\n",i);

	//RTC时钟源配置成LSE（外部32.768K）
	RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);//这里使用LSI作RTC时钟源，因此工程用不到RTC,只用到后备RAM

	//RTC开启
	RCC_RTCCLKCmd(ENABLE);

	//开启后需要等待APB1时钟与RTC时钟同步，才能读写寄存器
	RTC_WaitForSynchro();

	//读写寄存器前，要确定上一个操作已经结束
	RTC_WaitForLastTask();

	//设置RTC分频器，使RTC时钟为1Hz
	//RTC period = RTCCLK/RTC_PR = (32.768 KHz)/(32767+1)
	RTC_SetPrescaler(39999);
#endif

	//等待寄存器写入完成
	RTC_WaitForLastTask();

	RCC_RTCCLKCmd(ENABLE);
	//等待RTC时钟与APB1时钟同步
	RTC_WaitForSynchro();

	//使能秒中断
	//RTC_ITConfig(RTC_IT_SEC, ENABLE);
	//使能闹钟中断
	//RTC_ITConfig(RTC_IT_ALR, ENABLE);// | RTC_IT_SEC  

	//等待写入完成
	RTC_WaitForLastTask();


	return;
}
/*******************************************************************************
* Function Name  : RTC_Config
* Description    : 上电时调用本函数，自动检查是否需要RTC初始化，
* 					若需要重新初始化RTC，则调用RTC_Configuration()完成相应操作
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void RTC_Config(void)
{
	//我们在BKP的后备寄存器1中，存了一个特殊字符0xA5A5
	//第一次上电或后备电源掉电后，该寄存器数据丢失，
	//表明RTC数据丢失，需要重新配置
	if (BKP_ReadBackupRegister(BKP_DR1) != 0xA5A5)
	{
		log_printf("rtc power lost\r\n");
		//重新配置RTC
		RTC_RCCCfg();
		//配置完成后，向后备寄存器中写特殊字符0xA5A5
		BKP_WriteBackupRegister(BKP_DR1, 0xA5A5);
	}
	else
	{
		//若后备寄存器没有掉电，则无需重新配置RTC
		//这里我们可以利用RCC_GetFlagStatus()函数查看本次复位类型
		if (RCC_GetFlagStatus(RCC_FLAG_PORRST) != RESET)
		{
			//这是上电复位
		}
		else if (RCC_GetFlagStatus(RCC_FLAG_PINRST) != RESET)
		{
			//这是外部RST管脚复位
		}
		//清除RCC中复位标志
		RCC_ClearFlag();

		//虽然RTC模块不需要重新配置，且掉电后依靠后备电池依然运行
		//但是每次上电后，还是要使能RTCCLK???????
		RCC_RTCCLKCmd(ENABLE);
		//等待RTC时钟与APB1时钟同步
		RTC_WaitForSynchro();

		//使能秒中断
		//RTC_ITConfig(RTC_IT_SEC, ENABLE);
		//使能闹钟中断
		//RTC_ITConfig(RTC_IT_ALR, ENABLE);// | RTC_IT_SEC
		//等待操作完成
		RTC_WaitForLastTask();
	}

	return;
}

void RTC_Init(void)
{
	RTC_Config();
}

#endif

