/********************************************************************
* 版权 ： -------------micony--------------
* 文件 :motor.c
* 版本 ：
* 描述 ：
* 作者 : 韦云智
* 时间 ：2015-2-13
* 说明 ：
*/
#include "sys.h"


motor_t motor;

char motorStr[7][6]={"STOP","FORW","BACK","LEFT","RIGT","RUND","SLOW"};


__IO uint16_t *left_pwm,*right_pwm;

/*
清扫电机
*/

//arr：自动重装值
//psc：时钟预分频数
/*
左边刷电机	53	PE14	左边刷电机PWM驱动   TIM1_CH4
	54	PE13 =>改成其他的	左边刷电流检测（H/L电平检测）
右边刷电机	55	PE9=>PE11	右边刷电机PWM驱动
	56	PE11 =>PE9	右边刷过流检测（H/L电平检测）
时钟初始化，定时器初始化，定时器1.用于
*/
void timer1_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;

	TIM_DeInit(TIM1);//初始化TIM1寄存器
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1 | RCC_APB2Periph_GPIOE  | RCC_APB2Periph_AFIO, ENABLE);  //使能GPIO外设和AFIO复用功能模块时钟

	GPIO_PinRemapConfig(GPIO_FullRemap_TIM1, ENABLE); //Timer1完全重映射

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14; //TIM1_CH1
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  //复用推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOE, &GPIO_InitStructure);//初始化GPIO

	/*
	GPIO_InitStructure.GPIO_Pin =  LIBAT_CHPWM_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

	GPIO_Init(LIBAT_CHPWM_PORT, &GPIO_InitStructure);
	LIBAT_CHPWM_PORT->ODR &= ~LIBAT_CHPWM_PIN;
	*/
	//GPIO_ResetBits(LIBAT_CHPWM_PORT, LIBAT_CHPWM_PIN);

    //初始化TIM1
	TIM_TimeBaseStructure.TIM_Period = MAX_PWM; //设置在下一个更新事件装入活动的自动重装载寄存器周期的值
	TIM_TimeBaseStructure.TIM_Prescaler =(4-1); //设置用来作为TIMx时钟频率除数的预分频值
	TIM_TimeBaseStructure.TIM_ClockDivision = 0; //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
	TIM_TimeBaseStructure.TIM_RepetitionCounter = 0x0;
	TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure); //根据TIM_TimeBaseInitStruct中指定的参数初始化TIMx的时间基数单位


	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1; //选择定时器模式:TIM脉冲宽度调制模式1
 	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; //比较输出使能
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; //输出极性:TIM输出比较极性高
	TIM_OCInitStructure.TIM_Pulse =0;
	/*
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low; //输出极性:TIM输出比较极性高
	//初始化TIM1 Channel1 PWM模式
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1; //选择定时器模式:TIM脉冲宽度调制模式1
 	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; //比较输出使能
	//TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; //输出极性:TIM输出比较极性高
	TIM_OCInitStructure.TIM_Pulse =1000;*/
	TIM_OC4Init(TIM1, &TIM_OCInitStructure);  //根据T指定的参数初始化外设TIM1 OC1

/*
	//充电PWM
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_Pulse = 0;
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low;
	TIM_OC3Init(LIBAT_CHPWM_TIM, &TIM_OCInitStructure);
	TIM_OC3PreloadConfig(LIBAT_CHPWM_TIM, TIM_OCPreload_Enable);

	*/

	//TIM_OC2PreloadConfig(TIM1, TIM_OCPreload_Enable);  //使能TIM1在CCR1上的预装载寄存器
	TIM_OC4PreloadConfig(TIM1, TIM_OCPreload_Enable);  //使能TIM1在CCR1上的预装载寄存器
 	TIM_ARRPreloadConfig(TIM1, ENABLE);                //使能TIMx在ARR上的预装载寄存器
	TIM_Cmd(TIM1, ENABLE);  //使能TIM1
	TIM_CtrlPWMOutputs(TIM1, ENABLE);//设置PMW主输出
	//TIM1->CCR2=700;
	//TIM1->CCR4=700;
	//while(1);

	//EN_TM1_CH1();
	//EN_TM1_CH4();
	//TIM1->CCR1=500;
	//TIM1->CCR4=500;

}

void timer2_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;

	TIM_DeInit(TIM2);//初始化TIM1寄存器
 	RCC_APB1PeriphClockCmd(RCC_APB2Periph_GPIOB  | RCC_APB2Periph_AFIO, ENABLE);  //使能GPIO外设和AFIO复用功能模块时钟

	GPIO_PinRemapConfig(GPIO_FullRemap_TIM2, ENABLE); //Timer2完全重映射

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  //复用推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);//初始化GPIO

	// 72M /72 / 500 = 2k
	TIM_TimeBaseStructure.TIM_Period = 500; //设置在下一个更新事件装入活动的自动重装载寄存器周期的值
	TIM_TimeBaseStructure.TIM_Prescaler =(72-1); //设置用来作为TIMx时钟频率除数的预分频值
	TIM_TimeBaseStructure.TIM_ClockDivision = 0; //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
	TIM_TimeBaseStructure.TIM_RepetitionCounter = 0x0;
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure); //根据TIM_TimeBaseInitStruct中指定的参数初始化TIMx的时间基数单位

	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1; //选择定时器模式:TIM脉冲宽度调制模式1
 	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; //比较输出使能
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; //输出极性:TIM输出比较极性高
	TIM_OCInitStructure.TIM_Pulse =0;

	TIM_OC4Init(TIM2, &TIM_OCInitStructure);  //根据T指定的参数初始化外设TIM1 OC1

	TIM_OC4PreloadConfig(TIM2, TIM_OCPreload_Enable);  //使能TIM1在CCR1上的预装载寄存器
 	TIM_ARRPreloadConfig(TIM2, ENABLE);                //使能TIMx在ARR上的预装载寄存器
	TIM_Cmd(TIM2, ENABLE);  //使能TIM1
	//TIM_CtrlPWMOutputs(TIM2, ENABLE);//设置PMW主输出
}

//int ccr3_1=0;

//lock_Init(9); //系统时钟设置 delay_init(72); //延时初始化
/*
脉冲宽度调制模式可以产生一个由TIMx_ARR寄存器确定频率、由TIMx_CCRx寄存器确定占空比的信号。
在TIMx_CCMRx寄存器中的OCxM位写入’110’(PWM 模式1) 或’111’(PWM 模式2) ，能够独立地设置每个OCx输出通道产生一路PWM。
必须通过设置TIMx_CCMRx寄存器的OCxPE位使能相应的预装载寄存器，
最后还要设置TIMx_CR1 寄存器的ARPE 位，( 在向上计数或中心对称模式中)使能自动重装载的预装载寄存器。
仅当发生一个更新事件的时候，预装载寄存器才能被传送到影子寄存器，
因此在计数器开始计数之前，必须通过设置TIMx_EGR寄存器中的UG位来初始化所有的寄存器。
OCx的极性可以通过软件在TIMx_CCER寄存器中的CCxP位设置，它可以设置为高电平有效或低电平有效。
OCx的输出使能通过(TIMx_CCER和TIMx_BDTR寄存器中)CCxE、CCxNE、MOE、OSSI和OSSR 位的组合控制。
详见TIMx_CCER寄存器的描述。在PWM模式(模式1或模式2) 下，TIMx_CNT和TIMx_CCRx始终在进行比较，(
 依据计数器的计数方向)以确定是否符合TIMx_CCRx≤TIMx_CNT或者TIMx_CNT≤TIMx_CCRx。
 根据TIMx_CR1 寄存器中CMS位的状态，定时器能够产生边沿对齐的PWM信号或中央对齐的PWM信号。

根据以上信息的提示，我们可以归纳出寄存器操作的步骤：

（1） 使能相应的定时器时钟线。

（2） 配置tim1四路pwm输出引脚为复用功能输出。

（3） 配置tim1的分频比和计数装载值。

（4） 配置相应通道的pwm输出模式和使能预装载。

（5） 打开自动重装载预装载允许位。

（6） 根据需要选择向上或者是向下计数模式。

（7） 打开相应通道的输出使能。

（8） 配置各通道的占空比。

（9） 开启开启OC和OCN。

（10） 最后使能定时器1.

关于分频比和计数装载值的选择：

Tim1的最大时钟频率为72M，假设我们想得到一个频率为4KHZ，占空比0~100%步进为1的pwm。
根据计数器的时钟频率(CK_CNT) 等于f CK_PSC/( PSC[15:0]+1)。
因为占空比0~100%步进为1，所以我们把计数值设为100.
比较值可以设为0~100之间，实现占空比的调节。
计数器的时钟频率(CK_CNT)应为4KHZ X 100=400KHZ。所以PSC = 179。ARR=100。有不明白的请看手册。

*/

