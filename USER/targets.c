
/*
特征物走法和矫正
*/

#include "sys.h"

#if TARGE_RUN

XY_T xy[MAX_TARGE_XY];
#endif
//检测是否需要矫正的走法

//特征物走法检测
int16_t run_ajust_check(void)
{
//	int idx;
//	TOBST *t_obst;

//	xy[0].x = 1;
	//沿边消失掉，就退出，消失5次，且走了一个格子，则退出
	if(navigat->adj_nsta == LEFT_SIDE_NEAR )		//左沿边
	{
		if(LEFT_ADC() < IRD_LEFT_ADJ_MIN)
		{
			//log_printf("adj lost l:%d\r\n",LEFT_ADC());
			if(navigat->adj_c_lost++ > LOST_NUBUER && WALK_DIST() >= HW_GRID)
			{
				motor_run(GO_STOP,0,0,0);
				if(LEFT_ADC() < SIDE_LOST_LEFT)
					navigat->adj_run_ret = ADJ_SIDE_LOST;	//特征物走法正确
				else
					navigat->adj_run_ret = ADJ_FAIL;
				log_printf("[run_ajust_check]lost12,ret=%d,ir=%d\r\n",navigat->adj_run_ret,LEFT_ADC());
				return 0;
			}
		}else
			navigat->adj_c_lost = 0;
	}
	if(navigat->adj_nsta == RIGHT_SIDE_NEAR )		//左沿边
	{
		if(RIGHT_ADC() < IRD_RIGHT_ADJ_MIN)
		{
		//	log_printf("adj lost r:%d\r\n",RIGHT_ADC());
			if(navigat->adj_c_lost++ > LOST_NUBUER && WALK_DIST()  >= HW_GRID)
			{
				motor_run(GO_STOP,0,0,0);
				if(RIGHT_ADC() < SIDE_LOST_RIGHT)
					navigat->adj_run_ret = ADJ_SIDE_LOST;	//特征物走法正确
				else
					navigat->adj_run_ret = ADJ_FAIL;
				log_printf("[run_ajust_check]lost13,ret=%d,ir=%d\r\n",navigat->adj_run_ret,RIGHT_ADC());
				return 0;
			}
		}else
		  navigat->adj_c_lost = 0;
	}


	return 1;
}
#if  AJUST_RUN

//沿墙走一段，找特征物
/**********************************************************************************
 *函数名称:robot_ajust_run
 			type - TYPE_ADJ_FIRST_RUN 工字型的时候，特征物走法，这个走法会判断，且短距离会不再回头寻找特征物，避免老是回头
 			       TYPE_ADJ_CHECK_AGAIN  导航回来再校验特征物
 *函数入参:gSta 本来想传碰撞状态进来，后来用sys->gsta代替了
 *		   x_dir X轴经过的方向，用来计算X轴从哪里走过来，判断这个方向有没有清扫过
 *函数功能:正面碰撞的时候，沿墙走一遍，直到碰撞，或是墙消失，记下来做特征物。
 *********************************************************************************/
