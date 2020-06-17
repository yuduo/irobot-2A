

#include "sys.h"

uint16_t sWorkTimeMinCt,sWorkTimeMin,sSleepTimeMin;

struct sys_t syst,*sys;

gyro_t kgyro;
uint8_t m_stack[STACK_LEN];		//系统堆栈，在函数内部临时使用	
uint8_t	m_stack2[STACK_LEN2];			//局部使用
//

//struct config_t config,*cfg;

//struct irda_t  rx_irda,rx_irda1,rx_irda8;

 #if 0
void read_Flashbuf(u32 addr,u8 *p,u16 n)
{										   
	while(n--)
	{
		*(p++)=*((u8*)addr++);
	}	
}

void configSave(void)
{


	uint16_t i=0;
	uint32_t r1,addr;
	uint8_t *p;								 
		 
	i=CONFIG_SIZE;	//计算字节数
	p=(u8 * )cfg;		   		//对齐拷贝地址
	addr = CFG_ADDER;	   		//FLASH目标地址
//	OS_ENTER_CRITICAL() ;	   //关中断

	FLASH_Unlock();//FLASH解锁
	FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);//清标志位
	FLASH_ErasePage(CFG_ADDER);	 //清除页
	while(i--)
	{
		r1=*(p++);
		r1|=*(p++)<<8;
		r1|=*(p++)<<16;
		r1|=*(p++)<<24;
		FLASH_ProgramWord(addr, r1);
		addr+=4;
	}
	FLASH_Lock();	
//	OS_EXIT_CRITICAL() ; 

//	LEDon;

//	for(i = 0; i < configDataSize; i++)
	{
		//read data from eeprom,
		//check, if it has changed, only then rewrite;
		//WriteToEEPROM(i, configData[i]);
		//Delay_ms(5);							
	}
	
//	LEDoff;
}

#endif







void init_sys(void)
{
	sys=&syst;
//	cfg=&config;
	sys->mState = GO_FORWARD;		//前进
//	sys->nextState=GO_FORWARD;
//	sys->tBr=0;
	sys->c_hw=sys->m_cHw1=sys->m_cHw2=0;
	//sys->trg_speed=NOMORL_SPEED;
//	sys->trace =0;
//	sys->turn_right=0;
	sys->ajust_agle = 3.0f;
	sys->c_bum = 0;
	sys->slip = 0;
//	sys->cleanMod = CLEAN_RANDOM;
	//sys->c_near_angle = 0;
	msTmr=0;
	sys->power_on = FALSE;
	sys->sState = 0xFF;
//	sys->dust_pwm_value = NORM_DUST_PWM;//éè????è?μ?·??ú×a?ù
//	sys->ird_en = 0;
	sys->shut_down_motor = 0;
	sys->t_idle = 0;
//	sys->bottom_ir = 0;
	sys->near_debug = 0;
	sys->ajust_agle = (float)cfg->ajust_agle / 10.0f;
	//sys->m_navi = navigat;
	//sys->m_cfg = cfg;
	init_gyro_g();
	 sys->upmap_en_flag = 1;  
	 sys->sync_wifi = 0;
	 sys->wifistatechange = 0;
	 sys->dis_statechange = 0;
	 	 sys->power_go = 0;
	 	 sys->sys_charge_cur = 500;	//2018-10-09 JZZ
	 	 sys->init_prs = 1;
		 sys->prs = 100;

sys->dusk_usetime_alarm= 0;
sys->strainer_usetime_clear= 0;
sys->side_rush_usetime_clear= 0;
sys->main_rush_usetime_clear= 0;
sys->wifiAreaNoCalc_en = 0;

sys->dccheck_enable = DISABLE;

	sys->auto_lamp_sta = 0;
	sys->dust_pwm_lev = 1;

	sys->upgrade_mode = RESET;
	sys->work_mod = MWO_NORMAL;
	sys->f_work_mode = sys->work_mod;
	sys->Blankets_ground = RESET;
	sys->run_speed = MIDLE_SPEED;
	sys->dust_pwm_value = DUST_MODE_NORMAL;	
	sys->pair_sta = REMOTE_PAIR_NU;
	sys->wifi_switch = SET;
	sys->wifi_switch_old = RESET;
}



#if ROOMA_GYROONLY
/*
0	?íD÷
1	???ú??é¨
2	1-×?D???é¨
3	??μ???é¨
4	??3?
5	??±???é¨
6	×??ˉ??é¨
7	?y?ú3?μ?
253	1??ú

#define SYS_IDLE		0x00		//???D
#define SYS_NAVIGAT		0x01		//μ?o?
#define SYS_FOCUS		0x02		//??μ?
#define SYS_RANDOM		0x03		//???ú
#define SYS_DOCK		0x04		//??3?
#define SYS_NEAR		0x05		//??±?
#define SYS_CHARGE		0x06		//3?μ?
#define SYS_TEST_BD		0x07		//??????2a¨o???ê¨o?
#define SYS_POWEROFF	0xfd	//1??ú,′?×′ì?????á???μ?oìía(′??·?ú1.2μ?ó2?t°?±????T·¨1?±?)/ò￡???óê?,????è?2?1?±?
*/
//?μí3×′ì?×a??μ?WIFIμ?±à??
//static uint8_t m_sys_state2wifi_list[8] = {0,6,3,3,4,5,7,7};
static uint8_t m_sys_state2wifi_list[8] = {0,6,3,5,4,5,7,7};
uint8_t sys_state_convert2wifi(void)
{
	if(sys->sState == SYS_POWEROFF)
		return SYS_POWEROFF;
	return m_sys_state2wifi_list[sys->sState];
}
//WIFIμ?±à??×a??μ??μí3×′ì?
static uint8_t m_wifi2sysstate_list[8] = {SYS_IDLE,SYS_NAVIGAT,SYS_NAVIGAT,SYS_RANDOM,SYS_FOCUS,SYS_RANDOM,SYS_DOCK,SYS_IDLE};
uint8_t wifi_state_convert2sys(uint8_t wifi_mode)
{
	if(wifi_mode == SYS_POWEROFF)
		return SYS_POWEROFF;
	return m_wifi2sysstate_list[wifi_mode];
}
#endif