void motor_timer_init(void)
{

 TIM_TimeBaseInitTypeDef   TIM_TimeBaseStructure;
 //  TIM_ICInitTypeDef  	   TIM_ICInitStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;


     /* TIM4 clock enable */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);

  /* GPIOD clock enable */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

  TIM_ARRPreloadConfig(TIM4, ENABLE);





	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_6 | GPIO_Pin_7| GPIO_Pin_8| GPIO_Pin_9;
	  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	  GPIO_Init(GPIOB, &GPIO_InitStructure);

	TIM_TimeBaseStructure.TIM_Period = MAX_PWM;

  TIM_TimeBaseStructure.TIM_Prescaler = (4-1);


  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_CounterMode =TIM_CounterMode_Down;

  TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);

  /* PWM1 Mode configuration: Channel1 */
  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
  TIM_OCInitStructure.TIM_Pulse = 800;
  TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;

  TIM_OC1Init(TIM4, &TIM_OCInitStructure);
  TIM_OC2Init(TIM4, &TIM_OCInitStructure);
  TIM_OC3Init(TIM4, &TIM_OCInitStructure);
  TIM_OC4Init(TIM4, &TIM_OCInitStructure);

  TIM_OC1PreloadConfig(TIM4, TIM_OCPreload_Enable);
  TIM_OC2PreloadConfig(TIM4, TIM_OCPreload_Enable);
  TIM_OC3PreloadConfig(TIM4, TIM_OCPreload_Enable);
  TIM_OC4PreloadConfig(TIM4, TIM_OCPreload_Enable);


  /* TIM4 enable counter */
  TIM_Cmd(TIM4, ENABLE);

	l_motor_stop();
	r_motor_stop();

}

void l_motor_stop(void)
{
	DIS_CH3();
	DIS_CH4();
}

void l_motor_set_pwm(uint8_t HL,int pwm)
{
	EN_CH3();
	EN_CH4();
	if(HL==GO_FORWARD)
	{
#if(1 == MOTOR_ZHENG)
		TIM4->CCR4=0;			//高电平
		TIM4->CCR3=MAX_PWM -pwm;
#else
		TIM4->CCR3=0;			//高电平
		TIM4->CCR4=MAX_PWM -pwm;
#endif
	}else
	{
#if(1 == MOTOR_ZHENG)
		TIM4->CCR3=0;			//高电平
		TIM4->CCR4=MAX_PWM -pwm;
#else
		TIM4->CCR4=0;			//高电平
		TIM4->CCR3=MAX_PWM -pwm;
#endif
	}

	sys->pwm1 = pwm;
}

// motor2 input1：PA6 input2：PA7  PWM:PA9
void r_motor_stop(void)
{
	DIS_CH1();
	DIS_CH2();
}

void motor_wheel_backward(u8 wheel,u16 speed)
{
	TIM_OCInitTypeDef  TIM_OCInitStruct;
	TIM_OCInitStruct.TIM_Pulse = speed;
	TIM_OCInitStruct.TIM_OCPolarity = TIM_OCPolarity_High ;
#if(1 == MOTOR_ZHENG)
	if( READ_BIT(wheel,RIGHT_WHEEL))
	{
		TIM_OCInitStruct.TIM_OCMode = TIM_OCMode_PWM1;
		TIM_OC4Init(TIM4,&TIM_OCInitStruct);
		TIM_OCInitStruct.TIM_OCMode = TIM_ForcedAction_Active;
		TIM_OC3Init(TIM4,&TIM_OCInitStruct);
		TIM_CCxCmd(TIM4,TIM_Channel_3,TIM_CCx_Enable);
		TIM_CCxCmd(TIM4,TIM_Channel_4,TIM_CCx_Enable);
		right_pwm= &(TIM4->CCR4);
	}
	if(READ_BIT(wheel,LEFT_WHEEL))
	{
		TIM_OCInitStruct.TIM_OCMode = TIM_OCMode_PWM1;
		TIM_OC2Init(TIM4,&TIM_OCInitStruct);
		TIM_OCInitStruct.TIM_OCMode = TIM_ForcedAction_Active;
		TIM_OC1Init(TIM4,&TIM_OCInitStruct);
		TIM_CCxCmd(TIM4,TIM_Channel_1,TIM_CCx_Enable);
		TIM_CCxCmd(TIM4,TIM_Channel_2,TIM_CCx_Enable);
		left_pwm  = &(TIM4->CCR2);
	}
#else
	if( READ_BIT(wheel,RIGHT_WHEEL))
	{
		TIM_OCInitStruct.TIM_OCMode = TIM_OCMode_PWM1;
		TIM_OC3Init(TIM4,&TIM_OCInitStruct);
		TIM_OCInitStruct.TIM_OCMode = TIM_ForcedAction_Active;
		TIM_OC4Init(TIM4,&TIM_OCInitStruct);
		TIM_CCxCmd(TIM4,TIM_Channel_4,TIM_CCx_Enable);
		TIM_CCxCmd(TIM4,TIM_Channel_3,TIM_CCx_Enable);
		right_pwm= &(TIM4->CCR3);
	}
	if(READ_BIT(wheel,LEFT_WHEEL))
	{
		TIM_OCInitStruct.TIM_OCMode = TIM_OCMode_PWM1;
		TIM_OC1Init(TIM4,&TIM_OCInitStruct);
		TIM_OCInitStruct.TIM_OCMode = TIM_ForcedAction_Active;
		TIM_OC2Init(TIM4,&TIM_OCInitStruct);
		TIM_CCxCmd(TIM4,TIM_Channel_2,TIM_CCx_Enable);
		TIM_CCxCmd(TIM4,TIM_Channel_1,TIM_CCx_Enable);
		left_pwm  = &(TIM4->CCR1);
	}
#endif
}