char robot_ajust_run(uint8_t type,int x_dir,TOBST *g_obst)
{
//	int y_dir;
	int next_deg=0;
	int begin_deg=0;	//开始的角度
	//extern int begin_deg=0;
//	int i;
//	uint8_t sta;
	uint8_t n_sta_back;		//回来的时候，延边的方向
	short bx,toy;//,by;
	int dist1;
	TOBST tobst;
	///uint16_t n_adc=0;	//记录转过来的ADC
	uint16_t state;			///2018-07-16 jzz
	state = sys->sState;	//忘记初始化了

    log_printf("===robot_ajust_run start:%d\r\n",type);
	//第一次走特征物，才检测条件
	if(type == TYPE_ADJ_FIRST_RUN)
	{
	if(sys->t_adj_run ==0)
	{
#if EDGE_DRAW_MAP
		//micony20190305 只要有红外，就可以

		/*if( sys->g_sta[3] < 3000 )
		//if(sys->g_sta[2] < 1500  && sys->g_sta[3] < 3000 &&  sys->g_sta[4] <  1500)
		{
			log_printf("physical bum,don't draw map(%d,%d,%d)\r\n",sys->g_sta[2] ,sys->g_sta[3] ,  sys->g_sta[4] );
			return 0;
		}*/


		//如果已经走过了，也不能再走了。判断((X_NOW,Y_NOW)) (X_NOW+1,Y_NOW)(X_NOW-1,Y_NOW)(X_NOW+2,Y_NOW)(X_NOW-2,Y_NOW) 是障碍物，则退出
		/*if((point_sta(X_NOW,Y_NOW)) == OBST_STA ||(point_sta(X_NOW+1,Y_NOW)) == OBST_STA ||(point_sta(X_NOW-1,Y_NOW)) == OBST_STA ||(point_sta(X_NOW+2,Y_NOW)) == OBST_STA||(point_sta(X_NOW-2,Y_NOW)) == OBST_STA)
		//if(((X_NOW,Y_NOW)||(X_NOW+1,Y_NOW)||(X_NOW-1,Y_NOW) ||(X_NOW+2,Y_NOW)||(X_NOW-2,Y_NOW))==OBST_XY(navigat->tx, navigat->ty))
		//if((X_NOW,Y_NOW)==OBST_XY(X,Y)||(X_NOW+1,Y_NOW)==OBST_XY(X,Y)||(X_NOW-1,Y_NOW)==OBST_XY(X,Y) ||(X_NOW+2,Y_NOW)==OBST_XY(X,Y)||(X_NOW-2,Y_NOW)==OBST_XY(X,Y))
		{
		  log_printf(" The second edge\r\n");
		  return 0;
		}

	    if((begin_deg==navigat->angle)&&(point_sta(X_NOW,Y_NOW)) == OBST_STA )
	    //if((point_sta(X_NOW,Y_NOW)) == OBST_STA ||(point_sta(X_NOW+1,Y_NOW)) == OBST_STA ||(point_sta(X_NOW-1,Y_NOW)) == OBST_STA ||(point_sta(X_NOW+2,Y_NOW)) == OBST_STA||(point_sta(X_NOW-2,Y_NOW)) == OBST_STA)
	    {



         log_printf(" The second edge\r\n");
          return 0;
	    }
		*/
#else
		if(navigat->j_obst>=(MAX_TARGET-2))
		{
			log_printf("[adj_run]num over\r\n");
			return 0;
		}
		//正面的碰撞才走
		//if((sys->g_sta[2] < MIN_IRDA || sys->g_sta[3] < MIN_IRDA || sys->g_sta[4] < MIN_IRDA))
		if(!MIDLE_HAVE_IRDA2())
		{
			log_printf("[adj_run]not mid\r\n");
			return 0;
		}

		if(RIGHT_ADC() >= cfg->side_right_adc || LEFT_ADC() >= cfg->side_left_adc ||
			LEFT_ADC1() >=1800 || RIGHT_ADC1() >=1800)
		{
			log_printf("side adc,not adj\r\n");
			return 0;
		}

		if(navigat->from_charge)		//从充电桩出来
		{
			if(navigat->angle == 0 && dis_xy(X_NOW,navigat->dock_x) < 3 && dis_xy(Y_NOW,navigat->dock_y) < 20)
			{
				log_printf("near dock ,not adj\r\n");
				return 0;
			}
		}

		//已经测试过，就不测试了,不加上后边的条件，发现有时会重复两次
		if(get_adj_obst(navigat->tx, navigat->ty) ||
			get_adj_obst(navigat->tx+1, navigat->ty) || get_adj_obst(navigat->tx-1, navigat->ty) ||
			get_adj_obst(navigat->tx+2, navigat->ty) || get_adj_obst(navigat->tx-2, navigat->ty) ||
			get_adj_obst(navigat->tx+3, navigat->ty) || get_adj_obst(navigat->tx-3, navigat->ty) ||
			get_adj_obst(navigat->tx+4, navigat->ty) || get_adj_obst(navigat->tx-4, navigat->ty))
		{
			log_printf("have adj()\r\n");
			return 0;
		}
		}
#endif
		log_printf("[adj_run]type=%d,dir=%d,c=%d\r\n",type,x_dir,navigat->j_obst);
		//计算出沿边的方向和要转到的角度
		if(navigat->side == LEFT_SIDE)
		{
			turn_to_deg_v(90);
			//沿边的方向
			navigat->adj_nsta 	= navigat->lst_angle == 0?RIGHT_SIDE_NEAR:LEFT_SIDE_NEAR;
			n_sta_back			= navigat->lst_angle == 0?LEFT_SIDE_NEAR:RIGHT_SIDE_NEAR;
			next_deg = 270;		//保存掉头回去的方向，一会走特征物的方向完了之后，可能要掉头走回去回到原来的位置
			begin_deg = 90;
			toy = navigat->ty +1;		//回来后Y坐标
		}
		else
		{
			turn_to_deg_v(270);
			navigat->adj_nsta 	= navigat->lst_angle == 180?RIGHT_SIDE_NEAR:LEFT_SIDE_NEAR;
			n_sta_back			= navigat->lst_angle == 180?LEFT_SIDE_NEAR:RIGHT_SIDE_NEAR;
			next_deg = 90;
			begin_deg = 270;
			toy = navigat->ty -1;
		}
		if(state != sys->sState)			///2018-07-17 jzz
			return 0;
		tobst.x_dir = navigat->lst_angle == 0?1:-1;
	}
    else

	{
		if(g_obst == NULL)
			return 0;
		//计算出沿边的方向和要转到的角度
		//if(navigat->angle ==0 || navigat->angle == 180)
		//{
			if(g_obst->bret == ADJ_MID_BUM)	//如果小头是碰撞，则往大头走
			{
				//沿边的方向
				navigat->adj_nsta	= navigat->angle == 0?RIGHT_SIDE_NEAR:LEFT_SIDE_NEAR;
				n_sta_back			= navigat->angle == 0?LEFT_SIDE_NEAR:RIGHT_SIDE_NEAR;
				next_deg = 270; 	//保存掉头回去的方向，一会走特征物的方向完了之后，可能要掉头走回去回到原来的位置
				begin_deg = 90;
			}
			else
			{
				navigat->adj_nsta	= navigat->angle == 180?RIGHT_SIDE_NEAR:LEFT_SIDE_NEAR;
				n_sta_back			= navigat->angle == 180?LEFT_SIDE_NEAR:RIGHT_SIDE_NEAR;
				next_deg = 90;
				begin_deg = 270;
			}
			// navigat->begin_deg;
          // navigat->begin_deg =next_deg;
		/*
		//垂直方向，
		}else if(navigat->angle ==90 || navigat->angle == 270)
		{
			navigat->adj_nsta	= navigat->angle == 90?RIGHT_SIDE_NEAR:LEFT_SIDE_NEAR;
			n_sta_back			= navigat->angle == 90?LEFT_SIDE_NEAR:RIGHT_SIDE_NEAR;
			next_deg = 180; 	//保存掉头回去的方向，一会走特征物的方向完了之后，可能要掉头走回去回到原来的位置
			begin_deg = 0;


		}else
		{
			log_printf("angle errr=3.1f\r\n",navigat->angle);
			return 0;
		}
		*/
		log_printf("[adj_run]type=%d,begin=(%d,%d,)end=(%d,%d,),n_sta=%d,%d,deg=%d,%d\r\n",type,
					g_obst->bx,g_obst->by,g_obst->ex,g_obst->ey,navigat->adj_nsta,n_sta_back,begin_deg,next_deg);
		turn_to_deg_v(begin_deg);
		if(state != sys->sState)			///2018-07-17 jzz
			return 0;
		toy = navigat->ty ;  //回来的点
	}
	//navigat->adj_nsta = 0;
	if(state != sys->sState)		///2018-07-16 jzz
		return 0;
	if(navigat->adj_nsta ==0)
	{
		log_printf("[adj_run]nsta error\r\n");
		return 0;
	}
	delay_ms_sensers(300);

	//转过来后，判断一下侧面是否有红外，没有红外就退出了
	if(navigat->adj_nsta ==RIGHT_SIDE_NEAR)
	{
		if(RIGHT_ADC() < IRD_RIGHT_ADJ_MIN)
		{
			log_printf("[adj_run]r_adc error(%d)\r\n",RIGHT_ADC() );
			return 0;
		}
		///n_adc = RIGHT_ADC();
	}
	if(navigat->adj_nsta ==LEFT_SIDE_NEAR)
	{
		if(LEFT_ADC() < IRD_LEFT_ADJ_MIN)
		{
			log_printf("[adj_run]left adc error(%d)\r\n",LEFT_ADC() );
			return 0;
		}
		///n_adc = LEFT_ADC();
	}

#if ADJ_NAER_TURN_V
	//转到垂直的位置，未详细测试，先去掉，这样的话，如果不是垂直的撞向墙，延边矫正角度就失败了。
	near_turn_check(navigat->adj_nsta);
	if(state != sys->sState)		///2018-07-16 jzz
		return 0;
#endif
//	STORT_XY();					//存储当前的点，一会回来失败的时候，要延边回来就需要用到
	navigat->adj_walk_dis = navigat->adj_c_lost=0;
	//tobst = &navigat->adj_obst[navigat->j_obst++];
	log_printf("[adj_run]near go,c=%d\r\n",navigat->j_obst);

	//sta = navigat->sta;
//	navigat->sta = NAV_STA_ADJ;
	navigat->adj_run = TRUE;			//特征物的走法，
	//if(navigat->angle ==90 || navigat->angle == 270)
	navigat->adj_x_org = navigat->x_org_f;	//记录一下初始的X原始坐标，用于格式化X轴的原始坐标
	navigat->adj_run_ret = 0;

#if LASER_IR_ONLY_ONE
//只有中间一个红外,用来记录刚旋转过来时左边或右边的非测距红外红外值
	log_printf("n_adc:%d\r\n",n_adc);
	if(n_adc > 500 && n_adc < 4000)
	{
		uint16_t min,max,lock;
		min = n_adc - 1000;
		if(min>1000) min = 1000;
		else if(min <580) min=580;			////正式是500 测试700
		max = n_adc + 1500;
		if(max>3500) max = 3500;			//500 后改成 1500
		lock = n_adc -800;
		if(lock>1200)	lock = 1200;
		log_printf("reset side ir1:%d,%d,%d,%d,%d,[%d,%d,%d,%d]\r\n",cfg->min_left_adc,cfg->max_left_adc,cfg->lock_left_adc,cfg->lost_left_adc,cfg->side_left_adc,n_adc,min,max,lock);

		cfg->min_left_adc = cfg->min_right_adc = min;
		cfg->max_left_adc = cfg->max_right_adc = max;
		cfg->lock_left_adc = cfg->lock_right_adc = lock;
		cfg->lost_left_adc = min;
		cfg->lost_right_adc = min;
		cfg->side_left_adc = min;
		cfg->side_right_adc = min;
		log_printf("reset side ir2,min=%d,lock=%d,max=%d\r\n",min,lock,max);
	}
#endif

#if EDGE_DRAW_MAP		//micony20190305 沿边画地图

	go_edgeways_for_map(navigat->adj_nsta ,20,begin_deg);//沿边走10格

	log_printf("[adj_run]go back,now=(%d,%d,),to=(%d,%d,)\r\n",X_NOW,Y_NOW,bx,toy);
	navigat->c_go = navigat->c_turn = 0;
	//回来有碰撞
	navigat->adj_x_org = 0	;		//不再强制X轴了
	navigat->near.pid->c_lost_flag = 0;      //yanbian  2019 03 09 add
  	if(toy != Y_NOW)
	{
		float angle = sys->angle;
		/*
		if(toy > Y_NOW)
			turn_to_deg(90);					//在TOY负半轴转到90
		else
			turn_to_deg(270);					//在toy正半轴转到270			如果相等了呢，可以故意调试测试一下
		if(state != sys->sState)		///2018-07-16 jzz
			return 0;
		if(disfloat(sys->angle,angle) <=40)		//方向不变，延边就不变
		{
			log_printf("angle not chagne\r\n");
			n_sta_back = navigat->adj_nsta;
		}else
			n_sta_back = navigat->adj_nsta == LEFT_SIDE_NEAR?RIGHT_SIDE_NEAR:LEFT_SIDE_NEAR;
		*/
		if(navigat->adj_nsta == LEFT_SIDE_NEAR)
		{
			n_sta_back = RIGHT_SIDE_NEAR;
			log_printf("now near side=%d,turn dir=%d,back=%d\r\n",navigat->adj_nsta,GO_LEFT,RIGHT_SIDE_NEAR);

			robot_turn_deg(GO_LEFT,DEG_TURN_PWM,160);
		}else
		{
			n_sta_back = LEFT_SIDE_NEAR;
			log_printf("now near side=%d,turn dir=%d,back=%d\r\n",navigat->adj_nsta,GO_RIGTH,LEFT_SIDE_NEAR);
			robot_turn_deg(GO_RIGTH,DEG_TURN_PWM,160);


		}
		log_printf("motor_go_edgeways,nsta=%d\r\n",n_sta_back);
		//motor_go_edgeways(n_sta_back /*| 0x80*/ ,0,toy,GO_NEAR_TYPE_ADJ,0);
		adj_back_edgeways(n_sta_back,bx,toy);
		if(state != sys->sState)		///2018-07-16 jzz
			return 0;
		log_printf("motor_go_edgeways_OK\r\n");
	}
	if(navigat->lst_angle ==0)
		turn_to_deg(180);
	else
		turn_to_deg(0);						//旋转到0度
	save_map(1, 1,0);
	if(state != sys->sState)		///2018-07-16 jzz
		return 0;
#else

	//===================================运动，找特征物=============================
	//第一次走
	motor_go_forwark(0,(NO_SIDE_NEAR /*| 0x80*/), run_ajust_check); 	//等级1延边
	if(state != sys->sState)		///2018-07-16 jzz
		return 0;
	delay_ms(80);
	log_printf("after go fw,gsta=%d,ret=%d\r\n",sys->gsta,navigat->adj_run_ret);
	//=============================校验结果的计算==================================
	//if(navigat->adj_run_ret == 0)		//没有结果
	//	navigat->adj_run_ret = ADJ_BUM;

	dist1 = navigat->adj_walk_dis = navigat->walk_dis;			//记录下走的路程
	if(dist1 < HW_GRID)	//前进不够一格，就不走了。
	{
		log_printf("[adj_run]short dis=%d,not go adj\r\n",navigat->walk_dis);
		navigat->adj_run = FALSE;
		return 0;
	}

	log_printf("[adj_run]walkdis=%d,now turn to %d,and go back\r\n",navigat->adj_walk_dis,next_deg);

	tobst.ex = X_NOW;
	tobst.ey = Y_NOW;
	tobst.eret = navigat->adj_run_ret;
	tobst.bx = X_NOW;
	tobst.by = Y_NOW;
	tobst.bret = navigat->adj_run_ret;
	log_printf("begin(%d,%d,%d)\r\n",tobst.bx,tobst.by,tobst.bret);

	//工字型，第一次走特征物，如果距离不远的话，就不回去了。
	if(type == TYPE_ADJ_FIRST_RUN && navigat->adj_walk_dis < (HW_GRID+HW_HF_GRID))
	{
		log_printf("short dist,not adj\r\n");
		navigat->adj_run = FALSE;
		goto l_robot_ajust_run_end;
	}
	//转回去

	turn_to_deg(next_deg);				//旋转,如果是只有一个测距红外,认为左右两侧红外一样属性
	if(state != sys->sState)		///2018-07-16 jzz
		return 0;

	//在走回去
	log_printf("adj go back again\r\n");
	navigat->adj_x_org = navigat->x_org_f;
	navigat->adj_run_ret = 0;
	navigat->adj_nsta = navigat->adj_nsta == LEFT_SIDE_NEAR?RIGHT_SIDE_NEAR:LEFT_SIDE_NEAR;
	//===================================运动，找特征物=============================
	motor_go_forwark(0,(NO_SIDE_NEAR /*| 0x80*/), run_ajust_check); 	//等级1延边
	if(state != sys->sState)		///2018-07-16 jzz
		return 0;
	delay_ms(80);
	log_printf("after go fw,gsta=%d,ret=%d\r\n",sys->gsta,navigat->adj_run_ret);
	navigat->adj_run = FALSE;
	//再走回去检测，这里就好退出了，，特征物的校验，则在这里校验起来。
	if(type == TYPE_ADJ_CHECK_AGAIN)
	{
		int16_t ax,ay;		//矫正的值
		uint8_t ajusxy = 0;
		if(Y_NOW > tobst.by)	//超过开始的点，则为结束的点
		{
			tobst.ex = X_NOW;
			tobst.ey = Y_NOW;
			tobst.eret = navigat->adj_run_ret;
			log_printf("end(%d,%d,%d)ret = %d\r\n",tobst.ex,tobst.ey,tobst.eret,navigat->adj_run_ret);
			//差异不大，则矫正,且前后的结果有一头一致
			log_printf("targe=(%d,%d,%d)-(%d,%d,%d),my=(%d,%d,%d)-(%d,%d,%d)\r\n",
				g_obst->bx,g_obst->by,g_obst->bret,g_obst->ex,g_obst->ey,g_obst->eret,
				tobst.bx,tobst.by,tobst.bret,tobst.ex,tobst.ey,tobst.eret);
			if(dis_xy((tobst.ey - tobst.by),(g_obst->ey - g_obst->by)) <=1  &&	//micony2017-08-26条件苛刻点，2改成1
				(tobst.bret == g_obst->bret || tobst.eret == g_obst->eret) &&
				(navigat->adj_run_ret == ADJ_SIDE_LOST || navigat->adj_run_ret == ADJ_MID_BUM))
			{
				ajusxy = AJUST_X | AJUST_Y;
				ax  = g_obst->ex;
				ay	= g_obst->ey;

			}else if((tobst.ey - tobst.by) >=(g_obst->ey - g_obst->by) / 2 &&
					(tobst.ey - tobst.by) > 3)	//长度够，至少可以校验X轴
			{
				ajusxy = AJUST_X ;
				ax  = g_obst->ex;
				ay	= Y_NOW;
			}
		}
		else
		{
			tobst.bx = X_NOW;
			tobst.by = Y_NOW;
			tobst.bret = navigat->adj_run_ret;
			log_printf("begin(%d,%d,%d)ret=%d\r\n",tobst.bx,tobst.by,tobst.bret,navigat->adj_run_ret);
			log_printf("targe=(%d,%d,%d)-(%d,%d,%d),my=(%d,%d,%d)-(%d,%d,%d)\r\n",
				g_obst->bx,g_obst->by,g_obst->bret,g_obst->ex,g_obst->ey,g_obst->eret,
				tobst.bx,tobst.by,tobst.bret,tobst.ex,tobst.ey,tobst.eret);
			if(dis_xy((tobst.ey - tobst.by),(g_obst->ey - g_obst->by)) <=1  &&  	//micony2017-08-26条件苛刻点，2改成1
				(tobst.bret == g_obst->bret || tobst.eret == g_obst->eret) &&
				(navigat->adj_run_ret == ADJ_SIDE_LOST || navigat->adj_run_ret == ADJ_MID_BUM))
			{
				ajusxy = AJUST_X | AJUST_Y;
				ax  = g_obst->bx;
				ay	= g_obst->by;

			}
			else if((tobst.ey - tobst.by) >=(g_obst->ey - g_obst->by) / 2 &&
					(tobst.ey - tobst.by) > 4)	//长度够，至少可以校验X轴
			{
				ajusxy = AJUST_X ;
				ax  = g_obst->bx;
				ay	= Y_NOW;
			}
		}
		if(ajusxy)
		{
			log_printf("ajusxy=%d\r\n",ajusxy);
			if(check_map_reason(X_NOW,Y_NOW,ax,ay))
			{
				save_now_point(navigat);						//存储坐标
				ajust_xy_map(ax,ay,ajusxy)	;	//校准坐标
				calc_point_offset(navigat); 				//计算偏移量
			}
			/*
			if(verify_on_map(navigat,10)==0)				//地图校验失败，则回滚
			{
				log_printf("verify map error2\r\n");
				restor_point(navigat);
				return 0;
			}*/
		}
		navigat->c_turn = navigat->c_go = 0;
		//return 1;
		//该点已经清扫，就退出了。
		/*
		这样简单的退出，有问题，有时候进入未清扫区域，也会退出。
		if(SCANE_XY(X_NOW,Y_NOW))
		{
			return 1;
		}
		*/
	}
	else			//第一次找的特征直线
	{
		TOBST *mobst;
		uint8_t other_adj=0;
//		int16_t yy,xx;

		if(Y_NOW > tobst.by)	//超过开始的点，则为结束的点
		{
			tobst.ex = X_NOW;
			tobst.ey = Y_NOW;
			tobst.eret = navigat->adj_run_ret;
			log_printf("end(%d,%d,%d)\r\n",tobst.ex,tobst.ey,tobst.eret);
		}
		else
		{
			tobst.bx = X_NOW;
			tobst.by = Y_NOW;
			tobst.bret = navigat->adj_run_ret;
			log_printf("begin(%d,%d,%d)\r\n",tobst.bx,tobst.by,tobst.bret);
		}
		navigat->adj_walk_dis = navigat->walk_dis;
		log_printf("adj walk ok,dist1=%d,nowdist=%d\r\n",dist1,navigat->adj_walk_dis);
		//判断是不是从重复的ADJ
		other_adj = 0;
		/*
		yy = (tobst.by + tobst.ey) / 2;
		for(xx=tobst.bx - 5; xx <= tobst.bx + 5;xx++)
		{
			mobst = get_adj_obst(xx, navigat->ty);
			if(mobst !=NULL)
			{
				log_printf("found an other adj,xdir=%d,%d\r\n",mobst->x_dir ,tobst.x_dir);
				if(mobst->x_dir == tobst.x_dir)		//方向一致,发现一致的adj
				{
					log_printf("the same side..\r\n");
					other_adj = 1;
					break;
				}
			}
		}
		*/
		if(other_adj == 1)
		{
			int16_t ax,ay;		//矫正的值
			uint8_t ajusxy = 0;
			if(dis_xy((tobst.ey - tobst.by),(mobst->ey - mobst->by)) <=2	&&
				(tobst.bret == mobst->bret || tobst.eret == mobst->eret) &&
				(navigat->adj_run_ret == ADJ_SIDE_LOST || navigat->adj_run_ret == ADJ_MID_BUM))
			{
				ajusxy = AJUST_X | AJUST_Y;
				ax	= g_obst->bx;
				ay	= g_obst->by;
			}
			else if((tobst.ey - tobst.by) >=(mobst->ey - mobst->by) / 2 &&
					(tobst.ey - tobst.by) > 4)	//长度够，至少可以校验X轴
			{
				ajusxy = AJUST_X ;
				ax	= g_obst->bx;
				ay	= Y_NOW;
			}
			if(ajusxy)
			{
				log_printf("ajusxy=%d\r\n",ajusxy);
				ajust_xy_map(ax,ay,ajusxy)	;	//校准坐标
			}
		}
		else
		{
			//插入特征物列表
			if(navigat->j_obst < MAX_TARGET)
			{
				log_printf("[adj_run]insert(%d,%d,%d)-(%d,%d,%d)idx=%d\r\n",
							tobst.bx,tobst.by,tobst.bret,tobst.ex,tobst.ey,tobst.eret,navigat->j_obst);
				mobst = &navigat->adj_obst[navigat->j_obst++];
				mobst->bx = tobst.bx;
				mobst->by = tobst.by;
				mobst->bret = tobst.bret;
				mobst->ex = tobst.ex;
				mobst->ey = tobst.ey;
				mobst->eret = tobst.eret;
				mobst->x_org = navigat->x_org;
				mobst->x_dir = tobst.x_dir;
			}else
				log_printf("[adj_run]target overflow,%d\r\n",navigat->j_obst);
		}

	}





	log_printf("[adj_run]go back,now=(%d,%d,),to=(%d,%d,)\r\n",X_NOW,Y_NOW,bx,toy);
	navigat->c_go = navigat->c_turn = 0;
	//回来有碰撞
	navigat->adj_x_org = 0	;		//不再强制X轴了
	if(toy != Y_NOW)
	{
		float angle = sys->angle;
		if(toy > Y_NOW)
			turn_to_deg(90);					//在TOY负半轴转到90
		else
			turn_to_deg(270);					//在toy正半轴转到270			如果相等了呢，可以故意调试测试一下
		if(state != sys->sState)		///2018-07-16 jzz
			return 0;
		if(disfloat(sys->angle,angle) <=40)		//方向不变，延边就不变
		{
			log_printf("angle not chagne\r\n");
			n_sta_back = navigat->adj_nsta;
		}else
			n_sta_back = navigat->adj_nsta == LEFT_SIDE_NEAR?RIGHT_SIDE_NEAR:LEFT_SIDE_NEAR;
		log_printf("motor_go_edgeways,nsta=%d\r\n",n_sta_back);
		//motor_go_edgeways(n_sta_back /*| 0x80*/ ,0,toy,GO_NEAR_TYPE_ADJ,0);
		adj_back_edgeways(n_sta_back,bx,toy);
		if(state != sys->sState)		///2018-07-16 jzz
			return 0;
		log_printf("motor_go_edgeways_OK1\r\n");
	}
	/*
	if(motor_go_forwark(next_deg,NO_SIDE_NEAR, NULL)==0)
	{
		log_printf("[adj_run]go back bum\r\n");
		//走得差不多了，就不管了，直接走
		if(disxy(navigat->walk_dis,navigat->adj_walk_dis) < HW_HF_GRID)
		{

		}else
		{
			log_printf("go back error,near wall,n_near=%d\r\n",n_sta_back);
			motor_go_edgeways(n_sta_back ,bx,by,GO_NEAR_TYPE_ADJ,0);
			log_printf("near ok,now=(%d,%d,%3.1f)\r\n",X_NOW,Y_NOW,A_NOW);
			//near_cross_xy(int tox,int toy,uint8_t ctype,int dir)
		}
	}
	*/
l_robot_ajust_run_end:
	if(navigat->lst_angle ==0)
		turn_to_deg(180);
	else
		turn_to_deg(0);						//旋转到0度
	save_map(1, 1,0);
	if(state != sys->sState)		///2018-07-16 jzz
		return 0;
#endif
	return 1;

}