void init_gyro_g(void)
{

		kgyro.angle = &(sys->angle);
		kgyro.rpwm = &(sys->rpwm);
		kgyro.lpwm = &(sys->lpwm);
		kgyro.pwm = &(sys->pwm);
		kgyro.gyro_ofs1 = &(cfg->gyro_ofs1);
		kgyro.gyro_ofs2 = &(cfg->gyro_ofs2);
		kgyro.kp2 = &(cfg->kp2);
		kgyro.ki2 = &(cfg->ki2);
		kgyro.kd2 = &(cfg->kd2);
		kgyro.x_org_f = &(navigat->x_org_f);
		kgyro.y_org_f = &(navigat->y_org_f);		
		kgyro.x_org_r = &(navigat->x_org_r);
		kgyro.y_org_r = &(navigat->y_org_r);		
		kgyro.x_org_t = &(navigat->x_org_t);
		kgyro.y_org_t = &(navigat->y_org_t);		
		kgyro.x_org = &(navigat->x_org);
		kgyro.y_org = &(navigat->y_org);	
		kgyro.radius = &(navigat->radius);
		kgyro.hw_grid =&(navigat->hw_grid);
		kgyro.pid = &rpid;
	
}


char check_io(uint16_t valu)
{
 	int i,j;
	for(i=0;i<5;i++)
	{
		if((GPIOB->IDR & valu)!=0)
			return 0;
	    for(j=0;j<10;j++);
	}
	return 1;
}

char check_io_h(uint16_t valu)
{
 	int i,j;
	for(i=0;i<5;i++)
	{
		if((GPIOB->IDR & valu)!=valu)
			return 0;
	    for(j=0;j<10;j++);
	}
	return 1;
}

int disXY(int x,int y)
{
	return(x>y?(x-y):(y-x));
}
int dis_xy(short x,short y)
{
	return(x>y?(x-y):(y-x));
}
float disf_xy(float x,float y)
{
	return(x>y?(x-y):(y-x)); 
}


