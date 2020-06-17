/**
  ******************************************************************************
  * @file 		main.c
  * @author  	micony
  * @version  V2.0.0
  * @date  		04/06/2014
#include <stm32f10x.h>
  * @brief  	初始化、主函数、有限状态机。
#include "RTE_Components.h"             // Component selection
#include "RTE_Components.h"             // Component selection
  ******************************************************************************
  * @copy
  *sd
  * 版权所有、禁止拷贝
  * 杭州艾豆智能科技有限公司(http://www.idleintel.com/)保留所有知识产权及软件著作权
  * 必须经过授权才能使用
  * 2A
  * <h2><center>&copy; COPYRIGHT 2009 idleintel</center></h2>
  */
#include "sys.h"

#define SW_VERSION		"HOOVER-2A_20200615_V2.2.5"

const	char copyright[]="No copy allowed.Copyright  http://www.idleintel.com/";
const	char copyright21[]="警告，此代码版权归杭州艾豆智能科技有限公司所有，未经授权任何人不可随意拷贝改动，请致电13757122544";

#define RSTBY_IWDG	0x20000000
#define RSTBY_SW	0x10000000
uint32_t StartupStatu;
uint8_t SysResetSrc;//?μí3?′???′

int main(void)
{
	//unsigned int test_delay = 0;
	int i;
	u8 irData_bak[6];
	u8 sta;
	StartupStatu = RCC->CSR;
	for(i = 0;i < 255;i ++);
	RCC->CSR |= 0x01000000;
	SysResetSrc = 0;
	if(StartupStatu & RSTBY_IWDG)
		SysResetSrc = 0x01;//?ì2é?′???′
	else if(StartupStatu & RSTBY_SW)
		SysResetSrc = 0x02;//?ì2é?′???′

	disable_irq();
	Periph_Init();	 				//IO口等初始化
	usart1_init();	 				//串口初始化

	if(SysResetSrc == 0x01)
		log_printf("[rst_info]IWDG\r\n");
	else if(SysResetSrc == 0x02)
		log_printf("[rst_info]SW\r\n");
	else
		log_printf("[rst_info]NORM\r\n");



	I2C1_GPIO_Init();				//前轮码盘I2C初始化
	init_cfg();						//配置文件初始化
	navigat_init(3);				//导航参数
	init_sys();						//系统初始化

	motor_timer_init();				//电机PWM初始化
	irda_timer_cfg();				//红外障碍探测时钟初始化
	ir_recv_ext_config();			//回冲接收
	SysTick_Init();					//系统时钟，1ms一次中断
	RTC_Init();
	adc1_init();					//ADC初始化，采用DMA

	all_key_led_off();
	AUTO_LAMP_OFF();
	sys->power_on = 1;

	timer5_config();				//定时器初始化


	usart3_init();					//WIFI串口初始化
	usart4_init();
	spi_master();
	timer1_init();					//滚刷 充电
	timer2_init();					//无源蜂鸣器

	sys->volt = 16500;				//给个初始的电压
	log_printf("\r\nrstrstrst: %d\r\n", StartupStatu);
    log_printf("MOTOR_POWER_OFF 2\r\n");
	MOTOR_POWER_OFF();				//关掉所有电机电源

	delay_ms(30);
	enable_irq();

	ny3p_init();					//声音

	copyright_www_idleintel_com("No copy allowed.Copyright idleintel,please tel:13757122544",
								 &kgyro,SPI1_ReadWriteByte,spi_cs,log_printf,delay);
	copyright_idleintel_no_one_is_allowed_to_copy(\
"警告，此代码版权归杭州艾豆智能科技有限公司所有，未经授权任何人不可随意拷贝改动，请致电13757122544",\
	"http://www.idleintel.com/");

	copyright_idleintel_illegal_copy_tell_13757122544();


	sys->sState = 0xff;//开机状态设为未知,防止湿拖模式下的语音播报与开机音乐播报冲突 add by wonton2004 20170118
	NOMORL_PWM  = 580;
	pd_gyro_int(GO_FORWARD_PWM);		//PID初始化
	sys->voice = 1;					//这里关掉声音，避免声音
	delay_ms_sensers(100);			//延迟并检查传感器
	sys->voice = 0;

	ramote_pair_mode();
#if ROOMA_GYROONLY
	struct tm aa;
	aa = Time_GetCalendarTime();
	log_printf("%02d:%02d:%02d\r\n",aa.tm_hour,aa.tm_min,aa.tm_sec);
	time_t tt = Time_GetUnixTime();
	dp_tx_data(DISP_CMD_SET_TIME,LEN_CMD_SET_TIME,(uint8_t *)&tt); 	//开机同步,wifi同步时间也要上报
	mstimeout(&sys->sync_loop,0);
	sys->sync_flg = 0;
	if(cfg->sn[0] == 0xff)//序列号未写入时,可确认PCB为未测试状态,此时将WIFI模块设置为配网模式,以辨别WIFI模块是否工作正常(不正常时工作状态为0xff)
	{
	}
#endif
	//ny3p_play(VOICE_DIDI);

	sys->sState =SYS_IDLE;
	i=0;

	sys->wifi_led_enable = 1;
	wifi_protocol_init();
	illegal_copy_tell_13757122544_gsend(0x03,cfg->gyro_ajust,cfg->gyro_ajust);
	log_printf("enter main loop\r\n");
	log_printf("MOTOR_POWER_OFF 3\r\n");
	MOTOR_POWER_OFF();

	sn_print(cfg->sn, 21);
	navigat->is_walk = 1;

	delay_ms(50);


	delay_ms_tm5(1000);
	I2C_ReadS_24C(0 ,(uint8_t *)sys,ERP_LEN);		//?eà′μ?ê±oò￡??è?áè?2?êy
	if(sys->m_area == 0xffff || sys->m_worktime == 0xffff || sys->t_area == 0xffffffff || sys->t_clean == 0xffffffff)
	{
		sys->m_area = 0;
		sys->m_worktime = 0;
		sys->t_area = 0;
		sys->t_clean = 0;
		I2C_WriteS_24C(0,(uint8_t *)sys, ERP_LEN);
		log_printf("new robot...reset user data!\r\n");
	}
	log_printf("read epr=%d,%d,(%d,%d)\r\n",sys->t_area,sys->t_clean,sys->m_area,sys->m_worktime);
#if 0
//while(1)
{
	log_printf("write...\r\n");
	uint8_t dat[100];
	//íù 0 μ?100μ?μ??·D′è? 0 μ?100 μ?êy?Y
	for(uint16_t i=0;i<100;i++)
	{
			dat[i] = i;

	}
	I2C_WriteS_24C(0,dat, 100);
	delay_ms(100);
	log_printf("read:\r\n");
	 //?áè?3?à′?￡?￡?￡

	I2C_ReadS_24C(0 ,dat,100);


		for(uint16_t i=0;i<100;i++)
	{
			log_printf("%d\r\n",dat[i]);

	}
}

#endif
	for(i=0;i<20;i++)
	{
		www_idleintel_com();
		delay_ms_tm5(10);
	}
	      //ny3p_play(VOICE_DIDI);
#if 0
#if DUST_BOX_FULL_CHECK
	dust_full_check_pro(0);			//2018-10-23 jzz  设备开机,直接获取尘满时间信息
#endif
#else
#if CONSUMABLES_CHECK_EN
	consumables_check_pro(0);
#endif
#endif
	sys->pwm = SYS_PWM;
	sta = sys->sState = SYS_IDLE;
	//sta = sys->sState = SYS_NAVIGAT;
#if POWER_ON_RUN_VER
		delay_ms_tm5(5000);
#endif

	MPU6500_Init();
	ny3p_play(VOICE_DIDI);

	reset_gyro(1);
	sta = SYS_IDLE;
	//sta = SYS_NAVIGAT;
	//LED5_OFF();
	/************************************************
	1.有限状态机，很多状态都是阻塞运行，
	2.每一个阻塞中都会调用 get_sensers(&sys->gSta); 检测传感器 和proc_uart_task();检测外部控制动作。
	4.get_sensers 和proc_uart_task 会置系统状态变量 sys->sState,该变量变成SYS_IDLE,所以的阻塞都会退出，回到最外层。
	5.get_sensers 为传感器函数，包括碰撞，红外，对地红外，里边都会置相应的变量，有的会把状态变成SYS_IDLE导致外部函数停机。
      碰撞置变量 sys->gSta,碰撞红外置变量 sys->g_sta[](sys->g_sta[0]左边，sys->g_sta[6]右边	)
      对地红外置变量sys->g_bottom[][]
    6.所有的控制都放在 proc_uart_task 函数中处理，包括串口命令，红外遥控，按键，wifi，
      控制仿照串口的控制即可。
	***************************************************************/
		//sys->work_errcode = 0;//初始化会有报警，暂时没有查从那里来的
#if 0	//debug

	MOTOR_POWER_ON();
	MOTOR_CTRL_SIDE(200,200);
#endif
	sys->dccheck_enable = ENABLE;
	motor_run(GO_STOP,0,0,0);
	sys->t_beep_on = 0;


	//GPIO_SetBits(PORT_IR_CTRL,PIN_IR_CTRL); //低电平，关灯
	//GPIO_SetBits(GPIOD,GPIO_Pin_14);	//低电平，关灯

 		//GPIO_ResetBits(PORT_IR_CTRL,PIN_IR_CTRL);	//低电平，关灯
		//GPIO_ResetBits(GPIOD,GPIO_Pin_14);	//低电平，关灯
#if POWER_ON_RUN_VER
		sys->sState = SYS_NAVIGAT;
#endif
	sys->wifi_sta = WIFI_STA_STANDBY;
	while(1)
	{
		if(sys->sState == SYS_DOCK && sta == SYS_CHARGE)
			sta = sys->sState;
		if(sys->sState != SYS_TEST_BD )
			get_sensers(&sys->gSta);			//传感器采样
		proc_uart_task();

		if(SET == sys->upgrade_mode)
		{
			upgrade_self_task();
		}

		if(sta != sys->sState)
		{
			init_sys_status(sys->sState,sta);
		}
		sta = sys->sState;



		switch(sys->sState)
		{
			case SYS_NAVIGAT:
				proc_navigat_task(sys->gSta);
				break;
			case SYS_FOCUS:
				motor_go_spot_task();
				break;
			case SYS_RANDOM:
				proc_random_task();
				break;
			case SYS_DOCK:
				proc_charge_task();
				break;
			case SYS_CHARGE:
				libat_charge_task();
				break;

#if (0 == SIMPLIFY)
			case SYS_TEST_BD:
				//libat_charge_task();
				if(tm_mode == TM_MODE_BOARD)
					proc_bdtest_task();
				else
				{
					proc_mactest_task();
				}
				break;
			case SYS_TEST_SEFT:			//2018--8-11 同步上海版本
				test_seft_task(1);
				break;

#endif
			default:
				if(TIM5->CNT >=5000)
				{
					//log_printf("GPIOD:%X\r\n", GPIOD->IDR);
					TIM5->CNT = 0;
					sys->sState_auxi = 0;		///辅助状态
				#if 0//debug
					if(sys->t_idle < T_IDLE_IR)		//运动
				#else
					if(proc_motor_run_task()==0 && sys->t_idle < T_IDLE_IR)		//运动
				#endif
					{
						//500ms
						if(i++>250)
						{
							i=0;
							//计数空闲的时间，超过3分钟，关红外
							if(sys->t_idle < T_IDLE_IR)
							{
								sys->t_idle++;
							}
							//休眠
							if(sys->t_idle >= T_IDLE_IR)
							{
								if(sys->power_on )
								{
									log_printf("power off...\r\n");
									sys->power_on = 0;
									all_key_led_off();
									//关屏幕
									//dp_tx_data(PD_MSG_WORK_STA, 0, 0,0,0);

								}
							//	    sys->power_led_on = 1;
									ny3p_play(VOICE_M_SLEEP);
									uint8_t n_powerdown = 0;
									dp_tx_data(DISP_CMD_DPINFO,1,&n_powerdown);
								//}
							}

						//	log_printf("%d,%d\r\n",sys->t_idle ,sys->power_on );

							sys->sState = SYS_IDLE;
							//usart4_write(">>>\r\n",5);
							robot_run_ctrl();				//控制电机的行走
							read_ir_data(irData_bak,0);
							illegal_copy_tell_13757122544_gsend(GYRO_SET_OFS_MSG,cfg->gyro_ofs1,cfg->gyro_ofs2);

							delay_ms(10);
						    //	 gyro_offset_manage(10);
							//get_gyro_ofs();
							//get_gyro_org();
							//delay_ms(10);
							////////////
							/*
							SPI_CS(0);			//工装测试指令
							SPI1_ReadWriteByte(0xB5);
							uint8_t n_res = SPI1_ReadWriteByte(0);
							SPI_CS(1);
							log_printf("n_res:%2X\r\n",n_res);
							*/
							//////////
							log_printf("\r\n---------------Software Version: %s----------------------\r\n", SW_VERSION);

							www_idleintel_com();
							illegal_copy_tell_13757122544_gsend(0x03,cfg->gyro_ajust,cfg->gyro_ajust);
#if 1			//测试不打印
							log_printf("BASE:(%5.3f),hw(%d,%d,%d,%d),bum(%d,%d,%d,%d),ir(%d,[%d,%d,%d],%d),b(%d,%d)\r\n",
									sys->angle,
									navigat->distance,
									navigat->gun_cnt,
									motor.c_left_hw,motor.c_right_hw,
									sys->gSta & MASK_BUM_LEFT,sys->gSta & MASK_BUM_LEFT2,sys->gSta & MASK_BUM_RIGHT,sys->gSta & MASK_BUM_RIGHT2,
									sys->g_sta[0],sys->g_sta[2],sys->g_sta[3],sys->g_sta[4],sys->g_sta[6],
									sys->g_buton[0][0],sys->g_buton[0][2]);

							log_printf("SENS:(%d,%d)M(%d),%d,%d,v=%d,d=%d,ofs=%d\r\n",LEFT_MOTOR_LEAVE(),RIGHT_MOTOR_LEAVE(),I_MID_ADC(),READ_DUSTBOX_DET(),READ_VWALL_DET(),sys->volt,V_PHOTORESISITOR(),sys->gyro_offset);
							log_printf("DOCK:(%d,%d,%d,%d)\r\n",irData_bak[IR_L_PIN_NUM], irData_bak[IR_ML_PIN_NUM], irData_bak[IR_MR_PIN_NUM], irData_bak[IR_R_PIN_NUM]);//,debug_ir_original_bk[IR_L_PIN_NUM],debug_ir_original_bk[IR_M_PIN_NUM],debug_ir_original_bk[IR_R_PIN_NUM],DOCK_DETECT()); //尘合、虚拟墙、灰尘，抹布
							log_printf("ADCV:(0)%d,%d,%d,%d;(4)%d,%d,%d,%d;(8)%d,%d,%d,%d;(12)%d,%d,%d,%d\r\n",
							adc_converted_value[0],adc_converted_value[1],adc_converted_value[2],adc_converted_value[3],
							adc_converted_value[4],adc_converted_value[5],adc_converted_value[6],adc_converted_value[7],
							adc_converted_value[8],adc_converted_value[9],adc_converted_value[10],adc_converted_value[11],
							adc_converted_value[12],adc_converted_value[13],adc_converted_value[14],adc_converted_value[15]);
							log_printf("=========================\r\n\r\n");
								irData_bak[IR_L_PIN_NUM] = irData_bak[IR_ML_PIN_NUM] = irData_bak[IR_MR_PIN_NUM] = irData_bak[IR_R_PIN_NUM] = 0;
							//uint16_t dock_ir_info = read_ir_dock_knk();

							uint8_t	dock_signal = ir_dock_insight(1);
								if(dock_signal)
									log_printf("top = %d\r\n",dock_signal);
							struct tm timenow = Time_GetCalendarTime();
							log_printf("%d/%d/%d %d:%d:%d\r\n",timenow.tm_year,timenow.tm_mon,timenow.tm_mday,timenow.tm_hour,timenow.tm_min,timenow.tm_sec);

#else
/*
							log_printf("|%5.3f,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d|\r\n",
									sys->angle,motor.c_front_hw,navigat->gun_cnt,motor.c_left_hw,motor.c_right_hw,
									sys->gSta ,
									sys->g_sta[0],sys->g_sta[1],sys->g_sta[2],sys->g_sta[3],sys->g_sta[4],sys->g_sta[5],sys->g_sta[6],
									sys->g_buton[0][0],sys->g_buton[0][2],
									LEFT_MOTOR_LEAVE(),RIGHT_MOTOR_LEAVE(),
									irData_bak[IR_L_PIN_NUM], irData_bak[IR_M_PIN_NUM], irData_bak[IR_R_PIN_NUM],
									DOCK_DETECT(),READ_DUSTBOX_DET(),READ_VWALL_DET(),
									adc_converted_value[0],adc_converted_value[1],adc_converted_value[2],adc_converted_value[3],
									adc_converted_value[4],adc_converted_value[5],adc_converted_value[6],adc_converted_value[7],
									adc_converted_value[8],adc_converted_value[9],adc_converted_value[10],adc_converted_value[11],
									adc_converted_value[12],adc_converted_value[13],adc_converted_value[14],adc_converted_value[15],
									LiBat_GetBatVolt());
								*/


#endif                    // delay_ms(100);

							//debug_ir_original_bk[IR_L_PIN_NUM] = 0;
							//debug_ir_original_bk[IR_M_PIN_NUM] = 0;
							//debug_ir_original_bk[IR_R_PIN_NUM] = 0;
							//log_printf("DUST_ADC:%d %d\r\n",V_DUST_ADC(),LiBat_GetBatVolt());

						}
					}
				}
				break;
		}
	}
}