char adj_back_edgeways(uint8_t n_sta ,short tox,short toy)
{
	int calue=0;
	uint8_t sta;
	short llx=0,lly=0;
//	short lx=X_NOW,ly=Y_NOW;
	short by = Y_NOW;
//	short bx = X_NOW;
//	short nx1,ny1;
	float lagle;		//记录最后的延边角度，用于计算是否转弯过多
//	uint16_t	gSta;
	uint32_t	t_begin;//开始的时间
	int ret_calc=1;
//	uint16_t c_dock_data=0;
	int16_t c_lost=0;
	uint16_t c_round=0;		//转圈的次数
	int c_near_wall = 0;


	short x_begin_line=0,y_begin_line = 0;		//一条线开始的X，Y的坐标

//	u8 irData_bak[6];
	//int c_dock = 0;
	int16_t dist_y = dis_xy(Y_NOW,toy)+2;

	NEAR_WALL *near = &navigat->near;

	float m_angle[MAX_C_M_ANGLE];			//保存最近的20个碰撞的角度
	int16_t c_m_angle=0;

#if LAGER_ROUND_MODE
	int16_t cyc_x[MAX_C_M_ANGLE],cyc_y[MAX_C_M_ANGLE];
	uint8_t tmp=0;
#else
	int16_t c_near_wall1 = 0;
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
#if JUDGE_PID_CTRL_WHEEL_STOP
	m_speed_pid_ctrl(0,0,0,0,0,0,0);		//记录左轮霍尔数
#else
	m_speed_pid_ctrl(0,0);
#endif

	motor_run(GO_FORWARD, GO_NEAR_PWM_FAST, 0, 0);
	if(n_sta & 0x80)
		init_near_wall_navi(n_sta & 0x7F);
	else
		init_near_wall_navi(NO_SIDE_NEAR);
	n_sta &=0x7F;
	//init_near_wall_navi(NO_SIDE_NEAR);	//刚开始不需要一直沿边
	navigat->is_walk =1;
	log_printf("[adj_back_edgeways]sta=%d(%d),now=(%d,%d,)to=(%d,%d,)dis=%d\r\n",navigat->near.n_sta,n_sta,X_NOW,Y_NOW,tox,toy,dist_y);
	//navigat->wheel_dir = 1;
	sta = sys->sState;
	lagle = sys->angle;
	mstimeout(&t_begin,0);
	while(1)
	{

		proc_uart_task();
		get_sensers(&sys->gSta);
		if(sys->sState !=sta)
			return RET_NEAR_ERROR;
		CHECK_NAVI_STA_RT(0);
		//碰撞
		/*
		if(sys->gSta & (MASK_BUM_LEFT | MASK_BUM_RIGHT | MASK_BUM_LEFT2  | MASK_BUM_RIGHT2 ))
			{
				if(sys->fall_sta)		//跌落，则
					motor_back_off(BACK_HW*8);
				else
					motor_back_off(BACK_HW);
               log_printf("bum 999\r\n");
			}
		*/
		if(sys->gSta & (MASK_BUM_MIDL) ||(sys->gSta & (MASK_BUM_LEFT | MASK_BUM_LEFT2 | MASK_BUM_RIGHT | MASK_BUM_RIGHT2))||  ret_calc==0 ||  c_lost >=MAX_NEAR_LOST)
		{
//			gSta = sys->gSta;	//记录碰撞的状态
			motor_run(GO_STOP,0,0,0);
			log_printf("1BUM    \r\n");
			coordinate_calcu(0);														//计算出原始的坐标系
			coord_org2map(navigat->x_org,navigat->y_org,&navigat->tx,&navigat->ty); //坐标系转换
			motor_run(GO_STOP,0,0,0);
			log_printf("\r\n-----adj_back_edgeways bum(%d,%d,%d,%f,%f),gsta=%d,irda=(%d,%d,%d,%d)angle=%3.1f,ret=%d,lost=%d\r\n",navigat->tx,navigat->ty,motor.c_left_hw,navigat->x_org_f,navigat->y_org_f,
							sys->gSta,sys->g_sta[0],sys->g_sta[1],sys->g_sta[5],sys->g_sta[6],sys->angle,ret_calc,c_lost);


			if(!(ret_calc==0 || c_lost >=MAX_NEAR_LOST))
				c_round = 0;
			if(ret_calc==0)
					gyro_mapwhl();
//			gSta = sys->gSta;
			//motor_run(GO_STOP,0,0,0);
			if(sys->work_mod & MWO_MOP)
				delay_ms(200);
			//www_idleintel_com();
			lagle = sys->angle;

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
				//goto l_go_edeways;
				return RET_NEAR_OK;
			}
#else////2018-07-10  未测试6
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
				return RET_NEAR_OK;
			}
#endif
			/**************************************************************************
				如果tox，toy有设置了置，则到点了，就停下来。
			*****************************************************************************/
#if EDGE_DRAW_MAP
			if(   (Y_NOW == toy ) ||	//到点了
				( (by > toy && Y_NOW < toy) || (by < toy && Y_NOW > toy))/* ||
				 dis_xy(Y_NOW,toy) >= dist_y*/) //adj回去，Y轴超过?
#else
			if(   (Y_NOW == toy ) || 	//到点了
				( (by > toy && Y_NOW < toy) || (by < toy && Y_NOW > toy)) ||
				 dis_xy(Y_NOW,toy) >= dist_y) //adj回去，Y轴超过了
#endif
			{
				log_printf("xy ok1(%d,%d,%3.1f),(by=%d,toy=%d),dist_ty=%d\r\n",X_NOW,Y_NOW,sys->angle,by,toy,dist_y);
				motor_run(GO_STOP,0,0,0);
				return RET_NEAR_OK;
			}


			if(near->n_sta ==NO_SIDE_NEAR)
			{
				init_near_wall_navi(n_sta);
			}

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
				if(ret_calc==0)
					gyro_mapwhl();

				if(near->n_sta == RIGHT_SIDE_NEAR)
				{
					robot_turn_deg(GO_RIGTH,TURN_DEG_PWM,30);
					goto l_go_edeways;
				}else if(near->n_sta == LEFT_SIDE_NEAR)
				{
					robot_turn_deg(GO_LEFT,TURN_DEG_PWM,30);
					goto l_go_edeways;
				}

			}




			if(near->n_sta == RIGHT_SIDE_NEAR)
			{
				//modify201710	发现这个bug，导致反复碰撞。之前不知道为何如此
				/*
				if((gSta & MASK_BUM_LEFT))
				{
					log_printf("midbum,10deg\r\n");
					robot_turn_deg(GO_RIGTH,TURN_DEG_PWM,10);
				}else
				*/
				{
					robot_turn_deg(GO_LEFT,TURN_DEG_PWM,12);
				}
			}else if(near->n_sta == LEFT_SIDE_NEAR)
			{
				//log_printf("gsta=%d,left=%d,%d\r\n",sys->gSta,sys->g_sta[0],sys->g_sta[1]);
				//modify201710	发现这个bug，导致反复碰撞。之前不知道为何如此
				/*
				if((gSta & MASK_BUM_RIGHT)  )
				{
					log_printf("midbuml,10deg\r\n");
					robot_turn_deg(GO_LEFT,TURN_DEG_PWM,10);
				}else
				*/
				{
						//log_printf("midbuml,12deg\r\n");
					robot_turn_deg(GO_RIGTH,TURN_DEG_PWM,12);
				}

			}else
			{
				log_printf("RET_NEAR_ERROR,nsta=%d\r\n",n_sta);
				return RET_NEAR_ERROR;
			}
