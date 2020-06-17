
/*
路径，分区清扫
*/
#include "sys.h"



ROUTER_T Router,*rout;

void init_router(void)
{
	rout = &Router;
	rout->xline_len = 0;
	rout->max_len = 0;
}

/*
检测是否重复的清扫
*/

#define MAX_REPEAT_COUNT		6
uint8_t check_repeat_walk(void)
{
	uint16_t len;
	if(rout->xline_len < MAX_REPEAT_COUNT)
		return 0;
	len = rout->xline_len;

	if(disxy(rout->xline[len-1].by,rout->xline[len-MAX_REPEAT_COUNT].by) <=1)
	{
		short i,c=0;
		//int16_t bx=rout->xline[len-1].bx,ex = rout->xline[len-1].ex;
		//int16_t ydir = 0;
		log_printf("found repeat_walk:\r\n");
		for(i=MAX_REPEAT_COUNT;i>0;i--)
		{
			log_printf("(%d,%d,)->(%d,%d,)\r\n",rout->xline[len-i].bx,rout->xline[len-i].by,rout->xline[len-i].ex,rout->xline[len-i].by);
			if(disxy(rout->xline[len-1].by,rout->xline[len-i].by) <=1)
				c++;
				/*
			//取最小的
			if(bx > rout->xline[len-i].bx)
				bx = rout->xline[len-i].bx;
			if(bx > rout->xline[len-i].ex)
				bx = rout->xline[len-i].ex;
			//取最大的
			if(ex < rout->xline[len-i].bx)
				ex = rout->xline[len-i].bx;
			if(ex < rout->xline[len-i].ex)
				ex = rout->xline[len-i].ex;	
			*/
		}
		//ydir = navigat->side == LEFT_SIDE?1:-1;
		//log_printf("c=%d,bx=%d,ex=%d,ydir=%d\r\n",c,bx,ex,ydir);
		log_printf("c=%d\r\n",c);
		if(c >=MAX_REPEAT_COUNT)
		{
		//	int16_t x;
			c = 0;
			for(i=MAX_REPEAT_COUNT;i>0;i--)
			{
				if(disxy(rout->xline[len-3].by,rout->xline[len-i].by) <=1)
					c++;
			}
	
			log_printf("found repeatwalk,c=%d\r\n",c);
			if(c >=MAX_REPEAT_COUNT)
			{
				/*
				for(x= bx;x<=ex;x++)
				{
					set_osbt(x,Y_NOW,1,1,"repeat");
				}
				*/
				return 1;
			}
		}
	}
	return 0;
	
}



XLINE_T *insert_x_line(int16_t bx,int16_t by,int16_t ex,int16_t ey)
{
	log_printf("[insert_x_line](%d,%d,)->(%d,%d,)\r\n",bx,by,ex,ey);

	//micony201710 从头来
	if(rout->xline_len >=MAX_XLINE)
	{
		rout->xline_len = 0;
	}
	
	
	//if(rout->xline_len < (MAX_XLINE-1))
	{
		XLINE_T *xline;
		int16_t nby=0;
		//计算中间是不是漏掉的。
		if(rout->xline_len > 1)
		{
			xline =&rout->xline[rout->xline_len-1];
			if(navigat->side == LEFT_SIDE&& xline->by == (by-2))		//格了一格
				nby = by -1;
			else if(navigat->side == RIGHT_SIDE&& xline->by == (by+2))	
				nby = by+1;
		}
		if(nby != 0)
		{
			xline = &rout->xline[rout->xline_len++];
			
			if(bx <= ex)
			{
				xline->bx = bx;
				xline->ex = ex;
			}else
			{
				xline->bx = ex;
				xline->ex = bx;
			}
			xline->by = nby;
			xline->clean = 0;
			log_printf("tow xline(%d,%d,)->(%d,%d,)|",xline->bx,xline->by,xline->ex,xline->by);
			xline = &rout->xline[rout->xline_len++];
			if(bx <= ex)
			{
				xline->bx = bx;
				xline->ex = ex;
			}else
			{
				xline->bx = ex;
				xline->ex = bx;
			}
			xline->by = by;
			xline->clean = 0;
			log_printf("(%d,%d,)->(%d,%d,)len=%d\r\n",xline->bx,xline->by,xline->ex,xline->by,rout->xline_len);

		}
		else
		{
			xline = &rout->xline[rout->xline_len++];
			if(bx <= ex)
			{
				xline->bx = bx;
				xline->ex = ex;
			}
			else
			{
				xline->bx = ex;
				xline->ex = bx;
			}
			xline->by = by;
			xline->idx = rout->index;
			xline->clean = 0;
			log_printf("xline(%d,%d,%d)->(%d,%d,)len=%d\r\n",xline->bx,xline->by,xline->idx,xline->ex,xline->by,rout->xline_len);
			//起点和终点的Y轴不一样，是因为延边的缘故，要把这条线加上。
			if(by !=ey)
			{
				int16_t x_dir = 0,x,i=0;
				x_dir = navigat->angle = 0? -1:1;		//反方向找。
				log_printf("by!=ey,insert this line...,x_dir=%d\r\n",x_dir);
				x = X_NOW;
				i=0;
				while(1)
				{
					if(clean_sta(x,ey) == 0 || OBST_XY(x,ey))
					{
						break;
					}
					x+=x_dir;
					if(i++ >=50)
					{
						log_printf("big error\r\n");
						break;
					}
				}
				log_printf("this begin=(%d,%d,)\r\n",x,ey);
				xline = &rout->xline[rout->xline_len++];
				xline->clean = 0;
				if(x <= ex)
				{
					xline->bx = x;
					xline->ex = ex;
				}else
				{
					xline->bx = ex;
					xline->ex = x;
				}
				xline->by = ey;
				xline->idx = rout->index;
				log_printf("xline2(%d,%d,%d)->(%d,%d,)len=%d",xline->bx,xline->by,xline->idx,xline->ex,xline->by,rout->xline_len);
				
			}
		}
		
		if(rout->max_len < rout->xline_len)
			rout->max_len = rout->xline_len;		

		//log_printf("xline(%d,%d,)-(%d,%d,)%d\r\n",xline->bx,xline->by,xline->ex,xline->by,rout->xline_len);
		//xline->ey = ey;
		return xline;
		
	}
//	return NULL;
}
/*
找到本条未清扫点，距离当前点最近的点。

*/
void x_unclean(int16_t ux,int16_t uy,int16_t *ox,int16_t *oy,uint8_t side)
{

	int16_t x_dir=0,y_dir = 0;
	uint8_t sta1,sta2;
	int16_t x,y;
	//距离不远，就不找了。
	if(dis_xy(ux,X_NOW) < 4)
	{
		log_printf("x short(%d,%d,)\r\n",ux,uy);
		*ox = ux;
		*oy = uy;
		return ;
	}

	x_dir = (X_NOW > ux)?1:-1;
	y_dir = side == LEFT_SIDE?1:-1;
	x = ux;
	y = uy;
	while(1)
	{
		sta1 = point_sta(x+x_dir,y);	//清扫点
		sta2 = point_sta(x+x_dir,y+y_dir); //未清扫点

		if(sta1 !=SCAN_STA)			//不是清扫点，则退出
		{
			log_printf("nearunclr(%d,%d,)\r\n",x,y);
			break;
		}
		if(sta2 != UNCLN_STA)
		{
			log_printf("nearunclr2(%d,%d,)\r\n",x,y);
			break;
		}
		x+=x_dir;
		
	}
	*ox = x;
	*oy = y;
	
}