void motor_wheel_forward(u8 wheel,u16 speed)
{
	TIM_OCInitTypeDef  TIM_OCInitStruct;



	TIM_OCInitStruct.TIM_Pulse = speed;
	TIM_OCInitStruct.TIM_OCPolarity = TIM_OCPolarity_High ;

#if(1 == MOTOR_ZHENG)
	if( READ_BIT(wheel,RIGHT_WHEEL)){

		TIM_OCInitStruct.TIM_OCMode = TIM_OCMode_PWM1;
		TIM_OC3Init(TIM4,&TIM_OCInitStruct);

		TIM_OCInitStruct.TIM_OCMode = TIM_ForcedAction_Active;
		TIM_OC4Init(TIM4,&TIM_OCInitStruct);
		TIM_CCxCmd(TIM4,TIM_Channel_3,TIM_CCx_Enable);
		TIM_CCxCmd(TIM4,TIM_Channel_4,TIM_CCx_Enable);
		 right_pwm= &(TIM4->CCR3);
	}

	if(READ_BIT(wheel,LEFT_WHEEL)){

		TIM_OCInitStruct.TIM_OCMode = TIM_OCMode_PWM1;
		TIM_OC1Init(TIM4,&TIM_OCInitStruct);

		TIM_OCInitStruct.TIM_OCMode = TIM_ForcedAction_Active;
		TIM_OC2Init(TIM4,&TIM_OCInitStruct);
		TIM_CCxCmd(TIM4,TIM_Channel_1,TIM_CCx_Enable);
		TIM_CCxCmd(TIM4,TIM_Channel_2,TIM_CCx_Enable);
		left_pwm = &(TIM4->CCR1);
	}
#else
	if( READ_BIT(wheel,RIGHT_WHEEL)){

		TIM_OCInitStruct.TIM_OCMode = TIM_OCMode_PWM1;
		TIM_OC4Init(TIM4,&TIM_OCInitStruct);

		TIM_OCInitStruct.TIM_OCMode = TIM_ForcedAction_Active;
		TIM_OC3Init(TIM4,&TIM_OCInitStruct);
		TIM_CCxCmd(TIM4,TIM_Channel_3,TIM_CCx_Enable);
		TIM_CCxCmd(TIM4,TIM_Channel_4,TIM_CCx_Enable);
		 right_pwm= &(TIM4->CCR4);
	}

	if(READ_BIT(wheel,LEFT_WHEEL)){

		TIM_OCInitStruct.TIM_OCMode = TIM_OCMode_PWM1;
		TIM_OC2Init(TIM4,&TIM_OCInitStruct);

		TIM_OCInitStruct.TIM_OCMode = TIM_ForcedAction_Active;
		TIM_OC1Init(TIM4,&TIM_OCInitStruct);
		TIM_CCxCmd(TIM4,TIM_Channel_2,TIM_CCx_Enable);
		TIM_CCxCmd(TIM4,TIM_Channel_1,TIM_CCx_Enable);
		left_pwm = &(TIM4->CCR2);
	}
#endif
}


void motor_wheel_stop(u8 wheel)
{
	TIM_OCInitTypeDef  TIM_OCInitStruct;



	TIM_OCInitStruct.TIM_Pulse = 0;
	TIM_OCInitStruct.TIM_OCPolarity = TIM_OCPolarity_High ;



	if( READ_BIT(wheel,RIGHT_WHEEL)){

		TIM_OCInitStruct.TIM_OCMode = TIM_ForcedAction_Active;
		TIM_OC3Init(TIM4,&TIM_OCInitStruct);

		TIM_OCInitStruct.TIM_OCMode = TIM_ForcedAction_Active;
		TIM_OC4Init(TIM4,&TIM_OCInitStruct);
		TIM_CCxCmd(TIM4,TIM_Channel_3,TIM_CCx_Enable);
		TIM_CCxCmd(TIM4,TIM_Channel_4,TIM_CCx_Enable);
		//*right_pwm =1000;

	}

	if(READ_BIT(wheel,LEFT_WHEEL)){

		TIM_OCInitStruct.TIM_OCMode = TIM_ForcedAction_Active;
		TIM_OC1Init(TIM4,&TIM_OCInitStruct);

		TIM_OCInitStruct.TIM_OCMode = TIM_ForcedAction_Active;
		TIM_OC2Init(TIM4,&TIM_OCInitStruct);
		TIM_CCxCmd(TIM4,TIM_Channel_1,TIM_CCx_Enable);
		TIM_CCxCmd(TIM4,TIM_Channel_2,TIM_CCx_Enable);
		//*left_pwm=1000;
	}
}

void r_motor_set_pwm(uint8_t HL,int pwm)
{

	EN_CH1();
	EN_CH2();
	if(HL==GO_FORWARD)
	{
#if(1 == MOTOR_ZHENG)
		TIM4->CCR2=0;			//高电平
		TIM4->CCR1=MAX_PWM - pwm;
#else
		TIM4->CCR1=0;			//高电平
		TIM4->CCR2=MAX_PWM - pwm;
#endif
	}else
	{
#if(1 == MOTOR_ZHENG)
		TIM4->CCR1=0;			//高电平
		TIM4->CCR2=MAX_PWM - pwm;
#else
		TIM4->CCR2=0;			//高电平
		TIM4->CCR1=MAX_PWM - pwm;
#endif
	}

//	sys->pwm2 = MAX_PWM - pwm;

	//TIM1->CCR2=pwm;

}
/*
    输入捕获模式可以用来测量脉冲宽度或者测量频率。STM32的定时器，除了TIM6和TIM7，其他定时器都有输入捕获功能。STM32的输入捕获，简单的说就是通过检测TIMx_CHx上的边沿信号，在边沿信号发生跳变（比如上升沿/下降沿）的时候，将当前定时器的值（TIMx_CNT）存放到对应的通道的捕获/比较寄存（TIMx_CCRx）里面，完成一次捕获。同时还可以配置捕获时是否触发中断/DMA 等.

     例如：我们用到TIM5_CH1来捕获高电平脉宽，也就是要先设置输入捕获为上升沿检测，记录发生上升沿的时候TIM5_CNT的值。然后配置捕获信号为下降沿捕获，当下降沿到来时，发生捕获，并记录此时的TIM5_CNT值。这样，前后两次TIM5_CNT之差，就是高电平的脉宽，同时TIM5的计数频率我们是知道的，从而可以计算出高电平脉宽的准确时间。

     首先TIMx_ARR和TIMx_PSC，这两个寄存器用来设自动重装载值和TIMx的时钟分频。

     再来看看捕获/比较模式寄存器1：TIMx_CCMR1，这个寄存器在输入捕获的时候，非常有用；TIMx_CCMR1明显是针对2个通道的配置，低八位[7：0]用于捕获/比较通道1的控制，而高八位[15：8]则用于捕获/比较通道2的控制，因为TIMx还有CCMR2这个寄存器，所以可以知道CCMR2是用来控制通道3和通道4（详见《STM32参考手册》290页，14.4.8节）。

    这里用到TIM5的捕获/比较通道1，我们重点介绍TIMx_CMMR1的[7:0]位（其实高8位配置类似）。

    再来看看捕获/比较使能寄存器：TIMx_CCER；

    接下来我们再看看DMA/中断使能寄存器：TIMx_DIER，我们需要用到中断来处理捕获数据，所以必须开启通道1的捕获比较中断，即CC1IE设置为1。

    控制寄存器：TIMx_CR1，我们只用到了它的最低位，也就是用来使能定时器的；

    最后再来看看捕获/比较寄存器1：TIMx_CCR1，该寄存器用来存储捕获发生时，TIMx_CNT的值，我们从TIMx_CCR1就可以读出通道1捕获发生时刻的TIMx_CNT值，通过两次捕获（一次上升沿捕获，一次下降沿捕获）的差值，就可以计算出高电平脉冲的宽度。

    使能捕获和更新中断（设置TIM5的DIER寄存器）
   因为我们要捕获的是高电平信号的脉宽，所以，第一次捕获是上升沿，第二次捕获时下降沿，必须在捕获上升沿之后，设置捕获边沿为下降沿，同时，如果脉宽比较长，那么定时器就会溢出，对溢出必须做处理，否则结果就不准了。这两件事，我们都在中断里面做，所以必须开启捕获中断和更新中断。
*/
#if 0
#define  TIMER TIM2
void init_tim2_camp(u16 psc, u16 arr, u8 way, u8 dir)
{
	RCC->APB1ENR |= 1 << 0;   //使能定时器2时钟
	RCC->APB2ENR |= 1 << 2; //使能PortA
/*
	switch (way)
	{
		case 1:
			GPIOA->CRL &= 0xfffffff0;	  	//PA0
			GPIOA->CRL |= 0x00000008;
			break;
		case 2:
	*/
			GPIOA->CRL &= 0xffffff00;   //PA0 PA1
			GPIOA->CRL |= 0x00000088;
	/*
			break;
		case 3:
			GPIOA->CRL &= 0xfffff000;
			GPIOA->CRL |= 0x00000888;
			break;
		case 4:
			GPIOA->CRL &= 0xffff0000;
			GPIOA->CRL |= 0x00008888;
			break;
	}
	*/

	TIMER->PSC = psc;
	TIMER->ARR = arr;

	switch (way)
	{
		case 4:
			TIMER->CCMR2 |= 1 << 8;
			if (dir == 0)
				TIMER->CCER |= 1 << 13; //下降沿捕获
			else
				TIMER->CCER &= ~(1 << 13); //上升沿捕获
			TIMER->CCER |= 1 << 12;
			TIMER->DIER |= 1 << 4;
		case 3: //CCR3 PA2
			TIMER->CCMR2 |= 1 << 0;
			if (dir == 0)
				TIMER->CCER |= 1 << 9; //下降沿捕获
			else
				TIMER->CCER &= ~(1 << 9); //上升沿捕获
			TIMER->CCER |= 1 << 8;
			TIMER->DIER |= 1 << 3;
		case 2: //CCR2 PA1
			TIMER->CCMR1 |= 1 << 8; //CCR2配置通道方向:输入
			TIMER->CCMR1 |= 5 << 12;		//滤波器设为5
			if (dir == 0)
				TIMER->CCER |= 1 << 5; //下降沿捕获
			else
				TIMER->CCER &= ~(1 << 5); //上升沿捕获
			TIMER->CCER |= 1 << 4; //CCR2通道捕获使能
			TIMER->DIER |= 1 << 2; //CCR2通道允许捕获中断
		case 1: //>CCR1 PA0
			TIMER->CCMR1 |= 1 << 0; //CCR1配置通道方向：输入
			TIMER->CCMR1 |= 5 << 4;		//滤波器设为5
			if (dir == 0)
				TIMER->CCER |= 1 << 1; //下降沿捕获
			else
				TIMER->CCER &= ~(1 << 1); //上升沿捕获
			TIMER->CCER |= 1 << 0;     //CCR1捕获使能
			TIMER->DIER |= 1 << 1;  //CCR1通道允许捕获中断
			break;
	}
	TIMER->DIER |= 1 << 0;     //允许更新中断
//	MY_NVIC_Init(1, 2, TIM2_IRQChannel, 2);     //中断
	TIMER->CR1 = 0x01;    //使能定时器
	TIMER->SR &= ~(1 << 0);
}


