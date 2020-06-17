
#include "sys.h"

RmCln_t g_rand_clean;
void init_random_task(void)
{
	navigat_init(0);
	navigat->distance = 0;
	//g_rand_clean.gSta = RMCLN_NEAR;//RMCLN_RM;
	g_rand_clean.gSta = RMCLN_RM;//;
	g_rand_clean.slip = 0;
	g_rand_clean.check_lost = 0;
	g_rand_clean.short_distance = 0;
	www_idleintel_com();
	navigat->angle = sys->angle;
	sys->t_work = msTmr;		//记录开始的时间
	sys->work_tmr = 0;
}
#define MAX_RAND_IRDA		2300
#define HAVE_IRDA			600//cfg->have_irda

float turn_hw_fifo[10]={100,100,60,60,50,70,90,90,90,90};
//随机模式
void proc_random_task(void)
{

	if(sys->tSta !=0)
	{
		robot_usart_test_cmd();
		return ;
	}
#if (0)
	turn_to_deg(0);
	motor_go_forwark(0,NO_SIDE_NEAR,NULL);		//直行
	turn_to_deg(180);
	motor_go_forwark(0,NO_SIDE_NEAR,NULL);		//直行
#else
//	static int tmr = 0;//,i;
//	static int t_slip=0,c_slip=0,t_near=0,t_near_ok=0;
	motor_go_forwark(0,NO_SIDE_NEAR,NULL);		//直行
	//左边碰撞
	if(sys->sState == SYS_IDLE)
		return ;

	{
		log_printf("------random bum------\r\n");
		//www_idleintel_com();

		if(navigat->distance < 100)
			g_rand_clean.short_distance++;
		else
			g_rand_clean.short_distance =0;
		if(g_rand_clean.short_distance >=10)
		{
			if(g_rand_clean.gSta !=RMCLN_NEAR)
				g_rand_clean.c_near_wall = 0;
			g_rand_clean.gSta = RMCLN_NEAR;
		}
		if(g_rand_clean.gSta == RMCLN_NEAR)
		{
			if(g_rand_clean.c_near_wall ++ >=10)
			{
				log_printf("near wall over\r\n");
				g_rand_clean.gSta = RMCLN_RM;
			}
		}
	/*
		log_printf("bum !!,dis=%d,c=%d,gsta=%d,%f,%d,(%d,%d)(%d,%d,%d,%d,%d,%d,%d)\r\n",navigat->distance,g_rand_clean.short_distance ,g_rand_clean.gSta,sys->angle,sys->gyro_offset,
								sys->gSta & MASK_BUM_LEFT,sys->gSta & MASK_BUM_RIGHT,
								sys->g_sta[0],sys->g_sta[1],sys->g_sta[2],sys->g_sta[3],sys->g_sta[4],sys->g_sta[5],sys->g_sta[6]);




		if(sys->gSta & MASK_BUM_MIDL || g_rand_clean.slip )
		{
			motor_back_off(10);
		}
	*/


		/*
		if(g_rand_clean.gSta == RMCLN_NEAR)
		{
			// motor_turn(GO_LEFT,80);
			g_rand_clean.is_near = 1;
			turn_vertical_right();
		}else
		*/
		{
			if(sys->gSta & MASK_LEFT || sys->g_sta[1] < MAX_RAND_IRDA || sys->g_sta[2] < MAX_RAND_IRDA || sys->g_sta[3] < MAX_RAND_IRDA)
				motor_turn(GO_RIGTH,600,turn_hw_fifo[get_rand_idx()]);
			else
				motor_turn(GO_LEFT,600,turn_hw_fifo[get_rand_idx()]);
		}
		www_idleintel_com();
		navigat->angle = sys->angle;
		g_rand_clean.slip = 0;

		// lpid.out = 0;

	}
#endif
}

