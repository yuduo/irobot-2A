/******************** (C) COPYRIGHT 2012 WildFire Team ***************************
 * 文件名  ：SysTick.c
 * 描述    ：SysTick 系统滴答时钟10us中断函数库,中断时间可自由配置，
 *           常用的有 1us 10us 1ms 中断。         

 * 库版本  ：ST3.5.0
*/
#include "sys.h"
//#include "SysTick.h"
#include "adc.h"
#include "usart.h"
#include "bsp.h"
#include "motor.h"
//#include "iclean.h"


#if ROOMA_GYROONLY
volatile uint32_t	msTmr=0,msledTmr=0,msWifiTmr=0,msChargeTmr = 0,msledwifitmr = 0; 
#else
volatile uint32_t	msTmr=0,msledTmr=0,msWifiTmr=0,msChargeTmr = 0,msledwifitmr = 0;
#endif

//volatile  uint8_t sysIRQ=0;
/*启动系统滴答定时器 SysTick */
void SysTick_Init(void)
{
	/* SystemFrequency / 1000    1ms中断一次
	 * SystemFrequency / 100000	 10us中断一次
	 * SystemFrequency / 1000000 1us中断一次
	 
	 SysTick_Config(SystemCoreClock /200);//200hz系统频率
//	SysTick_Config(SystemCoreClock /10);//200hz系统频率
*/
//	while(SysTick_Config(SystemCoreClock / 100));  //10ms	
   	while(SysTick_Config(SystemCoreClock / 1000));  //100ms	
//	  if (SysTick_Config(SystemCoreClock / OS_TICKS_PER_SEC))
		   
}

/*定时器中断服务函数，构建系统运行周期*/


static uint8_t c = 0;
#if ROOMA_GYROONLY
uint8_t n_10ms_tick = 0;
#endif
void SysTick_Handler()
{	
	msTmr++;
	msledTmr ++;			//led使用
	msledwifitmr ++;
	msWifiTmr++;
	if(sys->sState == SYS_CHARGE)
		msChargeTmr++;
	else
		msChargeTmr = 0;
#if ROOMA_GYROONLY
	//msledTmr ++;			//led使用
	//msWifiTmr++;
	if(sWorkTimeMinCt++ > 60000)
	{
		//log_printf("%d,%d\r\n",sWorkTimeMinCt,sWorkTimeMin);
		sWorkTimeMinCt = 0;
		sWorkTimeMin ++;
		sSleepTimeMin++;
	}

	#if DP_PANEL_EN == 0 || DP_PANEL_EN == 3
	if(n_10ms_tick ++ > 9)		///10ms   9
	{
		n_10ms_tick = 0;
		proc_led_task(sys->sState,get_libat_percent(sys->volt));
	}
	#endif
#endif
#if	LOCKED_MOTOR_PARA
	judge_left_right_motor_locked_rotor_process(0);
#endif
	// 1HZ 的声音
	/**/
	if(1 == cfg->speaker_able)
	{
	if(sys->t_beep_on >0)
	{
		if((sys->t_beep_on % 1000) ==0)  // 1秒钟向一次
		{
			BEEP_ON();
			sys->t_beep_off = msTmr + 500;		//500后关掉
		//	log_printf("BOn\r\n");
		}
		sys->t_beep_on--;
	}
	if(sys->t_beep_off  >0 && msTmr > sys->t_beep_off )
	{
		sys->t_beep_off = 0;
		BEEP_OFF();
			//log_printf("BOff\r\n");
		}
	}
	else
	{
		sys->t_beep_on = 0;
		sys->t_beep_off = 0;
		BEEP_OFF();
	}
	//暂停闪灯
	if(sys->t_flash_halt)
	{
		if((sys->t_flash_halt % 250) ==0)
		{
			if(sys->t_halt_on)
			{
				LED3_ON();
				sys->t_halt_on =0;
				
			}else
			{
				LED3_OFF();
				sys->t_halt_on =1;
			}
		
		}
		sys->t_flash_halt--;
		if(sys->t_flash_halt==0)
			LED3_OFF();
	}
	if(c++>=2)
	{
		calc_gyro_on_board();
		c = 0;
	}
	
//	uart_timer();
}

//判断是否超时(与设定的时间) 
//注意：msec输入参数为0时的情况
uint8_t mstimeout(uint32_t *timep,uint32_t msec)
{
	uint32_t time,diff;
	uint8_t result=0;
	time=msTmr;
	diff=time- *timep;
	if((msec==0)||(diff>=msec))
	{
		*timep=time;
		result=1;
	}
	return result;
}

//延迟函数
void delay(uint32_t msec)
{
	uint32_t LastTO;
	mstimeout(&LastTO,0);             //设置上次超时时间为当前时间
	while(!mstimeout(&LastTO,msec))   //判断是否超时
	{
	//	feed_dog();                 //喂狗
	}
}
void delay_key_wifi_sensers(uint32_t msec)
{
	uint32_t LastTO;
	mstimeout(&LastTO,0);             //设置上次超时时间为当前时间
	while(!mstimeout(&LastTO,msec))   //判断是否超时
	{
	//	feed_dog();                 //喂狗
		get_sensers(&sys->gSta);
		proc_key_task(sys->sState);
				
	}
}