void init_tim4_camp(u16 psc, u16 arr, u8 way, u8 dir)
{
	GPIO_InitTypeDef GPIO_InitStructure;
// TIM_TimeBaseInitTypeDef   TIM_TimeBaseStructure;
// TIM_ICInitTypeDef  	   TIM_ICInitStructure;

	//PB8 TIM4 CH3   pb8 TIM4 ch4 暂时用于红外的接受
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_8;// | GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IN_FLOATING;//悬空输入
	GPIO_Init(GPIOB,&GPIO_InitStructure);

	TIM4->PSC = psc;
	TIM4->ARR = arr;
  /*
	switch (way)
	{
		case 4:


			//CH4
			TIMER->CCMR2 |= 1 << 8;
			TIMER->CCMR2 |= 5 << 12;		//滤波器设为5
			if (dir == 0)
				TIMER->CCER |= 1 << 13; //下降沿捕获
			else
				TIMER->CCER &= ~(1 << 13); //上升沿捕获
			TIMER->CCER |= 1 << 12;
			TIMER->DIER |= 1 << 4;

			*/
		//	CH3
			TIM4->CCMR2 |= 1 << 0;
			TIM4->CCMR2 |= 5 << 4;		//滤波器设为5
			if (dir == 0)
				TIM4->CCER |= 1 << 9; //下降沿捕获
			else
				TIM4->CCER &= ~(1 << 9); //上升沿捕获
			TIM4->CCER |= 1 << 8;
			TIM4->DIER |= 1 << 3;
		/*
		case 2: //CCR2 PA1
			TIMER->CCMR1 |= 1 << 8; //CCR2配置通道方向:输入
			TIMER->CCMR1 |= 5 << 12;		//滤波器设为5
			if (dir == 0)
				TIMER->CCER |= 1 << 5; //下降沿捕获
			else
				TIMER->CCER &= ~(1 << 5); //上升沿捕获
			TIMER->CCER |= 1 << 4; //CCR2通道捕获使能
			TIMER->DIER |= 1 << 2; //CCR2通道允许捕获中断
		case 1: //>CCR1 PA0
			TIMER->CCMR1 |= 1 << 0; //CCR1配置通道方向：输入
			TIMER->CCMR1 |= 5 << 4;		//滤波器设为5
			if (dir == 0)
				TIMER->CCER |= 1 << 1; //下降沿捕获
			else
				TIMER->CCER &= ~(1 << 1); //上升沿捕获
			TIMER->CCER |= 1 << 0;     //CCR1捕获使能
			TIMER->DIER |= 1 << 1;  //CCR1通道允许捕获中断
			break;
	}
	 */
	TIM4->DIER |= 1 << 0;     //允许更新中断
//	MY_NVIC_Init(1, 2, TIM2_IRQChannel, 2);     //中断
	TIM4->CR1 = 0x01;    //使能定时器
	TIM4->SR &= ~(1 << 0);

}

#endif
/*

转圈函数
*/
/*
void motor_turn(uint8_t RL,int cHw)
{
	if(RL==TURN_R)		//右拐
	{
		l_motor_stop();  	//右电机停止
		motor2_set_pwm(GO_FORWARD,TURN_PWM);	//左电机转
		//c_hw1=0;
		c_hw=cHw;	  //转的霍尔数
		sys->mState = MST_RIGHT;
	}else if(RL == TURN_L)
	{
 		r_motor_stop();  	//右电机停止
		motor1_set_pwm(GO_FORWARD,TURN_PWM);	//左电机转
		//c_hw2=0;
		c_hw=cHw;	  //转的霍尔数
		sys->mState = MST_LEFT;
	}
}
*/
/*
	dir:方向
	pwm:pwm
	cHw:运动的霍尔数，会换算成圈数，没有节制的运动，为0
	next:下一个动作，霍尔数完后的动作
*/