l_go_edeways:
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
			//gyro_whlmap();

			motor_run(GO_FORWARD, GO_NEAR_PWM_FAST, 0, 0);
			motor.c_left_hw = motor.c_right_hw = 0;
#if JUDGE_PID_CTRL_WHEEL_STOP
	m_speed_pid_ctrl(0,0,0,0,0,0,0);		//记录左轮霍尔数
#else
			m_speed_pid_ctrl(0,0);
#endif
			c_near_wall = 0;
			navigat->near.pid->c_lost = 0;
			navigat->near.pid->c_lost_flag = 0;
			ret_calc = 1;
			//记录起点的位置
			x_begin_line = X_NOW;
			y_begin_line = Y_NOW;
#if !LAGER_ROUND_MODE
			c_near_wall1 = 0;
#endif
		}

		if(TIM5->CNT >=5000)
		{
			TIM5->CNT = 0;
			www_idleintel_com();

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
#if !CALE_BY_FRON
				ret_calc = coordinate_calcu(0);														//计算出原始的坐标系
#endif
				//ret_calc = coordinate_calcu();														//计算出原始的坐标系
				coord_org2map(navigat->x_org,navigat->y_org,&navigat->tx,&navigat->ty);	//坐标系转换
				//不是0度，90度，80度的方向,刷新坐标
				/**/
				if(x_begin_line != X_NOW && y_begin_line != Y_NOW)
				{
					ajust_xy_by_near_line(x_begin_line,y_begin_line,X_NOW,Y_NOW,sys->angle,LINE_TYPE_LOST,n_sta);
					x_begin_line = X_NOW;
					y_begin_line = Y_NOW;
				}
#if !LAGER_ROUND_MODE
				if(c_near_wall1 ++ >=30)
				{

					for(c_m_angle = 0;c_m_angle<MAX_C_M_ANGLE;c_m_angle++)
						m_angle[c_m_angle] = 0;
					c_m_angle = 0;
					c_near_wall1 = 0;
					log_printf("log near\r\n");
				}
#endif
				//延边数据
				if(LEFT_ADC() > cfg->lock_left_adc || RIGHT_ADC() > cfg->lock_right_adc)
					c_near_wall ++ ;
#if !JUDGE_PID_CTRL_WHEEL_STOP
				if(c_near_wall >=20 && dis_xy(L_FORWRK_PWM,R_FORWRK_PWM) < 100)
				{
					m_speed_pid_ctrl(0,1);			///
				}
#endif
				c_near_wall = 0;

				if( *(navigat->near.pid->adc) > navigat->near.pid->min_adc) //延边
				{
					//c_near++;
					c_round = 0;
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
#if EDGE_DRAW_MAP
				if(   (Y_NOW == toy ) ||	//到点了
				( (by > toy && Y_NOW < toy) || (by < toy && Y_NOW > toy))/* ||
				 dis_xy(Y_NOW,toy) >= dist_y*/) //adj回去，Y轴超过了

#else
				if(   (Y_NOW == toy ) || 	//到点了
				( (by > toy && Y_NOW < toy) || (by < toy && Y_NOW > toy)) ||
				 dis_xy(Y_NOW,toy) >= dist_y) //adj回去，Y轴超过了
#endif
				{
				    log_printf("xy ok2(%d,%d,%3.1f),(by=%d,toy=%d),dist_ty=%d\r\n",X_NOW,Y_NOW,sys->angle,by,toy,dist_y);

					motor_run(GO_STOP,0,0,0);
					return RET_NEAR_OK;
				}
			}
		}
	}