void init_tim3_camp(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
  //  TIM_ICInitTypeDef TIM_ICInitStruct;
  // TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
   
   //RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM8 , ENABLE);
   //RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure); 
   /*使用TIM8_CH2(PC7口)作为捕获输入引脚*/ 


	TIM3->PSC = 71;
	TIM3->ARR = 2000;
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
	
		
		//	CH3
			TIM4->CCMR2 |= 1 << 0;
			TIM4->CCMR2 |= 5 << 4;		//滤波器设为5
			if (dir == 0)
				TIM4->CCER |= 1 << 9; //下降沿捕获
			else 
				TIM4->CCER &= ~(1 << 9); //上升沿捕获
			TIM4->CCER |= 1 << 8;
			TIM4->DIER |= 1 << 3; 
	
		case 2: //CCR2 PA1 	*/
			TIM3->CCMR1 |= 1 << 8; //CCR2配置通道方向:输入
			TIM3->CCMR1 |= 5 << 12;		//滤波器设为5
		//	if (dir == 0)
		//		TIMER->CCER |= 1 << 5; //下降沿捕获
		//	else 
				TIM3->CCER &= ~(1 << 5); //上升沿捕获
			TIM3->CCER |= 1 << 4; //CCR2通道捕获使能
			TIM3->DIER |= 1 << 2; //CCR2通道允许捕获中断
		/*	case 1: //>CCR1 PA0
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
	TIM3->DIER |= 1 << 0;     //允许更新中断
//	MY_NVIC_Init(1, 2, TIM2_IRQChannel, 2);     //中断
	TIM3->CR1 = 0x01;    //使能定时器
	TIM3->SR &= ~(1 << 0);


}

void en_pwm(uint8_t chnl,uint8_t en)
{
//	log_printf("en pwm,chnl=%d,en=%d\r\n",chnl, en);
 	switch(chnl)
 	{
 		case 0:
			TIM3->CCER &=0xCCCC;		//PA8 PWM禁止	
 			break;
		case 1:
			if(en)
				TIM3->CCER|=3<<0; //OC1 输出使能
			else
				TIM3->CCER &=0xFFFC;		//PA8 PWM禁止
			break;
		case 2:						//PA7,目前只用着一路
			if(en)
				TIM3->CCER|=3<<4; //OC2 输出使能
			else
				TIM3->CCER &=0xFFCF;		//PA9 PWM禁止
			break;
		case 3:
			if(en)
				TIM3->CCER|=3<<8; //OC2 输出使能
			else
				TIM3->CCER &=0xFCFF;		//PA9 PWM禁止
			break;
		case 4:
			if(en)
				TIM3->CCER|=3<<12; //OC2 输出使能	
			else
				TIM3->CCER &=0xCFFF;		//PA9 PWM禁止
			break;
		
	
	
 	}
	
}

void waiting_stop(void)
{
	while(sys->mState !=GO_STOP)
	{
	}
}

void waiting_hw(uint16_t chw)
{
	uint16_t gSta;
	while(chw > sys->m_cHw1)
	{
		get_sensers(&gSta);
		//wy190527 新增左右两侧的碰撞开关检测
		if(gSta & (MASK_BUM_LEFT | MASK_BUM_LEFT2 | MASK_BUM_RIGHT | MASK_BUM_RIGHT2 | MASK_FW_MIDL))
		{
			log_printf("[waiting_hw]found bum!!!!\r\n");
			return;
		}
	}
	log_printf("[waiting_hw]m_cHw1=%d(%d)\r\n",sys->m_cHw1,chw);
}

#define NEAR_LEFT	1
#define NEAR_RIGHT	2

#define NEAR_FORWARD_HW		200
#define NEAR_MAX_TURN		300
#define NEAR_MAX_BACK		40
uint8_t wall_sta=0;
//uint8_t c_bum=0;
uint8_t c_lost_left=0;
void init_near_wall(void)
{
	log_printf("init_near_wall\r\n");
//	sys->cleanMod = CLEAN_NEAR;
	wall_sta=0;
//	c_bum =0;
	motor_run(GO_STOP, 0, 0, 0);
	motor.c_left_hw = motor.c_right_hw = 0;
	motor_run(GO_FORWARD,NOMORL_PWM,0,GO_FORWARD);
}

#define NER_TURN_PWM	780
void near_wall_mode(uint8_t gsta)
{
//	uint16_t g_sta=0; 
	uint16_t gSta,g_sta1;
	uint8_t n_loop=0,i;
//	uint16_t c_hw;
	get_sensers(&gSta);
	for(i=0;i<5;i++)
	{
		get_sensers(&g_sta1);
		gSta |=g_sta1;
	}

	switch(wall_sta)
	{
		case 0:			//没进入左右模式
			if(gSta &  ( MASK_FW_LEFT) && sys->mState ==GO_FORWARD)
			{
				log_printf("[nearwall]FW_MIDL,in the left side,turn right...\r\n");
				motor_run(GO_STOP, 0, 0, 0);
				delay(5);
				//motor.c_left_hw = motor.c_right_hw = 0;
				motor_run(GO_RIGTH,NER_TURN_PWM,70,0);		
				n_loop=1;
				wall_sta=NEAR_LEFT;
				//motor_run(GO_F_SLOW, SLOW_PWM, 3000, GO_F_SLOW);
			}
			//if((sys->mState ==GO_FORWARD) || (sys->mState ==GO_F_SLOW))
			{

				if(gSta & (MASK_BUM_LEFT |MASK_BUM_RIGHT))
				{
					//motor.c_left_hw = motor.c_right_hw = 0;
					log_printf("[nearwall]left bum,back,and turn right 20 hw\r\n");
					motor_run(GO_BACK,NOMORL_PWM,100,0);	
					waiting_stop();
					delay(5);
					if((gSta & (MASK_BUM_LEFT |MASK_BUM_RIGHT)) ==MASK_BUM_LEFT)
					{
						log_printf("only left bum,turn 90hw\r\n");
						motor_run(GO_RIGTH,NER_TURN_PWM,50,0);
					}
					else
					{
						log_printf("middle bum ,turn 200chw\r\n");
						motor_run(GO_RIGTH,NER_TURN_PWM,80,0);
					}
					
					log_printf("[nearwall]turn ok,GO_FORWARD and into NEAR_LEFT mode\r\n");
					wall_sta=NEAR_LEFT;
					n_loop=1;
				}	

			}
			break;
		case NEAR_LEFT:

			if(gSta & (MASK_BUM_LEFT | MASK_BUM_RIGHT |  MASK_FW_MIDL ))
			{
				if(gSta & (MASK_BUM_LEFT | MASK_BUM_RIGHT))
				{
					log_printf("[NEAR LEFT]bum!!back ,and ,turn right\r\n");
				}
				else
					log_printf("[NEAR LEFT]left slant,stop and turn right 10 chw\r\n");

				motor_run(GO_STOP, 0, 0, 0);
				//delay(20);
				//motor.c_left_hw = motor.c_right_hw = 0;
				if(gSta & (MASK_BUM_LEFT | MASK_BUM_RIGHT))
				{					
					motor_run(GO_BACK,NOMORL_PWM,80,0);	
					waiting_stop();	
				}
				//if(gSta && MASK_FW_MIDL)
				//	motor_run(GO_RIGTH,NER_TURN_PWM,100,0);
				//else
					motor_run(GO_RIGTH,NER_TURN_PWM,35,0);
				n_loop=1;
				
			}else

			if(gSta & MASK_LEFT)
			{
				motor_run(GO_STOP, 0, 0, 0);
				motor_run(GO_RIGTH,NER_TURN_PWM,10,0);
				n_loop=1;
				c_lost_left=0;
			}else
			{
				c_lost_left ++;
				motor_run(GO_STOP, 0, 0, 0);
				if(c_lost_left >3 )
					motor_run(GO_LEFT,NER_TURN_PWM,30,0);
				else
					motor_run(GO_LEFT,NER_TURN_PWM,10,0);
				n_loop=1;
				log_printf("c lost left=%d\r\n",c_lost_left);
			}
			
			

			break;
		case NEAR_RIGHT:
			log_printf("right side ,no code...\r\n");
			/*
			if(gSta & (MASK_BUM_LEFT | MASK_BUM_RIGHT |MASK_FW_RIGHT| MASK_FW_MIDL))
			{
				if(gSta & (MASK_BUM_LEFT | MASK_BUM_RIGHT))
				{
					c_bum++;
					log_printf("[NEAR RIGHT]bum!!c_bum=%d,back ,and ,turn left\r\n",c_bum);
				}
				else
					log_printf("[NEAR RIGHT]right slant,stop and turn left 10 chw\r\n");

				motor_run(GO_STOP, 0, 0, 0);
				delay(20);
				if(gSta & (MASK_BUM_LEFT | MASK_BUM_RIGHT))
				{
					motor_run(GO_BACK,NOMORL_PWM,20,0);	
					waiting_stop();	
					delay(20);
					motor_run(GO_LEFT,NER_TURN_PWM,20,0);
				}else
					motor_run(GO_LEFT,NER_TURN_PWM,10,0);
				n_loop=1;
				
			}else
			if((gSta & MASK_RIGHT)==0)
			{
				log_printf("[NEAR LEFT]lost right wall,turn right\r\n");
				motor_run(GO_STOP, 0, 0, 0);
				delay(20);
				motor_run(GO_RIGTH,NER_TURN_PWM,10,0);
				n_loop=1;
			}
			*/
			break;

	}

	if(n_loop)
	{
		waiting_stop();
	//	delay(20);
		log_printf("[n_loop]turn ok,GO_FORWARD\r\n");
		//motor.c_left_hw = motor.c_right_hw = 0;
		motor_run(GO_FORWARD,NOMORL_PWM,0,GO_FORWARD);
		//sys->trg_speed2=sys->trg_speed=2200;
		/*
		if(c_bum >=3)
		{
			log_printf("maney bumm.....!!!\r\n");
			waiting_hw(0);
		}
		else
		*/
			waiting_hw(80);
	}
	