void motor_turn(uint8_t dir,int pwm,float agle)
{
//	int pwm=600;
	//int dg_turn=deg - 2.5;
//	int c;
	float dis_agle,b_agle;
	log_printf("motor_turn,dir=%s,agle=%f\r\n",get_motor_sta(dir),agle);
	motor.c_left_hw = motor.c_right_hw = 0;
	www_idleintel_com();
	b_agle= format_agle(sys->angle, ANGLE_360);
	motor_run(dir,pwm,0,0);
	while(1)
	{
		delay_ms(5);
		www_idleintel_com();

		dis_agle = b_agle -format_agle(sys->angle, ANGLE_360);
		if(dis_agle > 180)
			dis_agle = 360 - dis_agle;
		if(dis_agle <-180)
			dis_agle += 360;
		if(dis_agle >=agle || dis_agle <=-agle)
			break;

		//log_printf("%3.1f,%3.1f\r\n",format_agle(sys->angle, ANGLE_360),dis_agle);
	}
	motor_run(GO_STOP,0,0,0);
	/*
	TIM5->CNT =0;
	c=0;
	while(1)
	{
		if(TIM5->CNT >=2000)
		{
			if(motor.c_left_hw > deg)
			{
				motor_run(GO_STOP,0,0,0);
				break;
			}
		//log_printf("%d,%d,%d\r\n",c++,motor.c_left_hw,motor.c_right_hw);
		}
	}
	*/
	delay_ms(20);
	log_printf("..OK,c=%d,%d\r\n",motor.c_left_hw,motor.c_right_hw);
	motor.c_left_hw = motor.c_right_hw = 0;
	//为了找充电座直行PID

}
void motor_turn_circle(uint8_t dir,int pwm,int hw)
{
//l_motor_turn_circle:
//	float bagle = sys->angle;
	char flage = 1;
	int k=0;
	int count=0,count1=0;
	//uint16_t max_adc=0,less=0;
	float max_agle=0;

//	int hw1=0,hw2=0,lhw1=0,lhw2=0;
	//int pid_pwm;
	log_printf("[motor_turn_circle]hw=%d,dir =%d\r\n",hw,dir);
	/*
	if(dir == GO_LEFT)
	{
		r_motor_set_pwm(GO_FORWARD,600);
		l_motor_set_pwm(GO_FORWARD,700);
	}else
	{
		l_motor_set_pwm(GO_FORWARD,600);
		r_motor_set_pwm(GO_FORWARD,700);
	}
	*/
	MOTOR_POWER_ON();
	if(sys->shut_down_motor)
		MOTOR_CTRL1(NORM_SIDE_PWM_L,NORM_SIDE_PWM_R,NORM_MID_PWM,sys->dust_pwm_value)
	else
		MOTOR_CTRL(NORM_SIDE_PWM_L,NORM_SIDE_PWM_R,NORM_MID_PWM,sys->dust_pwm_value)

//	motor_run(dir,pwm,0,0);
	motor.c_left_hw = motor.c_right_hw = 0;

	motor_turn_dir(dir ,pwm,pwm);
	turn_round_pid(0);
	navigat->distance = 0;
	TIM5->CNT =0;
	navigat->is_walk = 0;

//	msTmr = 0;
	while(1)
	{
		get_sensers(&sys->gSta);
		if(TIM5->CNT >=1000)
		{
			TIM5->CNT=0;

			count++;
			count1++;
			if(count >=5)
			{
				count = 0;
				www_idleintel_com();
				turn_round_pid(TURN_SPEED);
				if(sys->angle > 180 && flage)
				{
					k ++;

					flage = 0;
					log_printf("k=%d,tmr=%d\r\n",k,msTmr / 1000);
				}
				if(sys->angle < 10)
					flage = 1;
				if(k >= hw /* & dis_float(0 , sys->angle) < 5*/)
				{
					if((dir == GO_LEFT && sys->angle > 355) || (dir == GO_RIGTH&& sys->angle < 5))
					{
						motor_run(GO_STOP,0,0,0);
						break;
					}
				}
			}
			/*

			*/
			if(count1>=2)
			{
				count1 = 0;
				/*
				log_printf("%d,%d,%d,%d,%d,%d,%d,%3.1f\r\n",sys->g_sta[0],sys->g_sta[1],sys->g_sta[2],sys->g_sta[3]
													  ,sys->g_sta[4],sys->g_sta[5],sys->g_sta[6],sys->angle);

				if(dir == GO_LEFT)
					log_printf("%d\r\n",sys->g_sta[6]);
				else
					log_printf("%d\r\n",sys->g_sta[0]);
				if(max_adc < sys->g_sta[6])
				{
					max_adc = sys->g_sta[6];
					max_agle = sys->angle;
				}
				if(sys->g_sta[6] < max_adc && sys->g_sta[6] > 500)
				{
					less++;
					if(less >=6)
					{
						motor_run(GO_STOP,0,0,0);
						break;

					}
				}else
					less = 0;
				*/
			}





		}

	}
	log_printf("count=%d,angle=%3.3f,k=%d,max=%3.3f\r\n",count,sys->angle,k,max_agle);
	//goto l_motor_turn_circle;

}

void motor_turn_360(uint8_t dir)
{
//	int pwm=600;

	int c;
	float cp_deg,cp_deg1;
	//gyro_offset_manage(300);
//	log_printf("motor_turn,begin angle=%f\r\n",sys->angle);
	motor_run(dir,600,0,0);
	if(dir == GO_LEFT)
	{
		cp_deg = 0 - sys->ajust_agle;
		cp_deg1 = 360 - sys->ajust_agle;
	}
	else
		cp_deg = sys->ajust_agle;
	TIM5->CNT =0;
	c=0;

	while(1)
	{
		if(TIM5->CNT >=2000)
		{

			TIM5->CNT=0;
			www_idleintel_com();
			if(sys->angle >=100 && sys->angle < 290)
				c=1;
			if(c)
			{
				if(dir == GO_LEFT )
				{
					if((sys->angle <10 && sys->angle >=cp_deg) || sys->angle >cp_deg1)
					{
						motor_run(GO_STOP,0,0,0);
						log_printf("stop:%f,%f\r\n",sys->angle,cp_deg);
						break;
					}
				}
				if(dir == GO_RIGTH)
				{
					if(sys->angle <=cp_deg)
					{
						motor_run(GO_STOP,0,0,0);
						log_printf("stop:%f,%f\r\n",sys->angle,cp_deg);
						break;
					}
				}
			}
		}
	}

	delay_ms(600);
	www_idleintel_com();
	log_printf("turn ok ,agle=%f,ajust=%f,%f\r\n",sys->angle,sys->ajust_agle,cp_deg);
}

#define TURN_START_SLOW			0

void motor_turn_dir(uint8_t dir ,int left_pwm,int right_pwm)
{
#if TURN_START_SLOW
	int i,start_pwm=MAX_PWM;
#endif
	//uint8_t mSta = sys->mState;

	sys->mState=dir;				//记录本次的方向
	sys->c_right_drop = sys->c_left_drop = 0;
#if	LOCKED_MOTOR_PARA
	judge_left_right_motor_locked_rotor_process(1);
#endif
	switch(dir)
	{
		case GO_RIGTH:			//左边停止，右边转
			//没有停下来
			/*
			if(mSta == GO_FORWARD)
			{
				motor_wheel_stop(RIGHT_WHEEL);
			}else
			*/
			{
				motor_wheel_backward(RIGHT_WHEEL, 900);
				motor_wheel_forward(LEFT_WHEEL, 900);
				delay_ms(50);

				motor_wheel_backward(RIGHT_WHEEL, right_pwm);
				motor_wheel_forward(LEFT_WHEEL, left_pwm);
			}
			sys->left_dir = GO_FORWARD;
			sys->right_pwm = sys->left_pwm = left_pwm;
			break;
		case GO_LEFT:
		/*
			if(mSta == GO_FORWARD)
			{
				motor_wheel_stop(LEFT_WHEEL);
			}else
			*/
			{
				motor_wheel_backward(LEFT_WHEEL, 900);
				motor_wheel_forward(RIGHT_WHEEL, 900);
				delay_ms(50);


				motor_wheel_backward(LEFT_WHEEL, left_pwm);
				motor_wheel_forward(RIGHT_WHEEL, right_pwm);
			}
			sys->left_dir = GO_BACK;
			sys->right_pwm = sys->left_pwm = left_pwm;
			break;

	}
	//sys->mState = dir;
}

