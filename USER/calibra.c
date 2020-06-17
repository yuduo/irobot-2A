#include "sys.h"



/*
校准地图

*/

//中间有没有障碍物，没有未清扫区域，有效的一条线
uint8_t line_valid(int16_t bx,int16_t by,int16_t ex)
{
	int16_t bx1,ex1,x,c=0;
	
	bx1 = bx < ex?bx:ex;
	ex1 = bx < ex?ex:bx;	
	
	//中间有障碍物，或未清扫的，就是无效的。
	/*
	发现有时候有误差导致顶端的两个总是有未清扫，或障碍物，导致没有矫正，
	试着把顶端的判断去掉看看。
	*/
	for(x = bx1+1; x<ex1;x++)
	{
		if(OBST_XY(x,by) )
		{
			log_printf("line_valid:(%d,%d,)\r\n",x,by);
		//	if(x == bx1 || x == ex1
			return 0;
		}
		if(clean_sta(x,by)==0)
		{
			log_printf("line_valid_no_clean:(%d,%d,)\r\n",x,by);
			c++;
		}
		if(c >=2)
			return 0;
	}
	return 1;
}

uint8_t ajust_ofset_y(int16_t x,int16_t y,int16_t bx1,int16_t ex1,uint8_t check_len,int16_t walk_len,int16_t x_dir)
{
	if(OBST_XY(x,y) )			//是障碍物
	{
		
		log_printf("[ajust_ofset_y]found obst=(%d,%d,),(%d,%d,)->(%d,%d,),check=%d,len=%d,dir=%d\r\n",x,y,bx1,y,ex1,y
					,check_len,walk_len,x_dir);
		if(line_valid(bx1,y,ex1)==0) //中间没有障碍物
		{
			log_printf("not vline\r\n");
			return 0;
		}
		log_printf("line ok");
		if(check_len)		//要校验长度
		{
			int16_t len;
			len = get_linex_len(x-x_dir,y);
			log_printf("check len(%d,%d,)len=%d,walk=%d\r\n",x-x_dir,y,len,walk_len);
			if(dis_xy(len,walk_len) <=2)
			{
				log_printf("len ok ,ajust\r\n");
				ajust_xy_map(X_NOW,y,AJUST_Y);
				return 1;
			}
		}else
		{
			ajust_xy_map(X_NOW,y,AJUST_Y);
			return 1;
		}

	}
	return 0;

}
/*
移动X轴看是否合理
一定要
*/
uint8_t ajust_ofset_x(int16_t bx,int16_t by,int16_t ex,int16_t x_ofs,int16_t walk_len)
{
	int16_t bx1,ex1,len;
	//计算出偏移量
	bx1 = bx+x_ofs;
	ex1 = ex+x_ofs;
	log_printf("[ajust_ofset_x](%d,%d,)-(%d,%d,)x_ofs=%d,wlen=%d,ofs=(%d,%d,)\r\n",bx,by,ex,by,x_ofs,walk_len,bx1,by);
	//穿过障碍物或是中间有未清扫的，非法的线。
	if(line_valid(bx1,by,ex1)==0)
	{
		log_printf("line error\r\n");
		return 0;
	}
	//如果两端吻合，就OK
	if(OBST_XY(bx1-1,by) && OBST_XY(ex1+1,by))
	{
		log_printf("obstOK\r\n");
		return 1;
	}
	//两端都是障碍物，则，需要两端吻合
	len = get_linex_len(bx1,by);
	log_printf("peerlen=%d,my=%d\r\n",len,walk_len);
	if(walk_len > 0 && dis_xy(walk_len,len) >=2)
	{
		log_printf("len error\r\n");
		return 0;
	}
	log_printf("OK,AJUST..X\r\n");
	ajust_xy_map(X_NOW+x_ofs,Y_NOW,AJUST_X);
	return 1;
	
}
//检测穿过障碍物的状态
int16_t  check_cross_obst_sta(int16_t bx,int16_t ex,int16_t yy,int16_t x_dir,int16_t *ox,int16_t *oy)
{
	int16_t g_cross = 0;
	int16_t i = 0,x_cros,c_clr=0;
	/*
	while(1)
	{
		
		if(x_cros == ex)					//到头了
			break;
		if(SCANE_XY(x_cros,yy))		//已经清扫过的个数
			c_clr++;

		
		if(i++>=50)
		{
			log_printf("i error\r\n");
			break;
		}
		x_cros+=x_dir;
	}
	*/
	log_printf("[cross_obst_sta](%d,%d,)->(%d,%d,)xdir=%d\r\n",bx,yy,ex,yy,x_dir);
	x_cros = bx + x_dir;
	while(1)
	{
		if(x_cros == ex)					//到头了
			break;
		if(SCANE_XY(x_cros,yy))		//已经清扫过的个数
			c_clr++;

		if(OBST_XY(x_cros,yy))		//障碍物点
		{
			log_printf("cross obst(%d,%d,)\r\n",x_cros,yy);
			g_cross = 1;
			//判断是怎样穿越的。如果前方还是清扫区域，且不是穿过一套县
			if(obst_line_90_270_light(x_cros,yy,1,1)) //穿过了一条线
			{	
				log_printf("cross line\r\n");
				g_cross = 2;
			}
			*ox = x_cros;
			*oy = yy;
			break;
		}
		
		if(i++>=50)
		{
			log_printf("i error\r\n");
			break;
		}
		x_cros+=x_dir;
	}

	if(c_clr <=2 && g_cross == 0)
	{
		log_printf("few clean,not cross,clr=%d\r\n",c_clr);
		return -1;
	}
	return g_cross;
	/*
	//穿过障碍物，计算是不是到了未清扫的区域。
	if(g_cross)
	{
		x_cros+=x_dir;
		c = i= 0;
		while(1)
		{
			if(x_cros == ex)					//到头了
				break;
			if(clean_sta(x_cros,yy) ==0)
				c++;
			if(i++>=10)
			{
				log_printf("i2 error\r\n");
				break;
			}
				
		}
		if( c>=3 && clean_sta(x_cros,yy) ==0 && c_clr >3)
		{
			log_printf("cross to unclr\r\n");
			g_cross = 3;
		}
	}
	
	return g_cross;*/
}


