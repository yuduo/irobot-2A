

#include "sys.h"

#if ROOMA_GYROONLY
///////////////////////////////按键及LED显示代码段///////////////////////////////////////////
#define KEY_SHAKE_CT_MAX	5//100 防抖的计数最小值
#define KEY_LONG_PRESS_MAX	1000//4000 长按的计时器最小值taio

//uint8_t n_wifi_work_state;

//static uint8_t key_value = 0;
int keystrokes;
// keystrokes=0;

/**********************************************************************
* Function Name  : key_io_init
* 显控面板的IO口初始化,包括按键及LED相关IO口的初始化
**********************************************************************/
void key_io_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
#if(1 == HOOVER)
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
	GPIO_Init(GPIOB,&GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3  |GPIO_Pin_4 |GPIO_Pin_12;
	GPIO_Init(GPIOD,&GPIO_InitStructure);
#else
#if DP_PANEL_EN > 1
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11  |GPIO_Pin_10; //LED两个控制引脚
	GPIO_Init(GPIOC,&GPIO_InitStructure);
	GPIOC->ODR &= ~GPIO_Pin_11;
	GPIOC->ODR &= ~GPIO_Pin_10;
#else
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11  |GPIO_Pin_12; //LED两个控制引脚
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	GPIOA->ODR &= ~GPIO_Pin_11;
	GPIOA->ODR &= ~GPIO_Pin_12;
#endif
#endif

	GPIO_InitStructure.GPIO_Pin = KEY_WIFI_PIN |KEY_SPOT_PIN|KEY_DOCK_PIN|KEY_CLEAN_PIN; //KEY1 KEY2 KEY3 KEY4
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(GPIOA,&GPIO_InitStructure);
}


#ifndef KEY_MAX_COUNT
#define KEY_MAX_COUNT  4
#endif
void all_key_led_off(void)
{
	LED1_OFF();
	//
	LED3_OFF();
	LED4_OFF();
	sys->t_flash_halt = 0;
	sys->t_halt_on = 1;
	if(sys->sState != SYS_TEST_BD)
	log_printf("all_key_led_off\r\n");
}