//	return 0;
}

//在走一次，做矫正
#if 0
int run_ajust_again(void)
{
//	int idx;
	//沿边消失掉，就退出
	if(navigat->adj_nsta == LEFT_SIDE_NEAR )		//左沿边
	{
		if(LEFT_ADC() < IRD_LEFT_ADJ_MIN)
		{
			log_printf("adj:%d\r\n",LEFT_ADC());
			if(navigat->adj_c_lost++ > LOST_NUBUER)
			{
				motor_run(GO_STOP,0,0,0);
				if(LEFT_ADC() < SIDE_LOST_LEFT)
					navigat->adj_run_ret = ADJ_SIDE_LOST;	//特征物走法正确
				else
					navigat->adj_run_ret = ADJ_FAIL;
				log_printf("[run_ajust_check]lost,ret=%d\r\n",navigat->adj_run_ret);
				return 0;
			}
		}else
			navigat->adj_c_lost = 0;
	}
	if(navigat->adj_nsta == RIGHT_SIDE_NEAR )		//左沿边
	{
		if(RIGHT_ADC() < IRD_RIGHT_ADJ_MIN)
		{
			log_printf("adjr:%d\r\n",RIGHT_ADC());
			if(navigat->adj_c_lost++ > LOST_NUBUER)
			{
				motor_run(GO_STOP,0,0,0);
				//真正的失去了墙边，才算是找到特征物
				if(RIGHT_ADC() < SIDE_LOST_RIGHT)
					navigat->adj_run_ret = ADJ_SIDE_LOST;	//特征物走法正确
				else
					navigat->adj_run_ret = ADJ_FAIL;
				log_printf("[run_ajust_check]lost2,ret=%d\r\n",navigat->adj_run_ret);
				return 0;
			}
		}else
		  navigat->adj_c_lost = 0;
	}


	return 1;
}
#endif
//导航到特征点
/*
 type - 0 不强制导航，1强制导航
 不强制导航:根据转的圈数和行走的次数来
返回值

#define ADJ_NAVI_NO_RUN		0x00	//不符合导航到特征点的条件，没动，没导航过去
#define ADJ_NAVI_RUN_OK		0x01	//符合导航到特征点的条件，且导航过去，矫正成功
#define ADJ_NAVI_RUN_ERR	0x02	//符合导航到特征点的条件，且导航过去，不过是否，但机子动了

*/
int THERE_OBST(short X,short Y)
{
	return (OBST_XY(X-1,Y) || OBST_XY(X,Y) || OBST_XY(X+1,Y));
}
#define ADJ_XY		0x00
#define ADJ_X		0x01
#define ADJ_Y		0x02