#define MAX_NAVI_ERR2	4


//这条线是否未清扫。
uint8_t unclr_xline(XLINE_T *xline,int16_t *tox,int16_t *toy,uint8_t type)
{
	uint16_t x,y,c_left=0,c_right=0,c_left1=0,c_right1=0;
	int16_t maxi=0;
	GET_MAX_I();
	
	y = xline->by;
	c_left= c_right=0;		//左右未清扫的区域
	log_printf("[unclr_xline]search(%d,%d,)->(%d,%d,)side=%d,%d\r\n",xline->bx,xline->by,xline->ex,xline->by,navigat->side,xline->clean);
	if(xline->clean)
		return 0;
	for(x = xline->bx;x <= xline->ex;x++)
	{
		if(search_navi_err_table(x,y)  >= MAX_NAVI_ERR2)
		{	
			log_printf("err20(%d,%d,)\r\n",x,y);
			continue;
		}else
		if(search_navi_repeat_table(x,y) >= 3)		//连续去了三次，就不去了
		{
			log_printf("err21(%d,%d,)\r\n",x,y);
			insert_navi_err_table(x,y,MAX_NAVI_ERR2);
			continue;
		}
		if(clean_sta(x,y) == 0 || OBST_XY(x,y))
		{
		//	log_printf("err23(%d,%d,)\r\n",x,y);
			continue;
		}
		/*
			处理一种极限情况，门比较小，就只有两格，结果一格未清扫，一格清扫，但下下格都未清扫，
			没有这条，就过不去了。
		有问题，导致有一个点的时候，经常会进去
		if((clean_sta(x,y+1) ==0 && clean_sta(x,y+2) ==0 && clean_sta(x,y+3) ==0 ) && //一个点为清扫
		    ((OBST_XY(x-1,y+1) ==0 && clean_sta(x-1,y+2) ==0 && clean_sta(x-1,y+3) ==0 ) ||  //下一不是障碍物，且下下个点未清扫
			 (OBST_XY(x+1,y+1) ==0 && clean_sta(x+1,y+2) ==0 && clean_sta(x+1,y+3) ==0 )))  //下一不是障碍物，且下下个点未清扫
		{
			*tox = x;
			*toy = y;
			log_printf("[unclr_xline]left_1,(%d,%d,)\r\n",x,y);
			return LEFT_SIDE;			
		}
		if((clean_sta(x,y-1) ==0 && clean_sta(x,y-2) ==0 && clean_sta(x,y-3) ==0 ) && //一个点为清扫
		    ((OBST_XY(x-1,y-1) ==0 && clean_sta(x-1,y-2) ==0 && clean_sta(x-1,y-3) ==0 ) ||  //下一不是障碍物，且下下个点未清扫
			 (OBST_XY(x+1,y-1) ==0 && clean_sta(x+1,y-2) ==0 && clean_sta(x+1,y-3) ==0 )))  //下一不是障碍物，且下下个点未清扫
		{
			*tox = x;
			*toy = y;
			log_printf("[unclr_xline]right_1,(%d,%d,)\r\n",x,y);
			return RIGHT_SIDE;			
		}*/
		//左边的线没有清扫，且右边的线清扫了。
		//modify201710 
		//midify 201712 改成 OBST_STA，背后不是障碍物就可以进去，这样一长条线能进去
#if FORCE_SIDE_NAVI
		if(navigat->side == LEFT_SIDE)
#endif		
		{
			if(clean_sta(x,y+1) ==0 && clean_sta(x,y+2) ==0 && clean_sta(x,y+3) ==0 &&  clean_sta(x,y+4) ==0)
			{
				if(rout->max_len > 20)
				{
					if(point_sta(x,y-1) == SCAN_STA ) 
						c_left++;
					else
						c_left = 0;
				}else
					c_left++;
				c_left1++;		//很长的一条未清扫的，也要过去，
			}
			else
			{
//				log_printf("LEFT_SIDE Incalculation \r\n");
				c_left = 0;
				c_left1 = 0;
			}
			if(c_left >=MAX_UNCLEAN_IN || c_left1>= 8) 
			{
				int16_t c_unclean=0,c_clean=0;
				
			//	if(uint8_t is_unclear_side(x,y,x-1,y,1))
				uint8_t kret=is_unclear_side(x,y,x-1,y,1,&c_unclean,&c_clean);
				log_printf("[unclr_xline]left,(%d,%d,%d)%d,%d",x,y,c_left,c_unclean,c_clean);
				if(type ==0 && c_left>=4)
					return LEFT_SIDE;
				if(kret || (c_left >=5 && c_unclean >=maxi && c_clean >=1) || c_left1>=8)
				{
					 
					 log_printf(",ok,");
					 if(check_unclr_round(x,y,LEFT_SIDE)==0)
					 {
					 	log_printf(",ok2\r\n");
					 	x_unclean(x,y,tox,toy,LEFT_SIDE);
						return LEFT_SIDE;
					}else
						log_printf(",err\r\n");
				}else
					log_printf(",error\r\n");				
			}
		}
		//右边的线清扫，且左边的线没清扫
		//midify 201712 改成 OBST_STA，背后不是障碍物就可以进去，这样一长条线能进去
#if FORCE_SIDE_NAVI		
		if(navigat->side == RIGHT_SIDE)
#endif		
		{
			if(clean_sta(x,y-1) ==0 && clean_sta(x,y-2) ==0 && clean_sta(x,y-3) ==0 && clean_sta(x,y-4) ==0 )
			{
				if(rout->max_len > 20)
				{
					if(point_sta(x,y+1) == SCAN_STA)
						c_right++;
					else
						c_right = 0;
				}else
					c_right++;
				c_right1++;
			}
			else
			{
//				log_printf("RIGHT_SIDE Incalculation \r\n");
				c_right = 0;
				c_right1 = 0;
			}
			if(c_right >= MAX_UNCLEAN_IN || c_right1 >=8 )
			{
				
				int16_t c_unclean=0,c_clean=0;
				
				uint8_t kret=is_unclear_side(x,y,x-1,y,-1,&c_unclean,&c_clean);
				log_printf("[unclr_xline]right,(%d,%d,%d)%d,%d",x,y,c_right,c_unclean,c_clean);
				if(type ==0 && c_right>=4)
					return RIGHT_SIDE;
				if(kret || (c_right >=5 && c_unclean >=maxi && c_clean >=1) || c_right1 >=8)
				{
					 
					 log_printf(",ok\r\n");
					 if(check_unclr_round(x,y,RIGHT_SIDE)==0)
					 {
					 	log_printf(",ok2\r\n");
					 	x_unclean(x,y,tox,toy,RIGHT_SIDE);
						return RIGHT_SIDE;
					 }else
					 	log_printf(",err\r\n");
				}else
					log_printf(",error\r\n");
			}
		}
		
	}
	return NO_SIDE;

}