//根据两边的障碍物来判定，先去那边清扫。那边离清扫点或障碍物最近，就先去那边
int get_navi_deg(int16_t tx,int16_t ty,int16_t *ox)
{
	int16_t x1,x2,i;
	x1 =tx-1;
	x2 =tx+1;

	for(i=0;i<10;i++)
	{
		if(point_sta(x1,ty) != UNCLN_STA)
		{
			log_printf("found obstdir-(%d,%d,)\r\n",x1,ty);
			*ox = x1-1;		//允许多走一格	
			return 180;
		}
		if(point_sta(x2,ty) != UNCLN_STA)
		{
			log_printf("found obstdir+(%d,%d,)\r\n",x1,ty);
			*ox = x2+1;		//允许多走一格	
			return 0;

		}
		x1--;
		x2++;
	}
	*ox = tx+3;
	return 0;
	
}

/*
导航到未清扫点的走法，走一下，估算一下位置
*/
int unclr_piont_walk(uint8_t old_side)
{
	char  ret;
	int16_t y_dir = navigat->side == LEFT_SIDE?1:-1;
	int16_t ox;
	int deg;
	uint8_t side = 0;


	deg =get_navi_deg(X_NOW,Y_NOW+y_dir,&ox);
	log_printf("[unclr_piont_walk]angle=%d,side=%d\r\n",deg,navigat->side);
#if !FORCE_SIDE_NAVI
	//校验方向
	uint8_t sta_left,sta_right;
	sta_left = point_sta(X_NOW,Y_NOW+1);
	sta_right = point_sta(X_NOW,Y_NOW-1);
	if(sta_left == UNCLN_STA && sta_right !=UNCLN_STA)
		side = LEFT_SIDE;
	else if(sta_left != UNCLN_STA && sta_right == UNCLN_STA)
		side = RIGHT_SIDE;
	log_printf("sta=%d,%d,side=%d\r\n",sta_left,sta_right,side);
	if(side != 0)
	{
		navigat->side = side;
		log_printf("set side =%d\r\n",navigat->side);
		
	}
		
#endif
	
	//导航到点了
	//if(X_NOW == tox && Y_NOW == toy)
	{
		//校验方向
		if(navigat->side == LEFT_SIDE)
		{
			ret = walk_map_cord(navigat->tx,navigat->ty+2,NO_ANGLE_V,0);
			motor_go_forwark(100,NO_SIDE_NEAR,NULL);
		}
		else
		{
			ret =walk_map_cord(navigat->tx,navigat->ty-2,NO_ANGLE_V,0);
			//从充电座出来，且刚转到右边，多走半格，避免装上充电桩
			if(old_side == LEFT_SIDE && navigat->from_charge && dis_xy(Y_NOW,navigat->dock_y) < 3)
			{
				motor_go_forwark(200,NO_SIDE_NEAR,NULL);
			}
		}

		
	}
	log_printf("[unclr_piont_walk]walk ret=%d,gsta=%d\r\n",ret,sys->gsta);
	//走进去了，地图矫正
	if(ret >0)
	{

	}
/*
	//碰撞了，延边进去一下
	if(ret ==0)
	{
		ret = nearwall_cross_map(navigat->y_org+400*y_dir,angle,0,20);	//延边
	}
	*/
	navigat->m_navi_run = 2;	//导航到位第一次运行，需要校准。
	log_printf("[unclr_piont_walk]turn to=%d,and walk to(%d,%d,)\r\n",deg,ox,Y_NOW);
	turn_to_deg(deg);
	//回头走几步，然后掉头，如果这个不走的话，会导致总是有越来越多的末端未清扫点。
	walk_map_cord(ox,Y_NOW,NO_ANGLE_V,0);
	log_printf("[unclr_piont_walk]walk ok(%d,%d,)turn back\r\n",X_NOW,Y_NOW);
	//掉头走回去
	if(deg == 0)
		turn_to_deg(180);
	else
		turn_to_deg(0);

	return 1;
}
/*
校准地图
0 - 不动
1 - 动了。
*/
int16_t calibrat_map(int16_t tx,int16_t ty)
{
	int16_t ny=0,nx=0;

	int16_t xret=0;
	int16_t xx1,xx2 ,yy1,yy2,i,ox,oy;
//	int16_t x1=0;
	uint8_t adj = 0;
	uint8_t dir1=1,dir2=1;
	int16_t ret=0;
	//=========================================找一条Y轴的线，来矫正X轴
	
	xx1 = tx;
	yy1 = yy2 = ty;
	ny = 0;
	dir1=dir2=1;
	log_printf("[calibrat_map](%d,%d,)\r\n",tx,ty);
	//找到可以一延边过去的点。
	for(i = 0;i<8;i++)
	{
		yy2--;
		//=============================往正方向走==============================
		if(OBST_XY(xx1+1,yy1) || OBST_XY(xx1-1,yy1) || OBST_XY(xx1,yy1))		//这条线不够开阔
		{
			dir1 = 0;										//不开阔的线，不能走了
		}
		if(dir1)
		{
			xret =obst_line_on_90_270(xx1,yy1,12,&ox,&oy);
			if(xret ==2 || xret==-2)   //如果是一条严格的线，就直接退出了。
			{
				ny = yy1;
				log_printf("found liney=(%d,%d,)\r\n",xx1,yy1);
				break;
			}
			
		}
		
		//================================往负方向走====================================
		if(OBST_XY(xx1+1,yy2) || OBST_XY(xx1-1,yy2)||OBST_XY(xx1,yy2))		//这条线不够开阔
		{
			dir2 = 0;										//不开阔的线，不能走了
		}
		if( dir2)
		{
			xret =obst_line_on_90_270(xx1,yy2,12,&ox,&oy);
			if(xret == 2 || xret == -2)   //如果是一条严格的线，就直接退出了。
			{
				ny = yy2;
				log_printf("found liney2=(%d,%d,)\r\n",xx1,yy2);
				break;
			}		
			
		}
		
		yy1 ++;
	}
	//===有一条直线，能走过去==========
	if(ny > 0)
	{
		adj = walk_adj_x(tx,ny,xret,ox,oy);
	
		if(adj == AJUST_X)		//只矫正X轴，则看看能否矫正Y轴
		{
			navigat->c_v_go = 0;
			//if(ajust_y_from_x(X_NOW,Y_NOW) == AJUST_Y)  发现这样矫正，有风险
 			//	return 1;
 			ret = 1;
		}
		if(adj >0)
			ret = 1;
		else
			ret = 0;
		/*
		if(adj == (AJUST_X | AJUST_Y))
		{
			log_printf("[calibrat_map]ajust xy ok1\r\n");
			return 1;
		}
		*/
	}

		
	//=====================查找一条X轴的线，用来矫正Y轴。=================================
	log_printf("[calibrat_map]Analy X\r\n");
	xx2 = xx1 = tx;
	yy1 = ty;
	nx = 0;
	//找到可以一延边过去的点。
	for(i = 0;i<8;i++)
	{
		xx2--;

		//=============================往正方向走==============================
		if(OBST_XY(xx1,yy1+1) || OBST_XY(xx1,yy1-1) || OBST_XY(xx1,yy1) || clean_sta(xx1,yy1) ==0)		//这条线不够开阔
		{
			dir1 = 0;										//不开阔的线，不能走了
		}
		if( dir1)
		{
			xret =obst_line_on_0_180(xx1,yy1,12,&ox,&oy);
			if(xret ==2 || xret==-2)   //如果是一条严格的线，就直接退出了。
			{
				nx = xx1;
				log_printf("found linex=(%d,%d,)\r\n",xx1,yy1);
				break;
			}
		}
		
		//================================往负方向走====================================
		if(OBST_XY(xx2,yy1+1) || OBST_XY(xx2,yy1-1) || OBST_XY(xx2,yy1) || clean_sta(xx2,yy1) ==0)		//这条线不够开阔
		{
			dir2 = 0;										//不开阔的线，不能走了
		}
		if( dir2)
		{
			xret =obst_line_on_0_180(xx2,yy1,12,&ox,&oy);
			if(xret == 2 || xret == -2)   //如果是一条严格的线，就直接退出了。
			{
				nx = xx2;
				log_printf("found linex2=(%d,%d,)\r\n",xx2,yy1);
				break;
			}	
		}
		
		xx1 ++;
	}
	//===有一条直线，能走过去==========
	if(nx > 0)
	{
		adj = walk_adj_y(nx,ty,xret,ox,oy);
		if(adj > 0)
			return 1;
		else
			return 0;

	}
	return ret;
}
#define MAX_WAL_HW_ADJ		(10*HW_GRID)
/*
走到点位，然后冲向一条直线去矫正X轴
*/
uint8_t walk_adj_y(int16_t tox,int16_t toy,int16_t dir,int16_t ox,int16_t oy)
{
	char ret;
	int16_t y_dir=0;
	int bdeg;		//回来的角度，如果冲过去，没有碰撞，则按照这个角度回来。
	log_printf("[walk_adj_y](%d,%d,)->(%d,%d,%d),(%d,%d,)\r\n",X_NOW,Y_NOW,tox,toy,dir,ox,oy);
	//只有直线，才走过去。
	if(dir !=2 && dir !=-2)
	{
		log_printf("[walk_adj_y]err dir(%d)\r\n",dir);
		return 0;
	}
	//Y轴不动
	if(toy != Y_NOW)
	{
		log_printf("[walk_adj_y]toy error\r\n");
		return 0;
	}
	//========第一步，不是本点，需要移动一下，调用 walk_map_cord_only 走到点。==============
	if(tox != X_NOW )
	{
		ret = walk_map_cord_only(tox,toy,1000,10);
		//其实走失败了，是不是考虑也矫正一下?
		if(ret ==0)
		{
			log_printf("[walk_adj_y]walk bum error\r\n");
			return AJUST_MOVE;
		}
	}

	//Y+方向
	if(dir ==2)
	{
		y_dir = -1;
		log_printf("[walk_adj_y]turn to 90\r\n");
		turn_to_deg(90);
		bdeg = 270;
	}else
	{
		y_dir = 1;
		log_printf("[walk_adj_y]turn to 270\r\n");
		turn_to_deg(270);
		bdeg = 90;
	}
	//=========== 第二步，向直线冲过去==================
	ret = motor_go_forwark_only(MAX_WAL_HW_ADJ,NO_SIDE_NEAR);
	//碰撞了，则计算矫正
	if(ret ==0 && sys->gsta !=0)
	{
//		uint8_t c_ret=0;
		log_printf("[walk_adj_y]bum ajd x\r\n");
		//这里就这样子矫正X轴，其实，还是有问题的，
		if(IR_MIDLE_BUM())		//中间碰撞则矫正回来
		{
			int16_t y;
			y = oy + y_dir;
			log_printf("[walk_adj_y]mid bum ajust y\r\n");
			ajust_xy_map(X_NOW,y,AJUST_Y);
			navigat->c_v_go = 0;
		}else		//不是中间的碰撞，则不矫正，其实是有时候也可以矫正的。后边考虑进一步的优化
		{
			log_printf("[walk_adj_y]not mid bum(%d,%d,%d)\r\n",sys->g_sta[2],sys->g_sta[3],sys->g_sta[4]);
			return AJUST_MOVE;
		}
		return AJUST_Y;
		/*
		//判断是不是T字形，或一条断头的线
		c_ret = check_line_x_sta(ox,oy,5,dir);
		log_printf("[walk_adj_y]ret=%d\r\n",c_ret);
		
		if(c_ret == LINE_X_STA_T_LEFT || c_ret == LINE_X_STA_L_LEFT)
		{
			turn_to_deg_v(180);
		}else if(c_ret == LINE_X_STA_L_RIGHT || c_ret == LINE_X_STA_T_RIGHT)
		{
			turn_to_deg_v(0);
		}else
		{
			log_printf("cret error\r\n");
			return AJUST_Y;
		}	
		*/
	}else
	{
		log_printf("[walk_adj_y]not bum turn to %d and back\r\n",bdeg);
		turn_to_deg(bdeg);
		motor_go_forwark_only(MAX_WAL_HW_ADJ,NO_SIDE_NEAR);
	}
	return AJUST_MOVE;
}