char navigat_to_target(uint8_t type)
{
	short nx,ny;
	//int x,y;
	int idx;//,j,i;
	float angle;//,a_back=0;
	TOBST *obst;//,*eobt;
//	uint8_t sta;
	uint8_t c_target=0;
	uint8_t router = 0;
	uint8_t	adj_type = 0;
	short x,y;
	uint16_t state;
//	int y,x;
	//不强制导航，要先计算一下，是否符合导航的条件
#if !NAVI_TO_TARGET

	return ADJ_NAVI_NO_RUN;
#endif
	state = sys->sState;
	//刚刚矫正完，一律不矫正
	if(navigat->c_target_go < 8)
		return ADJ_NAVI_NO_RUN;
	//近距离的校准，就去判断
	if(type == NAV_ADJ_ANGLE)
	{
		if(navigat->c_turn < 20 )
			return ADJ_NAVI_NO_RUN;
		log_printf("[navitag]NAV_ADJ_ANGLE...,%d,%d\r\n",navigat->c_turn ,navigat->c_go );
		goto l_nav_adj_check;

	}
	if( type == NAVA_ADJ_NEAR)
	{
		if(navigat->c_turn < 10 && navigat->c_go < 10)
			return ADJ_NAVI_NO_RUN;
		log_printf("[navitag]near...,%d,%d\r\n",navigat->c_turn ,navigat->c_go );
		goto l_nav_adj_check;
	}
	if(type ==0)
	{
		if(navigat->c_turn < 30 || navigat->c_go < 30)
		{
			log_printf("[navitag]c error,%d,%d\r\n",navigat->c_turn ,navigat->c_go);
			//goto l_navi_to_targ_check_again
			return ADJ_NAVI_NO_RUN;
		}
	}
		//不是强制导航，就根据次数来
	if(type != NAV_ADJ_FORCE_ALL)
	{
		if(navigat->c_go_force < 10)
		{
			log_printf("[navitag]c_go_force error,%d\r\n",navigat->c_go_force);
			//goto l_navi_to_targ_check_again
			return ADJ_NAVI_NO_RUN;
		}
	}
	log_printf("[navitag]c=%d\r\n",navigat->j_obst);
l_nav_adj_check:
	//找特征物列表里，能不能简单的导航过去，能导航过去，就调用导航函数navigat_robot，导航过去
	for(idx=0;idx<navigat->j_obst;idx++)
	{
		obst = &navigat->adj_obst[idx];
		//计算出中心点的坐标
		y = (obst->by + obst->ey) / 2;
		x = obst->ex;
		//从充电桩出来,在附近的特征物，要比较长，且只能到导航到离充电比较远的地方
		if(navigat->from_charge && dis_xy(x,navigat->dock_x) < 5)
		{
			log_printf("targe near dock(%d,%d,%d)i=%d\r\n",x,y,obst->x_dir,idx);
			if(dis_xy(y,navigat->dock_y) < 5)
			{
				log_printf("near dock,not get\r\n");
				continue;
			}
		}else
			log_printf("targe(%d,%d,%d)i=%d\r\n",x,y,obst->x_dir,idx);
		//if( OBST_XY(x+1,y) && OBST_XY(x-1,y) ==0)
		if(dis_xy(obst->by,obst->ey) < 6)
		{
			log_printf("short dist(%d,%d,)\r\n",x,y);
			continue;
		}
		if(obst->x_dir == 1)
		{
			angle = 0;
			//这有隐患
			x-=1;

		}//else if( OBST_XY(x+1,y)==0 && OBST_XY(x-1,y) ==1)
		else if(obst->x_dir == -1)
		{
			angle = 180;
			x +=1;
		}
		else
		{
			log_printf("no obst_12(%d,%d,)\r\n",x,y);
			continue;
		}



		//能按照简单的路径导航过去的，可以考虑导航
		router = 0;
		if(router_p2p(navigat->tx,navigat->ty, x,y,&nx,&ny) )
		{
			router = 1;
		}
		//如果是强制导航，则考虑更复杂的路线
		if(router == 0 && type == NAV_ADJ_FORCE_ALL)
		{
			if(search_route(navigat->tx,navigat->ty,x,y,&nx,&ny))
				router = 1;
		}
		if(router == 1)		//能导航过去
		{
			//如果不是强制校验特征点，则在特征点的附近才矫正
			if(type ==0 && navigat->c_turn < 30 && navigat->c_go < 30)
			{
				int max_dis;
				if(type == NAV_ADJ_FORCE_H)
					max_dis = 10;
				else
					max_dis = 5;
				if(disXY(navigat->tx, x) >= max_dis || disXY(navigat->ty, y) >= max_dis )
				{
					log_printf("[navitag]long dis out,(%d,%d,)\r\n",x,y);
					continue;
				}
			}
			//太远的，也不能去校准了。
			if(disXY(navigat->tx, x) >= 55 || disXY(navigat->ty, y) >= 55 )
			{
				log_printf("[navitag]long dis out2,(%d,%d,)\r\n",x,y);
				continue;
			}
			if( type == NAVA_ADJ_NEAR)
			{
				if(disXY(navigat->tx, x) >= 5 || disXY(navigat->ty, y) >= 5
					|| obst == navigat->l_adj_obst)
				{
					log_printf("[navitag]near long dis out2,(%d,%d,)\r\n",x,y);
					continue;
				}

			}
			if( type == NAV_ADJ_ANGLE)
			{
				if(disXY(navigat->tx, x) >= 5 || disXY(navigat->ty, y) >= 5 )
				{
					log_printf("[navitag]near long dis out2,(%d,%d,)\r\n",x,y);
					continue;
				}

			}
			goto l_navi_to_targ;

		}
	}
//l_navi_to_targ_check_again:
	adj_type++;
	log_printf("[navitag]end of check adj_type=%d\r\n",adj_type);
	return ADJ_NAVI_NO_RUN;
	/*
	if(adj_type>=2)
	{
		log_printf("[navitag]no run\r\n");
		return ADJ_NAVI_NO_RUN;
	}else
		goto l_nav_adj_check;
		*/
l_navi_to_targ:
	log_printf("[navitag]found target=(%d,%d,%3.1f)\r\n",x,y,angle);

	if(type == NAV_ADJ_FORCE_H || c_target <=2)
		navigat->c_go_force = 0;		//强制特征物矫正复位
	//导航过去
	navigat->sta = NAV_STA_NAVIGATE;
	navigat->l_adj_obst = obst;		//记录最后矫正的特征点
	navigat->c_target_go = 0;
	//if(navigat_robot(NAVI_TYPE_NJST,DONT_SEARCH_UNCLR,obst->x,obst->y,angle) == NAVIGAT_OK)
	//=========================================导航到adj点=====================================
	if(navigat_to_point(x,y,angle) == NAVIGAT_OK)
	{
		//导航到位了，先垂直的冲过去，然后再拐弯走，校准
		log_printf("[navitag]turn to %3.1f\r\n",angle);

		//转到垂直的角度，然后走过去
		turn_to_deg(angle);
		if(sys->sState != state)			///2018-07-16 jzz
			return ADJ_NAVI_NO_RUN;
		//========================================垂直墙壁冲过去=====================================
		if(motor_go_forwark(6*HW_GRID,NO_SIDE_NEAR, NULL) ==1)	//如果没有碰撞，则错误了。
		{
			log_printf("[navitag]navi targe error!!");
			return ADJ_NAVI_RUN_ERR;
			 //回来，调整Y轴坐标，再冲一次
			// if(navi_to_new_adj(obst->x,obst->y,eobt->x,eobt->y,navigat->walk_dis)==0)
			//	return ADJ_NAVI_RUN_ERR;
		}
		if(sys->sState != state)			///2018-07-16 jzz
			return ADJ_NAVI_NO_RUN;
		if(robot_ajust_run(TYPE_ADJ_CHECK_AGAIN,0,obst))
		{

			return ADJ_NAVI_RUN_OK;
		}
		else
			return ADJ_NAVI_RUN_ERR;



	}else
	{
		log_printf("[navitag]navi ERROR\r\n");
		return ADJ_NAVI_RUN_ERR;
	}


}

//在X轴上 连续的四个被清扫，且没有障碍物
int four_cln_sta(short x ,short y)
{
	if(SCANE_XY(x,y) && SCANE_XY(x-1,y) && SCANE_XY(x-2,y) && SCANE_XY(x+1,y) && SCANE_XY(x+2,y) &&
		OBST_XY(x,y)==0 && OBST_XY(x-1,y)==0 && OBST_XY(x-2,y)==0 && OBST_XY(x+1,y)==0 && OBST_XY(x+2,y)==0 )
		return 1;
	else
		return 0;

}
/*
寻找特征点最适合的点
bx,by 本来该去的特征点
ex,ey :特征点终点
dist:刚才垂直冲过去，走过的距离，用于后退回去

*/
int navi_to_new_adj(short bx,short by,short ex,short ey,short dist)
{
	int i;
	short ox,oy;
	int deg;
	float angle;
	//后退回去
	deg = navigat->angle ==0?180:0;
	log_printf("[navi_to_new_adj]turn back,to %d\r\n",deg);
	turn_to_deg(deg);
	if(motor_go_forwark(dist+10,NO_SIDE_NEAR,NULL)==0)
	{
		log_printf("[navi_to_new_adj]walk error\r\n");
		return 0;
	}

	//往Y轴正向找，找到一片可以去的区域,计算出是Y轴过头，还是过尾了。
	for(i=1;i<6;i++)
	{
		if(four_cln_sta(X_NOW,Y_NOW+i))		//发现这个区域不是障碍物了，那么可能是走到这里来了
		{
			//过头了。后退五格
			ox = bx;
			oy = by -5;	//后退五格
			log_printf("[navi_to_new_adj]found four cln,may be here(%d,%d,) navi to(%d,%d,)\r\n",X_NOW,Y_NOW+i,ox,oy);
			goto l_navi_to_new_adj;
		}
	}
	for(i=1;i<6;i++)
	{
		//在尾巴去了，再往前5格
		if(four_cln_sta(X_NOW,Y_NOW - i))		//发现这个区域不是障碍物了，那么可能是走到这里来了
		{
			ox = bx;
			oy = by + 5;	//再往前5格
			log_printf("[navi_to_new_adj]found four cln,may be here(%d,%d,) navi to(%d,%d,)\r\n",X_NOW,Y_NOW - i,ox,oy);
			goto l_navi_to_new_adj;
		}
	}
	return 0;
l_navi_to_new_adj:

	angle = navigat->angle ;		//回来后的角度


	if(walk_map_cord_only(X_NOW,oy,angle,0)==0)
	{
		log_printf("[navi_to_new_adj]walk_map error\r\n");
		return 0;
	}
	log_printf("[navi_to_new_adj]walk to obst...\r\n");
	if(motor_go_forwark(3*HW_GRID,NO_SIDE_NEAR, NULL) ==1)	//如果没有碰撞，则错误了。
	{
		log_printf("[navi_to_new_adj]navi targe error!!");
		return 0;
	}
	return 1;

}
//90度或270°碰撞的时候校准Y轴
#if ADJ_Y_BUM

void ajust_y_on_move(void)
{
	int i,c=0;
	TOBST *t_obst;
	log_printf("[ajust_y_on_move]searchting...\r\n");
	for(i=0;i<navigat->j_obst;i++)
	{
		t_obst = &navigat->adj_obst[i];
		//只有中间碰撞，才能矫正
		if(t_obst->angle == NO_ANGLE_V )
			c++;
		//X轴相等、Y轴相差不大，且特征点是终点,且特征时正面碰撞
		if(navigat->tx == t_obst->x && disXY(navigat->ty, t_obst->y) <=2 && t_obst->angle == NO_ANGLE_V && t_obst->sta ==ADJ_MID_BUM)
		{
			if(i>0)
			{
				log_printf("[ajust_y_on_move]found i=%d,(%d,%d,%3.1f,%3.1f)c=%d\r\n",i,t_obst->x,t_obst->y,navigat->adj_obst[i-1].angle,sys->angle,c);
				//角度一致才能校准
				if(disfloat(navigat->adj_obst[i-1].angle, sys->angle) <=20)
				{
					if(c <=2)
						ajust_xy_org(t_obst->x_org, t_obst->y_org, 80, AJUST_X | AJUST_Y);
					else
						ajust_xy_org(t_obst->x_org, t_obst->y_org, 50, AJUST_X | AJUST_Y);
					navigat->force_obst = 1;		//暂时让它强制置障碍物。因为校准过后，Y轴的坐标变了，可能会有在原先的扫描点上置障碍物。
				}
			}
		}

	}
}
#endif
#else
char navigat_to_target(uint8_t type)
{


	return ADJ_NAVI_NO_RUN;
}
#endif

