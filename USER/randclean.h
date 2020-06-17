
#ifndef _H_RANDCLEAN_
#define _H_RANDCLEAN_

#define RMCLN_RM	0x00		//随机
#define RMCLN_NEAR	0x01		//贴墙

typedef struct rm_cln_t
{
	uint8_t gSta;		//清扫模式
	uint8_t slip;
	uint8_t	is_near;
	uint8_t	c_turn_near_right;		//转了多次后，就不让再转了。
	uint8_t	short_distance;			//是否长时间的短距离运行。
	uint16_t	c_near_wall;		//沿墙了多次时间
	int		check_lost;
}RmCln_t;

void proc_random_task(void);
void init_random_task(void);
char get_rand_idx(void);
void turn_vertical_right(void)	;
void turn_near_right(void);
//void near_wall_pid(void);
void proc_test_task(void);
//void near_wall_pid(float *agle,int c_lost);

char motor_go_forwark_for_test(void);
void robot_usart_test_cmd(void);


#endif
