
#include "sys.h"




/*
	返回
	0-非正面碰撞
	1-成功
	2-正面碰撞

*/

/************************************************************************************************
 * 函数名称 : motor_go_forwark
 * 入    参 : hw     :  要行走的霍尔数，左轮计数，31个霍尔一个mm，
 *			  nsta   :  是否强制延边。参考LEFT_SIDE_NEAR 等定义
 *			  m_go_check : 判断函数，如果想在里边做些判断，可传入判断函数，如果没有则传NULL
 * 返 回 值 : 0 - 碰撞，或状态错误退出
 *			  1 - 走完霍尔数退出
 *			  sys->g_sta[] 七路前撞红外的ADC差值，用于判断碰撞，排序是0 左边 - > 6 右边
 * 说    明 :
 *				1.函数根据angle传入的角度做角度PID直行
 *				2.根据 nsta 决定是否强制延边，如果不强制延边，发现墙边的时候，会自动延边
 *				3.根据 hw决定走的行程，用左轮计算，0则无限行走，知道碰撞
 *				4.函数阻塞运行，但检测串口和传感器，红外指令，如果串口或红外遥控有指令下来，会改变
 *				  系统状态 sys->sState  ，如果发现系统状态改变，或回到空闲状态，则退出
 *************************************************************************************************/

char motor_go_forwark(int hw,uint8_t nsta,m_go_func m_go_check)
{
	static short c_calc = 0;
//	static short scan_x = 0,scan_y=0;
	int ret =0;
//	uint8_t clr = 0;		//进入清扫区域
//	int y_dir,side_s=0;
	uint8_t  sta = sys->sState ;
	uint32_t t_walk;
	uint8_t dock_bum = 0;
	SIDEOBST *sobst;
//	uint16_t kldist=0;
//	int16_t bx,by;
#if PRINTF_PWR_STA
//	uint8_t c_power=0;
#endif
if(sys->motor_power_off ==1)
 		MOTOR_POWER_ON();
#if !CALE_BY_BKHW
	float bx,by;	//记录开始走的时候的坐标
	int slip=0;		//打滑
	int c_front_stop=0;
#endif
//	short bx = X_NOW,by=Y_NOW;

	int kk=0;
#if ARC_Z_GO
	short xdir =  navigat->angle== 0?1:-1;
	int xorg,dist=0;
#if ADJ_END
	check_front_target(xdir,&xorg);				//取到该停止X原始坐标
#endif
	//计算出距离。
	if(xorg !=0)
	{
		dist = xorg > navigat->x_org?(xorg - navigat->x_org):(navigat->x_org - xorg);
	}
#endif
/*
#if ARC_Z_GO
#if ADJ_END
	TOBST *tost =NULL;
	short xdir =  navigat->lst_angle == 0?1:-1;
	if(navigat->j_obst > 0)
	{
		tost = &navigat->adj_obst[navigat->j_obst-1];
	}
#endif
#endif
*/
	//记录侧面的障碍物
	sobst = (SIDEOBST *)m_stack;
	sobst->len = 0;
	navigat->walk_len = 0;
	insert_walk_fifo(navigat->tx,navigat->ty);
	navigat->is_cross_obst = 0;

 	CHECK_NAVI_STA_RT(0);
	pd_gyro_int(GO_FORWARD_PWM);

	www_idleintel_com();  //取一下角度   navigat->angle;
	motor.c_left_hw = motor.c_right_hw = motor.c_front_hw = 0;
	//motor.c_left_hw = motor.total_left_hw = motor.total_right_hw=0;

	navigat->side_obst = 0;
	navigat->c_side_obst = 0;
	sys->slip =0;

	sys->c_ajust_agle = 0;
	sys->total_ajust_agle = 0;
	navigat->out = navigat->angle;
	navigat->adj_run_ret = FALSE;

	navigat->bum_side = 0;
	navigat->walk_dis = 0;
	navigat->is_bum = 0;
	//sys->c_near_wall_left = sys->c_near_wall_right =0;

	navigat->distance = sys->c_pid_hw = 0;
	sys->gsta= 0;

	TIM5->CNT = 0;
	init_near_wall_navi(nsta); //NO_SIDE_NEAR
	navigat->c_go++;
	navigat->c_go_force++;
	//重新映射一下坐标
	cord_calc_store(0);
	gyro_whlmap();

	init_doci_ir();					//充电桩

	log_printf("[motor_go_fw](%d,%d,%3.1f,%3.1f)pwm=%d,ld=%d,nsta=%d,ofs=%3.3f\r\n",navigat->tx,navigat->ty,sys->angle,navigat->angle,sys->pwm,0,nsta,0);
	calc_timr_upload();
	//记录下最后的坐标
	navigat->l_dist = navigat->distance;
	sys->c_front_stop = 0;		//前轮停止的次数清零
//	c_ird = 0;
	t_walk = msTmr;
#if JUDGE_PID_CTRL_WHEEL_STOP
	m_speed_pid_ctrl(0,0,0,0,0,0,0);		//记录左轮霍尔数
#else
	m_speed_pid_ctrl(0,0);		//记录左轮霍尔数
#endif
	//caluc_bum_by_front(0);
	robot_whele_stop(0);
	//delay_ms_sensers(10);		//延迟一下，采一下红外
	delay_ms_sensers(5);
/*
#if ARC_Z_GO
	xx = X_NOW;
	for(i=0;i<70;i++)
	{
		if(OBST_XY(xx + xdir*2,Y_NOW))
		{
			log_printf("found obst=(%d,%d,),stop point=(%d,%d,)\r\n",xx + xdir*2,Y_NOW,xx,Y_NOW);
			break;
		}
		xx +=xdir;
	}
	if(dis_xy(xx,X_NOW) >=69)
	{
		xx = 0;
		log_printf("not found obst\r\n");
	}
#endif
*/
	motor.c_left_hw = motor.c_right_hw = 0;

#if 0
	motor.c_left_hw = motor.c_right_hw = 0;
	motor_run(GO_BACK, 600, 0, 0);
	while(1)
	{
		log_printf("%d,%d\r\n",motor.c_left_hw, motor.c_right_hw);
		motor.c_left_hw = motor.c_right_hw = 0;
		delay_ms(300);
		proc_uart_task();
		if(sys->sState != sta || sys->sState == SYS_IDLE)			//
		{
			log_printf("SYS_IDLE2,sta=%d\r\n",sys->sState);
			navigat->is_walk = 0;
			ret = 0;
			return 0;
		}


	}
#endif

	motor_run(GO_FORWARD, GO_FORWARD_PWM, 0, 0);
	//delay_ms(2);
	navigat->is_walk = 1;
	c_calc = 40;
#if TEST_FOR_LABO
	navigat->c_side_obst = 0;
#endif
//	l_dist = 0;
	sys->navi_run = 1;
	sys->walk_bum = 0;
	navigat->c_dock_m = navigat->c_dock_l = navigat->c_dock_r = 0;	//发现充电座计数清零
	printf_power_sta();
//	log_printf("SYS sta=%d,dist=%d\r\n",sys->sState,dist);
	sys->max_front_stop = 18;
	navigat->dis_angle_adj = 100;
	sys->g_t_walk = 0;
	log_printf("[motor_go_fw]1\r\n");
	log_printf("[motor_go_forwark]OO!side %d\r\n",navigat->side);
	sys->t_adj_run=0;
//	static uint16_t c_still_go=0;
	while(1)
	{
		get_sensers(&sys->gSta);			//
		proc_uart_task();
		if(sys->sState != sta || sys->sState == SYS_IDLE)			//
		{
			log_printf("SYS_IDLE2,sta=%d\r\n",sys->sState);
			navigat->is_walk = 0;
			ret = 0;
			break ;
		}


		if(TIM5->CNT >=10000)
		{
			TIM5->CNT = 0;
			kk++;


			//===================================减速=========================================

			if((sys->pwm != FW_SLOW_PWM) )			{
				if((SLOW_IRD_MIDL()) ||
				   (hw > 200 && WALK_DIST() > (hw-200)))
				{
					log_printf("slow(%d,%d,%d,%d,%d,%d,%d)\r\n",
						sys->g_sta[0],sys->g_sta[1],sys->g_sta[2],sys->g_sta[3],sys->g_sta[4],sys->g_sta[5],sys->g_sta[6]);

					motor_run(GO_FORWARD, FW_SLOW_PWM, 0, 0);
					sys->pwm = FW_SLOW_PWM;
				}
			}
              if(hw !=GO_FOR_DOCK_HW)
			dock_bum = proc_doc_ir();
			//======================行程走完==========================================
			if(WALK_DIST() > (hw-5) && hw >0)	//避免擦边
			{
				motor_run(GO_STOP,0,0,0);
				ret = 1;
				break;
			}


				//==========================PID===============================
				navigat->out = format_agle(navigat->out,ANGLE_360);

				proc_line_pid(navigat->out);
			//计算坐标，用于记录走过的地方
			//一次大概20个霍尔左右
#if CALE_BY_FRON
				if(coordinate_calcu(1) ==0 )													//计算出原始的坐标系
				{
					log_printf("whele err,stop\r\n");
					//左右轮打滑
					motor_run(GO_STOP,0,0,0);
					ret = 0;
					break;
				}
#endif
			c_calc++;
			if(c_calc >=25)		//250ms
			{
				c_calc = 0;

				//log_printf("IM=%d,%d,%d\r\n",I_MID_ADC(),I_MOTER_LEFT(),I_MOTER_RIGHT());
				if(motor.c_left_hw > 10 &&
					!(hw > 10 && WALK_DIST() > (hw-50))) 	//提前减速
					m_speed_pid_ctrl(0,1);


#if !CALE_BY_FRON

				if(coordinate_calcu(1) ==0 )													//计算出原始的坐标系
				{

				}
#endif
				//laser_move_adj(navigat,1);
				coord_org2map(navigat->x_org,navigat->y_org,&navigat->tx,&navigat->ty);	//坐标系转换




				//printf_power_sta();

				log_printf("*(%d,%d,%3.1f,0)[%d,%d]*<sp=%d,%d,i=%d,%d,%d>\r\n",navigat->tx,navigat->ty,sys->angle,L_FORWRK_PWM,R_FORWRK_PWM,
						sys->left_speed,sys->right_speed,I_MID_ADC(),sys->left_curr,sys->right_curr);


			}
			//执行以下入参函数
			if(m_go_check !=NULL)
			{
				if(m_go_check()==0)
				{
					log_printf("m_go_check ok\r\n");
					break;
				}
			}
		//	log_printf("E");
			//if(rrett)
			//	log_printf("bum=%d\r\n",sys->gSta);
		}
#if CALE_BY_BKHW
	#define IR_DIST		100
#else
	#define IR_DIST		20
#endif
		//wy190527 增加左右两侧的碰撞开关检测
		//	log_printf("[motor_go_fw]2\r\n");
		if(disxy(motor.c_left_hw,motor.c_right_hw) > 1300 ||
					motor.c_left_hw > 15462 || motor.c_right_hw > 15462)
		{
			log_printf("left=%d,right=%d,dis over\r\n",motor.c_left_hw,motor.c_right_hw);
			motor_run(GO_STOP,0,0,0);
			break;
		}

		if((sys->gSta & (MASK_BUM_LEFT | MASK_BUM_LEFT2| MASK_BUM_RIGHT| MASK_BUM_RIGHT2)) || MIDLE_IRDA()   ||
			dock_bum /*|| front_bum || (l_dist > MIN_LASER_DIST && l_dist <=8 && WALK_DIST() > 500)*/)		//碰撞了
		{
		//	log_printf("[motor_go_fw]3\r\n");

			sys->gsta = sys->gSta ;						//& MASK_BUM_MIDL;
			sys->walk_bum = sys->bum_sta;				//记录碰撞的状态



#if BUM_LINE_ADJ
			sys->kdis_yaw++;
			log_printf("[motor_go_forwark]bum_sta=%d,walk_bum=%d,kdis=%d\r\n", sys->bum_sta,sys->walk_bum,sys->kdis_yaw);
			if(sys->g_sta[LEFT_FRONT_IR] >= 1000 && sys->g_sta[RIGHT_FRONT_IR] >= 1000/* && sys->kdis_yaw > 10*/)
			{
				motor_run(GO_FORWARD,700,0,0);
				L_FORWRK_PWM = R_FORWRK_PWM = 700;
				delay_ms(500);
				motor_run(GO_STOP,0,0,0);
				www_idleintel_com();

				log_printf("mid bum,go now ir=%d,%d,angle=%3.1f,kdis=%d,cdis=%d\r\n",sys->g_sta[LEFT_FRONT_IR] ,sys->g_sta[RIGHT_FRONT_IR],sys->angle,sys->kdis_yaw,sys->cdis_yaw);

				if(sys->g_sta[LEFT_FRONT_IR] >= 1500 && sys->g_sta[RIGHT_FRONT_IR] >= 1500 )
				{
					float dis = disfloat(sys->angle, navigat->angle);
					log_printf("get yaw dis=%3.1f,c=%d,%d\r\n",dis,sys->kdis_yaw,sys->cdis_yaw);

					if(dis < 20)
					{
						log_printf("set yaw=%3.1f\r\n",navigat->angle);
						sys->yaw = sys->angle = navigat->angle;
						/*
						if(sys->cdis_yaw < 3)
						{
							sys->dis_yaw[sys->cdis_yaw++] = dis;
							log_printf("cdis_yaw2=%d\r\n",sys->cdis_yaw);
						}
						if(sys->cdis_yaw >= 3)
						{
							log_printf("disfifo=%3.1f,%3.1f,%3.1f\r\n",sys->dis_yaw[0],sys->dis_yaw[1],sys->dis_yaw[2]);
							if(disfloat(sys->dis_yaw[0],sys->dis_yaw[1]) < 1 && disfloat(sys->dis_yaw[0],sys->dis_yaw[2]) < 1)
							{
								illegal_copy_tell_13757122544_gsend(0x02,navigat->angle,100);
								log_printf("kadj_a:%3.1f,%3.1f,",sys->angle,navigat->angle);
								delay_ms(10);
								www_idleintel_com();
								log_printf("new=%3.1f\r\n",sys->angle);
							}
							sys->cdis_yaw = sys->kdis_yaw = 0;
						}
						*/
					}else
						log_printf("dis err=%3.1f\r\n",dis);
				}else
					sys->kdis_yaw = 5;

			}

#endif
			if(MIDLE_IRDA() &&  WALK_DIST() > IR_DIST)
				sys->gsta |= MASK_FW_MIDL;				//中间的红外



			//log_printf("[go_forw]bum!gSta=%d(%d),ir=(%d,%d,%d)!!",sys->gSta,sys->gsta,sys->g_sta[2] ,sys->g_sta[3] ,sys->g_sta[4]);
			log_printf("[go_forw]bum!gSta=%d(%d),ir=(%d,%d,%d)%d,dist=%d,dock=%d!!!\r\n",sys->gSta,sys->gsta,sys->g_sta[2] ,sys->g_sta[3] ,sys->g_sta[4],MIN_MIDLE_IDA,0,dock_bum);

			motor_run(GO_STOP,0,0,0);

			//wy190527 新增左右两侧碰撞开关检测
			if(sys->gsta & (MASK_BUM_LEFT | MASK_BUM_LEFT2 | MASK_BUM_RIGHT | MASK_BUM_RIGHT2) || dock_bum || MIDLE_IRDA())  				//非红外的左右碰撞,需要后退一些
			{
				int distl = 0;
				navigat->is_bum = 1;

				//delay_ms_sensers(100);
				motor_run(GO_STOP,0,0,0);
				//delay_ms(50);
				delay_sensor(50);
#define BACK_HW3	8

				 if(sys->fall_sta || dock_bum)
				 {//跌落
					distl = BACK_HW * 5;
					sys->c_walk_drop++;
				}
				else
				{
					distl = BACK_HW *3;
					/*
					if(sys->c_walk_drop  > 0)
						sys->c_walk_drop--;
					*/
				}


				//motor_back_off(distl);//miconydrawmap
				motor_back_off(distl);


			}
			navigat->is_walk =0;


			//不是侧面碰撞，才置障碍物 导航也要置障碍物，否则，还是有问题，不过不是强制障碍物。
			//if(navigat->bum_side ==NO_SIDE)		//导航的时候不设置障碍物

			ret = 0;
			break;
		}
	}



//	save_map(0,1,0);
	if(navigat->arc_z_go != 1)	///	陀螺仪版本,遇到已经沿过的边，不停电机
	    motor_run(GO_STOP,0,0,0);
	navigat->is_walk = 0;

	coordinate_calcu(0);
	if(motor.c_left_hw < 3000)
		navigat->c_target_go++;
	else
		navigat->c_target_go = 0;
	if(motor.c_left_hw > 8000 && sys->c_walk_drop >0) // 3000 左右是一米
	{
		log_printf("reset cdrop=%d\r\n",sys->c_walk_drop);
		sys->c_walk_drop = 0;
	}

	log_printf("\r\ngo_forwark ok,dis=%d,dis1=%d,left=%d,right=%d,ldis=%d,tmr=%d,xy(%d,%d,)org(%d,%d)%d,dock=%d,%d,%d,target_go=%d\r\n",
		hw,navigat->distance,motor.c_left_hw,motor.c_right_hw,0,t_walk,
		navigat->tx,navigat->ty,navigat->x_org,navigat->y_org,sys->c_front_stop,
		navigat->c_dock_l,navigat->c_dock_m,navigat->c_dock_r,navigat->c_target_go);






	printf_power_sta();
;
	return ret;
}


/*****************************************************************************
 * 函数名称:
 * 入    参:	type  :
 *						 GO_NEAR_TYPE_NO		0x00		//普通延边
 *						 GO_NEAR_TYPE_NAVI		0x01		//中间如果能导航到终点，则退出去导航
 *						 GO_NEAR_TYPE_ADJ	   0x02
 *函数功能:	延边导航，找地方矫正，如果到目的地，则退出
 *
 *****************************************************************************/
