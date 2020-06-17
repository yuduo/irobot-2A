//#include "includes.h"
#include "stdio.h"
#include "string.h"
#include "stm32f10x.h"
#include "stm32f10x_conf.h"
#include "stm32f10x_it.h"
#include "bsp.h"
#include "BLDC.h"
#include "usart.h"
/**********常量定义**********/


int	bldc_dir;
uint16_t 	bldcPWM;

#define IDLE    0
#define	START	1
#define	RUN	    2
#define	STOP    3
#define FAULT   4
#define HIGH	1480
#define LOW     3
/*********全局变量***********/
u8 state;				   //主状态
FlagStatus Direction = SET;//初始化后为正转
uint8_t stalling_count = 0;		   //堵转计数器
FlagStatus zheng_fan = RESET;//初始化后为正转


/**********************************************************************
* Description    : 对定时器1和定时器3的GPIO定义
* Input          : None
* Output         : None
* Return         : None
* Attention      : None
**********************************************************************/
void BLDC_GPIOConfig(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
 	 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9|GPIO_Pin_10;  //TIM1输出
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;						   
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
			 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14|GPIO_Pin_15;//TIM1输出
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;						   
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;              //TIM3的霍尔输入
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;                         //TIM3的霍尔输入
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;                         
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_SetBits(GPIOB,GPIO_Pin_3) ;
	
//    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;                         //TIM1_BKIN
//    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
//    GPIO_Init(GPIOB, &GPIO_InitStructure);
	   
}
/**********************************************************************
* Description    : None
* Input          : None
* Output         : None
* Return         : None
* Attention      : None
**********************************************************************/
void BLDC_TIM1Config(void)
{
   TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;			   //基本结构体变量定义
   TIM_OCInitTypeDef  		TIM_OCInitStructure;               //输出结构体变量定义
   TIM_BDTRInitTypeDef  	TIM_BDTRInitStructure;			   //死区刹车结构体变量定义

   TIM_DeInit(TIM1);

/*
  PWM T=1/(72M/3) *1500
  PWM =	72M/3/1500 =72M/(3*1500)=16K
*/
   TIM_TimeBaseStructure.TIM_Prescaler = 2;					   //预分频值
   TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_CenterAligned2;//中央对齐计数模式,输出比较标志位只有在比较器向上计算被设置
   TIM_TimeBaseStructure.TIM_Period = 1500 - 1;					   //PWM 16K   设置下一个更新时间装入活动的自动重装载寄存器周期的值
   TIM_TimeBaseStructure.TIM_ClockDivision = 0;						//设置时钟分割
   TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;				   

   TIM_TimeBaseInit(TIM1,&TIM_TimeBaseStructure);
 /*
   	PWM模式2，TIM1_CNT > TIM1_CCR1时为有效点评
 */
   TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1; 		   //TIM输出通道初始化
   TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Disable; 
   TIM_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Disable;                  
   TIM_OCInitStructure.TIM_Pulse =200; 
   TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; 
   TIM_OCInitStructure.TIM_OCNPolarity = TIM_OCNPolarity_High;         
   TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Reset;//死区后，输出低电平，TIM_OCIdleState_Set;
   TIM_OCInitStructure.TIM_OCNIdleState = TIM_OCIdleState_Reset;          
  
   /*
   TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1; 		   //TIM输出通道初始化
   TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Disable; 
   TIM_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Disable;                  
   TIM_OCInitStructure.TIM_Pulse =300; 
   TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; 
   TIM_OCInitStructure.TIM_OCNPolarity = TIM_OCNPolarity_High;         
   TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Reset;//TIM_OCIdleState_Set;
   TIM_OCInitStructure.TIM_OCNIdleState = TIM_OCIdleState_Reset;          
  
  TIM_OCInitStructure.TIM_OCMode=TIM_OCMode_PWM1;                    //设置为pwm1输出模式
  TIM_OCInitStructure.TIM_Pulse=300;                   //设置占空比时间
  TIM_OCInitStructure.TIM_OCPolarity=TIM_OCPolarity_Low;                 //设置输出极性
  TIM_OCInitStructure.TIM_OutputState=TIM_OutputState_Disable;        //使能该通道输出
  TIM_OCInitStructure.TIM_OCNPolarity=TIM_OCNPolarity_Low;        //设置互补端输出极性
  TIM_OCInitStructure.TIM_OutputNState=TIM_OutputNState_Disable;//使能互补端输出
  TIM_OCInitStructure.TIM_OCIdleState=TIM_OCIdleState_Reset;        //死区后输出状态
  TIM_OCInitStructure.TIM_OCNIdleState=TIM_OCNIdleState_Reset;//死区后互补端输出状态
   TIM_OC1Init(TIM1,&TIM_OCInitStructure); 
  */ 
  // TIM_OCInitStructure.TIM_Pulse =300;
   TIM_OC1Init(TIM1,&TIM_OCInitStructure);
   TIM_OC2Init(TIM1,&TIM_OCInitStructure);
   TIM_OC3Init(TIM1,&TIM_OCInitStructure);

/*
   TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2; 		   //TIM输出通道4初始化，用来触发AD注入采样
   TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Disable;                   
   TIM_OCInitStructure.TIM_Pulse =1495; 
   TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;           
  
   TIM_OC4Init(TIM1,&TIM_OCInitStructure); 

   */
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1; 
	 TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;//TIM_OCMode_PWM1;////CCR4这个输出使能后才能用来触发ADC1的注入通道采样
	TIM_OCInitStructure.TIM_Pulse = 200;    
//	TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Set;
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low;//TIM_OCPolarity_High; 	//输出比较极性高。
	TIM_OC4Init(TIM1, &TIM_OCInitStructure);

 
   TIM_BDTRInitStructure.TIM_OSSRState = TIM_OSSRState_Enable;	//死区刹车初始化
   TIM_BDTRInitStructure.TIM_OSSIState = TIM_OSSIState_Enable;
   TIM_BDTRInitStructure.TIM_LOCKLevel = TIM_LOCKLevel_1; 
   TIM_BDTRInitStructure.TIM_DeadTime = 100;
   TIM_BDTRInitStructure.TIM_Break = TIM_Break_Disable;	 //如打开，开机无输出且状态紊乱？？？？
   TIM_BDTRInitStructure.TIM_BreakPolarity =  TIM_BreakPolarity_Low ;
   TIM_BDTRInitStructure.TIM_AutomaticOutput = TIM_AutomaticOutput_Disable;

   TIM_BDTRConfig(TIM1,&TIM_BDTRInitStructure);


   TIM_OC1PreloadConfig(TIM1,TIM_OCPreload_Enable);  //使能捕获比较寄存器预装载（通道1）

   TIM_OC2PreloadConfig(TIM1,TIM_OCPreload_Enable);	 //使能捕获比较寄存器预装载（通道2）

   TIM_OC3PreloadConfig(TIM1,TIM_OCPreload_Enable);	 //使能捕获比较寄存器预装载（通道3）

 //  TIM_OC4PreloadConfig(TIM1,TIM_OCPreload_Enable);	 //使能捕获比较寄存器预装载（通道3）
   
   TIM_SelectInputTrigger(TIM1, TIM_TS_ITR2);        //输入触发源选择TIM3   
  
   //TIM_SelectSlaveMode(TIM1, TIM_SlaveMode_Trigger);//从模式选择 触发	  

   TIM_CtrlPWMOutputs(TIM1,ENABLE);		//开启什么主模式，貌似PWM配对的事情。

//   TIM_ClearITPendingBit(TIM1, TIM_IT_Break|TIM_IT_COM);
   TIM_ITConfig(TIM1, TIM_IT_Break | TIM_IT_COM ,ENABLE);
  //  TIM_ITConfig(TIM1,TIM_IT_CC1|TIM_IT_CC2|TIM_IT_CC3, ENABLE);
 //  TIM_ClearITPendingBit( TIM1, TIM_IT_COM);
 //  TIM_ITConfig(TIM1, TIM_IT_COM ,ENABLE);
  // TIM_ITConfig(TIM1, TIM_IT_CC4, ENABLE);	 //CCR4的中断，这个通过设置CCR4的pulse来控制产生中断相当于PWM-ON的位置


   TIM_Cmd(TIM1,ENABLE);


/*
使用外部时钟的例子
TIM_ITRxExternalClockConfig(TIM3, TIM_TS_ITR1);//外部时钟模式1，触发源为TIMER2
TIMx->SMCR=TIM_TS_ITR1
TIMx->SMCR=	！=TIM_SlaveMode_External1；

*/
   
}
/**********************************************************************
* Description    : None
* Input          : None
* Output         : None
* Return         : None
* Attention      : None
**********************************************************************/
void BLDC_TIM3Config(void)
{
   TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;			   //基本结构体变量定义
   TIM_ICInitTypeDef  TIM_ICInitStructure;                      //定义结构体变量
   TIM_OCInitTypeDef  TIM_OCInitStructure;                     //输出结构体变量定义
      
   TIM_DeInit(TIM3);
  /*
	一圈最大时间

	1/(72M/72)*65535=64ms
	是不是意味着
   一对极，最大为64*3=200ms,两对基一圈400ms	则一分钟 60000/400=	 150圈，最低速度！就是一秒两圈，最慢速度。
  */
   TIM_TimeBaseStructure.TIM_Prescaler = 71;				   //预分频
   TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
   TIM_TimeBaseStructure.TIM_Period =65535;	   			//
   TIM_TimeBaseStructure.TIM_ClockDivision = 0;
   TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;

   TIM_TimeBaseInit(TIM3,&TIM_TimeBaseStructure); 
     
    //霍尔捕获。 
   TIM_ICInitStructure.TIM_Channel = TIM_Channel_1;            //选择通道1
   TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising; //输入上升沿捕获  
   TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_TRC;  //配置通道为输入，并映射到哪里
   TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;       //输入捕获预分频值
   TIM_ICInitStructure.TIM_ICFilter = 10;                      //输入滤波器带宽设置

   TIM_ICInit(TIM3, &TIM_ICInitStructure);                     //输入通道配置

   //通道2输出，引发TIM1的COM
   TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2; 		    //TIM输出通道初始化
   TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;             
   TIM_OCInitStructure.TIM_Pulse =1023;   		//TIMx->CCR2
   TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;      
  
   TIM_OC2Init(TIM3,&TIM_OCInitStructure);
   
   //应该是 产生产生比较/捕获事件４
   //TIMx->CCMR2   =  TIM_OCMode_Timing << 8
   TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_Timing; 		    //TIM输出通道初始化
   TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Disable;             
   TIM_OCInitStructure.TIM_Pulse =65535; 
   TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;      
  
   TIM_OC4Init(TIM3,&TIM_OCInitStructure);
  /*
 设置 >CR2 的 TIM_CR2_TI1S，
 CH1，CH2，CH3经异或后，连接到II1脚输入
  */
   TIM_SelectHallSensor(TIM3,ENABLE);                          //使能TIMx的霍尔传感器接口
 
 /*
    SMCR = 	 TIM_TS_TI1F_ED; TS设置成TIFI_ED
 */  
   TIM_SelectInputTrigger(TIM3, TIM_TS_TI1F_ED);               //输入触发源选择   
 
 /*
 SMCR |= TIM_SlaveMode_Reset;
 SMCR的SMS，设置为100.复位模式。选中的出发输入(TRGI）的上升沿重新初始化计数器。并产生一个更新寄存器的信号
 */ 
   TIM_SelectSlaveMode(TIM3, TIM_SlaveMode_Reset);             //复位模式，一个霍尔传感器信号来的时候，复位。

  /*
 SMCR BIT7 MSM 设置为1
触发输入（TRGI)上的事件被延迟了，以允许在当前定时器（通过TRGO)与它的从定时器建的完美同步
这对要求把几个定时器同步到一个单一的外部事件时是非常有用的。
 */ 
   TIM_SelectMasterSlaveMode(TIM3, TIM_MasterSlaveMode_Enable);//主从模式选择        
 
 /*
 CR2= 0x50 MMS=101 OC2REF信号被作为触发输出
 */  
   TIM_SelectOutputTrigger(TIM3, TIM_TRGOSource_OC2Ref);      //选择输出触发模式(TRGO端)
// TIMx->DIER |= TIM_IT;
   TIM_ITConfig(TIM3, TIM_IT_Trigger|TIM_IT_CC4, ENABLE);      //开定时器中断 
   //TIM_Cmd(TIM3,ENABLE);
}
/*
void timer4_cfg(void)
{
 TIM_TimeBaseInitType ;

 TIM_DeInit(TIM4);

 TIM_TimeBaseStructure.TIM_Period=2000;		 //ARR的值
 TIM_TimeBaseStructure.TIM_Prescaler=71;
 TIM_TimeBaseStructure.TIM_ClockDivision=TIM_CKD_DIV1; //采样分频
 TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up; //向上计数模式
 TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
// TIM_PrescalerConfig(TIM2,0x8C9F,TIM_PSCReloadMode_Immediate);//时钟分频系数36000，所以定时器时钟为2K
 TIM_ARRPreloadConfig(TIM4, DISABLE);//禁止ARR预装载缓冲器
 TIM_ITConfig(TIM4,TIM_IT_Update,ENABLE);

// TIM_Cmd(TIM2, ENABLE);	//开启时钟

}
*/

