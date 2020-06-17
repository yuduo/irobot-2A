
#include "sys.h"


unsigned char IrData_bak[4];

ChargeInfo charge_info;

#define WALK_DIST_MAX	7000	//walk_dist
#define YINDX_MAX		7
int yaw_queu[]=	{0,		180,	72,		252,	144,	324,	216,	36,		288,	108 };
int dist_queu[]=	{WALK_DIST_MAX,	WALK_DIST_MAX,	WALK_DIST_MAX,	WALK_DIST_MAX,	WALK_DIST_MAX,	WALK_DIST_MAX,	WALK_DIST_MAX,	WALK_DIST_MAX,	WALK_DIST_MAX,	WALK_DIST_MAX};

static uint16_t yindx=0;

//转角计数
float TURN_ANGLE = 0.5f;
float TURN_ANGLE1 = 0.8f;
/*
回充初始化，可能会是从导航模式过来，
如果从导航模式过来，且有充电桩的坐标，则先导航回去。
*/
void init_charge(uint8_t sta)     //初始化
{
		//static uint8_t c_supend=0;
		//SET_DST_MOTER(0);
	memset(&charge_info,0x00,sizeof(charge_info));
	reset_gyro(1);
	www_idleintel_com();
	//CHECK_IDLE_STA();
	sys->sState = SYS_DOCK;          //状态是回充
	sys->sState_auxi = 1;
	charge_info.out = sys->angle;
	charge_info.first_run = 0;			//0 要转圈

	charge_info.knock_back = 0;			//碰撞次数，避免中线处频繁的碰撞
	charge_info.c_turn = 0;				//碰撞转圈次数，8次后，开始沿边
	charge_info.c_side = 0;
	charge_info.disc_signal = 0;
	charge_info.dock_side = DOCK_NO ;

	yindx = 0;
	for(int i=0;i<YINDX_MAX;i++)
		dist_queu[i] = WALK_DIST_MAX;
	//读取转角基准值，越大，转角幅度越大
	int16_t a=cfg->r[IC_DOCK_SET_ANGLE] /100;
	TURN_ANGLE = (float)a / 10.0f;
	a=cfg->r[IC_DOCK_SET_ANGLE] % 100;
	TURN_ANGLE1 = (float)a / 10.0f;
	log_printf("init dock,last sta=%d,from charge=%d,x_dock=%d,now=(%d,%d,),turn angle=%3.3f,%3.3f\r\n",
		sta,navigat->from_charge,navigat->x_dock,X_NOW,Y_NOW,TURN_ANGLE,TURN_ANGLE1);         //充电座的坐标

	sys->dock_near = 0;
#if 0
	//从导航模式过来，比如工字清扫扫完，或工字清扫按回充。如果有充电桩的位置，这先导航回去。
	if(sta == SYS_NAVIGAT && navigat->from_charge && navigat->x_dock > 0 && navigat->x_dock < MAX_GRID )		//从导航过来的，则导航过去
	{
		if(X_NOW > 0 && X_NOW < MAX_GRID && Y_NOW > 0 && Y_NOW < MAX_GRID)
		{
			log_printf("navi to dock...\r\n");
			navigat->mx = X_NOW;
			navigat->my = Y_NOW;
			navi_to_dock();
			if(sys->sState == SYS_IDLE)
				return;
			//从充电桩出来，则导航回到充电桩附近
			navigat->suspend = FALSE;
			navigat_to_point(navigat->x_dock,navigat->dock_y,0) ;

		}
	}
#endif
	sys->t_work = msTmr;		//记录开始的时间
	sys->t_navi_work=0;
	sys->work_tmr = 0;
	sys->pwm = 600;
	//charge_info.dock_fw = RESET;
	charge_info.side_keep_max = 0;
	ir_dock_insight(1);
	sys->sState = SYS_DOCK;
	motor_run(GO_STOP,0,0,0);
	MOTOR_POWER_ON();
	//MOTOR_CTRL_SIDE(DOCK_SIDE_PWM_L,DOCK_SIDE_PWM_R);
	SET_MID_MOTER(200);     //20200429 原100现200 2a是14v电机要求与2b速度一致   20200506改为0 客户要求回充不开滚刷
	DUST_MOTOR_OFF();
	log_printf("init_charge...end..\r\n");
}