#if 0



	if((gSta &  MASK_FW_MIDL ) && sys->mState == GO_FORWARD)
	{
		log_printf("[nearwall]FW_MIDL,slow down...\r\n");
		motor_run(GO_F_SLOW, SLOW_PWM, 3000, GO_F_SLOW);		
	}else if((sys->mState ==GO_FORWARD) || (sys->mState ==GO_F_SLOW))
	{
		if(wall_sta ==NEAR_RIGHT && sys->m_cHw1> NEAR_FORWARD_HW)		//右边延边
		{
			if((gSta & MASK_RIGHT) ==0)		//沿边消失了
			{
				log_printf("[nearwall]right side,lost wall,turn right 10 hw\r\n");
				motor_run(GO_STOP, 0, 0, 0);
				motor_run(GO_RIGTH,NER_TURN_PWM,10,0);
				waiting_stop();
				delay(20);
				log_printf("[nearwall]right side,turn right ok,GO_FORWARD\r\n");
				motor_run(GO_FORWARD,NOMORL_PWM,0,GO_FORWARD);
			}
		}else if(wall_sta ==NEAR_LEFT&&  sys->m_cHw1> NEAR_FORWARD_HW)
		{
			if((gSta & MASK_LEFT) ==0)		//沿边消失了
			{
				log_printf("[nearwall]left side,lost wall,turn left 10 hw\r\n");
				motor_run(GO_STOP, 0, 0, 0);
				motor_run(GO_LEFT,NER_TURN_PWM,10,0);
				waiting_stop();
				delay(20);
				log_printf("[nearwall]left side,turn left ok,GO_FORWARD\r\n");
				motor_run(GO_FORWARD,NOMORL_PWM,0,GO_FORWARD);
			}
		}
	}

	if(gSta & MASK_BUM_LEFT)
	{
		log_printf("[nearwall]left bum,back\r\n");
		motor_run(GO_BACK,NOMORL_PWM,NEAR_MAX_BACK,0);	
		waiting_stop();

		if(wall_sta == NEAR_RIGHT)
		{
			log_printf("[nearwall]I am in right,turn left\r\n");
			motor_run(GO_LEFT,NER_TURN_PWM,NEAR_MAX_TURN,0);
		}else
		{
			//往右边转，直到前方的探头消失
			log_printf("[nearwall]turn right and waiting for ird dis\r\n");
			motor_run(GO_RIGTH,NER_TURN_PWM,NEAR_MAX_TURN,0);
			wall_sta=NEAR_LEFT;
		}
		n_loop=1;
	}else if(gSta & MASK_BUM_RIGHT)
	{
		log_printf("[nearwall]right bum,back\r\n");
		motor_run(GO_BACK,NOMORL_PWM,NEAR_MAX_BACK,0);	
		waiting_stop();
		delay(20);
		//往右边转，直到前方的探头消失
		if(wall_sta == NEAR_LEFT)
		{
			log_printf("[nearwall]i am in left,TURN RIGHT\r\n");
			motor_run(GO_RIGTH,NER_TURN_PWM,NEAR_MAX_TURN,0);
		}else
		{
			log_printf("[nearwall]turn left and waiting for ird dis\r\n");
			motor_run(GO_LEFT,NER_TURN_PWM,NEAR_MAX_TURN,0);
			wall_sta=NEAR_RIGHT;
		}
		n_loop=1;
		
	}


	c_hw=0;
	if(n_loop)
		log_printf("begin nloop...\r\n");
	
	while(n_loop)
	{
		get_sensers(&gSta);
		get_sensers(&g_sta1);
		get_sensers(&g_sta2);
		gSta |=g_sta1;
		gSta |=g_sta2;
	//	log_printf("[near_wall_mode]loop,gSta:%02X\r\n",gSta);
		if(sys->mState ==GO_STOP)
		{
			log_printf("[nearwall]motor stop,turn error,GO_FORWARD\r\n");
			motor_run(GO_STOP, 0, 0, 0);
			motor_run(GO_FORWARD,NOMORL_PWM,0,GO_FORWARD);			
			break;
		}
		//if(((gSta & (MASK_FW_LEFT | MASK_FW_RIGHT | MASK_FW_RIGHT))==0)
		//   && sys->m_cHw1 > 30)




		if(((gSta & (MASK_FW_LEFT | MASK_FW_RIGHT | MASK_FW_RIGHT))==0) && sys->m_cHw1 > 30)
		{
			log_printf("[nearwall]ird Disappear,turn OK,GO_FORWARD\r\n");
			c_hw = sys->m_cHw1;
			delay(20);
			motor_run(GO_STOP, 0, 0, 0);
			/*
			motor_run(GO_STOP, 0, 0, 0);
			while(1)
			{
				proc_msg_printf();
			}*/
			motor_run(GO_FORWARD,NOMORL_PWM,0,GO_FORWARD);	
			
			break;
		}
		
	}
	/*
	if(c_hw >0)
	{
		while((sys->m_cHw1-c_hw) < 50);

		log_printf("[nearwall]ird Disappear,turn OK,STOP AND GO_FORWARD\r\n");
		motor_run(GO_STOP, 0, 0, 0);
		motor_run(GO_FORWARD,NOMORL_PWM,0,GO_FORWARD);

	}
	*/