void Flaghuanxiang(void)
{
	zheng_fan=~zheng_fan;

}
void set_direction(FlagStatus dir)
{
	zheng_fan= dir;
}
void bdlc_set_pwm(uint16_t pwm)
{
	   TIM1->CCR1=pwm;	   //输出到TIM1的PWM寄存器
  	   TIM1->CCR2=pwm;
  	   TIM1->CCR3=pwm; 
	   TIM1->CCR4=pwm;
	   bldcPWM=pwm;
}
/**********************************************************************
* Description    : None
* Input          : None
* Output         : None
* Return         : None
* Attention      : None
**********************************************************************/
/*
序列 角度  A	B	C |	 A	B	C  		
1 	   0   0    0   1 |  +   -	 		ab
2	   60  0	1	1 |  +   	-	    ac
3	   120 0	1	0 |	  	+	-	    bc
4	   180 1	1	0 |  -	+	 	    ba
5	   240 1	0	0 |	 -	 	+	  	ca
6	   300 1	0	1 |	  	-	+	    cb

A - PA6
B - PA7
C - PB0
电机		板子		IO
A黄色		C			PB0
B白色		B			PA7
C蓝色		A			PA6

CBA

方向1
正传:264513
462315

154

1 	   0   		0   0   1 |   ab	4	0x3180
2	   60  		0	1	1 |   ac	6	0x3108
3	   120 		0	1	0 |   bc	2	0x3018
4	   180 		1	1	0 |   ba	3	0x3810
5	   240 		1	0	0 |   ca	1	0x3801
6	   300 		1	0	1 |   cb	5	0x3081
反转
1 	   0   		0   0   1 |   ba	4	0x3810
2	   60  		0	1	1 |   ca	6	0x3801
3	   120 		0	1	0 |   cb	2	0x3081
4	   180 		1	1	0 |   ab	3	0x3180
5	   240 		1	0	0 |   ac	1	0x3108
6	   300 		1	0	1 |   bc	5	0x3018
*/


			//			 		1     2      3       4     5      6