void init_sys_status(uint8_t sta,uint8_t old_sta)
{


	//×′ì?
	if(sys->sState == SYS_NAVIGAT && sys->full2go)
		sys->wifi_sta = WIFI_STA_FULL2GO;
	else if(sys->sState == SYS_NAVIGAT)
		sys->wifi_sta = WIFI_STA_AUTO;
	else if(sys->sState == SYS_FOCUS)
		sys->wifi_sta = WIFI_STA_SPOT;
	else if(sys->sState == SYS_DOCK)
	    sys->wifi_sta = WIFI_STA_FINISH;
	else if(sys->sState == SYS_CHARGE)
		sys->wifi_sta = WIFI_STA_CHARGE;
	else
	{
		if(sys->pause)
			sys->wifi_sta = WIFI_STA_PAUSE;
		else
			sys->wifi_sta = WIFI_STA_STANDBY;
	}

	log_printf("[init_sys_status]sta=%d,old=%d,wifista=%d,pause=%d\r\n",sta,old_sta,sys->wifi_sta,sys->pause);

	//?aà?′óμ?o?á÷3ì1yà′?￡é¨íêá?￡???è?
	if(old_sta == SYS_NAVIGAT  && old_sta != sta)
	{
		log_printf("last sta=navi,area=%d,%d,t=%d,%d\r\n",sys->t_area,sys->m_area,sys->t_clean,sys->t_navi_work);
		sys->t_area +=sys->m_area;			//??é¨×ü???y?????￡
		//?aà?? 1óD??é¨μ?ê±3¤


        log_printf("last sta=navi,area=%d,%d,t=%d,%d\r\n",sys->t_area,sys->m_area,sys->t_clean,sys->t_navi_work);

		sys->m_worktime = sys->t_navi_work/60;
		sys->t_clean+= sys->m_worktime;
        log_printf("sys->m_area=%d\r\n",sys->m_area);
		I2C_WriteS_24C(0,(uint8_t *)sys, ERP_LEN);
		mcu_dp_value_update(DPID_TOTAL_AREA,sys->t_area);  //×ü???yé?±¨ 20200520
    	mcu_dp_value_update(DPID_CLEAN_AREA,sys->t_clean); //×ü??é¨ê±??é?±¨
        mcu_dp_value_update(DPID_CLEAN_AREA,sys->m_area);  //êμê±???yé?±¨
	}
	if(old_sta == SYS_NAVIGAT || old_sta == SYS_DOCK || old_sta == SYS_FOCUS)
	{
		motor_run(GO_STOP,0,0,0);
		log_printf("end of clean,save t_agitator,twork=%d(%d,%d)\r\n",navigat->t_navigat,cfg->t_agitator_inspection,cfg->t_agitator_replacemen);
		if(sys->t_navi_work > 0)
		{
			//cfg->t_agitator_inspection+=sys->t_navi_work / 60;
			//cfg->t_agitator_replacemen+=sys->t_navi_work / 60;
			save_cfg();
		}
		///老的状态可能是回充和集中清扫
		if( old_sta == SYS_DOCK)
		{
#if SIDE_RUSH_ALARM_CHECK
			side_rush_alarm_check_pro(2);
#endif
		}
       // log_printf("key12 %d..\r\n",sys->sState);
		if(old_sta == SYS_NAVIGAT  || old_sta == SYS_FOCUS)
		{
#if DUST_BOX_FULL_CHECK
			dust_full_check_pro(2); 	//清扫结束更新尘满累积时间
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
//			wifi_updata_clean_record();				//上报清扫记录使用
			sys->t_work = msTmr;				//记录开始的时间
			sys->t_navi_work=0;

		}
		//log_printf("key13 %d..\r\n",sys->sState);
		//clean_over_updata_clean_record();
#if 0		//从充电座出来
		if(old_sta == SYS_NAVIGAT)			//从充电座出来,之前是导航模式,切换到空闲或者回充模式
		{
			if(sys->work_errcode == MWERR_NONE)				//2019-01-25 add jzz 如果有错误,就不返回充电座了.
			{
				if( navigat->from_charge && navigat->x_dock > 0 && navigat->x_dock < MAX_GRID)
				{
					ny3p_play(VOICE_M_DOCK);
					log_printf("last sta == navi,stop and router to dock\r\n");
					motor_run(GO_STOP,0,0,0);
					sys->power_go = 0;
					init_charge(old_sta);  //涉及回充相关
					return ;
				}
			}
		}
#endif
		//printf_scan_obst();
	}
	//log_printf("key14 %d..\r\n",sys->sState);
	if(sta == SYS_IDLE)
	{
		log_printf("reset idle timer\r\n");
		sys->t_idle = 0;
	}

	/*else
	{
		log_printf("update  led,start......\r\n");
			sys->t_navi_work =0;
		sys->work_mod = 0;
		if(sys->dust_pwm_value == DUST_MODE_SILENT)
			sys->work_mod |= MWO_SILENT;
		else if(sys->dust_pwm_value == DUST_MODE_NORMAL)
			sys->work_mod |=MWO_NORMAL;
		else if(sys->dust_pwm_value == DUST_MODE_TURBO)
			sys->work_mod |=MWO_TURBO;

		if(sys->full2go)
			sys->work_mod |=MWO_FULL2GO;
		dp_tx_data(PD_MSG_WORK_STA, 0x80,sys->work_mod,0,0);		//上报开机一下。
		delay_ms(5);


	}
	*/

	timer5_config();
	/*
	if(sys->sState !=SYS_IDLE && old_sta == SYS_IDLE)
		ny3p_play(VOICE_M_STOP);
	*/
	//------------------------------------------modidied by wonton2004 20170118
#if 0
	if(sys->sState !=SYS_IDLE)
		dp_tx_data(PD_MSG_WORK_STA, sta,sys->work_mod, 0, 0);
#else
	//dp_tx_data(PD_MSG_WORK_STA, sta | (ROBOT_MOVE_STA() ? 0x80:0),sys->work_mod, 0, 0);
	//if(sys->sState !=SYS_CHARGE)
	log_printf("upload sta=%d\r\n",sys->sState);

			if(sta != SYS_IDLE)
				sys->work_errcode = MWERR_NONE;

	//开关机上报
	//#define POW_ON_STA	0X80






	//dp_tx_data(PD_MSG_WORK_STA, 0x80, sys->work_mod,0,0);

#endif
//log_printf("key18 %d..\r\n",sys->sState);
	pd_gyro_int(GO_FORWARD_PWM);
	if(sys->sState != SYS_IDLE)
	{
		//illegal_copy_tell_13757122544_gsend(0x03,cfg->gyro_ajust,cfg->gyro_ajust);
		//delay_ms(2);

		motor_run(GO_STOP,0,0,0);
		//delay_ms_sensers(300);	//等待陀螺仪稳定
		//delay_ms(300);
		//mpu6500_date_offset(4000);
	}
	/*
	if(sys->sState == SYS_NAVIGAT && sta !=SYS_NAVIGAT)
	{
		navigat->suspend = 1;
		printf_scan_obst();
	}
	*/
	if(sta != SYS_NAVIGAT && sta != SYS_IDLE)
		navigat->suspend =FALSE;
	mcu_dp_value_update(DPID_TYPE, sys->wifi_sta); //VALUEDíêy?Yé?±¨;
	if(sta !=SYS_CHARGE)
 		//sta=SYS_FOCUS;
		 log_printf("key17 %d..\r\n",sys->sState);
		 if (sys->sState==SYS_IDLE)
		 {
		    log_printf("key27 %d..\r\n",sys->sState);

		 }
	switch(sta)
{
//         log_printf("key22 %d..\r\n",sys->sState);
		case SYS_NAVIGAT:
			//ny3p_play(VOICE_M_START);

			sys->power_go = 1;
#if WIFICONFIG
		init_stream_clean_id();
#endif
			navigat_init(1);
			log_printf("debug state:%d\r\n",sys->sState);
			if(sys->sState != SYS_IDLE )
				sys->sState = SYS_NAVIGAT;

#if LASER_SIDE
			motor_go_draw_map(navigat);
			turn_to_deg(0);
#endif

			if(sys->sState != SYS_IDLE )
				sys->sState = SYS_NAVIGAT;

			break;
///**/ log_printf("key20 %d..\r\n",sys->sState);
		case SYS_RANDOM:
			//timer1_init();					//时钟1，控制两个边刷
			//timer2_init();					//控制中扫电机和吸尘电机
			ny3p_play(VOICE_M_START);
			#if 0
			init_random_task();
			#else
			proc_burn_in_init();
			#endif
			//MOTOR_CTRL(NORM_SIDE_PWM,NORM_SIDE_PWM,NORM_MID_PWM,NORM_DUST_PWM);
			break;
//			log_printf("key16 %d..\r\n",sys->sState);
		case SYS_FOCUS:                           //test  SYS_FOCUS
#if WIFICONFIG
		if(old_sta == SYS_IDLE)
			init_stream_clean_id();
			//pwifistream->wifiAreaNoCalc = 1;
#endif
		    sys->power_go = 1;
			//ny3p_play(VOICE_M_CLN_HEART);
			sys->sState = SYS_FOCUS;
			init_focus_task();
            log_printf("SYS_FOCUS ok %d..\r\n",sys->sState);
			if(sys->sState != SYS_IDLE)
				sys->sState = SYS_FOCUS;
			sys->run_speed = HIGHT_SPEED;
			//sys->run_speed = LOW_SPEED;


			break;
		case SYS_DOCK:			///按键 APP 低电压
		//如果之前不是导航，则陀螺仪复位一下
		//增加结束上报清扫记录
#if WIFICONFIG
		if(old_sta == SYS_IDLE)
			init_stream_clean_id();
#endif
			sys->sState_auxi=1;
			ny3p_play(VOICE_M_DOCK);
			if(old_sta != SYS_NAVIGAT)
			{
//				int i;
				log_printf("dock reset gyro...\r\n");
				//micony201809
				//reset_gyro(0);
				illegal_copy_tell_13757122544_gsend(0x01,0,0);

			}
			timer1_init();					//时钟1，控制两个边刷
			timer2_init();					//控制中扫电机和吸尘电机
			log_printf("key17 %d..\r\n",sys->sState);
			//micony201809
			delay_sensor(100);
			if(sys->sState == SYS_IDLE)
			 break;
			sys->sState = SYS_DOCK;
			MOTOR_POWER_OFF();
			MOTOR_POWER_ON();
			MOTOR_POWER_OFF();
			//MOTOR_CTRL(DOCK_SIDE_PWM,DOCK_SIDE_PWM,DOCK_DUST_PWM,DOCK_MID_PWM);
			init_charge(old_sta);	//这个是什么

		break;

		case SYS_CHARGE:
		sys->sState_auxi = 0;
		sys->power_go = 0;		///

			delay_ms(20);
			sys->charge_sta = NO_CHARGE;
			if(DOCK_DETECT())
				sys->charge_sta |= DOCK_CHARGE;
			if(EXTERAL_AC_DETECT())
				sys->charge_sta |=AC_CHARGE;


			sys->power_on = 1;
			all_key_led_off();
			//WIFI配网状态
#if 0

#else
			//=============================================================================
			//WIFI联网状态
			//=============================================================================
			if(sys->wifi_old_state != mcu_get_wifi_work_state())
			{		log_printf("wifioldstate:%d,wifinewstate%d\r\n",sys->wifi_old_state,mcu_get_wifi_work_state());
				sys->wifi_old_state = mcu_get_wifi_work_state();
				wifi_sync_state_pro(sys->wifi_old_state);
			}
#endif



			sys->work_errcode = MWERR_NONE;
//--------------------------------------------------------------------------------------------end
			log_printf("charge mode=%02X\r\n",sys->charge_sta);
			sys->wifistatechange = 1;
			if(sys->charge_sta != NO_CHARGE)
			{
				ny3p_play(VOICE_CHARING);
				#if 0		//2018-11-26jzz
				update_dock_pioint();
				#endif
				sys->sState = SYS_CHARGE;


	//dp_tx_data(PD_MSG_WORK_STA, temp_state,sys->work_mod,0,0);
				STOP_ALL_MOTOR();
				LiBat_HalInit();//初始化
				illegal_copy_tell_13757122544_gsend(GYRO_RESET_MSG,GYRO_REST_VALUE,GYRO_REST_VALUE);
			    //SET_MID_MOTER(0);   //20200428 充电坐上bug加入强制关闭
				/*
				if(navigat->from_charge )
			   	{
					coordinate_calcu();														//计算出原始的坐标系
					coord_org2map(navigat->x_org,navigat->y_org,&navigat->tx,&navigat->ty);	//坐标系转换
					log_printf("charge..now point=(%d,%d,)\r\n",X_NOW,Y_NOW);
					navigat->suspend = TRUE;	//暂时清掉
			   	}else
			   		navigat->suspend = FALSE;
			   	*/

			}
			else
			{
//					int i;
				if(old_sta == SYS_DOCK)			//回充过来的 ，则退一下，再回充
				{
					log_printf("charge err\r\n");
					MOTOR_POWER_ON();
					MOTOR_CTRL1(0,0,0,0);
					motor_run(GO_STOP,0,0,0);
					init_charge(SYS_IDLE);
					delay(100);
					delay_sensor(100);
					//motor_run(GO_BACK,DOCK_PWM_GO,400,0);		//回退一下
					//motor_run(GO_STOP,0,0,0);
					sys->sState = SYS_DOCK;
					log_printf("go back1...\r\n");
					motor_back_off(80);
					delay_ms(300);
					//reset_gyro(0);
					usart_init_info();
					MOTOR_POWER_ON();
					MOTOR_CTRL1(0,0,0,0);
					turn_to_deg(180);
					navigat->angle = 180;

					log_printf("turn to 180,go...\r\n");
					motor_go_forwark(2000,NO_SIDE_NEAR,NULL);

					timer1_init();					//时钟1，控制两个边刷
					timer2_init();					//控制中扫电机和吸尘电机
					sys->power_go = 0;
					ny3p_play(VOICE_M_DOCK);
					sys->sState = SYS_DOCK;
					MOTOR_POWER_ON();
					//MOTOR_CTRL(DOCK_SIDE_PWM,DOCK_SIDE_PWM,DOCK_DUST_PWM,DOCK_MID_PWM);
					init_charge(SYS_IDLE);
				}
				else
					sys->sState = SYS_IDLE;			////2018-10-09  待测试
			}
			break;
		//--------------------------------------------------------add by wonton2004 20170410
		case SYS_TEST_BD:
			timer1_init();					//时钟1，控制两个边刷
			timer2_init();					//控制中扫电机和吸尘电机
			senser_gpio_config();
			motor_run(GO_STOP, 0, 0, 0);
			//MOTOR_CTRL(1000, 1000, 0, 0);
			STOP_ALL_MOTOR();
			break;
		//-------------------------------------------------------------------------------end
		default:
			navigat->is_walk = 1;
			motor_run(GO_STOP, 0, 0, 0);
			STOP_ALL_MOTOR();
			break;
	}
}