#endif	
}
  /*
void send_irda(uint8_t chnl,uint16_t bt)
{
	irda_timer_cfg();
 	sys->tx_byte= bt;
 	sys->chnl = chnl;
 	en_pwm(chnl, 1);						
 	TIM_Cmd(STIME, ENABLE); //是能定时器
 	STIME->ARR = 800;	//低电平0.8ms
 	sys->sta=1;			//发送引导吗
 	log_printf("[send_irda]chnl=%d,=%02X\r\n",chnl,bt);
 	//发送的时候，把接收的关掉byte
 	TIM_Cmd(TIM8, DISABLE); 
	
}
 */
#if 0

//测试方
uint8_t  c_dock_cyc=0;
uint8_t  c_max_cock;
//uint8_t  motor_sta=0;

#define  DOCK_AREA_NO	0x00		//没找到区域
#define  DOCK_AREA_LR	0x01		//左右
#define  DOCK_AREA_M	0x02		//  456
#define  DOCK_AREA_L		0x03		//6 7 8
#define  DOCK_OK			0x04

#define DOCK_AREA_S	0xFF

extern char motorStr[7][6];

uint8_t dock_area;		//区域

uint16_t c_right;
uint16_t r_count=0,l_count=0;
uint16_t c_lost=0;

#define SITE_NONE	0x00
#define SITE_LEFT  	0x01
#define SITE_RIGHT 	0x02
struct site_t
{
	uint8_t 	site;		//
	uint8_t 	*v_ird;	//用于判断垂直的IRD
	uint8_t	*m_ird;
	uint8_t	*my_ird;
	uint8_t 	 *m_line_ird;	//中线线的标志
	uint8_t    m_dir;	//转回来，垂直的方向	
	uint8_t	back_dir;	//转回去一点
	uint8_t  	is_middle;
};

struct site_t l_site,r_site,*p_site;

void  docking_init(void)
{
	log_printf("docking_init...\r\n");
	dock_area=DOCK_AREA_NO;
	sys->sState = SYS_DOCK;
	motor_run(GO_RIGTH,DOCK_PWM,3000,GO_STOP);	//注意到，这里要用到 sys->nextState
	
	l_site.v_ird  = &iLeft.l_360;
	l_site.m_dir = GO_LEFT;
	l_site.back_dir = GO_LEFT;
	l_site.m_ird =  &iL360.right;
	l_site.my_ird =  &iL360.left;
	l_site.m_line_ird = &iLeft.left;

	r_site.v_ird = &iRight.r_360;
	r_site.m_dir = GO_RIGTH;
	r_site.back_dir = GO_RIGTH;
	r_site.m_ird = &iL360.left;
	r_site.my_ird =  &iL360.right;
	r_site.m_line_ird = & iLeft.left;
	

}

#define DOCK_RIGHT_CHW		300
#define DOCK_INIT_CHW			800