char motor_go_edgeways(uint8_t n_sta ,short tox,short toy,uint8_t type,uint8_t is_save)
{
//	int xx=0,yy=0,x1=0,y1=0;
	int calue=0;
//	int by = navigat->y_org;f
//	uint8_t gSta;
	uint8_t sta;
//	int	lx=0,ly=0;
	short llx=0,lly=0;
//	short lx=X_NOW,ly=Y_NOW;
//	int bx = X_NOW;
	short by = Y_NOW;
//	short bx = X_NOW;
//	short nx1,ny1;
	float lagle;		//记录最后的延边角度，用于计算是否转弯过多
//	uint16_t	gSta;
	uint32_t	t_begin = sys->t_navi_work;
	int ret_calc=1;
	////uint16_t c_dock_data=0;
	int16_t c_lost=0;
	uint16_t c_drop=0;
	///uint16_t c_round=0;		//转圈的次数
	uint8_t dock_bum = 0;


	short x_begin_line=0,y_begin_line = 0;		//一条线开始的X，Y的坐标


//	u8 irData_bak[6];

	NEAR_WALL *near = &navigat->near;
	float m_angle[MAX_C_M_ANGLE];			//保存最近的20个碰撞的角度
	int16_t c_m_angle=0;
if(sys->motor_power_off ==1)
 		MOTOR_POWER_ON();

#if LAGER_ROUND_MODE
	int16_t cyc_x[MAX_C_M_ANGLE],cyc_y[MAX_C_M_ANGLE];
	uint8_t tmp=0;
#else
	int16_t c_near_wall = 0;
#endif

	for(c_m_angle = 0;c_m_angle<MAX_C_M_ANGLE;c_m_angle++)
		m_angle[c_m_angle] = 0;
	c_m_angle = 0;
	CHECK_NAVI_STA_RT(0);
	pd_gyro_int(GO_NEAR_PWM_FAST);
	navigat->out =navigat->angle;
	navigat->distance = 0;
	motor.c_left_hw = motor.c_right_hw = 0;
	cord_calc_store(0);
	gyro_whlmap();
	robot_whele_stop(0);
	motor_run(GO_FORWARD, GO_NEAR_PWM_FAST, 0, 0);
	if(n_sta & 0x80)
		init_near_wall_navi(n_sta & 0x7F);
	else
		init_near_wall_navi(NO_SIDE_NEAR);
	n_sta &=0x7F;
	navigat->is_walk =1;

	proc_dock_near_bum(0);
	log_printf("[motor_go_edgeways]sta=%d(%d),type=%d,is_save=%d,now=(%d,%d,)to=(%d,%d,)\r\n",navigat->near.n_sta,n_sta,type,is_save,X_NOW,Y_NOW,tox,toy);
	//navigat->wheel_dir = 1;
	sta = sys->sState;
	lagle = sys->angle;
	//mstimeout(&t_begin,0);
	printf_power_sta();
	sys->g_t_walk = 0;
	//micony20191022
	m_speed_pid_ctrl(0,0);
//单轮PID初始化
	single_whele_pid(0,motor.c_left_hw,left_pwm);
	single_whele_pid(0,motor.c_right_hw,left_pwm);
	while(1)
	{

		proc_uart_task();
		get_sensers(&sys->gSta);
		if(sys->sState !=sta)
			return RET_NEAR_ERROR;
			////////jzz 2018-05-03
/*
		if(sys->mState == GO_STOP)
		{
			motor_run(GO_FORWARD, GO_NEAR_PWM_FAST, 0, 0);//有BUG，会多跑
		}
*/
		if(disxy(motor.c_left_hw,motor.c_right_hw) > 2000 ||
					motor.c_left_hw > 10462 || motor.c_right_hw > 10462)
		{
			log_printf("left=%d,right=%d,dis over\r\n",motor.c_left_hw,motor.c_right_hw);
			motor_run(GO_STOP,0,0,0);
			sys->gSta = MASK_BUM_MIDL;
			//return 0;
		}
		//CHECK_NAVI_STA_RT(0);
		//碰撞
		if(sys->gSta & (MASK_BUM_MIDL | MASK_BUM_LEFT | MASK_BUM_LEFT2 | MASK_BUM_RIGHT | MASK_BUM_RIGHT2) ||/*ret_calc==0 ||*/
		             c_lost >=MAX_NEAR_LOST || MIDLE_IRDA() || dock_bum)
		{
			//gSta = sys->gSta;	//记录碰撞的状态vv     2019  10 21
            sys->gsta = sys->gSta ;
			motor_run(GO_STOP,0,0,0);
			log_printf("BUM,time%d,%d\r\n\r\n",sys->t_navi_work , t_begin);
			//delay_ms(200);
			//	if(sys->gSta & (MASK_BUM_LEFT | MASK_BUM_RIGHT | MASK_BUM_LEFT2  | MASK_BUM_RIGHT2 ))
			{
				if(sys->fall_sta)		//跌落，则
				{
					motor_back_off(BACK_HW*5);
					c_drop++;
					sys->c_walk_drop++;
				}
				else
				{
					motor_back_off(BACK_HW);
					/*
					if(sys->c_walk_drop > 0)
						sys->c_walk_drop--;
						*/
				}
					log_printf("BUM1\r\n");
				if(c_drop >= 3)
					return 0;

			}
	#if 1     //20200511 影响沿边关闭
			if(type==0)
			{
				if((sys->t_navi_work - t_begin) >=120)
				{
					log_printf("timeout13\r\n");
					return 0;
				}
			}
	#endif
			/*if(sys->gSta & (MASK_BUM_MIDL))
			{
				if(sys->fall_sta)		//跌落，则
					motor_back_off(BACK_HW*8);
				else
					motor_back_off(BACK_HW);

			}*/
			//delay_ms_sensers(200);
			coordinate_calcu(0);														//计算出原始的坐标系
			coord_org2map(navigat->x_org,navigat->y_org,&navigat->tx,&navigat->ty); //坐标系转换
			motor_run(GO_STOP,0,0,0);
			log_printf("\r\n-----go_edgeways bum(%d,%d,%d,%f,%f),gsta=%d,irda=(%d,%d,%d,%d,%d,%d,%d)angle=%3.1f,ret=%d,lost=%d,%d\r\n",navigat->tx,navigat->ty,motor.c_left_hw,navigat->x_org_f,navigat->y_org_f,
							sys->gSta,sys->g_sta[0],sys->g_sta[1],sys->g_sta[2],sys->g_sta[3],sys->g_sta[4],sys->g_sta[5],sys->g_sta[6],sys->angle,ret_calc,c_lost,c_m_angle);

			if(ret_calc==0)
					gyro_mapwhl();

#if !LAGER_ROUND_MODE
			if(c_m_angle >=MAX_C_M_ANGLE)
				c_m_angle = 0;
			m_angle[c_m_angle++] = sys->angle;		//保持角度

			if(near_round_360(m_angle,c_m_angle))
			{
				log_printf("found round...go line...\r\n");

				init_near_wall_navi(NO_SIDE_NEAR);
				for(c_m_angle = 0;c_m_angle<MAX_C_M_ANGLE;c_m_angle++)
					m_angle[c_m_angle] = 0;
				c_m_angle = 0;
				goto l_mgo_edeways;
			}

#else			//2018-07-12 jzz  必须有更新坐标操作  已测试ok
			if(near_large_round_360(cyc_x,cyc_y,m_angle,c_m_angle))						///绕大桩判断函数
			{
				log_printf("found large round...go line...\r\n");
				if(near->n_sta == RIGHT_SIDE_NEAR)
				{
					robot_turn_deg(GO_LEFT,DEG_TURN_PWM,135);
					//goto l_mgo_edeways;
				}else if(near->n_sta == LEFT_SIDE_NEAR)
				{
					robot_turn_deg(GO_RIGTH,DEG_TURN_PWM,135);
					//goto l_mgo_edeways;
				}
				for(tmp=0;tmp<c_m_angle;tmp++)
				{
					log_printf("%02d(%03d,%03d,%5.3f)\r\n",tmp,cyc_x[tmp],cyc_y[tmp],m_angle[tmp]);
				}
				init_near_wall_navi(NO_SIDE_NEAR);
				for(c_m_angle = 0;c_m_angle<MAX_C_M_ANGLE;c_m_angle++)
					m_angle[c_m_angle] = 0;
				c_m_angle = 0;
				goto l_mgo_edeways;
			}
#endif
			/*if(!(ret_calc==0 || c_lost >=MAX_NEAR_LOST))
				c_round = 0;*/
			if(ret_calc==0)
					gyro_mapwhl();
//			gSta = sys->gSta;
			//motor_run(GO_STOP,0,0,0);

			//www_idleintel_com();
			lagle = sys->angle;
			//init_near_wall_navi(n_sta);		//碰撞后，才开始一直沿边

			navigat->distance = 0;
			motor.c_left_hw = 0;

			if( type == GO_NEAR_TYPE_DOCK)
			{
				//找充电桩
				////read_ir_data_timeout(irData_bak,0);

				if(detect_dock())						//发现充电桩，则退出
					goto l_go_out_for_dock;
/*
				if(irData_bak[IR_L_PIN_NUM] ||  irData_bak[IR_ML_PIN_NUM]||   irData_bak[IR_R_PIN_NUM] || ir_dock_insight(1))
				{
					//int16_t i;
					//for(i=0;i<3;i++)
					{
						//delay_ms(80);
						//read_ir_data(irData_bak,0);
					//log_printf("[motor_go_edgeways]bum  dock(%d,%d,%d)%d\r\n",
					//	irData_bak[IR_L_PIN_NUM] ,irData_bak[IR_ML_PIN_NUM],irData_bak[IR_R_PIN_NUM],c_dock_data);
						//if(irData_bak[IR_L_PIN_NUM] ||  irData_bak[IR_ML_PIN_NUM] || irData_bak[IR_R_PIN_NUM])
						if(1)
						{
							c_dock_data++;
							log_printf("ir=%d,%d,%d,c=%d\r\n",irData_bak[IR_L_PIN_NUM] , irData_bak[IR_ML_PIN_NUM] , irData_bak[IR_R_PIN_NUM],c_dock_data);
							if(c_dock_data >=3)
							{
								log_printf("found dock..\r\n");
								goto l_go_out_for_dock;
							}
						}else
							c_dock_data=0;
					}

				}else
					c_dock_data = 0;
*/
			}
			/*
			motor_back_off(5);
			delay_ms_sensers(200);
			coordinate_calcu();														//计算出原始的坐标系

			coord_org2map(navigat->x_org,navigat->y_org,&navigat->tx,&navigat->ty);	//坐标系转换
			log_printf("(%d,%d,%d,%3.3f,%3.3f,%3.3f)\r\n",navigat->tx,navigat->ty,motor.c_left_hw,navigat->x_org_f,navigat->y_org_f,sys->angle);
			*/


			/**************************************************************************
				如果tox，toy有设置了置，则到点了，就停下来。
			*****************************************************************************/
			if(  (((X_NOW == tox || tox ==0) && (Y_NOW == toy || toy==0)) && tox && toy) || 	//到点了
				(type == GO_NEAR_TYPE_ADJ &&  ((by > toy && Y_NOW < toy) || (by < toy && Y_NOW > toy)))) //adj回去，Y轴超过了
			{
				log_printf("xy ok(%d,%d,%3.1f)\r\n",X_NOW,Y_NOW,sys->angle);
				motor_run(GO_STOP,0,0,0);
				//delay_ms_sensers(200);
				//coordinate_calcu();														//计算出原始的坐标系
				//coord_org2map(navigat->x_org,navigat->y_org,&navigat->tx,&navigat->ty);	//坐标系转换
				return RET_NEAR_OK;
			}


			if(near->n_sta ==NO_SIDE_NEAR)
			{
				init_near_wall_navi(n_sta);
			}
		#if 0
			if( (ret_calc==0 || c_lost >= MAX_NEAR_LOST)&& *(navigat->near.pid->adc) < navigat->near.pid->min_adc )
			{
				c_round++;
				log_printf("lost or calc error(%d,%d),cround=%d\r\n",ret_calc,c_lost,c_round);
				/*
				if(c_round >=3)
				{
					log_printf("big round not near\r\n");
					init_near_wall_navi(NO_SIDE_NEAR);
					goto l_go_edeways;
				}
				*/
				if(ret_calc)
				{
					gyro_whlmap();
				}
				if(near->n_sta == RIGHT_SIDE_NEAR)
				{
					robot_turn_deg(GO_RIGTH,DEG_TURN_PWM,NEAR_LOST_DEG);
					goto l_mgo_edeways;
				}else if(near->n_sta == LEFT_SIDE_NEAR)
				{
					robot_turn_deg(GO_LEFT,DEG_TURN_PWM,NEAR_LOST_DEG);
					goto l_mgo_edeways;
				}

			}
		#endif

			if(near->n_sta == RIGHT_SIDE_NEAR)
			{
				if(sys->fall_sta)
						robot_turn_deg(GO_LEFT,DEG_TURN_PWM,25);
					else

						robot_turn_deg(GO_LEFT,DEG_TURN_PWM,12);


			}else if(near->n_sta == LEFT_SIDE_NEAR)
			{
				//if(check_round_bum(1))
				//	robot_turn_deg(GO_RIGTH,DEG_TURN_PWM,12);

				//else
				{
					//if(BUM_RIGHT(gSta) || MIDLE_HAVE_IRDA2() || LEFT_IR_BUM2())
					//{
					//	robot_turn_deg(GO_RIGTH,DEG_TURN_PWM,WALL_LOST_DEG);
					//}
					//else
					if(sys->fall_sta)
						robot_turn_deg(GO_RIGTH,DEG_TURN_PWM,25);
					else
					if(sys->g_sta[1] > 900 && sys->g_sta[2] > 900)
					{
						robot_turn_deg(GO_RIGTH,DEG_TURN_PWM,12);
					}
					else
					{
						robot_turn_deg(GO_RIGTH,DEG_TURN_PWM,12);
					}
				}

			}


		/*
			if(near->n_sta == RIGHT_SIDE_NEAR)
			{

				if(BUM_LEFT(gSta)
					)
				{
					//log_printf("midbum,60deg\r\n");
					robot_turn_deg(GO_LEFT,DEG_TURN_PWM,WALL_LOST_DEG);
				}else if((gSta & MASK_BUM_RIGHT ) && RIGHT_IRDA2() )
				{
					//log_printf("right bum,10deg\r\n");
					//robot_turn_deg(GO_LEFT,650,12);
					robot_turn_deg(GO_LEFT,DEG_TURN_PWM,12);
				}else
				{
					//log_printf("midbum,30deg\r\n");
					//robot_turn_deg(GO_LEFT,650,30);
					robot_turn_deg(GO_LEFT,DEG_TURN_PWM,30);
				}
			}else if(near->n_sta == LEFT_SIDE_NEAR)
			{
				//log_printf("gsta=%d,left=%d,%d\r\n",sys->gSta,sys->g_sta[0],sys->g_sta[1]);

				if( BUM_RIGHT(gSta)
					 )
				{
					//log_printf("midbuml,60deg\r\n");
					robot_turn_deg(GO_RIGTH,DEG_TURN_PWM,WALL_LOST_DEG);
				}else if((gSta & MASK_BUM_LEFT) && LEFT_IRDA2())
				{
						//log_printf("midbuml,12deg\r\n");
					robot_turn_deg(GO_RIGTH,DEG_TURN_PWM,12);
				}else
				{
					//log_printf("midbuml,30deg\r\n");
					robot_turn_deg(GO_RIGTH,DEG_TURN_PWM,30);
				}

			}
			*/
			else
			{
				log_printf("RET_NEAR_ERROR,nsta=%d\r\n",n_sta);
				return RET_NEAR_ERROR;
			}
l_mgo_edeways:

			//转圈了。。
			/*
			log_printf("t=%d,%d,%d,%d\r\n",t_quadrant[0],t_quadrant[1],t_quadrant[2],t_quadrant[3]);
			if(t_quadrant[0] > 0 && t_quadrant[1] > 0 && t_quadrant[2] > 0 && t_quadrant[3] > 0)
			{
				if( disxy_32(t_quadrant[0], t_quadrant[1]) < 10000 &&
					disxy_32(t_quadrant[1] ,t_quadrant[2]) < 10000 &&
					disxy_32(t_quadrant[2] , t_quadrant[3]) < 10000 )
				{



					log_printf("round near.. turn 40 and go line...\r\n");


					if(near->n_sta == RIGHT_SIDE_NEAR)
					{
						robot_turn_deg(GO_LEFT,DEG_TURN_PWM,40);
					}else if(near->n_sta == LEFT_SIDE_NEAR)
					{
						robot_turn_deg(GO_RIGTH,DEG_TURN_PWM,40);
					}

					www_idleintel_com();

					near->n_sta =NO_SIDE_NEAR	;
					navigat->out = sys->angle;
					t_quadrant[0] =t_quadrant[1] =t_quadrant[2] =t_quadrant[3] = 0;
				}

			}
			*/
			c_lost=0;
			//coordinate_calcu();


			motor_run(GO_STOP,0,0,0);
		//	log_printf("after bk(%d,%d,%d,%3.3f,%3.3f,%3.3f)\r\n==============\r\n",navigat->tx,navigat->ty,motor.c_left_hw,navigat->x_org_f,navigat->y_org_f,sys->angle);
			//if(ccc++ >=5)
			//	while(1);
			navigat->distance = 0;
			navigat->is_walk = 1;
			pd_gyro_int(GO_NEAR_PWM_FAST);
			navigat->out =sys->angle;
			cord_calc_store(0);
			gyro_whlmap();
			motor.c_left_hw = motor.c_right_hw = navigat->distance = 0;				///这个是本来就加了的

			motor_run(GO_FORWARD, GO_NEAR_PWM_FAST, 0, 0);

			navigat->near.pid->c_lost = 0;
			navigat->near.pid->c_lost_flag = 0;			//失去墙的标志
			ret_calc = 1;
			//记录起点的位置
			x_begin_line = X_NOW;
			y_begin_line = Y_NOW;
			sys->g_t_walk = 0;
			//micony20191022
	m_speed_pid_ctrl(0,0);
//单轮PID初始化
	single_whele_pid(0,motor.c_left_hw,left_pwm);
	single_whele_pid(0,motor.c_right_hw,left_pwm);
#if !LAGER_ROUND_MODE
			c_near_wall = 0;
#endif
		//	motor_run(GO_RIGHT_RD,0,0,0);
		//	navigat->near_sta = LOST_WALL_RIGHT;
		}

		if(TIM5->CNT >=5000)
		{
			TIM5->CNT = 0;

			navigat->out = format_agle(navigat->out,ANGLE_360);
			//proc_line_pid(navigat->out);
			//navigat_near_wall_pid(&navigat->out,5);
				proc_nearwall_pid(navigat->out,near->n_sta);
				navigat_near_wall_pid(&navigat->out,5);

						////2018-07-12 jzz debug先拿掉，若不通过，再恢复
			if(near->n_sta ==NO_SIDE_NEAR)
			{
				if(RIGHT_ADC() >= cfg->lock_right_adc)
					init_near_wall_navi(RIGHT_SIDE_NEAR);
				else if(LEFT_ADC() >= cfg->lock_left_adc)
					init_near_wall_navi(LEFT_SIDE_NEAR);
			}

			if(robot_whele_stop(1))
					continue;			//直接出来，给碰撞做准备

			dock_bum = proc_dock_near_bum(1);					//充电桩碰撞


			if( type == GO_NEAR_TYPE_DOCK )
			{
				//找充电桩
			//	c_dock = 0;
					if(detect_dock())
						goto l_go_out_for_dock;
/*
				read_ir_data_timeout(irData_bak,0);
				if(irData_bak[IR_L_PIN_NUM] ||	irData_bak[IR_ML_PIN_NUM] || irData_bak[IR_R_PIN_NUM] || ir_dock_insight(1))
				{
					log_printf("[motor_go_edgeways]found dock(%d,%d,%d)%d\r\n",
						irData_bak[IR_L_PIN_NUM] ,irData_bak[IR_ML_PIN_NUM],irData_bak[IR_R_PIN_NUM],c_dock_data);
					//if(c_dock_data++ >=3)
					{
						log_printf("[motor_go_edgeways]found dock\r\n");
						motor_run(GO_STOP,0,0,0);
						//delay_ms_sensers(200);
						coordinate_calcu(0); 													//计算出原始的坐标系
						coord_org2map(navigat->x_org,navigat->y_org,&navigat->tx,&navigat->ty); //坐标系转换
						goto	l_go_out_for_dock;
					}

				}else
					c_dock_data = 0;
*/
			}


			if( *(navigat->near.pid->adc) > navigat->near.pid->min_adc) //延边
			  lagle = sys->angle;
			else		//延边消失，转的角度超过180度，则失败退出
			{
				if(disfloat( lagle , sys->angle) > 180)
				{
					log_printf("lost over(%d,%d,%3.1f,%3.1f)\r\n",X_NOW,Y_NOW,sys->angle,lagle);
					motor_run(GO_STOP,0,0,0);
					//delay_ms_sensers(200);
					coordinate_calcu(0);														//计算出原始的坐标系
					coord_org2map(navigat->x_org,navigat->y_org,&navigat->tx,&navigat->ty);	//坐标系转换
					return RET_NEAR_ERROR;
				}
			}
#if CALE_BY_FRON
				ret_calc = coordinate_calcu(1);														//计算出原始的坐标系
#endif

			if(calue++ >=50)///200ms
			{
				m_speed_pid_ctrl(0,1);
				printf_power_sta();
#if !LAGER_ROUND_MODE
				if(c_near_wall ++ >=30)		///6s
				//if(c_near_wall ++ >=30*5)
				{

					for(c_m_angle = 0;c_m_angle<MAX_C_M_ANGLE;c_m_angle++)
						m_angle[c_m_angle] = 0;
					c_m_angle = 0;
					c_near_wall = 0;
					log_printf("log near\r\n");
				}
#endif

				calue = 0;
#if !CALE_BY_FRON
				ret_calc = coordinate_calcu(0);														//计算出原始的坐标系
#endif
				coord_org2map(navigat->x_org,navigat->y_org,&navigat->tx,&navigat->ty);	//坐标系转换
				//不是0度，90度，80度的方向,刷新坐标
				/**/
				if(x_begin_line != X_NOW && y_begin_line != Y_NOW)
				{
//					ajust_xy_by_near_line(x_begin_line,y_begin_line,X_NOW,Y_NOW,sys->angle,LINE_TYPE_LOST,n_sta);
					x_begin_line = X_NOW;
					y_begin_line = Y_NOW;
				}


				if( *(navigat->near.pid->adc) > navigat->near.pid->min_adc) //延边
				{
					//c_near++;
					//c_round = 0;

				}
				else if(near->n_sta !=NO_SIDE_NEAR)	//强制延边，则计数丢失的个数
					c_lost ++;

				if(llx!=navigat->tx || lly!=navigat->ty)
				{
#if WALK_PRINTF
					log_printf("*(%d,%d,%3.1f,0)-[%3.1f,%3.1f,0]*\r\n",navigat->tx,navigat->ty,sys->angle,navigat->x_org_f,navigat->y_org_f);
#endif
	#if WIFICONFIG
					updata_stream_cache_data(navigat->tx,navigat->ty,0,POINT_SCAN);
	#endif

	#if LAGER_ROUND_MODE
					if(c_m_angle >=MAX_C_M_ANGLE)
					{
						for(tmp=0;tmp<MAX_C_M_ANGLE-1;tmp++)
						{
							cyc_x[tmp] = cyc_x[tmp+1];
							cyc_y[tmp] = cyc_y[tmp+1];
							m_angle[tmp] = m_angle[tmp+1] ;
						}
						cyc_x[MAX_C_M_ANGLE-1] = X_NOW;
						cyc_y[MAX_C_M_ANGLE-1] = Y_NOW;
						m_angle[MAX_C_M_ANGLE-1] = sys->angle;				//保证最新的40个点
					}
					else
					{
						cyc_x[c_m_angle] = X_NOW;
						cyc_y[c_m_angle] = Y_NOW;
						m_angle[c_m_angle++] = sys->angle;		//保存角度
					}
	#endif
					llx = navigat->tx;
					lly = navigat->ty;
				}
				/**************************************************************************
					如果tox，toy有设置了置，则到点了，就停下来。
				*****************************************************************************/
				if(  (((X_NOW == tox || tox ==0) && (Y_NOW == toy || toy==0)) && (tox || toy)) || 	//到点了
				(type == GO_NEAR_TYPE_ADJ &&  ((by > toy && Y_NOW < toy) || (by < toy && Y_NOW > toy)))) //adj回去，Y轴超过了
				{
						log_printf("xy ok(%d,%d,%3.1f)\r\n",X_NOW,Y_NOW,sys->angle);
						motor_run(GO_STOP,0,0,0);
						//delay_ms_sensers(200);
						coordinate_calcu(0);														//计算出原始的坐标系
						coord_org2map(navigat->x_org,navigat->y_org,&navigat->tx,&navigat->ty);	//坐标系转换
						return RET_NEAR_OK;
				}



				if(type == GO_NEAR_DRAW_MAP)		//画地图
				{
					if(Y_NOW < 100 &&  Y_NOW > 98 && disXY(X_NOW,100) < 3)
					{
					//	log_printf("[motor_go_edgeways]to begin point2\r\n",X_NOW,Y_NOW);
						motor_run(GO_STOP,0,0,0);
						//delay_ms_sensers(200);
						coordinate_calcu(0);														//计算出原始的坐标系
						coord_org2map(navigat->x_org,navigat->y_org,&navigat->tx,&navigat->ty);	//坐标系转换
						return RET_NEAR_OK;
					}
				}

			}
		}
/*
		if(sys->mState == GO_STOP)
		{
			motor_run(GO_FORWARD, GO_NEAR_PWM_FAST, 0, 0);//有BUG，会多跑
		}
*/
	}


//	return 0;

l_go_out_for_dock:
	if(near->n_sta == RIGHT_SIDE_NEAR)
	{
		robot_turn_deg(GO_LEFT,DEG_TURN_PWM,60);
	}else if(near->n_sta == LEFT_SIDE_NEAR)
	{
		robot_turn_deg(GO_RIGTH,DEG_TURN_PWM,60);
	}
	log_printf("[motor_go_fw]...\r\n");
	CHECK_IDLE_STA_RT(0);
	sys->sState = SYS_DOCK;
	motor_go_fw(GO_FOR_DOCK_HW,NO_SIDE,0);
	return 0;
	//delay_ms(1000);
}



#if 0

/************************************************************************************************
 * 函数名称 : motor_go_forwark
 * 入    参 : hw     :  要行走的霍尔数，左轮计数，31个霍尔一个mm，
 *			  nsta   :  是否强制延边。参考LEFT_SIDE_NEAR 等定义
 *			  angle	 :  角度，该值传进来的值，就是锁定PID的值，如果有延边，该值会改变
 *			  m_go_check : 判断函数，如果想在里边做些判断，可传入判断函数，如果没有则传NULL
 *			  bk_dist:碰撞后退的步数
 * 返 回 值 : 0 - 碰撞，或状态错误退出
 *			  1 - 走完霍尔数退出
 *			  sys->g_sta[] 七路前撞红外的ADC差值，用于判断碰撞，排序是0 左边 - > 6 右边
 * 说    明 :
 *				1.函数根据angle传入的角度做角度PID直行
 *				2.根据 nsta 决定是否强制延边，如果不强制延边，发现墙边的时候，会自动延边
 *				3.根据 hw决定走的行程，用左轮计算，0则无限行走，知道碰撞
 *				4.函数阻塞运行，但检测串口和传感器，红外指令，如果串口或红外遥控有指令下来，会改变
 *				  系统状态 sys->sState  ，如果发现系统状态改变，或回到空闲状态，则退出
 *************************************************************************************************/
static uint32_t t_run;// = 0;
char motor_go_forwark_2a(int hw,int bk_dist,uint8_t nsta,float *angle,m_go_func m_go_check)
{

	int ret =0;
//	uint8_t  sta = sys->sState ;
//	float deg=NO_ANGLE_V;
//	int c_deg=0;
	//uint16_t kk;
	uint8_t old_state;
	uint16_t cc = 0;
	uint16_t tt=0;
	float last_agle=0;		//上次角度
	uint16_t invariant=0;	//角度不变的次数
	uint16_t n_pid_breakdown = 0;//计算角度环崩溃的次数,在直行时,角度环连续崩溃10次认为机器卡死了
	uint16_t n_spmax,n_spmin;//判断角度环是否崩溃的上下限变量
	uint8_t n_padchk_ct = 0;//判断角度环崩溃的计时器,以5ms为单位,20ms检查一次

	CHECK_NAVI_STA_RT(0);
	pid_init(GO_FORWARD_PWM);				//陀螺仪角度环PID初始化

	calc_gyro();  //取一下角度
	motor.c_left_hw = motor.c_right_hw = 0;	//码盘初始化

	//m_hal->out = sys->angle;// = 0;

	sys->gsta= 0;							//记录碰撞状态参数初始化


	//TIM5->CNT = 0;
	mstimeout(&sys->t_loop,0);
	init_near_wall_navi(nsta); 				//延边函数初始化
	log_printf("[motor_go_fw]sysSta=%d,auto_sta=%d,PWM(%d),set=%3.1f-%3.1f\r\n",sys->sState,sys->auto_sta,GO_FORWARD_PWM,*angle,sys->angle);
	motor_run(GO_FORWARD, GO_FORWARD_PWM, 0, 0);
	old_state = sys->sState;
//	t_run = msTmr;
	mstimeout(&t_run,0);
	m_speed_pid_walk(0,0);
	while(1)
	{
		//auto模式

		get_sensers(&sys->gSta);			//传感器参数初始化
		proc_uart_task();					//串口函数
		CHECK_NAVI_STA_RT(0);

		if(sys->sState != old_state)
		{
			log_printf("[motor_go_fw]old_state\r\n");
			motor_run(GO_STOP,0,0,0);
			//STOP_ALL_MOTOR();
			return 0;
		}

		//if(TIM5->CNT >=5000)				//每隔5ms做一次角度PID
		if(mstimeout(&sys->t_loop,5))
		{
			calc_gyro();					//取角度
			//micony
			if(m_speed_pid_walk(300,1))		//轮子停止了
			{
				motor_run(GO_STOP,0,0,0);
				ret = 0;
				break;
			}

			//micony 直行角度不变表明顶死
#if ICLEAN_HW_VERSION == 0x1402
			if(dis_float(last_agle,sys->angle) < 0.01)//0.01->0.005
#else
			if(dis_float(last_agle,sys->angle) < 0.005)//0.01->0.005
#endif
			{
				invariant++;
				if(invariant >=600)
				{
					motor_run(GO_STOP,0,0,0);
					log_printf("invariant angle(%3.3f),bum!\r\n",sys->angle);
					ret = 0;
					break;
				}

			}else
				invariant = 0;

			last_agle	  	= sys->angle;
			//TIM5->CNT = 0;
		/*
			kk++;
			if(kk > 100)
			{
				kk = 0;
				log_printf("[MGF]ag:%3.1f\r\n",sys->angle);
			}*/
			//if(dis_float(m_hal->out, sys->angle) >=15 )

			if(dis_float(*angle, sys->angle) >=15 )
			{
				log_printf("ang:%3.1f\r\n",sys->angle);
				if(cc++ >=10)
				{
					motor_run(GO_STOP,0,0,0);
					log_printf("side slip in forwark angle,%f,%f\r\n",*angle,sys->angle);
					ret = 0;
					break;
				}


			}else
				cc =0;

			if(mstimeout(&t_run,20000))
			{
				//没进入循环前，初始化的时候mstimeout(&t_run,0);
				motor_run(GO_STOP,0,0,0);
				log_printf("side slip in forwark timeout,%f,%f\r\n",*angle,sys->angle);
				ret = 0;
				break;
			}

			//===================================减速=========================================
			/*
			if(sys->pwm != FW_SLOW_PWM)
			{
				if(SLOW_IRD_MIDL())
				{
					//log_printf("slow(%d,%d,%d,%d,%d,%d,%d)cfg=%d\r\n",
					//motor_run(GO_FORWARD, FW_SLOW_PWM, 0, 0);
					motor_run(GO_FORWARD, 450, 0, 0);
				}
			}
			*/


			//======================行程走完==========================================
			if(WALK_DIST() > (hw-5) && hw >0)	//避免擦边
			{
				motor_run(GO_STOP,0,0,0);
				log_printf("[motor_go_fw]hw over,\r\n");
				ret = 1;
				break;
			}
			//======================PID=====================================

			//log_printf("ma>%3.1f,f>%3.1f,",m_hal->angle,*angle);
			*angle= format_agle(*angle,ANGLE_360);		//格式化角度
			//log_printf(",%3.1f\r\n",*angle);
			//if(deg != NO_ANGLE_V)
			n_spmax = proc_line_pid(*angle);						//角度PID
			//log_printf("ff>%3.1f,",*angle);
			navigat_near_wall_pid(angle,5);			//延边PID
			//log_printf("fff>%3.1f\r\n",*angle);

			//=================执行以下入参函数===========================
			if(m_go_check !=NULL)
			{
				if(m_go_check()==0)
				{
					log_printf("[motor_go_fw]cb out\r\n");
					break;
				}
			}

			if(n_padchk_ct ++ > 3)
			{
				n_padchk_ct = 0;
				#if 0
				n_spmax = WHEEL_PID_MAX;
				if(sys->sState == SYS_DOCK || sys->sState == SYS_NEAR
					|| (sys->sState == SYS_AUTO && sys->auto_sta == SYS_NEAR))
					n_spmin = WHEEL_PID_WORK_MIN;//250->350
				else
					n_spmin = WHEEL_PID_MIN;//250->350

				if(sys->lpwm == n_spmin ||  sys->rpwm == n_spmin)
				{
					if(n_pid_breakdown++ > 19)
					{
						log_printf("[motor_go_fw]pid break down\r\n");
						break;
					}
				}
				else
				{
					n_pid_breakdown = 0;
				}
				#else
				if(n_spmax == 1)
				{
					if(n_pid_breakdown++ > 19)
					{
						log_printf("[motor_go_fw]pid break down\r\n");
						break;
					}
				}
				else
				{
					n_pid_breakdown = 0;
				}
				#endif
			}
		}

		//===========================碰撞或红外探测到障碍物============================================
		if((sys->gSta & (MASK_BUM_LEFT | MASK_BUM_RIGHT)) || (MIDLE_IRDA() && WALK_DIST() > 100))		//碰撞了
		{

			sys->gsta = sys->gSta & MASK_BUM_MIDL;
			if(MIDLE_IRDA() &&  WALK_DIST() > 100)
				sys->gsta |= MASK_FW_MIDL;		//中间的红外
			log_printf("[go_forw]bum!gSta=%d(%d),ir=(%d,%d,%d)!!-3\r\n",sys->gSta,sys->gsta,sys->g_sta[2] ,sys->g_sta[3] ,sys->g_sta[4]);

			motor_run(GO_STOP,0,0,0);


			if(sys->gsta & (MASK_BUM_LEFT | MASK_BUM_RIGHT))
			{
				//delay_ms_sensers(100);
			#if 0//ICLEAN_MAC_TYPE == 2// || ICLEAN_MAC_TYPE == 0	//小圆机，跌落多退点
				if(sys->g_fall_sta)
					bk_dist *=3;
			#elif 0//ICLEAN_MAC_TYPE == 0
				if(sys->g_fall_sta)
					;//bk_dist *=2;
			#endif
				//motor_back_off(bk_dist,BACK_OFF_PWM);
				motor_back_off(bk_dist);
			}
			ret = 0;
			break;
		}
	}
	//每隔10分钟，陀螺仪要矫正一下
	/*
	if(mstimeout(&sys->t_gyro,600000))
	{
		motor_run(GO_STOP,0,0,0);
		delay_ms_sensers(300);	//等待陀螺仪稳定
		mpu6500_date_offset(2000);
	}
	*/
	log_printf("fwok--\r\n");
	return ret;
}
#endif


char motor_go_fw(int hw,uint8_t nsta,uint8_t type)
{
		//uint16_t g_sta;
	//	uint16_t last_ird;
		int tmr=0;//,tg=0;
		//int	c_stop = 0; 	//停止的次数
		static int c_calc = 0;
		static int scan_x = 0,scan_y=0;
		int ret =1;//,obst=0;
//		int y_dir,side_s=0;
		uint8_t  sta = sys->sState ;
	//	uint8_t bum = 0;
//		uint32_t t_walk;
		uint8_t dock_bum=0;
		//uint8_t front_bum=0;
		int16_t c= 0;



		CHECK_NAVI_STA_RT(0);

		www_idleintel_com();  //取一下角度
		motor.c_right_hw = motor.c_front_hw = 0;
		motor.c_left_hw = motor.total_left_hw = motor.total_right_hw=0;

		sys->slip =0;

		sys->c_ajust_agle = 0;
		sys->total_ajust_agle = 0;
		//navigat->out = navigat->angle;
		navigat->out = format_agle(sys->angle,ANGLE_360);
		navigat->adj_run_ret = FALSE;

		navigat->bum_side = 0;
		navigat->walk_dis = 0;
		navigat->is_bum = 0;
		//sys->c_near_wall_left = sys->c_near_wall_right =0;

		navigat->distance = sys->c_pid_hw = 0;
		sys->gsta= 0;


		init_doci_ir();
		init_near_wall_navi(nsta); //NO_SIDE_NEAR
		navigat->c_go++;
		navigat->c_go_force++;
		coordinate_calcu(0);
		gyro_whlmap();
		//cord_calc_store(0);

		log_printf("[motor_go_fw](%d,%d,)org(%d,%d),lr(%3.1f,%3.f,%3.1f,%3.1f)a=%3.1f,ld=%d\r\n",navigat->tx,navigat->ty,navigat->x_org,navigat->y_org,navigat->x_org_f,navigat->y_org_f,navigat->x_org_r,navigat->y_org_r,sys->angle,0);
		//log_printf("[motor_go_fw](%d,%d,)-(%d,%d)\r\n",navigat->tx,navigat->ty,navigat->x_org,navigat->y_org);
		calc_timr_upload();
		//记录下最后的坐标
		navigat->l_dist = navigat->distance;
		sys->c_front_stop = 0;		//前轮停止的次数清零
		CHECK_NAVI_STA_RT(0);
	//	c_ird = 0;
//		t_walk = msTmr;

		TIM5->CNT = 0;
		pd_gyro_int(GO_FORWARD_PWM);
		init_doci_ir(); 				//充电桩
		motor.c_left_hw = motor.c_right_hw = 0;
		motor_run(GO_FORWARD, GO_FORWARD_PWM, 0, 0);
		navigat->is_walk = 1;
		printf_power_sta();
		sys->g_t_walk = 0;
		while(1)
		{

			get_sensers(&sys->gSta);			//
			proc_uart_task();

			if(sys->sState != sta || sys->sState == SYS_IDLE)
			{
				log_printf("SYS_IDLE\r\n");
				ret = 0;
				navigat->is_walk = 0;

				break;
			}

			//============================每隔5ms取一下角度，PID计算，并在里边计算坐标=============================
			if(TIM5->CNT >=5000)
			{

				TIM5->CNT = 0;
				//log_printf("B");
				tmr ++; 	//用于计算红外
			//	front_bum = caluc_bum_by_front(1);
				robot_whele_stop(1);
				//www_idleintel_com();
				if(type == 1)			//检测中线
				{
					if(c++ >= 9)
					{
						c = 0;
						read_ir_data(IrData_bak,0);
						if(IrData_bak[IR_L_PIN_NUM]==IR_MID || IrData_bak[IR_R_PIN_NUM]==IR_MID)
						{
							motor_run(GO_STOP,0,0,0);
							ret =2;
							log_printf("%d,%d,%d\r\n",IrData_bak[IR_L_PIN_NUM],IrData_bak[IR_ML_PIN_NUM],IrData_bak[IR_R_PIN_NUM]);
							break;
						}
					}

				}

				if(motor.c_left_hw > (hw-10) && hw >0)	//避免擦边
				{
					motor_run(GO_STOP,0,0,0);
					ret = 1;
					break;
				}
				//=========================PID计算========================
				if(hw > 40 || hw ==0 )
				{
					proc_line_pid(navigat->out);
				}
				//dock_bum = proc_doc_ir();
				c_calc++;
#if CALE_BY_FRON

					if(coordinate_calcu(1)==0)														//计算出原始的坐标系
					{
						motor_run(GO_STOP,0,0,0);
						ret = 0;
						break;
					}
#endif
				//=======================计算坐标===========================
				if(c_calc >=40)
				{
					c_calc = 0;

#if !CALE_BY_FRON

					if(coordinate_calcu(0)==0)														//计算出原始的坐标系
					{
						motor_run(GO_STOP,0,0,0);
						ret = 0;
						break;
					}
#endif
					coord_org2map(navigat->x_org,navigat->y_org,&navigat->tx,&navigat->ty); //坐标系转换
					//计算是否停止了。
					//这里会导致可能清扫区域和屏幕上的显示的不太一样。
					if(scan_x != navigat->tx)
					{
#if WALK_PRINTF
						log_printf("*(%d,%d,%3.1f,0)[%d,%d,%d,%d,%d]*%d,%d\r\n",navigat->tx,navigat->ty,sys->angle,
																sys->g_sta[0],sys->g_sta[1],sys->g_sta[3],sys->g_sta[5],sys->g_sta[6],navigat->distance,msTmr);
					//	if(side_s)
						//	log_printf("*(%d,%d,%3.1f,0)[0,0,0,0]*midscan\r\n",navigat->tx,navigat->ty+y_dir,sys->angle);
#endif
	#if WIFICONFIG
						updata_stream_cache_data(navigat->tx,navigat->ty,0,POINT_SCAN);
	#endif
						scan_x = navigat->tx;
						navigat->scan_x++;
					}
					if(scan_y != navigat->ty)
					{
#if WALK_PRINTF
						log_printf("*(%d,%d,%3.1f,0)[%d,%d,%d,%d,%d]*%d,%d\r\n",navigat->tx,navigat->ty,sys->angle,
																			sys->g_sta[0],sys->g_sta[1],sys->g_sta[3],sys->g_sta[5],sys->g_sta[6],navigat->distance,msTmr);
#endif
	#if WIFICONFIG
						updata_stream_cache_data(navigat->tx,navigat->ty,0,POINT_SCAN);
	#endif
						scan_y = navigat->ty;
					}




				}


			}
			//wy190527 新增左右两侧碰撞开关检测
			if((sys->gSta & (MASK_BUM_LEFT | MASK_BUM_LEFT2 | MASK_BUM_RIGHT | MASK_BUM_RIGHT2)) || (MIDLE_IRDA() && tmr >3)/* || (l_dist > MIN_LASER_DIST && l_dist < 8) || front_bum*/ || dock_bum)		//碰撞了
			{
			      sys->gsta = sys->gSta;   //2019 10  21
				//sys->gsta = sys->gSta & MASK_BUM_MIDL;
				if(MIDLE_IRDA() && tmr >3)
					sys->gsta |= MASK_FW_MIDL;		//中间的红外
				log_printf("[go_forw_ony]bum!gSta=%d(%d),ir=(%d,%d,%d)dockbum=%d!!",sys->gSta,sys->gsta,sys->g_sta[2] ,sys->g_sta[3] ,sys->g_sta[4],dock_bum);
	//			bum = 1;


				//如果不是碰撞，前边的红外探测到，则再前进一下
				if((sys->gsta & MASK_BUM_MIDL) == 0)
					//delay_ms(80);		//再前进一下
					delay_sensor(80);
				motor_run(GO_STOP,0,0,0);
			//	proc_gyro_task(WAIT_STOP_CNT, WAIT_STOP_COUNT);
				if(sys->gsta & (MASK_BUM_LEFT | MASK_BUM_LEFT2 | MASK_BUM_RIGHT | MASK_BUM_RIGHT2))
				{
					navigat->is_bum = 1;
					//delay_ms_sensers(100);
					if(sys->fall_sta)		//跌落，则
					motor_back_off(BACK_HW*8);
				else
					motor_back_off(BACK_HW);
				}
				navigat->is_walk =0;

				ret = 0;
				break;
			}

			if(sys->mState == GO_STOP)
			{
				motor_run(GO_FORWARD, GO_FORWARD_PWM, 0, 0);//有BUG，会多跑
			}

		}


		navigat->is_walk = 0;


		log_printf("\r\ngo_forwark_ony ok,hw=%d,dis=%d,left=%d,right=%d,now=(%d,%d,%3.1f) org=(%d,%d)\r\n",
			hw,navigat->distance,motor.c_left_hw,motor.c_right_hw,
			navigat->tx,navigat->ty,sys->angle,navigat->x_org,navigat->y_org);

		navigat->walk_dis = navigat->distance;		//计算走过的距离

		cord_calc_store(1); 					//坐标系变换
		//gyro_whlmap();
		return ret;
}




