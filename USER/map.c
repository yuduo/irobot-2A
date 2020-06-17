#include "sys.h"





//uint8_t robot_whele_stop(uint8_t type)
uint8_t robot_whele_stop(uint8_t type)
{
#if JUDGE_STOP_FRONT


#if 0
	static int16_t koc=0;
	int16_t max_koc=2;

	if(type ==0)
	{
		koc = 0;
		return 0;
	}
	//直行的时候校验得更严格一点
	if(type ==2)
	{
		if((sys->angle >176 && sys->angle < 184) ||
			(sys->angle >86 && sys->angle < 94) ||
			(sys->angle >266 && sys->angle < 274) ||
			(sys->angle >-4 && sys->angle < 4) || sys->angle >356)
		{
			max_koc = 50;
		}else
			max_koc = 3;
	}



	int16_t cc=0;



	for(int16_t i=0;i<MAX_FRONT_HW;i++)
	{
		if(sys->front_hw[i]==0)
			cc++;
	}
	if((navigat->distance > 20 || motor.c_left_hw >1000) && (/**/motor.
c_front_stop == 0 || cc > sys->max_front_stop))
	{

		if(type ==2)			//直行的时候判断得更严格点
		{
			koc++;
			log_printf("wheleStop,left=%d,front=%d,cstop=%d,cc=%d,max=%d,fw=%d,%d\r\n",
motor.c_left_hw,navigat->distance,motor.c_front_stop,cc,sys->max_front_stop,
koc ,max_koc);
			if(koc >=max_koc)
			{
				motor_run(GO_STOP,0,0,0);	//

				delay_ms(20);
				gyro_whlmap();
				navigat->whele_stop = 1;
				sys->gSta |= MASK_BUM_MIDL;
				return 1;

			}
		}else
		{
			motor_run(GO_STOP,0,0,0);	//
			log_printf("wheleStop,left=%d,front=%d,cstop=%d,cc=%d,max=%d\r\n",motor.
c_left_hw,navigat->distance,motor.c_front_stop,cc,sys->max_front_stop);
			delay_ms(20);
			gyro_whlmap();
			navigat->whele_stop = 1;
			sys->gSta |= MASK_BUM_MIDL;
			return 1;
		}
	}
	//直行的话，下边就不检测了。
	if(max_koc >=5)
		return 0;
#endif
	static uint16_t c=0;
	static int last_dis=0;
	//static int kkod=0;



	navigat->whele_stop =0;
	sys->g_t_walk++;
/*
	if(motor.c_left_hw < 50)
	{
		if(kkod++ >=10)
		{
			kkod=0;
			log_printf("t=%d\r\n",sys->g_t_walk);
		}
		if(sys->g_t_walk < 1000)		// 秒钟内不计算
		{
			last_dis = navigat->distance;
	//		last_left = motor.c_left_hw;
			c = 0;
			navigat->whele_stop =0;

			return 0;
		}
		//log_printf("");
	}
	*/
	c++;
	/*
	if( c > 5)
	{
		if(navigat->distance > 10 && last_dis == navigat->distance)
		{
			motor_run(GO_STOP,0,0,0);	//
			log_printf("whelestop,last=(%d,%d)\r\n",last_dis,navigat->distance);
			delay_ms(20);
			gyro_whlmap();
			navigat->whele_stop = 1;
			sys->gSta |= MASK_BUM_MIDL;
			return 1;

		}
	}
	*/
	//5ms 进来一次。
	if(c>=1000)
	{
		if(navigat->distance > 20 || sys->g_t_walk > 1000)
		{
			//log_printf("whele:%d,%d\r\n",navigat->distance,last_dis);
			if(disXY(last_dis,navigat->distance) < 20)		//????,
			{
				motor_run(GO_STOP,0,0,0);	//
				log_printf("whelestop2,last=(%d,%d)\r\n",last_dis,navigat->distance);
				delay_ms(20);
				gyro_whlmap();
				navigat->whele_stop = 1;
				sys->gSta |= MASK_BUM_MIDL;
				return 1;
			}
		}
		last_dis = navigat->distance;
		//log_printf("l=%d\r\n",last_dis);
//		last_left = motor.c_left_hw;
		c = 0;
	}
	return 0;
#else
		navigat->whele_stop =0;
	return 0;
#endif
}
int coordinate_calcu(uint8_t type)
{
	return 0;
}
int coordinate_calcu_test(void)
{
	return 0;
}

void cord_calc_store(char reset_dist)
{

}


/*
x = X / N + 100            y = Y/N + 100
X  =  (x-100) * N             Y = (y-100) * N
转换后，确保映射的坐标系，范围在0~200内。

//四舍5入的算法
int div_4_5(int xy,int hw)
{
	float xy1;
	short yx;
	xy1 = (float)xy / (float)(hw );
	if(xy1 > 0)
		xy1 +=0.5;
	else
		xy1 -=0.5;
	yx = (short)xy1;
	return yx;

}

*/

float format_agle(float angle2,uint8_t type)
{
	float agl = angle2;
	if(angle2 == NO_ANGLE_V)
		return NO_ANGLE_V;
	//log_printf("%f,%f,",angle2,agl);
	if(type == ANGLE_360)
	{
		if(angle2 < 0)
			agl += 360;
		else if(angle2 >=360)
			agl = angle2-360;


	}else if(type == ANGLE_180)
	{
		if(angle2 > 180)
			agl = agl - 360;
		else if(angle2 < -180)
			agl += 360;
	}
	//log_printf("%f\r\n",agl);
	return agl;
}