uint8_t sta_site=0;
uint8_t cleft=0;
//侧边行走



	
char docking_site(void)
{
	switch(sta_site)
	{
		case 0:		//侧边，电机右转或左转，寻找垂直点
			//没找到那个点，转过头了，则失败，现在怎么处理，没想好
			if(sys->m_cHw1 >=1000)
			{
				log_printf("[docking_site]not found V,m_cHw1 >=1000\r\n");
				sta_site=20;
				break;
			}
			if(sys->mState == GO_STOP)
			{
				log_printf("[docking_site]back OK,now GO_FORWARD for founding middle...\r\n");
				motor_run(GO_FORWARD, cfg->dock_pwm, 4000, GO_STOP);
				sys->trg_speed=sys->trg_speed2=cfg->dock_speed;
				sta_site = 1;		//进入直行阶段
				p_site->is_middle = 0;
				cleft=0;
			}else
			{
				if( *p_site->v_ird ==0)		//消失了，证明垂直了。
				{
					log_printf("[docking_site]v_ird lost,we found V!!,now go little back %d...\r\n",cfg->dock_hw_back);
					motor_run(GO_STOP, 0, 0, 0);
					//稍微转回去一点
					motor_run(p_site->back_dir, cfg->dock_pwm, cfg->dock_hw_back, GO_STOP);
					waiting_stop();
				}
			}
			break;
		case 1:	//直行阶段，寻找中心点
		/*
			//这里要做个判断
			if(sys->m_cHw1 >=2000)
			{
				log_printf("[docking_site]not found M,m_cHw1 >=2000\r\n");
				sta_site=20;
				break;
			}

			*/
			/*
			if(sys->mState == GO_STOP)		//停止了，要转回去。
			{
				log_printf("[docking_site]found M,now turn to middle\r\n");
				p_site->is_middle =0;
				motor_run(p_site->m_dir, 870, 2000, GO_STOP);
				sta_site = 2;
				break;
			}else
			*/
			//if(sys->mState == GO_FORWARD)
			{

			//	if()
				//if(*p_site->m_ird && *p_site->my_ird==0 )			//到了中间点了，再直行一点
				if(((iL360.left==0 && iL360.right >0) && (p_site->back_dir == GO_LEFT)) ||
					((iL360.right==0 && iL360.left>0) && (p_site->back_dir == GO_RIGTH)) )
				{ 
					if(cleft++ >=2/**/)
					{
						
						log_printf("[docking_site]found Middle,stop and still go %d chw...\r\n",cfg->dock_hw_forwak);
						motor_run(GO_STOP, 0, 0, 0);
						//motor_run(GO_FORWARD, 860, 170, GO_STOP);
						motor_run(GO_FORWARD, cfg->dock_pwm+60, cfg->dock_hw_forwak, GO_STOP);
						sys->trg_speed=sys->trg_speed2=0;
						sta_site = 2;
					}
				}
			}
			break;
		case 2:
			if(sys->mState == GO_STOP)		//停止了，要转回去。
			{
				log_printf("[docking_site]go 170 over,now turn to middle\r\n");
				p_site->is_middle =0;
				
				motor_run(p_site->m_dir, cfg->dock_pwm+30, 2000, GO_STOP);
				sta_site = 3;
				cleft=0;
				
				break;
			}	
			break;
		case 3:		//寻找中心线
			if(sys->m_cHw1 >=1000)
			{
				log_printf("[docking_site]found middle line ERROR\r\n");
				sta_site=20;
				break;
			}
			//if(iLeft.left)
			//	p_site->is_middle|=1;
			//if(iRight.right)
			//	p_site->is_middle |=2;
			//if((p_site->is_middle & 3) ==3)
			//if(*p_site->m_line_ird)
			//if( (iLeft.left &&  (iRight.right || iRight.left) && p_site->m_dir == GO_LEFT)	||
			//	(iRight.right&&  (iLeft.right || iLeft.left) && p_site->m_dir == GO_RIGTH))
			 if(iLeft.left &&  (iRight.right || iRight.left) )
			
			 {
			 	if(cleft++ > cfg->dock_hw_turn)
			 	{
					log_printf("[docking_site]found middle line OK\r\n");
					motor_run(GO_STOP, 0, 0, 0);
					//while(1);
					return 1;
				}
			}
			break;
		default:
			log_printf("[docking_site]error,sta_site=%d\r\n",sta_site);
			//return 2;
			break;
	}
	return 0;
		
}
uint8_t sta_midle=0;
int m_pwm=0;
#define M_PWM	770
char docking_middle(void)
{
	char i,c=0,j;
	switch(sta_midle)
	{
		case 0:		

			if(iLeft.left || iRight.right)	//继续直行
			{
				if(sys->mState !=GO_FORWARD)
				{
					log_printf("[dock_middle]moter sta!=FORWARD,now GO_FORWARD\r\n");
					motor_run(GO_STOP, 0, 0, 0);
					motor_run(GO_FORWARD, cfg->dock_pwm+30, 3000, GO_STOP);
				}

				if(iLeft.left && iRight.right)		//最中间位置
				{
					log_printf("[dock_middle]left && right,we are in the middle of the middle,GO_FORWARD...\r\n");
					motor_run(GO_FORWARD, cfg->dock_m_pwm, 3000, GO_STOP);
					sta_midle=1;
					
				}				
			}else 
			{
				if(sys->mState == GO_FORWARD )
				{
					if(iLeft.right)
					{
						log_printf("[dock_middle]lost the midle,in the right,mast go left\r\n");
						motor_run(GO_STOP, 0, 0, 0);
						motor_run(GO_LEFT, cfg->dock_pwm, 1000, GO_STOP);
					}else if(iRight.left)
					{
						log_printf("[dock_middle]lost the midle,in the left,mast go right\r\n");
						motor_run(GO_STOP, 0, 0, 0);
						motor_run(GO_RIGTH, cfg->dock_pwm, 1000, GO_STOP);
					}else if(iL360.left )
					{
						log_printf("[dock_middle]lost the midle,in the left left,mast go right\r\n");
						motor_run(GO_STOP, 0, 0, 0);
						motor_run(GO_RIGTH, cfg->dock_pwm, 1000, GO_STOP);
					}else
					{
						log_printf("[dock_middle]lost the midle,in the right right,mast go right\r\n");
						motor_run(GO_STOP, 0, 0, 0);
						motor_run(GO_LEFT, cfg->dock_pwm, 1000, GO_STOP);

					}
				}
			}
	
			break;
		case 1:
			//log_printf("MMMMM\r\n");
			if(iLeft.left && iRight.right)	//继续直行
			{
				if(sys->mState !=GO_FORWARD)
				{
					log_printf("MM,moter sta != forward,now GO_FORWARD\r\n");
					motor_run(GO_STOP, 0, 0, 0);
					motor_run(GO_FORWARD, cfg->dock_m_pwm, 3000, GO_STOP);
				}
	
			}else if(iLeft.left)
			{
				log_printf("MM,in the LEFT,now to the right\r\n");
				motor_run(GO_STOP, 0, 0, 0);
				motor_run(GO_RIGTH, cfg->dock_m_pwm, 3000, GO_STOP);				
			}else 
			{
				log_printf("MM,in the RIGHT,now to the left\r\n");
				motor_run(GO_STOP, 0, 0, 0);
				motor_run(GO_LEFT, cfg->dock_m_pwm, 3000, GO_STOP);	
			}
			c=0;
			for(i=0;i<50;i++)
			{
				if((GPIOE->IDR & 1) ==0)
					c++;
				for(j=0;j<8;j++);
			}
			if(c >=40)
			{
				log_printf("charge OK,STOP \r\n");
				motor_run(GO_STOP, 0, 0, 0);
				return 1;
			}
			break;
	}
	return 0;
}



	
void docking(void)
{
	char i,c,j;
//左接收 左3
/**/
	if(sys_debug.debug ==DEBUG_SES)
	{
		log_printf("(%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d)\r\n",
		iLeft.l_360,iLeft.left,iLeft.right,iLeft.r_360,
		iL360.l_360,iL360.left,iL360.right,iL360.r_360,
		iRight.l_360,iRight.left,iRight.right,iRight.r_360);
	}


  /*
	iLeft.left=iLeft.right=iLeft.l_360=0;
	
	iRight.left=iRight.right=iRight.l_360=0;
	iL360.left=iL360.right=iL360.l_360=0;	
	iLeft.r_360=iRight.r_360=iL360.r_360=0;
	return ;
	*/		
	//计算，去除有问题的数据，计算偏左，偏右。
	switch (dock_area)
	{
		case DOCK_AREA_NO:
		//	log_printf("[DOCK_AREA_NO]m_cHw1=%d,%d\r\n",sys->m_cHw1,iLeft.l_360);
			if(sys->mState == GO_RIGTH)
			{
				if(sys->m_cHw1 < 1000)		//第一圈发现
				{
					if(iLeft.l_360)		//发现在左边
					{
						dock_area = DOCK_AREA_LR;	
						log_printf("[DOCK_AREA_NO]found left side.we will turn right...\r\n");
						motor_run(GO_STOP, 0, 0, 0);
						p_site = &l_site;
						sta_site=0	 ;
						motor_run(GO_RIGTH, cfg->dock_pwm, 2000, GO_STOP);
						
						//dock_area=20;
					}else if(iRight.r_360)  //发现右边
					{
						log_printf("[DOCK_AREA_NO]found RIGHT side.we will turn left...\r\n");
						dock_area = DOCK_AREA_LR;		
						motor_run(GO_STOP, 0, 0, 0);
						p_site = &r_site;
						sta_site=0	 ;
						motor_run(GO_LEFT, cfg->dock_pwm, 2000, GO_STOP);
						
					}
					/*
					else if(iLeft.left &&  iRight.right) 
					{
						log_printf("[DOCK_AREA_NO]found the middle.\r\n");
						motor_run(GO_STOP, 0, 0, 0);
						dock_area = DOCK_AREA_M;
						//motor_run(GO_FORWARD, DOCK_PWM, 3000, GO_STOP);
						sta_midle =0;						
					}
					*/
				}
			}
			break;
		case DOCK_AREA_LR:
			switch(docking_site())
			{
				case 1:		//找到中间的线
					dock_area = DOCK_AREA_M;
					//motor_run(GO_FORWARD, DOCK_PWM, 3000, GO_STOP);
					sta_midle =0;
					break;
				case 2:		//失败

					break;
				default :
					break;
			}
			break;
		case DOCK_AREA_M:
		/**/
			switch(docking_middle())
			{
				case 1:
					dock_area = DOCK_OK;
					break;
				case 2:
					break;
				default:
					break;
			}
			
			break;
		case DOCK_OK:
			c=0;

			for(i=0;i<60;i++)
			{
				if((GPIOE->IDR & 1))
					c++;
				for(j=0;j<10;j++) ;
			}
			if(c >=50)
			{
				log_printf("charge lost,init docking...\r\n\r\n");
				docking_init();
			}
			
			break;
		default:
			break;
	}


	iLeft.left=iLeft.right=iLeft.l_360=0;
	
	iRight.left=iRight.right=iRight.l_360=0;
	iL360.left=iL360.right=iL360.l_360=0;	
	iLeft.r_360=iRight.r_360=iL360.r_360=0;
	
}