void delay_sensor(uint32_t msec)
{
	uint32_t LastTO;
	mstimeout(&LastTO,0);             //设置上次超时时间为当前时间
	while(!mstimeout(&LastTO,msec))   //判断是否超时
	{
	//	feed_dog();                 //喂狗
		get_sensers(&sys->gSta);
		//proc_uart_task();
	}
}
void charge_turn_delay_sensor(uint32_t msec)
{
	uint32_t LastTO;
	mstimeout(&LastTO,0);             //设置上次超时时间为当前时间
//	turn_round_pid(0);
	TIM5->CNT = 0;
	while(!mstimeout(&LastTO,msec))   //判断是否超时
	{
	//	feed_dog();                 //喂狗
		proc_uart_task();    
		get_sensers(&sys->gSta);
		if(sys->sState != SYS_DOCK)
			return;
		if(TIM5->CNT >=5000)
		{
			TIM5->CNT = 0;
			www_idleintel_com();
			turn_round_pid(CHARGE_TURN_SPEED_DELAY);//PID
		}
	}
}

void timer5_config(void)//Main(loop) Timer configuration
{
	//Timer2 Config
	TIM_DeInit(TIM5);
	TIM_InternalClockConfig(TIM5);
	TIM5->PSC = 71;	// Set prescaler (PSC + 1)
	TIM5->ARR = 65535;//TIM2_AUTORELOADVALUE;           // Auto reload value 2000
	TIM5->DIER = TIM_DIER_UIE; // Enable update interrupt (timer level)
	TIM5->CNT=0;
	TIM5->CR1 = TIM_CR1_CEN;   // Enable timer
//	NVIC_EnableIRQ(TIM2_IRQn); // Enable interrupt from TIM2 (NVIC level)
}


void delay_ms_tm5(uint32_t ms)
{

	
	for(int i=0;i<ms;i++)
	{
		TIM5->CNT = 0;
		while(TIM5->CNT < 1000);
	}

}

void delay_ms(uint32_t ms)
{/*
	uint32_t LastTO;
	mstimeout(&LastTO,0);			  //设置上次超时时间为当前时间
	while(!mstimeout(&LastTO,ms))   //判断是否超时
	{
	//	feed_dog(); 				//喂狗
	}
	*/
	/**/
#if 1	
		int c=0,k;
		k = (ms * 10) / 3;
		//sys->t_idle = 0;			//???放在这里有问题的。先关闭，不关闭的话掉永远不能关闭红外测距的发射灯
		while(1)
		{
			if(get_irda(&sys->gSta))
				c++;
			if(c>=k)
			{
			//	sys->ird_en = 0;
				return ;
			}
			
		}
#else
	volatile uint32_t nCount;
	RCC_ClocksTypeDef RCC_Clocks;
	RCC_GetClocksFreq (&RCC_Clocks);
	sys->t_idle = 0;
	nCount=(RCC_Clocks.HCLK_Frequency/10000)*ms;
	//log_printf("delay_ms fre:%d,c=%d\r\n",RCC_Clocks.HCLK_Frequency,nCount);
	for (; nCount!=0; nCount--)
	{
		//get_sensers(&sys->gSta);
	}
	//log_printf("delay ok\r\n");
#endif	
}

void delay_ms_sensers(uint32_t ms)
{
	int c=0,k;
	k = (ms * 10) / 3;
	sys->t_idle = 0;
	while(1)
	{
		if(get_irda(&sys->gSta))
			c++;
		if(c>=k)
		{
		//	sys->ird_en = 0;
			return ;
		}
		
	}
}
void turn_delay(float angle,int speed)
{
	uint32_t LastTO;
	mstimeout(&LastTO,0);             //设置上次超时时间为当前时间
//	turn_round_pid(0);
	TIM5->CNT = 0;
	float b_agle = sys->angle;
	float dis_agle=0;
	uint32_t t=0;
	turn_round_pid(0);
	while(1)   //判断是否超时
	{
	//	feed_dog();                 //喂狗
		get_sensers(&sys->gSta);
		
		if(TIM5->CNT >=5000)
		{
			TIM5->CNT = 0;
		//	www_idleintel_com();
			turn_round_pid(TURN_SPEED);//PID

			dis_agle = b_agle -sys->angle;
			if(dis_agle > 180)
				dis_agle = 360 - dis_agle;
			if(dis_agle <-180)
				dis_agle += 360;
			if(dis_agle >=angle || dis_agle <=-angle)
			{
				log_printf("delay turn angle=%3.1f,%3.1f,dis=%3.1f\r\n",sys->angle,angle,dis_agle);
				motor_run(GO_STOP,0,0,0);
				return;

			}
			if(t++>=200)
			{
				log_printf("delay turn timeout,\r\n",sys->angle,angle);
				motor_run(GO_STOP,0,0,0);
				return;
			}
		}
	}
}