/*****************************************************************************
 * 函数名称:
 * 入    参:
 * 				type  : GO_TYPE_SAVE:存储坐标，障碍点  GO_TYPE_NO_SAVE - 不存储
 *				t_out :退出 条件
 *						OUT_TYPE_Y				0x01		//工字型的方式
 *						OUT_TYPE_SIDE			0x02		//侧面斜过去
 *				n_sta	:最高位为1，则表示不立马延边，而是先走，碰撞后再延边
 *				dir   : 1 Y轴正方向 -1 Y轴负方向
 *****************************************************************************/
 char motor_go_near_wall(uint8_t n_sta)
{
	//short xx=0,yy=0,x1=0,y1=0;
	int calue=0;
	int by = navigat->y_org;
//	uint16_t gSta;
	uint8_t sta;
//	short	lx=0,ly=0;
//	short llx=0,lly=0;
	uint32_t t_near;
//	short bxx = X_NOW,byy=Y_NOW;
//	int ret_calc=1;
//	char ret = 0;
//	uint8_t dock_bum = 0;
	uint16_t cbum=0;
//	uint16_t lleft=0,lright=0;
	SIDEOBST *m_obst = (SIDEOBST *)m_stack;
//	static uint8_t kidx=0;
//	int dist;
//	float begin_angle = sys->angle;		//开始的角度
	NEAR_WALL *near = &navigat->near;
//	float m_angle[MAX_C_M_ANGLE];			//保存最近的20个碰撞的角度
//	int16_t c_m_angle=0;
#if LAGER_ROUND_MODE
////	int16_t cyc_x[MAX_C_M_ANGLE],cyc_y[MAX_C_M_ANGLE];
//	uint8_t tmp=0;//,c_ok=0;
#else
	int16_t c_near_wall = 0;
#endif


#if JUDGE_PID_CTRL_WHEEL_STOP
		static int idex=0,hw_l=0,hw_r=0;	//,lhw2=0;
#endif


	//for(c_m_angle = 0;c_m_angle<MAX_C_M_ANGLE;c_m_angle++)
	//	m_angle[c_m_angle] = 0;
//	c_m_angle = 0;

if(sys->motor_power_off ==1)
 		MOTOR_POWER_ON();
	m_obst->len = 0;
	navigat->distance = 0;
	CHECK_NAVI_STA_RT(0);
//	motor_run(GO_ROUND,600,0,0);
	pd_gyro_int(GO_NEAR_PWM);
	navigat->out =navigat->angle;
	motor.c_left_hw=motor.c_right_hw = 0;

	if(n_sta & 0x80)		//不是强制延边
		init_near_wall_navi(NO_SIDE_NEAR);
	else
		init_near_wall_navi(n_sta);
	init_doci_ir();					//充电桩
//	navigat->near.pid->max_lost = 12;		//延边丢失的处理
	//init_near_wall_navi(NO_SIDE_NEAR);	//刚开始不需要一直沿边

	cord_calc_store(0);
	gyro_whlmap();
	proc_dock_near_bum(0);
	www_idleintel_com();

	log_printf("[go_near_wall]sta=%d,by=%d,(%d,%d,%3.1f)\r\n",navigat->near.n_sta,by,navigat->x_org,navigat->y_org,sys->angle);
	//navigat->wheel_dir = 1;
	sta = sys->sState;
	t_near = msTmr;
	robot_whele_stop(0);
	motor.c_left_hw = motor.c_right_hw = 0;
	motor_run(GO_FORWARD, GO_NEAR_PWM, 0, 0);
	navigat->is_walk =1;
	//if(t_out == OUT_TYPE_Y)
	{
		sys->min_pwm = -100;
		sys->max_pwm = 100;
	}
	printf_power_sta();
	sys->g_t_walk = 0;
//	uint16_t nowalk=0;
	//micony20191022
	m_speed_pid_ctrl(0,0);
//单轮PID初始化
	single_whele_pid(0,motor.c_left_hw,left_pwm);
	single_whele_pid(0,motor.c_right_hw,left_pwm);
	//coordinate_calcu();
//	idex=0;
	while(1)
	{
		proc_uart_task();
		get_sensers(&sys->gSta);
		if(sys->sState !=sta)
			return 0;
		CHECK_NAVI_STA_RT(0);

		if(sys->gSta & (MASK_BUM_MIDL) ||(sys->gSta & (MASK_BUM_LEFT | MASK_BUM_LEFT2 | MASK_BUM_RIGHT | MASK_BUM_RIGHT2))
			|| MIDLE_IRDA() )  //
		{
//			gSta = sys->gSta;	//记录碰撞的状态
			www_idleintel_com();
			cbum++;
			log_printf("----------\r\n[near_wall]bum,sta=%d,angle=%3.1f,t=%d,sta=%d,ir=(%d,%d,%d),by=%d,now=%d,side=%d\r\n",
				near->n_sta,sys->angle,msTmr - t_near,sys->gSta,sys->g_sta[2],sys->g_sta[3],sys->g_sta[4],
					by,navigat->y_org,navigat->side);
			motor_run(GO_STOP,0,0,0);

			motor_back_off(BACK_HW);
			return 0;

		}

		if(TIM5->CNT >=5000)
		{
			TIM5->CNT = 0;
#if CALE_BY_BKHW
			//calc_xy_org(motor.c_left_hw - dist, &navigat->x_orgf, &navigat->y_orgf);
			//dist = motor.c_left_hw ;
#endif

			navigat->out = format_agle(navigat->out,ANGLE_360);
			www_idleintel_com();
			proc_line_pid(navigat->out);
			if(calue++>10)
			{
			//	log_printf("lr=%d,%d\r\n",motor.c_left_hw,motor.c_right_hw);
				if(motor.c_left_hw > 600 ||  motor.c_right_hw  >600)
					return 1;
				calue = 0;
			}


		}

	}

//	return 0;
	//delay_ms(1000);
}
 #if 0
char motor_go_near_wall(uint8_t n_sta)
{
	//short xx=0,yy=0,x1=0,y1=0;
	int calue=0;
	int by = navigat->y_org;
	uint16_t gSta;
	uint8_t sta;
//	short	lx=0,ly=0;
//	short llx=0,lly=0;
	uint32_t t_near;
//	short bxx = X_NOW,byy=Y_NOW;
	int ret_calc=1;
//	char ret = 0;
//	uint8_t dock_bum = 0;
	uint16_t cbum=0;
//	uint16_t lleft=0,lright=0;
	SIDEOBST *m_obst = (SIDEOBST *)m_stack;
//	static uint8_t kidx=0;
//	int dist;
//	float begin_angle = sys->angle;		//开始的角度
	NEAR_WALL *near = &navigat->near;
	float m_angle[MAX_C_M_ANGLE];			//保存最近的20个碰撞的角度
	int16_t c_m_angle=0;
#if LAGER_ROUND_MODE
	int16_t cyc_x[MAX_C_M_ANGLE],cyc_y[MAX_C_M_ANGLE];
	uint8_t tmp=0;//,c_ok=0;
#else
	int16_t c_near_wall = 0;
#endif


#if JUDGE_PID_CTRL_WHEEL_STOP
		static int idex=0,hw_l=0,hw_r=0;	//,lhw2=0;
#endif


	for(c_m_angle = 0;c_m_angle<MAX_C_M_ANGLE;c_m_angle++)
		m_angle[c_m_angle] = 0;
	c_m_angle = 0;


	m_obst->len = 0;
	navigat->distance = 0;
	CHECK_NAVI_STA_RT(0);
//	motor_run(GO_ROUND,600,0,0);
	pd_gyro_int(GO_NEAR_PWM);
	navigat->out =navigat->angle;
	motor.c_left_hw=motor.c_right_hw = 0;

	if(n_sta & 0x80)		//不是强制延边
		init_near_wall_navi(NO_SIDE_NEAR);
	else
		init_near_wall_navi(n_sta);
	init_doci_ir();					//充电桩
	navigat->near.pid->max_lost = 12;		//延边丢失的处理
	//init_near_wall_navi(NO_SIDE_NEAR);	//刚开始不需要一直沿边

	cord_calc_store(0);
	gyro_whlmap();
	proc_dock_near_bum(0);
	www_idleintel_com();

	log_printf("[go_near_wall]sta=%d,by=%d,(%d,%d,%3.1f)\r\n",navigat->near.n_sta,by,navigat->x_org,navigat->y_org,sys->angle);
	//navigat->wheel_dir = 1;
	sta = sys->sState;
	t_near = msTmr;
	robot_whele_stop(0);
	motor.c_left_hw = motor.c_right_hw = 0;
	motor_run(GO_FORWARD, GO_NEAR_PWM, 0, 0);
	navigat->is_walk =1;
	//if(t_out == OUT_TYPE_Y)
	{
		sys->min_pwm = -100;
		sys->max_pwm = 100;
	}
	printf_power_sta();
	sys->g_t_walk = 0;
//	uint16_t nowalk=0;
	//micony20191022
	m_speed_pid_ctrl(0,0);
//单轮PID初始化
	single_whele_pid(0,motor.c_left_hw,left_pwm);
	single_whele_pid(0,motor.c_right_hw,left_pwm);
	//coordinate_calcu();
//	idex=0;
	while(1)
	{
		proc_uart_task();
		get_sensers(&sys->gSta);
		if(sys->sState !=sta)
			return 0;
		CHECK_NAVI_STA_RT(0);

		if(sys->gSta & (MASK_BUM_MIDL) ||(sys->gSta & (MASK_BUM_LEFT | MASK_BUM_LEFT2 | MASK_BUM_RIGHT | MASK_BUM_RIGHT2))
			|| MIDLE_IRDA() )  //
		{
			gSta = sys->gSta;	//记录碰撞的状态
			www_idleintel_com();
			cbum++;
			log_printf("----------\r\n[near_wall]bum,sta=%d,angle=%3.1f,t=%d,sta=%d,ir=(%d,%d,%d),by=%d,now=%d,side=%d\r\n",
				near->n_sta,sys->angle,msTmr - t_near,sys->gSta,sys->g_sta[2],sys->g_sta[3],sys->g_sta[4],
					by,navigat->y_org,navigat->side);
			motor_run(GO_STOP,0,0,0);
			if(sys->work_mod & MWO_MOP)
			//delay(100);
			delay_sensor(100);
				coordinate_calcu(1);												//计算出原始的坐标系
				coord_org2map(navigat->x_org,navigat->y_org,&navigat->tx,&navigat->ty);	//坐标系转换

#if !LAGER_ROUND_MODE
			if(c_m_angle >=MAX_C_M_ANGLE)
				c_m_angle = 0;
			m_angle[c_m_angle++] = sys->angle;		//保持角度

			if(near_round_360(m_angle,c_m_angle))
			{
				log_printf("found round...go line...\r\n");

				init_near_wall_navi(NO_SIDE_NEAR);
				for(c_m_angle = 0;c_m_angle<MAX_C_M_ANGLE;c_m_angle++)
					m_angle[c_m_angle] = 0;
				c_m_angle = 0;
				goto l_got_neawall;
			}
#else////2018-07-10
			if(near_large_round_360(cyc_x,cyc_y,m_angle,c_m_angle)) 					///绕大桩判断函数
			{
				log_printf("found large round...go line...\r\n");
				if(near->n_sta == RIGHT_SIDE_NEAR)
				{
					robot_turn_deg(GO_LEFT,DEG_TURN_PWM,135);
					//goto l_mgo_edeways;
				}else if(near->n_sta == LEFT_SIDE_NEAR)
				{
					robot_turn_deg(GO_RIGTH,DEG_TURN_PWM,135);
					//goto l_mgo_edeways;
				}
				for(tmp=0;tmp<c_m_angle;tmp++)
				{
					log_printf("%02d(%03d,%03d,%03f)\r\n",tmp,cyc_x[tmp],cyc_y[tmp],m_angle[tmp]);
				}
				init_near_wall_navi(NO_SIDE_NEAR);
				for(c_m_angle = 0;c_m_angle<MAX_C_M_ANGLE;c_m_angle++)
					m_angle[c_m_angle] = 0;
				c_m_angle = 0;
				return 0;
			}
#endif
			if(ret_calc==0)
				gyro_mapwhl();
		//	navigat->wheel_dir = 0;
			//delay_ms(100);
			//init_near_wall_navi(n_sta);		//碰撞后，才开始一直沿边

			navigat->distance = 0;
			log_printf("bum=%d,n_sta=%02x\r\n",gSta,n_sta);
			//碰撞了，才后退。
			//delay(100);
			delay_sensor(100);
			if(gSta & MASK_BUM_MIDL)
			{

				if(sys->fall_sta)		//跌落，则
					motor_back_off(BACK_HW*8);
				else
					motor_back_off(BACK_HW);
				//delay(80);
				delay_sensor(80);
				//delay_ms_sensers(200);
			}//else
			//delay(50);
			delay_sensor(50);
			//	delay_ms_sensers(100);
			ret_calc = coordinate_calcu(0);														//计算出原始的坐标系
			coord_org2map(navigat->x_org,navigat->y_org,&navigat->tx,&navigat->ty);	//坐标系转换
#if WALK_PRINTF
			log_printf("*(%d,%d,%3.1f,0)-[%3.1f,%3.1f][%3.1f,%3.1f]*\r\n",navigat->tx,navigat->ty,sys->angle,navigat->x_org_f,navigat->y_org_f,navigat->x_org_r,navigat->y_org_r);
#endif
#if WIFICONFIG
			updata_stream_cache_data(navigat->tx,navigat->ty,0,POINT_SCAN);
#endif
			log_printf("side=%d,by=%d,yorg=%d\r\n",navigat->side,by,navigat->y_org);
			if((navigat->side == LEFT_SIDE && by < navigat->y_org && (navigat->y_org - by) > 1000) ||
				(navigat->side == RIGHT_SIDE&& by > navigat->y_org && (by - navigat->y_org ) > 1000) )
			{
				log_printf("foud back ,return 2\r\n");
				return 2;
			}
			if(disXY(by,navigat->y_org ) > 1000)
			{
				log_printf("yover\r\n");
				return 1;
			}
			if(near->n_sta ==NO_SIDE_NEAR )
			{
				init_near_wall_navi(n_sta & 0x7F);  //miconydrawmap
				//return 0;
			}

			if(n_sta == NO_SIDE_NEAR)
			{
				log_printf("nearwall,no side near,out\r\n");
				return 0;
			}
		//	log_printf("back off ok\r\n");
			if(near->n_sta == RIGHT_SIDE_NEAR)
			{
				//micony20191022
				/*
				if(BUM_LEFT(gSta) || MIDLE_HAVE_IRDA2() || RIGHT_IR_BUM2())
				{
					//log_printf("midbum,60deg\r\n");
					robot_turn_deg(GO_LEFT,DEG_TURN_PWM,WALL_LOST_DEG);
				}else
				*/if((gSta & MASK_BUM_RIGHT ) && RIGHT_IRDA2() )
				{
					//log_printf("right bum,10deg\r\n");
					//robot_turn_deg(GO_LEFT,650,12);
					robot_turn_deg(GO_LEFT,DEG_TURN_PWM,12);
				}else
				{
					//log_printf("midbum,30deg\r\n");
					//robot_turn_deg(GO_LEFT,650,30);
					robot_turn_deg(GO_LEFT,DEG_TURN_PWM,20);
				}
			}else if(near->n_sta == LEFT_SIDE_NEAR)
			{
				//log_printf("gsta=%d,left=%d,%d\r\n",sys->gSta,sys->g_sta[0],sys->g_sta[1]);
				//micony20191022
/*
				if(BUM_RIGHT(gSta) || MIDLE_HAVE_IRDA2() || LEFT_IR_BUM2())
				{
					//log_printf("midbuml,60deg\r\n");
					robot_turn_deg(GO_RIGTH,DEG_TURN_PWM,WALL_LOST_DEG);
				}else
				*/if((gSta & MASK_BUM_LEFT) && LEFT_IRDA2() )
				{
						//log_printf("midbuml,12deg\r\n");
					robot_turn_deg(GO_RIGTH,DEG_TURN_PWM,12);
				}else
				{
					//log_printf("midbuml,30deg\r\n");
					robot_turn_deg(GO_RIGTH,DEG_TURN_PWM,20);
				}

			}
//l_got_neawall:
			ret_calc = 1;
			navigat->distance = 0;
			navigat->is_walk = 1;
			pd_gyro_int(GO_NEAR_PWM);
			navigat->out =sys->angle;
			cord_calc_store(0);
			gyro_whlmap();
			log_printf("=========sta=%d,go_fw=========\r\n",near->n_sta);

			//micony20191022
			m_speed_pid_ctrl(0,0);
		//单轮PID初始化
			single_whele_pid(0,motor.c_left_hw,left_pwm);
			single_whele_pid(0,motor.c_right_hw,left_pwm);
			motor_run(GO_FORWARD, GO_NEAR_PWM, 0, 0);
			navigat->near.pid->c_lost = 0;
			navigat->near.pid->c_lost_flag = 0;			//失去墙的标志
			motor.c_left_hw = motor.c_right_hw = navigat->distance = 0; 		///本来就是这里的
			sys->g_t_walk = 0;
		//	navigat->wheel_dir = 1;
			t_near = msTmr;
//			dock_bum = 0;
//			nowalk=0;
////			lleft=lright=0;
#if !LAGER_ROUND_MODE
			c_near_wall = 0;
#endif
			//test

		//	motor_run(GO_RIGHT_RD,0,0,0);
		//	navigat->near_sta = LOST_WALL_RIGHT;
		}

		if(TIM5->CNT >=5000)
		{
			TIM5->CNT = 0;
#if CALE_BY_BKHW
			//calc_xy_org(motor.c_left_hw - dist, &navigat->x_orgf, &navigat->y_orgf);
			//dist = motor.c_left_hw ;
#endif
			if(robot_whele_stop(1))
					continue;			//直接出来，给碰撞做准备
			navigat->out = format_agle(navigat->out,ANGLE_360);
		//	log_printf("s=%3.1f\r\n",navigat->out);
			//micony20191022
			if( near->n_sta ==NO_SIDE_NEAR)  //不沿边
			{
				proc_line_pid(navigat->out);
			}else			//沿边
			{
				//micony20191022
				proc_nearwall_pid(navigat->out,near->n_sta);
				navigat_near_wall_pid(&navigat->out,5);

			}

			//micony20191022  加个速度环
			if(calue++ >=50)		//200ms
			{
				calue = 0;
				coordinate_calcu(1);												//计算出原始的坐标系
				coord_org2map(navigat->x_org,navigat->y_org,&navigat->tx,&navigat->ty);	//坐标系转换
				log_printf("l=%d,%d,pwm=%d,%d\r\n",motor.c_left_hw,motor.c_right_hw,L_FORWRK_PWM,R_FORWRK_PWM);
				//micony20191022  加个速度环
				m_speed_pid_ctrl(0,1);
				log_printf("side=%d,by=%d,yorg=%d\r\n",navigat->side,by,navigat->y_org);
				if((navigat->side == LEFT_SIDE && by < navigat->y_org && (navigat->y_org - by) > 1000) ||
					(navigat->side == RIGHT_SIDE&& by > navigat->y_org && (by - navigat->y_org ) > 1000) )
				{
					log_printf("foud back ,return 2\r\n");
					return 2;
				}
				if(disXY(by,navigat->y_org ) > 1000)
				{
					log_printf("yover\r\n");
					return 1;
				}
			}



		}

	}

//	return 0;
	//delay_ms(1000);
}
#endif
//modify2017-08-17 short 改成int，否则会溢出
char nearwall_cross_map(int y,float angle)
{
	uint8_t n_sta;
	char ret;


	CHECK_NAVI_STA_RT(0);		//?D??×′ì?ê?·?óD??±?￡???±??òí?3?

	//?????ó±?μ?×′ì?
	if(navigat->side == LEFT_SIDE)		//×ó±???óò￡??????ó±?μ?×′ì?
	{
		n_sta = navigat->lst_angle == 0?RIGHT_SIDE_NEAR:LEFT_SIDE_NEAR;

		turn_to_deg(90);
	}else
	{
		n_sta = navigat->lst_angle == 0?LEFT_SIDE_NEAR:RIGHT_SIDE_NEAR;

		turn_to_deg(270);
	}

	n_sta |=0x80;

	log_printf("[nearwall_cross_map]now=(%d,%d,%d,%d)dis=%d\r\n",X_NOW,Y_NOW,navigat->x_org,navigat->y_org,y);
	//ret = motor_go_near_wall(n_sta);
	ret = motor_go_forwark(600,NO_SIDE_NEAR,NULL);

	if(ret ==0)
		sys->c_loop++;
	else
		sys->c_loop = 0;
	log_printf("c loop=%d,ret=%d\r\n",sys->c_loop,ret);
	if(sys->c_loop > 3)
	{
		navigat->side = navigat->side == LEFT_SIDE?RIGHT_SIDE:LEFT_SIDE;
		sys->c_loop=0;
		sys->c_go_bum = 100;//延边去
	}

	turn_to_deg(angle);		//1¤×?Dí￡?????à′
	return 1;
}


/*
检测前方是否有特征物
*/
#if ARC_Z_GO
#if ADJ_END