//这条线是否未清扫。找最近X的方向
uint8_t unclr_xline_near(XLINE_T *xline,int16_t tx,int16_t ty,int16_t *tox,int16_t *toy)
{
	uint16_t x,y,c_left=0,c_right=0;//,c_left1=0,c_right1=0;
//	int16_t maxi=0;
	
	y = xline->by;
	log_printf("[unclr_xliner]search(%d,%d,)->(%d,%d,)side=%d,%d\r\n",xline->bx,xline->by,xline->ex,xline->by,navigat->side,xline->clean);
	if(disxy(ty,y) >3)
		return 0;
	for(x = xline->bx;x <= xline->ex;x++)
	{
		
		
		if(navigat->side == LEFT_SIDE)
		{
			if(clean_sta(x,y+1) ==0 && clean_sta(x,y+2) ==0 && clean_sta(x,y+3) ==0 )
			{
					c_left++;
			}
			else
				c_left = 0;
			if(c_left >=MAX_CLEAN_FRONT && disxy(x,tx) <= 5) 
			{
				*tox = x;
				*toy = y;
				log_printf("leftunclr(%d,%d,%d)\r\n",x,y,c_left);
				return LEFT_SIDE;
			}
		}
	
		if(navigat->side == RIGHT_SIDE)
		{
			if(clean_sta(x,y-1) ==0 && clean_sta(x,y-2) ==0 && clean_sta(x,y-3) ==0  )
			{
					c_right++;
			}
			else
				c_right = 0;
			if(c_right>=MAX_CLEAN_FRONT &&  disxy(x,tx) <= 5)
			{
				*tox = x;
				*toy = y;
				log_printf("rightunclr(%d,%d,%d)\r\n",x,y,c_right);
				return RIGHT_SIDE;
				
			}
		}
		
	}
	return NO_SIDE;

}

/*
获取原始的坐标
*/
uint8_t get_org_coord(int16_t *tox,int16_t *toy)
{
//	int16_t bx,ex;
	//找不到，则返回最后一个
	navigat->can_navi = 1;
	log_printf("[get_org],cgo=%d,bx=%d,ex=%d\r\n",navigat->c_go,navigat->first_bx ,navigat->first_ex);
	/*
	if(navigat->c_go < 20)
	{
		if(navigat->first_ex > 0 && navigat->first_bx > 0)
		{
			int16_t x,c_unclr_left=0,c_unclr_right=0;
			
			log_printf("check first ex,bx:\r\n");
			bx = navigat->first_ex > navigat->first_bx?navigat->first_bx:navigat->first_ex;
			ex = navigat->first_ex < navigat->first_bx?navigat->first_bx:navigat->first_ex;

			for(x = bx;x<ex;x++)
			{
				if(point_sta(x,98) == 0 )
					c_unclr_right++;
				else
					c_unclr_right = 0;
				if(point_sta(x,102) == 0 )
					c_unclr_left++;
				else
					c_unclr_left = 0;
				if(c_unclr_left >=3)
				{
					*tox = x;
					*toy = 100;
					log_printf("found left(%d,%d,)\r\n",x,100);
					return LEFT_SIDE;
				}
				if(c_unclr_right >=3)
				{
					*tox = x;
					*toy = 100;
					log_printf("found right(%d,%d,)\r\n",x,100);
					return RIGHT_SIDE;
				}
				
			}
		
		}
	}
	*/
	if(navigat->from_charge == TRUE)		//从充电座出来，那么地址
	{
		log_printf("search from charge:\r\n");
		{
			int k;
			for(k=96;k>90;k--)
			{
				if(OBST_XY(k-2, 100))
				{
					break;
				}
			}
			*tox = k;
			*toy = navigat->dock_y - 1;
			navigat->tox = *tox;
			navigat->toy = *toy;
			log_printf("  [search_dock](%d,%d,)\r\n",*tox,*toy);
		}
	
	}else
	{
		*tox = 100;
		*toy = 100;
		navigat->tox = *tox;
		navigat->toy = *toy;
		log_printf("  [search_unclear21](%d,%d,)\r\n",*tox,*toy);

	}
	navigat->ntox = navigat->tox;
	navigat->ntoy = navigat->toy;
	return NO_SIDE;

}
uint8_t search_unclr_in_erea(int16_t *tox,int16_t *toy)
{
	XLINE_T *xline; 
	int16_t i;
	uint8_t side;
	//xline = area->xline->prev;	//上一条
	//搜索回去，查看未清扫区域
	*tox = *toy = 0;
	i = rout->max_len - 1;
	while(i > 0)
	{
		xline = &rout->xline[i--];
		side = unclr_xline(xline,tox,toy,1);
		if( side!= NO_SIDE)
		{
			navigat->tox = *tox;
			navigat->toy = *toy;
			return side;
		}
		
		
	}
	//找不到，则返回最后一个
	if(navigat->from_charge == TRUE)		//从充电座出来，那么地址
	{
		log_printf("search from charge:\r\n");
		{
			int k;
			for(k=96;k>90;k--)
			{
				if(OBST_XY(k-2, 100))
				{
					break;
				}
			}
			*tox = k;
			*toy = 99;
			navigat->tox = *tox;
			navigat->toy = *toy;
			log_printf("  [search_dock](%d,%d,)\r\n",*tox,*toy);
		}
	
	}else
	{
		*tox = 100;
		*toy = 100;
		navigat->tox = *tox;
		navigat->toy = *toy;
		log_printf("  [search_unclear21](%d,%d,)\r\n",*tox,*toy);

	}	
	return NO_SIDE;
}



