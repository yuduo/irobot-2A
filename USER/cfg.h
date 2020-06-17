

#ifndef _CFG_H
#define _CFG_H

#define CFG_ADDER	 0x0807F000
// #define CFG_ADDER	0x08032000
#define MAX_CFG_LEN		512//256



#define MIN_CLIFF_SENSOR_VALUE		cfg->cliff_ala_max//2200

#define T_CLIFF_SENSOR_ERR			cfg->cliff_ala_tmr//600000			// 10分钟持续


#define IC_DOCK_TURN_SPEED			0		//回充转圈速度
#define IC_DOCK_M_WALK_SPEED		1		//回充中线速度
#define IC_DOCK_WALK_SPEED			2		//回充速度
#define IC_DOCK_SET_ANGLE			3
#define IC_AGI_PID					4		//滚刷速度环
#define IC_BUBER_ERR_T				5		//碰撞条的故障时间
#define IC_DUSTBOX_MAX				6		//尘合满阀门值
#define IC_DUSTBOX_T				7		//尘合满时长
#define IC_DUST_MID				8		//灰尘浓度中
#define IC_DUST_HIGH			9		//灰尘浓度高
#define CARPET_L_PRA			10		//左轮地毯参数
#define CARPET_R_PRA			11		//右轮地毯参数
#define IC_AGI_AMP_FACTOR		12			//中扫系数
#define IC_LMOT_AMP_FACTOR			13			//左轮系数
#define IC_RMOT_AMP_FACTOR			14			//右轮系数

#define IC_FAN_CUR_ERR				15			//风机电流过大告警阀门
#define IC_FAN_S_LRCUR_ERR			16			//静音模式风机堵转错误
#define IC_FAN_N_LRCUR_ERR			17			//正常模式风机堵转错误
#define IC_FAN_T_LRCUR_ERR			18			//强力模式风机堵转错误
#define IC_FAN_LRCUR_TMR			19			//风机堵转时间判断阈值
#define IC_LED_EN					20			//是否开关灯

//-------------------------------------------add by wonton2004 @2020.6.2
#define IC_FLOORSENSOR_EN					21			//地毯检测是否打开
#define IC_DUSTSENSOR_EN					22			//灰尘浓度检测是否打开
#define IC_EDGE_STATUS						23			//沿边功能是否打开,这个就在这里存一下,实际功能先不做
//-------------------------------------------add end@2020.6.2





/*
正常程序
速度测试
红外测试
传感器测试
*/
#define DEBUG_NO		0x00
#define DEBUG_PWM		0x01
#define DEBUG_IRD		0x02
#define DEBUG_SES		0x03
#define DEBUG_MOTER		0x04

#define MAGIC 0x55AC
struct cfg_t
{
	uint16_t	magic;

	//回冲调试信息
	//uint8_t	 dock_stop_step[8];		//停止步
	uint8_t	 msg;
	uint8_t  crc;
	uint16_t kp1;
	uint16_t ki1;
	uint16_t kd1;
	uint16_t kp2;
	uint16_t ki2;
	uint16_t kd2;

	// 9
	uint16_t ajust_agle;		// 	0
	uint16_t gyro_ajust;		//	 陀螺仪的调整
	uint16_t midle_adc;			//	1 中间红外的红外范围
	uint16_t stop_timr;			//	2 前方发现红外后，继续前行的时间。

	//13
	uint16_t min_left_adc;		//
	uint16_t lock_left_adc;
	uint16_t max_left_adc;
	uint16_t side_left_adc;
	uint16_t lost_left_adc;

	uint16_t min_right_adc;		//8
	uint16_t lock_right_adc;
	uint16_t max_right_adc;
	uint16_t side_right_adc;
	uint16_t lost_right_adc;	// 12

	//23
	uint16_t m_left_adc;		//	13中左红外
	uint16_t m_right_adc;		//中右红外

	uint16_t slow_left;			//转回来的步数
	uint16_t slow_right	;
	int16_t  gyro_ofs1;
	int16_t  gyro_ofs2;

	uint16_t c_pid_near;			//PID计算趋势变化的次数
	uint16_t a_pid_near;			//PID计算趋势变化的大小


	// 35
	uint16_t l_c_turn;			//第一次转圈，转到切中线的个数
	uint16_t dock_l_delay;
	uint16_t dock_l_count;			//转过来到中线的个数

