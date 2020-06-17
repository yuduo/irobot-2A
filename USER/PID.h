#ifndef _PID_H
#define _PID_H


#define NEAR_PID_NEW		1

#define TURN_SPEED					cfg->turn_speed//20    //20
#define CHARGE_TURN_SPEED			(cfg->r[IC_DOCK_TURN_SPEED] / 100)		//30		//回充转速，高位  7695 = 1E0F ,高位 1E = 30 低位 0F = 15
#define CHARGE_TURN_SPEED_DELAY		(cfg->r[IC_DOCK_TURN_SPEED] %100)//15	//回充转速，低位



#define SINGLE_NAER_PID		cfg->nearwall_cnt//20


//#define EN_PID		1

//#define PID_DEBUG	0


/*
#define BLDC_MAX_SPEED   4096	 //800 3500
#define BLDC_MIN_SPEED   1
#define PWM_Max			1400
#define PWM_Min     	10
*/
//测试，用于低速测试，怕伤己伤人啊！
#define BLDC_MAX_SPEED   80	 //800 3500
#define BLDC_MIN_SPEED   1
#define PWM_Max			200
#define PWM_Min     	10




typedef struct near_pid_t
{
	//unsigned char 	sta;		//状态 0 - 不延边 1-延
	unsigned char		level;		//延边等级
	unsigned char 	obst;		//侧面是否有障碍物，侧面有障碍物的话，延边丢失，不能做PID
	unsigned char 	dist_ok;	//矫正距离符合要求

	uint8_t 		debug;

	unsigned short	min_adc;
	unsigned short	max_adc;
	unsigned short	max_adc2;
	unsigned short	lock_adc;

	unsigned short 	real_lock;
	unsigned short    real_max;

	unsigned short	last_adc;

	unsigned short	count;
	//unsigned short 	c_near;		//沿边的次数
	//unsigned short 	c_near2;
	unsigned short 	c_lost;		//计算失去墙的时间
	unsigned short	c_dis_a;	//角度计数差异的时间
	//unsigned short	max_lost;
//	unsigned short	c_angle;	//角度的列表
	unsigned short	c_big_change;
	unsigned short 	dist;

	unsigned short	c_lost_flag; 	//失去墙的标志    2018-11-03 add

	unsigned short	small_dist;		//矫正期间的小距离

	unsigned short 	*adc;		//主红外
	unsigned short	*adc2;		//侧面第二个红外
	float		l_angle;	//丢失时候的角度
	float		a_go_outside;	//往外走的角度
	float		a_go_inside;	//往里走的角度
	float 		*yaw;			//系统角度
	float		*angle;			//目标角度
	uint16_t    *c_pid_near;
	uint16_t 	*a_pid_near;
	uint16_t	*nearwall_lost;
	uint8_t 	*nearwall_signal_lost;
	int			*distance;
	uint16_t 	*c_near_a;
	uint16_t	*max_near_a;
	float 	*yaw_buff;
	

}NEAR_PID;



extern 	NEAR_PID  r_near_pid,l_near_pid;

typedef struct _pid_t{
	uint8_t debug;
	uint8_t r1;
	float SetSpeed;			
	float ActualSpeed;		//定义实际值
	float err; 				//定义偏差值
	float err_next;	//定义上一个偏差值
	float integral;
	float err_last;//定义最上前的偏差值
	float Kp,Ki,Kd;	//定义比例、积分、微分系数
	float out;
	int16_t  *max_pwm;
	int16_t  *min_pwm;
	int		*rpwm;
	int		*lpwm;
	int		*pwm;
	int 	*setminpwm;
	int		*setmaxpwm;
	
	//float angle;
//	float max;
	//int	  c_pid;
}pid_t;


extern  pid_t rpid,*pid;

//extern pid_t lpid,rpid;

void pd_gyro_int(int out);
void proc_hw_pid(void);


int navigat_near_wall_pid(float *agle,int c_lost);

int near_wall_pid(unsigned char nsta,NEAR_PID *npid,float *agle);
void init_near_pid(char level);
void turn_round_pid(int sta);

//#if JUDGE_PID_CTRL_WHEEL_STOP
//void m_speed_pid_ctrl(unsigned short speed,unsigned char type,int hw,int c_left_hw_min,unsigned char ird_state,int hw_min,int walk_dist_min);
//#else
void m_speed_pid_ctrl(unsigned short speed,unsigned char type);
//#endif
void charge_turn_pid(int sta);
int  m_nearwall_pid(void);
void init_nearpid_differ(int pwm,unsigned short speed,unsigned char msta);
signed short dock_speed_pid_ctrl(unsigned short min,unsigned short max,unsigned char type);
void single_whele_pid(int rspeed,int chw,volatile uint16_t *whele_pwm);
char proc_nearwall_pid(float set_angle,uint8_t nside);

void m_speed_pid_indep(uint16_t lspeed,uint16_t rspeed,uint8_t type);
#endif