/*
在这个区域内找距离最近的点，过去。
范围在 y_min和y_max之间，
router_sta :0 - 根据地图边缘找，且计算是否能导航


router_sta:2	只找未清扫点

*/
#define XLEN_CHECK_UCLR		1
uint8_t search_unclr_in_erea_near(int16_t tx,int16_t ty,int16_t *tox,int16_t *toy,int16_t y_min,int16_t y_max,uint8_t router_sta,uint8_t kside)
{
#if XLEN_CHECK_UCLR
	XLINE_T *xline; 
	int16_t i;//,c=0;
	int16_t /*y1,y2,*/miny,maxy;//x,;
	//int16_t tox,toy;
	uint8_t side,m_side;
	int16_t ntx,nty;
	//uint16_t count = 0;
	int angle;
	//uint8_t k1,k2;

	int16_t x_dir,y_dir;
#else
		int16_t miny,maxy;
		uint8_t side,m_side;
	int angle;
	int16_t x_dir,y_dir;
#endif	

	log_printf("[search_unclr_in_erea_near]...\r\n");
	//micony201710
	miny = y_min <= y_max?y_min:y_max;
	maxy = y_min > y_max?y_min:y_max;
	if(router_sta == 0)
	{
		side = unclr_by_edgeways(&angle,&m_side,0);
//#if FORCE_SIDE_NAVI
		if(side == navigat->side)
//#else
//		if(side == LEFT_SIDE || side == RIGHT_SIDE)
//#endif		
		{
			navigat->tox = *tox = navigat->func_arg.arg16_1;
			navigat->toy = *toy = navigat->func_arg.arg16_2;	
			 log_printf("found unclr edgeways(%d,%d,)\r\n",navigat->tox,navigat->toy);	
			 navigat->can_navi = 1;
			 return side;
		}	
		//c = 0;
		//计算一种由于侧面碰撞导致的导航，此时应该继续前行的。
		x_dir = navigat->lst_angle ==0?-1:1;
		y_dir = navigat->side == LEFT_SIDE? -1:1;
		log_printf("[search_unclr]now=(%d,%d,%d,%d),min=%d,max=%d\r\n",X_NOW,Y_NOW,x_dir,y_dir,miny,maxy);
		//前方是不是碰撞,前方未去，直接去前方
		if(OBST_XY(X_NOW +x_dir,Y_NOW) && clean_sta(X_NOW +x_dir*2,Y_NOW) == 0 && clean_sta(X_NOW +x_dir*3,Y_NOW) == 0)
		{
			log_printf("front obst(%d,%d,%d,%d).\r\n",X_NOW +x_dir,Y_NOW,x_dir,y_dir);
			if(SCANE_XY(X_NOW,Y_NOW+y_dir) && SCANE_XY(X_NOW+x_dir,Y_NOW+y_dir) && 
				SCANE_XY(X_NOW+x_dir*2,Y_NOW+y_dir) && SCANE_XY(X_NOW+x_dir*3,Y_NOW+y_dir))
			{
				 navigat->tox = *tox = X_NOW+x_dir*2;
				 navigat->toy = *toy = Y_NOW+y_dir;
				 
				 log_printf("found unclr infront(%d,%d,)\r\n",navigat->tox,navigat->toy);
				 navigat->can_navi = 1;
				 return navigat->side;
			}
		}
	}

	//清零未清扫表
	navigat->uc_len = 0;
	for(i=0;i<UC_LEN;i++)
	{
		navigat->uc_x[i] = navigat->uc_y[i] =0;
	}
	
	//从当前点开始
//	y1 = ty;
//	y2 = ty-1;
//	c = 0;
//	k1 = k2 =  0;
	log_printf("max xline=%d\r\n",rout->max_len);
//	while(1)
	//{
		
		for(i=0;i<rout->max_len;i++)
		{
			xline = &rout->xline[rout->max_len -i-1];
		//	if(xline->by == y1 || xline->by==y2)		//Y相等的线
			{
				log_printf("xline=%d,",rout->max_len -i-1);
				side = unclr_xline(xline,tox,toy,1);
				//if(side != NO_SIDE/* && side == m_side*/)		//micony20170725 方向一致
#if FORCE_SIDE_NAVI				
				if(side == navigat->side)
#else
				if(side == LEFT_SIDE || side == RIGHT_SIDE)
#endif				
				{

					if(navigat->uc_len < UC_LEN)
					{
						navigat->uc_x[navigat->uc_len] = *tox;
						navigat->uc_y[navigat->uc_len] = *toy;
						navigat->uc_side[navigat->uc_len++] = side;
						log_printf("found clr=(%d,%d,%d)uclen=%d\r\n",*tox,*toy,side,navigat->uc_len);
					}
				}
			}
		}
		/*
		if(y1 <= maxy)
			y1++;
		else
			k1 = 1;
		if(y2 >=miny)
			y2--;
		else
			k2 = 1;
		if(k1 && k2)
			break;
		//if(c++>=100)
		//	break;
	}
*/
	//micony201710  	只取未清扫点
	if(router_sta == 2)
		goto l_end_search_unl;
	if(navigat->uc_len ==0)
	{
		log_printf("not found uclr\r\n");
		/*
		if(count == 1)
		{
			log_printf("search second time\r\n");
			goto l_begin_suclr;		//查低耳边
		}	
*/		
		goto l_end_search_unl;
	}
	log_printf("check router simple..,unclear len=%d\r\n",navigat->uc_len);
	//复杂路径导航,近处一定要导航过去。
	if(navigat->uc_len)
	{
		for(i=0;i<navigat->uc_len;i++)
		{
		//micony201805  14->10
			if(dis_xy(navigat->ty,navigat->uc_y[i]) <=2 && dis_xy(navigat->tx,navigat->uc_x[i]) <=10  )		
			{
				log_printf("check rout:(%d,%d,)\r\n",navigat->uc_x[i],navigat->uc_y[i]);
				//micony201805
				//if(search_route(navigat->tx,navigat->ty, navigat->uc_x[i],navigat->uc_y[i],&ntx,&nty) )	
				if(path_navigation(navigat->uc_x[i],navigat->uc_y[i]))
				{
					navigat->can_navi = 1;
					*tox = navigat->tox = navigat->uc_x[i];
					*toy = navigat->toy = navigat->uc_y[i];
					log_printf("route complex ok,to=(%d,%d,%d)\r\n",navigat->uc_x[i],navigat->uc_y[i],navigat->uc_side[i]);
					return navigat->uc_side[i];
					
				}
			}
		}
	}
	//优先去能简单导航过去的。
	if(navigat->uc_len)
	{
		for(i=0;i<navigat->uc_len;i++)
		{
			log_printf("check rout1:(%d,%d,)\r\n",navigat->uc_x[i],navigat->uc_y[i]);
			if(router_p2p(navigat->tx,navigat->ty, navigat->uc_x[i],navigat->uc_y[i],&ntx,&nty) )	
			{
				navigat->can_navi = 1;
				*tox = navigat->tox = navigat->uc_x[i];
				*toy = navigat->toy = navigat->uc_y[i];
				log_printf("route simple ok,to=(%d,%d,%d)\r\n",navigat->uc_x[i],navigat->uc_y[i],navigat->uc_side[i]);
				return navigat->uc_side[i];
				
			}
		}
	}
	log_printf("check router complex..\r\n");
	//复杂路径导航
	if(navigat->uc_len)
	{
		for(i=0;i<navigat->uc_len;i++)
		{
			log_printf("check rout2:(%d,%d,)\r\n",navigat->uc_x[i],navigat->uc_y[i]);
			//micony201805
			if(path_navigation(navigat->uc_x[i],navigat->uc_y[i]))
			//if(search_route(navigat->tx,navigat->ty, navigat->uc_x[i],navigat->uc_y[i],&ntx,&nty) )	
			{
				navigat->can_navi = 1;
				*tox = navigat->tox = navigat->uc_x[i];
				*toy = navigat->toy = navigat->uc_y[i];
				log_printf("route complex ok,to=(%d,%d,%d)\r\n",navigat->uc_x[i],navigat->uc_y[i],navigat->uc_side[i]);
				return navigat->uc_side[i];
				
			}
		}
	}
	if(navigat->uc_len)
	{
		i = 0;
		*tox = navigat->tox = navigat->uc_x[i];
		*toy = navigat->toy = navigat->uc_y[i];
		log_printf("route complex ok,to=(%d,%d,%d)\r\n",navigat->uc_x[i],navigat->uc_y[i],navigat->uc_side[i]);
		navigat->can_navi = 0;
		log_printf("[search_unclr]found uclr,but not foud road,go..(%d,%d,)\r\n",*tox,*toy);
		return navigat->uc_side[i];	
		
	}
	//micony20170725 方向一致
	/*
	if(count == 1)
	{
		log_printf("search second time\r\n");
		goto l_begin_suclr;		//查低耳边  查第二遍
	}
	*/

l_end_search_unl:

#if FORCE_SIDE_NAVI
	if(navigat->side == LEFT_SIDE )		//左边扫完，扫右边
	{
		int16_t kx,ky,nx,ny;
		//ret = 1;
		kx = (navigat->first_bx + navigat->first_ex) >> 1;
		ky = 100;
		log_printf("left side clean OK,org=(%d,%d,)\r\n",kx,ky);
		if(point_sta(kx,ky) == UNCLN_STA)
		{
			ky++;
			log_printf("unclr ,set org=(%d,%d,)\r\n",kx,ky);			
		}
		//micony201805
		//if(search_route(navigat->tx,navigat->ty,kx,ky,&nx,&ny))
		if(path_navigation(kx,ky))
			navigat->can_navi = 1;		//能导航过去。
		//navigat->unlen = 0;
		//navigat->nav_ucln = NULL;
		navigat->side = RIGHT_SIDE;
		*tox  =navigat->toxx[0]=navigat->toxx[1] =kx;
		*toy  =navigat->toyy[0]=navigat->toyy[1] =ky;
		navigat->to_type[0] = navigat->to_type[1] =TYPE_ENDPOINT;		//导航点，碰撞不会被删
		navigat->tolen = 2;
		//rout->xline_len = 0;
		//rout->max_len = 0;		
		return navigat->side;
	}
#endif
	return NO_SIDE;
}