extern uint8_t r_key_err;
static uint32_t n_key_read_tmr = 0;
static uint16_t n_key_pressed_ct[KEY_MAX_COUNT] = {0xffff};
static uint8_t key_read(void)
{
	if(n_key_read_tmr != msTmr)
	{
		n_key_read_tmr = msTmr;
	}
	else
		return 0;
	{
		if(KEY_RD_DOCK() == 1)//按键被按下    回充
		{
			//log_printf("\t\t--1\r\n");
            		////log_printf("[PKT0]%d,%d\r\n",KEY_RD_WIFI(),n_key_pressed_ct[KEY_WIFI_NO]);
		    keystrokes++;

			if(n_key_pressed_ct[KEY_DOCK_NO] < 0xfff0)//n_key_pressed_ct[KEY_START_NO] ++;
			{
				n_key_pressed_ct[KEY_DOCK_NO] ++;
				/*if(n_key_pressed_ct[KEY_START_NO] >= KEY_LONG_PRESS_MAX)
				{
					if(n_key_pressed_ct[KEY_START_NO] == KEY_LONG_PRESS_MAX)//长按
					{
						n_key_pressed_ct[KEY_START_NO] = 0xfff0 + 1;//停止计数
						log_printf("[PKT]KEY_START_LONG_NO\r\n");
						return KEY_START_LONG_NO+ 1;
					}
					n_key_pressed_ct[KEY_START_NO] = 0xfff0 + 1;//停止计数
				}*/
			}
			else if(n_key_pressed_ct[KEY_DOCK_NO] == 0xffff)n_key_pressed_ct[KEY_DOCK_NO] = 0;
		}
		else
		{
			if(n_key_pressed_ct[KEY_DOCK_NO] < 0xfff0 && n_key_pressed_ct[KEY_DOCK_NO] > KEY_SHAKE_CT_MAX)
			{
				////log_printf("[PKT]KEY_WIFI_NO\r\n");
				n_key_pressed_ct[KEY_DOCK_NO] = 0xffff;
				log_printf("\r\n[KEY] Dock\r\n");
				return KEY_DOCK_NO + 1;
			}
			n_key_pressed_ct[KEY_DOCK_NO] = 0xffff;
		}

		if(KEY_RD_CLEAN() == 1)//按键被按下  导航清扫
		{
			//log_printf("\t\t--2\r\n");
			if(n_key_pressed_ct[KEY_CLEAN_NO] < 0xfff0)//n_key_pressed_ct[KEY_START_NO] ++;
			{
				n_key_pressed_ct[KEY_CLEAN_NO] ++;
				/*if(n_key_pressed_ct[KEY_WIFI_NO] >= KEY_LONG_PRESS_MAX)
				{
					if(n_key_pressed_ct[KEY_START_NO] == KEY_LONG_PRESS_MAX)//长按
					{
						n_key_pressed_ct[KEY_START_NO] = 0xfff0 + 1;//停止计数
						log_printf("[PKT]KEY_START_LONG_NO\r\n");
						return KEY_START_LONG_NO+ 1;
					}
					n_key_pressed_ct[KEY_START_NO] = 0xfff0 + 1;//停止计数
				}*/
			}
			else if(n_key_pressed_ct[KEY_CLEAN_NO] == 0xffff)n_key_pressed_ct[KEY_CLEAN_NO] = 0;
		}
		else
		{
			if(n_key_pressed_ct[KEY_CLEAN_NO] < 0xfff0 && n_key_pressed_ct[KEY_CLEAN_NO] > KEY_CLEAN_NO)
			{
				////log_printf("[PKT]KEY_DOCK_NO\r\n");
				n_key_pressed_ct[KEY_CLEAN_NO] = 0xffff;
				log_printf("\r\n[KEY] Clean\r\n");
				return KEY_CLEAN_NO + 1;
			}
			n_key_pressed_ct[KEY_CLEAN_NO] = 0xffff;
		}

		if(KEY_RD_WIFI() == 1)//按键被按下    调风机
		{
            		//log_printf("[PKT0]KEY_WIFI_NO=%d,%d\r\n",KEY_RD_WIFI(),n_key_pressed_ct[KEY_WIFI_NO]);
			if(n_key_pressed_ct[KEY_WIFI_NO] < 0xfff0)
			{
				n_key_pressed_ct[KEY_WIFI_NO] ++;
				LED2_ON();
				if(n_key_pressed_ct[KEY_WIFI_NO] >= KEY_LONG_PRESS_MAX)
				{
					if(n_key_pressed_ct[KEY_WIFI_NO] == KEY_LONG_PRESS_MAX)//长按
					{
						n_key_pressed_ct[KEY_WIFI_NO] = 0xfff0 + 1;//停止计数
						log_printf("\r\n[KEY]Wi-Fi long\r\n");
						return KEY_WIFI_L_NO+ 1;
					}
					n_key_pressed_ct[KEY_WIFI_NO] = 0xfff0 + 1;//停止计数
				}
			}
			else if(n_key_pressed_ct[KEY_WIFI_NO] == 0xffff)
			{
				////log_printf("[PKT0]zero\r\n");
				n_key_pressed_ct[KEY_WIFI_NO] = 0;
			}

		}
		else
		{
			if(n_key_pressed_ct[KEY_WIFI_NO] < 0xfff0 && n_key_pressed_ct[KEY_WIFI_NO] > KEY_SHAKE_CT_MAX)
			{
				//LED2_OFF();
				n_key_pressed_ct[KEY_WIFI_NO] = 0xffff;
				log_printf("\r\n[KEY] Wi-Fi\r\n");
				return KEY_WIFI_NO + 1;
			}
			n_key_pressed_ct[KEY_WIFI_NO] = 0xffff;
		}

	 	if(KEY_RD_SPOT() == 1)//按键被按下    定点清扫
		{
			//log_printf("\t\t--1\r\n");
			if(n_key_pressed_ct[KEY_SPOT_NO] < 0xfff0)
			{
				n_key_pressed_ct[KEY_SPOT_NO] ++;

			}
			else if(n_key_pressed_ct[KEY_SPOT_NO] == 0xffff)n_key_pressed_ct[KEY_SPOT_NO] = 0;
		}
		else
		{
			if(n_key_pressed_ct[KEY_SPOT_NO] < 0xfff0 && n_key_pressed_ct[KEY_SPOT_NO] > KEY_SHAKE_CT_MAX)
			{
				n_key_pressed_ct[KEY_SPOT_NO] = 0xffff;
				log_printf("\r\n[KEY] Spot\r\n");
				return KEY_SPOT_NO + 1;
			}
			n_key_pressed_ct[KEY_SPOT_NO] = 0xffff;
		}
	}
	//return res;
	return 0;
}
/**********************************************************************
* Function Name  : KEY_Read
* Description    : None
* Input          : None
* Output         : None
* Return         : None
* Attention      : None
**********************************************************************/
uint8_t proc_key_task(uint8_t sys_state)
{
	uint8_t key_value;
//	static unsigned char fan_level = 0;
	static unsigned char fan_number = 0;
	//static unsigned char fan_State = 0;
	key_value = key_read();
	if(key_value --)	//先减一次
	{
		if(READ_PSW_DET())
		{
			ny3p_play(VOICE_START_CHARGE);
		}
		else
		{
			log_printf("[PKT]key:%d\r\n",key_value);
			if(sys_state == SYS_TEST_BD)// && key_value != KEY_START_LONG_NO)
			{
				ny3p_play(VOICE_DIDI);
				log_printf("sys_state == SYS_TEST_BD\r\n");
				if(KEY_DOCK_NO == key_value)
					r_key_err &= 0xFE;
				if(KEY_CLEAN_NO == key_value)
					r_key_err &= 0xFD;
				return 0;
			}
#if 1		///因为不能调试WIFI，所以先拿掉
			if(sys->power_on == 0/* && key_value != KEY_DOCK_NO*/)
			{
				log_printf("sys->power_on == 0 && key_value != KEY_POWER_NO\r\n");
				sys->power_on = 1;
				//return 0;
			}
#endif

			ny3p_play(VOICE_DIDI);
			log_printf("all_key_led_off(14)\r\n");
			//all_key_led_off();

			sSleepTimeMin = 0;
			switch(key_value)
			{
				case     KEY_DOCK_NO:          //回充
				   sys->power_on = 1;
					sys->t_beep_on = 0;
					if(sys->sState == SYS_CHARGE)	//充电黑屏模式下,强制屏幕开机亮起,充电下没有关机状态
					{
						dp_led_light_contrl(0,sys->sState,0,sys->charge_sta,sys->prs,sys->work_errcode,sys->wifi_state);
					}
					/*
					if(sys->sState == SYS_NAVIGAT)
					{
						log_printf("clean now,please stop fisrt\r\n");
						break;
					}
					*/
					if(sys->sState == SYS_DOCK || sys->sState == SYS_DOCK_NEAR)
					{
                       // LED2_OFF();
                         LED4_OFF();
						log_printf("key charge1,%d..\r\n",sys->sState);
						init_sys_sta(SYS_IDLE);
						//init_sys_status(sys->sState,SYS_FOCUS);
						//STOP_ALL_MOTOR();
						break;
					}else if(sys->sState == SYS_CHARGE)
					{
						log_printf("key charge2,%d..\r\n",sys->sState);
						break;
					}else
					{
						all_key_led_off();
						sys->gey_key_clear_err = 1;
					    // LED2_ON();
					      LED4_ON();
					     log_printf("key charge3,%d..\r\n",sys->sState);

						init_sys_sta(SYS_DOCK);
						//SET_DST_MOTER(0);

					}
					break;

				 case    KEY_CLEAN_NO:      //导航清扫
					 if(sys->sState == SYS_CHARGE)	 //充电黑屏模式下,强制屏幕开机亮起,充电下没有关机状态
					 {
						 dp_led_light_contrl(0,sys->sState,0,sys->charge_sta,sys->prs,sys->work_errcode,sys->wifi_state);
					 }

				       sys->t_beep_on = 0;
				       sys->power_on = 1;
					if(sys->power_on == 0)
					{
						sys->power_on = 1;
						ny3p_play(VOICE_SELECT_M);
						init_sys_sta(SYS_IDLE);
						log_printf("work_errcode:%04\r\n ",sys->work_errcode );
					}
					else
					{
						if(EXTERAL_AC_DETECT())
						{
							log_printf("EXTERAL_AC_DETECT,out\r\n");
							return 0;
						}
						/*
						if(sys->sState ==  SYS_DOCK)
						{
							log_printf("find dock now,please stop dock first\r\n");
							break;
						}
						*/
						motor_run(GO_STOP,0,0,0);
						if(sys->sState == SYS_NAVIGAT  )
						 {
                            log_printf("key4 %d..\r\n",sys->sState);
                             all_key_led_off();
                           //  LED1_OFF();
							sys->sState = SYS_IDLE;
						//	if(sys->spot_flag == 0)
							{
							sys->t_halt_on=1;
							sys->t_flash_halt = 600000;		//10分钟
							}
                            LED3_OFF();
							init_sys_sta(SYS_IDLE);
							STOP_ALL_MOTOR();
							//delay_sensor(2000);
						 }
						 else //if(sys->sState == SYS_IDLE || sys->sState == SYS_CHARGE)
						 {
						 	 all_key_led_off();
						    log_printf("key0 %d..\r\n",sys->sState);
						    //LED1_ON();
						    LED3_ON();
						    sys->gey_key_clear_err = 1;
							init_sys_sta(SYS_NAVIGAT);
							//ny3p_play(VOICE_M_NAVI);
							delay_ms(500);
						 }

					}

					break;
                 case    KEY_SPOT_NO:       //定点清扫
#if 0
					if(AP_STATE < mcu_get_wifi_work_state())
					{
						log_printf("\r\n[wi-fi]get time!!!!\r\n");
                 		mcu_get_system_time();
                 	}
#endif
        #if 1
						sys->t_beep_on = 0;
						if(sys->sState == SYS_CHARGE)	//充电黑屏模式下,强制屏幕开机亮起,充电下没有关机状态
						{
							dp_led_light_contrl(0,sys->sState,0,sys->charge_sta,sys->prs,sys->work_errcode,sys->wifi_state);
						}
					     LED3_OFF();
					     LED4_OFF();
					     log_printf("--------------------------\r\n");
					     sys->power_on = 1;
					     /*
						if (sys->spot_flag++ >=1)
                        {
							sys->spot_flag=0;
							log_printf("key spot_flag 11 %d,%d..\r\n",sys->sState,sys->spot_flag);
					    }
					    */
						if(sys->sState != SYS_FOCUS)
                        {

					   	 	log_printf("key spot_flag 12 %d,%d..\r\n",sys->sState,sys->spot_flag);
						    // init_focus_task();
						    // sys->sState  = SYS_FOCUS;
						  	//sys->sState_auxi = 0;
					        //proc_focus_task();
						    // sys->power_go = 1;
						    //ny3p_play(VOICE_M_CLN_HEART);

							//init_focus_task();
							//s->sState = SYS_FOCUS;

							//init_sys_status(SYS_FOCUS,SYS_IDLE);
                            //init_sys_status(SYS_FOCUS,sys->sState);
                            if(sys->sState == SYS_NAVIGAT)
                            {
                            	log_printf("navi stop...\r\n");
									motor_run(GO_STOP,0,0,0);
                           	 		delay_ms(200);
                           	 }
							if(sys->sState == SYS_CHARGE)//充电状态不允许集中清扫
                           	 {
								log_printf("charge...out\r\n");
								break;
                           	 }
							 //LED3_ON();
							 sys->gey_key_clear_err = 1;
						    LED1_ON();
                          	sys->sState = SYS_FOCUS;
                          	sys->spot_flag = 1;
                      		sys->f_work_mode = MWO_SPOT;
                      		log_printf("set f_work_mode:%d\r\n",sys->f_work_mode);
				            log_printf("key12  focus  %d..\r\n",sys->sState);
							//if(sys->sState != SYS_IDLE)
							//sys->sState = SYS_FOCUS;
							break;
				        }else
                       {
				       // if (sys->sState  != SYS_FOCUS)
				         {
                         		sys->spot_flag  = 0;
                         		 LED1_OFF();
                          		sys->sState  = SYS_IDLE;
								log_printf("key spot_flag 13 %d,%d..\r\n",sys->sState,sys->spot_flag);
								if(sys->full2go)
									sys->f_work_mode = MWO_FULL2GO;
								else
								sys->f_work_mode = sys->work_mod;		//集中清扫模式 按键恢复
								log_printf("set f_work_mode:%d\r\n",sys->f_work_mode);
				         }
				       }
				    break;
          #endif
				 case   KEY_WIFI_NO:         //Wi-Fi开关
					 {
					 	sys->power_on = 1;
					 	if(SET == sys->wifi_switch)
					 	{
							sys->wifi_switch = RESET;
					 	}
					 	else
					 	{
							sys->wifi_switch = SET;
					 	}
					 	mcu_sync_wifi_net_state();
					 	log_printf("wifi_set_apmode()\r\n");
						wifi_set_apmode();
						  if(sys->sState == SYS_NAVIGAT)
                            {
                                LED3_ON();
                       	 		delay_ms(200);
                       	       log_printf("LEDmode()\r\n");
							}
//					 	mcu_sync_wifi_net_state();
					 	/*
						uint8_t senddata[8] = {0x55, 0xAA, 0x03, 0x05, 0x00, 0x01, 0x00, 0x08};
						LED2_ON();
						delay_ms(1000);
						usart3_write(senddata,8);
						delay_ms(100);
						LED2_OFF();
						*/
					}
					break;
				 case   KEY_WIFI_L_NO:         //重置WiFi
					 {
						if(sys->sState == SYS_CHARGE)	//充电黑屏模式下,强制屏幕开机亮起,充电下没有关机状态
						{
							dp_led_light_contrl(0,sys->sState,0,sys->charge_sta,sys->prs,sys->work_errcode,sys->wifi_state);
						}
					 	if(0 == cfg->wifi_config_able)
					 	{
					 		/*
							if(mcu_get_wifi_work_state() == WIFI_CONN_CLOUD)
							{
								sys->wifi_config_state = SMART_CONFIG;
							}
							else if(mcu_get_wifi_work_state() == SMART_CONFIG_STATE)
							{
								sys->wifi_config_state = AP_CONFIG;
							}
							else if(mcu_get_wifi_work_state() == AP_STATE)
							{
								sys->wifi_config_state = SMART_CONFIG;
							}
							else
							{
								sys->wifi_config_state = SMART_CONFIG;
							}
							*/
							sys->wifi_config_state = SMART_CONFIG;

							log_printf("[WIFI]wifi_mode_state:%d, set:%d,wifistate:%d\r\n",mcu_get_wifi_work_state(),sys->wifi_config_state,sys->wifi_state);
							uint8_t n_wifi_retry;
							msWifiTmr = 0;
							sys->wifistatechange_tmr = msWifiTmr;
							log_printf("cofig_start_tmr:%d\r\n",msWifiTmr-sys->wifistatechange_tmr);
							for(n_wifi_retry = 0;n_wifi_retry < 100;n_wifi_retry++)
							{
								mcu_set_wifi_mode(sys->wifi_config_state);
								wifi_uart_service();
								if(mcu_get_wifimode_flag() == SET_WIFICONFIG_SUCCESS)
									break;
							}
							log_printf("n_wifi_retry:%d\r\n",n_wifi_retry);
							cfg->wifi_config_able = 1;
							save_cfg();
					 	}
					 	else
					 	{
							mcu_reset_wifi_module();
							cfg->wifi_config_able = 0;
							save_cfg();
					 	}
					}
					break;
				default:
					break;
			}
			key_value ++;
		}
	}
	return key_value;
}