void check_front_target(short xdir,int *xorg)
{
	TOBST *tost =NULL;
	//short xdir =  navigat->lst_angle == 0?1:-1;
	int i;
	*xorg = 0;
	log_printf("--check_front_target--\r\n");
	for(i=0;i<navigat->j_obst;i++)									//这里的作用是?
	{
		tost = &navigat->adj_obst[i];		//特征物
		if(xdir == tost->x_dir && Y_NOW > tost->by && Y_NOW < tost->ey)		//方向一致
		{
			log_printf("found target(%d,%d,%d)\r\n",tost->bx,Y_NOW,tost->x_dir);
			if((xdir ==1 && X_NOW < tost->bx) || (xdir ==-1 && X_NOW > tost->bx))
			{
				if(xdir ==1)
					*xorg = tost->x_org - 700;
				else
					*xorg = tost->x_org + 700;
				log_printf("OKsorg = %d\r\n",*xorg);
				return ;
			}
		}
	}
}
#endif
#endif
//计数象限数
uint8_t count_quadrant(uint8_t quadrant_1,uint8_t quadrant2,uint8_t quadrant3,uint8_t quadrant4)
{
	uint8_t c_quadrant=0;
	if(quadrant_1)
		c_quadrant++;
	if(quadrant2)
		c_quadrant++;
	if(quadrant3)
		c_quadrant++;
	if(quadrant4)
		c_quadrant++;
	return c_quadrant;
}
uint8_t get_quadrant(float angle)
{
	if(angle >=0 && angle <=90)
		return 1;
	if(angle >90 && angle <=180)
		return 2;
	if(angle >180 && angle <=270)
		return 3;
	if((angle >270 && angle <=360) ||angle < 0)
		return 4;
	return 0;
}
//
uint8_t near_large_round_360(int16_t *cyc_x,int16_t *cyc_y,float *m_angle,int16_t c_m_angle)
{
	int16_t i/*,j=0*/;
	uint8_t quadrant_1 = 0,quadrant2=0,quadrant3=0,quadrant4=0;
	uint8_t c_quadrant=0;
//	log_printf("[near_large_round_360]\r\n");
#if 0
	for(i=c_m_angle-1;i>=0;i--)
	{
		if( m_angle[i] == 0)
			continue;
		if(m_angle[i] >=0 && m_angle[i] <=90)
			quadrant_1++;
		if(m_angle[i] >90 && m_angle[i] <=180)
			quadrant2++;
		if(m_angle[i] >180 && m_angle[i] <=270)
			quadrant3++;
		if((m_angle[i] >270 && m_angle[i] <=360) || m_angle[i] < 0)
			quadrant4++;
		j++;
		//log_printf("quadrant=%3.1f,%d,%d,%d,%d\r\n",m_angle[i],quadrant_1,quadrant2,quadrant3,quadrant4);
		if(c_quadrant >=4 && j > 8)  //包含4个象限值  间隔大于8个点
		{
 			if(  (disxy(get_quadrant(m_angle[c_m_angle-1]),get_quadrant(m_angle[i-3]))<=1) && (disxy(get_quadrant(m_angle[c_m_angle-2]),get_quadrant(m_angle[i-2]))<=1)
 				&& 	(disxy(get_quadrant(m_angle[c_m_angle-3]),get_quadrant(m_angle[i-1]))<=1)  && (disxy(get_quadrant(m_angle[c_m_angle-4]),get_quadrant(m_angle[i]))<=1)   ) 	//连续3个点在同一象限内
			{
				if( (disxy(cyc_x[c_m_angle-1],cyc_x[i-3])<=1) && (disxy(cyc_y[c_m_angle-1],cyc_y[i-3])<=1)
					&& (disxy(cyc_x[c_m_angle-2],cyc_x[i-2])<=1) && (disxy(cyc_y[c_m_angle-2],cyc_y[i-2])<=1)
					&& (disxy(cyc_x[c_m_angle-3],cyc_x[i-1])<=1) && (disxy(cyc_y[c_m_angle-3],cyc_y[i-1])<=1)
					&& (disxy(cyc_x[c_m_angle-4],cyc_x[i])<=1) && (disxy(cyc_y[c_m_angle-4],cyc_y[i])<=1)
					)	//连续4个点与后面其中4个XY坐标不大于	1
				{
					log_printf("[near_large_round_360]found round1...\r\n");
					log_printf("%d(%d,%d,%5.3f),%d(%d,%d,%5.3f),%d(%d,%d,%5.3f),%d(%d,%d,%5.3f)\r\n",
							c_m_angle-1,cyc_x[c_m_angle-1],cyc_y[c_m_angle-1],m_angle[c_m_angle-1],
							c_m_angle-2,cyc_x[c_m_angle-2],cyc_y[c_m_angle-2],m_angle[c_m_angle-2],
							c_m_angle-3,cyc_x[c_m_angle-3],cyc_y[c_m_angle-3],m_angle[c_m_angle-3],
							c_m_angle-4,cyc_x[c_m_angle-4],cyc_y[c_m_angle-4],m_angle[c_m_angle-4]);
					log_printf("%d(%d,%d,%5.3f),%d(%d,%d,%5.3f),%d(%d,%d,%5.3f),%d(%d,%d,%5.3f)\r\n",
							i-3,cyc_x[i-3],cyc_y[i-3],m_angle[i-3],
							i-2,cyc_x[i-2],cyc_y[i-2],m_angle[i-2],
							i-1,cyc_x[i-1],cyc_y[i-1],m_angle[i-1],
							i,cyc_x[i],cyc_y[i],m_angle[i]);
					/*
					for(c_m_angle = 0;c_m_angle<MAX_C_M_ANGLE;c_m_angle++)
						m_angle[c_m_angle] = 0;
					c_m_angle = 0;
					*/
					return 1;
				}
			}
		}
		else
		{
			c_quadrant =count_quadrant(quadrant_1,quadrant2,quadrant3,quadrant4);	//取象限数
		}
		if(i==0)
			break;
	}
#else		///已测试OK，可以判断出正在绕大桩,配合旋转135度可以OK
	for(i=0;i<c_m_angle;i++)
	{						///已测试OK，可以判断出正在绕大桩但是会让机器继续绕圈圈
		if( m_angle[i] == 0)
			continue;
		if(m_angle[i] >=0 && m_angle[i] <=90)
			quadrant_1++;
		if(m_angle[i] >90 && m_angle[i] <=180)
			quadrant2++;
		if(m_angle[i] >180 && m_angle[i] <=270)
			quadrant3++;
		if((m_angle[i] >270 && m_angle[i] <=360) || m_angle[i] < 0)
			quadrant4++;
		//log_printf("quadrant=%3.1f,%d,%d,%d,%d\r\n",m_angle[i],quadrant_1,quadrant2,quadrant3,quadrant4);
		if(c_quadrant >=4 && i > 8)  //包含4个象限值  间隔大于8个点
		{
	//已测试OK
			if(  (disxy(get_quadrant(m_angle[0]),get_quadrant(m_angle[i-3]))<=1) && (disxy(get_quadrant(m_angle[1]),get_quadrant(m_angle[i-2]))<=1)
				&&	(disxy(get_quadrant(m_angle[2]),get_quadrant(m_angle[i-1]))<=1)  && (disxy(get_quadrant(m_angle[3]),get_quadrant(m_angle[i]))<=1)	)	//连续3个点在同一象限内
			{
				if( (disxy(cyc_x[0],cyc_x[i-3])<=1) && (disxy(cyc_y[0],cyc_y[i-3])<=1)
					&& (disxy(cyc_x[1],cyc_x[i-2])<=1) && (disxy(cyc_y[1],cyc_y[i-2])<=1)
					&& (disxy(cyc_x[2],cyc_x[i-1])<=1) && (disxy(cyc_y[2],cyc_y[i-1])<=1)
					&& (disxy(cyc_x[3],cyc_x[i])<=1) && (disxy(cyc_y[3],cyc_y[i])<=1)
					)	//连续4个点与后面其中4个XY坐标不大于	1
				{
					log_printf("[near_large_round_360]found round1...\r\n");
					log_printf("%d(%d,%d,%5.3f),%d(%d,%d,%5.3f),%d(%d,%d,%5.3f),%d(%d,%d,%5.3f)\r\n",
							0,cyc_x[0],cyc_y[0],m_angle[0],1,cyc_x[1],cyc_y[1],m_angle[1],2,cyc_x[2],cyc_y[2],m_angle[2],
							3,cyc_x[3],cyc_y[3],m_angle[3]);
					log_printf("%d(%d,%d,%5.3f),%d(%d,%d,%5.3f),%d(%d,%d,%5.3f),%d(%d,%d,%5.3f)\r\n",
							i-3,cyc_x[i-3],cyc_y[i-3],m_angle[i-3],i-2,cyc_x[i-2],cyc_y[i-2],m_angle[i-2],
							i-1,cyc_x[i-1],cyc_y[i-1],m_angle[i-1], i,cyc_x[i],cyc_y[i],m_angle[i]);
					/*
					for(c_m_angle = 0;c_m_angle<MAX_C_M_ANGLE;c_m_angle++)
						m_angle[c_m_angle] = 0;
					c_m_angle = 0;
					*/
					return 1;
				}
			}

		}
		else
		{
			c_quadrant =count_quadrant(quadrant_1,quadrant2,quadrant3,quadrant4);	//取象限数
		}
	}

#endif
//	log_printf("quadrant=%d,%d,%d,%d\r\n",quadrant_1,quadrant2,quadrant3,quadrant4);
	/*
	if(c_m_angle>=MAX_C_M_ANGLE)
	{
		for(i=0;i<c_m_angle;i++)
		{
			log_printf("%d(%d,%d,%5.3f)\r\n",i,cyc_x[i],cyc_y[i],m_angle[i]);
		}
	}
	*/
#if 0
	if(c_quadrant >=4 && (quadrant_1 >=3 || quadrant2 >=3 || quadrant3 >=3 || quadrant4>=3))
	{
		log_printf("[near_large_round_360]found round2...\r\n");
		/*
		for(c_m_angle = 0;c_m_angle<MAX_C_M_ANGLE;c_m_angle++)
				m_angle[c_m_angle] = 0;
		c_m_angle = 0;
		*/
		return 1;
	}
#endif
	return 0;

}

///绕小桩判断函数
//计算是否360度转圈圈
uint8_t near_round_360(float *m_angle,int16_t c_m_angle)
{
	int16_t i;
	uint8_t quadrant_1 = 0,quadrant2=0,quadrant3=0,quadrant4=0;
	uint8_t c_quadrant=0;
	log_printf("[near_round_360]\r\n");
	for(i=0;i<MAX_C_M_ANGLE;i++)
	{
		if( m_angle[i] == 0)
			continue;
		if(m_angle[i] >=0 && m_angle[i] <=90)
			quadrant_1++;
		if(m_angle[i] >90 && m_angle[i] <=180)
			quadrant2++;
		if(m_angle[i] >180 && m_angle[i] <=270)
			quadrant3++;
		if((m_angle[i] >270 && m_angle[i] <=360) || m_angle[i] < 0)
			quadrant4++;
		//log_printf("quadrant=%3.1f,%d,%d,%d,%d\r\n",m_angle[i],quadrant_1,quadrant2,quadrant3,quadrant4);
		if(c_quadrant >=4 && i > 3)
		{
 			if((get_quadrant(m_angle[0]) == get_quadrant(m_angle[i]) && get_quadrant(m_angle[0]) == get_quadrant(m_angle[i-1])) ||
 			   (get_quadrant(m_angle[1]) == get_quadrant(m_angle[i]) && get_quadrant(m_angle[0]) == get_quadrant(m_angle[i-1]))  ||
 			   (get_quadrant(m_angle[2]) == get_quadrant(m_angle[i]) && get_quadrant(m_angle[0]) == get_quadrant(m_angle[i-1]))  ||
 			    (get_quadrant(m_angle[4]) == get_quadrant(m_angle[i]) && get_quadrant(m_angle[0]) == get_quadrant(m_angle[i-1]))||
 			     (get_quadrant(m_angle[5]) == get_quadrant(m_angle[i]) && get_quadrant(m_angle[0]) == get_quadrant(m_angle[i-1])))
			{
				log_printf("found round...\r\n");
				for(c_m_angle = 0;c_m_angle<MAX_C_M_ANGLE;c_m_angle++)
					m_angle[c_m_angle] = 0;
				c_m_angle = 0;
				return 1;
			}
		}else
		{
			c_quadrant =count_quadrant(quadrant_1,quadrant2,quadrant3,quadrant4);	//取象限数
		}
	}
	log_printf("quadrant=%d,%d,%d,%d\r\n",quadrant_1,quadrant2,quadrant3,quadrant4);
	/**/
	if(c_quadrant >=4 && (quadrant_1 >=3 || quadrant2 >=3 || quadrant3 >=3 || quadrant4>=3))
	{
		log_printf("found round2...\r\n");
		for(c_m_angle = 0;c_m_angle<MAX_C_M_ANGLE;c_m_angle++)
				m_angle[c_m_angle] = 0;
		c_m_angle = 0;
		return 1;
	}
	return 0;
}

//转圈的标准函数
//dir 方向 左右 pwm 速度 agle 角度
// 0:超时
// 1:旋转OK
// 2:状态改变
uint8_t robot_turn_deg_getout_trouble(uint8_t dir,int pwm,float agle)
{
//	int pwm=600;
	//int dg_turn=deg - 2.5;
//	int c=0;

	float dis_agle,b_agle;
	float out_deg;
	int midl=0;

//	uint32_t t;
	uint8_t sState = sys->sState;;
	www_idleintel_com();
	printf_power_sta();
	log_printf("turn_charge,dir=%s,agle=%f\r\n",get_motor_sta(dir),agle);
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
//
	out_deg = NO_ANGLE_V;
//	t=0;
	coordinate_calcu(0);
	while(1)
	{
		//get_sensers(&sys->gSta);
		proc_uart_task();
		CHECK_NAVI_STA2();
		//laser_calc_move(navigat,MAX_LASR_DIST);
		if(sys->sState !=sState|| sys->sState == SYS_IDLE)
		{
			//break;
			log_printf("state change\r\n");
			return 2;
		}
#if LASER_SIDE
		laser_scan_dist(navigat,LASER_CALC_GO_FW);
#endif
		if(TIM5->CNT >=5000)
		{
			TIM5->CNT = 0;
			turn_round_pid(TURN_SPEED);
			www_idleintel_com();

			dis_agle = b_agle -sys->angle;
			if(dis_agle > 180)
				dis_agle = 360 - dis_agle;
			if(dis_agle <-180)
				dis_agle += 360;
			if(dis_agle >=agle || dis_agle <=-agle)
			{
				log_printf("dis_agle >=agle || dis_agle <=-agle:%f,%f\r\n",dis_agle,agle);
				//break;
				return 1;
			}
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
							log_printf("found V_angle=%3.1f,%d\r\n",sys->angle,*(navigat->near.pid->adc2));
							//break;
							return 0;			//这个是什么状态返回呢?确定一下!
						}
						//log_printf("\r\n");
					}
				}
			}else
			{
				if(disfloat(sys->angle,out_deg) > 5)	//再转10°
				{
					log_printf("VOK,v_agle=%3.1f\r\n",sys->angle);
					//break;
					return 1;			//旋转OK
				}
			}

		}
		if(sys->mState == GO_STOP)	////jzz 2018-04-28
		{
			motor_run(dir,pwm,0,0);//有BUG，会多跑
		}

		//log_printf("%3.1f,%3.1f\r\n",sys->angle,dis_agle);
	}
	//wy190528 下面不会执行
	/*motor_run(GO_STOP,0,0,0);                             ////后加入的,有问题吗？
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
	log_printf("..OK,c=%d,%d,agl=%3.1f\r\n",motor.c_left_hw,motor.c_right_hw,sys->angle);
	return 0;*/
}


unsigned char turn_check_bum_pro(unsigned char flag,unsigned char dir,int pwm,int turn_speed)
{
	if(flag ==0)
	{
		sys->c_escape = 0;
		return 0;
	}
	if(sys->gSta & MASK_BUM_ALL)
	{
		uint8_t retu=0;
		log_printf("turn bum,bum=(%d,%d,%d,%d)\r\n",sys->gSta & MASK_BUM_LEFT,sys->gSta & MASK_BUM_LEFT2,sys->gSta & MASK_BUM_RIGHT,sys->gSta & MASK_BUM_RIGHT2);
		gyro_mapwhl();
		if(dir == GO_RIGTH && sys->gSta & MASK_BUM_LEFT2) //须有反向转一下
		{
			log_printf("turn right and left front bum.turn left and back..\r\n");
			motor_turn_dir(GO_LEFT,TURN_DEG_PWM,TURN_DEG_PWM);
			turn_delay(30,turn_speed);
			retu = 1;
		}else if(dir == GO_LEFT && sys->gSta & MASK_BUM_RIGHT) //须有反向转一下
		{
			log_printf("turn left and right front bum.turn left and back..\r\n");
			motor_turn_dir(GO_RIGTH,TURN_DEG_PWM,TURN_DEG_PWM);
			turn_delay(30,turn_speed);
			retu = 1;
		}
		log_printf("bum and back...\r\n");
		motor_back_off(20);
		//转回去
		if(retu==1)
		{
			motor_turn_dir(dir,TURN_DEG_PWM,TURN_DEG_PWM);
			turn_delay(30,turn_speed);

		}
		sys->c_escape++;
//				esc_yaw = 	sys->angle;

		log_printf("turn_check_bum_pro break,c=%d...\r\n",sys->c_escape);
		//脱困了5次，则反方向旋转一下
		if((sys->c_escape  % 5) ==0)
		{
			log_printf("escape err,reverse_turn...\r\n");
		//	c_escape = 0;
			if(dir== GO_LEFT)
				motor_turn_dir(GO_RIGTH,600,TURN_DEG_PWM);
			else
				motor_turn_dir(GO_LEFT,600,TURN_DEG_PWM);
			turn_round_pid(0);
			turn_delay(50,turn_speed);
			navigat->angle = sys->angle;
			if((sys->c_escape  % 10) ==0)
			{
				log_printf("esp back off...\r\n");
				motor_back_off(200);
			}else
			{
				log_printf("esp fw....\r\n");
				motor_go_forwark(300,NO_SIDE_NEAR,NULL);
			}

		}
		motor_run(dir,pwm,0,0);

		turn_round_pid(0);
		return 1;
	}
	return 0;
}


void turn_to_deg(int deg)
{
	float to_agle = deg;
	int turn_dir =0;//,c_angle_ok=0,angle_ok;
	float yaw;
	uint8_t	sState = sys->sState;
//	char flage=0;
//	int c=0,hw1,hw2;
	uint32_t 	t_turn=0;
//	uint16_t t = 0;

//	int16_t reverse_turn=0;		//反向旋转的技术
	///uint32_t t=0;
if(sys->motor_power_off ==1)
 		MOTOR_POWER_ON();
	get_gyro_org();

	www_idleintel_com();
	yaw = sys->angle;

	CHECK_NAVI_STA();
	delay_ms_sensers(10);
//	get_gyro_org();
	printf_power_sta();
	log_printf("\r\n[turn_to_deg]sta=%d,angle=%3.1f(%3.1f),to=%d(",sys->sState,sys->angle,navigat->angle,deg);
	//delay_ms_sensers(20);

	//testmicony
	//delay_ms(300);
	if(yaw > 180 && deg ==0)
	{
		to_agle = 360;
	}
	if(deg == 270 && yaw < 90)
	{
			yaw +=360;
	}
	if(deg == 90 && yaw > 270)
	{
		yaw -=360;
	}

#if TURN_TO_DEG == 2
	if(yaw < to_agle)
	{
		turn_dir = GO_LEFT;
		to_agle -=sys->ajust_agle;
	}
	else
	{
		turn_dir = GO_RIGTH;
		to_agle +=sys->ajust_agle;
	}
#endif


#if TURN_TO_DEG == 0

	turn_dir = GO_LEFT;
	to_agle -=sys->ajust_agle;
#endif

#if TURN_TO_DEG == 1

	turn_dir = GO_RIGTH;
	to_agle +=sys->ajust_agle;
#endif
	if(disfloat(yaw, to_agle) <=sys->ajust_agle)
	{
		log_printf("the same angle\r\n");
		navigat->angle = deg;
		return ;
	}

	sys->near_wall = 0;
	coordinate_calcu(0);
	log_printf("%3.1f)%d,pwm=%d\r\n",to_agle,turn_dir,TURN_DEG_PWM);

	turn_round_pid(0);
    motor.c_left_hw = motor.c_right_hw = 0;
	//motor_turn_dir(turn_dir ,sys->left_pwm,cfg->go_forward_pwm);
	motor_turn_dir(turn_dir ,TURN_DEG_PWM,TURN_DEG_PWM);
	sys->right_pwm = sys->left_pwm = DEG_TURN_PWM;
	sys->near_wall = 0;
	navigat->is_walk = FALSE;
	navigat->c_turn++;


	turn_check_bum_pro(0,turn_dir,TURN_DEG_PWM,0);


	t_turn = msTmr;
	int turn_speed = TURN_SPEED;
//	float esc_yaw = 1000;
	uint32_t turn_timer = 0;
	while(1)
	{
		proc_uart_task();
		get_sensers(&sys->gSta);

		//=================判断退出机制=====================================
		if(sys->sState !=sState || sys->sState == SYS_IDLE)
		{
			log_printf("[turn_to_deg] state change\r\n");
			motor_run(GO_STOP,0,0,0);
			STOP_ALL_MOTOR();
			gyro_whlmap();
#if 1//SYS_VER == VER_KESMAN
			set_ir_alter_dis(0);
#endif
			break ;
		}


		turn_check_bum_pro(1,turn_dir,TURN_DEG_PWM,turn_speed);

		CHECK_NAVI_STA();
		CHECK_IDLE_STA();
		if(TIM5->CNT >=5000)		//5ms
		{
			TIM5->CNT = 0;

			www_idleintel_com();
			turn_round_pid(TURN_SPEED);//PID
			if(turn_timer++ >= 12000) //一分钟后，脱困失败
			{
				log_printf("escape err,t=%d\r\n",turn_timer);
				sys->t_beep_on = 10000;		//叫10秒钟
				sys->work_errcode |= MWERR_CANT_ESCAPE_WITHIN_TIMES;//add by wonton2004 20170118
				sys->sState = SYS_IDLE;
				break;
			}

			if(deg >= 270 && sys->angle < 90)
			{
				sys->angle +=360;
			}else if(deg == 90 && sys->angle > 270)
			{
				sys->angle -=360;
			}
			if(disfloat(sys->angle, deg) < sys->ajust_agle)			//当差值小于修正的角度,认为旋转到目标角度了
			{
				log_printf("turn ok,angle=%3.1f,%3.1f,%3.1f\r\n",sys->angle,deg,sys->ajust_agle);
				motor_run(GO_STOP,0,0,0);
				break;
			}
/*
			if(t++>=1000)		//10s 超时			7.5S 超时2018-08-20 jzz
			{
				log_printf("timeout,fw..\r\n");
				navigat->angle = sys->angle;
				motor_go_forwark(300,NO_SIDE_NEAR,NULL);
				motor_turn_dir(turn_dir ,TURN_DEG_PWM,TURN_DEG_PWM);	//BUG 会多转
	      		turn_round_pid(0);
	      		t = 0;
			}
			*/
			if(sys->mState == GO_STOP)
			{
				log_printf("found stop..turn again2...\r\n");
				motor_turn_dir(turn_dir ,TURN_DEG_PWM,TURN_DEG_PWM);	//BUG 会多转
	      		turn_round_pid(0);

			}
			///
		}
	}
#if LASER_EN
	calc_scan_laser(navigat,1);
#endif
#if LASER_SIDE ||  LASER_DRAW
	calc_scan_laser(navigat,0);		//只是矫正
#endif
	//proc_gyro_task(WAIT_STOP_CNT, WAIT_STOP_COUNT);				//等待陀螺仪稳定

	//	DELAY_MS(10);
	//delay_ms(20);
	delay_sensor(20);
	www_idleintel_com();
	//cord_calc_store(0);
	//gyro_whlmap();
	gyro_mapwhl();
	navigat->angle = deg;

	ir_dock_insight(1);
	sys->near_wall = 0;
	log_printf("     OK,angle=%f(%f)%d,%d,xy=(%d,%d,%d,%d,%3.3f,%3.3f)\r\n\r\n",sys->angle,navigat->angle,msTmr - t_turn,motor.c_left_hw
						,X_NOW,Y_NOW,navigat->x_org,navigat->y_org,navigat->x_org_f,navigat->y_org_f);

	//testmicony
	//delay_ms(300);
}

//转圈的标准函数
//dir 方向 左右 pwm 速度 agle 角度
uint8_t robot_turn_deg(uint8_t dir,int pwm,float agle)
{
//	int pwm=600;
	//int dg_turn=deg - 2.5;
//	uint16_t t=0;

	float dis_agle,b_agle;
	float out_deg;
	int midl=0;
	//uint16_t c_escape = 0;		//脱困次数
	///uint32_t t;
	uint8_t sState = sys->sState;;
	www_idleintel_com();
	printf_power_sta();
	log_printf("turn_charge,dir=%s,agle=%f\r\n",get_motor_sta(dir),agle);
	motor.c_left_hw = motor.c_right_hw = 0;
	//log_printf("1\r\n");
	CHECK_NAVI_STA2();
if(sys->motor_power_off ==1)
 		MOTOR_POWER_ON();
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

//uint32_t t = 0;
//	last_turn_stat.turn_agle = agle;
	turn_check_bum_pro(0,dir,0,0);
	int turn_speed = TURN_SPEED;
//	float esc_yaw=1000;
	uint32_t turn_timer = 0;
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
			if(turn_timer++ >= 12000) //一分钟后，脱困失败
			{
				log_printf("escape err,t=%d\r\n",turn_timer);
				sys->t_beep_on = 10000;		//叫10秒钟
				sys->work_errcode |= MWERR_CANT_ESCAPE_WITHIN_TIMES;//add by wonton2004 20170118
				sys->sState = SYS_IDLE;
				break;
			}
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
				log_printf("found stop..turn again2...\r\n");
				motor_run(dir,pwm,0,0);
				turn_round_pid(0);

			}

		}
		if(sys->mState == GO_STOP)	////jzz 2018-04-28
		{
			motor_run(dir,pwm,0,0);//有BUG，会多跑
				turn_round_pid(0);
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
	log_printf("..OK,c=%d,%d,agl=%3.1f\r\n",motor.c_left_hw,motor.c_right_hw,sys->angle);
	return 0;
}
//掉头180度
#if	ARC_Z_GO

short turn_back_180(void)
{
	float yaw,deg,dis;
	uint8_t dir = 0;
	uint32_t c_test_time = 0;
	uint32_t	t_turn=0;

	get_gyro_org();

	www_idleintel_com();

	CHECK_NAVI_STA_RT(0);
	//delay_ms_sensers(10);
	//计算出掉头的方向
	if(navigat->lst_angle== 0)
	{
		yaw = 180;
		if(navigat->side == LEFT_SIDE)
			dir = GO_LEFT;
		else
			dir = GO_RIGTH;
	}else
	{
		yaw = 0;
		if(navigat->side == RIGHT_SIDE)
			dir = GO_LEFT;
		else
			dir = GO_RIGTH;
	}
	get_gyro_org();
	printf_power_sta();
	log_printf("\r\n[turn_back_180]sta=%d,angle=%3.1f,last angle=%3.1f,to angle=%3.1f,dir=%d\r\n",
		sys->sState,sys->angle,navigat->lst_angle,yaw,dir);

	sys->near_wall = 0;
	coordinate_calcu(0);
//	turn_round_pid(0);

	sys->right_pwm = sys->left_pwm = DEG_TURN_PWM;
	sys->near_wall = 0;
	navigat->is_walk = FALSE;
	navigat->c_turn++;

	motor.c_left_hw = motor.c_right_hw = 0;
	if(dir == GO_RIGTH)
	{
		motor_wheel_stop(LEFT_WHEEL);
		log_printf("stop LEFT_WHEEL\r\n");
		//motor_wheel_forward(RIGHT_WHEEL, 700);
	}
	else
	{
		motor_wheel_stop(RIGHT_WHEEL);
		log_printf("stop RIGHT_WHEEL\r\n");
		//motor_wheel_forward(LEFT_WHEEL, 700);
	}

	while(1)
	{
		proc_uart_task();
		get_sensers(&sys->gSta);

		if(sys->sState == SYS_IDLE)
			return 0;
		CHECK_NAVI_STA_RT(0);
	//	CHECK_IDLE_STA();
		//wy190527 新增左右两侧碰撞开关检测
		if(sys->gSta & (MASK_BUM_LEFT | MASK_BUM_LEFT2 | MASK_BUM_RIGHT | MASK_BUM_RIGHT2))
		{
			motor_run(GO_STOP,0,0,0);
			www_idleintel_com();
			coordinate_calcu(0);
			gyro_mapwhl();
			log_printf("bum,gsta=%d\r\n",sys->gSta);
			return 0;
		}
		if(TIM5->CNT >=5000)			//5ms
		{
			TIM5->CNT = 0;

			c_test_time++;
			if(c_test_time>2000)		//////10s  超时,这个应该为半圈+1圈的时间这样是不是可以保证角度准确 jzz 2018-06-13
			{
				log_printf("turn_back_180 c_test_time out\r\n");
				return 0;
				}

			www_idleintel_com();
			deg = sys->angle;

			if(deg >= 360 && deg < 320)
			{
				deg -=360;
			}

			//if(disfloat(deg,yaw) < sys->ajust_agle)
			dis = deg - yaw;
			//log_printf("%3.1f,%3.1f\r\n",deg,yaw);
			//if(dis < 0.7 && dis > -0.7)
			if(dis < 0.7 && dis > -0.7)		//判断在多少的范围内停下来
			{
				//motor_run(GO_STOP,0,0,0);
				//delay_ms(200);
				//ajust_angle_pid(turn_dir,deg,flage);
				log_printf("turn over,c_test_time:%d\r\n",c_test_time);
				c_test_time = 0;
				break;
			}
		}
	 	if(sys->mState == GO_STOP)////////jzz 2018-04-28
		{
			 if(dir == GO_RIGTH)
			 {
				 motor_wheel_stop(LEFT_WHEEL);
				// log_printf("stop LEFT_WHEEL\r\n");
				 //motor_wheel_forward(RIGHT_WHEEL, 700);
			 }
			 else
			 {
				 motor_wheel_stop(RIGHT_WHEEL);
				 //log_printf("stop RIGHT_WHEEL\r\n");
				 //motor_wheel_forward(LEFT_WHEEL, 700);
			 }
		}
	}

	navigat->angle = yaw;
	www_idleintel_com();
	coordinate_calcu(0);														//计算出原始的坐标系

	coord_org2map(navigat->x_org,navigat->y_org,&navigat->tx,&navigat->ty);	//坐标系转换

	gyro_mapwhl();
//navigat->angle = deg;

	ir_dock_insight(1);
	sys->near_wall = 0;
	log_printf("\r\nOK,angle=%f(%f)%d,%d,xy=(%d,%d,%d,%d,%3.3f,%3.3f)\r\n\r\n",sys->angle,navigat->angle,msTmr - t_turn,motor.c_left_hw
						,X_NOW,Y_NOW,navigat->x_org,navigat->y_org,navigat->x_org_f,navigat->y_org_f);
	return 1;
}
#endif

/*
转到平行墙的角度
*/
#if TURN_90_V

#define TURNV_TIME_MIN	120
#define TURNV_TIME_MAX	600
#define TURNV_ANGLE_MAX	4

#if TURN_V_ADJ
struct __turnv_angle{
//	uint8_t id;//当前角度差的序号0-255循环计数
	//uint8_t valid;//数据是否有效1为有效,0为无效
	float angle_err;//记录角度差
	uint32_t time_rec;//当前的时间戳
};
static struct __turnv_angle turnv_angle[TURNV_ANGLE_MAX];

static uint8_t turnv_angle_tail = 0;

#define MAX_ADJ_A_V		3
uint8_t adj_a_v(void)
{
	float tt = 0;
	int c=0,k;
	log_printf("[adj_a_v]:\r\n");
	for(int16_t i=0;i<TURNV_ANGLE_MAX;i++)
	{
		//10分钟内
		if(turnv_angle[i].time_rec > 0 && turnv_angle[i].time_rec <= sys->t_navi_work &&
		 (sys->t_navi_work - turnv_angle[i].time_rec) < 600)
		 {
			tt += turnv_angle[i].angle_err;
			c++;
			log_printf("%d,%3.3f,%d\r\n",i,turnv_angle[i].angle_err,c);
		 }

	}
	//超过5个则可计算
	if(c >=TURNV_ANGLE_MAX)
	{
		tt /= c;		//取到平均数
		log_printf("avr=%3.3f\r\n",tt);
		k = 0;
		//范围不对
		if((tt <5 && tt>-5) || tt > 20 || tt < -20)
		{
			log_printf("tterr\r\n");
			return 0;
		}
		for(int16_t i=0;i<TURNV_ANGLE_MAX;i++)
		{
			//10分钟内
			if(turnv_angle[i].time_rec > 0 && turnv_angle[i].time_rec <= sys->t_navi_work &&
			 (sys->t_navi_work - turnv_angle[i].time_rec) < 600)
			 {

				if(disf_xy(turnv_angle[i].angle_err,tt) < 5)	//5度以内的偏差
					k++;
			 }
		}
		 log_printf("k=%d\r\n",k);
		 //个数相同
		// if(k==c && c>=4)
		if(k >=TURNV_ANGLE_MAX)
		 {
		 	uint16_t percent;
		 	int16_t deg = (int16_t)(sys->angle + tt);
		 	/*
			if(c >=8)
				percent	= 60;
			else if(c>=5)
				percent = 40;
			else
				percent = 20;
			*/
			percent = 50;
			log_printf("adjk:%d,%d,%3.3f\r\n",percent,deg,tt);
			illegal_copy_tell_13757122544_gsend(0x02,deg, percent);
		 }


	}
	return 0;

}
#endif