uint8_t unclr_xline_by_point(int16_t ty,int16_t *tox,int16_t *toy,uint8_t can_rout)
{
	XLINE_T *xline; 
	int16_t i;
	uint8_t side;
	int16_t ntx,nty;

	for(i=0;i<rout->max_len;i++)
	{
		xline = &rout->xline[i];
		if(xline->by == ty )		//Y相等的线
		{
			side = unclr_xline(xline,tox,toy,1);
			if(side != NO_SIDE)
			{
				if(can_rout)
				{
					if(search_route(X_NOW,Y_NOW,*tox,*toy,&ntx,&nty)==0)
					{
						log_printf("cannot rout\r\n");
						continue;
					}
				}
				navigat->tox = *tox;
				navigat->toy = *toy;
				return side;
			}
		}
	}
	return NO_SIDE;
}
//判断连个点是否可走得通。
uint8_t can_router_p2p(int16_t bx,int16_t by,int16_t ex,int16_t ey)
{
	if(bx == ex )
	{
		if(osbt_sta_y(bx,by,ey) == 0)		//X轴相等
			return 1;
		else
			return 0;
	}
	else if(by == ey )
	{
		if(osbt_sta_x(bx,by,ex) == 0)		//X轴相等
			return 1;
		else
			return 0;
	}
	return 0;
}
/*
导航到附近

int16_t navi_to_neaar()
{
	side = unclr_by_edgeways(&angle,&m_side,1);
	//if(side == LEFT_SIDE || side == RIGHT_SIDE)
	if(side == navigat->side)
	{
		navigat->tox = *tox = navigat->func_arg.arg16_1;
		navigat->toy = *toy = navigat->func_arg.arg16_2;	
		 log_printf("found unclr edgeways(%d,%d,)\r\n",navigat->tox,navigat->toy);	
		 navigat->can_navi = 1;
		 return side;
	}
}
*/
uint8_t is_road_valid(int16_t tox,int16_t toy)
{
	int i;
	log_printf("[is_road_valid]:\r\n");
		
	if(navigat->c_road == 0 || navigat->c_road >=20)
	{
		log_printf("c_road error(%d)\r\n",navigat->c_road);
		navigat->c_road = 0;
	}
	for(i=0;i<navigat->c_road;i++)
	{
		log_printf("#(%d,%d,0.0,r)-[road=%d]#\r\n",navigat->x_road[i],navigat->y_road[i],i);
	}
	if(can_router_p2p(X_NOW,Y_NOW,navigat->x_road[0],navigat->y_road[0])==0)
	{
		log_printf("errroad(%d,%d,)->(%d,%d,)\r\n",X_NOW,Y_NOW,navigat->x_road[0],navigat->y_road[0]);
		return 0;
	}
	for(i=0;i<navigat->c_road-1;i++)
	{
		if(can_router_p2p(navigat->x_road[i],navigat->y_road[i],navigat->x_road[i+1],navigat->y_road[i+1])==0)
		{
			log_printf("errroad(%d,%d,%d)->(%d,%d,)\r\n",navigat->x_road[i],navigat->y_road[i],i,navigat->x_road[i+1],navigat->y_road[i+1]);
			return 0;
		}
	}
	return 1;
}