void proc_charge_task(void)
{
	static int c_charge = 0;
	static int c_calus=0;

	DUST_MOTOR_OFF();		//
	SET_MID_MOTER(200);     //回充只开滚刷  没电时所有电机都不开  加了之后待测试。。。
	CHECK_IDLE_STA();
	if(sys->sState != SYS_DOCK)
	{
		motor_run(GO_STOP,0,0,0);
		return ;
	}
	//===========================================碰撞================================================
	if(sys->gSta & MASK_BUM_ALL /*||  (MIDLE_IRDA() )*/)
	{
		charge_info.knock_back++;
		log_printf("bum!!!,gsta=%d,c=%d,dock_sta=%d,dock_side=%d,cturn=%d,yindx=%d\r\n",sys->gSta,charge_info.c_midle,charge_info.knock_back,charge_info.dock_side,charge_info.c_turn,yindx);

		motor_run(GO_STOP,0,0,0);

		//www_idleintel_com();
		motor_run(GO_STOP,0,0,0);

		//没发现中线，或是多次碰撞。则先随机乱跑8次，超过8次，则沿边

		//有信号，而且碰撞超过5次
		if(charge_info.disc_signal  && charge_info.knock_back > 3)
		{
			log_printf("first 3...=%d\r\n",charge_info.knock_back);
			uint16_t gSta = sys->gSta;
			charge_back_off(800);
			if(gSta & (MASK_BUM_LEFT | MASK_BUM_LEFT2))	//左边碰撞，往右转
				robot_turn_deg(GO_RIGTH,700,85);
			else
				robot_turn_deg(GO_LEFT,700,85);
			//强制抛开
			navigat->angle = sys->angle;
			//motor_go_forwark(2000,NO_SIDE_NEAR,0,NULL);
			motor_go_forwark(2000,NO_SIDE_NEAR,NULL);
			sys->dock_near = 0;
			sys->sState = SYS_DOCK;
			charge_info.first_run = 0;		//转圈找充电桩
//					charge_info.c_lost = 0;
			charge_info.c_midle = 0;
			charge_info.knock_back = 0;

		}else

		if( charge_info.c_midle < 5 || charge_info.knock_back >=5)
		{
			/*
			//随机碰撞8次
			if(charge_info.c_turn < 4)
			{
				log_printf("bum ,c=%d,random...\r\n",charge_info.c_turn);
				charge_back_off(800);
				charge_info.first_run = 0;		//转圈找充电桩
//				charge_info.c_lost = 0;
				charge_info.c_midle = 0;
			}
			else if(charge_info.c_turn < 15)
			{
				charge_back_off(800);
				charge_info.c_turn++;
				if(motor_turn_for_dock(GO_RIGTH,700,90))
				{
					charge_info.first_run = 0;		//转圈找充电桩
					charge_info.c_midle = 0;
				}else
				{
					log_printf("turn and fw...\r\n");
					motor.c_left_hw = motor.c_right_hw = 0;
					motor_fw_charge();
				}
			}
			*/
			if(yindx < YINDX_MAX)
			{
				log_printf("start go..,turn to=%d\r\n",yaw_queu[yindx]);
#if BACK_STAR_ORG_POINT
				if(motor.c_left_hw>900)
					dist_queu[yindx] = motor.c_left_hw-900;
				else
					dist_queu[yindx] = 0;
				log_printf("%d_dist:%d\r\n",yindx,dist_queu[yindx]);
#endif
				charge_back_off(800);
				charge_info.first_run = 0;		//转圈找充电桩
//				charge_info.c_lost = 0;
				charge_info.c_midle = 0;
			}

			else			//沿边走
			{
				sys->sState = SYS_DOCK_NEAR;		//目的是给延边的时候开红外

				sys->dock_near = NEAR_DOCK_STA; //延边状态，否则，在检测传感器的时候，会因为低电压而不停的在进入充电状态

				motor_go_edgeways(RIGHT_SIDE_NEAR,0,0,GO_NEAR_TYPE_DOCK,0);
				if(sys->sState == SYS_IDLE)//2018-05-26 jzz
				{
					return;
				}
				sys->dock_near = 0;
				sys->sState = SYS_DOCK;
				charge_info.first_run = 0;		//转圈找充电桩
//					charge_info.c_lost = 0;
				charge_info.c_midle = 0;
				motor_fw_charge();
				log_printf("[do_charging_work]edgeways out\r\n");
			}
		}else	//发现中线的碰
		{
			charge_back_off(800);//100->300
			robot_turn_deg(GO_LEFT,700,170);
			//www_idleintel_com();
			navigat->angle = sys->angle;
			motor_go_forwark(3000,NO_SIDE_NEAR,NULL);
			if(sys->sState == SYS_IDLE)//2018-05-26 jzz
				return;
			charge_info.first_run = 0;
			log_printf("first run again...\r\n");
			motor_fw_charge();
		}
	}
	CHECK_IDLE_STA();


	if(TIM5->CNT >=11000)      //11000
	{
		TIM5->CNT = 0;


		if(disfloat(charge_info.out,sys->angle) >=15)
		{
			log_printf("angle error(%3.1f,%3.1f)\r\n",charge_info.out,sys->angle);
			charge_info.out=sys->angle;
		}
		//www_idleintel_com();
		dock_speed_pid_ctrl(40,110,1);						//速度环
		p_gyro_calce(charge_info.out,sys->angle);

		L_FORWRK_PWM =  sys->lpwm;
		R_FORWRK_PWM =  sys->rpwm;
		c_charge++ ;
		if(c_charge >=8)// 8->16
		{
			c_charge = 0;
			//回充线程，每隔80ms处理一次
			do_charging_work(&charge_info);	 		//---------------------------------回充最重要的部分
			//2018-09-26 jzz
			if(sys->sState == SYS_IDLE)
			{
				sys->sState_auxi=0;
				return;
			}

		}
		if(c_calus ++ >50)
		{
			c_calus = 0;
			coordinate_calcu(0); 													//计算出原始的坐标系
			coord_org2map(navigat->x_org,navigat->y_org,&navigat->tx,&navigat->ty); //坐标系转换
			log_printf("*(%d,%d,%3.1f,0)-[%3.1f,%3.1f,0]*\r\n",navigat->tx,navigat->ty,sys->angle,navigat->x_org_f,navigat->y_org_f);

		}
		//log_printf("%d,%d\r\n",L_FORWRK_PWM,R_FORWRK_PWM);
	}

}
//static t_cpu_time wall_timer;



/*
回充流程

*/