void motor_run(uint8_t dir ,int pwm,int cHw,uint8_t next)
{
	//前进或倒退，两个轮子都转
	log_printf("[motor run]%s,pwm=%d\r\n",&motorStr[dir][0],pwm);
	//test
	//if(dir !=GO_STOP)
	//motor.c_right_hw = motor.c_left_hw =0;
//	sys->trg_speed=sys->trg_speed2=0;
	//int start_pwm;
	sys->nearwall_signal_lost = RESET;		//沿边丢失的标记，每次运动的时候，清零
	if(dir != GO_STOP)
	{
		navigat->whele_stop = 0;
		//micony201710
		if(sys->sState == SYS_DOCK && sys->motor_power_off)
		{
		    log_printf("MOTOR_POWER_OFF 3\r\n");
			log_printf("found motoroff,power on...\r\n");
			MOTOR_POWER_ON();
			MOTOR_CTRL_SIDE(DOCK_SIDE_PWM_L,DOCK_SIDE_PWM_R);
		}
#if	LOCKED_MOTOR_PARA
		judge_left_right_motor_locked_rotor_process(1);
#endif
	}
	sys->c_right_drop = sys->c_left_drop = 0;
	sys->c_front_hw = 0;
	for(uint8_t i;i<MAX_FRONT_HW;i++)
		sys->front_hw[i] = 1;
	sys->max_front_stop = 15;

	switch(dir)
	{
		case GO_STOP:
		/*
			{
				int i;
				if( *left_pwm > 300 && *left_pwm < 800 && sys->mState != GO_STOP)
				{
					for(i=0;i<10;i++)
					{
						if(*left_pwm < 900)
							*left_pwm +=100;
						if(*right_pwm < 900)
							*right_pwm +=100;
						log_printf("stop:%d,%d\r\n",*left_pwm,*right_pwm);
						if(*left_pwm >=890 ||  *right_pwm >=890)
						{
							 motor_wheel_stop(DOUBLE_WHEEL);
							 break;
						}
						delay(20);
					}
				}

			}
			*/
			 motor_wheel_stop(DOUBLE_WHEEL);
			 sys->c_left=sys->c_right=0;
			 delay(20);
			 break;
		case GO_FORWARD:


			motor_wheel_forward(DOUBLE_WHEEL, pwm);
			break;
		case GO_BACK:


			motor_wheel_backward(DOUBLE_WHEEL, pwm);
			break;
		case GO_RIGTH:			//左边停止，右边转
		/*
			start_pwm = MAX_PWM;
			while(1)
			{
				start_pwm -= 100;
				if((start_pwm ) <= pwm)
					break;

				motor_wheel_backward(RIGHT_WHEEL, start_pwm);
				motor_wheel_forward(LEFT_WHEEL, start_pwm);
				delay_ms(3);
			}
			*/
			motor_wheel_backward(RIGHT_WHEEL, pwm);
			motor_wheel_forward(LEFT_WHEEL, pwm);
			sys->left_dir = GO_FORWARD;
			sys->right_pwm = sys->left_pwm = pwm;
			break;
		case GO_LEFT:
		/*
			start_pwm = MAX_PWM;
			while(1)
			{
				start_pwm -= 50;
				if((start_pwm ) <= pwm)
					break;

				motor_wheel_backward(LEFT_WHEEL, start_pwm);
				motor_wheel_forward(RIGHT_WHEEL, start_pwm);
				delay_ms(2);
			}
			*/
			motor_wheel_backward(LEFT_WHEEL, pwm);
			motor_wheel_forward(RIGHT_WHEEL, pwm);
			sys->left_dir = GO_BACK;
			sys->right_pwm = sys->left_pwm = pwm;
			break;
		case GO_ROUND:		 // 转圈
			r_motor_set_pwm(GO_FORWARD,560);
			l_motor_set_pwm(GO_FORWARD,800);
			//sys->trg_speed=1100;
		//	sys->trg_speed2=1800;		//这个速度要调试过

			break;
		case GO_F_SLOW:		  //低速前进
			r_motor_set_pwm(GO_FORWARD,pwm);
			l_motor_set_pwm(GO_FORWARD,pwm);
			break;
		case GO_SL_ROUND:
			r_motor_set_pwm(GO_FORWARD,420);
			l_motor_set_pwm(GO_FORWARD,550);
			//sys->trg_speed=2200;
			//sys->trg_speed2=1700;		//这个速度要调试过
			//log_printf("set l speed=%d, r_speed=%d\r\n",sys->trg_speed,sys->trg_speed2);
			sys->c_left=sys->c_right=0;
			break;
		case GO_LEFT_RD:
			r_motor_set_pwm(GO_FORWARD,560);
			l_motor_set_pwm(GO_FORWARD,800);
			break;
		case GO_RIGHT_RD:
			log_printf("GO_RIGHT_RD\r\n");
			//l_motor_set_pwm(GO_FORWARD,560);
			//r_motor_set_pwm(GO_FORWARD,800);
			motor_wheel_forward(RIGHT_WHEEL, 800);
			motor_wheel_forward(LEFT_WHEEL, 550);
			break;
		default :
			r_motor_set_pwm(GO_FORWARD,NOMORL_PWM);
			l_motor_set_pwm(GO_FORWARD,NOMORL_PWM);
			break;



	}
	//sys->pwm = pwm;
	//sys->c_hw=cHw;					//需要转的的霍尔数
	sys->mState=dir;				//记录本次的方向
	sys->m_dist = motor.c_left_hw;
//	sys->nextState=next;			//霍尔数完后，的云动方向
	//sys->c_timeout_2=0;
	//sys->m_cHw1=sys->m_cHw2=0;			//两个电机的霍尔数清零

//	sys->chw_next=50;

	sys->m_speed1=0;

	//printf_motor_sta();
 // log_printf("cHW1=%d,cHW2=%d,cHw=%d,,m_state=%d,motor=%s,netx=%s\r\n",sys->m_cHw1,sys->m_cHw2,sys->c_hw,sys->mState,get_motor_sta(sys->mState),get_motor_sta(sys->nextState));
}


/*
直行PID进程，注意，必须在飞阻塞的线程中进行。
*/

char proc_line_pid(float set_angle)
{
//#if GYRO_VERSION
	static uint8_t count=0;		//2018-07-16 jzz

	//www_idleintel_com();
	//这里有个bug，必须set_angle == navigat->out  没设置角度，则取当前角度
	count ++;
	if( count>=2)//2018-07-16 jzz
	{
		count = 0;
//#endif
		if(set_angle == NO_ANGLE_V)
		{
			set_angle = navigat->out = sys->angle;
			log_printf("set = %3.1f\r\n",sys->angle);
		}
		set_angle = format_agle(set_angle, ANGLE_360);
		/*
		if(sys->angle > 330)
			sys->angle -= 360.0f;
		if(set_angle > 330)
			set_angle -=360;
		*/
		p_gyro_calce(set_angle,sys->angle);

		L_FORWRK_PWM =  sys->lpwm;
		R_FORWRK_PWM =  sys->rpwm;

	//	log_printf("%d,%d\r\n",L_FORWRK_PWM,R_FORWRK_PWM);
//#if GYRO_VERSION
	}
//#endif
	return 1;
}
char proc_back_pid(float set_angle)
{


	//www_idleintel_com();
	set_angle = format_agle(set_angle, ANGLE_360);
	/*
	if(sys->angle > 330)
		sys->angle -= 360.0f;
	if(set_angle > 330)
		set_angle -=360;
	*/
	p_gyro_calce(set_angle,sys->angle);

	R_FORWRK_PWM =  sys->lpwm;
	L_FORWRK_PWM =  sys->rpwm;

	return 1;
}