/*
导航运动
*/
uint8_t navigat_motion(short tox,short toy,XLINE_T *xline)
{
	int i;//,j,k,y_dir;
	int c=0;
	int ret ;
	char is_near=0;
//	int16_t last_near_x = 0,last_near_y = 0;
	int16_t x_bum,y_bum;
	int16_t state;
//	uint8_t is_road=0;
//	uint8_t dock_adj=0;		//是否要充电桩矫正	
	navigat->sta = NAV_STA_NAVIGATE;
	navigat->ret_search = 1;		//默认不是中间过渡点
	navigat->y_max_draw = 0;
	//不需要搜索未清扫区域的时候，几个参数要初始化一下，要不然调用 search_route 函数的时候，或下边的调用的时候可能有问题
	state = sys->sState;
	navigat->tox = navigat->toxx[0] = tox ;
	navigat->toy = navigat->toyy[0]= toy;
	navigat->to_type[1] = navigat->to_type[0] = TYPE_ENDPOINT;		//导航点，碰撞不会被删

	//为了导航的需要，必须搞两个
	navigat->toxx[1]=navigat->toxx[0];
	navigat->toyy[1]=navigat->toyy[0];

	navigat->tolen = 2;
	navigat->rever_navi = 0; //还没反向运动
//	if(Y_NOW > 110 || Y_NOW < 80)
	//	dock_adj = 1;
	log_printf("========================================\r\n");
	log_printf("[navigat_motion]\r\n");
	log_printf("========================================\r\n");
	
l_begin_navigat_motion:	
	navigat->tox = navigat->toxx[0] = tox ;
	navigat->toy = navigat->toyy[0]= toy;
	check_now_point(navigat->tx,navigat->ty,1);		//检测当前点的合法性，如果当前点在障碍的上，则，要移动一下。
	CHECK_NAVI_STA_RT(0);

	log_printf("\r\n[navigat_motion]c=%d,now point=(%d,%d,),to=(%d,%d,)\r\n",c,navigat->tx,navigat->ty,tox,toy);
	if(c ==0)		//第一步导航，保存导航点，避免重复导航
	{
		insert_navi_repeat_table(tox,toy,1);
	}
/**********************************************************************************************************
	连续三次失败，如果是特征点导航，或是直接导航到某点，则退出，
	存储失败点，如果已经存在失败点，计数加1
	强制导航到特征物，去矫正一下
********************************************************************************************************/
	if(c++ >=4)
	{
		log_printf("[navigat_motion]navigat_robot error,save err point,to=(%d,%d,)\r\n",tox,toy);
		//存储失败的点
		insert_navi_err_table(tox,toy,2);
		return NAVI_MT_GO_ERR;
	}
	//====================================================================搜索路径========================================
	//是否要查询未清扫点
	//判断路基是否存在，且有效。
//	if(is_road_valid(tox,toy) == 0)
	{
		ret = search_route(navigat->tx,navigat->ty,tox,toy,&navigat->nextx,&navigat->nexty);	//如果不要搜索未清扫区域，则根据目标坐标计算路径
		//路径搜索失败，走弓字形回去
		if(ret==0)
		{ 
			log_printf("[navigat_motion]router error,to=(%d,%d,)\r\n",navigat->tox,navigat->toy);
			return NAVI_MT_ROUT_ERR;
		}
		if(navigat->c_road == 0 || navigat->c_road >=20)
		{
			log_printf("c_road error(%d)\r\n",navigat->c_road);
			navigat->c_road = 0;
		}
	}
	/*
	if(sys->low_power && navigat->ntox >0 )		//低电压告警
	{
		log_printf("back to dock...\r\n");
		ny3p_play(VOICE_M_DOCK);
	}
	*/
	navigat->x_road[navigat->c_road] = tox;
	navigat->y_road[navigat->c_road++] = toy;

	//比较近的地方，不延边，不矫正
	if(disxy(Y_NOW,navigat->toy) <3)
		is_near = 1;
		
	for(i=0;i<navigat->c_road;i++)
	{
		log_printf("#(%d,%d,0.0,r)-[road=%d]#\r\n",navigat->x_road[i],navigat->y_road[i],i);
	}

	navigat->sta = NAV_STA_NAVIGATE;

	//==================================================走导航路径==================================================
//	mast_navi_near = 0;
	for(i=0;i<navigat->c_road;i++)	
	{
		log_printf("===========[navigat_motion]router step=%d==========\r\n",i);
		CHECK_NAVI_STA_RT(0);
		//判断是不是没有必要的折线。
		if((i+1) < navigat->c_road)		//还有下一步
		{
			//X轴相等看看两次是不是在本次的两侧，是的话，就不走下一步了，直接走下下一步
			if(X_NOW == navigat->x_road[i] && X_NOW == navigat->x_road[i+1])
			{
				if(( Y_NOW > navigat->y_road[i] && Y_NOW < navigat->y_road[i+1]) || 
				   ( Y_NOW < navigat->y_road[i] && Y_NOW > navigat->y_road[i+1]))
				{
					log_printf("error side,now=(%d,%d,),to=(%d,%d,),next=(%d,%d,),go next\r\n",X_NOW,Y_NOW,
						navigat->x_road[i],navigat->y_road[i],navigat->x_road[i+1],navigat->y_road[i+1]);
					continue;
				}
			}else if(Y_NOW == navigat->y_road[i] && Y_NOW == navigat->y_road[i+1])
			{
				if(( X_NOW > navigat->x_road[i] && X_NOW < navigat->x_road[i+1]) || 
								   ( X_NOW  < navigat->x_road[i] && X_NOW  > navigat->x_road[i+1]))
				{
					log_printf("error side2,now=(%d,%d,),to=(%d,%d,),next=(%d,%d,),go next\r\n",X_NOW,Y_NOW,
						navigat->x_road[i],navigat->y_road[i],navigat->x_road[i+1],navigat->y_road[i+1]);
					continue;
				}

			}
		}
		/*
		if(dis_xy(navigat->y_road[i],navigat->dock_y) <=2 && navigat->dock_adj_y)
		{
			log_printf("near dock,,must be adj(%d)..\r\n",navigat->dock_adj_y);
			navigat->dock_adj_y = 0;
		}
		*/
		
		//不是导航回到充电桩才去用充电桩矫正。
		if(navigat->side != NO_SIDE && navigat->sheepflod == 0)
		{
			if(navi_go_dock_adj(0) > 0)
			{
				log_printf("navi_go_dock_adj ok,renavi...c=%d\r\n",c);
				if(c > 0)
					c--;
			
				goto l_begin_navigat_motion;
			}
		}
		if(state != sys->sState)		///2018-07-13 jzz
		{
			motor_run(GO_STOP,0,0,0); 
			return NAVI_MT_GO_ERR;
		}	
		
		ret = navi_walk_map(navigat->x_road[i],navigat->y_road[i]);			/////sys->sState已经变成dock模式 但是没有处理
		
		//CHECK_NAVI_STA_ONLY();
		if(sys->sState !=state) 		///2018-07-13 jzz
		{ 
			motor_run(GO_STOP,0,0,0); 
			return NAVI_MT_GO_ERR;				///state已经是空闲状态
		}	
		if(ret ==0)			//碰撞
		{
			CHECK_NAVI_STA_RT(0);
			navigat->navi_near = 0;
			x_bum = X_NOW;
			y_bum = Y_NOW;
			save_map(0,0,1);		//保存一下，只有为清扫点，才保存障碍物,不加上这个，在碰撞的时候，如果是为清扫点，就没有置障碍物，导致一直在这里导航碰撞
			log_printf("navi bum,to=(%d,%d,)-now=(%d,%d,),i=%d,adc=%d,%d,ret=%d\r\n",navigat->x_road[i] ,navigat->y_road[i],navigat->tx,navigat->ty,i,MIDLE_ADC(),sys->c_fw_rd,navigat->ret_search);
			ret = proc_navigat_bum(navigat->x_road[i] ,navigat->y_road[i],1);
			log_printf("navibum ok,ret=%d\r\n",ret);
			if(ret == CROSS_XY_NAVI)		//去导航
				goto l_begin_navigat_motion;//重新计算路径
			else if (ret == CROSS_XY_OK)	//导航成功
				break;
			else if(ret == CROSS_XY_POINT_OK)
			{

			}else if(ret == CROSS_XY_ERR)		//需要工字型走一段路
			{
				log_printf("corss error,go near..\r\n");
				/*
					如果导航到跟前了，延边还过不去，就放弃改点了
					发现有这样的情况，周边障碍物比较多，到了导航点跟前了，碰撞就乱延边了。
					导致一直去不了，这样，不如索性放弃。
				*/				
				if(dis_xy(x_bum,navigat->tox) < 2 && dis_xy(y_bum,navigat->toy) < 2)
				{
					log_printf("we drop this point\r\n");
					insert_navi_err_table(navigat->tox,navigat->toy,4);	//放弃改点
				}else
					adj_go_nearwall(tox ,toy);
				return NAVI_MT_GO_ERR;
			/*
				uint8_t g_side = navigat->toy > Y_NOW ?LEFT_SIDE:RIGHT_SIDE;
				log_printf("near_cross_xy error,z_go\r\n");
				z_go_for_map(g_side,navigat->tox,navigat->toy,5);
				return 0;
				*/
				//c = 0;
				//goto l_begin_navigat;//重新计算路径
			}
		}else if(ret ==2)
		{
			navigat->navi_near = 0;
			log_printf("navi_walk_map ret =2 renavi..\r\n");
			c = 0;			//micony0815发现多次延边导航回来后，会导致改点次数过多导致失败了。
			goto l_begin_navigat_motion;//重新计算路径
		}
/////sys->sState已经变成dock模式 但是没有处理
		navigat->navi_near = 1;
		
		CHECK_NAVI_STA_RT(0);
		
		//本次距离较短，切下次距离也比较短
		if(navigat->walk_dis < (HW_GRID / 3) &&  disxy(navigat->tx,navigat->x_road[i+1]) < 6 && disxy(navigat->ty,navigat->y_road[i+1])<6)
		{
			log_printf("short dis not adj\r\n");
			continue;
		}
		//比较远的地方才考虑延边
		
		if(is_near ==0 && navigat->sheepflod == 0)
		{
#if NEAR_ADJ_NAV	
			int rtn;
			if(navigat->walk_dis > 4000)		//超过一米多，就让再校验一次
				navigat->c_go+=30;				//引发 NAVA_ADJ_NEAR
			rtn=navigat_to_target(NAVA_ADJ_NEAR);
			if(state != sys->sState)		///2018-07-16 jzz
				return NAVI_MT_GO_ERR;		
			if( rtn!=ADJ_NAVI_NO_RUN)
			{
				log_printf("have move ,search unclr again2...\r\n");
				c = 0;
				goto l_begin_navigat_motion;
			}	
#endif		
		
		}
		if(navigat->tx == tox && navigat->ty == toy)
		{
			log_printf("navi for.. ok\r\n");
			break;
		}	
		
		//如果没有导航到位置上，走失败了，有时候会走过了，那再修正一次。
	}
	log_printf("----navi end.....\r\n");
	if(navigat->side != NO_SIDE && navigat->sheepflod ==0 && navigat->navi_to_dock == 0)
	{
		if(navi_go_dock_adj(0) > 0)
		{
			log_printf("navi_go_dock_adj ok,renavi...c=%d\r\n",c);
			if(c > 0)
				c--;
		
			goto l_begin_navigat_motion;
		}
	}

	log_printf("==naviOK==(%d,%d,3.1f)\r\n",X_NOW,Y_NOW,sys->angle);
	navigat->navi_side = NO_SIDE; 
	//micony20190307 导航到位了，刷新时间。
	navigat->t_navigat = sys->t_navi_work;		//刷新导航时间


	navigat->sta = NAV_STA_NORMORL;	
	return NAVI_MT_OK;


}