unsigned char do_charging_work(ChargeInfo *Info)
{
	int i;

	CHECK_IDLE_STA_RT(0);
	read_ir_data(IrData_bak,0);

	log_printf("(%d,%d,%d,%d),%3.1f,%d\r\n",IrData_bak[IR_L_PIN_NUM], IrData_bak[IR_ML_PIN_NUM],IrData_bak[IR_MR_PIN_NUM], IrData_bak[IR_R_PIN_NUM],sys->angle,DOCK_DETECT());

	if(  DOCK_DETECT())		//充电成功,通过IO口电平检测充电是否正常。
	{
		log_printf("detect dock ok\r\n");
			for(i=0;i<10;i++)
			{
				if(DOCK_DETECT() ==0)
					break;
				delay(5);
			}
			if(i>=5)
			{
				memset((uint8_t *)Info,0,sizeof(ChargeInfo));
			}
	}
	//走的距离够远，原地转圈，没找到信号，随机走的时候，才会走到这里
	if(charge_info.walk_dist  < 5000)
		charge_info.walk_dist = 5000;
	if(  motor.c_left_hw >=charge_info.walk_dist )
	{
		Info->first_run = 0;			//原地转圈
		log_printf("dock_fw first run again,hw=%d,%d,max=%d\r\n",motor.c_left_hw,motor.c_right_hw,charge_info.walk_dist);
	}

	////2018-09-26 jzz
	if(sys->sState !=SYS_DOCK)
	{
		log_printf("[charging_turn_round]!!SYS_DOCK\r\n");
		motor_run(GO_STOP,0,0,0);
		STOP_ALL_MOTOR();
		gyro_whlmap();
//		sys->sState_auxi=0;
		return 0;
	}





	//有信号
	if(IrData_bak[IR_L_PIN_NUM] || IrData_bak[IR_ML_PIN_NUM]|| IrData_bak[IR_MR_PIN_NUM]||IrData_bak[IR_R_PIN_NUM])
	{
		Info->walk_dist =motor.c_left_hw+5000;
		Info->disc_signal =1;
//		Info->c_lost = 0;
//		c_signal_lost = 0;
		//还没发现充电桩，则原地转圈，找充电桩
		if(charge_info.dock_side == DOCK_NO)		//没有发现过信号，则选择一圈
		{
			charge_info.first_run = 0;
			log_printf("[do_charging_work]found dock ,first run...\r\n");
		}

	}

	//============================第一次运行，转圈找信号，找到垂直方向,走===============================
	if(Info->first_run == 0)
	{

		Info->c_midle= 0;		//?????òμ??D??μ?′?êy
		Info->t_turn= 0;
		PRINTF("first run\r\n");
		charging_turn_round();	 //第一次走  做一个转圈
		Info->first_run = 1;		 //转一圈后 第一次走
		Info->side_keep_max = 0;
		}

	//=====================发现信号调整流程，根据充电桩和信号内容细调这里==============================

	if((0 < Info->c_midle) || ((0 != (IrData_bak[IR_MR_PIN_NUM] & IR_MID)) || (0 != (IrData_bak[IR_ML_PIN_NUM] & IR_MID))))
		{
		//这里是进入中线的流程
		//正中线//(6 6) || (2 4)
		if(((IR_MID == (IrData_bak[IR_MR_PIN_NUM] & IR_MID)) && (IR_MID == (IrData_bak[IR_ML_PIN_NUM] & IR_MID)))
		|| ((IR_M_RIGHT == (IrData_bak[IR_MR_PIN_NUM] & IR_MID)) && (IR_M_LEFT == (IrData_bak[IR_ML_PIN_NUM] & IR_MID))))
		{
			Info->c_midle++;
			log_printf("MM2,c=%d\r\n",Info->c_midle);
		}else
		//micony  右侧收到中线，证明斜的
		if((IrData_bak[IR_R_PIN_NUM]  & IR_MID) && Info->c_midle < 4)
		{
			log_printf("RR5,c=%d\r\n",Info->c_midle);
			set_walk_angle(GO_RIGTH, TURN_ANGLE * 5.0);
			motor_fw_charge();
		}else if((IrData_bak[IR_L_PIN_NUM]  & IR_MID) && Info->c_midle < 4)
		{
			log_printf("LL5,c=%d\r\n",Info->c_midle);
			set_walk_angle(GO_LEFT, TURN_ANGLE * 5.0);
			motor_fw_charge();
		}
		//中左中右都有中线 (6 2) || (6 4) || (2 6) || (4 6)
		else if((0 != (IrData_bak[IR_MR_PIN_NUM] & IR_MID)) && (0 != (IrData_bak[IR_ML_PIN_NUM] & IR_MID)))
		{
			//(2 x)
			if(0 != (IrData_bak[IR_MR_PIN_NUM] & IR_M_LEFT))		/*中线偏左*/
			{
				log_printf("MR1\r\n");
				set_walk_angle(GO_RIGTH, TURN_ANGLE * 1.0);     //0.5
				motor_fw_charge();
			}
			//(x 4)
			else if(0 != (IrData_bak[IR_ML_PIN_NUM] & IR_M_RIGHT))		/*中线偏右*/
			{
				log_printf("ML1\r\n");
				set_walk_angle(GO_LEFT, TURN_ANGLE * 1.5);         //1.0
				motor_fw_charge();
			}
		}

		//中右没有中线
		else if(0 == (IrData_bak[IR_MR_PIN_NUM] & IR_MID))
		{
			//中左有右线
			if(0 != (IrData_bak[IR_ML_PIN_NUM] & IR_M_RIGHT))
			{
				log_printf("ML2\r\n");
				set_walk_angle(GO_LEFT, TURN_ANGLE * 3.0);
				motor_fw_charge();
			}
			//中左有左线
			else if(0 != (IrData_bak[IR_ML_PIN_NUM] & IR_M_LEFT))
			{
				log_printf("ML3\r\n");
				set_walk_angle(GO_LEFT, TURN_ANGLE * 2.0);
				motor_fw_charge();
			}
		}
		//中左没有中线
		else if(0 == (IrData_bak[IR_ML_PIN_NUM] & IR_MID))
		{
			//中右有右线

			if(0 != (IrData_bak[IR_MR_PIN_NUM] & IR_M_RIGHT))
			{
				log_printf("MR4\r\n");
				set_walk_angle(GO_RIGTH, TURN_ANGLE * 2.0);
				motor_fw_charge();
			}
			//中右有左线
			else if(0 != (IrData_bak[IR_MR_PIN_NUM] & IR_M_LEFT))
			{
				log_printf("MR5\r\n");
				set_walk_angle(GO_RIGTH, TURN_ANGLE * 3.0);    //   3.5
				motor_fw_charge();
			}
		}
		charge_info.c_side = 0;
	}

	else				//这里还没有进入中线的流程

	{

		//在左侧的时候，侧面发现中线，则大拐过去
		if(((LEFT_SIDE == Info->dock_side) || (0 == IrData_bak[IR_R_PIN_NUM]))
			&& (IrData_bak[IR_L_PIN_NUM] & IR_MID))
		{
			log_printf("LL1\r\n");
			if(IrData_bak[IR_L_PIN_NUM] & IR_M_RIGHT)
				set_walk_angle(GO_LEFT, TURN_ANGLE * .60);
			else
				set_walk_angle(GO_LEFT, TURN_ANGLE * 5.0);
			motor_fw_charge();
			charge_info.c_side = 0;
		}else //左侧切过去，
		if(((LEFT_SIDE == Info->dock_side) || (0 == IrData_bak[IR_R_PIN_NUM]))
			&& (IrData_bak[IR_L_PIN_NUM] & IR_LEFT))
		{
			log_printf(" LL2\r\n");
			if( ++(Info->side_keep_max) > 100)  //切得太多了，重新找
			{
				Info->side_keep_max = 0;
				Info->first_run = 0;
				log_printf("[do_charging_work]side l first_run\r\n");
			}
			else
				motor_fw_charge();
			charge_info.c_side = 0;
				////motor_fw_charge();
		}else /*右侧切进来，垂直切中线的情况，需要大拐*/
		if(((RIGHT_SIDE == Info->dock_side) || (0 == IrData_bak[IR_L_PIN_NUM]))
			&& (IrData_bak[IR_R_PIN_NUM] & IR_MID))
		 {
			log_printf("RR1\r\n");
			if(IrData_bak[IR_R_PIN_NUM] & IR_M_LEFT)
				set_walk_angle(GO_RIGTH, TURN_ANGLE * 6.0);
			else
				set_walk_angle(GO_RIGTH, TURN_ANGLE * 5.0);
			motor_fw_charge();
			charge_info.c_side = 0;
		 }else //右侧切过去
		if(((RIGHT_SIDE == Info->dock_side) || (0 == IrData_bak[IR_L_PIN_NUM]))
			&& (IrData_bak[IR_R_PIN_NUM] & IR_RIGHT))
		 {
			log_printf("RR2\r\n");
			if(100 < ++(Info->side_keep_max))   //切得太多了，重新找
			{
				Info->side_keep_max = 0;
				Info->first_run = 0;
				log_printf("[do_charging_work]side r first_run\r\n");
			}
			else
				motor_fw_charge();
			Info->c_side = 0;
		 } else if( IrData_bak[IR_R_PIN_NUM]==0 && IrData_bak[IR_L_PIN_NUM]==0 &&
		  			(IrData_bak[IR_ML_PIN_NUM] == IR_RIGHT || IrData_bak[IR_ML_PIN_NUM] == IR_LEFT ||
		  			 IrData_bak[IR_MR_PIN_NUM] == IR_RIGHT || IrData_bak[IR_MR_PIN_NUM] == IR_LEFT))
		  {
			Info->c_side++;
			log_printf("side go...c=%d\r\n",Info->c_side);
			if(Info->c_side > 8)
			{
				Info->first_run = 0;
		 }
		  }else
		  	Info->c_side=0;

	}

	return 0;
}