////非测距红外最早的版本替换这个函数，可以完成旋转与边是垂直状态 使用action-沿边旋转可以垂直.c中turn_to_deg_v替换
//这个函数有问题
void turn_to_deg_v(int deg)
{
	float to_agle = deg;
	int turn_dir =0;//,c_angle_ok=0,angle_ok;
//	short idx1=100;//,idx2=100;
//	uint8_t sta = 0;
//	float ba = sys->angle;
	uint16_t max_adc_left=0;//,less_left=0;
	uint16_t max_adc_right=0;//,less_right=0;
	uint16_t max_adc_midle=0;
	uint16_t last_left=0,last_right=0;
	uint16_t c_shake_left = 0,c_shake_right = 0;
	float v_angle_left=0,v_angle_right=0,v_angle_midle=0;
	uint8_t sState = sys->sState ;
	uint8_t must_v = 0 ;	//是否要垂直算法
	uint16_t count1=0,count2=0;//,count3=0;
	float turn_ofset=0;
	uint8_t turn_ok=0;
//	uint16_t pesent = 20;
	uint16_t left_vary = 0,right_vary=0;
//	char flage=0;
//	int c=0,hw1,hw2;
	uint32_t	t_turn=0;
	uint32_t t=0;
	float yaw;
	uint8_t n_shake_left = 0,n_shake_right = 0;//在捕捉变化趋势时,有反向变化或抖动,则此计数器加1,大于3时,将趋势变化的计数器清0,重头算过

	log_printf("\r\n[turn_v]angle=%3.1f(%3.1f),to=%d\r\n",sys->angle,navigat->angle,deg);
	delay_ms_sensers(50);
	get_gyro_org();
	delay_ms_sensers(50);
	CHECK_NAVI_STA();
	www_idleintel_com();
	yaw = sys->angle;

	if(yaw > 180 && deg ==0)
	{
		to_agle = 360;
	}
	if(deg == 270 && yaw < 90)
	{
			yaw +=360;
	}
	if(deg == 90 && yaw > 270)
	{
		yaw -=360;
	}

#if TURN_TO_DEG == 0
	turn_dir = GO_LEFT;
	to_agle -=sys->ajust_agle;
#endif

#if TURN_TO_DEG == 1
	turn_dir = GO_RIGTH;
	to_agle +=sys->ajust_agle;
#endif
	if(disfloat(yaw, to_agle) <=sys->ajust_agle)

	{
		log_printf("the same angle\r\n");
		return ;
	}

	uint8_t c_m_adc=0;
	if(sys->g_sta[2] >= 300 )//600 -> 350// modified by wonton2004 20170519
		c_m_adc++;

	if(sys->g_sta[3] >= 300 )//600 -> 350// modified by wonton2004 20170519
		c_m_adc++;

	if(sys->g_sta[4] >= 300)//600 -> 350// modified by wonton2004 20170519
		c_m_adc++;
//	pesent = 10;
	if(MIDLE_HAVE_IRDA3() && c_m_adc >=2 && (sys->gsta & MASK_BUM_MIDL) == 0)		//中间3个红外大于阈值 must_v=1
	{
		must_v = 1;
		//判断是不是一条线,0度，180度走法的工字型走法，判断前方是否是一条直线
		if(navigat->angle ==0 || navigat->angle == 180)
		{
			if( (OBST_XY(X_NOW +1,Y_NOW-1) && OBST_XY(X_NOW +1,Y_NOW-2) ) || //一条线
				(OBST_XY(X_NOW +1,Y_NOW+1) && OBST_XY(X_NOW +1,Y_NOW+2) ) ||
				(OBST_XY(X_NOW -1,Y_NOW+1) && OBST_XY(X_NOW +1,Y_NOW+2) ) ||
				(OBST_XY(X_NOW -1,Y_NOW-1) && OBST_XY(X_NOW +1,Y_NOW-2) ) ||
				(OBST_XY(X_NOW ,Y_NOW+1) && OBST_XY(X_NOW ,Y_NOW+2) ) ||
				(OBST_XY(X_NOW ,Y_NOW-1) && OBST_XY(X_NOW ,Y_NOW-2) ) )
			 {
				log_printf("[turn_v]aline\r\n");
//				pesent = 20;
			 }
		}
	}
	else
	{
		must_v = 0; 			//不需要垂直算法
		log_printf("[turn_v]not midle,%d,%d,%d,%d,%d,%d,%d\r\n",sys->g_sta[0],sys->g_sta[1],sys->g_sta[2],sys->g_sta[3]
																,sys->g_sta[4],sys->g_sta[5],sys->g_sta[6]);
	}
	/*
#if  NOT_TURN_DEG_V
	must_v = 0;
#endif
*/
	//must_v = 1;//test
	to_agle = deg;
	if(must_v )
		turn_ofset = 20;
	else
		turn_ofset = 0;
#if TURN_TO_DEG == 2
	if(yaw < to_agle)
	{
		turn_dir = GO_LEFT;
		to_agle += turn_ofset;		//左转20°
	}
	else
	{
		turn_dir = GO_RIGTH;
		to_agle -=turn_ofset;		//多转20度
	}
#endif
	sys->near_wall = 0;
	coordinate_calcu(0);
	log_printf("dir=%d,pwm(%d)mustv=%d(%3.1f)\r\n",turn_dir,TURN_DEG_PWM,must_v,to_agle);
	turn_round_pid(0);
    motor.c_left_hw = motor.c_right_hw = 0;
	motor_turn_dir(turn_dir ,TURN_DEG_PWM,TURN_DEG_PWM);
	sys->right_pwm = sys->left_pwm = TURN_DEG_PWM;
	sys->near_wall = 0;
	navigat->is_walk = FALSE;
	navigat->c_turn++;
	//motor.c_left_hw = motor.c_right_hw = 0;
	t_turn = msTmr;
	while(1)
	{
		proc_uart_task();
		get_sensers(&sys->gSta);
#if LASER_EN || LASER_SIDE
		laser_scan_dist(navigat,LASER_CALC_GO_FW);
#endif
		//=================判断退出机制=====================================2018-04-28
		if(sys->sState !=sState)
		{
			log_printf("[turn_to_deg_v]!!SYS_DOCK\r\n");
			//motor_run(GO_STOP,0,0,0);
			STOP_ALL_MOTOR();
			//gyro_whlmap();
#if 1//SYS_VER == VER_KESMAN
			//set_ir_alter_dis(0);
#endif
			return ;
		}
		CHECK_NAVI_STA();
		if(TIM5->CNT >=1000)
		{
			TIM5->CNT = 0;
			count1++;
			count2++;
			if(count1 >=5)//5->20
			{
				www_idleintel_com();

				turn_round_pid(TURN_SPEED);//PID
				count1 = 0;

				if(deg >= 270 && sys->angle < 90)
				{
					sys->angle +=360;
				}else if(deg == 90 && sys->angle > 270)
				{
					sys->angle -=360;
				}
				if(disfloat(sys->angle, to_agle) < sys->ajust_agle)
				{
					motor_run(GO_STOP,0,0,0);
					//delay_ms(200);
					//ajust_angle_pid(turn_dir,deg,flage);
					log_printf("OK3,%3.3f->%3.3(%3.3f)f\r\n",sys->angle, to_agle,sys->ajust_agle);
					//while(1);
					break;
				}

				if(disfloat(sys->angle, deg) < sys->ajust_agle)
				{
					turn_ok = 1;		//到点了，先记录一下，万一垂直失败，可以停下来了。
				}
				/*
				log_printf("%d,%d,%d,%d,%d,%d,%d,%3.1f\r\n",sys->g_sta[0],sys->g_sta[1],sys->g_sta[2],sys->g_sta[3]
													  ,sys->g_sta[4],sys->g_sta[5],sys->g_sta[6],sys->angle);
				*/
				//log_printf("%d,%d,%3.1f\r\n",sys->g_sta[0],sys->g_sta[6],sys->angle);

			}
			//======================每隔两秒钟判断一下侧面红外，判断垂直方向=======================
			//寻找最大的点
			/*
			log_printf("%d,%d,%d,%d,%d,%d,%d,%3.1f\r\n",sys->g_sta[0],sys->g_sta[1],sys->g_sta[2],sys->g_sta[3]
																  ,sys->g_sta[4],sys->g_sta[5],sys->g_sta[6],sys->angle);
			*/
			//log_printf("%d,%d,%3.1f\r\n",sys->g_sta[5],sys->g_sta[6],sys->angle);
			if(count2>=8)//3->20
			{
				count2 = 0;
				//log_printf("\r\n%d\t%d\t%3.1f\t%d\t%d\t",sys->g_sta[0],sys->g_sta[6],sys->angle,
											//max_adc_right,c_shake_right);
				//							max_adc_left,c_shake_left);

			#if 0
				//中间的最大值
				//if(MIDLE_ADC() > 400 && MIDLE_ADC() < 4000)
				if(MIDLE_ADC() > 1350 && MIDLE_ADC() < 4000)// modified by wonton2004 20170519
				{
					if(max_adc_midle < MIDLE_ADC()) 	//找到最大的红外
					{
						max_adc_midle = MIDLE_ADC();
						v_angle_midle= sys->angle;
					}
				}
				//左边最大的值
				//if(LEFT_ADC() > 400 && LEFT_ADC() < 4000)
				if(LEFT_ADC() > 350 && LEFT_ADC() < 4000)// modified by wonton2004 20170519
				{
					if(max_adc_left< LEFT_ADC())		//找到最大的红外
					{
						max_adc_left = LEFT_ADC();
						v_angle_left= sys->angle;
						less_left = 0;
					}
				}
				//右边最大的值
				//if(RIGHT_ADC() > 400 && RIGHT_ADC() < 4000)
				if(RIGHT_ADC() > 350 && RIGHT_ADC() < 4000)// modified by wonton2004 20170519
				{
					if(max_adc_right< RIGHT_ADC())
					{
						max_adc_right= RIGHT_ADC();
						v_angle_right= sys->angle;

						less_right = 0;
					}
				}
			#else
				if(must_v)
				{
					//if(MIDLE_ADC() > 400 && MIDLE_ADC() < 4000)
					if(MIDLE_ADC() > 1350 && MIDLE_ADC() < 4000)// modified by wonton2004 20170519
					{
						if(max_adc_midle < MIDLE_ADC()) 	//找到最大的红外
						{
							max_adc_midle = MIDLE_ADC();
							v_angle_midle= sys->angle;
						}
					}
					//左边最大的值
					//if(LEFT_ADC() > 400 && LEFT_ADC() < 4000)
					if(LEFT_ADC() > 1000
						&& last_left > 0 && disxy(last_left,LEFT_ADC()) > 20)
					//if(LEFT_ADC() > 1350 && LEFT_ADC() < 4000)// modified by wonton2004 20170519
					{
						if(max_adc_left< LEFT_ADC())		//找到最大的红外
						{
							//log_printf("%%\t");			/////
							max_adc_left = LEFT_ADC();
							v_angle_left= sys->angle;
//							less_left = 0;
							left_vary = 0;
							if(c_shake_left ++ > 10)//有持续的红外值变大时,left_vary = 1，表示机器正在接近墙壁
							{
								if(left_vary == 0)
								{
									//log_printf("left_vary = 1\r\n");
									//motor_run(GO_STOP,0,0,0);
									//while(1);
									//TargetSysReset();
									left_vary = 1;
								}
							}
						}
						else if(max_adc_left >= LEFT_ADC())//当红外值小于最大值时,需要查看机器是否正在远离墙壁
						{
							if(disxy(last_left,LEFT_ADC()) > 100 && c_shake_left > 3)//存在阶跃信号,要重头算过
							{
								c_shake_left = 0;
								n_shake_left = 0;
								left_vary = 0;
								log_printf("left slot!");
									//delay_ms(5);
									delay_sensor(5);
								//motor_run(GO_STOP,0,0,0);
								//TargetSysReset();
								//break;
							}
							else
							{
								if(c_shake_left > 3)
								{
									if(n_shake_left ++ > 3)
										c_shake_left = 0;
								}
								else
									c_shake_left = 0;//红外值小于最大值时,此变量清0
								if(c_shake_left == 0)
									n_shake_left = 0;
								/*if(left_vary != 0)//当之前已经靠近过墙壁时,红外值小于最大记录值,表示机器正在远离墙壁
													//此时记录个数,及时停止旋转
								{
									if(left_vary ++ > 5)//红外值持续变大超过n个,机器停止
									{
									}
								}*/
							}
						}

						if(left_vary)
						{
							if(disxy(last_left,LEFT_ADC()) < 10)		//变化后，又平缓了，则不是直线了
							{
								left_vary++;
								if(left_vary >= 10)
								{
									log_printf("Verr1\r\n");
									must_v = 0;
									to_agle = deg;
									if(turn_ok) 	//如果转到了，就停下来了
									{
										log_printf("have been OK\r\n");
										motor_run(GO_STOP,0,0,0);
										//while(1);
										break;
									}
								}
							}
							else
							{
								if(max_adc_left >= LEFT_ADC())//当红外值小于最大值时,需要查看机器是否正在远离墙壁
								{
									//log_printf("$$\t");			////
									if(left_vary ++ > 3)//红外值持续变大超过n个,机器停止
									{
										motor_run(GO_STOP,0,0,0);
										log_printf("OK1\r\n");
										must_v = 3;
											//while(1);
										break;
									}
								}
							}
						}
					}
					last_left = LEFT_ADC();
					//右边最大的值
					//if(RIGHT_ADC() > 400 && RIGHT_ADC() < 4000)
					//log_printf("##");
					if(RIGHT_ADC() > 1000 &&
						last_right> 0 && disxy(last_right,RIGHT_ADC()) > 20)
					//if(RIGHT_ADC() > 1350 && RIGHT_ADC() < 4000)// modified by wonton2004 20170519
					{
					//log_printf("$$");
						if(max_adc_right< RIGHT_ADC())
						{
							max_adc_right= RIGHT_ADC();
							v_angle_right= sys->angle;
							//less_right = 0;
							right_vary = 0;
							if(c_shake_right ++ > 10)//有持续的红外值变大时,left_vary = 1，表示机器正在接近墙壁
							{
								if(right_vary == 0)
								{
									//log_printf("right_vary = 1\t");
									//delay_ms(5);
									delay_sensor(5);
									//motor_run(GO_STOP,0,0,0);
									//while(1);
									//TargetSysReset();
									right_vary = 1;
								}
							}
						}
						else if(max_adc_right >= RIGHT_ADC())//当红外值小于最大值时,需要查看机器是否正在远离墙壁
						{
							if(disxy(last_right,RIGHT_ADC()) > 100 && c_shake_right > 3)//存在阶跃信号,要重头算过
							{
								c_shake_right = 0;
								n_shake_right = 0;
								right_vary = 0;
								log_printf("right slot!");
									//delay_ms(5);
									delay_sensor(5);
								//motor_run(GO_STOP,0,0,0);
								//TargetSysReset();
								//break;
							}
							else
							{
								if(c_shake_right > 3)
								{
									if(n_shake_right ++ > 3)
										c_shake_right = 0;
								}
								else
									c_shake_right = 0;//红外值小于最大值时,此变量清0
								if(c_shake_right == 0)
									n_shake_right = 0;
								/*if(left_vary != 0)//当之前已经靠近过墙壁时,红外值小于最大记录值,表示机器正在远离墙壁
													//此时记录个数,及时停止旋转
								{
									if(left_vary ++ > 5)//红外值持续变大超过n个,机器停止
									{
									}
								}*/
							}
						}

						if(right_vary)
						{
							if(disxy(last_right,RIGHT_ADC()) < 1)		//变化后，又平缓了，则不是直线了
							{
								right_vary++;
								if(right_vary >=10)
								{
									log_printf("Verr2\r\n");
									must_v = 0;
									to_agle = deg;
									if(turn_ok) 	//如果转到了，就停下来了
									{
										log_printf("have been OK\r\n");
										motor_run(GO_STOP,0,0,0);
										//TargetSysReset();
										break;
									}
								}
							}
							else
							{
								if(max_adc_right > RIGHT_ADC() && c_shake_right == 0)//当红外值小于最大值时,需要查看机器是否正在远离墙壁
								{
									if(right_vary ++ > 3)//红外值持续变大超过n个,机器停止
									{
										motor_run(GO_STOP,0,0,0);
										log_printf("OK2-max=%d-ss:%d\r\n",max_adc_right,c_shake_right);
										must_v = 3;
										//	while(1);
										break;
									}
								}
							}
						}
					}
					last_right = RIGHT_ADC();
				}
				//log_printf("\r\n");
			#endif

			#if 0
				if(must_v)
				{

					//左侧有红外	,则先找到最大的红外
					if(LEFT_ADC() > 200 && LEFT_ADC() < 4000)
					{
						//log_printf("%d\r\n",LEFT_ADC());
						//判断是不是正常的红外，如果有突变，就不是正常的红外
						if(last_left > 0 && disxy(last_left,LEFT_ADC()) > 200)
						{
							//变化了
							//if(left_vary ==0)
							left_vary = 1;
							if(c_shake_left ++ >=6) 	// 3	次突变
							{
								log_printf("Verr\r\n");
								must_v = 0;
								to_agle = deg;
								if(turn_ok) 	//如果转到了，就停下来了
								{
									log_printf("have been OK\r\n");
									motor_run(GO_STOP,0,0,0);
									while(1);
									break;
								}
							}
						}
						if(left_vary)
						{
							if(disxy(last_left,LEFT_ADC()) < 90)		//变化后，又平缓了，则不是直线了
							{
								left_vary++;
								if(left_vary >=10)
								{
									log_printf("Verr1\r\n");
									must_v = 0;
									to_agle = deg;
									if(turn_ok) 	//如果转到了，就停下来了
									{
										log_printf("have been OK\r\n");
										motor_run(GO_STOP,0,0,0);
										//while(1);
										break;
									}
								}
							}
						}
						last_left = LEFT_ADC();

						//发现最大的红外后，再变小，从大变到小
						if(LEFT_ADC() < max_adc_left && LEFT_ADC() > 900 && max_adc_left < 4000)
						{
							less_left++;
							if(less_left >=105)				//连续6个变小
							{
								motor_run(GO_STOP,0,0,0);
								log_printf("OK2\r\n");
								must_v = 3;
								n_adc = LEFT_ADC();//	while(1);
								break;
							}
						}else
							less_left = 0;
					}
					// right adc
					if(RIGHT_ADC() > 200 && RIGHT_ADC() < 4000)
					{
						//log_printf("%d\r\n",RIGHT_ADC());
						if(last_right> 0 && disxy(last_right,RIGHT_ADC()) > 200)
						{
							//变化了
							//if(right_vary ==0)
							right_vary = 1;
							if(c_shake_right ++ >=3)
							{
								log_printf("Verr\r\n");
								must_v = 0;
								to_agle = deg;
								if(turn_ok) 	//如果转到了，就停下来了
								{
									log_printf("have been OK\r\n");
									motor_run(GO_STOP,0,0,0);
									//while(1);
									break;
								}
							}
						}
						if(right_vary)
						{
							if(disxy(last_right,RIGHT_ADC()) < 90)		//变化后，又平缓了，则不是直线了
							{
								right_vary++;
								if(right_vary >=10)
								{
									log_printf("Verr3\r\n");
									must_v = 0;
									to_agle = deg;
									if(turn_ok) 	//如果转到了，就停下来了
									{
										log_printf("have been OK\r\n");
										motor_run(GO_STOP,0,0,0);
									//while(1);
										break;
									}
								}
							}
						}
						last_right = RIGHT_ADC();

						if(RIGHT_ADC() < max_adc_right && RIGHT_ADC() > 400 && max_adc_right < 4000)
						{
							less_right++;
							if(less_right >=100)

							{
								log_printf("OK1\r\n");
								motor_run(GO_STOP,0,0,0);
								must_v = 4;
								n_adc = RIGHT_ADC();	//while(1);
								break;
							}
						}else

							less_right = 0;


					}
				}
			#endif
			}
		}
#if TURN_TIMEOUT_EN
		if(t++>=6000)		//15s 超时			7.5S 超时2018-08-20 jzz
		{
			log_printf("timeout\r\n");
			//return 0;
			break;
		}
#endif
		if(sys->mState == GO_STOP)////////jzz 2018-04-28
		{
		//	motor_run(GO_FORWARD,sys->pwm ,0,0);///
			//log_printf("get_sensers  ----sys->mState == GO_STOPr\n");
      		motor_turn_dir(turn_dir ,TURN_DEG_PWM,TURN_DEG_PWM);;	//BUG 会多转
		}
	}

/*
								log_printf("---------1\r\n");
								motor_run(GO_STOP,0,0,0);
								TargetSysReset();
									//while(1);
								*/
	delay_ms_sensers(100);
	www_idleintel_com();
	//cord_calc_store(0);
	//gyro_whlmap();
	gyro_mapwhl();
	navigat->angle = deg;
	sys->near_wall = 0;
#if TURN_V_ADJ
#if 1
	if(must_v >=3)
	{
		log_printf("Vok ajust,now =%3.3f -> %d,",sys->angle,deg);
		float dis;
		uint8_t ko=0;
		//取到差值
		//90度方向
		dis = 90 - sys->angle;
		if(dis < 25 && dis >-25)
		{
			ko=1;
		}else
		{
			dis = 270 - sys->angle;
			if(dis < 25 && dis >-25)
			{
				ko=1;
			}
		}
		if(ko )
		{
			if(turnv_angle_tail >= TURNV_ANGLE_MAX)turnv_angle_tail = 0;
			turnv_angle[turnv_angle_tail].angle_err = dis;
			turnv_angle[turnv_angle_tail ++].time_rec = sys->t_navi_work;
			log_printf("VOK,anlge=%3.3f,dis=%3.3f,tmr=%d,c=%d\r\n",sys->angle,dis,sys->t_navi_work,turnv_angle_tail);
			adj_a_v();
		}
	}
#else
	if(must_v >=3)
	{
		//把当前的角度全部转换到90度附近,再作记录
		float n_angle_err = 0.0;
		uint32_t n_valid_time;
		uint8_t i;

		n_angle_err = (sys->angle > 230.0 ? 270.0:90.0) - sys->angle;
		//turnv_angle[turnv_angle_ct ++]. = (int16_t)(n_angle_err*100);
		turnv_angle[turnv_angle_tail].valid = 1;
		turnv_angle[turnv_angle_tail].angle_err = (int16_t)(n_angle_err*100);
		turnv_angle[turnv_angle_tail].id = turnv_angle[turnv_angle_tail?(turnv_angle_tail - 1):(TURNV_ANGLE_MAX - 1)].id + 1;
		log_printf("[turn_to_deg_v]-----\t%d-%3.1f-%d\r\n",turnv_angle_ct,sys->angle,turnv_angle[turnv_angle_tail].angle_err);
		turnv_angle[turnv_angle_tail ++].time_rec = sys->t_navi_work;
		if(turnv_angle_tail >= TURNV_ANGLE_MAX)turnv_angle_tail = 0;

		if(turnv_angle_tail == turnv_angle_head)
		{
			turnv_angle_head = (turnv_angle_head == (TURNV_ANGLE_MAX - 1)) ? 0:(turnv_angle_head + 1);
			turnv_angle_ct = 0;
		}
		else
			turnv_angle_ct ++;
		//if(turnv_angle_ct == 1)
		//{
		//	turnv_timerec = sys->t_navi_work;//这里同步一下时间,便于以后计算时间差
		//}

		n_valid_time = (sys->t_navi_work > TURNV_TIME_MAX)?(sys->t_navi_work - TURNV_TIME_MAX):0;

		for(i = turnv_angle_head;i != turnv_angle_tail;)//查找一遍所有数据,将超过TURNV_TIME_MAX的数据设为无效
		{
			if(turnv_angle[i].time_rec < n_valid_time)
			{
				turnv_angle[i].valid = 0;
				turnv_angle_ct = turnv_angle_ct ? (turnv_angle_ct - 1):0;
				turnv_angle_head = (turnv_angle_head == (TURNV_ANGLE_MAX - 1)) ? 0:(turnv_angle_head + 1);
			}
			i = (i == (TURNV_ANGLE_MAX - 1)) ? 0:(i + 1);
		}
		if(sys->t_navi_work > turnv_angle[turnv_angle_head].time_rec && (sys->t_navi_work - turnv_angle[turnv_angle_head].time_rec) > TURNV_TIME_MIN)//5分钟以上,允许校正角度
		{
			if(turnv_angle_ct > 5)
			{
				int16_t n_sum = 0;
				//uint8_t i;
				uint8_t n_invalid_ct = 0;//用来记录无效数据的个数,作为评估是否需要校正角度的依据,无效数据过多就放弃这次校正
				float n_angle_correct = 0;
				//uint8_t n_angle_tail;

				log_printf("\r\n");
				for(i = turnv_angle_head;i != turnv_angle_tail;)
				{
					n_sum += turnv_angle[i].angle_err;
					i = (i == (TURNV_ANGLE_MAX - 1)) ? 0:(i + 1);
					log_printf("%d\r\n",turnv_angle[i].angle_err);
				}
				n_sum /= turnv_angle_ct;

				log_printf("-------------\r\n[turn_to_deg_v] get angle %d-%d\r\n-------------\r\n",n_sum,turnv_angle_ct);

				for(i = turnv_angle_head;i != turnv_angle_tail;)//检查数据的有效性,数组当中超出平均值5度以上的数值
												//超出30%,就认为此次数据无效,不对角度进行校正
				{
					if(disxy(n_sum,turnv_angle[i].angle_err) > 300)
					{
						n_invalid_ct ++;
						turnv_angle_head = (turnv_angle_head == (TURNV_ANGLE_MAX - 1)) ? 0:(turnv_angle_head + 1);
					}
					i = (i == (TURNV_ANGLE_MAX - 1)) ? 0:(i + 1);
				}
				if(((turnv_angle_ct *10/3) > n_invalid_ct) && (n_sum > 200 || n_sum < -200))//无效数据的数量小于门限值,进行校正
				{
					n_angle_correct = (float)n_sum;
					n_angle_correct = n_angle_correct / 100.0 + sys->angle;
					illegal_copy_tell_13757122544_gsend(0x02,n_angle_correct, 30);
					log_printf("adj_a_turn_degv:%3.1f,%3.1f,80\r\n",sys->angle,n_angle_correct);
					//turnv_angle_head = (turnv_angle_head >= (TURNV_ANGLE_MAX - 2)) ? 1:(turnv_angle_head + 2);
					//turnv_angle_ct -= 2;
					//turnv_timerec = sys->t_navi_work;//重新同步时间
				}
				turnv_angle_head = (turnv_angle_head >= (TURNV_ANGLE_MAX - 2)) ? 1:(turnv_angle_head + 2);
				turnv_angle_ct -= 2;

				delay_ms(5);
				//turnv_angle_ct = 0;

				//motor_run(GO_STOP,0,0,0);
				//TargetSysReset();
			}
		}
		//if(sys->sState == SYS_NAVIGAT || )
	}
#endif
#endif
	//delay_ms(2);
	delay_sensor(2);
	www_idleintel_com();
	log_printf("[turn_v]OK,angle=%f(%f)left_a:%3.3f,midle_a:%3.3f,right_a:%3.3f,%d,%d,xy=(%d,%d,%d,%d,%3.3f,%3.3f)\r\n\r\n",sys->angle,navigat->angle,v_angle_left,v_angle_midle,v_angle_right,msTmr - t_turn,motor.c_left_hw
						,X_NOW,Y_NOW,navigat->x_org,navigat->y_org,navigat->x_org_f,navigat->y_org_f);
}

#endif