/*
根据映射地图，直线运动,不记录行走的点和
bk_dis :后退的距离，0，为前进多少，后退多少
最好的方式是，沿X轴，或Y轴行走

有一次碰到这样的问题 :
180°走，从X的坐标 140 走到123,结果就走过头了，走到122去了。
误差的问题。

返回 0 - 碰撞
     1- ok	
     2 - 延边了，需要重新导航
*/

uint8_t navi_walk_map(short x,short y)
{
	int16_t ny=0,nx=0;
	int16_t state;
	//羊圈里，不走边边。

	state = sys->sState;
	log_printf("[navi_walk_map](%d,%d,)->(%d,%d,)sheep=%d,near=%d\r\n",X_NOW,Y_NOW,x,y,navigat->sheepflod,navigat->navi_near);
	if((dis_xy(x,X_NOW) < 5 && dis_xy(y,Y_NOW) < 5) || navigat->sheepflod || navigat->navi_near == 0)
	{
		log_printf("not near navi\r\n");
		return walk_map_cord_only(x ,y,1000,10);
	}

	if(X_NOW == x) 	//沿着Y轴走。
	{
		int16_t xret;
		int16_t yy = Y_NOW,ydir,i,ox,oy;
		int16_t y1=0,max;
		int16_t max_grid = 5;
		ydir =y > Y_NOW?1:-1;
		
		log_printf("[navi_walk_map]near point1:ydir=%d\r\n",ydir);
		max = dis_xy(Y_NOW,y);
		if(max > 10)
		{
			max_grid = 5;
		}else
		if(max > 5)
			max_grid = 3;
		else
		{
			return walk_map_cord_only(x ,y,1000,10);
		}		
		
		//找到可以一延边过去的点。
		for(i = 0;i<max;i++)
		{
			yy +=ydir;
			if(yy == y)
				break;
			xret =obst_line_on_90_270(X_NOW,yy,max_grid,&ox,&oy);
			if(navigat->from_charge)			//从充电桩出来，比较沿着充电桩的线路走，通杀
			{
				if(dis_xy(ox,navigat->dock_x) < 4 && dis_xy(oy, navigat->dock_y) < 20)	//充电桩附近的这条线，通杀，不走
				{
					log_printf("a line near dock,(%d,%d,)no go near\r\n",ox,oy);
					ny = 0;
					break;
				}
			}
			if(xret ==2 || xret==-2)   //如果是一条严格的线，就直接退出了。
			{
				ny = yy;
				log_printf("point can near go2=(%d,%d,)\r\n",X_NOW,ny);
				break;
			}
			/*
			不是一条线，会有问题，测试发现会乱延边
			*/
			
			if((xret ==1 || xret==-1) && y1==0)  //是一条线，再继续找看看，有没有严格的一条线
			{
				y1 = yy;
				log_printf("point can near go2=(%d,%d,)\r\n",X_NOW,y1);
			}
			
		}
		
		if(y1 && ny == 0)
		{
			ny =y1;
			log_printf("getx=%d\r\n",ny);
		}
		if(ny > 0)
		{
			int16_t disX = dis_xy(X_NOW,ox);
			log_printf("disX=%d,dixY=%d\r\n",disX,max);
			if(disX >= max || dis_xy(disX,max) < 3)
			{
				log_printf("log dis to near ,not near...\r\n");
				ny = 0;
			}
		}
		//没有课矫正的，就直接走了
		if(ny ==0)
		{
			return walk_map_cord_only(x ,y,1000,10);
		}else
		{
			//走到能去直线的地方，走失败，直接退出了。
			if( walk_map_cord_only(X_NOW,ny,1000,10)==0)
				return 0;
			//再检查看看，是否有直线的地方，有就去，没有就去到本来要去的点
			xret = adj_go_nearwall(X_NOW,y);
			return 2;			//要重新导航

		}
	}else if(Y_NOW == y)
	{
			int16_t xret;
			int16_t xx = X_NOW,xdir,i,ox,oy;
			int16_t x1=0,max;
			int16_t max_grid = 5;
			xdir =x > X_NOW?1:-1;
			
			log_printf("[navi_walk_map]near point0:xdir=%d\r\n",xdir);
			max = dis_xy(X_NOW,x);
			if(max < 5)
				max_grid = 3;
			if(max <=3)
			{
				return walk_map_cord_only(x ,y,1000,10);
			}
			//找到可以一延边过去的点。
			for(i = 0;i<max;i++)
			{
				xx +=xdir;
				if(xx == x)
					break;
				xret =obst_line_on_0_180(xx,Y_NOW,max_grid,&ox,&oy);
				if(xret ==2 || xret==-2)   //如果是一条严格的线，就直接退出了。
				{
					nx = xx;
					log_printf("point can near go2=(%d,%d,)\r\n",xx,Y_NOW);
					break;
				}
			
				if((xret ==1 || xret==-1) && x1==0)  //是一条线，再继续找看看，有没有严格的一条线
				{
					x1 = xx;
					log_printf("point can near go3=(%d,%d,)\r\n",xx,Y_NOW);
				}
				if(state!=sys->sState)
					return 0;
			
			}
			
			if(nx==0 && x1 > 0)
			{
				ny =x1;
				log_printf("gety=%d\r\n",x1);
			}
			//如果去延边的距离，比去目的地的距离要远，就不去延边了。
			if(ny > 0)
			{
				int16_t disY = dis_xy(Y_NOW,oy);
				log_printf("disX=%d,dixY=%d\r\n",disY,max);
				if(disY >= max || dis_xy(disY,max) < 3)
				{
					log_printf("log dis to near2 ,not near...\r\n");
					ny = 0;
				}
			}
			
			//没有课矫正的，就直接走了
			if(nx ==0)
			{
				return walk_map_cord_only(x ,y,1000,10);
			}else
			{
				//走到能去直线的地方，走失败，直接退出了。
				if( walk_map_cord_only(nx ,y,1000,10)==0)
					return 0;
				//再检查看看，是否有直线的地方，有就去，没有就去到本来要去的点
				xret = adj_go_nearwall(x,Y_NOW);
				return 2;			//要重新导航
	
			}

	}else
		return walk_map_cord_only(x ,y,1000,10);


}