struct r_ird_t iLeft,iRight,iL360;
#define MAX_RX_BIT		8


void rx_ird(struct r_ird_t *rx_ird)
{

		 uint32_t ccr,t_now;
		 int idx;
		 t_now= sys->tird + STIME->CNT;		//获取当前的时间
		ccr = t_now-rx_ird->t_last;				//计算时间差
		rx_ird->t_last=t_now;

	//	log_printf("ccr:%d\r\n",ccr);
		//时间太长了，重新开始接收,第二个上升沿，是引导码，去掉
		if(ccr >=300)
		{
			rx_ird->rx_idx1=rx_ird->rx_byte1=rx_ird->rx_rbe1=0;
		}

		

		if(rx_ird->rx_idx1 > 0  && rx_ird->rx_idx1 <=MAX_RX_BIT)  //去除掉引导码
		{
			  idx=rx_ird->rx_idx1-1;

			  if(ccr >=60 && ccr < 135)		// 0 以1000为中心
			  {
				// if(ccr<60 || ccr >80)
				 //	rx_ird->rx_rbe1++;
				 	//log_printf("far ccr=%d\r\n",ccr);
			  }
			  else if(ccr >=145 && ccr < 200)	  // 1 以1200为中心
			  {
				rx_ird->rx_byte1 |=(1 <<idx);
				// if(ccr<90 || ccr >130)
				 //	rx_ird->rx_rbe1++;
				 				
			  }else
			  {
				//log_printf("rx error,idx=%d,ccr=%d\r\n",rx_ird->rx_idx1,ccr);
			  	rx_ird->rx_idx1=rx_ird->rx_byte1=rx_ird->rx_rbe1=0;
			  	return ;
			  }

	
			  
			  if(rx_ird->rx_idx1 ==MAX_RX_BIT)
			  {
			  	//if(sys->trace)
			  //	log_printf("rx:%X,%d\r\n",rx_ird->rx_byte1,rx_ird->rx_rbe1);
			  //	insert_irda(sys->rx_byte1);
			  /**/
			  	if(rx_ird->rx_byte1==0xE1)
			  		rx_ird->right++;
			  	else if(rx_ird->rx_byte1==0xD2)
			  		rx_ird->l_360++;
			  	else if(rx_ird->rx_byte1==0xC3)
			  		rx_ird->left++;		
		  		else if(rx_ird->rx_byte1==0xB4)
			  		rx_ird->r_360++;		
			  	
			  	rx_ird->rx_idx1=rx_ird->rx_byte1=rx_ird->rx_rbe1=0;
			  }

			
		}
		rx_ird->rx_idx1++;
}
#endif