//uint16_t	zarycced[]={0x00,0x3018,0x3180,0x3108,0x3801,0x3810,0x3081,0x00};
uint16_t	zarycced[]={0x00,0x3801,0x3018,0x3810,0x3180,0x3081,0x3108,0x00};
uint16_t	fzarycced[]={0x00,0x3108,0x3081,0x3180,0x3810,0x3018,0x3801,0x00};
//u8			mBlPwmAry[]={0,4,2,2,1,4,1,0};

uint8_t	hwDIRary[]={4,6,2,3,1,5};

uint8_t	l_hw[3];

void dirCala(uint8_t hw)
{
	u8 i,k,j;
   	 //计算方向
	  for(i=0;i<6;i++)
	  {
		   if(hwDIRary[i]==hw)
		   		break;
	  }
	  if(i<6)
	  {
	  	  k=i;
	  	  for(j=0;j<2;j++)
		  {
			  k++;
			  k=k>=6?0:k;
			  if(hwDIRary[k]!=l_hw[j])
			  	break;
		  }
		  if(j==2)	 //正向
		  {
			 // t_printf("B");
			 bldc_dir=1;
		  }else
		  {
		  	  k=i;
		  	  for(j=0;j<2;j++)
			  {
			  	  if(k==0)
				  	k=5;
				   else
				  	k--;
				  
				  if(hwDIRary[k]!=l_hw[j])
				  	break;
			  }
			  if(j==2)	 //以此反，角度为负时，取此向。
			  {
			  	bldc_dir=-1;
			  	/*
				  t_printf("T");
				  set_direction(RESET); //正传
				  */
			  }

		  }
		  
	  }


		
		l_hw[2]=l_hw[1];
		l_hw[1]=l_hw[0];
		l_hw[0]=hw;
		
}