/*
在充电桩附近，上充电桩校验一下。
0 - 没动
1,2 动了。
type = 0 矫正，不去充电
type = 1 去充电
*/
//#define MAX_DOCK_HW_GO		6000
uint8_t navi_go_dock_adj(uint8_t type)
{
	int16_t by;//bx,		//记录初始的坐标
	int distance;
	//bx = X_NOW;
	by = Y_NOW;

	//teset201804
	//return 0;

	log_printf("[go_dock_adj]now=(%d,%d,),dock=(%d,%d,),to=(%d,%d,)%d\r\n",X_NOW,Y_NOW,navigat->dock_x,navigat->dock_y,navigat->x_dock,navigat->dock_y,navigat->dock_adj_y);
	if(navigat->sheepflod ==1)
	{
		log_printf("sheepflod not dock..\r\n");
		return 0;
	}
	if(navigat->navi_to_dock)
	{
		log_printf("navitodock not dockadj..\r\n");
		return 0;
	}
	if(navigat->dock_y ==0)
	{
		log_printf("not found dock\r\n");
		return 0;
	} 
	if(type == 0)
	{
		//避免重复的矫正
		if(navigat->min_ty > 85 && navigat->max_ty < 115)		//还走的不远，就不需要矫正
		{
			log_printf("maxy=%d,miny=%d,not adj by dock\r\n",navigat->max_ty,navigat->min_ty);
			return 0;
		}
		if(dis_xy(navigat->dock_adj_y,Y_NOW) < 7 )
		{
			log_printf("last dock adj y=%d,not adj\r\n",navigat->dock_adj_y);
			return 0;
		}

		if(dis_xy(Y_NOW,navigat->dock_y) >=5 /*|| dis_xy(X_NOW,navigat->x_dock) > 8*/)
		{
			log_printf("big dist,not dock adj\r\n");
			return 0;
		}		
	}
	

	if(dis_xy(X_NOW,navigat->x_dock) >=3)	//距离超过要走到那个位置去。
	{
		int16_t x,x_dir,i=0;
		x_dir = X_NOW > navigat->x_dock?-1:1;
		x = X_NOW;
		log_printf("[go_dock_adj]check x,dir=%d\r\n",x_dir);
		while(1)
		{
			if( x == navigat->x_dock)
				break;
			if(OBST_XY(x,Y_NOW))
			{
				log_printf("found OBST(%d,%d,)\r\n",x,Y_NOW);
				return 0;
			}
			x += x_dir;
			if(i++ > 10)
				break;			
		}
		if(walk_map_cord_only(navigat->x_dock,Y_NOW,1000,10) == 0)
		{
			log_printf("[go_dock_adj]walk bum,not adj\r\n");
			navigat->dock_adj_y = by;  //不让它再去找充电桩了。
			return 0; 
		}
	}
	
	navigat->dock_adj_y = by;
	if(Y_NOW > navigat->dock_y)	
	{
		turn_to_deg(270);
		robot_turn_deg(GO_LEFT,600,8);	//稍微转一下
	}
	else
	{
		turn_to_deg(90);
		robot_turn_deg(GO_RIGTH,600,8);
	}
	distance = (dis_xy(navigat->dock_y,Y_NOW)+4)* HW_GRID;
	//走过去，发现中线
	if(motor_go_fw(distance,NO_SIDE,1) == 2 )
		goto l_dock_go_midle;

	//distance = (dis_xy(by,Y_NOW)+4)* HW_GRID;
	CHECK_NAVI_STA_RT(0);
	log_printf("not found midle,go back,angle=%3.1f,%3.1f,dis=%d\r\n",navigat->angle,sys->angle,distance);
	if(navigat->angle == 90)
		turn_to_deg(270);
	else
		turn_to_deg(90);
	distance += HW_GRID * 4;
	if(motor_go_fw(distance,NO_SIDE,1) == 2 )
		goto l_dock_go_midle;
	log_printf("not found mid,now=(%d,%d,)\r\n",X_NOW,Y_NOW);
	////////////////////
	//如果这里是未清扫点，再掉头走回去。		///2018-06-16 jzz
	if(point_sta(X_NOW,Y_NOW) == UNCLN_STA)
	{
		if(navigat->angle == 90)
		{
			turn_to_deg(270);
			robot_turn_deg(GO_LEFT,600,8);	//稍微转一下
		}
		else
		{
			turn_to_deg(90);
			robot_turn_deg(GO_RIGTH,600,8);
		}
		if(motor_go_fw(distance,NO_SIDE,1) == 2 )
			goto l_dock_go_midle;		
	}
	///////////////////
	return 1;
l_dock_go_midle:
	CHECK_NAVI_STA_RT(0);
	log_printf("go fw found midle\r\n");
	turn_to_deg(0);
	if(type ==1)
	{
		sys->sState = SYS_DOCK;
		return 2;
	}
	CHECK_NAVI_STA_RT(0);
	dock_go_middle(by);
	sys->pwm = SYS_PWM;
	return 2;
}