/*
转圈圈，找座子，发现座子后，找垂直方向切过去。
*/

void charging_turn_round(void)
{
	float min_dist_angle = 0.00;
	uint8_t turn_turn_side = GO_LEFT;

	int c=0;
	int count=0;
	uint8_t old_state;
	uint16_t c_mide=0,c_left,c_right;		//中间发现左中右的计数
	uint8_t dock_side = 0;

	//侧面的接收头发现充电桩时机器的角度，当车头没有收到信号的时候，这个角度就有用了。
	float lyaw = 1000,myaw= 1000,ryaw = 1000;

	//用于计算是在充电桩的左侧，右侧还是中间
	uint16_t cleft=0,cright=0,cmidle=0;
	uint32_t hw_timeout = 3000;		//转圈超时的霍尔数，2000左右一圈
	if(RIGHT_SIDE == charge_info.dock_side)
		turn_turn_side= GO_RIGTH;

	charge_info.dock_side = NO_SIDE;		//默认没有找到充电桩的方向

	CHECK_IDLE_STA();

	motor_run(GO_STOP,0,0,0);

	charge_info.c_turn++;			//计数

	log_printf("[charging_turn_round]c=%d,timeout=%d\r\n",charge_info.c_turn,hw_timeout);

#if !BACK_STAR_ORG_POINT
		charge_info.walk_dist = WALK_DIST_MAX;			//转圈过去后，没发现充电桩，最多能走5000霍尔
#endif

	old_state = sys->sState;
	www_idleintel_com();
	coordinate_calcu(0);



	get_gyro_org();
	motor.c_left_hw = motor.c_right_hw = 0;                     //左右轮的霍尔值取0
	sys->pwm = DOCK_TURN_PWM;
	sys->left_pwm = sys->right_pwm = sys->pwm;
	motor_run(turn_turn_side/*GO_LEFT*/,DOCK_TURN_PWM,0,0);                       //向左转弯

		turn_round_pid(0);


	if(old_state != sys->sState)
	{
		motor_run(GO_STOP,0,0,0);
		log_printf("[charging_turn_round]old_state\r\n",c);
		return ;
	}
	DUST_MOTOR_OFF();		//

	count = 0;
	c_mide = c_left = c_right = 0;
	TIM5->CNT = 0;                                          //一系列初始化
	//============================第一步，转圈圈，发现座子的时候退出=====================================
	while(1)
	{
		get_sensers(&sys->gSta);                           //所有的传感器相关 红外
		proc_uart_task();                                  //所有的通讯相关   显控 WiFi 遥控器 串口工具 按键  转圈需要

		/*碰撞要退出*/
		if(sys->gSta & (MASK_BUM_LEFT | MASK_BUM_RIGHT))
		{
			motor_run(GO_STOP,0,0,0);
			log_printf("[charging_turn_round]round bum break out\r\n");
			gyro_whlmap();
			charge_info.first_run = 0;
			return ;
		}



		//=================判断退出机制=====================================
		if(sys->sState !=SYS_DOCK)                       //系统状态 回充
		{
			log_printf("[charging_turn_round]!!SYS_DOCK_1\r\n");
			motor_run(GO_STOP,0,0,0);
			STOP_ALL_MOTOR();
			gyro_whlmap();
			return ;
		}

		if(sys->mState == GO_STOP)         //sstate  状态未改变 要恢复电机动作
		{
			motor_run(turn_turn_side/*GO_LEFT*/,DOCK_TURN_PWM,0,0);
		}
		//==========================超时退出========================
		if( (count++ > 5000) || ( motor.c_right_hw >=hw_timeout) || (motor.c_left_hw >=hw_timeout) ||  (yindx!=0 && yindx<YINDX_MAX)  /*(((yindx+2)%2 == 0) && (yindx!=0 && yindx < YINDX_MAX))*/)			//一圈就够了
		{
			count = 0;
			log_printf("turn timeout,left=%d,right=%d,sta=%d,yaw=%3.1f,%3.1f,%3.1f,yindx:%d\r\n",motor.c_left_hw,motor.c_right_hw,sys->sState,lyaw , myaw ,ryaw,yindx);

			motor_run(GO_STOP,0,0,0);
			//超时退出，如果侧面发现了信号，则转过指定角度，切过去
			if(lyaw < 360 || myaw < 360 || ryaw < 360)
			{
				charge_info.disc_signal = 1;
				if(cleft > cright && cleft >= cmidle)
					dock_side = LEFT_SIDE;              //LEFT_SIDE
				else if(cright > cleft && cright >= cmidle )
					dock_side = RIGHT_SIDE;
				else
					dock_side = NO_SIDE;
				charge_info.dock_side = dock_side;
				log_printf("turn lidar found dock side=%d\r\n",dock_side);
				motor_turn_lidar( turn_turn_side/*GO_LEFT*/, 700,lyaw,myaw,ryaw);
			}
			else
			{
				if(yindx < YINDX_MAX)
				{
					log_printf("start go..,idx = %d,turn to=%d\r\n",yindx,yaw_queu[yindx]);
					turn_to_deg(yaw_queu[yindx]);
#if BACK_STAR_ORG_POINT
					charge_info.walk_dist = dist_queu[yindx];
#endif
					yindx++;
					log_printf("updat yindx:%d\r\n",yindx);
				}
				else if(charge_info.c_turn >1)		//这里都是随机碰撞过来的，要转过指定角度，执行随机走动
				{
					motor_turn(GO_RIGTH,650,90);
				}
			}
			goto l_turn_charge_end;
		}

		charge_turn_delay_sensor(80);			//延迟80ms，并做转圈速度环

		read_ir_data(IrData_bak,0);				//读取充电桩数据


		//左 中 右侧计数，用于计算是在那一侧
		//左侧
		if( IrData_bak[IR_L_PIN_NUM] 	& IR_LEFT_SIDE ||
			IrData_bak[IR_ML_PIN_NUM] 	& IR_LEFT_SIDE ||
			IrData_bak[IR_MR_PIN_NUM] 	& IR_LEFT_SIDE ||
			IrData_bak[IR_R_PIN_NUM] 	& IR_LEFT_SIDE  )
			cleft++;
		//右侧
		if( IrData_bak[IR_L_PIN_NUM] 	& IR_RIGHT_SIDE ||
			IrData_bak[IR_ML_PIN_NUM] 	& IR_RIGHT_SIDE ||
			IrData_bak[IR_MR_PIN_NUM] 	& IR_RIGHT_SIDE ||
			IrData_bak[IR_R_PIN_NUM] 	& IR_RIGHT_SIDE  )
			cright++;
		//中间
		if(		((IrData_bak[IR_L_PIN_NUM] & (IR_RIGHT|IR_LEFT) ==0 ) && IrData_bak[IR_L_PIN_NUM] & IR_MID) ||
				((IrData_bak[IR_ML_PIN_NUM] & (IR_RIGHT|IR_LEFT) ==0 ) && IrData_bak[IR_ML_PIN_NUM] & IR_MID) ||
				((IrData_bak[IR_MR_PIN_NUM] & (IR_RIGHT|IR_LEFT) ==0 ) && IrData_bak[IR_MR_PIN_NUM] & IR_MID) ||
				((IrData_bak[IR_R_PIN_NUM] & (IR_RIGHT|IR_LEFT) ==0 ) && IrData_bak[IR_R_PIN_NUM] & IR_MID) )
			cmidle++;

		log_printf("[ctr](%d,%d,%d,%d)(%d,%d,%d)%d\r\n"
		,IrData_bak[IR_L_PIN_NUM],IrData_bak[IR_ML_PIN_NUM],IrData_bak[IR_MR_PIN_NUM],IrData_bak[IR_R_PIN_NUM],
			cleft,cmidle,cright,motor.c_left_hw);
		//=================连续发现中线有三个一样的数据则退出=============================
		www_idleintel_com();
		//中间发现信号，记录角度
		if((IrData_bak[IR_ML_PIN_NUM] !=0) || (IrData_bak[IR_MR_PIN_NUM] !=0))
		{
			myaw = sys->angle;
			log_printf("myaw =%3.3f\r\n",myaw );
		}
		else if(IrData_bak[IR_L_PIN_NUM]!=0)	//侧面发现信号，记录角度
		{
            lyaw = sys->angle;
			log_printf("lyaw =%3.3f\r\n",lyaw );
		}
		else if(IrData_bak[IR_R_PIN_NUM]!=0)
		{
            ryaw = sys->angle;
			log_printf("ryaw =%3.3f\r\n",ryaw );
		}
		//通过中间的信号，来计算充电桩的方位
		if((IrData_bak[IR_ML_PIN_NUM] & IR_LEFT) ||  (IrData_bak[IR_MR_PIN_NUM] & IR_LEFT))
		{
			c_left++;
		}else
			c_left = 0;

		if((IrData_bak[IR_ML_PIN_NUM] & IR_MID) || (IrData_bak[IR_MR_PIN_NUM] & IR_MID)
			/*&& (0 == IrData_bak[IR_L_PIN_NUM]) && (0 == IrData_bak[IR_R_PIN_NUM])*/)
		{
			c_mide++;
		}else
			c_mide = 0;

		if((IrData_bak[IR_ML_PIN_NUM] & IR_RIGHT) ||  (IrData_bak[IR_MR_PIN_NUM] & IR_RIGHT))
		{
			c_right++;
		}else
			c_right = 0;

		//中间的计数够了，证明找到充电桩了，则退出，此时机器应该是指向充电桩的位置

		if(c_mide >=3 || c_left>=3 || c_right >=3)
		{
			log_printf("found dock count=%d,%d,%d,break out\r\n",c_left,c_mide,c_right);
			charge_info.disc_signal = 1;
			break;
		}
	 }
	//出来的时候，中间计数不够的话，用侧面的角度来计算
	if(c_mide< 2 && c_left < 2  && c_right < 2)
	{
		motor_turn_lidar( turn_turn_side/*GO_LEFT*/, 700,lyaw,min_dist_angle,ryaw);
		if(cleft > cright && cleft >= cmidle)
			dock_side = LEFT_SIDE;              //LEFT_SIDE
		else if(cright > cleft && cright >= cmidle )
			dock_side = RIGHT_SIDE;
		else
			dock_side = NO_SIDE;
		charge_info.disc_signal = 1;
		charge_info.dock_side = dock_side;
		log_printf("turn lidar found dock side=%d\r\n",dock_side);
		motor.c_left_hw = 0;
		//charge_info.dock_fw = SET;
		goto l_turn_charge_end;
	}

	motor_run(GO_STOP,0,0,0);                       //停止
	delay_ms(100);
	log_printf("step1,maybe found dock..\r\n");     //打印找到充电座

	//read_ir_data(IrData_bak,0);		//uart4自动更新

	//===========================================第二步，分析出在左侧还是右侧，垂直切进去=============================
	dock_side = 0;

	//计算充电桩的方位
	if(cleft > cright && cleft >= cmidle)
		dock_side = LEFT_SIDE;              //LEFT_SIDE
	else if(cright > cleft && cright >= cmidle )
		dock_side = RIGHT_SIDE;
	else if(cmidle > 0 || cleft > 0 || cright > 0)
		dock_side = MIDLE_SIDE;
	else
		dock_side = NO_SIDE;
	charge_info.dock_side = dock_side;	//记录方向

	log_printf("dockir(%d,%d,%d,%d)(%d,%d,%d)dock_side=%d\r\n",IrData_bak[IR_L_PIN_NUM],IrData_bak[IR_ML_PIN_NUM],
					IrData_bak[IR_MR_PIN_NUM],IrData_bak[IR_R_PIN_NUM],cleft,cmidle,cright,dock_side);
	//再充电桩的右侧，则转到没信号的方位，切过去
	if(dock_side == RIGHT_SIDE)
	{
		uint16_t n_timeout = 120;
		c_mide = 0;
		log_printf("right side...\r\n");
		motor.c_left_hw = motor.c_right_hw = 0;
		motor_run(GO_LEFT,DOCK_TURN_PWM,0,0);  //判断出机器在充电座的右边 那么机器往左转  GO_LEFT
		turn_round_pid(0);
		while(n_timeout --)
		{
			charge_turn_delay_sensor(80);		//2018-10-25 jzz
			if(sys->sState !=SYS_DOCK)                       //系统状态 回充
		{
			log_printf("[charging_turn_round]!!SYS_DOCK_1\r\n");
			motor_run(GO_STOP,0,0,0);
			STOP_ALL_MOTOR();
			gyro_mapwhl();
			return ;
		}
			read_ir_data(IrData_bak,0);
			log_printf("RIGHT %d,%d,%d,%d : %d\r\n",IrData_bak[IR_L_PIN_NUM],IrData_bak[IR_ML_PIN_NUM],
					IrData_bak[IR_MR_PIN_NUM],IrData_bak[IR_R_PIN_NUM],cfg->r_c_turn);
			//一直转到中间没有信号，这个就是横切的方位。
			if((IrData_bak[IR_ML_PIN_NUM] ==0 ) && (IrData_bak[IR_MR_PIN_NUM] ==0))
			{
				c_mide ++;
				if(c_mide  >=cfg->r_c_turn)//5)// 2->5->2->12		////----------------接收的红外中线个数
				{
					motor_run(GO_STOP,0,0,0);//---------
					log_printf("turn ok,to V side right dis=%d,%d\r\n",motor.c_left_hw,motor.c_right_hw);
					goto l_turn_charge_end;
				}
			}else
				c_mide = 0;
		}
	}
	else  if(dock_side == LEFT_SIDE)  //在左侧
	{
		uint16_t n_timeout = 120;
		motor.c_left_hw = motor.c_right_hw = 0;
		motor_run(GO_RIGTH,DOCK_TURN_PWM,0,0);		//右转过去，知道中间信号消失
		c_mide = 0;
		log_printf("left side\r\n");
		turn_round_pid(0);			///2018-10-26
		while(n_timeout --)
		{
			charge_turn_delay_sensor(80);		//2018-10-25 jzz
			if(sys->sState !=SYS_DOCK)						 //系统状态 回充
					{
						log_printf("[charging_turn_round]!!SYS_DOCK_1\r\n");
						motor_run(GO_STOP,0,0,0);
						STOP_ALL_MOTOR();
						gyro_whlmap();
						return ;
					}

			read_ir_data(IrData_bak,0);
			log_printf("LEFT:%d,%d,%d,%d : %d\r\n",IrData_bak[IR_L_PIN_NUM],IrData_bak[IR_ML_PIN_NUM],
					IrData_bak[IR_MR_PIN_NUM],IrData_bak[IR_R_PIN_NUM],cfg->l_c_turn);


			if((IrData_bak[IR_ML_PIN_NUM] == 0 ) && ( IrData_bak[IR_MR_PIN_NUM] == 0))
			{
				c_mide ++;

				if(c_mide  >=cfg->l_c_turn)//if(c_mide  >=5)// 2->5->12

				{
					motor_run(GO_STOP,0,0,0);//---------
					log_printf("turn ok,to V side left dis=%d,%d\r\n",motor.c_left_hw,motor.c_right_hw);
					log_printf("step2\r\n");

					goto l_turn_charge_end;
				}
			}else
					c_mide = 0 ;
		}
	}else if((IR_MID == IrData_bak[IR_ML_PIN_NUM]) && (IR_MID == IrData_bak[IR_MR_PIN_NUM]))
	{

		log_printf("[charging_turn_round]found midle\r\n");
//		charge_info.dock_sta = DOCK_STA_MID;
		goto l_turn_charge_end;

	}else
	{
		log_printf("[charging_turn_round]not found dock\r\n");
		if(lyaw < 360 || myaw < 360 || ryaw < 360)
		{
			if(motor_turn_lidar( GO_LEFT, 700,lyaw,myaw,ryaw) ==1)
	      		goto l_turn_charge_end;
	      }
		goto l_turn_charge_end;
	}

l_turn_charge_end:
	//send_imu_data(0,0,0,sys->spl_data,INFO_TURN,sys->clean_sta);

	//sys->forced_open_laser_irda = RESET;
	set_ir_alter_dis(0);
	delay_ms_sensers(100);
	coordinate_calcu(0);
	gyro_whlmap();
	log_printf("[charging_turn_round]end ,go fw\r\n");
	motor_fw_charge();
}