char get_rand_idx(void)
{
	int	i;
	while(1)
	{
		i=rand() % 10;
		if(i >=0 && i < 10)
			break;
	}
	return (i);

}
void turn_vertical_right(void)
{
	int c=0;
	int t_slip=0,c_slip=0;
	log_printf("turn_vertical_right");
	motor.c_left_hw=0;
	motor_run(GO_LEFT,700,0,0);
	TIM5->CNT =0;
	g_rand_clean.c_turn_near_right = 0;
	while(1)
	{
		if(TIM5->CNT >=2000)
		{
			www_idleintel_com();
			if(sys->g_sta[3] > HAVE_IRDA && sys->g_sta[4] > HAVE_IRDA /*&& (sys->g_sta[4] <= MAX_RAND_IRDA || sys->g_sta[4] <= MAX_RAND_IRDA )*/)
			{
				if(c++>=25)
				{
					log_printf("OK,hw=%d,(%d,%d,%d,%d,%d,%d,%d)\r\n",motor.c_left_hw,sys->g_sta[0],sys->g_sta[1]
												,sys->g_sta[2],sys->g_sta[3],sys->g_sta[4],sys->g_sta[5],sys->g_sta[6]);
					motor_run(GO_STOP,0,0,0);
					break;
				}
			}else
				c=0;
			if(motor.c_left_hw > 1000)
			{
				log_printf("OVER,hw=%d,(%d,%d,%d,%d,%d,%d,%d)\r\n",motor.c_left_hw,sys->g_sta[0],sys->g_sta[1]
											,sys->g_sta[2],sys->g_sta[3],sys->g_sta[4],sys->g_sta[5],sys->g_sta[6]);
				motor_run(GO_STOP,0,0,0);
				break;
			}
			if(t_slip ++ > 800)
			{
				if(c_slip != navigat->distance)
					c_slip = navigat->distance;
				else
				{
					log_printf("slip..\r\n");
				//	g_rand_clean.slip = 1;
					motor_run(GO_STOP,700,0,0);
					motor_back_off(10);
					break;
				}
				t_slip = 0;
			}
		}
	}

	delay_ms(200);
	log_printf(",c=%d\r\n",motor.c_left_hw);
	/*
	while(1)
	{
		delay_ms(400);
		www_idleintel_com();
			log_printf("OVER,hw=%d,(%d,%d,%d,%d,%d,%d,%d)\r\n",motor.c_left_hw,sys->g_sta[0],sys->g_sta[1]
											,sys->g_sta[2],sys->g_sta[3],sys->g_sta[4],sys->g_sta[5],sys->g_sta[6]);

	}
	*/
}

void turn_near_right(void)
{

	int t_slip=0,c_slip=0;
	log_printf("turn_near_right:");
	motor.c_left_hw=0;
	//motor_run(GO_RIGTH,700,0,0);
	r_motor_set_pwm(GO_FORWARD, 800);
	l_motor_set_pwm(GO_FORWARD, 450);
	TIM5->CNT =0;

	while(1)
	{
		if(TIM5->CNT >=2000)
		{
			www_idleintel_com();
			if(sys->g_sta[3] < HAVE_IRDA || sys->g_sta[4] < HAVE_IRDA ||sys->g_sta[5] < HAVE_IRDA||sys->g_sta[6] < HAVE_IRDA)
			{
				log_printf("OK,hw=%d,(%d,%d,%d,%d,%d,%d,%d)\r\n",motor.c_left_hw,sys->g_sta[0],sys->g_sta[1]
											,sys->g_sta[2],sys->g_sta[3],sys->g_sta[4],sys->g_sta[5],sys->g_sta[6]);
				//motor_run(GO_STOP,0,0,0);
				break;
			}
			if(motor.c_left_hw > 2000)
			{
				log_printf("OVER,hw=%d,(%d,%d,%d,%d,%d,%d,%d)\r\n",motor.c_left_hw,sys->g_sta[0],sys->g_sta[1]
											,sys->g_sta[2],sys->g_sta[3],sys->g_sta[4],sys->g_sta[5],sys->g_sta[6]);
			//	motor_run(GO_STOP,0,0,0);
				break;
			}
			if(t_slip ++ > 800)
			{
				if(c_slip != navigat->distance)
					c_slip = navigat->distance;
				else
				{
					log_printf("slip..\r\n");
					//g_rand_clean.slip = 1;
					motor_run(GO_STOP,700,0,0);
					motor_back_off(10);
					break;
				}
				t_slip = 0;
			}
		}
	}

	//delay_ms(200);
	log_printf(",c=%d\r\n",motor.c_left_hw);
	g_rand_clean.check_lost = 80;
	g_rand_clean.c_turn_near_right ++;

	pd_gyro_int(GO_FORWARD_PWM);
	illegal_copy_tell_13757122544_gsend(0x01,0,0);
	motor_run(GO_FORWARD, GO_FORWARD_PWM, 0, 0);
//	lpid.out = 0;
}