//#if !ICLEAN_HW_FEGOOLCD
#define PROC_LED_MODE_BAT_OFF	0
#define PROC_LED_MODE_BAT_ON	1
#define PROC_LED_MODE_BAT_FLASH	2


uint8_t led_wifi_mode = 0;

static uint32_t n_led_task_tmr = 0;



void led_off_bright(void)
{
//	LED1_OFF();
//	LED2_OFF();
}
void led_long_bright(void)
{
	//LED1_ON();
	//LED2_OFF();
}
void led_blue_long_bright(void)
{
	//LED1_ON();
	//LED2_ON();
}

void led_alternate_scintillation(void)
{
	static uint8_t led_state = 0;

	if ( msledTmr < (n_led_task_tmr + 250))
	{
		if(led_state == 0)
		{
			//LED1_ON();
			//LED2_OFF();
			led_state = 1;
		}
	}
	else if( msledTmr < (n_led_task_tmr + 500))
	{
		if(led_state == 1)
		{
			//LED1_OFF();
			//LED2_ON();
			led_state = 2;
		}
	}
	else
	{
		n_led_task_tmr = msledTmr;
		led_state = 0;
	}
}
void led_blue_twinkle(void)
{
	static uint8_t led_state = 0;

	if ( msledTmr < (n_led_task_tmr + 750))
	{
		if(led_state == 0)
		{
			//LED1_ON();
			//LED2_ON();
			led_state = 1;
		}
	}
	else if( msledTmr < (n_led_task_tmr + 1000))
	{
		if(led_state == 1)
		{
			//LED1_ON();
			//LED2_OFF();
			led_state = 2;
		}
	}
	else
	{
		n_led_task_tmr = msledTmr;
		led_state = 0;
	}
}
void led_blue_fast_twinkle(void)
{
	static uint8_t led_state = 0;

	if ( msledTmr < (n_led_task_tmr + 100))
	{
		if(led_state == 0)
		{
			//LED1_ON();
			//LED2_ON();
			led_state = 1;
		}
	}
	else if( msledTmr < (n_led_task_tmr + 500))
	{
		if(led_state == 1)
		{
			//LED1_ON();
			//LED2_OFF();
			led_state = 2;
		}
	}
	else
	{
		n_led_task_tmr = msledTmr;
		led_state = 0;
	}
}
void led_blue_slow_twinkle(void)
{
	static uint8_t led_state = 0;

	if ( msledTmr < (n_led_task_tmr + 100))
	{
		if(led_state == 0)
		{
			//LED1_ON();
			//LED2_ON();
			led_state = 1;
		}
	}
	else if( msledTmr < (n_led_task_tmr + 1500))
	{
		if(led_state == 1)
		{
			//LED1_ON();
			//LED2_OFF();
			led_state = 2;
		}
	}
	else
	{
		n_led_task_tmr = msledTmr;
		led_state = 0;
	}
}