void near_turn_check(uint8_t nsta)
{
#if ADJ_NAER_TURN_V


	uint16_t i;
	if(nsta == LEFT_SIDE_NEAR)		//左沿边
	{
		if(LEFT_ADC()  < cfg->lock_left_adc && LEFT_ADC() > cfg->min_left_adc)
		{
			for(i=0;i<4;i++)
			{
				log_printf("[near_turn_check]side=%d,adc=%d,%d\r\n",nsta,LEFT_ADC(),LEFT_ADC1());
				if(LEFT_ADC()  < cfg->lock_left_adc && LEFT_ADC() > cfg->min_left_adc)
				{
					log_printf("in lost side,turn to left\r\n");
					robot_turn_deg(GO_LEFT,650,8);
				}else
					break;
			}
		}
		if(LEFT_ADC() > cfg->max_left_adc || LEFT_ADC1() >= cfg->max_left2_adc)
		{
			for(i=0;i<4;i++)
			{
				log_printf("[near_turn_check]side=%d,adc=%d,%d\r\n",nsta,LEFT_ADC(),LEFT_ADC1());
				if(LEFT_ADC() > cfg->max_left_adc || LEFT_ADC1() >= cfg->max_left2_adc)
				{
					log_printf("in near side,turn to right\r\n");
					robot_turn_deg(GO_RIGTH,650,8);
				}else
					break;
			}

		}

	}else if(nsta == RIGHT_SIDE_NEAR)
	{
		if(RIGHT_ADC()  < cfg->lock_right_adc && RIGHT_ADC() > cfg->min_right_adc)
		{
			for(i=0;i<4;i++)
			{
				log_printf("[near_turn_check]side=%d,adc=%d,%d\r\n",nsta,RIGHT_ADC(),RIGHT_ADC1());
				if(RIGHT_ADC()  < cfg->lock_right_adc && RIGHT_ADC() > cfg->min_right_adc)
				{
					log_printf("in lost side,turn to rigjt\r\n");
					robot_turn_deg(GO_RIGTH,650,8);
				}else
					break;
			}
		}
		if(RIGHT_ADC() > cfg->max_right_adc || RIGHT_ADC1() >= cfg->max_right2_adc)
		{
			for(i=0;i<4;i++)
			{
				log_printf("[near_turn_check]side=%d,adc=%d,%d\r\n",nsta,RIGHT_ADC(),RIGHT_ADC1());
				if(RIGHT_ADC() > cfg->max_right_adc || RIGHT_ADC1() >= cfg->max_right2_adc)
				{
					log_printf("in near side,turn to right\r\n");
					robot_turn_deg(GO_LEFT,650,8);
				}else
					break;
			}

		}


	}
#endif
}


/*
工字清扫的时候，
延边走一段，留下延边的数据
*/
int16_t target_run_z_go(void)
{
#if TARGE_RUN
	uint8_t n_sta;
	int deg;
	int16_t by=Y_NOW,idx;


	TARGET_T *target;
	CHECK_NAVI_STA_RT(0);

	if(!MIDLE_HAVE_IRDA2())
	{
		log_printf("[target_run_z_go]not mid\r\n");
		return 0;
	}
	deg = (int)navigat->lst_angle;
	target = get_target(deg,0,by,&idx);
	if(target != NULL)
	{
		int16_t x,y;
		x = xy[idx].x;
		y = xy[idx].y;
		log_printf("[target_run_z_go]found target=(%d,%d,%d),no run...\r\n",x,y,idx);
		//这里矫正。
		return 0;
	}

	if(navigat->side == LEFT_SIDE)
	{
		n_sta = navigat->lst_angle == 0?RIGHT_SIDE_NEAR:LEFT_SIDE_NEAR;
		deg = 90;
	}else
	{
		n_sta = navigat->lst_angle == 0?LEFT_SIDE_NEAR:RIGHT_SIDE_NEAR;
		deg = 270;
	}
	log_printf("[target_run_z_go](%d,%d,%3.1f)nsta=%d,turn to=%d\r\n",navigat->tx,navigat->ty,navigat->angle,n_sta,deg);
	turn_to_deg_v(deg);


	target_run_check(0,deg);
	motor_go_near_wall(0, n_sta,GO_TYPE_SAVE,OUT_TARGET_RUN,0,0);
	n_sta = n_sta == LEFT_SIDE_NEAR?RIGHT_SIDE_NEAR:LEFT_SIDE_NEAR;
	if(n_sta == LEFT_SIDE_NEAR)
		robot_turn_deg(GO_LEFT,DEG_TURN_PWM,WALL_LOST_DEG);
	else
		robot_turn_deg(GO_RIGTH,DEG_TURN_PWM,WALL_LOST_DEG);


	motor_go_near_wall(by, n_sta,GO_TYPE_SAVE,OUT_TARGET_BACK,0,0);
	if(navigat->lst_angle == 0)
		turn_to_deg(180);
	else
		turn_to_deg(0);
#endif
	return 1;
}

/*
根据X，Y 找到对应的特征点
*/
TARGET_T *get_target(int deg,int16_t xx,int16_t yy,int16_t *idx)
{
#if TARGE_RUN

	TARGET_T *target=NULL;
	for(int16_t i=0;i<navigat->i_target;i++)
	{
		target = &navigat->target[i];
		if(deg==0 || deg ==180)
		{
			if(target->deg != deg)
				continue;
		}
		for(int16_t j=target->bidx;j<target->eidx;j++)
		{
			if(xx >0 )
			{
				if(xx ==xy[j].x && yy == xy[j].y)
				{
					*idx = j;
					return target;
				}
			}else if( yy == xy[j].y)
			{
				*idx = j;
				return target;
			}

		}
	}
#endif
	return NULL;
}

int16_t target_run_check(uint8_t type,int deg)
{
#if TARGE_RUN

	static int16_t by,bx;
	static int 	bdeg;
	int16_t ret;
	switch(type)
	{
		//	初始参数
		case 0:
			by = Y_NOW;
			bx = X_NOW;
			bdeg = deg;
			return 0;
		case 1:
			//碰撞
			if(disxy(Y_NOW,by) >=10 || disxy(X_NOW,bx) >=10)
			{
				log_printf("[target_run]YOK\r\n");
				return 	1;
			}
		case 2: //延边
			//角度判断 不能折回来
			if(bdeg == 90)
			{
				if(sys->angle > 220 && sys->angle < 330)
				{
					log_printf("[target_run]angle err,%d->%3.1f\r\n",deg,sys->angle);
					return 	1;
				}
			}else if(bdeg ==270)
			{
				if(sys->angle > 30 && sys->angle < 150)
				{
					log_printf("[target_run]angle errs,%d->%3.1f\r\n",deg,sys->angle);
					return	1;
				}

			}
	}
#endif
	return 0;

}
#define MAX_LINEX_OBST		5
//检测侧面障碍物,是不是一条线，记录下来，作为特征物来矫正
void side_obst_line(void)
{
	log_printf("[side_obst_line]wlen=%d,olen=%d\r\n",navigat->walk_len,navigat->c_side_obst);
	if(navigat->c_side_obst < MAX_LINEX_OBST)			//若侧面障碍点小于5 直接返回
		return ;
	int16_t y_dir = navigat->side == LEFT_SIDE?1:-1;
	int16_t bx=0,by=0,ex=0;
	uint8_t sta;
	int16_t x,y;
	int16_t i;
	for(i=0;i<navigat->walk_len;i++)
	{
		x = navigat->x_walk[i];
		y = navigat->y_walk[i];
		sta = point_sta(x,y+y_dir);
		//扫描出第一个
		if(sta == OBST_STA)
		{
			if(bx ==0)
			{
				ex = bx = x;
				by = y;
			}
		}
		//扫描到头了
		if(ex !=0 )	//扫描出障碍物了
		{
			if(sta != OBST_STA || by != y)
			{
				ex = x;
				break;
			}
		}
	}
	log_printf("check sideobst(%d,%d,)->(%d,%d,)\r\n",bx,by,ex,by);
	if(disxy(bx,ex) >=MAX_LINEX_OBST)
	{
		if(navigat->c_linex < MAX_LINEX)
		{
			LINEX  *linex = &navigat->linex[navigat->c_linex++];
			linex->tx = (bx+ex) >> 1;
			linex->ty = by;
			linex->dir = y_dir;			//障碍物的方向
			log_printf("[side_obst_line]insert,len=%d,piont=(%d,%d,)\r\n",linex->tx,linex->ty,linex->dir);
		}
	}

}




//micony20190306	沿边画地图函数
/************************************************************************************
 * 函数名称: go_edgeways_for_map
 * 功    能: 工字清扫的时候，先沿边画个地图
 * 入    参: n_sta - 沿边方向
 *			 maxdist, X,Y轴移动的最大的差值，超过这个距离就停止了。
 * 出    参:
 * 说    明: 工字清扫走到底，如果是红外碰撞，且没有沿过边，则调用该函数沿边画地图。
 *************************************************************************************/