void robot_usart_test_cmd(void)
{
	//log_printf("[robot_usart_test_cmd]cmd=%d,v1=%d,v2=%d\r\n",sys->tSta,sys->tv1,sys->tv2);
	if(sys->tSta ==1)		//风机测试
	{
		MOTOR_POWER_ON();
		if(sys->tv1==1)
		{
			DUST_MOTOR_RANK1();
		}
		else if(sys->tv1 == 2)
		{
			DUST_MOTOR_RANK2();
		}
		else
		{
			DUST_MOTOR_RANK3();
		}
		TIM5->CNT =0;
		while(1)
		{
			if(sys->sState == SYS_IDLE)
			{
				log_printf("sys idle return ...\r\n");
				DUST_MOTOR_OFF();
				STOP_ALL_MOTOR();
				return ;
			}
			proc_uart_task();
			if(TIM5->CNT >=10000)
			{
				TIM5->CNT = 0;
				log_printf("adc,%d,%d\r\n",V_DUST_ADC(),0);

			}


		}

	}else if(sys->tSta ==2)	//滚刷测试
	{
		MOTOR_POWER_ON();
		timer1_init();					//时钟1，控制两个边刷
		timer2_init();					//控制中扫电机和吸尘电机s

		//MOTOR_CTRL(NORM_SIDE_PWM_L,NORM_SIDE_PWM_R,sys->tv1,0);

		//	MOTOR_POWER_ON();
		AGITATOR_PWM = sys->tv1;
		log_printf("agi go2 ,pwm=%d\r\n",AGITATOR_PWM);
		uint16_t i=0;
		uint16_t agi_cnt=0;
		while(1)
		{
			if(sys->sState == SYS_IDLE)
			{
				log_printf("sys idle return ...\r\n");
				DUST_MOTOR_OFF();
				STOP_ALL_MOTOR();
				return ;
			}
			proc_uart_task();
			if(TIM5->CNT >=5000)
			{
				TIM5->CNT = 0;
				i++;
				if(i>=50)
				{
					i=0;
					log_printf("agi cnt=%d,%d\r\n",navigat->gun_cnt - agi_cnt,navigat->gun_cnt);
					agi_cnt = navigat->gun_cnt;
				}

			}
		}
	}else if(sys->tSta == 3)		//后退测试
	{
		navigat_init(0);
		sys->sState = SYS_NAVIGAT;
		motor_back_off(sys->tv1);
		sys->sState = SYS_IDLE;
	}else if(sys->tSta == 4)  //转圈测试
	{
		navigat_init(0);
		sys->sState = SYS_NAVIGAT;
		//GO_LEFT 3,4
		robot_turn_deg(sys->tv1,700,sys->tv2);
		sys->sState = SYS_IDLE;
	}else if(sys->tSta == 5) //直行测试
	{
		sys->dust_pwm_lev = sys->tv1;		//值 0 - slient模式，1-normal 2-涡轮增压
		navigat_init(0);

		sys->sState = SYS_NAVIGAT;

		motor_go_forwark_for_test();
	}
	else if(sys->tSta == 13)  //沿边测试
	{

		log_printf(" near wall\r\n");

		navigat_init(0);
		MOTOR_CTRL(NORM_SIDE_PWM_L,NORM_SIDE_PWM_R,0,FOCUS_DUST_PWM);
		sys->sState = SYS_NAVIGAT;

		motor_go_edgeways(LEFT_SIDE_NEAR,0,0,GO_NEAR_TYPE_NO,0);


		STOP_ALL_MOTOR();
		sys->sState = SYS_IDLE;

	}

}




//转圈的标准函数
//dir 方向 左右 pwm 速度 agle 角度
uint8_t robot_turn_deg_test(uint8_t dir,int pwm,float agle)
{
//	uint16_t t=0;

	float dis_agle,b_agle;
	float out_deg;
	int midl=0,cc=0;
	//uint16_t c_escape = 0;		//脱困次数
//	int16_t reverse_turn=0;		//反向旋转的技术
	///uint32_t t;
	uint8_t sState = sys->sState;;
	www_idleintel_com();
	printf_power_sta();
	//log_printf("turn_charge,dir=%s,agle=%f\r\n",get_motor_sta(dir),agle);
	motor.c_left_hw = motor.c_right_hw = 0;
	//log_printf("1\r\n");
	CHECK_NAVI_STA2();

	b_agle = sys->angle;
	//log_printf("2\r\n");
	//coordinate_calcu();
	//gyro_whlmap();
	navigat->is_walk = 0;
	//log_printf("3\r\n");
	turn_round_pid(0);
	//log_printf("4\r\n");
	motor.c_left_hw = motor.c_right_hw = 0;
	motor_run(dir,pwm,0,0);
	sys->right_pwm = sys->left_pwm = pwm;
	TIM5->CNT = 0;
	//从中间碰到红外转过来
	if(MIDLE_HAVE_IRDA2() || *(navigat->near.pid->adc2) >= MAX_IR_SIDE2 )
		midl = 1;

	out_deg = NO_ANGLE_V;
	///t=0;
	coordinate_calcu(0);


//	last_turn_stat.turn_agle = agle;
	turn_check_bum_pro(0,dir,0,0);

	int turn_speed = TURN_SPEED;
//	float esc_yaw=1000;
//	uint32_t turn_timer = 0;
	sys->c_escape = 0;
	while(1)
	{
		get_sensers(&sys->gSta);
		CHECK_NAVI_STA2();
		//laser_calc_move(navigat,MAX_LASR_DIST);
		if(sys->sState !=sState|| sys->sState == SYS_IDLE)
			break;
			//return 0;
		if(turn_check_bum_pro(1,dir,pwm,turn_speed))
		{
			b_agle = sys->angle;
		}


		if(TIM5->CNT >=5000)
		{
			TIM5->CNT = 0;
			turn_round_pid(turn_speed);
			www_idleintel_com();


			dis_agle = b_agle -sys->angle;
			if(dis_agle > 180)
				dis_agle = 360 - dis_agle;
			if(dis_agle <-180)
				dis_agle += 360;
			if(dis_agle >=agle || dis_agle <=-agle)
				break;
			//中间没有红外，就退出
			if(out_deg == NO_ANGLE_V)
			{
				if(agle == WALL_LOST_DEG && midl && (dis_agle >=20 || dis_agle <=-20))
				{
					if(sys->g_sta[2] < 300 && sys->g_sta[3] < 150 && sys->g_sta[4] < 280 &&
						*(navigat->near.pid->adc2) < MAX_IR_SIDE2)
					{
						//out_deg = sys->angle;
						if(*(navigat->near.pid->adc2) < 1700)
						{
							//log_printf("found V_angle=%3.1f,%d\r\n",sys->angle,*(navigat->near.pid->adc2));
							break;
							//return 0;			//这个是什么状态返回呢?确定一下!
						}
						//log_printf("\r\n");
					}
				}
			}else
			{
				if(disfloat(sys->angle,out_deg) > 5)	//再转10°
				{
					//log_printf("VOK,v_agle=%3.1f\r\n",sys->angle);
					//break;
					return 1;			//旋转OK
				}
			}
/*
			if(t++>=1000)		//15s 超时  7.5s超时
			{
				log_printf("timeout,fw2..\r\n");
				b_agle = sys->angle;
				gyro_mapwhl();
				navigat->angle = sys->angle;
				motor_go_forwark(300,NO_SIDE_NEAR,NULL);
				motor_run(dir,pwm,0,0);
	      		turn_round_pid(0);
	      		t = 0;
			}
			*/
			if(sys->mState == GO_STOP)
			{
			//	log_printf("found stop..turn again...\r\n");
				motor_run(dir,pwm,0,0);
				turn_round_pid(0);

			}

			if(cc++>=8)
			{
				log_printf("*(%d,%d,%3.1f,0)[%d,%d]*,%d,%d,%d,%d,%d\r\n",navigat->tx,navigat->ty,sys->angle,L_FORWRK_PWM,R_FORWRK_PWM,
										sys->left_curr,sys->right_curr,I_MID_ADC(), V_DUST_ADC(),DUST_VALUE());//左轮，右轮，滚刷，风机，灰尘

				cc=0;
			}
		}


		//log_printf("%3.1f,%3.1f\r\n",sys->angle,dis_agle);
	}
	motor_run(GO_STOP,0,0,0);                             ////后加入的,有问题吗？
	DELAY_MS(100);
	//delay_ms_sensers(200);
#if LASER_SIDE
		calc_scan_laser(navigat,1);
#endif
//	www_idleintel_com();
	//gyro_whlmap();
	//gyro_mapwhl();
	gyro_mapwhl();
	//navigat->is_walk =1;
	//log_printf("..OK,c=%d,%d,agl=%3.1f\r\n",motor.c_left_hw,motor.c_right_hw,sys->angle);
	return 0;
}