#if 0
/*
导航过来后，一直要前进的步数。
*/
char cross_after_navi(int16_t y_dist,float angle,uint8_t  is_save,uint8_t nsta)
{
	int distance;
	uint8_t n_sta;
	char ret;
	short xx,yy;
	int dir;
	uint8_t out_t;
	int16_t y;


	coord_org2map(navigat->x_org, y, &xx, &yy);
	navigat->need_back = 0;

	sys->walk_sta = 0;
	log_printf("[cross_after_navi](%d,%d,%3.1f)->(%d,%d,%3.1f)save=%d,nsta=%d\r\n",navigat->tx,navigat->ty,navigat->angle,xx,yy,angle,is_save,nsta);

	//如果没有设定状态，则根据状态来

	if(y > navigat->y_org)
	{
		if(nsta >= 10)
			n_sta = navigat->lst_angle == 0?RIGHT_SIDE_NEAR:LEFT_SIDE_NEAR;
		else
			n_sta = nsta;
		turn_to_deg(90);
		distance = y - navigat->y_org;
		dir = 1;
	}else
	{
		if(nsta >= 10)
			n_sta = navigat->lst_angle == 0?LEFT_SIDE_NEAR:RIGHT_SIDE_NEAR;
		else
			n_sta = nsta;
		turn_to_deg(270);
		distance =  navigat->y_org - y;
		dir =-1;
	}

	//test

	/*
	//左边边延边，且是右边碰撞过来，要多转一点角度
	if(n_sta == LEFT_SIDE_NEAR && (sys->gsta  & MASK_BUM_RIGHT))
		robot_turn_deg(GO_RIGTH,650,20);
	if(n_sta == RIGHT_SIDE_NEAR && (sys->gsta & MASK_BUM_LEFT))
		robot_turn_deg(GO_LEFT,650,20);
	*/
	//n_sta = NO_SIDE_NEAR;
	out_t = OUT_TYPE_Y;

	ret =motor_go_near_wall(distance, n_sta,is_save,out_t,dir,3);
	//粗略的计算延边的次数，用于计算扫完后，往那边延边

	if(LEFT_ADC() >= cfg->side_left_adc || RIGHT_ADC() >=cfg->side_right_adc)
	{
		if(navigat->lst_angle == 0)
			navigat->c_near_0 ++;
		else
			navigat->c_near_180++;
	}

	log_printf("c_near0=%d,c_near180=%d,lstangle=%3.1f,ret=%d\r\n",navigat->c_near_0,navigat->c_near_180,ret);
#if Y_ORG_OFFSET > 0
	if(navigat->side == LEFT_SIDE)
		navigat->y_org_f -= Y_ORG_OFFSET;
	else
		navigat->y_org_f += Y_ORG_OFFSET;
#endif
	//前方清扫了。

	if(ret == 0)
	{
		return 0;
	}
	//不需要掉头
	if(ret ==2)
	{
		int16_t x_dir;
		uint8_t dir=0;
		x_dir = navigat->lst_angle==0?1:-1;
		//是否在垂直方向
		if(disfloat(90,sys->angle) < 40 || disfloat(sys->angle,270) < 40)
			dir = 1;
		log_printf("[near_wall]ret = 2,angle=%3.1f,ret=%d,adc=%d,%d\r\n",sys->angle,LEFT_ADC(),RIGHT_ADC());
		//前方有障碍物，或侧面有红外，就不经过了
		if(OBST_XY(X_NOW+x_dir,Y_NOW) || ((LEFT_ADC() > 300 || RIGHT_ADC()>300) && dir==1) ||
			(LEFT_ADC() > 700 || RIGHT_ADC()>700))
		{
			turn_to_deg(angle);
		}else
		{
			angle = angle ==180?0:180;
			turn_to_deg(angle);
			navigat->need_back = 1;
		}
		return 1;
	}
	if(ret ==1)
	{

		turn_to_deg(angle);
	}
	return 1;
}
#endif

/*
分析延边障碍物的合理性
max_ofs :最多的穿过清扫，或未清扫区域个数

*/

#define NEAR_CROSS_OK		0x00		//正常
#define NEAR_CROSS_UNCLR	0x01		//穿过未清扫点
#define NEAR_CROSS_CLR		0x02		//穿过清扫点，需要矫正坐标
int dispose_nearwall_obst(SIDEOBST *m_obst,int16_t max_ofs)
{
	int16_t i,k;
	int16_t	c_unclr=0;		//未清扫的个数
	OBST *obst;
	int16_t x,y;
//	uint16_t cx,cy;
	log_printf("[dispose_nw_ost]c=%d\r\n",m_obst->len);
	for(i=0;i<m_obst->len;i++)
	{
		obst = &m_obst->obst[i];
		//log_printf("(%d,%d,%d)8\r\n",obst->x,obst->y,obst->ir);
		//障碍点重合
		if(OBST_XY(obst->x,obst->y))
		{
			//log_printf("coin\r\n");
			continue;
		}
		x = obst->x;
		y = obst->y;
#if 0
		//==================================扫描出延边的数据，根据一条线来矫正====================================
		//if(obst->ir >=400)		//侧面有红外，延边了
		{
			//扫描接下来的几个数据，是延边，且X轴，或Y轴差异不大的，表明是一条直线的，
			cx = cy = 0;
			for(j=i;j<m_obst->len;j++)
			{	//延边，且X轴，Y轴差异不大，表明是一条直线
				if(x==m_obst->obst[j].x )
				{
					if(y != m_obst->obst[j].y)
						cx++;
				}
				else
					break;
			}
			for(j=i;j<m_obst->len;j++)
			{
				if(y == m_obst->obst[j].y)
					if(x!=m_obst->obst[j].x)
						cy++;
				else
					break;
			}
			j--;
			//超过四个点
			if( cx >=3 || cy>=3)
			{
				int16_t mac;
				mac = dis_xy(i,j) >=5?4:1;
				log_printf("found a line:(%d,%d,)->(%d,%d,)mac=%d,cx=%d,cy=%d\r\n",x,y,m_obst->obst[j].x,m_obst->obst[j].y,mac,cx,cy);
				//把数据矫正过来
				//==========X轴相等，表明沿着Y轴运动====
				if(cy >=3)
				{
					int16_t y_ofs;

					if( coin_line_90_270(&m_obst->obst[i],j-i,mac,&y_ofs))
					{
						log_printf("ajust x_ofs=%d\r\n",y_ofs);
						if(check_map_reason(X_NOW,Y_NOW,X_NOW,Y_NOW+y_ofs))
						{
//							ajust_xy_map(X_NOW,Y_NOW+y_ofs,AJUST_Y);

							//这里可以考虑拟合Y轴
							//memset((char *)m_stack,0x00,STACK_LEN);		//数据清零
							side_obst_to_map(m_obst,0,y_ofs,0);
							//return NEAR_CROSS_OK;
						}
					}



				}else
				//==========Y轴相等，表明沿着Y轴运动,矫正X轴====
				if(cx >=3)
				{
					int16_t x_ofs;
					if( coin_line_0_180(&m_obst->obst[i],j-i,mac,&x_ofs))
					{
						log_printf("ajust y_ofs=%d\r\n",x_ofs);
						if(check_map_reason(X_NOW+x_ofs,Y_NOW,X_NOW,Y_NOW))
						{
//							ajust_xy_map(X_NOW+x_ofs,Y_NOW,AJUST_X);

							//这里可以考虑拟合Y轴
							//memset((char *)m_stack,0x00,STACK_LEN);		//数据清零
							side_obst_to_map(m_obst,x_ofs,0,0);
							//return NEAR_CROSS_OK;
						}
					}

				}
			}
		}
#endif
		//穿过清扫的区域，或是穿过未清扫的区域，则计数
		k = 0;
		for(x=obst->x-3;x<=obst->x+3;x++)
		{
			for(y = obst->y-3;y<=obst->y+3;y++)
			{
				if(clean_sta(x,y) )		//已经经过
					k=1;
			}
		}
		if(k ==0)						//未清扫
			c_unclr++;
		else
			c_unclr =0;
		if(c_unclr >=3)					//连续三个未清扫
			return NEAR_CROSS_UNCLR;
	}
	return NEAR_CROSS_OK;
}

/*
uint8_t near_wall_loop(int16_t bx,int16_t by,uint8_t side,uint32_t tmr)
{
	return 0;

	static uint8_t sta=0;//,rsta=0;		//判断是否反向过了。

	//初始化
	if(bx ==0)
	{
		sta = 0;
//		rsta = 0;

		return 0;
	}
	if(side == LEFT_SIDE)		//往Y大的方向走
	{
		if(sta ==0)		//没有反向，要判断是否反向
		{
			if( Y_NOW < (by-4) && (msTmr - tmr) > 20000)
				sta = 1;		//发现反向了。
		}else					//反向过了。
		{
			if(Y_NOW > (by+2) && disxy(bx,X_NOW) <=4)
			{
				log_printf("loop OK1,bxy=(%d,%d,)\r\n",bx,by);
				return 1;
			}
		}

	}else if(side == RIGHT_SIDE)		//往Y小的方向走
	{
		if(sta ==0) 	//没有反向，要判断是否反向
		{
			if( Y_NOW > (by+4) && (msTmr - tmr) > 20000)
				sta = 1;		//发现反向了。
		}else					//反向过了。
		{
			if(Y_NOW < (by-2) && disxy(bx,X_NOW) <=4)
			{
				log_printf("loop OK2,bxy=(%d,%d,)\r\n",bx,by);
				return 1;
			}
		}

	}
	if((msTmr - tmr)  > 900000)	//大于15分钟，也认为玩了
	{
		log_printf("loop timeout\r\n");
		return 1;

	}
	return 0;

}*/
//判断是不是在一个地方转圈了。
//#define MAX_KXXYY		80
#define MAX_KXXYY		12*30

static short kxx[MAX_KXXYY];
static short kyy[MAX_KXXYY];
char check_round_bum(uint8_t type)
{
#if 0
	static uint16_t idx=0;
	short i;
	if(type ==0)
	{
		idx = 0;
		return 0;
	}
	if(idx >=MAX_KXXYY)
		idx = 0;
	kxx[idx] = X_NOW;			//进来存储当前x坐标
	kyy[idx++] = Y_NOW;			//进来存储当前y坐标

	for(i=1;i<MAX_KXXYY;i++)
	{
		if(dis_xy(kxx[0],kxx[i]) > 5)
			return 0;
		if(dis_xy(kyy[0],kyy[i]) > 5)
			return 0;
	}
	log_printf("round bum!!!\r\n");
	return 1;
#else		//debug 未测试
//在小于5的小坐标范围内完成12次大角度转弯,大约360,发现被困之后,12*20个=240度小角度转弯，再重复
	static uint16_t idx=0;
	short i;
	if(type ==0)
	{
		idx = 0;
		return 0;
	}

	kxx[idx] = X_NOW;			//进来存储当前x坐标
	kyy[idx] = Y_NOW; 		//进来存储当前y坐标

	for(i=1;i<idx;i++)
	{
		if(dis_xy(kxx[0],kxx[i]) > 5)		//将最后较大的坐标放进对比坐标里
		{
			kxx[0] = kxx[i];
			kyy[0] = kyy[i];
			idx = 0;
			return 0;
		}
		if(dis_xy(kyy[0],kyy[i]) > 5)
		{
			kxx[0] = kxx[i];
			kyy[0] = kyy[i];
			idx = 0;
			return 0;
		}
	}
	idx++;

	if(idx >= MAX_KXXYY/30)												///12*30=360
	{
		if(idx >=MAX_KXXYY)												///12*12=144度
			idx = 1;													//从	kxx[1]kyy[1] 开始存储 不算圈圈内,8次后再算圈圈内
		log_printf("round bum count out:%d!!!\r\n",idx);				//在圈圈内碰撞
		return 1;
	}
	else
	{
		log_printf("round bum count:%d\r\n",idx);						//在圈圈内碰撞累计
		return 0;
	}
#endif
}

#if 0
uint8_t nearwall_go_back(int16_t ty,float angle,uint8_t t_near)
{
//	int distance;
	uint8_t n_sta;
	uint16_t max_bum=6;
	uint8_t ret;
	int16_t may;
//	static uint8_t ln_sta = RIGHT_SIDE_NEAR;	//保存上一次的。


	sys->walk_sta = 0;
	log_printf("[nearwall_go_back](%d,%d,%3.1f)->(%d,%d,%3.1f)tnear=%d\r\n",navigat->tx,navigat->ty,navigat->angle,navigat->tx,ty,angle,t_near);
	CHECK_NAVI_STA_RT(0);
	/*
	//如果Y相等,则按照之前方向走，避免老是反复的循环，春节在家里测试，发现反复的循环
	if(ty == Y_NOW )
	{
		log_printf("the same y,lnsta=%d\r\n",n_sta);

	}else
	*/

	if(ty > Y_NOW)
	{
		n_sta = navigat->lst_angle == 0?RIGHT_SIDE_NEAR:LEFT_SIDE_NEAR;
		log_printf("----nearwall_go_back--ty(%d) > Y_NOW(%d),n_sta=%d,turn to 90\r\n",ty , Y_NOW,n_sta);
		turn_to_deg(90);
	}else
	{
		n_sta = navigat->lst_angle == 0?LEFT_SIDE_NEAR:RIGHT_SIDE_NEAR;
		log_printf("----nearwall_go_back--ty(%d) < Y_NOW(%d),n_sta=%d,turn to 270\r\n",ty , Y_NOW,n_sta);
		turn_to_deg(270);
	}
	if(dis_xy(ty,Y_NOW) < 3 || navigat->c_reverse_go >=3)		//距离较短，则一直延边
	{
		log_printf("short dis(%d,%d) or many reverse(%d),always near...\r\n",ty,Y_NOW,navigat->c_reverse_go);
		t_near = 1;
		max_bum = ty = may =0;

	}

	//ln_sta = n_sta；

	may = Y_NOW >=ty ?(Y_NOW - ty):(ty - Y_NOW);
	may+=2;
	ret= motor_nearwall_back(n_sta,0,ty,may,max_bum,0,t_near);

	log_printf("[nearwall_go_back]OK,(%d,%d,)\r\n",X_NOW,Y_NOW);
	turn_to_deg(angle);
	return ret;
}
#endif
#if  0
void motor_back_off(int hw)
{
	//float dt;
//	int c;
	uint8_t	sState = sys->sState;
	int i,tmr;
	uint16_t c_bum=0;
//	int dist=0
#if CALE_BY_BKHW
	int distance=motor.c_left_hw - hw * WHELE_HW;
#else
	int distance=navigat->distance - hw;
#endif

	CHECK_NAVI_STA();
	motor_run(GO_STOP,0,0,0);
	delay_ms(5);
	TIM5->CNT = 0;
	//motor.c_left_hw = 0;
	tmr = hw > 500 ?5000:500;	//tmr = hw > 50 ?2000:500;
	log_printf("  b&");
	motor_run(GO_BACK, BACK_OFF_PWM, 0, 0);
//	navigat->wheel_dir =-1;
	//navigat->is_walk = 1;

	i=0;

//	log_printf("befor bk,left(%3.1f,%3.1f)right(%3.1f,%3.1f)org(%d,%d)\r\n",navigat->x_org_f,navigat->y_org_f,navigat->x_org_r,,navigat->y_org_r,navigat->x_org,navigat->y_org);
	while(1)
	{
		get_sensers(&sys->gSta);
		proc_uart_task();
//////
		//=================判断退出机制=====================================
		if(sys->sState !=sState)
		{
			log_printf("[charging_turn_round]!!SYS_DOCK_3,%d,%d\r\n",sys->sState ,sState);
			motor_run(GO_STOP,0,0,0);
			STOP_ALL_MOTOR();
			gyro_whlmap();
#if SYS_VER == VER_KESMAN
			set_ir_alter_dis(0);
#endif
			break ;
		}
		if(sys->mState == GO_STOP)
		{
			motor_run(GO_BACK, BACK_OFF_PWM, 0, 0);
		}
//////
		if(TIM5->CNT >=5000)	//5ms检测一次
		{
			TIM5->CNT = 0;
			www_idleintel_com();	//2018-10-31 恢复取角度,保证后退坐标准确  sys->angle
			//log_printf("k");
			if(WALK_DIST()< distance || i++ >= tmr)//2.5s超时
			{
				motor_run(GO_STOP,0,0,0);
				log_printf("bof ok,dis=%d,%d\r\n",WALK_DIST() ,distance);
				break;
			}
			if( sys->gSta & (MASK_BUM_RIGHT) || sys->gSta & (MASK_BUM_RIGHT2)  ||
			     sys->gSta & (MASK_BUM_LEFT) || sys->gSta & (MASK_BUM_LEFT2)  )
			{
				if(c_bum++>30)		///150ms
				{
					motor_run(GO_STOP,0,0,0);
					log_printf("back off bum,go ou,dis=%d,%d\r\n",WALK_DIST() ,distance);
					break;
				}
			}
		}
	}

	//navigat->is_walk = 0;
	motor_run(GO_STOP,0,0,0);
	www_idleintel_com();
	coordinate_calcu(0);													//计算新的坐标
	//log_printf("c2\r\n");
	coord_org2map(navigat->x_org,navigat->y_org,&navigat->tx,&navigat->ty);	//坐标系转换
	log_printf("o&");
	//gyro_whlmap();
	log_printf("\r\n");
	//while(1);
    //log_printf("last bk,left(%3.1f,%3.1f)right(%3.1f,%3.1f)org(%d,%d)\r\n",navigat->x_org_f,navigat->y_org_f,navigat->x_org_r,,navigat->y_org_r,navigat->x_org,navigat->y_org);
	//log_printf("*(%d,%d,%f,0)-[%d,%d,%3.1f]\r\n\r\n",navigat->tx,navigat->ty,navigat->angle,navigat->x_org,navigat->y_org,sys->angle);
	//log_printf("hw=%d,x:%d,y%d,angle:%f,tx:%d,ty:%d\r\n",hw,navigat->x_org,navigat->y_org,navigat->angle,navigat->tx,navigat->ty);
	save_line_xy(navigat->x1_org,navigat->y1_org);
}
#endif
#if 1
//后退N霍尔
void motor_back_off(int hw)
{
	//float dt;
//	int c;
	uint8_t	sState = sys->sState;
	int i;//,tmr;
//	uint16_t c_bum=0;
	uint16_t c_calc=0;
//	int dist=0
//#if CALE_BY_BKHW
	int distance=motor.c_left_hw - hw * WHELE_HW;	//
//#else
//	int distance=navigat->distance - hw;			//
//#endif

	CHECK_NAVI_STA();
	motor_run(GO_STOP,0,0,0);
	//delay_ms(5);
	delay_sensor(5);
	TIM5->CNT = 0;
	uint32_t timout ;//= hw * cfg->r[IC_GO_BACK] / 100;
	if(hw < 50)
		timout = 70;
	else if(hw < 120)
		timout = 150;
	else
		timout = 500;
	//motor.c_left_hw = 0;
	//tmr = hw > 50 ?5000:500;
	uint16_t maxcnt= 3;//cfg->r[IC_GO_BACK] % 100;
	log_printf("  bof,hw=%d,dist=%d,left=%d,timeout=%d,maxcnt=%d\r\n",hw,distance,motor.c_left_hw ,timout,maxcnt);
	//motor.c_left_hw = motor.c_right_hw = 0;    //左右轮的霍尔值清一下
	log_printf("1back left=%d,back right=%d\r\n",sys->left_pwm,sys->right_pwm);//L_FORWRK_PWM
	//log_printf("2back left=%d,back right=%d\r\n",L_FORWRK_PWM,R_FORWRK_PWM);
	log_printf("3back left hw=%d,back right hw=%d\r\n",motor.c_left_hw,motor.c_right_hw);
	if(hw >= STRONG_BACKOFF)
		motor_run(GO_BACK, 400, 0, 0);
	else
		motor_run(GO_BACK, BACK_OFF_PWM, 0, 0);
//	navigat->wheel_dir =-1;
	//navigat->is_walk = 1;
	i=0;
//	uint16_t left_hw=motor.c_left_hw;
    	m_speed_pid_ctrl(0,0);
	uint16_t left_lst_hw=0,right_lst_hw=0;
	uint16_t c_left=0,c_right=0;
//	log_printf("befor bk,left(%3.1f,%3.1f)right(%3.1f,%3.1f)org(%d,%d)\r\n",navigat->x_org_f,navigat->y_org_f,navigat->x_org_r,,navigat->y_org_r,navigat->x_org,navigat->y_org);
	while(1)
	{
		get_sensers(&sys->gSta);
		proc_uart_task();

		//=================判断退出机制=====================================
		if(sys->sState !=sState)
		{
			log_printf("[motor_back_off]!!SYS_DOCK\r\n");
			motor_run(GO_STOP,0,0,0);
			STOP_ALL_MOTOR();
			gyro_whlmap();
#if 1//SYS_VER == VER_KESMAN
			set_ir_alter_dis(0);
#endif
			break;
		}
			if(sys->mState == GO_STOP)
		{
			motor_run(GO_BACK, BACK_OFF_PWM, 0, 0);
		}


		if(TIM5->CNT >=5000)				//10ms*500=5s 后退5s
		{
			TIM5->CNT = 0;
			www_idleintel_com();
			//log_printf("k");
			if(WALK_DIST()< distance || i++ >= timout)
   			{
				motor_run(GO_STOP,0,0,0);
				log_printf("bof ok,dis=%d,%d,t=%d,%d\r\n",WALK_DIST() ,distance,i,timout);
				break;
			}
			//micony20191022 后退加了个角度环
			//proc_back_pid(navigat->out);
			if(c_calc++ >=20)		//100ms
			{
				c_calc = 0;
			  ////m_speed_pid_ctrl(0,1);
				if(left_lst_hw == motor.c_left_hw)
					c_left++;
				else
				{
					c_left = 0;
					left_lst_hw = motor.c_left_hw;
				}
				/**/
				if(right_lst_hw == motor.c_right_hw)
					c_right++;
				else
				{
					c_right = 0;
					right_lst_hw = motor.c_right_hw;
				}

			 log_printf("BK hw=<%d,%d>pwm=(%d,%d,%d),%d,t=%d\r\n",motor.c_left_hw , motor.c_right_hw,L_FORWRK_PWM,R_FORWRK_PWM,sys->pwm,I_MID_ADC(),i);
			 if(c_left >=maxcnt || c_right >= maxcnt)
			 {
				log_printf("bk timeout\r\n");
				motor_run(GO_STOP,0,0,0);
				log_printf("bof ok,dis=%d,%d\r\n",WALK_DIST() ,distance);
				break;
			 }

			}

		}
		//if( i % 100 ==0)
		//	log_printf("%d\r\n",WALK_DIST() ,distance);
	//	if(sys->mState == GO_STOP)
	//	{
	//		motor_run(GO_BACK, BACK_OFF_PWM, 0, 0);
	//	   	log_printf("bof ok2,dis=%d,%d\r\n",WALK_DIST() ,distance);
    //  }
	}
	//navigat->is_walk = 0;
	motor_run(GO_STOP,0,0,0);
	www_idleintel_com();
	coordinate_calcu(0);					//计算新的坐标
	//log_printf("c2\r\n");
	coord_org2map(navigat->x_org,navigat->y_org,&navigat->tx,&navigat->ty);	//坐标系转换
	log_printf("o&t=%d\r\n",i);
	//gyro_whlmap();
	log_printf("\r\n");
	//while(1);
//	log_printf("last bk,left(%3.1f,%3.1f)right(%3.1f,%3.1f)org(%d,%d)\r\n",navigat->x_org_f,navigat->y_org_f,navigat->x_org_r,,navigat->y_org_r,navigat->x_org,navigat->y_org);
	//log_printf("*(%d,%d,%f,0)-[%d,%d,%3.1f]\r\n\r\n",navigat->tx,navigat->ty,navigat->angle,navigat->x_org,navigat->y_org,sys->angle);
	//log_printf("hw=%d,x:%d,y%d,angle:%f,tx:%d,ty:%d\r\n",hw,navigat->x_org,navigat->y_org,navigat->angle,navigat->tx,navigat->ty);
	save_line_xy(navigat->x1_org,navigat->y1_org);
}
#endif
//=================================================避开充电桩的算法====================================
#define MAX_DOC_IR		10
 u8 dock_ir[MAX_DOC_IR][3];
 uint16_t m_dock_midle=0;		//中线
uint8_t dock_len=0;
/*
回充红外的列表，10个列表
*/
void init_doci_ir(void)
{
	int16_t i;
	dock_len = 0;
	for(i=0;i<MAX_DOC_IR;i++)
	{
		dock_ir[i][0] = 0;
		dock_ir[i][1] = 0;
		dock_ir[i][2] = 0;
	}
	m_dock_midle = 0;
}

uint8_t proc_dock_near_bum(uint8_t flage)
{
	static uint8_t isbum=0;
	static uint8_t count=0;
	if(flage ==0)
	{
		isbum = 0;
		count = 0;
		return 0;
	}
	count++;
	if(count <8)
		return 0;
	if(ir_dock_insight(1))
	{
		//延边的情况下，角度对，则碰撞
		if(LEFT_ADC() >=cfg->min_left_adc || RIGHT_ADC() >=cfg->min_right_adc)
		{
			if(disfloat(sys->angle,90) < 20 || disfloat(sys->angle,270) < 20 )
			{
			//	log_printf("[dock_near_bum]angle=%3.3f,adc=%d,%d\r\n",LEFT_ADC(),RIGHT_ADC());
				isbum = 1;
				return 1;
			}
		}
		//已经碰撞了，只要发现就碰撞
		if(isbum)
		{
			log_printf("[dock_near_bum]have ben bum(%d,%d)\r\n",LEFT_ADC(),RIGHT_ADC());
			return 1;
		}
	}
	return 0;

}

void   proc_dock_coord(short bx,short by)
{
	if(navigat->c_dock_m >=10)		//发现充电桩
	{
		navigat->tx_dock[0] = (bx + X_NOW) >>1;
		navigat->ty_dock[0] = Y_NOW;
		log_printf("found m=%d,dock=(%d,%d,)\r\n",navigat->c_dock_m,navigat->tx_dock[0],navigat->ty_dock[0]);
	}else if(navigat->c_dock_l >=10)		//发现充电桩
	{
		navigat->tx_dock[1] = (bx + X_NOW) >>1;
		navigat->ty_dock[1] = Y_NOW;
		log_printf("found l=%d,dock=(%d,%d,)\r\n",navigat->c_dock_l,navigat->tx_dock[1],navigat->ty_dock[1]);
	}else if(navigat->c_dock_r >=10)		//发现充电桩
	{
		navigat->tx_dock[2] = (bx + X_NOW) >>1;
		navigat->ty_dock[2] = Y_NOW;
		log_printf("found r=%d,dock=(%d,%d,)\r\n",navigat->c_dock_r,navigat->tx_dock[2],navigat->ty_dock[2]);
	}
}