void motor_fw_charge(void)
{
	CHECK_IDLE_STA();
	if(sys->mState != GO_FORWARD )
	{
		int pwm;
			dock_speed_pid_ctrl(0,0,0);				//2018-09-17  jzz
		/*
		if(charge_info.c_midle >=20)
			sys->pwm = DOCK_PWM_MDL;
			//pwm = DOCK_PWM_MDL;
		else
		*/
			sys->pwm = DOCK_PWM_GO;
		pwm = sys->pwm;
		pd_gyro_int(pwm);
		delay_ms(5);
		www_idleintel_com();


		charge_info.out = sys->angle;
		//if(charge_info.c_mm )
		//	motor_run(GO_FORWARD,DOCK_PWM,0,0);
		//else
		log_printf("motor_fw_charge go,set angle=%3.1f,pwm=%d\r\n",charge_info.out,sys->pwm );
		get_gyro_org();
		motor.c_left_hw =motor.c_right_hw = 0;
		motor_run(GO_FORWARD,sys->pwm ,0,0);
	}

}



void charge_back_off(int hw)
{
	//float dt;
//	int c;

	int i,tmr;


	hw = 0 - hw ;


	TIM5->CNT = 0;
	motor.c_left_hw = 0;
	log_printf("back of,dist=%d\r\n",hw);
	motor.c_left_hw = motor.c_right_hw = 0;
	motor_run(GO_BACK, BACK_OFF_PWM, 0, 0);
	//navigat->wheel_dir =-1;
	//navigat->is_walk = 1;
	//tmr = hw > 50 ?2000:2000;
	tmr = 2000;
	i=0;
	//CHECK_NAVI_STA();

	while(1)
	{
		get_sensers(&sys->gSta);
		proc_uart_task();
		//=================判断退出机制=====================================
		if(sys->sState !=SYS_DOCK)
		{
			log_printf("[charge_back_off]!!SYS_DOCK\r\n");
			motor_run(GO_STOP,0,0,0);
			STOP_ALL_MOTOR();
			gyro_whlmap();
#if 1//SYS_VER == VER_KESMAN
			set_ir_alter_dis(0);
#endif
			return ;
		}

		if(sys->mState == GO_STOP)	//sState状态未改变，而电机停止了，要恢复电机的动作
		{
			motor_run(GO_BACK,BACK_OFF_PWM,0,0);
		}

		if(TIM5->CNT >=5000)
		{
			www_idleintel_com();
			//log_printf("k");
			if(motor.c_left_hw < hw || i++ >= tmr)
			{
				motor_run(GO_STOP,0,0,0);
				log_printf("bof ok,c_left_hw=%d,i=%d,tmr:%d\r\n",motor.c_left_hw ,i,tmr);
				break;
			}

		}
	}

	//wait_for_gyro_stop();				//等待陀螺仪稳定
//	motor_run(GO_STOP,0,0,0);
	delay_ms(200);
	//navigat->distance =0 - navigat->distance ;

	//这里会有些误差，要减去一点误差，这个干吗要这样。。。又去掉了 2016-04-22
	//navigat->distance = distance - navigat->distance - 3;
	log_printf("back dis=%d,agle=%f",motor.c_left_hw,sys->angle);

	//cord_calc_store(1);
	//log_printf("*(%d,%d,%f,0)-[%d,%d,%3.1f]\r\n\r\n",navigat->tx,navigat->ty,navigat->angle,navigat->x_org,navigat->y_org,sys->angle);
	//log_printf("hw=%d,x:%d,y%d,angle:%f,tx:%d,ty:%d\r\n",hw,navigat->x_org,navigat->y_org,navigat->angle,navigat->tx,navigat->ty);
	save_line_xy(navigat->x1_org,navigat->y1_org);
}