uint8_t walk_adj_x(int16_t tox,int16_t toy,int16_t dir,int16_t ox,int16_t oy)
{
	char ret;
	int16_t x_dir=0;
//	int bdeg;

	log_printf("[walk_adj_x](%d,%d,)->(%d,%d,%d),(%d,%d,)\r\n",X_NOW,Y_NOW,tox,toy,dir,ox,oy);
	//只有直线，才走过去。
	if(dir !=2 && dir !=-2)
	{
		log_printf("[walk_adj_x]err dir(%d)\r\n",dir);
		return 0;
	}
	//Y轴不动
	if(tox != X_NOW)
	{
		log_printf("[walk_adj_x]tox error\r\n");
		return 0;
	}
	//========第一步，不是本点，需要移动一下，调用 walk_map_cord_only 走到点。==============
	if(toy != Y_NOW )
	{
		ret = walk_map_cord_only(tox,toy,1000,10);
		//其实走失败了，是不是考虑也矫正一下?
		if(ret ==0)
		{
			log_printf("[walk_adj_x]walk bum error\r\n");
			return AJUST_MOVE;		//移动了
		}
	}

	//Y+方向
	if(dir ==2)
	{
		x_dir = -1;
		log_printf("[walk_adj_x]turn to 90\r\n");
		turn_to_deg(0);
//		bdeg = 180;
	}else
	{
		x_dir = 1;
		log_printf("[walk_adj_x]turn to 270\r\n");
		turn_to_deg(180);
	//	bdeg =  0;
	}
	//=========== 第二步，向直线冲过去==================
	ret = motor_go_forwark_only(MAX_WAL_HW_ADJ,NO_SIDE_NEAR);
	//碰撞了，则计算矫正
	if(ret ==0 && sys->gsta !=0)
	{
//		uint8_t c_ret=0;
		log_printf("[walk_adj_x]bum ajd x\r\n");
		//这里就这样子矫正X轴，其实，还是有问题的，
		if(IR_MIDLE_BUM())		//中间碰撞则矫正回来
		{
			int16_t x;
			x = ox + x_dir;
			log_printf("[walk_adj_x]mid bum ajust X(%d,%d,)->(%d,%d,)\r\n",X_NOW,Y_NOW,x,Y_NOW);
			if(dis_xy(x,X_NOW) <=3)
			{
				ajust_xy_map(x,Y_NOW,AJUST_X);
				navigat->c_v_go = 0;
			}else
			{
				log_printf("big dist no adjX \r\n");
				
			}
		}else		//不是中间的碰撞，则不矫正，其实是有时候也可以矫正的。后边考虑进一步的优化
		{
			log_printf("[walk_adj_x]not mid bum(%d,%d,%d)\r\n",sys->g_sta[2],sys->g_sta[3],sys->g_sta[4]);
			return AJUST_MOVE;
		}
		return AJUST_X;
		/*
		//判断是不是T字形，或一条断头的线
		c_ret = check_line_x_sta(ox,oy,5,dir);
		log_printf("[walk_adj_x]ret=%d\r\n",c_ret);
		
		if(c_ret == LINE_X_STA_T_LEFT || c_ret == LINE_X_STA_L_LEFT)
		{
			turn_to_deg_v(180);
		}else if(c_ret == LINE_X_STA_L_RIGHT || c_ret == LINE_X_STA_T_RIGHT)
		{
			turn_to_deg_v(0);
		}else
		{
			log_printf("cret error\r\n");
			return AJUST_Y;
		}	
		*/
	}
	return AJUST_MOVE;
}

