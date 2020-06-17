#ifndef _MAP_H_
#define _MAP_H_

#define DIR0		0
#define DIR180		1
#define DIR90		2
#define DIR270		3


#define MAPEDGE_DRAW			0x00	//画边缘
#define MAPEDGE_UNCLR			0x01	//找未清扫点
#define MAPEDGE_UNCLR_ROUTER	0x02	//未清扫点，且能导航过去
#define MAPEDGE_UNCLR_NOBACK	0x03	//未清扫点，不能回头
#define MAPEDGE_CLR_FRONT		0x04	//找未清扫点
#define MAPEDGE_TO_UNCLR		0x05	//延边去找未清扫点



#if LAB_DEMO_VERSION		//实验室演示版本
#define GET_MAX_I()	{if(sys->t_navi_work > 3600)	\
						maxi = 18;					\
					else							\
						maxi = 1;					\
					}
#else						//正常版本
#define GET_MAX_I()	{if(sys->t_navi_work > 1000)	\
						maxi = 18;					\
					else							\
						maxi = 5;					\
					}
#endif


#define CHECK_X_LINE  	0x00
#define CHECK_Y_LINE	0x01


#define MAX_ZLINE_C	     4//	7
#define MIN_ZLINE_FITT	 1//	4


#define UNCLN_STA		0	//未清扫
#define SCAN_STA		1	//清扫
#define OBST_STA		2	//障碍物

#define MAX_NAVI_ERR2	4



void side_obst_by_ir(void);
void side_obst_by_bum(uint16_t gsta);
int side_drap_map(navigat_t *navi,uint8_t side,int max);
void check_side_obst(SIDEOBST *side_obst);
short get_linex_obst(short x,short y,short *ox,short *oy,int dir);
short get_linex_len(short x,short y);
void set_osbt_ir(short x,short y,uint16_t ir,uint8_t value,uint8_t type,char *str);
void insert_side_obst(SIDEOBST *side_obst,short x,short y,uint16_t ir);
OBST *get_side_obst(SIDEOBST *side_obst,short x,short y,short tx,short ty);
//short map_coincide(navigat_t *navi,SIDEOBST *m_obst);
short verify_on_map(navigat_t *navi,short max_grid);
short check_map_reason(short tx,short ty,short lx,short ly);
void save_now_point(navigat_t *navi);
void calc_point_offset(navigat_t *navi);
void restor_point(navigat_t *navi);

int fitting_map(navigat_t *navi,SIDEOBST *m_obst,short len1,short ofset,short mix_cn);

void side_mobst_by_ir(SIDEOBST *mobst,uint16_t ir,uint8_t side);
void side_mobst_by_bum(SIDEOBST *mobst,uint16_t gsta);
void front_obst_by_bum(void);


void ajust_xy_org(int x,int y,int peer,uint8_t type);
int16_t side_obst_to_map(SIDEOBST *mobst,int16_t x_ofs,int16_t y_ofs,uint8_t is_save);
uint8_t the_same_side(OBST *obst1,OBST *obst2);
int coin_line_90_270(OBST *mobst,int16_t c_obst,int16_t mac,int16_t *x_ofs);
int coin_line_0_180(OBST *mobst,int16_t c_obst,int16_t mac,int16_t *y_ofs);

void dist2obst(short dist,float angle,int m_x_org,int m_y_org,short *xobst,short *yobst,int *x_org_obst,int *y_org_obst);
int coordinate_calcu(uint8_t type);

uint8_t unclr_line(int16_t tx,int16_t ty,uint8_t type);
int coordinate_calcu_s(uint8_t side);

uint8_t xline_coin(Z_XLINE *zline,int16_t x_ofst,int16_t y_ofst,int16_t min_c,uint16_t count);

uint8_t xline_fitting_map(Z_XLINE *zline,int16_t min_xline,int16_t max_xline);
uint8_t unclr_side(int16_t bx,int16_t y,int16_t ex);

int16_t obst_line_on_90_270(int16_t tx,int16_t ty,int16_t grid,int16_t *ox,int16_t *oy);
uint8_t get_both_ends_0_180(int16_t x,int16_t y,int16_t *bx,int16_t *ex);
uint8_t obst_line_90_270(int16_t tx,int16_t ty,int16_t y_ofs_90,int16_t y_ofs_270);
uint8_t obst_line_0_180(int16_t tx,int16_t ty,int16_t x_ofs_0,int16_t x_ofs_180);
int16_t obst_line_on_0_180(int16_t tx,int16_t ty,int16_t grid,int16_t *ox,int16_t *oy);
uint8_t robot_whele_stop(uint8_t type);

uint8_t point_sta(int16_t x,int16_t y);
void insert_edge_table(int16_t x,int16_t y ,int x_org,int y_org ,float angle);
int16_t near_mapedge_walk(uint8_t nside,uint8_t type,uint16_t max_line,int16_t xdir,int16_t ydir,uint16_t mlen,MEDGE  *m_edge,int16_t tox,int16_t toy);
uint8_t unclr_by_edgeways(int *angle,uint8_t *nside,uint8_t type);
uint8_t map_back_clean(int16_t tx,int16_t ty,int16_t tx1,uint8_t side);
float format_agle(float angle2,uint8_t type);
uint8_t get_nearwall_back_sta(uint8_t *nside,int16_t *xdir,int16_t tox,int16_t toy);
uint8_t is_unclear_side(int16_t x,int16_t y,int16_t x1,int16_t y1,int16_t ydir,int16_t *c_unclean,int16_t *c_clean);
uint8_t unclr_near_point(int16_t tx,int16_t ty);
uint8_t unclr_nearwall(int16_t tx,int16_t ty,uint8_t nside);
uint16_t get_mapedge_len(void);
int coordinate_calcu_test(void);
uint8_t check_unclr_round(int16_t ntx,int16_t nty,uint8_t side);
uint8_t around_unclr_check(int16_t tx,int16_t ty,uint8_t nside,int8_t lenth,int8_t wide);		//判断tx,ty,周围area*area个点是否为未清扫点



#endif