void init_sys_sta(uint8_t sta)
{
	if(sys->sState == sta)
		return;

	if(sys->sState != SYS_CHARGE)
		sys->wifi_led_enable = 1;

	if(sys->sState != SYS_IDLE || sta != SYS_IDLE )
	{
		motor_run(GO_STOP, 0, 0, 0);
		STOP_ALL_MOTOR();
		delay_ms(800);
	}
	sys->sState = sta;
	navigat->whele_stop = 0;
#if 0
	timer5_config();
	if(sys->sState !=SYS_IDLE && sta == SYS_IDLE)
		ny3p_play(VOICE_M_STOP);
	if(sys->sState !=SYS_IDLE)
		dp_tx_data(PD_MSG_WORK_STA, sta,sys->work_mod, 0, 0);
	pd_gyro_int(GO_FORWARD_PWM);
	if(sys->sState != SYS_IDLE)
	{
		illegal_copy_tell_13757122544_gsend(0x03,cfg->gyro_ajust,cfg->gyro_ajust);
		delay_ms(2);
	}
	/*
	if(sys->sState == SYS_NAVIGAT && sta !=SYS_NAVIGAT)
	{
		navigat->suspend = 1;
		printf_scan_obst();
	}
	*/
	if(sta != SYS_NAVIGAT && sta != SYS_IDLE)
		navigat->suspend =FALSE;

	if(sta !=SYS_CHARGE)
		sys->sState = sta;//SYS_DOCK;//SYS_NAVIGAT;
	switch(sta)
	{
		case SYS_NAVIGAT:
			ny3p_play(VOICE_M_START);
			navigat_init(1);
			sys->sState = SYS_NAVIGAT;

			break;
/*
		case SYS_RANDOM:
			timer1_init();					//时钟1，控制两个边刷
			timer2_init();					//控制中扫电机和吸尘电机
			init_random_task();
			navigat_init(0);
			MOTOR_CTRL(NORM_SIDE_PWM,NORM_SIDE_PWM,NORM_MID_PWM,NORM_DUST_PWM);
*/
		case SYS_FOCUS:
			ny3p_play(VOICE_M_CLN_HEART);
			init_focus_task();
			sys->sState = SYS_FOCUS;
			break;
		case SYS_DOCK:
			timer1_init();					//时钟1，控制两个边刷
			timer2_init();					//控制中扫电机和吸尘电机
			ny3p_play(VOICE_M_DOCK);
			sys->sState = SYS_DOCK;
			MOTOR_POWER_ON();
			init_charge();
			//mstimeout(&sys->t_loop,0);
			MOTOR_CTRL(DOCK_SIDE_PWM,DOCK_SIDE_PWM,DOCK_DUST_PWM,DOCK_MID_PWM);
			break;
		case SYS_NEAR:
			timer1_init();					//时钟1，控制两个边刷
			timer2_init();					//控制中扫电机和吸尘电机
			MOTOR_CTRL(200,200,0,0);

			go_near_wall(RIGHT_SIDE_NEAR);
			break;
		case SYS_CHARGE:
			sys->charge_sta = 0;
			delay_ms(20);
			if(DOCK_DETECT())
				sys->charge_sta |= DOCK_CHARGE;
			if(EXTERAL_AC_DETECT())
				sys->charge_sta |=AC_CHARGE;

			if(sys->charge_sta != 0)
			{
				ny3p_play(VOICE_M_DOCK);
				sys->sState = SYS_CHARGE;
				log_printf("KKKKIIIIKKI,charge mode=%02X\r\n",sys->charge_sta);
				dp_tx_data(PD_MSG_WORK_STA, sys->sState,0, 0, 0);
				ny3p_play(VOICE_CHARING);
				STOP_ALL_MOTOR();
				LiBat_HalInit();//初始化
				navigat->suspend = FALSE;	//暂时清掉
			}

			break;
		default:
			navigat->is_walk = 1;
			motor_run(GO_STOP, 0, 0, 0);
			STOP_ALL_MOTOR();

			break;
	}


#endif


}