/*
检测一条线，是断头还是T字形
*/
/*
uint8_t check_line_x_sta(int16_t ox,int16_t oy,int16_t len,int16_t y_dir)
{
	int16_t x,i;
	for(x = ox;x >= (ox - len);x--)
	{
		if(OBST_XY(x,oy+y_dir) && OBST_XY(x,oy+y_dir*2))
		{
			log_printf("[check_line_x_sta]found Tleft(%d,%d,%d)\r\n",x,oy,y_dir);	
			return LINE_X_STA_T_LEFT;
		}
		//已清扫
		if(SCANE_XY(x,oy-2) && SCANE_XY(x,oy-1) && SCANE_XY(x,oy) && SCANE_XY(x,oy+1) && SCANE_XY(x,oy+2) )
		{
			log_printf("[check_line_x_sta]found Lleft(%d,%d,%d)\r\n",x,oy,y_dir);	
			return LINE_X_STA_L_LEFT;
		}
	}
	for(x = ox;x <= (ox + len);x++)
	{
		if(OBST_XY(x,oy+y_dir) && OBST_XY(x,oy+y_dir*2))
		{
			log_printf("[check_line_x_sta]found Tright(%d,%d,%d)\r\n",x,oy,y_dir);	
			return LINE_X_STA_T_RIGHT;
		}
		//已清扫
		if(SCANE_XY(x,oy-2) && SCANE_XY(x,oy-1) && SCANE_XY(x,oy) && SCANE_XY(x,oy+1) && SCANE_XY(x,oy+2) )
		{
			log_printf("[check_line_x_sta]found Lright(%d,%d,%d)\r\n",x,oy,y_dir);	
			return LINE_X_STA_L_RIGHT;
		}
	}
	return LINE_X_STA_NO;
}

uint8_t check_line_y_sta(int16_t ox,int16_t oy,int16_t len,int16_t x_dir)
{
	int16_t y,i;
	for(y = oy;y >= (oy - len);y--)
	{
		if(OBST_XY(ox + x_dir,y) && OBST_XY(ox +x_dir *2,y))
		{
			log_printf("[check_line_x_sta]found Tlow(%d,%d,%d)\r\n",ox,y,x_dir);	
			return LINE_X_STA_T_LOW;
		}
		//已清扫
		if(SCANE_XY(ox-2,y) && SCANE_XY(ox-1,y) && SCANE_XY(ox,y) && SCANE_XY(ox+1,y) && SCANE_XY(ox+2,y) )
		{
			log_printf("[check_line_x_sta]found Llow(%d,%d,%d)\r\n",ox,y,x_dir);	
			return LINE_X_STA_L_LOW;
		}
	}
	for(y = oy;y <= (oy + len);y++)
	{
		if(OBST_XY(ox + x_dir,y) && OBST_XY(ox +x_dir *2,y))
		{
			log_printf("[check_line_x_sta]found Thight(%d,%d,%d)\r\n",ox,y,x_dir);	
			return LINE_X_STA_T_RIGHT;
		}
		//已清扫
		if(SCANE_XY(ox-2,y) && SCANE_XY(ox-1,y) && SCANE_XY(ox,y) && SCANE_XY(ox+1,y) && SCANE_XY(ox+2,y) )
		{
			log_printf("[check_line_x_sta]found Lhight(%d,%d,%d)\r\n",ox,y,x_dir);	
			return LINE_X_STA_L_HIGHT;
		}
	}
	return LINE_X_STA_NO;
}
*/
#define MAX_Y_LEN		5
/*
在校准了X轴的前提下，矫正Y轴。


uint8_t ajust_y_from_x(int16_t tx,int16_t ty)
{
	int16_t x1,x2,y;
	uint8_t dir1=1,dir2=1;
	int16_t c=0;
	int16_t tox=0,ox=0,oy=0;
	//找出X轴可去的方向
	x1 = tx;
	x2 = tx -1;

	//X,的正负方向走，
	while(1)
	{
		if(c++ >= 10)
		{
			log_printf("[ajust_y_from_x]c over\r\n");
			return 0;
		}
		//==============Y的正负方向走，找到障碍物===================
		//===往Y+方向走===
		if(OBST_XY(x1,ty) || OBST_XY(x1,ty+1) || OBST_XY(x1,ty-1))	//发现障碍物了。不管是这条线，还是侧面的线，都是狭长的线，都不适合过去
		{
			dir1 = 0;
		}
		if(dir1)
		{
			for(y = ty;y<=(MAX_Y_LEN+ty);y++)
			{
				if(OBST_XY(x1,y))		//找到障碍物，则退出
				{

					log_printf("[ajust_y_from_x]found obst(%d,%d,)\r\n",x1,y);
					tox = x1;
					ox 	= x1;
					oy	= y;
					break;
				}
				if(OBST_XY(x1+1,y) || OBST_XY(x1-1,y))		//如果侧面是障碍物，就是不开阔的线，则也退出，不走了。
				{
					log_printf("[ajust_y_from_x]found side obst(%d,%d,)\r\n",x1,y);
					break;
				}
			}
		}
		//找到要去矫正的点了。则退出，去矫正
		if(tox >0 && ox >0 && oy > 0)
		{
			log_printf("FoundA\r\n");
			break;
		}
		//===往Y负方向走===
		if(OBST_XY(x2,ty) || OBST_XY(x2,ty+1) || OBST_XY(x2,ty-1))	//发现障碍物了。不管是这条线，还是侧面的线，都是狭长的线，都不适合过去
		{
			dir2 = 0;
		}
		if(dir2)
		{
			for(y = ty-1;y >= (ty - MAX_Y_LEN);y--)
			{
				if(OBST_XY(x2,y))		//找到障碍物，则退出
				{

					log_printf("[ajust_y_from_x]found obst2(%d,%d,)\r\n",x2,y);
					tox = x2;
					ox 	= x2;
					oy	= y;
					break;
				}
				if(OBST_XY(x2+1,y) || OBST_XY(x2-1,y))		//如果侧面是障碍物，就是不开阔的线，则也退出，不走了。
				{
					log_printf("[ajust_y_from_x]found side obst(%d,%d,)\r\n",x2,y);
					break;
				}
			}
		}
		//找到要去矫正的点了。则退出，去矫正
		if(tox >0 && ox >0 && oy > 0)
		{
			log_printf("FoundA\r\n");
			break;
		}
		x1++;
		x2--;
	}

	if(tox >0 && ox >0 && oy > 0)
	{
		int16_t y_dir;
		int dist;
		log_printf("[ajust_y_from_x]found adj(%d,%d,)->(%d,%d,)->(%d,%d)\r\n",X_NOW,Y_NOW,tox,Y_NOW,ox,oy);
		if(tox != tx)		//需要走一下，
		{
			if(walk_map_cord_only(tox,ty,1000,10) == 0)
			{
				log_printf("bumA\r\n ");
				return 0;
			}
		}
		if(ox != X_NOW || oy == Y_NOW)
		{
			log_printf("Xerr\r\n");
			return 0;
		}
		if(oy > Y_NOW)
		{
			dist = (oy - Y_NOW + 5) * HW_GRID;
			turn_to_deg(90);
			y_dir = -1;
		}else
		{
			dist = ( Y_NOW - oy + 5) * HW_GRID;
			turn_to_deg(270);
			y_dir = 1;
		}
		//碰撞了，则矫正
		if(motor_go_forwark_only(dist,NO_SIDE_NEAR) == 0)
		{
			log_printf("[ajust_y_from_x]bum,now=(%d,%d,),targe=(%d,%d,)ajust\r\n",X_NOW,Y_NOW,X_NOW,oy + y_dir);
			ajust_xy_map(X_NOW,oy+y_dir,AJUST_Y);
			return AJUST_Y;
		}else
			return 0;
		
		

	}
	return 0;
}
*/