uint8_t motor_turn_lidar(uint8_t dir,int pwm,float lyaw,float dyaw,float ryaw)
{
	if(lyaw < 360)
	{
		log_printf("found lyaw=%3.2f,",lyaw);
		lyaw+=70;//100;
		if(lyaw > 360)
			lyaw -=360;
		log_printf("turn to %3.2f,",lyaw);
		turn_to_deg(lyaw);
	//	motor_go_forwark(5000,NO_SIDE_NEAR,NULL);
		sys->sState =SYS_DOCK;
		return 1;
	}
	if(dyaw < 360)
	{
		log_printf("found myaw=%3.2f,",dyaw);
		turn_to_deg(dyaw);
		//motor_go_forwark(5000,NO_SIDE_NEAR,NULL);
		sys->sState =SYS_DOCK;
		return 1;
	}
	if(ryaw < 360)
	{
		log_printf("found ryaw=%3.2f,",ryaw);
		ryaw -= 70;//100;
		if(ryaw < 0)
			ryaw +=360;
		log_printf("turn to %3.2f,",ryaw);
		turn_to_deg(ryaw);
		//motor_go_forwark(5000,NO_SIDE_NEAR,NULL);
		sys->sState =SYS_DOCK;
		return 1;
	}
	return 1;

}

void set_walk_angle(uint8_t dir,float agle)
{

	//www_idleintel_com();
	charge_info.out = sys->angle;

	log_printf("[set_angle]%d,now=%5.1f,alg=%5.1f,",dir,sys->angle,agle);
	pd_gyro_int(sys->pwm);     //初始化

	//限幅
	if(charge_info.dock_side == RIGHT_SIDE && dir == GO_LEFT )
	{
		sys->min_pwm = -50;//-220;//-50;//-160;
		sys->max_pwm = 50;//220;//50;//160;

	}
	else if(charge_info.dock_side == LEFT_SIDE && dir == GO_RIGTH )
	{
		sys->min_pwm = -50;//-220;//-50;//-160;
		sys->max_pwm = 50;//220;//50;//160;

	}
	/*else if(charge_info.c_midle>2)
	{
		sys->min_pwm = -50;//-100;//-160;
		sys->max_pwm = 50;//100;

	}else if(charge_info.dock_side == LEFT_SIDE || charge_info.dock_side == RIGHT_SIDE )
	{
		sys->min_pwm = -50;//-80;//-160;
		sys->max_pwm = 50;//80;

	}*/else
	{
		sys->min_pwm = -200;//-100;//-160;
		sys->max_pwm = 200;//100;

	}



#if 0

	//限幅
	if(charge_info.dock_side == RIGHT_SIDE && dir == GO_LEFT )
	{
		sys->min_pwm = -50;//-160;
		sys->max_pwm = 50;//160;

	}
	else if(charge_info.dock_side == LEFT_SIDE && dir == GO_RIGTH )
	{
		sys->min_pwm = -50;//-160;
		sys->max_pwm = 50;//160;

	}
	/*else if(charge_info.dock_side == LEFT_SIDE || charge_info.dock_side == RIGHT_SIDE )
	{
		sys->min_pwm = -200;//-160;
		sys->max_pwm = 200;

	}*/else

	{
		sys->min_pwm = -100;//-160;
		sys->max_pwm = 100;

	}
#endif
	//if(dis_agle < 20 && dis_agle > -20)
//	{
		if(dir == GO_LEFT)
			charge_info.out +=agle;
		else
			charge_info.out -=agle;        //角度
		log_printf("set=%f ",charge_info.out);
		if(charge_info.out >=360)
		{
			charge_info.out -=360;
			log_printf("overset=%f",charge_info.out);
		}
		if(charge_info.out < -100)
		{
			charge_info.out +=360;
			log_printf("overset=%f",charge_info.out);
		}
	//}else
	//	log_printf("set agl error,%f-%f",charge_info.out,sys->angle);
	//sys->pwm = DOCK_PWM_MDL;        //找到中线
	log_printf("(%d,%d)\r\n",sys->min_pwm,sys->max_pwm);

}

