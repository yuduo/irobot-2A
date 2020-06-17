
/*
集中清扫
*/
#include "sys.h"


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
void init_focus_task(void)
{
	focus.distance = FOCUS_BASE;
//	focus.index = 0;
	focus.cnt = 0;

	navigat_init(0);
	focus.cnt =0;
	focus.distance = FOCUS_BASE;

	fos_degs[0] = 0;
	fos_degs[1] = 90;
	fos_degs[2] = 180;
	fos_degs[3] = 270;


	log_printf("init focus ok\r\n");
}
/*
集中清扫的线程
*/

int proc_focus_task(void)
{
	int idx;
	//if(focus.distance <=20 || focus.k >=16)
	if(focus.cnt  >=20 || focus.distance < 10)
	{
		log_printf("[focus_task]dis=%d,count=%d,focus over\r\n",focus.distance,focus.cnt );
		motor_run(GO_STOP,0,0,0);
		STOP_ALL_MOTOR();
		sys->sState = SYS_IDLE;
		return 0;
	}
	//delay_ms(200);
	log_printf("-----------------\r\n[focus_task]go,dis=%d,idex=%d\r\n",focus.distance,focus.cnt );

	//motor_go_forwark(dis, NO_SIDE_NEAR, NULL);		//走路

	delay_ms(50);
	//mpu6500_date_offset(500);
	//calc_gyro();
	log_printf("--walk ok,agl=%f,dis=%d,count=%d\r\n",sys->angle,focus.distance,focus.cnt );
	if(focus.cnt  <= 10)
		//focus.distance = FOCUS_BASE + focus.cnt *10;
		focus.distance += FOCUS_INC;								//行程增加
	else
		//focus.distance = FOCUS_BASE + (16 -focus.cnt )*10;
		focus.distance -= FOCUS_INC;

	focus.cnt  ++;

	idx = focus.cnt  & 3;
	log_printf("--count=%d,nx=%d,deg=%f\r\n",focus.cnt ,idx,fos_degs[idx]);
	//turn_to_deg(fos_degs[idx]);						//转到角度
	//focus.index = focus.index >=MAX_FOCUS_DEGRE_COUNT?0:focus.index;//计算下一个角度
	return 1;

}