char * get_motor_sta(uint8_t indx)
{
	return (&motorStr[indx][0]);
}
void printf_motor_sta(void)
{
//#if 1//MOTOR_DEBUG
	log_printf("H1=%d,H2=%d,S1=%d,S2=%d,w=%d,%s\r\n",sys->m_cHw1,sys->m_cHw2,sys->m_speed1,sys->m_speed2,sys->c_hw,&motorStr[sys->mState][0]);
//#endif
}
/*
#define FW_SLOW_PWM			780//sys->pwm//620
#define BACK_OFF_PWM		780//650//(sys->pwm+80)//cfg->go_back_pwm
#define GO_FORWARD_PWM		780//sys->pwm//cfg->go_forward_pwm
#define GO_ROUTINT_PWM		780//sys->pwm//cfg->go_route_pwm
#define TURN_DEG_PWM		720//sys->pwm
#define GO_NEAR_PWM			700
#define GO_NEAR_PWM_FAST	630




uint16_t get_pwm(uint8_t type)
{
	log_printf("t=%d,v=%d\r\n",type,sys->volt);
	if(type == T_GO_FW_PWM )
	{
	//	if(sys->voice < 1400)
		//	return 680;
		if(sys->volt < 1460)
			return 700;
		return 780;

	}
	if (type == T_GO_NEAR_PWM)
	{
		if(sys->volt < 1460)
			return 640;
		return 700;

	}
	if (type == T_GO_TURN_DEG)
	{
		if(sys->volt < 1460)
			return 660;
		return 720;

	}

}
*/
#if LOCKED_MOTOR_PARA
#define CYC_MO_LO_TIME 	5000		//5S检测一次是否堵转
	//#define CYC_MO_INIT_TIME	20		//20ms检测一次是否初始化堵转
#define MO_LO_MIN_HW	10			//判断电机锁死的最小霍尔数
void judge_left_right_motor_locked_rotor_process(uint8_t type)
{
	static uint16_t motor_lock_times_count_s = 0;
	static int init_lock_left_hw = 0;
	static int init_lock_right_hw = 0;
	int lock_left_hw;
	int lock_right_hw;

	if(type == 1)
	{
		motor_lock_times_count_s = 0;
		init_lock_left_hw = motor.c_left_hw;
		init_lock_right_hw = motor.c_right_hw;
		return;
	}


	//20ms检测电机堵转参数是否需要初始
	/*
	if(sys->motor_lock_times_count_ms++ > CYC_MO_INIT_TIME)
	{
		sys->motor_lock_times_count_ms = 0;
		if(sys->mState == GO_STOP )
		{
			init_left_right_motor_locked_parameter();	////初始化初始HW值
		}
	}
	*/
	//在电机运行状态下,没有判断出是锁定才进行是否锁住电机
	if(sys->sState != SYS_CHARGE)
	{
		if(sys->mState != GO_STOP && sys->motor_locked_flag == 0)	//
		{
			if( motor_lock_times_count_s++ > CYC_MO_LO_TIME )
			{
				motor_lock_times_count_s = 0;
				lock_left_hw = motor.c_left_hw;
				lock_right_hw = motor.c_right_hw;

				if((disXY(init_lock_left_hw,lock_left_hw) <= MO_LO_MIN_HW) || (disXY(init_lock_right_hw,lock_right_hw) <= MO_LO_MIN_HW))
				{
					log_printf("==motor_locked:L[%d,%d]R[%d,%d]\r\n",lock_left_hw,init_lock_left_hw,lock_right_hw,init_lock_right_hw);
					sys->motor_locked_flag = 1;
				}
				else		//电机没有堵转
				{
					init_lock_left_hw = motor.c_left_hw;
					init_lock_right_hw = motor.c_right_hw;
				}
			}
		}
		else
			motor_lock_times_count_s = 0;
	}
}
#endif
static uint8_t s_sta = 0;
static uint8_t s_work_mode = 0;
static uint8_t s_dust_sensor = 0;
static FlagStatus s_blankets_ground = RESET;
//输入: 系统状态,工作模式,灰尘检测,地毯检测
//输出: 吸尘电机速度 滚刷速度 行走速度
//sta:系统状态 空闲 导航 dock change
//f_work_mode:6个工作模式:静音,正常,强力,full2go,手动,集中清扫
/* if(sys->f_work_mode==MWO_TURBO||sys->f_work_mode==MWO_SPOT)
	{
      log_printf("MWO_TURBO = %d\r\n",AGITATOR_PWM);
		AGITATOR_PWM = 999;
        return ;
	}*/