//红蓝两个灯闪烁函数
void led_twinkle_app(uint8_t sta)
{
	static uint8_t old_sta = 0;

	if(old_sta != sta)
	{
		if(sys->power_on != 0)
		{
			led_off_bright();
			led_long_bright();
			log_printf("led_run_linght,led_state:%d\r\n",sta);
		}
		old_sta = sta;
	}

	switch(sta)
	{
		case LED_RUN_BLU_STATE_OFF:
			led_off_bright();
			break;
		case LED_RUN_BLU_STATE_TWINKLE:
			led_alternate_scintillation();
			break;
		case LED_RUN_STATE_LONG_BRIGHT:
			led_long_bright();
			break;
		case LED_BLUE_STATE_TWINKLE:
			led_blue_twinkle();
			break;
		case LED_BLUE_STATE_FAST_TWINKLE:
			led_blue_fast_twinkle();
			break;
		case LED_BLUE_STATE_SLOW_TWINKLE:

			led_blue_slow_twinkle();
			break;
		case LED_BLUE_STATE_LONG_BRIGHT:
			led_blue_long_bright();
			break;
		default:
		break;
	}
}

uint8_t led_state_out(uint8_t sys_state, uint8_t v_bat_percent)
{
	uint8_t sta;

	if(sys->work_errcode ==  MWERR_NONE)				//是否有错误
	{
		if(sys->sState == SYS_CHARGE)
		{
			if(sys->volt<LIBAT_CHARGEVOLT_MAX)
				sta = LED_BLUE_STATE_LONG_BRIGHT;		//充电
			else
				sta = LED_BLUE_STATE_OFF;				//充满了
		}
		else
		{
			if(v_bat_percent< 30 || sys->sState == SYS_DOCK || sys->volt < 14000)		//小于30%欠电压告警
			{
				sta = LED_BLUE_STATE_TWINKLE;
			}
			else
			{
				//switch(n_wifi_work_state)
				//{
				//	case OK_CONFIG:
				//	case ERR_CONFIG:
				//	break;
				//	case AP_CONFIG:
				//		sta = LED_BLUE_STATE_SLOW_TWINKLE;
				//	break;
				//	case SMART_CONFIG:
				//		sta = LED_BLUE_STATE_FAST_TWINKLE;
				//	break;
				//}
			}
		}
	}
	else		//有错误1S运行,蓝灯交替闪烁
		sta = LED_RUN_BLU_STATE_TWINKLE;
	return sta;
}