/*
导航中，去一条线的地方矫正，再回来。

uint8_t navi_adj_to_line(navigat_t *navi)
{
	//这个方向，找Y轴的一条线。
	if(navi->angle == 90 || navi->angle == 270)
	{

	}

}
*/
uint8_t detect_dock(void)
{
	static uint16_t c_dock_data = 0;
	static uint16_t cnodock=0;
	//找充电桩
	read_ir_data_timeout(IrData_bak,0);

	if(IrData_bak[IR_L_PIN_NUM] ||	IrData_bak[IR_ML_PIN_NUM]||  IrData_bak[IR_MR_PIN_NUM] || IrData_bak[IR_R_PIN_NUM] || ir_dock_insight(1))
	{

			log_printf("[motor_go_edgeways]bum	dock(%d,%d,%d,%d)%d\r\n",
				IrData_bak[IR_L_PIN_NUM] ,IrData_bak[IR_ML_PIN_NUM],IrData_bak[IR_MR_PIN_NUM],IrData_bak[IR_R_PIN_NUM],c_dock_data);
			c_dock_data++;
			log_printf("docc=%d\r\n",c_dock_data);
			if(c_dock_data >=3)
			{
				log_printf("found dock..\r\n");
				return 1;
			}
			cnodock = 0;

	}else
	{
		if(cnodock++ >=3)
			c_dock_data = 0;
	}
	return 0;

}




