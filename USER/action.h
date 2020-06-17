#ifndef  _ACTION_H_
#define  _ACTION_H_

#define CHECK_SIDE_MAP_OK		0x00		//侧面OK
#define CHECK_SIDE_NOT_NEAR		0x01		//侧面有障碍物，但不贴边
#define CHECK_SIDE_NOT_OBST		0x02		//侧面贴边，但没障碍物

#define ADJ_RUN_ABLE		0		/*允许前撞红外不参与弓字清扫*/


#define STRONG_BACKOFF		80
#define FRONT_OK		0x00
#define FRONT_BUM		0x01
#define FRONT_STOP		0x03

#define LOST_NUBUER   50

#define RET_GO_OK  			0x00			//走完行程
#define RET_GO_BUM			0x01			//碰撞
#define RET_GO_SLIP			0x02			//打滑
#define RET_GO_SBUM			0x03			//斜滑了
#define RET_GO_STA			0x04			//状态改变了。


#define MAX_C_STOP		10
#define MAX_FRONT_STOP_IR		10	//有红外 ，
#define MAX_FRONT_STOP		20


#if SYS_VER	 == VER_KESMAN// || VER_SMALLBLACKMACH || VER_BIGWHITEMACH
#define  MAX_NEAR_LOST		15

#else
#define  MAX_NEAR_LOST		15
#endif

#if SYS_VER	 == VER_KESMAN// || VER_SMALLBLACKMACH || VER_BIGWHITEMACH
				#define NEAR_LOST_DEG		18
#else
				#define NEAR_LOST_DEG		10
#endif


#define DEG_TURN_PWM		750     //2019 10   04    600
//#define MAX_C_M_ANGLE		20//20

#if LAGER_ROUND_MODE
#define MAX_C_M_ANGLE		35//20
#else
#define MAX_C_M_ANGLE		20
#endif
#define GO_FOR_DOCK_HW			5111		//要是一个比较奇怪的数字
#define NEAR_WALL_NORMAL		0x00		//正常退出
#define NEAR_WALL_OK			0x03		//扫完
#define NAER_WALL_ROUTER		0x04		//可以导航

#define DIR_NO_LIMIT				0xFFFF	//方向控制不限时


char motor_go_draw_map(uint8_t n_sta);
int fitting_map_near_wall(int g_max,short len,short ofset,short mix_cn);
int check_adj_draw_map(uint16_t *cx,uint16_t *cy);
char robot_go_draw_map(uint8_t n_sta,int max_grid,uint8_t side,uint8_t fitt,m_go_func m_go_check);
#if TURN_90_V
void turn_to_deg_v(int deg);
#else
#define turn_to_deg_v(X)		turn_to_deg(X)
#endif
void turn_to_deg_on_dir(int deg,uint8_t dir);
void gyro_move_test(uint8_t dir,int c);
int nearwall_obst_vilad(SIDEOBST *m_obst,int16_t max_ofs);


uint8_t motor_nearwall_back(uint8_t n_sta,int16_t tox,int16_t toy,int16_t maxy,uint16_t maxbum,uint8_t is_navi,uint8_t tnear);
char motor_nearwall_reverse(short xx,int yy);
uint8_t nearwall_go_back(int16_t ty,float angle,uint8_t t_near);
char motor_go_fw(int hw,uint8_t nsta,uint8_t type);

void init_doci_ir(void);
void insert_dock_ir(u8 *data);
uint8_t in_dock_mline(void);

uint8_t proc_doc_ir(void);

void motor_single_run(uint8_t dir);
uint8_t proc_motor_run_task(void);
uint8_t caluc_bum_by_front(uint8_t type);
void printf_dock_msg(void);
char robot_coord_test(short hw,short hw1);
void turn_deg_test(uint8_t dir,uint16_t count);
void insert_walk_fifo(int16_t xx,int16_t yy);
void nearwall_adj_test(void);
uint8_t proc_dock_near_bum(uint8_t flage);
int16_t robot_go_edgeways(uint8_t n_sta ,uint8_t type,m_go_func m_go_check);
int16_t func_draw_map(void);
void memset_func_arg(void);
char motor_go_draw_map(uint8_t n_sta);

char motor_go_forwark_2a(int hw,int bk_dist,uint8_t nsta,float *angle,m_go_func m_go_check);
char motor_go_edgeways_2a(uint8_t n_sta ,int dist,uint32_t tmr,int c_bum);

uint8_t near_large_round_360(int16_t *cyc_x,int16_t *cyc_y,float *m_angle,int16_t c_m_angle);///2018-07-11 jzz
uint8_t near_round_360(float *m_angle,int16_t c_m_angle);
char check_round_bum(uint8_t type);
void   proc_dock_coord(short bx,short by);
char motor_go_adj(int hw,uint8_t nsta,m_go_func m_go_check);
uint8_t motor_coord_run_test(uint8_t n_sta);
uint8_t check_router_sta(int16_t tox,int16_t toy);
short turn_back_180(void);//
void check_front_target(short xdir,int *xorg);
uint8_t reset_gyro(uint8_t sound);
char motor_go_spot_task(void);

//uint8_t reset_gyro(void);
void gyro_offset_manage(u16 cnt);
uint8_t reset_angle(void);
extern uint8_t get_quadrant(float angle);
extern uint8_t count_quadrant(uint8_t quadrant_1,uint8_t quadrant2,uint8_t quadrant3,uint8_t quadrant4);

#if(1 == TURN_CHECK_BUM)
//#define TURN_BUM_ONLY_BACKOFF		0	//0:要先反转再后退	1:直接后退
#define TURN_BUM_F_NO	0
#define TURN_BUM_F_LEFT_RIGHT	1

typedef struct last_turn_sta_t
{
	unsigned char last_turn_flag;			//最后旋转的状态备份
	unsigned char c_mbum_leftBehind;
	unsigned char c_mbum_rightBehind;
	unsigned char c_mbum_Behind_max;
	unsigned short c_motor_back_off;
	unsigned short c_motor_go_forward;
	unsigned short c_turn_check_bum_clc;	//记录周期
	float turn_agle;				//旋转角度
	unsigned int c_timeout;
	unsigned int c_go_timeout;
}last_turn_staT;
extern unsigned char turn_check_bum_pro(unsigned char flag,unsigned char dir,int pwm,int turn_speed);
extern char test_go_line(void);
extern void wifi_motor_single_run(uint8_t dir);
#endif

#endif