void set_mode_and_power_pro(uint8_t sta,uint8_t f_work_mode,uint8_t dust_sensor,FlagStatus Blankets_ground)
{
	if(sys->sState == SYS_DOCK || sys->sState == SYS_DOCK_NEAR)
		return ;

	if(s_sta != sta || s_work_mode != f_work_mode || s_dust_sensor != dust_sensor || s_blankets_ground != Blankets_ground )
	{
		log_printf1("[set_mode_and_power_pro]sta:(%d)%d,work_mode:(%d)%d\r\n",s_sta,sta,s_work_mode,f_work_mode);
		log_printf1("[set_mode_and_power_pro]dust_sensor(%d)%d,Blankets_ground(%d)%d\r\n",s_dust_sensor,dust_sensor,s_blankets_ground,Blankets_ground);
		s_sta = sta;
		s_work_mode = f_work_mode;
		s_dust_sensor = dust_sensor;
		s_blankets_ground = Blankets_ground;
		switch(f_work_mode)
		{
			case MWO_SILENT:
				log_printf1("MWO_SILENT\r\n");
				sys->dust_pwm_lev = DUST_MODE_SILENT;
				sys->agi_speed = AGI_L_SPEED;			//
				//SET_MID_MOTER(cfg->l_agi_speed);
				//log_printf1("SET_MID_MOTER=%d\r\n",cfg->l_agi_speed);
				sys->run_speed = LOW_SPEED;				//
			break;
			case MWO_NORMAL:
			case MWO_FULL2GO:
				log_printf1("MWO_NORMAL\r\n");
				if(dust_sensor == DUST_LOW )
				{
					log_printf1("dust_sensor1=%d\r\n",DUST_LOW);

					sys->dust_pwm_lev = DUST_MODE_NORMAL;
					sys->agi_speed = AGI_ML_SPEED;		//
					if(Blankets_ground)
						sys->run_speed = MIDLE_M_SPEED;			//
					else
						sys->run_speed = MIDLE_H_SPEED;			//
				}
				else if(dust_sensor == DUST_MEDIUM)
				{
				   log_printf1("dust_sensor2=%d\r\n",DUST_MEDIUM);
					sys->dust_pwm_lev = DUST_MODE_NORMAL;
					if(Blankets_ground)
					{
						sys->agi_speed = AGI_M_SPEED;		//
						sys->run_speed = MIDLE_L_SPEED;			//
					}
					else
					{
						sys->agi_speed = AGI_ML_SPEED;		//
						sys->run_speed = MIDLE_SPEED; 		//
					}
				}
				else
				{
			        log_printf1("dust_sensor3=%d\r\n",DUST_MEDIUM);
					sys->dust_pwm_lev = DUST_MODE_TURBO;
					if(Blankets_ground)
						sys->agi_speed = AGI_MH_SPEED;		//
					else
						sys->agi_speed = AGI_M_SPEED;		//
					sys->run_speed = LOW_SPEED;			//
				}
			break;
			case MWO_TURBO:
			case MWO_MANUAL:
				log_printf1("MWO_TURBO\r\n");
				sys->dust_pwm_lev = DUST_MODE_TURBO;
				//sys->agi_speed = AGI_H_SPEED;		//
				sys->run_speed = HIGHT_SPEED; 		//
			break;
			case MWO_SPOT:
				log_printf1("MWO_TURBO\r\n");
				sys->dust_pwm_lev = DUST_MODE_TURBO;
				sys->agi_speed = AGI_H_SPEED;		//  在上面设置
				//sys->run_speed = HIGHT_SPEED;			//不设置速度
			break;
			default:

			break;
		}


		if((sta != SYS_IDLE && sta != SYS_CHARGE) || f_work_mode == MWO_MANUAL)
		{
			log_printf1("dust_motor_rank change right now,lev=%d\r\n",sys->dust_pwm_lev );
			if(sys->dust_pwm_lev == DUST_MODE_SILENT)
			{
				DUST_MOTOR_RANK1();
				AGITATOR_PWM = 100;
				log_printf1("MWO_TURBO1=%d\r\n",AGITATOR_PWM);
			}
			else if(sys->dust_pwm_lev == DUST_MODE_NORMAL)
			{
				DUST_MOTOR_RANK2();
				AGITATOR_PWM = 450;
				log_printf1("MWO_TURBO2=%d\r\n",AGITATOR_PWM);
			}
			else
			{
				DUST_MOTOR_RANK3();
				AGITATOR_PWM = 850;
				log_printf1("MWO_TURBO3=%d\r\n",AGITATOR_PWM);
			}
		}
		else
		{
			log_printf("dust_motor_rank change idle,mode=%d\r\n",f_work_mode);
			if(f_work_mode == MWO_MANUAL)
			{
				sys->dust_pwm_lev = DUST_MODE_TURBO;
				// sys->dust_pwm_lev =  DUST_MODE_NORMAL;
				sys->agi_speed = AGI_H_SPEED;		//
				sys->run_speed = HIGHT_SPEED;		//
				log_printf1("MWO_TURBO4\r\n");
			}
			else
			{
				//DUST_MOTOR_OFF();
				//sys->agi_speed = AGI_H_SPEED;		//这个为什么设置为高速忘记了,先屏蔽掉
			}
		}
		/*
			if(sta == SYS_NAVIGAT&&f_work_mode == MWO_MANUAL)   //因2a机器无法改变风机等级强行设置在导航状态下风机等级调到中档
		{
		        sys->dust_pwm_lev = DUST_MODE_NORMAL;
				log_printf1("MWO_TURBO5\r\n");
		}
		*/
		log_printf("[set_mode_and_power_pro]dust_pwm_lev:%d,agi_speed:%d,run_speed:%d\r\n",sys->dust_pwm_lev,sys->agi_speed,sys->run_speed);
#if 1	//2a		//测试结束可以拿掉
		if(sys->dust_pwm_lev == DUST_MODE_SILENT)
			log_printf1("Vacuum motor power:L_15\r\n");
		else if(sys->dust_pwm_lev == DUST_MODE_NORMAL)
			log_printf1("Vacuum motor power:M_42.5\r\n");
		else if(sys->dust_pwm_lev == DUST_MODE_TURBO)
			log_printf1("Vacuum motor power:H_70\r\n");
		if(sys->agi_speed == AGI_H_SPEED)
			log_printf("Agitator motor power:H_20\r\n");
		else if(sys->agi_speed == AGI_MH_SPEED)
			log_printf("Agitator motor power:M_10\r\n");
		else if(sys->agi_speed == AGI_M_SPEED)
			log_printf("Agitator motor power:M_10\r\n");
		else if(sys->agi_speed == AGI_ML_SPEED)
			log_printf("Agitator motor power:M_10\r\n");
		else if(sys->agi_speed == AGI_L_SPEED)
			log_printf("Agitator motor power:L_5\r\n");
		if(f_work_mode==MWO_SPOT)
			return;
		if(sys->run_speed == HIGHT_SPEED)
			log_printf("Speed:H_0.35\r\n");
		else if(sys->run_speed == MIDLE_H_SPEED)
			log_printf("Speed:M_0.27\r\n");
		else if(sys->run_speed == MIDLE_SPEED)
			log_printf("Speed:M_0.27\r\n");
		else if(sys->run_speed == MIDLE_M_SPEED)
			log_printf("Speed:M_0.27\r\n");
		else if(sys->run_speed == MIDLE_L_SPEED)
			log_printf("Speed:M_0.27\r\n");
		else if(sys->run_speed == LOW_SPEED)
			log_printf("Speed:L_0.2\r\n");
#else	//2b
		if(sys->dust_pwm_lev == DUST_MODE_SILENT)
			log_printf("Vacuum motor power:L_20\r\n");
		else if(sys->dust_pwm_lev == DUST_MODE_NORMAL)
			log_printf("Vacuum motor power:M_40\r\n");
		else if(sys->dust_pwm_lev == DUST_MODE_TURBO)
			log_printf("Vacuum motor power:H_120\r\n");
		if(sys->agi_speed == AGI_H_SPEED)
			log_printf("Agitator motor power:H_40\r\n");
		else if(sys->agi_speed == AGI_MH_SPEED)
			log_printf("Agitator motor power:MH_30\r\n");
		else if(sys->agi_speed == AGI_M_SPEED)
			log_printf("Agitator motor power:M_20\r\n");
		else if(sys->agi_speed == AGI_ML_SPEED)
			log_printf("Agitator motor power:ML_15\r\n");
		else if(sys->agi_speed == AGI_L_SPEED)
			log_printf("Agitator motor power:L_5\r\n");
		if(f_work_mode==MWO_SPOT)
			return;
		if(sys->run_speed == HIGHT_SPEED)
			log_printf("Speed:H_0.35\r\n");
		else if(sys->run_speed == MIDLE_H_SPEED)
			log_printf("Speed:MH_0.30\r\n");
		else if(sys->run_speed == MIDLE_SPEED)
			log_printf("Speed:M_0.27\r\n");
		else if(sys->run_speed == MIDLE_M_SPEED)
			log_printf("Speed:M_0.25\r\n");
		else if(sys->run_speed == MIDLE_L_SPEED)
			log_printf("Speed:M_0.22\r\n");
		else if(sys->run_speed == LOW_SPEED)
			log_printf("Speed:M_0.2\r\n");

#endif
	}
}
void dis_mode_pro(void)
{
	uint8_t up_work_mod;
	///1根据系统状态,工作模式,尘检,地毯信息,得到系统状态及各电机运转参数
	if(sys->sState == SYS_TEST_BD)
		return;
	set_mode_and_power_pro(sys->sState,sys->f_work_mode,sys->dust_sensor,sys->Blankets_ground);
	if(sys->gey_key_clear_err == 1)/// E2一直显示，有按键操作后清除
	{
		log_printf("gey_key_clear_err CLEAR\r\n");
		sys->gey_key_clear_err = 0;
		sys->work_errcode = MWERR_NONE;
		//dp_led_err_contrl(0,sys->sState,sys->work_errcode);
	}
	///2上报模式 指示灯
	if(sys->f_work_mode == MWO_SPOT)
		 up_work_mod = MWO_TURBO;
	else
		up_work_mod = sys->f_work_mode;
	dp_led_light_contrl(1,sys->sState,up_work_mod,sys->charge_sta,sys->prs,sys->work_errcode,sys->wifi_state);
	//if(sys->power_on)
	dp_led_err_contrl(1,sys->sState,sys->work_errcode);
}