//找出队列中最多的那个数组
int find_big_count( int *n ,int nNum )
{

	int i,j;
	int nTmp;
	int p[MAX_ADJ_XY] ;
	int idx=0;

	for(i=0;i<MAX_ADJ_XY;i++)
		p[i] = 0;
	for ( i = 0; i < nNum; i++)
	{
	
	    for ( j = 0; j < nNum;j++)
	    {
	        if (n[i] == n[j])
	        {
	            p[i]++;
	        }
	    }
	}
	nTmp = p[0];
	idx = 0;
	for ( i = 0; i< nNum;i++)
	{
	    if (p[i] >= nTmp)
	    {
	        nTmp = p[i];
	        idx = i;
	    }
	}
	if(idx >=MAX_ADJ_XY)
		return n[0];
	else
	 return n[idx];
}
 

void init_robt_run_ctrl(int deg,int ctrl)
{
	
	//控制走
	if(sys->walk_ctrl )	//已经在走了，就直接刷新计数器即可
		mstimeout(&sys->t_work,0);
	if(ctrl)
	{
		mstimeout(&sys->t_work,0);
		if(deg == 0)	//直行
		{
			motor_run(GO_FORWARD, 550, 0, 0);
		}else if(deg == 180)
		{
			motor_run(GO_BACK,550,0,0);
		}else if(deg == 90 || deg == 270)
		{
			turn_to_deg(deg);
			motor_run(GO_FORWARD, 550, 0, 0);
		}
	}
	sys->walk_ctrl = ctrl;
}
void robot_run_ctrl(void)
{
	if(sys->walk_ctrl)	//控制行走
	{
		if(mstimeout(&sys->t_work,500))
		{
			motor_run(GO_STOP,0,0,0);
			sys->walk_ctrl = 0;
		}

	}
}
uint8_t in_area(int16_t x,int16_t bx,int16_t ex)
{
	int16_t min,max;
	max = bx >= ex?bx:ex;
	min = bx <= ex?bx:ex;
	if(x >= min && x<=max)
	{
		return 1;
	}
	return 0;
}