	uint16_t r_c_turn;
	uint16_t dock_r_delay;
	uint16_t dock_r_count;			//转过来到中线的个数


	uint16_t slow_mid;		//3100
	uint16_t slow_pwm;
	uint16_t turn_pwm;
	//78
	uint16_t walk_pwm;
	uint16_t v6;

	//46
	uint16_t	h_run_speed;		//高速
	uint16_t 	m_run_speed;		//从右直走延时
	uint16_t	l_run_speed;		//从左转入中线的计入个数

	//滚刷速度
	uint16_t h_agi_speed;
	uint16_t l_agi_speed;

	uint16_t turn_speed;
	uint16_t nearwall_pwm;
	uint16_t nearwall_hw;

	uint16_t nearwall_lost;
	uint16_t nearwall_cnt;

	uint16_t nearwall_dit;		//沿边丢距离
	uint16_t t_agi_max;			//滚刷告警时长
	uint16_t i_agi_max;			//滚刷告警门限
	uint16_t i_left_whele_max;	//左轮告警门限

	uint16_t t_dust_box_max;		//尘合检测时长

	uint16_t i_whele_max;		//右轮告警门限
	uint16_t t_whele_max;		//轮组告警时长

	uint16_t  cliff_max;		//悬崖传感器门限
	uint16_t  cliff_ala_max;	//告警门限
	uint16_t  cliff_ala_tmr;	//告警时长,秒钟为单位

	uint16_t  t_drop_max;		//轮子悬空门限
	uint16_t  v_dust_max;		//灰尘度阀门值
	uint16_t  v_dust_cnt;		//灰尘度计数

	uint16_t  v_phts_led;		//光敏电阻的上限
	uint16_t  t_phts_led;		//光敏电阻的时间

	uint16_t  spot_speed_maxc;		//定点的速度
	uint16_t  spot_step_disa;		//不长和圈数
	//69
	uint16_t r[37];//

	uint16_t t_agitator_inspection;	//滚刷检测时间
	uint16_t t_agitator_replacemen;//滚刷更换时间     69*2

	uint16_t steam_clean_id;		//流数据传输的ID    49 *2
	//84
	uint32_t dust_full_time_alarm;		///3
	uint32_t strainer_use_time_alarm;	///4
	uint32_t side_rush_use_time_alarm;	///5
	uint32_t main_rush_use_time_alarm;	///5 //bytes 102    57 * 2
	uint8_t sn[21];//机器序列号,长度为21字节,即21个字符,最后一个字节为主板的测试状态,0为通过,0xff为未知(或未测试),其他200以下的值为测试不通过的次数
	uint8_t uc[1];// 232+24=256

	uint8_t wifi_config_able;			//wifi模组首次配网(重置功能)  	257
	uint8_t dust_pwm_lev;				//风机等级					258
	uint8_t speaker_able;				//喇叭使能					259
};

struct robot_msg_t
{
	//uint16_t	magic;
	uint8_t	 	msg;
	uint8_t 	crc;
	uint16_t	data[62];

};

#define SYS_DEBUG_LEN	30
struct sys_debug_t
{
	uint8_t		msg;
	uint8_t		crc;
	uint16_t 	imu;
	uint16_t 	gyro;
	uint16_t	pid1;
	uint16_t	r[12];

	uint16_t 	c_gyro;
	uint16_t 	c_imu;
	uint16_t	c_pid1;
	uint16_t	c_r[10];
	uint8_t		debug;
	uint8_t 	pwm;

};

struct h_cfg_t
{

	uint16_t magic;
	uint8_t	msg;
	uint8_t crc;
	uint16_t dock_hw_back;			//回头的步数
	uint16_t dock_hw_forwak;			//前冲的步数
	uint16_t dock_hw_turn;			//转回来的步数
	uint16_t dock_pwm;
	uint16_t dock_m_pwm;
	uint16_t dock_speed;
	uint16_t pwm_normal;
	uint16_t pwm_slow;
	uint16_t speed_normal;
};
extern struct cfg_t *cfg;
extern uint16_t NOMORL_PWM,SLOW_PWM;
extern char cfg_buf[];

extern struct sys_debug_t sys_debug;

void read_Flashbuf(u32 addr,u8 *p,u16 n);


void save_cfg(void);
void init_cfg(void);
void reset_cfg(void);
void proc_rx_uart(uint8_t *buff,int len);

#endif