uint8_t go_edgeways_for_map(uint8_t n_sta ,int16_t maxdist,int begin_deg )
{
#if EDGE_DRAW_MAP
	int calue=0;
	uint8_t sta;
	int16_t llx=0,lly=0;
	int16_t bx = X_NOW,by = Y_NOW;					//记录初始的坐标，用于计算沿边到位了。

	float lagle;						//记录最后的延边角度，用于计算是否转弯过多
	uint32_t	t_begin;				//开始的时间
	int ret_calc=1;
	int16_t c_lost=0;
	uint16_t c_round=0;		//转圈的次数
	int c_near_wall = 0;
    int16_t tox;
    int16_t toy;

//short x_begin_line=0,y_begin_line = 0;		//一条线开始的X，Y的坐标

	int16_t dist_y = dis_xy(Y_NOW,toy)+2;

	NEAR_WALL *near = &navigat->near;

	float m_angle[MAX_C_M_ANGLE];			//保存最近的20个碰撞的角度
	int16_t c_m_angle=0;

#if LAGER_ROUND_MODE
	int16_t cyc_x[MAX_C_M_ANGLE],cyc_y[MAX_C_M_ANGLE];
	uint8_t tmp=0;
#else
	int16_t c_near_wall1 = 0;
#endif

	for(c_m_angle = 0;c_m_angle<MAX_C_M_ANGLE;c_m_angle++)
		m_angle[c_m_angle] = 0;
	c_m_angle = 0;


//	motor_run(GO_ROUND,600,0,0);
	CHECK_NAVI_STA_RT(0);

	navigat->out =navigat->angle;
	navigat->distance = 0;
	motor.c_left_hw = motor.c_right_hw = 0;
	cord_calc_store(0);
	gyro_whlmap();
#if JUDGE_PID_CTRL_WHEEL_STOP
	m_speed_pid_ctrl(0,0,0,0,0,0,0);		//记录左轮霍尔数
#else
	m_speed_pid_ctrl(0,0);
#endif

	pd_gyro_int(GO_NEAR_PWM_FAST);			//初始化角度环PID
	init_near_wall_navi(n_sta);				//初始化沿边PID

	motor_run(GO_FORWARD, GO_NEAR_PWM_FAST, 0, 0);	//跑路
	navigat->c_go = navigat->c_turn = 0;       // add  2019 03 09

	navigat->is_walk =1;
	log_printf("[go_edgeways_for_map]sta=%d(%d),now=(%d,%d,)to=(%d,%d,)dis=%d\r\n",navigat->near.n_sta,n_sta,X_NOW,Y_NOW,tox,toy,dist_y);
	//navigat->wheel_dir = 1;
	sta = sys->sState;
	lagle = sys->angle;
	mstimeout(&t_begin,0);
	while(1)
	{

		proc_uart_task();
		get_sensers(&sys->gSta);
		if(sys->sState !=sta)
			return RET_NEAR_ERROR;
		CHECK_NAVI_STA_RT(0);
		//碰撞
		if(sys->gSta & (MASK_BUM_MIDL) ||(sys->gSta & (MASK_BUM_LEFT | MASK_BUM_LEFT2 | MASK_BUM_RIGHT | MASK_BUM_RIGHT2))||  ret_calc==0 ||  c_lost >=MAX_NEAR_LOST)
		{
//			gSta = sys->gSta;	//记录碰撞的状态
			motor_run(GO_STOP,0,0,0);
			//log_printf("BUM\r\n");
			coordinate_calcu(0);														//计算出原始的坐标系
			coord_org2map(navigat->x_org,navigat->y_org,&navigat->tx,&navigat->ty); //坐标系转换
			motor_run(GO_STOP,0,0,0);

			log_printf("\r\n-----go_edgeways_for_map bum(%d,%d,%d,%f,%f),gsta=%d,irda=(%d,%d,%d,%d)angle=%3.1f,ret=%d,lost=%d\r\n",navigat->tx,navigat->ty,motor.c_left_hw,navigat->x_org_f,navigat->y_org_f,
							sys->gSta,sys->g_sta[0],sys->g_sta[1],sys->g_sta[5],sys->g_sta[6],sys->angle,ret_calc,c_lost);

			side_obst_by_bum(sys->gSta);	//置障碍物
			if(!(ret_calc==0 || c_lost >=MAX_NEAR_LOST))
				c_round = 0;
			if(ret_calc==0)
					gyro_mapwhl();

			lagle = sys->angle;

			navigat->distance = 0;
			motor.c_left_hw = 0;

#if !LAGER_ROUND_MODE
			if(c_m_angle >=MAX_C_M_ANGLE)
				c_m_angle = 0;
			m_angle[c_m_angle++] = sys->angle;		//保持角度

			if(near_round_360(m_angle,c_m_angle))
			{
			/*
				log_printf("found round...go line...\r\n");

				init_near_wall_navi(NO_SIDE_NEAR);
				for(c_m_angle = 0;c_m_angle<MAX_C_M_ANGLE;c_m_angle++)
					m_angle[c_m_angle] = 0;
				c_m_angle = 0;
				//goto l_go_edeways;
				return RET_NEAR_OK;
				*/
			}
#else////2018-07-10  未测试6
			if(near_large_round_360(cyc_x,cyc_y,m_angle,c_m_angle)) 					///绕大桩判断函数
			{
			/*
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
				return RET_NEAR_OK;
				*/
			}
#endif
			/**************************************************************************
				如果行走的路程超过了限定的路程，则停下来
			*****************************************************************************/
			if(  disxy(bx,X_NOW) >=maxdist || disxy(by,Y_NOW) >= maxdist)
			{
				log_printf("xy ok3(%d,%d,)->(%d,%d,%3.1f),\r\n",bx,by,X_NOW,Y_NOW,sys->angle);
				motor_run(GO_STOP,0,0,0);
				return RET_NEAR_OK;
			}

			if((begin_deg == 90 &&  disfloat(sys->angle,270)< 20) ||
				(begin_deg == 270 &&  disfloat(sys->angle,90)< 20))
            {
				log_printf("angle ok(%d,%3.1f)\r\n",begin_deg,sys->angle);
				motor_run(GO_STOP,0,0,0);
				return RET_NEAR_OK;
			}




			if(near->n_sta ==NO_SIDE_NEAR)
			{
				init_near_wall_navi(n_sta);
			}



			if(near->n_sta == RIGHT_SIDE_NEAR)
			{
				//modify201710	发现这个bug，导致反复碰撞。之前不知道为何如此
				/*
				if((gSta & MASK_BUM_LEFT))
				{
					log_printf("midbum,10deg\r\n");
					robot_turn_deg(GO_RIGTH,TURN_DEG_PWM,10);
				}else
				*/
				{
					robot_turn_deg(GO_LEFT,TURN_DEG_PWM,12);
				}
			}else if(near->n_sta == LEFT_SIDE_NEAR)
			{
				//log_printf("gsta=%d,left=%d,%d\r\n",sys->gSta,sys->g_sta[0],sys->g_sta[1]);
				//modify201710	发现这个bug，导致反复碰撞。之前不知道为何如此
				/*
				if((gSta & MASK_BUM_RIGHT)  )
				{
					log_printf("midbuml,10deg\r\n");
					robot_turn_deg(GO_LEFT,TURN_DEG_PWM,10);
				}else
				*/
				{
						//log_printf("midbuml,12deg\r\n");
					robot_turn_deg(GO_RIGTH,TURN_DEG_PWM,12);
				}

			}else
			{
				log_printf("RET_NEAR_ERROR,nsta=%d\r\n",n_sta);
				return RET_NEAR_ERROR;
			}
//l_go_edeways1:
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
			//gyro_whlmap();

			motor_run(GO_FORWARD, GO_NEAR_PWM_FAST, 0, 0);
			motor.c_left_hw = motor.c_right_hw = 0;
#if JUDGE_PID_CTRL_WHEEL_STOP
	m_speed_pid_ctrl(0,0,0,0,0,0,0);		//记录左轮霍尔数
#else
			m_speed_pid_ctrl(0,0);
#endif
			c_near_wall = 0;
			navigat->near.pid->c_lost = 0;
			//navigat->near.pid->c_lost = 0;
			navigat->near.pid->c_lost_flag = 0;

			ret_calc = 1;

#if !LAGER_ROUND_MODE
			c_near_wall1 = 0;
#endif
		}

		if(TIM5->CNT >=5000)
		{
			TIM5->CNT = 0;
			www_idleintel_com();

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
#if !CALE_BY_FRON
				ret_calc = coordinate_calcu(0);														//计算出原始的坐标系
#endif
				//ret_calc = coordinate_calcu();														//计算出原始的坐标系
				coord_org2map(navigat->x_org,navigat->y_org,&navigat->tx,&navigat->ty);	//坐标系转换

#if !LAGER_ROUND_MODE
				if(c_near_wall1 ++ >=30)
				{

					for(c_m_angle = 0;c_m_angle<MAX_C_M_ANGLE;c_m_angle++)
						m_angle[c_m_angle] = 0;
					c_m_angle = 0;
					c_near_wall1 = 0;
					log_printf("log near\r\n");
				}
#endif
				//延边数据
				if(LEFT_ADC() > cfg->lock_left_adc || RIGHT_ADC() > cfg->lock_right_adc)
					c_near_wall ++ ;
#if !JUDGE_PID_CTRL_WHEEL_STOP
				if(c_near_wall >=20 && dis_xy(L_FORWRK_PWM,R_FORWRK_PWM) < 100)
				{
					m_speed_pid_ctrl(0,1);			///
				}
#endif
				c_near_wall = 0;

				if( *(navigat->near.pid->adc) > navigat->near.pid->min_adc) //延边
				{
					//c_near++;
					c_round = 0;
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
						如果行走的路程超过了限定的路程，则停下来
					*****************************************************************************/
					if(  disxy(bx,X_NOW) >=maxdist || disxy(by,Y_NOW) >= maxdist)
					{
						log_printf("xy ok4(%d,%d,)->(%d,%d,%3.1f),\r\n",bx,by,X_NOW,Y_NOW,sys->angle);
						motor_run(GO_STOP,0,0,0);
						return RET_NEAR_OK;
					}
					/*
					 if ((disxy(begin_deg,navigat->lst_angle))<=20)

			        {
						log_printf("angle ok(%d,%d)\r\n",begin_deg,navigat->lst_angle);
						motor_run(GO_STOP,0,0,0);
						return RET_NEAR_OK;
					}
					*/
					if((begin_deg == 90 &&  disfloat(sys->angle,270)< 20) ||
						(begin_deg == 270 &&  disfloat(sys->angle,90)< 20))
		            {
						log_printf("angle ok(%d,%3.1f)\r\n",begin_deg,sys->angle);
						motor_run(GO_STOP,0,0,0);
						return RET_NEAR_OK;
					}


					side_obst_by_ir();		//设置障碍物

			}
		}
	}
#else
	return 0;
#endif
}