int16_t map_edge_x(int16_t xx,int16_t yy,int16_t x_dir,int16_t y_dir,int16_t xlen,int16_t *ox,int16_t *oy,uint8_t *ret,uint8_t *clrsta)
{
	int16_t i,x,y,nx;
//	int16_t oox,ooy;
//	int16_t flat = 0;		//计算平坦 系数
	int16_t lenght=0;		//长度。
	uint8_t csta[8];
//	uint8_t isOK=0;
	*ret = 0;
	*clrsta = 0;
	if(SCANE_XY(xx,yy) ==0)
	{
		log_printf("[map_edge]not a scan(%d,%d,)\r\n",xx,yy);
		return  0 ;
	}
	//往X轴的正负方向走，找到本条线的边界
	x = xx;
	y = yy;
	for(i=0;i<xlen;i++)
	{
		if(clean_sta(x,y) == 0 || OBST_XY(x,y)==1)
		{
			//log_printf("[map_edge]found obst(%d,%d,)\r\n",x,y);
			break;
		}
		x+=x_dir;
		
	}
	if(i >=xlen)
	{
		log_printf("[map_edge]not found obst\r\n");
		return 0 ;
	}
	//沿着该条线的边界，依次找出地图的的边界  
	while(1)
	{
		y+=y_dir;
		lenght++;

		nx = 0;
		
		//连续取7个点//csta[3]是中间的点
		for(i=0;i<7;i++)
			csta[i] = point_sta(x-3+i,y);		//
		//=============有三种情况，可以继续=============
		if(  (/*csta[3] == UNCLN_STA || */csta[3] == OBST_STA) && (csta[3-x_dir]==SCAN_STA && csta[3-x_dir*2]==SCAN_STA))  //第一种情况，最好，刚好是障碍物，且后边都是清扫点
		{
			nx = x;
		}else if((/*csta[3+x_dir] == UNCLN_STA ||*/ csta[3+x_dir] == OBST_STA) && (csta[3]==SCAN_STA && csta[3-x_dir]==SCAN_STA))//第二种情况，差一格
		{
			nx = x+x_dir;
		}else if((/*csta[3-x_dir] == UNCLN_STA || */csta[3-x_dir] == OBST_STA) && (csta[3-2*x_dir]==SCAN_STA && csta[3-2*x_dir]==SCAN_STA))
		{
			nx = x-x_dir;
		}
		if(nx >0)			//一条线，
		{
			drow_point(nx,y,3);
			continue;
		}else
		{
			*ox = x;
			*oy = y;

			//T字形
			if((csta[3] == UNCLN_STA || csta[3] == OBST_STA) && csta[3-x_dir] == OBST_STA && csta[3-x_dir*2] == OBST_STA)
			{
				*ret = MAP_LINE_T; 
				return lenght;
			}
			//一条线
			if(csta[0] == SCAN_STA && csta[1] == SCAN_STA && csta[2] == SCAN_STA && csta[3] == SCAN_STA && csta[4] == SCAN_STA && 
				csta[5] == SCAN_STA && csta[6] == SCAN_STA )
			{
				*ret = MAP_LINE_L; 
				if(clean_sta(x+x_dir,y-y_dir)==0 && clean_sta(x+x_dir*2,y-y_dir) == 0)
				{
					log_printf("found uncln(%d,%d,)\r\n",x+x_dir,y-y_dir);
					*clrsta = 1;
				}
				return lenght;
			}
			return lenght;
		}

	}
	
	
}