uint8_t proc_doc_ir(void)
{
	uint8_t i,c_m=0,c_mn=0,c_left=0,c_right=0;
	static uint8_t count=0;
	static uint8_t ctop=0;
	count++;
	uint8_t dock_signal;
	if(sys->sState ==SYS_DOCK)
		return 0;
	if(count <= 16)
		return 0;
	count = 0;
	//log_printf("---------\r\n");
	read_ir_data(IrData_bak,0);

	insert_dock_ir(IrData_bak);

	//计数充电桩的位置
//#if (1 == SINGLE_LED_DOCK)
//	if(IrData_bak[IR_ML_PIN_NUM])
//#else
	if(IrData_bak[IR_ML_PIN_NUM] || IrData_bak[IR_MR_PIN_NUM])
//#endif
		navigat->c_dock_m++;

	if(IrData_bak[IR_L_PIN_NUM])
		navigat->c_dock_l++;

	if(IrData_bak[IR_R_PIN_NUM])
		navigat->c_dock_r++;


	dock_signal = ir_dock_insight(1);
#if DOCK_SIGNAL_UNBLE
	if(dock_signal && DOCK_BUM_IR())
#else
	if(dock_signal )
#endif
	{
		log_printf("walktop:%d\r\n",dock_signal);
		//if(ctop++>=2 )
			return 1;
		//if(disfloat(sys->angle,0) < 10)
		//if(navigat->angle == 0)
		//{
		/*
			if(sys->g_sta[2] >=80 || sys->g_sta[3] >=80 || sys->g_sta[4] >=80)
			{
				log_printf("==dock bum===ir=%d,%d,%d\r\n",sys->g_sta[2],sys->g_sta[3],sys->g_sta[4]);
				return 1;
			}
		//}
		if((navigat->angle == 90 || navigat->angle == 270) &&
			(LEFT_ADC() >=cfg->min_left_adc || RIGHT_ADC() >=cfg->min_right_adc))
		{
			log_printf("==dock bum2==(%d,%d)\r\n",LEFT_ADC(),RIGHT_ADC());
			return 1;

		}
		if(LEFT_ADC()> cfg->min_left_adc || RIGHT_ADC() > cfg->min_right_adc)
		{
			if(IrData_bak[IR_M_PIN_NUM]==IR_RIGHT || IrData_bak[IR_M_PIN_NUM]==IR_LEFT )
			{
				log_printf("==dock side bum(%d,%d,%d)===\r\n",IrData_bak[0],IrData_bak[1],IrData_bak[2]);
				return 1;
			}
		}
		*/

	}else
		ctop = 0;


	//if(IrData_bak[IR_M_PIN_NUM] == IR_MID)
	//s	m_dock_midle++;
	//log_printf("%d,%d,%d-%d,%d,%d\r\n",IrData_bak[0],IrData_bak[1],IrData_bak[2],sys->g_sta[2],sys->g_sta[3],sys->g_sta[4]);
	//if(sys->g_sta[2] >= DOCK_BUM_IR || sys->g_sta[3] >= DOCK_BUM_IR || sys->g_sta[4] >= DOCK_BUM_IR)
	//micony2017-12-26不知道为何加这个条件，导致导航回充老是碰撞，不知道改掉有没有问题
	if((DOCK_BUM_IR() /*|| navigat->navi_to_dock*/) && IrData_bak[IR_ML_PIN_NUM])
	{
		if(disfloat(sys->angle,0) < 20 || navigat->angle == 0 || navigat->navi_to_dock)
		{
			for(i = 0;i< MAX_DOC_IR;i++)
			{
				if(dock_ir[i][IR_ML_PIN_NUM] == IR_MID)
				 	c_m++;
				else if(dock_ir[i][IR_ML_PIN_NUM] == IR_LEFT)
					c_left++;
				else if(dock_ir[i][IR_ML_PIN_NUM] == IR_RIGHT)
					c_right++;
				if(dock_ir[i][IR_ML_PIN_NUM] > 0)
					c_mn++;
			}
			//log_printf("dockbum(%d,%d,%d,%d)\r\n",c_m,c_left,c_right,c_mn);
			if((c_m >=3&& c_mn>=5 )|| c_mn >=7)
			{
				log_printf("==dock bum===%d,%d,%d,ir=%d,%d,%d\r\n",m_dock_midle,c_m,c_mn,sys->ir_front_1,sys->g_sta[3],sys->ir_front_3);
				if(navigat->navi_to_dock)
				{
					navigat->navi_to_dock = 2;		//发现充电桩
					return 1;
				}
				if(dis_xy(Y_NOW,navigat->dock_y) <=5 && disfloat(sys->angle,0)<=2 && navigat->need_adj_dock
					&& navigat->from_charge)
				{
					//中线

					if(c_m >=8)
					{
						log_printf("in middle dock ajust..\r\n");
//						ajust_xy_map(navigat->dock_x -1 ,navigat->dock_y,AJUST_Y);//|AJUST_X);
						navigat->need_adj_dock = 0;
					}else if(c_left >=9)	//左边
					{
						log_printf("in left dock ajust..\r\n");
//						ajust_xy_map(navigat->dock_x -1 ,navigat->dock_y+1,AJUST_Y);//|AJUST_X);
						navigat->need_adj_dock = 0;

					}else if(c_right >=9)
					{
						log_printf("in left dock ajust..\r\n");
//						ajust_xy_map(navigat->dock_x -1 ,navigat->dock_y-1,AJUST_Y);//|AJUST_X);
						navigat->need_adj_dock = 0;

					}
				}

				/*
				if(c_m >=8 && dis_xy(Y_NOW,navigat->dock_y) <=4 && disfloat(sys->angle,0)<=2 && navigat->need_adj_dock)//中心
				{
					log_printf("in middle line ajust..\r\n");
//					ajust_xy_map(X_NOW,navigat->dock_y,AJUST_Y);
					navigat->need_adj_dock = 0;
				}else
				{
					navigat->dock_y = Y_NOW;
					log_printf("found midle ajus docky=(%d,%d,)\r\n",navigat->dock_x,navigat->dock_y);
				}
				*/
				return 1;			//dockbum
			}
		}
	}
	//log_printf("(%d,%d,%d)(%d,%d,%d)%d\r\n",IrData_bak[0],IrData_bak[1],IrData_bak[2],sys->g_sta[2],sys->g_sta[3],sys->g_sta[4],c_m);
	//侧面过来
	/*
	if((sys->g_sta[0] > 400  || sys->g_sta[6]  > 400) && navigat->from_charge )
	{	//在充电桩的周边
		if(Y_NOW > 96 && Y_NOW < 104 && X_NOW >96 && X_NOW < 104)
		{
			//if(sys->g_sta[2] >= 200 || sys->g_sta[3] >= 200 || sys->g_sta[4] >= 200)
			//不是往充电桩方向走

			if(dis_xy(sys->angle,0) > 10  )
			{
				for(i = 0;i< MAX_DOC_IR;i++)
				{
					if(dock_ir[i][IR_M_PIN_NUM] )
					 	c_m++;
				}
				if(c_m >=5)
				{
					log_printf("====dock side bum====side ir=(%d,%d)\r\n",sys->g_sta[0],sys->g_sta[6]);
					return 1;			//dockbum
				}
			}
		}
	}
	*/
	if(navigat->from_charge)
	{

		if(dis_xy(X_NOW,navigat->dock_x) <=1 && dis_xy(Y_NOW,navigat->dock_y)<=1)
		{
			//方向朝充电桩去
			if(disfloat(sys->angle,0) < 10 ||
			(disfloat(sys->angle,90) < 10 && Y_NOW < navigat->dock_y) ||
			(disfloat(sys->angle,270) < 10 && Y_NOW > navigat->dock_y))
			{
 				log_printf("near dock.dock bum\r\n");
				return 1;
			}
		}
	}

	return 0;

}
void insert_dock_ir(u8 *data)
{
	if(dock_len >=MAX_DOC_IR)
		dock_len = 0;
	dock_ir[dock_len][0] = data[0];
	dock_ir[dock_len][1] = data[1];
	dock_ir[dock_len++][2] = data[2];
}
//中间的收到中间，就是在中线了
uint8_t in_dock_mline(void)
{
	uint8_t i,c=0;
	for(i = 0;i< MAX_DOC_IR;i++)
	{
		if(dock_ir[i][IR_ML_PIN_NUM] == IR_MID)
			c++;
	}
	return c;
}
void printf_dock_msg(void)
{
	int16_t i;
	for(i = 0;i< MAX_DOC_IR;i++)
	{
		log_printf("dock(%d,%d,%d)\r\n",dock_ir[i][0] ,dock_ir[i][1] ,dock_ir[i][2] );
	}
}

uint16_t run_timeout = 200;
void motor_single_run(uint8_t dir)
{
	//再加上风机的运动
	log_printf("motor_single_run,dir=%d,mSta=%d\r\n",dir,sys->mState);
	if(dir == GO_FORWARD || dir == GO_BACK || dir == GO_LEFT || dir == GO_RIGTH)
	{
		//方向一致则只刷新定时器

		if(sys->mState == dir)
		{

				navigat->walk_dis = 0;
				run_timeout = 40;
				log_printf("the same dir ,flash timer,dir=%d,timr=%d\r\n",dir,motor.c_left_hw );

				//DUST_MOTOR_RANK3();
				/*
				if(dir == GO_FORWARD && motor.c_left_hw > 300)
				{

					log_printf("run timeout,stop\r\n");
					motor_run(GO_STOP,0,0,0);
					STOP_ALL_MOTOR();
					return ;
				}
				*/
		}
		else
		{
			log_printf("run...");
			run_timeout = 120;
			/*
			if(sys->mState == GO_FORWARD && dir == GO_BACK))
			{
				if(navigat->walk_dis >=run_timeout)
				{
					motor_run(GO_STOP,0,0,0);
					STOP_ALL_MOTOR();
				}
			}else
			*/
			{

				sys->work_errcode = MWERR_NONE;
				MOTOR_CTRL2(NORM_SIDE_PWM_L,NORM_SIDE_PWM_R,NORM_MID_PWM,sys->dust_pwm_value);
				//MOTOR_CTRL2(NORM_SIDE_PWM_L,NORM_SIDE_PWM_R,600,sys->dust_pwm_value);
				DUST_MOTOR_RANK3();
				motor_run(GO_STOP,0,0,0);
				www_idleintel_com();
				pd_gyro_int(500);
				//motor.c_left_hw = motor.c_right_hw = 0;

				motor_run(dir,500,0,0);
				navigat->out = format_agle(sys->angle,ANGLE_360);
				navigat->walk_dis = 0;
			}
			//if(dir == GO_FORWARD)
			//	run_timeout = 12000;
		}
	}
#if ROOMA_GYROONLY
	else if(dir == 5)
	{
		motor_run(GO_STOP,0,0,0);
		STOP_ALL_MOTOR();
	}
#endif

}

uint8_t proc_motor_run_task(void)
{

	if(sys->man_worktype == 1)
		navigat->walk_dis = 0;
	if(sys->mState == GO_FORWARD)
	{

		if( ++navigat->walk_dis >=run_timeout ||//300ms    500
			((sys->gSta & MASK_BUM_MIDL) ||(sys->gSta & (MASK_BUM_LEFT | MASK_BUM_LEFT2 | MASK_BUM_RIGHT | MASK_BUM_RIGHT2))
		|| MIDLE_IRDA()) || sys->fall_sta)
		{
			log_printf("run timeout,stop,%d,%d\r\n",sys->man_worktype,navigat->walk_dis);		//??开机斤这里了,初始化问题?
			motor_run(GO_STOP,0,0,0);
			if(sys->fall_sta)
			{
				sys->sState = SYS_NAVIGAT;
				motor_back_off(20);
				sys->sState = SYS_IDLE;
			}
			STOP_ALL_MOTOR();

			motor.c_left_hw =0;
			if(sys->full2go)
				sys->f_work_mode = MWO_FULL2GO;
			else
				sys->f_work_mode = sys->work_mod;		//手动模式恢复
			return 0;

		}

		proc_line_pid(navigat->out);
		return 1;
	}else if(sys->mState == GO_BACK)
	{

		if(++navigat->walk_dis >=run_timeout)		//300ms
		{
			log_printf("run timeout,stop\r\n");
			motor_run(GO_STOP,0,0,0);
			STOP_ALL_MOTOR();
			motor.c_left_hw =0;
			if(sys->full2go)
				sys->f_work_mode = MWO_FULL2GO;
			else
				sys->f_work_mode = sys->work_mod;		//手动模式恢复
			return 0;

		}
		proc_back_pid(navigat->out);
		return 1;
	}else if(sys->mState == GO_LEFT || sys->mState == GO_RIGTH)
	{

		www_idleintel_com();								//
		navigat->out = format_agle(sys->angle,ANGLE_360);		//jzz 2018-10-16
		if(++navigat->walk_dis >=run_timeout)		//300ms
		{
			log_printf("run timeout,stop\r\n");
			motor_run(GO_STOP,0,0,0);
			STOP_ALL_MOTOR();
			motor.c_left_hw =0;
			if(sys->full2go)
				sys->f_work_mode = MWO_FULL2GO;
			else
				sys->f_work_mode = sys->work_mod;		//手动模式恢复

			return 0;
		}
	}
	return 0;
}

//前轮计算碰撞
uint8_t caluc_bum_by_front(uint8_t type)
{
//
	//static int16_t c_front = 0;
	//static int		lst_dis=0;
#if 1//BORD_VERSION == 2
	return FRONT_OK;
#else
	if(type ==1)
	{
		if(c_front++ > 10)
		{
			c_front = 0;
			if(navigat->distance == lst_dis)
			{
				if(sys->g_sta[2] >=200 || sys->g_sta[3] >=100 || sys->g_sta[4] >=200)
				{
					return FRONT_BUM;
				}else
					return FRONT_STOP;

			}
			lst_dis = navigat->distance;
		}
	}else
	{
		lst_dis = navigat->distance;
		c_front = 0;
	}
	return FRONT_OK;
#endif
}


char robot_coord_test(short hw,short hw1)
{
#if 0
	int calue=0,ki=0;
	uint8_t ksta=0;


//	int32_t x_org1;//,x_org2;//,
//	int32_t y_org1;//,y_org2;//,;


#if CALE_ADJ_RIGHT
//	int32_t x_org2;//,
	//int32_t y_org2;//,;
#endif
#if CALE_BY_FRONT
//	static int distanc=0;
	int32_t x_org_front,y_org_front;
	float x_org_t,y_org_t;
	//int ret=0; asd

#endif

//	float x_org_f,y_org_f;
#if CALE_ADJ_RIGHT
//	float x_org_r,y_org_r;
#endif


	uint8_t n_sta = LEFT_SIDE_NEAR;//NO_SIDE_NEAR;
//	NEAR_WALL *near = &navigat->near;
	navigat->distance = 0;
	pd_gyro_int(GO_FORWARD_PWM);
	navigat->out =navigat->angle = 0;
	motor.c_left_hw=motor.c_right_hw = 0;
	navigat->distance = 0;

	init_near_wall_navi(n_sta);
	navigat->near.pid->max_lost = 3;		//延边丢失的处理

	cord_calc_store(0);
	gyro_whlmap();
	coordinate_calcu();
	//navigat->x_org_f = navigat->y_org_f = navigat->x_org_r = navigat->y_org_r = navigat->x_org_t = navigat->y_org_t =0;
	log_printf("[go_near_wall]sta=%d,dir=%d,now=(%d,%d,%d,%d,%3.1f)\r\n",navigat->near.n_sta,X_NOW,Y_NOW,navigat->x_org,navigat->y_org,sys->angle);
	motor_run(GO_FORWARD, 850, 0, 0);
	//L_FORWRK_PWM =  sys->lpwm;
	R_FORWRK_PWM =  500;
	navigat->is_walk =1;
	L_FORWRK_PWM =  hw;
	R_FORWRK_PWM =  hw1;
	while(1)
	{

		proc_uart_task();
		get_sensers(&sys->gSta);



		if(TIM5->CNT >=5000)
		{
			TIM5->CNT = 0;
			www_idleintel_com();
			if(sys->angle >=100 && sys->angle < 300)
			{
				if(ksta==0)
				{
					if(ki++ >=4)
					{
						motor_run(GO_STOP,0,0,0);
						break;
					}
				}
				ksta = 1;

			}else
				ksta = 0;

			if(calue++ >=4)
			{
				calue = 0;
				//coordinate_calcu();
				//log_printf("%3.1f %3.1f %3.1f %3.1f %3.1f %3.1f\r\n",
				//	navigat->x_org_f,navigat->y_org_f,navigat->x_org_r,navigat->y_org_r,navigat->x_org_t,navigat->y_org_t);




				//
					disable_irq();
//					x_org_f = navigat->x_org_f ;
				//	y_org_f = navigat->y_org_f;
#if CALE_ADJ_RIGHT
//					x_org_r = navigat->x_org_r;
			//		y_org_r = navigat->y_org_r;
#endif
 #if CALE_BY_FRONT
					x_org_t= navigat->x_org_t;
					y_org_t= navigat->y_org_t;
 #endif
					enable_irq();

//					x_org1	=	x_org_calc(x_org_f);
				//	y_org1	=	y_org_calc(y_org_f);
		 #if CALE_BY_FRONT
					x_org_front = x_org_calc_f(x_org_t);
					y_org_front = y_org_calc_f(y_org_t);

					x_org2	=	x_org_calc_r(x_org_r);
					y_org2	=	y_org_calc_r(y_org_r);

					log_printf("%d %d %d %d %d %d\r\n",x_org1,y_org1,x_org2,y_org2,x_org_front,y_org_front);

		#endif
				L_FORWRK_PWM =  hw;
				R_FORWRK_PWM =  hw1;




			}
		}

	}
	motor_run(GO_STOP,0,0,0);
	STOP_ALL_MOTOR();
	coordinate_calcu();
#endif
	return 0;
}


uint8_t motor_coord_run_test(uint8_t n_sta)
{
	uint8_t sta;
//	float lagle;		//记录最后的延边角度，用于计算是否转弯过多
	uint16_t	gSta;

//	uint32_t	tt;
	int16_t calue=0;
	int ret;



	NEAR_WALL *near = &navigat->near;

	navigat->distance = 0;
	sys->sState = SYS_NAVIGAT;
	pd_gyro_int(GO_NEAR_PWM);
	navigat->out =navigat->angle;
	motor.c_left_hw = motor.c_right_hw = 0;

	init_near_wall_navi(n_sta);

	navigat->is_walk =1;
	delay_ms_sensers(20);
	cord_calc_store(0);
	gyro_whlmap();

	//log_printf("[near_go_back]sta=%d(%d),now=(%d,%d,%d,%d),y=(%d,%d,)bum=%d,walkside=%d\r\n",navigat->near.n_sta,n_sta,X_NOW,Y_NOW,navigat->x_org,navigat->y_org,toy,maxy,maxbum,walk_side);

	sta = sys->sState;
	robot_whele_stop(0);
	sys->g_t_walk = 0;

    //motor.c_left_hw = motor.c_right_hw = 0;
	motor_run(GO_FORWARD, GO_NEAR_PWM, 0, 0);

	while(1)
	{

		proc_uart_task();
		get_sensers(&sys->gSta);
		if(sys->sState !=sta)
			return 0;
		CHECK_NAVI_STA_RT(0);

		//碰撞
		if(sys->gSta & (MASK_BUM_MIDL) ||(sys->gSta & (MASK_BUM_LEFT | MASK_BUM_LEFT2 | MASK_BUM_RIGHT | MASK_BUM_RIGHT2))||   MIDLE_IRDA() ||	RIGHT_IR_BUM2()|| LEFT_IR_BUM2() || ret ==0) 	//超过两个的丢失就另行计算了。
		{
			gSta = sys->gSta;	//记录碰撞的状态
			motor_run(GO_STOP,0,0,0);


			//gyro_mapwhl();


			log_printf("bum,ret=%d\r\n",ret);
			coordinate_calcu_test();
			if(gSta & MASK_BUM_MIDL)
			{
				//delay(10);
				delay_sensor(10);
				motor_back_off(BACK_HW);
				//delay(10);
				delay_sensor(10);
			}
			coordinate_calcu_test();
			coord_org2map(navigat->x_org,navigat->y_org,&navigat->tx,&navigat->ty); //坐标系转换



			if(near->n_sta ==NO_SIDE_NEAR)
			{
				init_near_wall_navi(n_sta);
			}
			if(near->n_sta == RIGHT_SIDE_NEAR)
			{

				if(BUM_LEFT(gSta) || MIDLE_HAVE_IRDA2() || RIGHT_IR_BUM2())
				{
					robot_turn_deg(GO_LEFT,DEG_TURN_PWM,WALL_LOST_DEG);
				}else if(sys->g_sta[6] > 900 && sys->g_sta[5] > 900)//贴墙了，则可以延边，不要转太多
				{
					robot_turn_deg(GO_LEFT,DEG_TURN_PWM,12);
				}else
					robot_turn_deg(GO_LEFT,DEG_TURN_PWM,25);

			}else if(near->n_sta == LEFT_SIDE_NEAR)
			{
				if(BUM_RIGHT(gSta) || MIDLE_HAVE_IRDA2() || LEFT_IR_BUM2())
				{
					robot_turn_deg(GO_RIGTH,DEG_TURN_PWM,WALL_LOST_DEG);
				}
				else if(sys->g_sta[1] > 900 && sys->g_sta[2] > 900)
				{
					robot_turn_deg(GO_RIGTH,DEG_TURN_PWM,12);
				}
				else
				{
					robot_turn_deg(GO_RIGTH,DEG_TURN_PWM,25);
				}

			}
//l_go_nearwall_back:

			motor_run(GO_STOP,0,0,0);
			navigat->distance = 0;
			navigat->is_walk = 1;
			log_printf("run..\r\n");
			coordinate_calcu_test();


			pd_gyro_int(GO_NEAR_PWM);
			navigat->out =sys->angle;
            //motor.c_left_hw = motor.c_right_hw = 0;
			motor_run(GO_FORWARD, GO_NEAR_PWM, 0, 0);
			navigat->near.pid->c_lost = 0;


		}

		if(TIM5->CNT >=5000)
		{
			TIM5->CNT = 0;
			navigat->out = format_agle(navigat->out,ANGLE_360);
			proc_line_pid(navigat->out);
			navigat_near_wall_pid(&navigat->out,5);


			if(calue++ >=10)
			{
				ret = coordinate_calcu_test();
				calue = 0;
				//coordinate_calcu();
				//coord_org2map(navigat->x_org,navigat->y_org,&navigat->tx,&navigat->ty);	//坐标系转换

			}
		}
		 if(sys->mState == GO_STOP)////////jzz 2018-04-28
		{
		//	motor_run(GO_FORWARD,sys->pwm ,0,0);///
			//log_printf("get_sensers  ----sys->mState == GO_STOPr\n");
      		motor_run(GO_FORWARD, GO_NEAR_PWM, 0, 0);	//BUG 会多转
		}
		///



	}
	//delay_ms(1000);
}

void turn_deg_test(uint8_t dir,uint16_t count)
{
	uint16_t i;
	int deg;

	MOTOR_POWER_ON();
	if(sys->shut_down_motor)
		MOTOR_CTRL1(NORM_SIDE_PWM_L,NORM_SIDE_PWM_R,NORM_MID_PWM,sys->dust_pwm_value)
	else
		MOTOR_CTRL(NORM_SIDE_PWM_L,NORM_SIDE_PWM_R,NORM_MID_PWM,sys->dust_pwm_value)
	sys->sState = SYS_NAVIGAT;
	if(dir == 3)
		deg = 90;
	else
		deg = 270;
	log_printf("turn_deg_test,dir=%d,deg=%d,c=%d\r\n",dir,deg,count);
	for(i=0;i<count;i++)
	{
		turn_to_deg(deg);
		delay_ms(100);
		turn_to_deg(180);
		delay_ms(100);
		turn_to_deg(deg);
		delay_ms(100);
		turn_to_deg(0);
		delay_ms(100);

	}
	STOP_ALL_MOTOR()
	sys->sState = SYS_IDLE;
}


void insert_walk_fifo(int16_t xx,int16_t yy)
{
	uint8_t insert=1;
	//log_printf("insert len=%d\r\n",navigat->walk_len);
	if(navigat->walk_len < MAX_WALK_GRID)
	{
		/**/
		if(navigat->walk_len > 0)
		{
			if( xx == navigat->x_walk[navigat->walk_len - 1] && yy == navigat->y_walk[navigat->walk_len - 1] )
			{
				insert = 0;
				//log_printf("insert0\r\n");
			}
		}

		if(insert)
		{
			navigat->x_walk[navigat->walk_len] = xx;
			navigat->y_walk[navigat->walk_len++]=yy;
			//log_printf("insert(%d,%d,%d)\r\n",xx,yy,navigat->walk_len);
		}
	}

}


/*
一个方向旋转
*/
void turn_to_deg_on_dir(int deg,uint8_t dir)
{

	float to_agle = deg;
	uint8_t turn_dir;

	uint32_t 	t_turn=0;

	log_printf("\r\n [turn_on_dir]angle=%3.1f(%3.1f),to=%d(",sys->angle,navigat->angle,deg);

	CHECK_NAVI_STA();

	if(sys->angle > 180 && deg ==0)
	{
		to_agle = 360;
	}
	if(deg == 270 && sys->angle < 90)
	{
			sys->angle +=360;
	}
	if(deg == 90 && sys->angle > 270)
	{
		sys->angle -=360;
	}




	if(dir == GO_LEFT)
	{
		turn_dir = GO_LEFT;
		to_agle -=sys->ajust_agle;
	}else
	{
		turn_dir = GO_RIGTH;
		to_agle +=sys->ajust_agle;
	}




	if(disfloat(sys->angle, to_agle) <=sys->ajust_agle)
	{
		log_printf("the same angle\r\n");
		return ;
	}


	sys->near_wall = 0;
	coordinate_calcu(0);
//	log_printf("%3.1f)%d,pwm(%d,%d)(%d,%d)\r\n",to_agle,turn_dir,sys->left_pwm,cfg->go_forward_pwm,navigat->x_org,navigat->y_org);

	turn_round_pid(0);
    motor.c_left_hw = motor.c_right_hw = 0;

	motor_turn_dir(turn_dir ,TURN_DEG_PWM,TURN_DEG_PWM);
	sys->right_pwm = sys->left_pwm = 680;//cfg->go_forward_pwm;

	sys->near_wall = 0;

	navigat->is_walk = FALSE;
	navigat->c_turn++;


	t_turn = msTmr;
	while(1)
	{
		proc_uart_task();
		get_sensers(&sys->gSta);


		CHECK_NAVI_STA();
		if(TIM5->CNT >=5000)
		{
			TIM5->CNT = 0;
			www_idleintel_com();
			turn_round_pid(TURN_SPEED);//PID

			if(deg >= 270 && sys->angle < 90)
			{
					sys->angle +=360;
			}else if(deg == 90 && sys->angle > 270)
			{
				sys->angle -=360;
			}

			if(disfloat(sys->angle, deg) < sys->ajust_agle)
			{
				motor_run(GO_STOP,0,0,0);
				//delay_ms(200);
				break;
			}

		}

	}

	//proc_gyro_task(WAIT_STOP_CNT, WAIT_STOP_COUNT);				//等待陀螺仪稳定
	//delay_ms_sensers(100);
	www_idleintel_com();
	//cord_calc_store(0);
	//gyro_whlmap();
	coordinate_calcu(0);
	gyro_mapwhl();
	navigat->angle = deg;

	sys->near_wall = 0;
	log_printf(" OK,angle=%f(%f)%d,%d,xy=(%d,%d,%d,%d,%3.3f,%3.3f)\r\n\r\n",sys->angle,navigat->angle,msTmr - t_turn,motor.c_left_hw
						,X_NOW,Y_NOW,navigat->x_org,navigat->y_org,navigat->x_org_f,navigat->y_org_f);

}

void gyro_move_test(uint8_t dir,int c)
{
	int c_turn_test = 0;
	ny3p_play(VOICE_M_START);
	navigat_init(1);
	sys->sState = SYS_NAVIGAT;
	while(1)
	{
		get_sensers(&sys->gSta);			//传感器采样
		proc_uart_task();
		if(sys->sState !=SYS_NAVIGAT)
		{
			motor_run(GO_STOP,0,0,0);
			break;
		}


		motor_go_forwark(0,NO_SIDE_NEAR,NULL);
		if(navigat->angle == 0 && c_turn_test >=c)
			break;
		c_turn_test++;

		log_printf("c_turn_test = %d\r\n",c_turn_test);
		if(navigat->angle == 0)
			turn_to_deg_on_dir(180,dir);
		else
			turn_to_deg_on_dir(0,dir);

	}
	sys->sState = SYS_IDLE;
	while(1)
	{
		delay_ms(500);
		www_idleintel_com();
		log_printf("%f\r\n",sys->angle);
	}
}


#if NEAR_DRAW_MAP

void memset_func_arg(void)
{
#if  NEAR_DRAW_MAP
	navigat->func_arg.type = 0;
	navigat->func_arg.type1= 0;
	navigat->func_arg.arg8_1= 0;
	navigat->func_arg.arg16_1= 0;
	navigat->func_arg.arg16_2= 0;
#endif
}
//画地图的时候
int16_t func_draw_map(void)
{
#if  NEAR_DRAW_MAP
	if(navigat->func_arg.type1 ==1)		//碰撞
	{
		//计算第一次碰撞 到时候的坐标，用于计算回环
		if(navigat->near.n_sta ==NO_SIDE_NEAR)		//初次碰撞，记录下地址坐标
		{
			navigat->func_arg.arg16_1 = X_NOW;		//存储其实的点
			navigat->func_arg.arg16_2 = Y_NOW;
			log_printf("[draw map]first bum,(%d,%d,)\r\n",X_NOW,Y_NOW);
		}
	}else if(navigat->func_arg.type1 ==0)		//延边
	{


	}
	//如果右边没扫到，则判断右边是否扫到  arg8_1 = 1 右边扫到， =0 右边没扫到
	if(navigat->func_arg.arg8_1 == 0)
	{
		if(Y_NOW < (navigat->func_arg.arg16_2 -1))  //右边扫到了。
		{
			navigat->func_arg.arg8_1 = 1;
			//log_printf("draw rightside\r\n");
		}
	}else if(navigat->func_arg.arg8_1 == 1)			//扫过了右边了，看是否又回到左边了
	{
		/*
			又扫会左边，则计算是否闭合
		*/
		if(Y_NOW > navigat->func_arg.arg16_2 )		//又回到了左边了,且90°方向
		{
			//log_printf("left\r\n");
			if(dis_xy(X_NOW,navigat->func_arg.arg16_2) <=4  && (sys->angle < 130 && sys->angle > 50))
			{
				log_printf("draw map OK\r\n");
				return 1;
			}
		}
	}

	//插入边缘表格
	if(navigat->func_arg.arg16_1 > 0 && navigat->func_arg.arg16_2 > 0)
		insert_edge_table(X_NOW,Y_NOW,navigat->x_org,navigat->y_org,sys->angle);
#endif
	return 0;
}



