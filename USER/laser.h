#ifndef _H_LASER_
#define _H_LASER_



#define RET_DRAW_MAP_NEAR_ERR		0x00	//延边画地图错误
#define RET_DRAW_MAP_NEAR_BUM		0x01	//延边画地图碰撞
#define RET_DRAW_MAP_NEAR_LOST		0x02	//延边画地图丢失

#define LINE_LEN		13





#define MAX_ADJ_XY	50

#define RADIUS_CM    	7//17//半径厘米
#define RADIUS_WH    	450//半径厘米 17*29


#define LASER_CALC_TURN		0x00		//转弯扫描
#define LASER_CALC_GO_FW	0x01		//直行扫描

#define RED_LASER	0
#if RED_LASER
#define LASER_LOST_DIST			888
#define NO_DIST					888
#define MAX_LASR_DIST			500

#else
#define LASER_LOST_DIST			8000
#define NO_DIST					8000
#define MAX_LASR_DIST			3000
#define MIN_LASER_DIST			100
#endif

#define MAX_LAST_DIST		25		//扫描的时候，确认最后一个是否需要取

#define MAX_OBST_STORAGE	10

#define ORG_TO_MM(X)		(X / CM_PER_HW)
#define MM_TO_ORG(X)		(X * CM_PER_HW)
#define MM_TO_XY(X)			((X / LASER_RES)	+	100)




struct dist_t
{
	uint8_t 	begin;
	uint8_t		crc;
	uint16_t 	dist;
};
//ó?óú????μ?áùê±±?′?′￠D??￠
#define MAX_OBST_FIFO		200
struct obst_fifo_t
{
	int 	len;
	OBST 	obst[MAX_OBST_FIFO];
	int 	dist[MAX_OBST_FIFO];
	float	deg[MAX_OBST_FIFO];
	
};





extern volatile uint16_t l_dist;

//extern LASER *laser;

void init_laser(void);
int perform_laser_dis(uint16_t *dist);
int laser_scan_dist(navigat_t *navi,int type);
void laser_test(uint8_t dir,int pwm,int hw);
void laser_turn_360(uint8_t dir);
void begin_scan(navigat_t *navi);
int check_line(int type,int x,int y,int c);
int get_angle_4(LASER *laser,int *dist_0,int *dist_90,int *dist_180,int *dist_270);
int calc_scan_laser(navigat_t *navi,int save);

int ajust_xy_by_laser(navigat_t *navi ,int dist,float angle,int dis);
int side_obst_count(int x,int y);
int laser_go_forward(void );
void scan_to_obst_map(LASER *laser ,struct obst_fifo_t *obst_f,int m_x_org,int m_y_org);
void check_ajust_sta(struct obst_fifo_t *obst_f,int *x_ok,int *y_ok);
int ajust_xy_scan_obst(struct obst_fifo_t *obst_f,int *x_ok,int *y_ok,int *m_x_org,int *m_y_org);



void set_osbt_l(int tx_org,int ty_org,int x,int y,int x_org,int y_org,uint8_t value,uint8_t type,int dist,float angle,char *str);
int laser_adj_handle(navigat_t *navi,struct obst_fifo_t *obst_f,int m_x_org,int m_y_org);


int laser_calc_move(navigat_t *navi,int max_dist);
int laser_move_scan(void);

//char motor_go_draw_map(navigat_t *navi);
void robot_lasr_near_run(uint8_t gSta);
int laser_move_adj(navigat_t *navi,int sta);
void laser_turn_deg(uint8_t dir,int pwm,float agle,int d_min);
void laser_turn_for_v(uint8_t dir,int pwm,float agle);
void laser_shake(uint8_t dir,int pwm,float agle);
void laser_turn_scan(uint8_t dir,int pwm,float agle);
int laser_cross_adj(navigat_t *navi,char type);
void laser_turn_for_adjust(uint8_t dir);

#if TEST_FOR_LABO
//测试版本，在实验室里，延边走出来
int test_for_near_wall(int *lturn);
#endif
#endif