char motor_go_forwark_for_test(void)
{

//	int ret =1;
//	uint8_t  sta = sys->sState ;
	uint16_t c_calc=0;

//	short bx = X_NOW,by=Y_NOW;



	pd_gyro_int(GO_FORWARD_PWM);

	www_idleintel_com();
	motor.c_left_hw = motor.c_right_hw = motor.c_front_hw = 0;



	navigat->out = sys->angle;



	navigat->distance = 0;
	sys->gsta= 0;

	TIM5->CNT = 0;



//	log_printf("[motor_go_fw](%d,%d,%3.1f,%3.1f)pwm=%d,ld=%d,nsta=%d,ofs=%3.3f\r\n",navigat->tx,navigat->ty,sys->angle,navigat->angle,sys->pwm,0,nsta,sys->yaw_ofset);
	navigat->l_dist = navigat->distance;

	m_speed_pid_ctrl(0,0);		//记录左轮霍尔数


	motor.c_left_hw = motor.c_right_hw = 0;



	motor_run(GO_FORWARD, GO_FORWARD_PWM, 0, 0);

	//agitator_speed_pid_ctrl(0,0);
	r_near_pid.min_adc 	= IRD_RIGHT_PID_LOCK;
	l_near_pid.min_adc 	= IRD_LEFT_PID_LOCK;
//	uint16_t agi_speed;

	// if(sys->dust_pwm_lev ==DUST_MODE_SILENT)
	//	agi_speed = cfg->l_agi_speed;
//	else
	//	agi_speed = cfg->h_agi_speed;



	while(1)
	{
		get_sensers(&sys->gSta);			//
		proc_uart_task();
		if(sys->sState == SYS_IDLE)
			return 0;

		if(TIM5->CNT >=5000)
		{
			TIM5->CNT = 0;

			c_calc++;
			if(c_calc >=8)		//250ms
			{
				c_calc = 0;

				if(motor.c_left_hw > 10 ) 	//提前减速
					m_speed_pid_ctrl(0,1);
				//agitator_speed_pid_ctrl(agi_speed,1);
				coordinate_calcu(1) ;
				coord_org2map(navigat->x_org,navigat->y_org,&navigat->tx,&navigat->ty);	//坐标系转换


				log_printf("Vb=%d,left=%d,%3.3f,%d,%3.3f,agi=%d,%3.3f\r\n",sys->volt,sys->left_curr,sys->vleft,sys->right_curr,sys->vright,sys->agi_curr,sys->vagi);//左轮，右轮，滚刷，风机，灰尘

			}

		}




	}

//	return ret;
}
//======================================集中清扫============================================
/*
#define FOCUS_INC	20
#define FOCUS_BASE	50

typedef struct _focus_t
{
	int 	cnt;
	int 	distance;		//走的距离
//	char	index;			//序号

}FOCUS_T;

FOCUS_T focus;

#define MAX_FOCUS_DEGRE_COUNT		4
float fos_degs[MAX_FOCUS_DEGRE_COUNT];//={0,90,180,270};
//int fos_count=0,focus.distance=0;
*/
#if 0
void init_focus_task(void)
{
	//focus.distance = FOCUS_BASE*WHELE_HW;
//	focus.index = 0;
	//focus.cnt = 0;

	//focus.distance = FOCUS_BASE*WHELE_HW;
	navigat_init(0);
	/*
	focus.cnt =0;


	fos_degs[0] = 0;
	fos_degs[1] = 90;
	fos_degs[2] = 180;
	fos_degs[3] = 270;
	*/
	DUST_MOTOR_RANK3();
	LED1_ON();
	log_printf("init focus ok\r\n");
}