#if 0
int unclr_piont_walk(uint8_t old_side)
{
	char  ret;
	int16_t y_dir = navigat->side == LEFT_SIDE?1:-1;
//	uint8_t side;
	float angle = NO_ANGLE_V;		//计算出要去的的角度
	if(OBST_XY(X_NOW+1,Y_NOW))		//前方有障碍物，一会碰撞的时候，往180°走，
		angle = 180;
	if(OBST_XY(X_NOW-1,Y_NOW))		//前方有障碍物，一会碰撞的时候，往180°走，
		angle = 0;
	if(angle ==NO_ANGLE_V)
	{
		if(SCANE_XY(X_NOW-2,Y_NOW+y_dir) == 0 && SCANE_XY(X_NOW-3,Y_NOW+y_dir) == 0 )
			angle = 180;
		else 
			angle = 0;
	}
	log_printf("[unclr_piont_walk]angle=%3.1f\r\n",angle);
	//导航到点了
	//if(X_NOW == tox && Y_NOW == toy)
	{
		//校验方向
		if(navigat->side == LEFT_SIDE)
			ret = walk_map_cord(navigat->tx,navigat->ty+1,NO_ANGLE_V,0);
		else
		{
			ret =walk_map_cord(navigat->tx,navigat->ty-1,NO_ANGLE_V,0);
			//从充电座出来，且刚转到右边，多走半格，避免装上充电桩
			if(old_side == LEFT_SIDE && navigat->from_charge)
			{
				motor_go_forwark(200,NO_SIDE_NEAR,NULL);
			}
		}

		
	}
	log_printf("[unclr_piont_walk]walk ret=%d,gsta=%d\r\n",ret,sys->gsta);

	//碰撞了，延边进去一下
	if(ret ==0)
	{
		ret = nearwall_cross_map(navigat->y_org+400*y_dir,angle,0,20);	//延边
	}
	
	navigat->m_navi_run = 2;	//导航到位第一次运行，需要校准。
	if((SCANE_XY(X_NOW+1,Y_NOW)==1 && SCANE_XY(X_NOW-1,Y_NOW)==0)	|| 	//左边清扫,右边未清扫
		(clean_sta(X_NOW+1,Y_NOW)==0 && SCANE_XY(X_NOW+1,Y_NOW)==1 && SCANE_XY(X_NOW-1,Y_NOW)==0)	)
	{
		turn_to_deg(180);	
	}else
		turn_to_deg(0);
	return 1;
}
#endif
/*
运动中拟合地图


int16_t fitting_on_map(void)
{
	uint8_t adj_xy=0;
	//先看看侧面是否有障碍物。

	//0度方向
	if(disfloat(sys->angle,0) < 15)
	{	//侧面有障碍物，但没有红外
		if(OBST_XY(X_NOW,Y_NOW+1) ==1 && LEFT_ADC() < cfg->min_left_adc)
		{
			log_printf("0_dir(%d,%d,),side obst but not ir\r\n",X_NOW,Y_NOW);
			
		}
	}
	
	if(OBST_XY(X_NOW,Y_NOW-1)==1)
	{
		//有障碍物，但不贴边
		if ((navigat->angle == 0 && RIGHT_ADC() < cfg->min_right_adc) ||
		     (navigat->angle == 180 && LEFT_ADC() < cfg->min_left_adc))
		{
			log_printf("found OBST but not near..\r\n");
			return CHECK_SIDE_NOT_NEAR;
		}else
			return CHECK_SIDE_MAP_OK;
	}else if(OBST_XY(X_NOW,Y_NOW + 1) ==1)
	{
		if ((navigat->angle == 0 && LEFT_ADC() < cfg->min_left_adc) ||
			 (navigat->angle == 180 && RIGHT_ADC() < cfg->min_right_adc))
		{
			log_printf("found OBST but not near2..\r\n");
			return CHECK_SIDE_NOT_NEAR;
		}else
			return CHECK_SIDE_MAP_OK;

	}
	//有红外，侧面没有障碍物
	if(LEFT_ADC() > cfg->min_left_adc)
	{
		if((navigat->angle == 0 && OBST_XY(X_NOW,Y_NOW+1) ==0 ) || 
			(navigat->angle == 180 && OBST_XY(X_NOW,Y_NOW-1) ==0 ) 
		{
			return CHECK_SIDE_NOT_OBST;
		}
	}
	if(RIGHT_ADC() > cfg->min_right_adc)
	{
		if((navigat->angle == 0 && OBST_XY(X_NOW,Y_NOW-1) ==0 ) || 
			(navigat->angle == 180 && OBST_XY(X_NOW,Y_NOW+1) ==0 ) 
		{
			return CHECK_SIDE_NOT_OBST;
		}

	}	
}
*/