/*
//============一次直行，每隔5ms打印一次左轮和右轮的霍尔数，结论是，基本上5个ms也就动一两个霍尔
0,0
1,0
2,0
3,0
5,1
6,1
8,1
10,2
12,2
13,3
*(100,100,180.6,0)[100,100]*
15,3
17,4
19,4
20,5
22,6
24,7
26,7
27,8
29,9
31,10
32,11
34,12
36,13
38,14
39,15
41,16
42,17
44,18
46,20
47,21
49,22
50,23
52,25
53,26
55,27
56,28
58,29
59,30
61,32
62,33
63,34
65,35
66,36
67,37
69,38
70,40
71,41
73,42
74,43
75,44
77,45
78,47
79,48
81,49
82,50
83,51
85,53
86,54
87,55
89,56
90,57
92,58
93,59
94,60
95,61
97,63
98,64
99,65
101,66
102,67
103,68
105,69
106,71
107,72
109,73
110,74
112,75
113,76
114,77
115,79
116,80
118,81
119,82
120,83
122,84
123,85
124,87
126,88
127,89
128,90
130,91
131,92
132,94
134,95
135,96
136,97
138,98
139,99
140,101
142,102
143,103
144,104
146,105
147,106
148,107
149,109
151,110
152,111
154,112
155,113
156,115
158,116
159,117
161,118
162,119
163,120
165,121
166,123
167,124
169,125
170,127
172,128
173,129
175,130
176,132
178,133
179,134
180,136
182,137
183,139
185,140
186,141
188,143
189,144
191,145
193,147
194,148
196,150
197,151
199,152
200,154
202,155
203,157
205,158
207,159
208,161
210,162
212,163
213,165
215,166
216,168
218,169
219,170
221,172
222,173
224,174
226,176
227,177
229,178
230,180
&<4,14907>&
*(99,100,179.9,0)[100,100]*
232,182
234,183
236,185
238,186
239,187
241,189
243,191
245,192
247,194
249,196
251,197
253,199
255,201
257,203
259,204
261,206
263,208
265,210
267,212
269,213
272,215
274,217
276,219
278,221
280,223
283,225
285,226
287,228
289,230
291,232
293,234
296,236
298,238
300,240
302,242
304,244
307,246
309,248
311,250
313,252
316,254
318,256
320,258
322,259
325,261
327,263
329,265
332,267
334,269
336,271
338,273
340,275
343,277
345,279
348,281
350,283
353,285
355,287
357,289
360,291
362,294
365,296
367,298
370,300
373,303
375,305
378,307
381,310
383,312
386,314
389,316
391,319
394,321
396,323
399,326
402,328
405,330
407,333
410,335
412,338
415,340
418,342
420,345
423,347
426,350
429,352
431,354
434,357
437,359
440,362
443,364
445,367
448,369
451,371
454,374
457,377
459,379
462,381
465,384
468,387
471,389
473,391
476,394
479,396
482,399
485,401
488,404
491,407
494,409
497,412
500,414
503,417
506,420
509,423
513,425
516,428
519,431
522,433
525,436
528,439
531,441
534,444
537,447
540,449
543,452
546,455
549,457
552,460
555,463
558,466
562,468
565,471
568,474
571,476
574,479
577,482
580,484
583,487
586,490
589,493
592,495
595,498
599,501
602,504
605,506
608,509
611,512
614,515
618,517
621,520
624,523
627,526
630,528
634,531
637,534
640,537
643,540
647,542
650,545
653,548
657,551
660,554
664,557
667,560
671,563
674,566
677,569
681,572
684,575
688,578
691,581
695,584
698,587
701,589
705,592
708,595
711,599
715,602
718,604
722,608
725,611
729,614
732,617
736,620
739,623
742,626
746,629
749,632
752,635
756,638
759,641
763,644
766,647
769,651
773,654
776,657
780,660
783,663
787,666
791,669
*(98,100,179.9,0)[98,100]*
795,673
798,676
802,679
805,682
809,685
812,688
816,691
819,694
823,697
826,701
830,704
834,707
838,710
841,713
845,716
849,720
853,723
856,726
860,729
863,732
867,735
871,739
874,742
878,745
882,748
885,751
889,755
893,758
897,761
900,764
904,767
908,770
911,774
915,777
919,780
922,783
926,786
929,790
933,793
937,796
941,799
944,802
948,805
952,809
955,812
959,815
963,818
966,822
970,825
974,828
977,831
981,834
985,838
988,841
992,844
996,847
1000,851
1004,854
1008,857
1011,860
1015,864
1019,867
1023,870
1027,874
1030,877
1034,881
1038,884
1042,887
1046,891
1049,895
1053,898
1057,901
1061,905
1065,908
1069,912
1073,915
1077,918
1081,921
1085,925
1088,928
1092,932
1096,935
1100,938
1103,942
1107,945
1111,948
1115,952
1119,955
1123,958
1127,962
1131,965
1134,968
1138,972
1142,975
1146,978
1150,982
1153,985
1157,988
1161,992
1165,995
*(97,100,180.0,0)[97,100]*
1169,999
1173,1003
1177,1006
1181,1009
1185,1013
1189,1016
1193,1020
1197,1023
1201,1026
1205,1030
1209,1033
1213,1037
1217,1040
1221,1044
1225,1047
1229,1051
1233,1054
1237,1058
1241,1061
1245,1065
1249,1068
1253,1072
1257,1075
1261,1079
1265,1082
1269,1086
1273,1089
1277,1093
1281,1096
1285,1099
1289,1103
1293,1106
1297,1110
1301,1113
1305,1117
1309,1121
1313,1124
1317,1128
1321,1131
1325,1135
1329,1138
1333,1142
1337,1145
1341,1149
1345,1153
1349,1156
1353,1160
1357,1163
1361,1167
1365,1171
1369,1174
1373,1178
1377,1181
1381,1185
1385,1189
1389,1192
1394,1196
1398,1199
1402,1203
1406,1207
1410,1210
1414,1214
1419,1217
1423,1221
1427,1224
1431,1228
1435,1232
1439,1235
1443,1239
1447,1242
1451,1246
1456,1250
1460,1253
1464,1257
1468,1260
1472,1264
1476,1267
1480,1271
1485,1275
1488,1278
1492,1282
1496,1285
1500,1289
1504,1293
1509,1296
1513,1300
1517,1303
1521,1307
1525,1310
1529,1314
1533,1317
1538,1321
1542,1325
1546,1328
1550,1332
1554,1335
1558,1339
1562,1342
1566,1346
1570,1350
1575,1353
1579,1357
1583,1360
1588,1364
1592,1367
1596,1371
1600,1375
1604,1378
1608,1382
1612,1386
1617,1390
1621,1393
[go_forw]bum!gSta=512(512),ir=(34,60,75)1900,dist=0,dock=0!!
gsta=512
  b&bof ok,dis=1567,1571
o&
*(97,100,180.0,0)-[-1564,0,180.6]*save,-1560.7,-337.0

go_forwark ok,dis=0,dis1=1349,left=1569,right=1547,ldis=0,tmr=4299,xy(97,100,)org(-1564,0)0,dock=0,0,0
&<7,14907>&
walk ok nearwall..nsta=1,err=0
normal nearback..
right p=152.000000,i=0.600000,d=235.500000
[near_go_back]sta=0(1),now=(97,100,-1564,0),y=(200,0,)bum=0,walkside=1,(96,,)
0,0
1,0
2,0
4,0
6,0
9,1
11,1
14,2
17,2
19,3
22,4
25,4
28,5
31,6
34,8
36,9
39,10
42,11
45,13
48,14
51,16
54,17
57,19
59,20
62,22
65,24
68,26
70,27
73,29
76,31
78,33
81,35
83,37
86,39
88,41
91,44
93,46
96,48
98,50
101,53
103,55
*(96,100,179.8,0)-[96,100,63824]*13023
107,58
109,60
112,62
114,64
117,67
119,69
122,71
124,73

*/