char motor_go_spot_task(void)
{
	static short c_calc = 0;
	uint8_t  sta = sys->sState ;
	uint16_t step=0,maxc=0;
	uint16_t  disa,spot_speed;
	uint16_t cycle=0;
	uint16_t speed=0;
	float angle = sys->angle;

	LED1_ON();
	LED2_OFF();
	LED3_OFF();
	LED4_OFF();
	//sys->spot_flag = 1;
	sys->t_flash_halt = 0;
	sys->t_halt_on = 1;

 	CHECK_NAVI_STA_RT(0);
 	log_printf("[motor_go_spot_task]begin ,reset gyro..\r\n\r\n");

	www_idleintel_com();  //取一下角度   navigat->angle;
	motor.c_left_hw = motor.c_right_hw = motor.c_front_hw = 0;
	navigat->out = navigat->angle;
//	navigat->distance = sys->c_pid_hw = 0;
	sys->gsta= 0;
	TIM5->CNT = 0;

	navigat->l_dist = navigat->distance;
	sys->c_front_stop = 0;		//前轮停止的次数清零
	m_speed_pid_ctrl(0,0);		//记录左轮霍尔数
	robot_whele_stop(0);
	delay_ms_sensers(5);
	motor.c_left_hw = motor.c_right_hw = 0;
	motor_wheel_forward(LEFT_WHEEL, 650);
	motor_wheel_forward(RIGHT_WHEEL, 900);
	sys->mState = GO_FORWARD;
	//delay_ms(2);
//	navigat->is_walk = 1;
	c_calc = 40;
//	sys->navi_run = 1;
//	sys->walk_bum = 0;
	printf_power_sta();

	step = cfg->spot_step_disa / 100;			//增加的步长
	disa = cfg->spot_step_disa % 100;			//切换速度的角度差
	spot_speed = cfg->spot_speed_maxc / 100;	//速度
	maxc = cfg->spot_speed_maxc % 100;			//最大圈数
	agitator_speed_pid_ctrl(0,0);
	log_printf("[motor_go_spot_task]angle=%3.1f,speed=%d,maxc=%d,step=%d,disa=%d\r\n\r\n",sys->angle
				,spot_speed,maxc,step,disa);
	m_speed_pid_indep(0,0,0);
	sta = sys->sState = SYS_FOCUS;
	log_printf("set agi speed=%d\r\n",speed);
	while(1)
	{
		get_sensers(&sys->gSta);			//
		proc_uart_task();
		if(sys->sState != sta || sys->sState == SYS_IDLE)			//
		{
			log_printf("[motor_go_spot_task],sta=%d\r\n",sys->sState);
//			navigat->is_walk = 0;
			break;
		}
		if(TIM5->CNT >=5000)
		{
			TIM5->CNT = 0;
			www_idleintel_com();
			if(disfloat(angle,sys->angle) > (float)disa)
			{
				cycle++;
				if(speed < spot_speed)
					speed+=step;
				log_printf("cycle = %d,set speed=%d\r\n",cycle,speed);
				if(cycle >=maxc)		//最大圈数
				{
					log_printf("ok\r\n");
					break;
				}
				log_printf("inc speed,angle=%3.1f,%3.1f,speed=%d\r\n",sys->angle,angle,speed);
				angle = sys->angle;
			}
			c_calc++;
			if(c_calc >=50)		//250ms
			{
				c_calc = 0;
				log_printf("...speed=%d\r\n",speed);
				m_speed_pid_indep(spot_speed,speed,1);
				agitator_speed_pid_ctrl(cfg->h_agi_speed,1);
				/*
				if(speed < cfg->spot_speed)
					speed+=cfg->spot_step;
				else
				{
					log_printf("speed ok\r\n");
					break;
				}
				*/
			}
		}
		if((sys->gSta & (MASK_BUM_LEFT | MASK_BUM_LEFT2| MASK_BUM_RIGHT| MASK_BUM_RIGHT2)) || MIDLE_IRDA() )		//碰撞了
		{
			uint16_t lpwm = L_FORWRK_PWM;
			uint16_t rpwm = R_FORWRK_PWM;

			sys->gsta = sys->gSta ;						//& MASK_BUM_MIDL;

			motor_run(GO_STOP,0,0,0);
			log_printf("[motor_go_spot_task]bum!gSta=%d(%d),ir=(%d,%d,%d)%d,dist=%d,dock=%d，adj=%d!!!\r\n",sys->gSta,sys->gsta,sys->g_sta[2] ,sys->g_sta[3] ,sys->g_sta[4],MIN_MIDLE_IDA,0,0);
		//	log_printf("lrdrop(%d,%d)\r\n",sys->g_buton[BOTOM_IDX][BOTOM_RIGHT_IDX],sys->g_buton[BOTOM_IDX][BOTOM_LEFT_IEX] ,sys->t_adj_run);
#if 0
			motor_run(GO_STOP,0,0,0);
			break;
#else
//			int out_dis = cycle*1000;	//退出的距离
			//motor_go_edgeways_for_spot(LEFT_SIDE_NEAR,out_dis);
			robot_turn_deg(GO_RIGTH,DEG_TURN_PWM,7);
			if(sys->sState != sta || sys->sState == SYS_IDLE)			//
			{
				log_printf("[motor_go_spot_task] sta=%d..edgeout\r\n",sys->sState);
			//	navigat->is_walk = 0;
				break;
			}
			motor.c_left_hw = motor.c_right_hw = 0;
			motor_wheel_forward(LEFT_WHEEL, lpwm);
			motor_wheel_forward(RIGHT_WHEEL, rpwm);
			m_speed_pid_indep(0,0,0);
#endif
		}
	}
	LED1_OFF();
	motor_run(GO_STOP,0,0,0);
	STOP_ALL_MOTOR();
	log_printf("[motor_go_spot_task]end\r\n\r\n");
	//sys->sState = SYS_IDLE;
	if(sys->full2go)
		sys->f_work_mode = MWO_FULL2GO;
	else
		sys->f_work_mode = sys->work_mod;		//集中清扫模式恢复
	sys->spot_flag = 0;
	return 0;
}