#if 0
int calibration_handle(navigat_t *navi,short bx,short by,uint16_t gsta)
{
	
	int i;
	if(gsta ==0 && !IR_MIDLE_BUM())
	{
		log_printf("[proc_walk_road]not bum,gsta=%d,ir=%d,%d,%d,walklen=%d\r\n",gsta,sys->g_sta[2],sys->g_sta[3],sys->g_sta[4],navigat->walk_len);
		return 0;
	}
	if(navi->angle == 0 || navi->angle == 180)		// X轴走动，工字型
	{
		short x;
		int x_dir;
		int obst_dir = 0;
//		int n_dir;
//		short walk_grid;
//		short cross_x = 0,cross_y = 0;
		short prev_x = 0,prev_y=0;		//预测的障碍点
		short dis;
		OBST *obst;
		
		
		//如果基本吻合，则矫正即可
		x_dir = navi->angle ==0?1:-1;
		//=====================找到预测的障碍点===================
		x = bx;		//开始点
		for(i=0;i<navigat->walk_len*2;i++)
		{
			//
			if(OBST_XY(x,navi->ty) && SCANE_XY(x - x_dir,navi->ty))
			{
				prev_x = x;
				prev_y = navi->ty;
				obst = get_obst(prev_x,prev_y);
				if(obst !=NULL)
				{
					obst_dir = obst->x > obst->tx?1:-1;
				}
				break;
			}
			x+=x_dir;
		}
		
		
		if(prev_x ==0 || prev_y ==0 || obst==NULL || obst_dir != x_dir)
		{
			log_printf("[proc_walk_road]not found obst(%d,%d,)dir=%d,%d\r\n",prev_x,prev_y,obst_dir,x_dir);
			return 0;
		}
		log_printf("[proc_walk_road]found prev obst(%d,%d,)now=(%d,%d,)\r\n",prev_x,prev_y,X_NOW,Y_NOW);
		//如果差异不大，直接使用,矫正使用
		dis = dis_xy(prev_x,X_NOW);
		//一个以内的差距，则取比较宽的条件来矫正
		if(dis <=1 && check_line(CHECK_Y_LINE,prev_x,prev_y,1)==1)
		{
			log_printf("[proc_walk_road]AJUST1,x_org=%d,%d\r\n",navi->x_org,obst->x_org);
			if(dis_xy(navi->x_org,obst->x_org) < 500)
			{
				ajust_xy_org(obst->x_org,navi->y_org,100,AJUST_X);
				return 1;
			}
		}
		if(dis <=2 &&  check_line(CHECK_Y_LINE,prev_x,prev_y,2)==1)
		{
			log_printf("[proc_walk_road]AJUST2,x_org=%d,%d\r\n",navi->x_org,obst->x_org);
			if(dis_xy(navi->x_org,obst->x_org) < HW_GRID*2)
			{
				ajust_xy_org(obst->x_org,navi->y_org,100,AJUST_X);
				return 1;
			}			
			
		}
		
		if(dis >=4 )
		{
			/*
			查询左右一格的Y轴，看看，是不是由于Y轴差异一格引起的误差；
			看看左右一个的Y轴，看看是不是在X点有障碍物，如果有，表明是Y轴偏移了。
			*/
			int ix,iy;
			short xdir[2],ydir[2]={1,-1};
			xdir[0] = x_dir;
			xdir[1] = 0;
			for(ix=0;ix<2;ix++)
			{
				for(iy=0;iy<2;iy++)
				{
					if(OBST_XY(navi->tx+xdir[ix],navi->ty+ydir[iy]) )
					{
						OBST *g_obst = get_obst(navi->tx+xdir[ix],navi->ty+ydir[iy]);
						if(g_obst !=NULL)
						{
							log_printf("[proc_walk_road]AJUST SIDE1,found obst=(%d,%d,)\r\n",navi->tx+xdir[ix],navi->ty+ydir[iy]);
							if(dis_xy(navi->x_org,g_obst->x_org) <400 && dis_xy(navi->y_org,g_obst->y_org) < 500)
							{
								ajust_xy_org(g_obst->x_org,g_obst->y_org,100,AJUST_X | AJUST_Y);
								return 1;
							}

						}
					}
				}
			}
			//一条线，则矫正一下,表明穿过了障碍物，或是提前没到，则延边去扫地图，矫正过来。
			if(check_line(CHECK_Y_LINE,prev_x,prev_y,2))
			{
				log_printf("big dist ,near wall fillting map\r\n");
		
				fitting_map_near_wall(10,8,5,5);
			}
		}
		
		return 0;
		{
/*
			差异很大，有几种可能
			1.坐标是对的，且在障碍物之前碰撞，因为引进新的障碍物，这情况不矫正，置障碍物
			2.坐标错了，由于打滑等原因，多走路，却坐标算短了，其实已经到了障碍物那里了，却认为没到
			  需要矫正到障碍物那里去
			3.X轴的坐标错了，Y轴对的，导致穿过障碍物了。
			4.X轴，Y轴都错了，码盘是对的，导致穿过障碍物，或提前到了。
			
*/
		}
		
#if 0		
		//walk_grid = bx > navi->tx?bx - navi->tx:navi->tx-bx;			//取到走过的路程
		//log_printf("[proc_walk_road]walkgrid=%d\r\n",walk_grid);
		//========================第一步，计算是否穿过障碍物 ========================
		if(navigat->is_cross_obst)
		{
			int c=0;
			
			for(i=0;i<navigat->walk_len;i++)
			{
				//是障碍物，计数障碍物的个数
				if(OBST_XY(navigat->x_walk[i],navigat->y_walk[i]))
				{
					c++;
					cross_x = navigat->x_walk[i];
					cross_y = navigat->y_walk[i];
				}
					
				
			}
			log_printf("[proc_walk_road]corss obst,c=%d\r\n",c);
			if(c >=3)   //穿越了一条障碍物了
			{
				//short x_obst,y_obst;
				OBST *obst;
				for(i=0;i<navigat->walk_len;i++)
				{
					if(OBST_XY(navigat->x_walk[i],navigat->y_walk[i]))
					{
						//取到障碍物
						obst = get_obst(navigat->x_walk[i],navigat->y_walk[i]);
						if(obst !=NULL)
						{
							short ret,ox=0,oy=0;
							ret = get_linex_obst(obst->tx,obst->ty,&ox,&oy,x_dir);
							
							log_printf("obst=(%d,%d,),r=(%d,%d,)ret=%d,endobst=(%d,%d,)\r\n",obst->x,obst->y,obst->tx,obst->ty,ret,ox,oy);
							if(ret && ox !=0)
							{
								//X轴障碍物的点，和当前点差异不大，则认为成功，矫正过来。
								if(disxy(navigat->tx , ox) < 3 )
								{
									log_printf("AJUST k1\r\n");
									ajust_xy_org(navigat->x_org,obst->y_org,100,AJUST_Y);
									return 1;
								}
							}
						}
						
					}
				}				
			}
		}
/*
		//前后有没有，有的话，矫正一下。
		for(x=navi->tx-1;x <= navi->tx+1;x++)
		{
			if(OBST_XY(x,navi->ty))
			{
				int obst_dir;
				OBST *obst = get_obst(x,navi->ty);
				if(obst!=0)
				{	
					obst_dir = obst->x > obst->tx?1:-1;
					if(obst_dir == x_dir)
					{
						log_printf("[proc_walk_road]found near obst(%d,%d,)\r\n",x,navi->ty);
						obst = get_obst(x,navi->ty);
						if(obst !=NULL)
						{
							ajust_xy_org(obst->x_org,obst->y_org,90,AJUST_X);
							return 1;
						}					

					}
				}
				
			}
		}
	*/
		//两侧找，看看哪个点最符合,穿过格
		if(cross_x > 0 && disxy(cross_x,navi->tx) >=3)
		{
			int x,y;
			//在周边的Y线找
			for(y = Y_NOW - 1;y <= (Y_NOW +1);y++)
			{
				
				for(x = X_NOW-1;x<=X_NOW+1;x++)
				{
					if(OBST_XY(x,y))
					{
						OBST *obst;
						log_printf("found side near obst(%d,%d,)\r\n",x,y);
						obst = get_obst(x,y);
						if(obst !=NULL)
						{
							int obst_dir = obst->x > obst->tx?1:-1;
							short len = get_linex_len(obst->tx,obst->ty);
							log_printf("oside=%d,%d,len=%d,\r\n",x_dir,obst_dir,len);
							//方向一致，行走的距离一致，则校验过来
							if( obst_dir == x_dir && disxy(len,navigat->walk_len) <=3)
							{
								log_printf("AJUST K4\r\n");
								ajust_xy_org(obst->x_org,obst->y_org,100,AJUST_X | AJUST_Y);
								return 0;
							}
						}
						
					}
				}
			}

		}
	#endif		
	}

	return 0;
}
#endif