//欠电压闪蓝灯
//有错误,蓝灯运行灯交替闪烁
//工作:常亮
//v_bat_percent:电池电量百分比(取值0-100)
void proc_led_task(uint8_t sys_state,uint8_t v_bat_percent)
{
//	static uint8_t n_task_delay_100ms = 0;
//	static uint8_t n_led_tmr[3] = {0};
	static uint8_t pow_sta = 0;
	uint8_t sta = 0;

	if(sys->power_on == 0 && pow_sta == 0)
	{
		led_twinkle_app(LED_RUN_BLU_STATE_OFF);
		pow_sta = 1;
	}
	else if(sys->power_on == 1 && pow_sta == 1)
	{
		led_twinkle_app(LED_RUN_STATE_LONG_BRIGHT);
		pow_sta = 0;
	}
	if(sys->power_on == 1)
	{
		sta = led_state_out(sys_state,v_bat_percent);
		led_twinkle_app(sta);
	}
}

//wifi按键指示灯任务
void proc_wifiled_task(void)
{
	static uint32_t n_msledwifitmr = 0;
	static uint8_t n_4hz_tmr = 0;
	uint8_t led_onoff_status = 0;
	uint32_t n_mstmr;
	uint8_t wifi_state = mcu_get_wifi_work_state();

	uint8_t temp_flag = 0;
	static uint8_t n_led_en = 0;

	static uint8_t n_wifi_state;
	if(wifi_state != 0x05)
	{
		if(n_wifi_state != wifi_state)
			temp_flag = 1;
		n_wifi_state = wifi_state;
	}

	//if(wifi_state == 0xff)
	//	mcu_sync_wifi_net_state();
	//wifi_state = AP_STATE;
	switch(n_wifi_state)
	{
		case AP_STATE:
			led_onoff_status = 0x55;
			break;
		case WIFI_NOT_CONNECTED:
			led_onoff_status = 0;
			break;
		case WIFI_CONN_ROUTER:
			led_onoff_status = 0x0f;
			break;
		case WIFI_CONN_CLOUD:
		case WIFI_CONN_READY:
			led_onoff_status = 0xff;
			break;
		default:
			led_onoff_status = 0;
			break;
	}

	if(temp_flag == 1 && led_onoff_status == 0)
	{
		log_printf("[wifiled]:%02X(%02X)\r\n",led_onoff_status,n_wifi_state);
	}
	if(n_led_en != sys->wifi_led_enable)
	{
		log_printf("[wifileden]:%d(%d)\r\n",sys->wifi_led_enable,n_led_en );
		n_led_en = sys->wifi_led_enable;
	}
	n_mstmr = msledwifitmr - n_msledwifitmr;
	if(sys->wifi_led_enable == 0)
	{
		LED2_OFF();
		return ;
	}
	if(n_mstmr > 125)
	{
		n_msledwifitmr = msledwifitmr;
		//log_printf("wifistate:%d-%02X\r\n",wifi_state,led_onoff_status);
		//if(sys->power_led_on == 0)
		if(sys->power_on )
		{
			if((led_onoff_status & (1 << n_4hz_tmr)) != 0)
			{
				LED2_ON();
			//	log_printf("LED2_ON\r\n");
			}
			else
			{
				LED2_OFF();
				//log_printf("off\r\n");
			}
		}
		n_4hz_tmr ++;
		if(n_4hz_tmr > 7)n_4hz_tmr = 0;
	}

}