/*
独立的双轮PID
*/
void m_speed_pid_indep(uint16_t lspeed,uint16_t rspeed,uint8_t type)
{

//	static int  c =0;
	static int lhw1=0,rhw1=0;	//记录上一次左轮的霍尔值
	int hw1=0;////,hw2=0;
	int pid_pwm;
	static uint16_t count=0;


	if(type ==0)
	{
		lhw1 = motor.c_left_hw;  //记录上一次左轮的霍尔值
		rhw1 = motor.c_right_hw;  //记录上一次左轮的霍尔值
		count = 0;
		return ;
	}
	//micony 2020101 必须丢弃前边的几圈，要不然前边的几圈算得不存导致速度狂调
	if(count < 2)
	{
		count++;
		return ;
	}
/*

每250ms执行一次，每29霍尔一厘米,290速度，就250ms10cm，= 0.1/0.25 = 0.4m/s
*/
	//速度的上下限




		int MAX_SPEED = lspeed+6;
		int MIN_SPEED = lspeed-6;


		//计算出左轮的速度
		hw1 = motor.c_left_hw - lhw1;  //用本次的霍尔值 减去上次的霍尔值 就得到速度
		lhw1 = motor.c_left_hw ;		//记录本次的霍尔值

	//速度小于
		if(L_FORWRK_PWM > 200 && L_FORWRK_PWM < 900)
		{
			if(hw1 < MIN_SPEED)
			{
				pid_pwm = (hw1 - MIN_SPEED) / 2;
				if(pid_pwm > 200 )
				   pid_pwm = 200;
				 if(pid_pwm < -200 )
				   pid_pwm = -200;

					L_FORWRK_PWM +=pid_pwm;
					if(L_FORWRK_PWM < 200)
						L_FORWRK_PWM = 200;
					if(L_FORWRK_PWM > 999)
						L_FORWRK_PWM = 999;

					log_printf("lspeed(%d)+:%d,pid=%d,pwm=%d\r\n",sys->run_speed,hw1,pid_pwm,L_FORWRK_PWM);

			}else if(hw1 > MAX_SPEED)
			{
				pid_pwm = (hw1 - MAX_SPEED) / 2;
				if(pid_pwm > 200 )
				   pid_pwm = 200;
				 if(pid_pwm < -200 )
				   pid_pwm = -200;

					L_FORWRK_PWM +=pid_pwm;
					if(L_FORWRK_PWM < 200)
						L_FORWRK_PWM = 200;
					if(L_FORWRK_PWM > 999)
						L_FORWRK_PWM = 999;
					log_printf("lspeed(%d)-:%d,pid=%d,pwm=%d\r\n",sys->run_speed,hw1,pid_pwm,L_FORWRK_PWM);
			}
		}
		MAX_SPEED = rspeed+6;
		MIN_SPEED = rspeed-6;

		//计算出右轮的速度
		hw1 = motor.c_right_hw- rhw1;  //用本次的霍尔值 减去上次的霍尔值 就得到速度
		rhw1 = motor.c_right_hw ;		//记录本次的霍尔值
		sys->right_speed = hw1;

	//速度小于
		if(R_FORWRK_PWM > 200 && R_FORWRK_PWM < 999)
		{
			if(hw1 < MIN_SPEED)
			{
				pid_pwm = (hw1 - MIN_SPEED) / 2;
				if(pid_pwm > 200 )
				   pid_pwm = 200;
				 if(pid_pwm < -200 )
				   pid_pwm = -200;
				R_FORWRK_PWM +=pid_pwm;
				if(R_FORWRK_PWM < 200)
					R_FORWRK_PWM = 200;
				if(R_FORWRK_PWM > 999)
					R_FORWRK_PWM = 999;
				log_printf("rspeed+:%d,pid=%d,pwm=%d\r\n",hw1,pid_pwm,R_FORWRK_PWM);
			}else if(hw1 > MAX_SPEED)
			{
				pid_pwm = (hw1 - MAX_SPEED) / 2;
				if(pid_pwm > 200 )
				   pid_pwm = 200;
				 if(pid_pwm < -200 )
				   pid_pwm = -200;
				R_FORWRK_PWM +=pid_pwm;

				if(R_FORWRK_PWM < 200)
					R_FORWRK_PWM = 200;
				if(R_FORWRK_PWM > 999)
					R_FORWRK_PWM = 999;
				log_printf("rspeed-:%d,pid=%d,pwm=%d\r\n",hw1,pid_pwm,R_FORWRK_PWM);
			}
		}
		//计算两轮都的pwm的中心值 这是给角度环用
		//sys->pwm = (R_FORWRK_PWM+L_FORWRK_PWM) >> 1;
		//log_printf("pwm=%d,%d,%d\r\n",L_FORWRK_PWM,R_FORWRK_PWM,sys->pwm);




}
#endif