uint8_t motor_turn_for_dock(uint8_t dir,int pwm,float agle)
{
//	int pwm=600;
	//int dg_turn=deg - 2.5;
//	int c;
	float dis_agle,b_agle;
	log_printf("motor_turn,dir=%s,agle=%f\r\n",get_motor_sta(dir),agle);
	motor.c_left_hw = motor.c_right_hw = 0;
	//www_idleintel_com();
	b_agle= format_agle(sys->angle, ANGLE_360);
	motor_run(dir,pwm,0,0);
	uint16_t c=0,c_dock=0;
	turn_check_bum_pro(0,dir,pwm,25);
	while(1)
	{
		get_sensers(&sys->gSta);			//
		proc_uart_task();
		turn_check_bum_pro(1,dir,pwm,TURN_SPEED);
		delay_ms(5);
	//	www_idleintel_com();
		if(sys->sState == SYS_IDLE)
			return 0;
		c++;
		if(c >=16)
		{
			c=0;

			read_ir_data(IrData_bak,0);

			log_printf("(%d,%d,%d,%d),%3.1f,c=%d\r\n",IrData_bak[IR_L_PIN_NUM], IrData_bak[IR_ML_PIN_NUM],IrData_bak[IR_MR_PIN_NUM], IrData_bak[IR_R_PIN_NUM],sys->angle,c_dock);
			if(IrData_bak[IR_L_PIN_NUM] || IrData_bak[IR_ML_PIN_NUM]||IrData_bak[IR_MR_PIN_NUM]  ||IrData_bak[IR_R_PIN_NUM])
			{
				c_dock++;
				if(c_dock >=3)
				{
					log_printf("found dock ,out2\r\n");
					motor_run(GO_STOP,0,0,0);
					return 1;
				}

			}
		}
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

	log_printf("..OK,c=%d,%d\r\n",motor.c_left_hw,motor.c_right_hw);
	motor.c_left_hw = motor.c_right_hw = 0;
	return 0;

}