void ramote_pair_mode(void)
{
#if 1
//	uint8_t i=0;
//	uint8_t key_value = 0;
	uint16_t n_key_pressed_ct = 0xffff;
	while(1)
	{
		if(KEY_RD_CLEAN())
		{
			//log_printf("\t\t--2\r\n");
				if(n_key_pressed_ct < 0xfff0)//n_key_pressed_ct[KEY_START_NO] ++;
			{
					n_key_pressed_ct ++;
				if(n_key_pressed_ct%20 == 0 && n_key_pressed_ct>=20)
					log_printf("ct:%d\r\n",n_key_pressed_ct);
					LED3_ON();
				if(n_key_pressed_ct >= (KEY_SHAKE_CT_MAX))
					{
					n_key_pressed_ct = KEY_SHAKE_CT_MAX;//停止计数
						if(sys->pair_sta == REMOTE_PAIR_NU)
						{
							log_printf("[PKT]PRESS_KEY_PAIR_REMOTE\r\n");
							sys->pair_sta = REMOTE_PAIR_ING;
							dp_tx_data(DISP_CMD_PAIR_REMOTE,LEN_CMD_PAIR_REMOTE,&sys->pair_sta);
							while(1)
							{
								dp_commrxtask();	//匹配模式不下发其他按键
								if(sys->pair_sta == REMOTE_PAIR_OK)	//匹配成功叫一下就好
								{
									ny3p_play(VOICE_SELECT_M);
									log_printf("REMOTE_PAIR_OK\r\n");
									break;
						}
								if(!KEY_RD_CLEAN())					//按键提前抬起了
								{
									log_printf("left key before pair remote\r\n");
									break;
								}
								delay(10);
							}
						}
					}
			}
				else if(n_key_pressed_ct == 0xffff)
					n_key_pressed_ct = 0;
		}
		else
		{
			LED3_OFF();
			if(n_key_pressed_ct < 0xfff0 && n_key_pressed_ct >= (KEY_SHAKE_CT_MAX))
			{
				////log_printf("[PKT]KEY_DOCK_NO\r\n");
				n_key_pressed_ct = 0xffff;
				//无论是否匹配成功都要退出匹配模式
				//return KEY_CLEAN_NO + 1;
				{
					sys->pair_sta = REMOTE_PAIR_NU;
					dp_tx_data(DISP_CMD_PAIR_REMOTE,LEN_CMD_PAIR_REMOTE,&sys->pair_sta);
						break;
				}
			}
			else if(n_key_pressed_ct<(KEY_LONG_PRESS_MAX))	//无效按键
				break;
			if(n_key_pressed_ct == 0xffff)
			{
				log_printf("no need pair remote\r\n");
				break;
			}
		}
		delay(10);
	}
#endif
}

///////////////////////////////红外遥控代码段///////////////////////////////////////////////