void huanxiang(void)
{
  u8 hwValue; 
#if HW_PRINTF
  static u8 lst=0;
#endif
  //hwValue = CBA
  hwValue=(u8)((GPIOA->IDR&0x000000c0)>>6);	//读转子位置
  hwValue|=(u8)((GPIOB->IDR&0x00000001)<<2);
 //  log_printf("%d",hwValue);
#if HW_PRINTF
 	if(lst!=hwValue)
	   write_hw(hwValue);
	lst = hwValue;
#endif
#if HW_DEBUG
//		recod_bldc(0,hwValue,0,0);
#endif
	  dirCala(hwValue);
	  if(zheng_fan==RESET)
	  	TIM1->CCER=zarycced[hwValue];
	  else								 //反转
		TIM1->CCER=fzarycced[hwValue]; //根据转子位置，决定CCER输出相位和转子字偏移量


}
/*
void check_run_ok(void)
{
  u8 hwValue,last,index=0; 
  int i;
  last=(u8)((GPIOA->IDR&0x000000c0)>>6);	//读转子位置
  last|=(u8)((GPIOB->IDR&0x00000001)<<2);
  index=last;
  for(i=0;i<80;i++)
  {
  	   Delayus(1000);
	  hwValue=(u8)((GPIOA->IDR&0x000000c0)>>6);	//读转子位置
	  hwValue|=(u8)((GPIOB->IDR&0x00000001)<<2);
	  log_printf("%d",hwValue);
	  if(last!=hwValue)
	  	return ;
	  last=hwValue;	 
   }
   log_printf("run error,%d\r\n",hwValue);
   index= hwValue+2;
   if(index==7)
   	index=1;
   TIM1->CCER=zarycced[index];
   log_printf("set to %d\r\n",index);

}
*/
/**************启动******************/
void BLDC_Start(void)
{
   TIM1->SMCR|=0x0006;        //开TIM1的输入触发	
   TIM1->DIER=0x0040;         //开TIM1的触发中断
   huanxiang();			      //调用换向函数，启动			  					 
   TIM3->CR1|=0x0001;		  //开TIM3
   TIM3->DIER|=0x0050;		  //开TIM3中断  
 //  Delayus(40);
//	check_run_ok();
}
void BLDC_Stop(void)
{
   TIM1->SMCR&=0xfff8;		  //关闭TIM1的输入触发
   TIM1->CCER=0;              //关闭TIM1的六路输出
   Delayus(40);			      //延时（加死区）
   TIM1->CCER=0x0ccc;         //打开三路下管，进行能耗制动
   while(stalling_count<9);    //等待电机停止（TIM3连续溢出10次，即认为电机已停转）   
   TIM1->CCER=0;              //关闭TIM1的六路输出，关刹车		  
   TIM3->CR1&=0xfffe;         //关闭TIM3						  
   TIM3->CNT=0;               //清TIM3的计数器				   
   TIM3->DIER&=0xffaf;		  //关TIM3中断
}

/**************停止******************/
/*void stop(void)
{
   TIM1->SMCR&=0xfff8;		  //关闭TIM1的输入触发
   TIM1->CCER=0;              //关闭TIM1的六路输出
   Delay(20);			      //延时（加死区）
   TIM1->CCER=0x0ccc;         //打开三路下管，进行能耗制动
   while(duzhuan_value<1);    //等待电机停止（TIM3连续溢出10次，即认为电机已停转）   
   TIM1->CCER=0;              //关闭TIM1的六路输出，关刹车		  
   TIM3->CR1&=0xfffe;         //关闭TIM3						  
   TIM3->CNT=0;               //清TIM3的计数器				   
   TIM3->DIER&=0xffaf;		  //关TIM3中断
   data_reset();     	      //复位运行参数
}*/