//沿墙的比较醋的PID
/*
void near_wall_pid(void)
{
	static int c_pid=0;
	int t_slip=0,c_slip=0;
	char pid=0;
	if(c_pid++ >=10)
	{
		c_pid = 0;
		www_idleintel_com();
		if(sys->g_sta[6] <= 142)		//靠近右边了
		{
			r_motor_set_pwm(GO_FORWARD, 800);
			l_motor_set_pwm(GO_FORWARD, 600);
			motor.c_left_hw=0;
			pid = 1;
			log_printf("near left,%d\r\n",sys->g_sta[6]);
		}else if(sys->g_sta[6] >167)
		{
			r_motor_set_pwm(GO_FORWARD, 600);
			l_motor_set_pwm(GO_FORWARD, 800);
			motor.c_left_hw=0;
			pid = 1;
			log_printf("lost wall,%d",sys->g_sta[6]);
		}
		if(pid)
		{
				while(1)
				{
					if(TIM5->CNT >=2000)
					{
						www_idleintel_com();
						if(motor.c_left_hw > 100)
						{
							log_printf("near_wall_pid,hw=%d,(%d,%d,%d,%d,%d,%d,%d)\r\n",motor.c_left_hw,sys->g_sta[0],sys->g_sta[1]
														,sys->g_sta[2],sys->g_sta[3],sys->g_sta[4],sys->g_sta[5],sys->g_sta[6]);
							break;
						}
						if(t_slip ++ > 800)
						{
							if(c_slip != navigat->distance)
								c_slip = navigat->distance;
							else
							{
								log_printf("slip..\r\n");
							//	g_rand_clean.slip = 1;
								motor_run(GO_STOP,700,0,0);
								motor_back_off(10);
								break;
							}
							t_slip = 0;
						}
					}
				}
				//motor_run(GO_STOP, 0,0,0);
				//while(1);
				pid_init(GO_FORWARD_PWM);
				illegal_copy_tell_13757122544_gsend(0x01,0,0);
				illegal_copy_tell_13757122544_gsend(0x01,0,0);
				navigat->distance = 0;
				lpid.out = 0;
				motor_run(GO_FORWARD, GO_FORWARD_PWM, 0, 0);
		}
	}
}

*/
/*
void proc_test_task(void)
{
	static int tmr = 0,c=0;
	static float agle=0;
	int i;

	//左边碰撞
	if(sys->gSta & MASK_BUM_MIDL)
	{
		www_idleintel_com();

		c++;

		log_printf("bum %f,(%d,%d,%d,%d,%d,%d,%d)\r\n",sys->angle,sys->g_sta[0],sys->g_sta[1],sys->g_sta[2],sys->g_sta[3],sys->g_sta[4],sys->g_sta[5],sys->g_sta[6]);
		motor_run(GO_STOP, 0, 0, 0);
		delay_ms(100);

		navigat->distance = 0;


		motor_back_off(10);
		turn_to_deg(185);

		for(i=0;i<2;i++)
		{
			illegal_copy_tell_13757122544_gsend(0x01,0,0);
			delay_ms(10);
		}

		www_idleintel_com();
		navigat->angle = 0;


		pid_init(GO_FORWARD_PWM);
		www_idleintel_com();
		illegal_copy_tell_13757122544_gsend(0x01,0,0);
		navigat->distance = 0;
		navigat->angle = 0;
		TIM5->CNT = 0;
		motor_run(GO_FORWARD, GO_FORWARD_PWM, 0, 0);
		sys->c_lost_wall_right	= 0;
		sys->c_near_wall_right 	= 0;
		sys->c_lost_wall_left	= 0;
		sys->c_near_wall_left 	= 0;
		 tmr = 0;
		 agle = 0;

	}else
	{
		if(TIM5->CNT >=2000)
		{

			//每隔10ms取一下数据
			if(tmr% 5 ==0)
			{
				www_idleintel_com();
				//if(c==0)
					log_printf("%f,(%d,%d,%d,%d,%d,%d,%d)\r\n",sys->angle,sys->g_sta[0],sys->g_sta[1],sys->g_sta[2],sys->g_sta[3],sys->g_sta[4],sys->g_sta[5],sys->g_sta[6]);
			}
			if(tmr++ >=15) //30ms 计算一次
			{
				proc_line_pid(agle);
				near_wall_pid2(&agle,20);
				tmr = 0;

			}

		}
	}
}
void near_wall_pid2(float *agle,int c_lost)
{
	//static int c_pid=0;

	//sys->near_pid = 0;		//可能有隐患
	//if(c_pid ++ >=2)
	{
	//	c_pid = 0;
		//右边PID
		if(sys->g_sta[5] < cfg->have_irda|| sys->g_sta[6] < cfg->have_irda  ) //3100
		{
			//计算贴墙的时间
			if(sys->g_sta[5] < 2800 || sys->g_sta[6] < 2800  )
			{
				if(sys->c_near_wall_right < 10)
					sys->c_near_wall_right ++;
				//持续贴墙后，才能重置离墙次数
				if(sys->c_near_wall_right >=5)
					sys->c_lost_wall_right = c_lost;
			}
			//太靠近右边了，往外拐点
			if(sys->g_sta[5] < 2900 || sys->g_sta[6] <=2350)
			{
				//if(*agle < 10)
				{
					log_printf("rp+:%f,%d,%d\r\n",*agle,sys->g_sta[5],sys->g_sta[6]);
					*agle+=0.4;
			//		sys->near_pid = 1;
				}
			//消失了，往里拐点，
			}else if(sys->g_sta[6] >= 2900)
			{
				//if(*agle >-10)
				{
					log_printf("rp-:%f,%d,%d\r\n",*agle,sys->g_sta[5],sys->g_sta[6]);
					*agle-=1;
				//	sys->near_pid = 1;
				}
			}
		} else
		{
			sys->c_near_wall_right=0;		//不再贴墙
			//消失后，还持续拐一段时间，确保不消失
			if(sys->c_lost_wall_right)
			{
				//if(*agle >-10)
				{
					log_printf("p2-:%f,%d,%d\r\n",*agle,sys->g_sta[5],sys->g_sta[6]);
					*agle-=1;
				//	sys->near_pid = 1;
				}
				sys->c_lost_wall_right--;

			}else 	//左边PID,只有不做右边PID，才能做左边的PID
			{

				if(sys->g_sta[0] < cfg->have_irda  || sys->g_sta[1] < cfg->have_irda   )
				{
					//计算贴墙的时间
					if(sys->g_sta[0] < 2800 || sys->g_sta[1] < 2800  )
					{
						if(sys->c_near_wall_left < 10)
							sys->c_near_wall_left ++;
						//持续贴墙后，才能重置离墙次数
						if(sys->c_near_wall_left >=8)
							sys->c_lost_wall_left = c_lost;
					}

					if(sys->g_sta[1] < 2900 || sys->g_sta[0] <=2350)	//向左斜了
					{
						//if(*agle < 10)
						{
							log_printf("lp+:%f,%d,%d\r\n",*agle,sys->g_sta[0],sys->g_sta[1]);
							*agle +=0.4;
							//sys->near_pid = 1;
						}
					}else if(sys->g_sta[0] >= 2900)	//消失了
					{
						//if(*agle >-10)
						{
							log_printf("lp-:%f,%d,%d\r\n",*agle,sys->g_sta[0],sys->g_sta[1]);
							*agle -=1;
						//	sys->near_pid = 1;
						}
					}
				} else
				{
					sys->c_near_wall_left=0;		//不再贴墙

					if(sys->c_lost_wall_left)
					{
						//if(*agle >-10)
						{
							log_printf("lp2-:%f,%d,%d\r\n",*agle,sys->g_sta[0],sys->g_sta[1]);
							*agle -=1;
							//sys->near_pid = 1;
						}
						sys->c_lost_wall_left--;

					}
				}


			}
		}
	}
}
*/