#if 0
#if WIFIAPP_EN
extern u8 wifi_set_connectmode;
#endif
#if ICLEAN_IRCTRL_TYPE == 2
static uint8_t irda_timeset_flag = 0xff;
#endif
//红外遥控接收任务
uint8_t proc_irctrl_task(uint8_t sys_state)
{
//	uint32_t n_IrCode;
	uint8_t n_Code;
	uint8_t n_beep = 1;

#if 1
	n_Code = get_irda_key();
	if(n_Code == IRDA_CODE_INVALID)return n_Code;
	log_printf("[IKP]:Code:%02x\r\n",n_Code);
	//return IRDA_CODE_INVALID;
#else
	n_IrCode = get_irda_32bits();
	if(!n_IrCode)return;

	//ny3p_play(VOICE_DIDI);

	n_Code = (uint8_t)(n_IrCode>>24);
	PRINTF("[IKP]:%08x\tCode:%02x\r\n",n_IrCode,n_Code);
#endif
#if ICLEAN_IRCTRL_TYPE == 2
	if(n_Code == IRDA_CODE_SETTIM)
	{
	}
#endif

	//if(sys_state == SYS_POWEROFF && n_Code != IRDA_CODE_POWER)
	if(sys->power_on == 0 && n_Code != IRDA_CODE_POWER)
	{
		return IRDA_CODE_INVALID;
	}
	/*
	if(n_Code >= IRDA_CODE_AUTO && n_Code <= IRDA_CODE_WIFI)
	{
		ny3p_play(VOICE_DIDI);
	}
*/
	sSleepTimeMin = 0;
	switch(n_Code)
	{
		//uint32_t n_time;
		//uint8_t n_schweek,i;
		//uint8_t *n_ptrByte;
		//uint8_t n_schhour,n_schminute;
 #if (defined IRDA_CODE_OK) && IRDA_CODE_OK != IRDA_CODE_WALLWALK && IRDA_CODE_OK != IRDA_CODE_AUTO
		case IRDA_CODE_OK:
	#if ICLEAN_HW_FEGOOLCD
		  if(sys->lcd_edit_lock)//如果屏幕正在编辑，就不能做开始/停止动作
		  	break;
	#endif
 #endif
		case IRDA_CODE_AUTO:
			if(get_irda_rep())break;
			//if((//sys_state == SYS_IDLE||
			//	(sys_state == SYS_CHARGE && DOCK_DETECT())) && sys_state != SYS_AUTO)//在空闲状态或在充电座上,允许触发自动打扫
			if(sys_state == SYS_NAVIGAT|| sys_state == SYS_FOCUS|| sys_state == SYS_NEAR || sys_state == SYS_RANDOM
					|| sys_state == SYS_DOCK)
			{
				n_beep = 0;
				ny3p_play(VOICE_M_STOP);
				delay_ms(700);
				init_sys_sta(SYS_IDLE);
				//log_printf("zzzz\r\n");
			}
			else if((//sys_state == SYS_IDLE||
				(!EXTERAL_AC_DETECT() ||  DOCK_DETECT())) && sys_state != SYS_NAVIGAT)//在空闲状态或在充电座上,允许触发自动打扫
			{
				n_beep = 0;
				//ny3p_play(VOICE_M_START);
				init_sys_sta(SYS_NAVIGAT);
			}
			break;
		case IRDA_CODE_DOCK:
			if(get_irda_rep() || sys_state == SYS_DOCK)
			{
				log_printf("ir dock rep\r\n");
				break;
			}
			/*if(sys_state == SYS_DOCK)//正在回充时,停止工作
			{
				ny3p_play(VOICE_M_STOP);
				init_sys_sta(SYS_IDLE,0);
			}
			else */if(sys_state != SYS_DOCK && !EXTERAL_AC_DETECT())//在非充电模式下停止按键被触发
			{
				n_beep = 0;
				init_sys_sta(SYS_DOCK);
				//ny3p_play(VOICE_M_DOCK);
			}
			break;
#ifdef IRDA_CODE_NAVI//弓字形清扫模式
		case IRDA_CODE_NAVI:
			if(get_irda_rep())break;
			/*if(sys_state == SYS_DOCK)//正在回充时,停止工作
			{
				ny3p_play(VOICE_M_STOP);
				init_sys_sta(SYS_IDLE,0);
			}
			else */if(sys_state != SYS_NAVIGAT && !EXTERAL_AC_DETECT())//在非充电模式下停止按键被触发
			{
				init_sys_sta(SYS_NAVIGAT);
				//ny3p_play(VOICE_M_DOCK);
			}
			break;
#endif
#ifdef IRDA_CODE_WALLWALK
		case IRDA_CODE_WALLWALK:

			if(get_irda_rep())break;
			//if(//sys_state == SYS_IDLE||
			//	(sys_state == SYS_CHARGE && DOCK_DETECT()))//在空闲状态或在充电座上,允许触发自动打扫
			if((!EXTERAL_AC_DETECT() ||  DOCK_DETECT()) && sys_state != SYS_RANDOM)//SYS_NEAR
			{
				n_beep = 0;
				//ny3p_play(VOICE_M_NEAR);
				init_sys_sta(SYS_RANDOM);
				//delay_ms(1000);
			}
			/*else if(!EXTERAL_AC_DETECT())//在非充电模式下"停止"按键被触发
			{
				ny3p_play(VOICE_M_STOP);
				init_sys_sta(SYS_IDLE,0);
			}*/
			break;
#endif
		case IRDA_CODE_SPOT:
			if(get_irda_rep())break;
			//if(//sys_state == SYS_IDLE||
			//	(sys_state == SYS_CHARGE && DOCK_DETECT()))//在空闲状态或在充电座上,允许触发自动打扫
			if(!EXTERAL_AC_DETECT() ||  DOCK_DETECT())
			{
				n_beep = 0;
				ny3p_play(VOICE_M_CLN_HEART);
				init_sys_sta(SYS_FOCUS);
			}
			/*else if(!EXTERAL_AC_DETECT())//在非充电模式下"停止"按键被触发
			{
				ny3p_play(VOICE_M_STOP);
				init_sys_sta(SYS_IDLE,0);
			}*/
			break;
#if 1//ICLEAN_IRCTRL_TYPE != 2
		case IRDA_CODE_MUTE:
			if(get_irda_rep())break;
			n_beep = 0;
		#ifdef NY3P_POWER_PIN
			if(NY3P_POWER_PORT->ODR & NY3P_POWER_PIN)
		#else
			//if(sys->mute == 0)
			if(sys->voice == 0)
		#endif
			{
		#ifdef NY3P_POWER_PIN
				NY3P_POWER_PORT->ODR &= ~NY3P_POWER_PIN;
		//#else
				//sys->mute = 1;
		//		sys->voice = 1;
		#endif
				log_printf("[IKP]:Voice Open\r\n");
				//delay_ms(500);
				voice_output(VOICE_DRIP);
				//vTaskDelay(TASK_DELAY_MS(500));
				delay_ms(500);
		#ifndef NY3P_POWER_PIN
				sys->voice = 1;
		#endif
			}
			else
			{
		#ifndef NY3P_POWER_PIN
				sys->voice = 0;
		#endif
				voice_output(VOICE_DIDI);
				log_printf("[IKP]:Voice Off\r\n");
				delay_ms(500);
		#ifdef NY3P_POWER_PIN
				NY3P_POWER_PORT->ODR |= NY3P_POWER_PIN;
		//#else
				//sys->mute = 0;
		//		sys->voice = 0;
		#endif
			}
			break;
#endif
		case IRDA_CODE_POWER:
			n_beep = 0;
			//log_printf("fjlsdada\r\n");
			if(get_irda_rep())break;
			/*if(//sys_state == SYS_IDLE||
				(sys_state == SYS_CHARGE && DOCK_DETECT()))//在空闲状态或在充电座上,允许触发自动打扫
			{
				//ny3p_play(VOICE_M_START);
				init_sys_sta(SYS_AUTO,0);
			}
			else if(!EXTERAL_AC_DETECT() && (sys_state != SYS_IDLE))//在非充电模式下"停止"按键被触发
			{
				ny3p_play(VOICE_M_STOP);
				init_sys_sta(SYS_IDLE,0);
			}*/
			//if(sys_state == SYS_POWEROFF)
			if(sys->power_on == 0)
			{
				log_printf("wake up\r\n");
				sys->power_on = 1;
				ny3p_play(VOICE_SELECT_M);
				init_sys_sta(SYS_IDLE);
				sys->t_idle = T_IDLE_IR;
			}
			else
			{
				if(sys_state == SYS_IDLE)//空闲状态下关机
				{
					ny3p_play(VOICE_M_SLEEP);
					sys->power_on = 0;
					sys->t_idle = T_IDLE_IR + 1;
					//init_sys_sta(SYS_POWEROFF);
				}
				else if(sys_state != SYS_CHARGE)
				{
					ny3p_play(VOICE_M_STOP);
					sys->power_on = 1;
					init_sys_sta(SYS_IDLE);
				}
			}
			break;
		case IRDA_CODE_UP:
		  if(sys->sState == SYS_IDLE)
		  {
			//sys->manState = MAN_FORWARD;
			//sys->mState = GO_FORWARD;
			//navigat->walk_dis = 0;
			motor_single_run(GO_FORWARD);
			sys->man_worktype = 0;
			//mstimeout(&sys->man_typetimer,0);
			//MOTOR_CTRL(NORM_SIDE_PWM_L,NORM_SIDE_PWM_R,NORM_MID_PWM,NORM_DUST_PWM);
			log_printf("[IKP]MAN_FORWARD,%d\r\n",sys->man_typetimer);
		  }
		  else
			  sys->manState = MAN_STOP;
		  n_beep = 0;
		  break;
		case IRDA_CODE_LEFT:
		  if(sys->sState == SYS_IDLE)
		  {
			motor_single_run(GO_LEFT);
			sys->man_worktype = 0;
			log_printf("[IKP]MAN_LEFT,%d\r\n",sys->man_typetimer);
		  }
		  else
			  sys->manState = MAN_STOP;
		  n_beep = 0;
		  break;
		case IRDA_CODE_RIGHT:
		  if(sys->sState == SYS_IDLE)
		  {
			motor_single_run(GO_RIGTH);
			sys->man_worktype = 0;
			log_printf("[IKP]MAN_LEFT,%d\r\n",sys->man_typetimer);
		  }
		  else
			  sys->manState = MAN_STOP;
		  n_beep = 0;
		  break;
		case IRDA_CODE_DOWN:
		  if(sys->sState == SYS_IDLE)
		  {
			motor_single_run(GO_BACK);
			sys->man_worktype = 0;
			log_printf("[IKP]MAN_RIGHT,%d\r\n",sys->man_typetimer);
		  }
		  else
			  sys->manState = MAN_STOP;
		  n_beep = 0;
		  break;
		case IRDA_CODE_SCHDL://这里用来做WIFI配网
#if WIFIAPP_EN
			if(get_irda_rep())break;
				{
					uint8_t n_wifi_retry = 0;
					uint8_t n_wifi_work_state = SMART_CONFIG;
					//mcu_set_wifi_mode(SMART_CONFIG);
					n_beep = 0;
					ny3p_play(VOICE_DIDI);
					if(mcu_get_wifi_work_state() == SMART_CONFIG_STATE)
					{
						n_wifi_work_state = AP_CONFIG;
					}
					PRINTF("[WIFI]wifi_mode_state:%d, set:%d\r\n",mcu_get_wifi_work_state(),n_wifi_work_state);
					for(n_wifi_retry = 0;n_wifi_retry < 5;n_wifi_retry++)
					{
						mcu_set_wifi_mode(n_wifi_work_state);
						delay_ms(100);
						if(mcu_get_wifimode_flag() == SET_WIFICONFIG_SUCCESS)
							break;
					}
					//if(n_wifi_retry >= 5)
					//{
					//	log_printf("[WIFI]mode failed!%d, set:%d\r\n",mcu_get_wifi_work_state(),n_wifi_work_state);
					//}
				}
#endif
#if 0
				{
					//sys->
					uint8_t n_modified = 0;
//					uint16_t n_old_value = sys->dust_pwm_value;
					n_beep = 0;
					if(get_irda_rep())break;
					if(sys->dust_pwm_value == NORM_DUST_PWM)
					{
						sys->dust_pwm_value = FORCE_DUST_PWM;
						ny3p_play(VOICE_M_SPEED_MODE);
						n_modified = 1;
					}
					else// if(sys->dust_pwm_value == FORCE_DUST_PWM)
					{
						sys->dust_pwm_value = NORM_DUST_PWM;
						ny3p_play(VOICE_DIDI);
						n_modified = 1;
					}
					if(n_modified && ((sys->sState ==SYS_NAVIGAT) || (sys->sState==SYS_FOCUS)
						|| (sys->sState==SYS_NEAR)|| (sys->sState==SYS_RANDOM)))//
					{
						SET_DST_MOTER(sys->dust_pwm_value);
					}
					else
					{
						//sys->dust_pwm_value = n_old_value;
						//ny3p_play(VOICE_DIDI);
					}
					log_printf("[dust_pwm_value]%d\r\n",sys->dust_pwm_value);
				}
#endif
				break;
		default:
#if ICLEAN_IRCTRL_TYPE != 2 && (defined IRDA_CODE_SCHDL)
			if(n_Code != IRDA_CODE_SCHDL)//因为预约按键在这里不处理,将交给UI任务,故此键未发出声音,所以在这里做例外
				n_beep = 0;
#endif
			break;
	}

	if(n_beep)
		ny3p_play(VOICE_DRIP);
	return n_Code;
}

#endif

#endif