int16_t robot_go_edgeways(uint8_t n_sta ,uint8_t type,m_go_func m_go_check)
{
	int calue=0;
	uint8_t sta;
	short llx=0,lly=0;

	float lagle;		//记录最后的延边角度，用于计算是否转弯过多
	uint16_t	gSta;
	uint32_t	t_begin;//开始的时间
	int ret_calc=1;

	int16_t c_lost=0;
	uint16_t c_round=0;		//转圈的次数

	NEAR_WALL *near = &navigat->near;
	float m_angle[MAX_C_M_ANGLE];			//保存最近的20个碰撞的角度
	int16_t c_m_angle=0;

#if LAGER_ROUND_MODE
	int16_t cyc_x[MAX_C_M_ANGLE],cyc_y[MAX_C_M_ANGLE];
	uint8_t tmp=0;
#else
	int16_t c_near_wall = 0;
#endif

	for(c_m_angle = 0;c_m_angle<MAX_C_M_ANGLE;c_m_angle++)
		m_angle[c_m_angle] = 0;
	c_m_angle = 0;


//	motor_run(GO_ROUND,600,0,0);
	CHECK_NAVI_STA_RT(0);
	pd_gyro_int(GO_NEAR_PWM_FAST);
	navigat->out =navigat->angle;
	navigat->distance = 0;
	motor.c_left_hw = motor.c_right_hw = 0;
	cord_calc_store(0);
	gyro_whlmap();
	robot_whele_stop(0);
	motor_run(GO_FORWARD, GO_NEAR_PWM_FAST, 0, 0);
	if(n_sta & 0x80)
		init_near_wall_navi(n_sta & 0x7F);
	else
		init_near_wall_navi(NO_SIDE_NEAR);
	n_sta &=0x7F;
	//init_near_wall_navi(NO_SIDE_NEAR);	//刚开始不需要一直沿边
	navigat->is_walk =1;
	log_printf("[robot_go_edgeways]sta=%d(%d),type=%d,now=(%d,%d,)\r\n",navigat->near.n_sta,n_sta,type);
	//navigat->wheel_dir = 1;
	sta = sys->sState;
	lagle = sys->angle;
	mstimeout(&t_begin,0);
	sys->g_t_walk = 0;
	while(1)
	{

		proc_uart_task();
		get_sensers(&sys->gSta);
		if(sys->sState !=sta)
			return RET_NEAR_ERROR;
		CHECK_NAVI_STA_RT(0);
		//碰撞
		if(sys->gSta & (MASK_BUM_MIDL) ||(sys->gSta & (MASK_BUM_LEFT | MASK_BUM_LEFT2 | MASK_BUM_RIGHT | MASK_BUM_RIGHT2))||  ret_calc==0 ||  c_lost >=MAX_NEAR_LOST || MIDLE_IRDA())
		{
			motor_run(GO_STOP,0,0,0);
			//log_printf("BUM\r\n");
			//delay_ms(200);
			if(sys->gSta & (MASK_BUM_MIDL))
			{
				if(sys->fall_sta)		//跌落，则
					motor_back_off(BACK_HW*8);
				else
					motor_back_off(BACK_HW);

			}
			coordinate_calcu(0);														//计算出原始的坐标系
			coord_org2map(navigat->x_org,navigat->y_org,&navigat->tx,&navigat->ty); //坐标系转换
			motor_run(GO_STOP,0,0,0);
			log_printf("\r\n-----edgeways bum(%d,%d,%d,%f,%f),gsta=%d,irda=(%d,%d,%d,%d)angle=%3.1f,ret=%d,lost=%d\r\n",navigat->tx,navigat->ty,motor.c_left_hw,navigat->x_org_f,navigat->y_org_f,
							sys->gSta,sys->g_sta[0],sys->g_sta[1],sys->g_sta[5],sys->g_sta[6],sys->angle,ret_calc,c_lost);


			if(!(ret_calc==0 || c_lost >=MAX_NEAR_LOST))
				c_round = 0;
			gSta = sys->gSta;
			if(ret_calc==0)
					gyro_mapwhl();
			//motor_run(GO_STOP,0,0,0);
			if(sys->work_mod & MWO_MOP)
				//delay_ms(200);
				delay_sensor(200);
			//www_idleintel_com();
			lagle = sys->angle;
			//init_near_wall_navi(n_sta);		//碰撞后，才开始一直沿边

			navigat->distance = 0;
			motor.c_left_hw = 0;

#if !LAGER_ROUND_MODE
			if(c_m_angle >=MAX_C_M_ANGLE)
				c_m_angle = 0;
			m_angle[c_m_angle++] = sys->angle;		//保持角度

			if(near_round_360(m_angle,c_m_angle))
			{
				log_printf("found round...go line...\r\n");

				init_near_wall_navi(NO_SIDE_NEAR);
				for(c_m_angle = 0;c_m_angle<MAX_C_M_ANGLE;c_m_angle++)
					m_angle[c_m_angle] = 0;
				c_m_angle = 0;
				goto l_go_edeways;
			}
#else////2018-07-10  未测试5

			if(near_large_round_360(cyc_x,cyc_y,m_angle,c_m_angle)) 					///绕大桩判断函数
			{
				log_printf("found large round...go line...\r\n");
				if(near->n_sta == RIGHT_SIDE_NEAR)
				{
					robot_turn_deg(GO_LEFT,DEG_TURN_PWM,135);
					//goto l_mgo_edeways;
				}else if(near->n_sta == LEFT_SIDE_NEAR)
				{
					robot_turn_deg(GO_RIGTH,DEG_TURN_PWM,135);
					//goto l_mgo_edeways;
				}
				for(tmp=0;tmp<c_m_angle;tmp++)
				{
					log_printf("%02d(%03d,%03d,%03f)\r\n",tmp,cyc_x[tmp],cyc_y[tmp],m_angle[tmp]);
				}
				init_near_wall_navi(NO_SIDE_NEAR);
				for(c_m_angle = 0;c_m_angle<MAX_C_M_ANGLE;c_m_angle++)
					m_angle[c_m_angle] = 0;
				c_m_angle = 0;
				goto l_go_edeways;
			}
#endif

			//if(ret_calc==0)
			//		gyro_mapwhl();

			if(m_go_check !=NULL)
			{
				navigat->func_arg.type1 = 1;	//bum
				int16_t ret = m_go_check();
				if( ret > 0)
				{
					log_printf("m_go_check\r\n");
					return ret;
				}
			}



			if(near->n_sta ==NO_SIDE_NEAR)
			{
				init_near_wall_navi(n_sta);
			}

			if( (ret_calc==0 || c_lost >= MAX_NEAR_LOST)&& *(navigat->near.pid->adc) < navigat->near.pid->min_adc )
			{
				c_round++;
				log_printf("u  lost or calc error(%d,%d),cround=%d\r\n",ret_calc,c_lost,c_round);
				if(ret_calc==0)
					gyro_mapwhl();
				if(near->n_sta == RIGHT_SIDE_NEAR)
				{
					robot_turn_deg(GO_RIGTH,DEG_TURN_PWM,12);
					goto l_go_edeways;
				}else if(near->n_sta == LEFT_SIDE_NEAR)
				{
					robot_turn_deg(GO_LEFT,DEG_TURN_PWM,12);
					goto l_go_edeways;
				}

			}

			if(near->n_sta == RIGHT_SIDE_NEAR)
			{

				if(BUM_LEFT(gSta)
					/*|| MIDLE_HAVE_IRDA2()*/)
				{
					//log_printf("midbum,60deg\r\n");
					robot_turn_deg(GO_LEFT,DEG_TURN_PWM,WALL_LOST_DEG);
				}else if((gSta & MASK_BUM_RIGHT ) && RIGHT_IRDA2() )
				{
					//log_printf("right bum,10deg\r\n");
					//robot_turn_deg(GO_LEFT,650,12);
					robot_turn_deg(GO_LEFT,DEG_TURN_PWM,12);
				}else
				{
					//log_printf("midbum,30deg\r\n");
					//robot_turn_deg(GO_LEFT,650,30);
					robot_turn_deg(GO_LEFT,DEG_TURN_PWM,30);
				}
			}else if(near->n_sta == LEFT_SIDE_NEAR)
			{
				//log_printf("gsta=%d,left=%d,%d\r\n",sys->gSta,sys->g_sta[0],sys->g_sta[1]);

				if( BUM_RIGHT(gSta)
					/*|| MIDLE_HAVE_IRDA2()*/ )
				{
					//log_printf("midbuml,60deg\r\n");
					robot_turn_deg(GO_RIGTH,DEG_TURN_PWM,WALL_LOST_DEG);
				}else if((gSta & MASK_BUM_LEFT) && LEFT_IRDA2())
				{
						//log_printf("midbuml,12deg\r\n");
					robot_turn_deg(GO_RIGTH,DEG_TURN_PWM,12);
				}else
				{
					//log_printf("midbuml,30deg\r\n");
					robot_turn_deg(GO_RIGTH,DEG_TURN_PWM,30);
				}

			}else
			{
				log_printf("RET_NEAR_ERROR,nsta=%d\r\n",n_sta);
				return RET_NEAR_ERROR;
			}
l_go_edeways:

			c_lost=0;
			motor_run(GO_STOP,0,0,0);

			navigat->distance = 0;
			navigat->is_walk = 1;
			pd_gyro_int(GO_NEAR_PWM_FAST);
			navigat->out =sys->angle;
			cord_calc_store(0);
			//gyro_whlmap();
			//motor.c_left_hw = motor.c_right_hw = 0;
			motor_run(GO_FORWARD, GO_NEAR_PWM_FAST, 0, 0);
			navigat->near.pid->c_lost = 0;
			ret_calc = 1;
			c_near_wall = 0;
			sys->g_t_walk = 0;
		}

		if(TIM5->CNT >=5000)
		{
			TIM5->CNT = 0;
			if(robot_whele_stop(1))
					continue;			//直接出来，给碰撞做准备
			navigat->out = format_agle(navigat->out,ANGLE_360);
			proc_line_pid(navigat->out);
			navigat_near_wall_pid(&navigat->out,5);
			if(near->n_sta ==NO_SIDE_NEAR)
			{
				if(RIGHT_ADC() >= cfg->lock_right_adc)
					init_near_wall_navi(RIGHT_SIDE_NEAR);
				else if(LEFT_ADC() >= cfg->lock_left_adc)
					init_near_wall_navi(LEFT_SIDE_NEAR);
			}

			if( *(navigat->near.pid->adc) > navigat->near.pid->min_adc) //延边
			  lagle = sys->angle;
			else		//延边消失，转的角度超过180度，则失败退出
			{
				if(disfloat( lagle , sys->angle) > 180)
				{
					log_printf("lost over(%d,%d,%3.1f,%3.1f)\r\n",X_NOW,Y_NOW,sys->angle,lagle);
					motor_run(GO_STOP,0,0,0);
					//delay_ms_sensers(200);
					coordinate_calcu(0);														//计算出原始的坐标系
					coord_org2map(navigat->x_org,navigat->y_org,&navigat->tx,&navigat->ty);	//坐标系转换
					return RET_NEAR_ERROR;
				}
			}
#if CALE_BY_FRON
				ret_calc = coordinate_calcu(1);														//计算出原始的坐标系
#endif
			if(calue++ >=40)
			{
				calue = 0;

#if !LAGER_ROUND_MODE

				if(c_near_wall ++ >=30)
				{

					for(c_m_angle = 0;c_m_angle<MAX_C_M_ANGLE;c_m_angle++)
						m_angle[c_m_angle] = 0;
					c_m_angle = 0;
					c_near_wall = 0;
					log_printf("log near\r\n");
				}
#endif
#if !CALE_BY_FRON
				ret_calc = coordinate_calcu(0);														//计算出原始的坐标系
#endif
				//ret_calc = coordinate_calcu();														//计算出原始的坐标系
				coord_org2map(navigat->x_org,navigat->y_org,&navigat->tx,&navigat->ty);	//坐标系转换


				if( *(navigat->near.pid->adc) > navigat->near.pid->min_adc) //延边
				{
					//c_near++;
					c_round = 0;

				}
				else if(near->n_sta !=NO_SIDE_NEAR)	//强制延边，则计数丢失的个数
					c_lost ++;

				if(llx!=navigat->tx || lly!=navigat->ty)
				{

					if(m_go_check !=NULL)
					{
						navigat->func_arg.type1 = 0;	//bum
						int16_t ret = m_go_check();
						if( ret > 0)
						{
							log_printf("m_go_check\r\n");
							return ret;
						}
					}
#if WALK_PRINTF
				//	log_printf("*(%d,%d,%3.1f,0)-[%3.1f,%3.1f,0]*\r\n",navigat->tx,navigat->ty,sys->angle,navigat->x_org_f,navigat->y_org_f);
#endif
#if LAGER_ROUND_MODE
					if(c_m_angle >=MAX_C_M_ANGLE)
					{
						for(tmp=0;tmp<MAX_C_M_ANGLE-1;tmp++)
						{
							cyc_x[tmp] = cyc_x[tmp+1];
							cyc_y[tmp] = cyc_y[tmp+1];
							m_angle[tmp] = m_angle[tmp+1] ;
						}
						cyc_x[MAX_C_M_ANGLE-1] = X_NOW;
						cyc_y[MAX_C_M_ANGLE-1] = Y_NOW;
						m_angle[MAX_C_M_ANGLE-1] = sys->angle;				//保证最新的40个点
					}
					else
					{
						cyc_x[c_m_angle] = X_NOW;
						cyc_y[c_m_angle] = Y_NOW;
						m_angle[c_m_angle++] = sys->angle;		//保存角度
					}
#endif

					llx = navigat->tx;
					lly = navigat->ty;
				}


			}
		}

	}

//	return 0;
	//delay_ms(1000);
}

/*
画地图的时候矫正坐标
*/
#define MAX_C_AJD_MAP		14

int check_adj_draw_map(uint16_t *cx,uint16_t *cy)
{
	//看X轴，是不是有可以矫正的
//	int angle = 0;
	log_printf("[adj_draw_map]cx=%d,angle=%3.1f\r\n",*cx,sys->angle);
	//===============================================Y轴的校验允许==================================
	if(*cx >=20)
	{
		//先找出X轴要去的方向。
		int x;
		//==========往Y-方向走，则看X+方向==========
		if((sys->angle <= 0 || sys->angle <= 360)  && sys->angle >=180)
		{
			log_printf("k1\r\n");
			for(x=navigat->tx;x<navigat->tx+30;x++)
			{
				//找到障碍点
				if(OBST_XY(x,navigat->ty)  )
				{
					//距离太小，就退出了
					if(dis_xy(x,X_NOW) <=2)
					{
						log_printf("narrow_1\r\n");
						break;
					}
					//一条线则考虑过去
					if(check_line(CHECK_Y_LINE,x,navigat->ty,2) == 1)
					{
						//侧面没有障碍物，也就是一条直线的障碍物。
						if(OBST_XY(x+1,navigat->ty) == 0 && OBST_XY(x-1,navigat->ty) == 0 &&
						  OBST_XY(x+1,navigat->ty+1) == 0 && OBST_XY(x-1,navigat->ty+1) == 0 &&
						  OBST_XY(x+1,navigat->ty-1) == 0 && OBST_XY(x-1,navigat->ty-1) == 0 )
						 {
							OBST *obst = get_obst(x,navigat->ty);
							//检查障碍物的方向是否正确
							if(obst !=NULL)
							{
								//方向正确
								if(obst->x > obst->tx)
								{
									motor_run(GO_STOP,0,0,0);
									navigat->draw_map_bk_angle = 180;		//回去的角度是0
									//navigat->draw_map_adj_dist = (x - navigat->tx + 4) * HW_GRID;
									navigat->draw_map_adj_x = x;
									log_printf("found adj obst(%d,%d,)to 180,bk=0\r\n",x,navigat->ty );
									turn_to_deg(0);

									return 1;			//要去校验了。
								}
							}
						}
					}else
						log_printf("not line1\r\n");
					break;
				}
			}
			return 0;
		}


		if(sys->angle  >= 0  && sys->angle <= 180)
		{
			log_printf("k2\r\n");
			for(x=navigat->tx;x > navigat->tx -30;x--)
			{
				//找到障碍点
				if(OBST_XY(x,navigat->ty)  )
				{
					//距离太小，就退出了
					if(dis_xy(x,navigat->tx) <=2)
					{
						log_printf("narrow_2\r\n");
						break;
					}
					//一条线则考虑过去
					if(check_line(CHECK_Y_LINE,x,navigat->ty,2) == 1)
					{
						if(OBST_XY(x+1,navigat->ty) == 0 && OBST_XY(x-1,navigat->ty) == 0 &&
						  OBST_XY(x+1,navigat->ty+1) == 0 && OBST_XY(x-1,navigat->ty+1) == 0 &&
						  OBST_XY(x+1,navigat->ty-1) == 0 && OBST_XY(x-1,navigat->ty-1) == 0 )
						 {
								OBST *obst = get_obst(x,navigat->ty);
								//检查障碍物的方向是否正确
								if(obst !=NULL)
								{
									//方向正确
									if(obst->x < obst->tx)
									{
										motor_run(GO_STOP,0,0,0);
										navigat->draw_map_bk_angle = 0;		//回去的角度是0
										//navigat->draw_map_adj_dist = (navigat->tx - x + 4) * HW_GRID;
										navigat->draw_map_adj_x = x;
										log_printf("found adj obst(%d,%d,),to 0,bk=180\r\n",x,navigat->ty);
										turn_to_deg(180);

										return 1	;		//要去校验了。
									}
								}
						}
					}else
					   log_printf("not line2\r\n");
					break;
				}
			}
			return 0;
		}

	}
	return 0;
}


//SIDEOBST	m1_obst;

/*****************************************************************************
 * 函数名称:
 * 函数入参: n_sta,延边状态 最高位为1，则进来就强制延边 ,fitt:是不是要扫描拟合地图
 * 函数功能:	延边扫地图，碰撞次数过多，或距离过远，就回来了
 *			  max_grid - 0 一值走到回到原点， >0多少个障碍物后退出。fitt有效的时候，才有效，也就是拟合地图的时候才有效。
 *****************************************************************************/
char motor_go_draw_map(uint8_t n_sta)
{

	memset_func_arg();
	navigat->edge_len = 0;
	robot_go_edgeways(RIGHT_SIDE_NEAR ,0,func_draw_map);

	return 0;
}
#endif


void gyro_offset_manage(u16 cnt)
{
int i;
	log_printf("gyro_offset_manage..\r\n");
	delay_ms_tm5(200);
	SPI1_ReadWriteByte(0xB1);
	delay_ms_tm5(800);
	for(i=0;i<100;i++)
	{
		if(get_gyro_ofs())
		{
			log_printf("ofset ok,i=%d\r\n",i);
			return ;
		}
		get_gyro_org();
		www_idleintel_com();
		delay_ms_tm5(50);
	}
	log_printf("angle=%3.3f,i=%d\r\n",(sys->angle),i);


}


uint8_t reset_gyro(uint8_t sound)
{
	int i;;//,c=0;
	log_printf("reset_gyro2...\r\n");
	www_idleintel_com();
	//delay_ms_tm5(100);

	mpu6500_date_offset(800);
	sys->yaw = 0;
	log_printf("angle=%3.3f,i=%d\r\n",sys->angle,i);
	return 0;
	//return (get_gyro_org());


}

uint8_t reset_angle(void)
//uint8_t reset_gyro(void)
{
	sys->yaw = sys->angle = 0;
	log_printf("sys_angle,OK");
/*
	int i,c=0;
	log_printf("reset_angle...\r\n");
	//www_idleintel_com();
	//delay_ms(10);
	//等待陀螺仪静止
	for(i=0;i<300;i++)
	{
		//illegal_copy_tell_13757122544_gsend(0x01,0,0);
		if(get_gyro_ofs())
			break;
		delay_ms(20);

		c++;
		if(c >=50)
		{
			ny3p_play(VOICE_DIDI);
			c =0;
		}
	}
	log_printf("gyro standby..\r\n");

	for(i=0;i<5;i++)
	{
		illegal_copy_tell_13757122544_gsend(0x01,0,0);
		delay(5);
		www_idleintel_com();
		if(disfloat(sys->angle,0) < 2)
		{
			log_printf("angle=%f,rsetok,i=%d\r\n",sys->angle,i);
			delay_ms(50);
			return 1;
		}
		delay(10);
	}
*/
	return 0;
	//return (get_gyro_org());
}

#if (1 == SIMPLIFY)
char test_go_line(void)
{
	short c_calc = 0;
	unsigned int t_walk = 0;
	int ret =1;
#if PRINTF_PWR_STA
	uint8_t c_power=0;
#endif

 	CHECK_NAVI_STA_RT(0);
	pd_gyro_int(GO_FORWARD_PWM);

	www_idleintel_com();  //取一下角度   navigat->angle;
	motor.c_left_hw = motor.c_right_hw = motor.c_front_hw = 0;
	navigat->out = navigat->angle;

	navigat->is_bum = 0;
	navigat->distance = sys->c_pid_hw = 0;
	sys->gsta= 0;

	TIM5->CNT = 0;
	gyro_whlmap();

	calc_timr_upload();
	navigat->l_dist = navigat->distance;

#if JUDGE_PID_CTRL_WHEEL_STOP
	m_speed_pid_ctrl(0,0,0,0,0,0,0);
#else
	m_speed_pid_ctrl(0,0);
#endif

	robot_whele_stop(0);
	delay_ms_sensers(5);

	motor.c_left_hw = motor.c_right_hw = 0;

	navigat->out = format_agle(navigat->out,ANGLE_360);
	if(1 == sys->test_line_go_ctrl)
	{
		motor_run(GO_FORWARD, GO_FORWARD_PWM, 0, 0);
	}
	else if(2 == sys->test_line_go_ctrl)
	{
		motor_run(GO_BACK, BACK_OFF_PWM, 0, 0);
	}

	c_calc = 40;

	printf_power_sta();

	while(1)
	{
		get_sensers(&sys->gSta);
		proc_uart_task();

		if(SYS_IDLE == sys->sState)
		{
			ret = 1;
			return ret;
		}

		if(TIM5->CNT >=5000)
		{
			//log_printf("[test_line]gSta=%d(%d), dir=%d, dist=%d\r\n", sys->gSta, sys->gsta,sys->walk_ctrl, navigat->distance);

			if(120000 <= ++t_walk)	//10分钟退出
			{
				sys->sState = SYS_IDLE;
				init_sys_sta(SYS_IDLE);
				STOP_ALL_MOTOR();
				ret = 1;
				return ret;
			}

			TIM5->CNT = 0;
			//robot_whele_stop(2);

			if(1 == sys->test_line_go_ctrl)
			{
				proc_line_pid(navigat->out);
			}
			else if(2 == sys->test_line_go_ctrl)
			{
				proc_back_pid(navigat->out);
			}

			c_calc++;
			if(c_calc >=50)		//250ms
			{
				c_calc = 0;
				//速度环
				m_speed_pid_ctrl(0,1,0,10,0,10,50);
			}
		}

		if((sys->gSta & (MASK_BUM_LEFT |  MASK_BUM_RIGHT))  ||(sys->gSta & (MASK_BUM_LEFT | MASK_BUM_LEFT2 | MASK_BUM_RIGHT | MASK_BUM_RIGHT2))||  MIDLE_IRDA() /*|| (2000 < WALK_DIST())*/)
		{
			sys->gsta = sys->gSta ;
			//log_printf("[test_line]gSta=%d(%d), dir=%d, dist=%d\r\n", sys->gSta, sys->gsta,sys->test_line_go_ctrl, navigat->distance);
#if (1)
			sys->sState = SYS_IDLE;
			init_sys_sta(SYS_IDLE);
			STOP_ALL_MOTOR();
			ret = 1;
			return ret;
#else
			motor_run(GO_STOP,0,0,0);
			delay_ms(100);

			if(1 == sys->test_line_go_ctrl)			//前进变后退
			{
				sys->test_line_go_ctrl = 2;
				navigat->distance = 0;
				motor_run(GO_BACK, BACK_OFF_PWM, 0, 0);
			}
			else if(2 == sys->test_line_go_ctrl)		//后退变前进
			{
				sys->test_line_go_ctrl = 1;
				navigat->distance = 0;
				motor_run(GO_FORWARD, GO_FORWARD_PWM, 0, 0);
			}

			ret = 0;
			continue;
#endif
		}
	}

	printf_power_sta();
	return ret;
}
#endif

void wifi_motor_single_run(uint8_t dir)
{
	log_printf("motor_single_run,dir=%d,mSta=%d\r\n",dir,sys->mState);
	if(dir == GO_FORWARD || dir == GO_BACK || dir == GO_LEFT || dir == GO_RIGTH)
	{
		if(sys->mState == dir)
		{
			navigat->walk_dis = 0;
			run_timeout = DIR_NO_LIMIT;
			log_printf("the same dir ,flash timer\r\n");
		}
		else
		{
			log_printf("run...");
			run_timeout = DIR_NO_LIMIT;
			MOTOR_CTRL2(NORM_SIDE_PWM_L,NORM_SIDE_PWM_R,0,sys->dust_pwm_value);
			motor_run(GO_STOP,0,0,0);
			www_idleintel_com();
			pd_gyro_int(600);
			motor_run(dir,600,0,0);
			navigat->out = format_agle(sys->angle,ANGLE_360);
			navigat->walk_dis = 0;
		}
	}
	else if(dir == GO_STOP)
	{
		www_idleintel_com();
		navigat->out = format_agle(sys->angle,ANGLE_360);
		run_timeout = 0;
		navigat->walk_dis = 0;
		motor_run(GO_STOP,0,0,0);
		STOP_ALL_MOTOR();
	}
}
#if 1
char motor_go_spot_task(void)
{
	static short c_calc = 0;
//	int ret =1;
	uint8_t  sta = sys->sState ;
//	uint16_t kldist=0;
//	int16_t bx,by;
//#if PRINTF_PWR_STA
//	uint8_t c_power=0;
//#endif
//
			LED1_ON();
			LED2_OFF();
			LED3_OFF();
			LED4_OFF();
			sys->t_flash_halt = 0;
			sys->t_halt_on = 1;

 	CHECK_NAVI_STA_RT(0);
 	log_printf("[motor_go_spot_task]begin ,reset gyro..\r\n\r\n");
	//illegal_copy_tell_13757122544_gsend(0x02,0,100);
	//delay_ms(10);
//	www_idleintel_com();  //取一下角度   navigat->angle;
	motor.c_left_hw = motor.c_right_hw = motor.c_front_hw = 0;
	//motor.c_left_hw = motor.total_left_hw = motor.total_right_hw=0;



	navigat->out = navigat->angle;



	navigat->distance = sys->c_pid_hw = 0;
	sys->gsta= 0;

	TIM5->CNT = 0;
//	init_near_wall_navi(nsta); //NO_SIDE_NEAR


	navigat->l_dist = navigat->distance;
	sys->c_front_stop = 0;		//前轮停止的次数清零

	m_speed_pid_ctrl(0,0);		//记录左轮霍尔数
	robot_whele_stop(0);
	delay_ms_sensers(5);

	motor.c_left_hw = motor.c_right_hw = 0;


	motor_wheel_forward(LEFT_WHEEL, 650);
	motor_wheel_forward(RIGHT_WHEEL, 900);
	sys->sState = GO_FORWARD;
	//delay_ms(2);
	navigat->is_walk = 1;
	c_calc = 40;

//	l_dist = 0;
	sys->navi_run = 1;
	sys->walk_bum = 0;
	printf_power_sta();

	uint16_t step=0,maxc=0;
	uint16_t  disa,spot_speed;
	step = cfg->spot_step_disa / 100;
	disa = cfg->spot_step_disa % 100;
	spot_speed = cfg->spot_speed_maxc / 100;
	maxc = cfg->spot_speed_maxc % 100;
	//agitator_speed_pid_ctrl(0,0);
	log_printf("[motor_go_spot_task]angle=%3.1f,speed=%d,maxc=%d,step=%d,disa=%d\r\n\r\n",sys->angle
				,spot_speed,maxc,step,disa);
	m_speed_pid_indep(0,0,0);
	uint16_t cycle=0;
//	uint8_t  t_cycle=0;

	uint16_t speed=0;
	sta = sys->sState = SYS_FOCUS;
	float angle = sys->angle;

	while(1)
	{
		get_sensers(&sys->gSta);			//
		proc_uart_task();
		if(sys->sState != sta || sys->sState == SYS_IDLE)			//
		{
			log_printf("SYS_IDLE2,sta=%d\r\n",sys->sState);
			navigat->is_walk = 0;
//			ret = 0;
			break ;
		}


		if(TIM5->CNT >=5000)
		{
			TIM5->CNT = 0;
		//	www_idleintel_com();
			if(disfloat(angle,sys->angle) > (float)disa)
			{
				cycle++;
//				t_cycle = 0;
				if(speed < spot_speed)
					speed+=step;

				log_printf("cycle = %d,set speed=%d\r\n",cycle,speed);
				if(cycle >=maxc)
				{
					log_printf("ok\r\n");
					break;
				}
				log_printf("inc speed,angle=%3.1f,%3.1f,speed=%d\r\n",sys,angle,speed);
				angle = sys->angle;
			}




			c_calc++;
			//if(c_calc % 10 ==0)
			//	log_printf("i={%d,%d,%d}%d,%d\r\n",I_MID_ADC(),I_MOTER_RIGHT(),I_MOTER_LEFT(),sys->i_left_whele,sys->i_right_whele);
			if(c_calc >=50)		//250ms
			{
				c_calc = 0;
				//log_printf("...speed=%d\r\n",speed);
				m_speed_pid_indep(spot_speed,speed,1);
			//	agitator_speed_pid_ctrl(150,1);
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
		//	log_printf("[motor_go_fw]3\r\n");
			uint16_t lpwm = L_FORWRK_PWM;
			uint16_t rpwm = R_FORWRK_PWM;

			sys->gsta = sys->gSta ;						//& MASK_BUM_MIDL;
			sys->walk_bum = sys->bum_sta;				//记录碰撞的状态

			if(MIDLE_IRDA() &&  WALK_DIST() > IR_DIST)
				sys->gsta |= MASK_FW_MIDL;				//中间的红外




			log_printf("[motor_go_spot_task]bum!gSta=%d(%d),ir=(%d,%d,%d)%d,dist=%d,dock=%d，adj=%d!!!\r\n",sys->gSta,sys->gsta,sys->g_sta[2] ,sys->g_sta[3] ,sys->g_sta[4],MIN_MIDLE_IDA,0,0);
			log_printf("lrdrop(%d,%d)\r\n",sys->g_buton[BOTOM_IDX][BOTOM_RIGHT_IDX],sys->g_buton[BOTOM_IDX][BOTOM_LEFT_IEX] ,sys->t_adj_run);

#if 0
						motor_run(GO_STOP,0,0,0);
						break;
#else
			//			int out_dis = cycle*1000;	//退出的距离
						//motor_go_edgeways_for_spot(LEFT_SIDE_NEAR,out_dis);
						robot_turn_deg(GO_RIGTH,DEG_TURN_PWM,15);
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
	sys->sState = SYS_IDLE;
	return 0;
}
#endif
