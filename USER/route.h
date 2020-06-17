
#ifndef _ROUTE_H_
#define _ROUTE_H_


#define MAX_XLINE		1000	


#define NAVI_MT_OK			0x00		//导航到点
#define NAVI_MT_ROUT_ERR	0x01		//搜索路径失败
#define NAVI_MT_GO_ERR		0x02		//导航过去失败


/*
//路径
typedef struct route_t
{
	XLINE_T 	*prev;		//上一步的路径
	XLINE_T  	*xline;
}ROUTE_T;
*/
typedef struct area_t
{
	struct area_t *fath;		//父节点，父区域
	uint16_t	xlen;		//路线的长度
	XLINE_T  	*xline;
	XLINE_T 	*bxline;	//开始的
}AREA_T;

//路径数据
typedef struct router_t
{
	
	//uint16_t 	alen;					//区域的总数
	uint16_t	xline_len;				//路径总数
	uint16_t	max_len;
	uint16_t 	index;					//区域序号
	uint16_t	t_begin;				//开始时间
	uint16_t	t_end;					//结束时间
//	AREA_T 		*area;					//当前的清扫区域
	XLINE_T		xline[MAX_XLINE];		//走过的路径
//	AREA_T   	g_area[MAX_AREA];		//总区域。
}ROUTER_T;


extern ROUTER_T *rout;

void init_router(void);
AREA_T *get_area(void);
void insert_route(AREA_T *erea, XLINE_T *xline);
XLINE_T *insert_x_line(int16_t bx,int16_t by,int16_t ex,int16_t ey);
XLINE_T *get_xline(int16_t tx,int16_t ty);
uint8_t search_unclr_in_erea(int16_t *tox,int16_t *toy);
uint8_t navigat_motion(short tox,short toy,XLINE_T *xline);
uint8_t search_unclr_in_erea_near(int16_t tx,int16_t ty,int16_t *tox,int16_t *toy,int16_t y_min,int16_t y_max,uint8_t notrouter,uint8_t kside);
uint8_t unclr_xline_by_point(int16_t ty,int16_t *tox,int16_t *toy,uint8_t can_rout);
uint8_t get_org_coord(int16_t *tox,int16_t *toy);
uint8_t navi_walk_map(short x,short y);
uint8_t check_repeat_walk(void);
uint8_t unclr_xline_near(XLINE_T *xline,int16_t tx,int16_t ty,int16_t *tox,int16_t *toy);


#endif

