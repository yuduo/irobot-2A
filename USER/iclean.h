#ifndef _ICLEAN_H
#define _ICLEAN_H



#define PRINTF		log_printf


// #define CFG_ADDER	 0x08019000

//系统工作状态，sys->sSta
#define SYS_IDLE		0x00		//空闲
#define SYS_NAVIGAT		0x01		//导航
#define SYS_FOCUS		0x02		//重点
#define SYS_RANDOM		0x03		//随机
#define SYS_DOCK		0x04		//回充
#define SYS_NEAR		0x05//沿边
#define SYS_CHARGE		0x06		//充电
#define SYS_TEST_BD		0x07		//?÷°?2aê??￡ê?
#define SYS_POWEROFF	0xfd	//1??ú,′?×′ì?????á???μ?oìía(′??·?ú1.2μ?ó2?t°?±????T·¨1?±?)/ò￡???óê?,????è?2?1?±?
#define SYS_TEST_SEFT	0x08		//?÷°?2aê??￡ê?
#define SYS_WALK		0x0A       //直行
#define SYS_DOCK_NEAR	0x0B		//回充延边





//运动状态
//#define ROBOT_MOVE_STA() ((sys->sState ==SYS_NAVIGAT) || (sys->sState==SYS_FOCUS) || (sys->sState==SYS_DOCK)||(sys->sState==SYS_NEAR))
//往里添加了随机模式,在台架测试的时候要用到
#define ROBOT_MOVE_STA() ((sys->sState ==SYS_NAVIGAT) || (sys->sState==SYS_FOCUS) || (sys->sState==SYS_DOCK)||(sys->sState==SYS_NEAR)||(sys->sState==SYS_RANDOM)) // modified by wonton2004 20170603

#define CLEAN_RANDOM	0x00
#define CLEAN_NEAR		0x01	//沿边模式
#define CLEAN_ROUND		0x02	//螺旋模式

#define MAX_IRDA 		8

#define DOCK_NO			0x00
#define DOCK_CHECK		0x01


#define RET_CHECKING	0x00
#define RET_CHECK_OK	0x01
#define RET_CHECK_FAL	0x03



#define FIND_MID_FLAG		0x1
#define FIND_MID_R_FLAG		0x2
#define FIND_MID_L_FLAG		0x4
#define FIND_LL_FLAG		0x8
#define FIND_RR_FLAG		0x10
#define FIND_NEAR_FLAG		0x20
#define FIND_DOC_BASE		0x40
#define FIND_L_M_FLAG		0x100
#define FIND_R_M_FLAG		0x200
#define FIND_ALL_FLAG		(FIND_MID_FLAG|FIND_MID_R_FLAG|FIND_MID_L_FLAG|FIND_LL_FLAG|FIND_RR_FLAG)



//机器错误代码
#if 0
#define MWERR_NONE			0
#define MWERR_CLRDUSTBOX	1//请清洁集尘盒与吸尘口
#define MWERR_MIDMOTOR		2//请检查中扫是否缠绕
#define MWERR_DUSTBOX		3//请检查集尘盒是否正确安装
#define MWERR_WHEELSUSP		4//请检查左右轮是否离地
#define MWERR_WHEELBIND		5//清检查左右轮是否缠绕
#define MWERR_FLRSENSOR		6//请检查地面感应器
#define MWERR_SIDEMOTOR		7//请检查边扫是否缠绕
#define MWERR_FRTMOTOR		8//请检查前轮是否缠绕
#define MWERR_POWEROFF		9//请检查电源开关是否打开
#define MWERR_DISPSTA		10///摄像头错误
#define MWERR_ESCAPE		11		//脱困失败
#else
//机器错误代码
#define ERR_NONE									0
#define ERR_MIDMOTOR								1//请检查中扫是否缠绕				-ok
#define ERR_LEFT_WHEELBIND							2//清检查左轮是否缠绕				-ok
#define ERR_RIGH_WHEELBIND							3//清检查右轮是否缠绕				-ok
#define ERR_DUSTBOX									4//请检查集尘盒是否正确安装		-ok
#define ERR_FAN_FILTER_BLOCKAGE						5//风机滤网堵住.					--need add3
#define ERR_BUMPER_FUNCTION_ERR						6//碰撞错误						-1
#define ERR_FLRSENSOR								7//请检查地面感应器				-ok
#define ERR_WHEELSUSP								8//请检查左右轮是否离地			-ok
#define ERR_CANT_ESCAPE_WITHIN_TIMES				9//脱困困难						-ok
#define ERR_SUCTION_MOTOR_MALFUNCTIONG				10//风机堵转 3档分别电流很大		--need add3
#define ERR_CLRDUSTBOX								11//尘满检测 时间累积				--1
#define ERR_AGITATOR_INSPECTIONG					12//检查滚刷						--need add3
#define ERR_AGITATOR_REPLACEMENT					13//更换滚刷 时间累积				--1

#define MWERR_NONE									ERR_NONE
#define MWERR_MIDMOTOR								(BIT_STA(ERR_MIDMOTOR-1))		//请检查中扫是否缠绕					-ok
#define MWERR_LEFT_WHEELBIND						(BIT_STA(ERR_LEFT_WHEELBIND-1))//2//清检查左轮是否缠绕				-ok
#define MWERR_RIGH_WHEELBIND						(BIT_STA(ERR_RIGH_WHEELBIND-1))//3//清检查右轮是否缠绕				-ok
#define MWERR_DUSTBOX								(BIT_STA(ERR_DUSTBOX-1))//4//请检查集尘盒是否正确安装					-ok
#define MWERR_FAN_FILTER_BLOCKAGE					(BIT_STA(ERR_FAN_FILTER_BLOCKAGE-1))//5//风机滤网堵住.						--need add3
#define MWERR_BUMPER_FUNCTION_ERR					(BIT_STA(ERR_BUMPER_FUNCTION_ERR-1))//6//碰撞错误					-ok
#define MWERR_FLRSENSOR								(BIT_STA(ERR_FLRSENSOR-1))//7//请检查地面感应器								-no
#define MWERR_WHEELSUSP								(BIT_STA(ERR_WHEELSUSP-1))//8//请检查左右轮是否离地					-ok
#define MWERR_CANT_ESCAPE_WITHIN_TIMES				(BIT_STA(ERR_CANT_ESCAPE_WITHIN_TIMES-1))//9//脱困困难				-ok
#define MWERR_SUCTION_MOTOR_MALFUNCTIONG			(BIT_STA(ERR_SUCTION_MOTOR_MALFUNCTIONG-1))//10//风机堵转 3档分别电流很大		-no
#define MWERR_CLRDUSTBOX							(BIT_STA(ERR_CLRDUSTBOX-1))//11//尘满检测 时间累积							--1
#define MWERR_AGITATOR_INSPECTIONG					(BIT_STA(ERR_AGITATOR_INSPECTIONG-1))//12//检查滚刷					-ok
#define MWERR_AGITATOR_REPLACEMENT					(BIT_STA(ERR_AGITATOR_REPLACEMENT-1))//13//更换滚刷 时间累积					--1


#endif
/*
struct irda_t{

	uint8_t freq;		//频率
	uint8_t indx;
	uint8_t data[8];
};


struct r_ird_t
{
 	uint32_t t_last;  //最后中断时间。。
	uint8_t rx_idx1;
	uint8_t rx_rbe1;
	uint8_t rx_byte1;
	uint8_t l_360;
	uint8_t left;
	uint8_t right;
	uint8_t r_360;

};
*/
#define CONFIG_SIZE		14
struct config_t
{
	//回冲调试信息
	uint8_t	 stop_step[8];		//停止步
	uint16_t hw_back;			//回头的步数
	uint16_t hw_forwak;			//前冲的步数
	uint16_t hw_turn;			//转回来的步数

};
#define MAX_NEAR_ANGLE		100

extern struct r_ird_t iLeft,iRight,iL360;
#define MAX_ADC_C		9

//延边PID算法
/*
typedef struct near_wall_type
{
	uint8_t c_lost_wall_right;
	uint8_t	c_near_wall_right;
	uint8_t c_lost_wall_left;
	uint8_t	c_near_wall_left;
}near_wall_t;
*/
#define WORK_STA_ERR		(2)
#define WORK_STA_SILENT		2
#define WORK_STA_WIFI		3

#define WORK_MOD_DOOR		(1)
//#define WORK_MOD_ORDER		(1<<1)
#define WORK_MOD_MOP		(1<<2)
#define WORK_MOD_ORDER		(1<<7)  //预约

#define NO_CHARGE			0
#define AC_CHARGE			1
#define DOCK_CHARGE			2
#define CHARG_DONE			4


#define STACK_LEN		4096
#if NEAR_DRAW_MAP
	#define STACK_LEN2		256
#else
	#define STACK_LEN2		4//4096
#endif



extern uint8_t m_stack[STACK_LEN];
extern uint8_t m_stack2[STACK_LEN2];			//局部使用
//----------------------------------------------------------------------add by wonton2004 20170115
//work_mod
#define MWO_SILENT		0x01	//底档
#define MWO_NORMAL		0x02	//自动挡
#define MWO_TURBO		0x04	 //高档
#define MWO_MANUAL		0x08	//手动挡
#define MWO_FULL2GO		0x10	//FULL AND GO
#define MWO_SPOT		0x20	//集中清扫
#define MWO_SCH			0x80	//预约清扫标记
#define DUST_LOW 			0x00	//吸尘低
#define DUST_MEDIUM 		0x01	//吸尘中
#define DUST_HIGH 			0x02	//吸尘高

#define MSINFO_DUSTFULL	0x01
#define MSINFO_ERROR	0x02
#define MSINFO_MUTE		0x04	//静音
#define MSINFO_DCJACK	0x10//DC头插入/手动充电
#define MSINFO_CON_WIFI		0x80	//WIFI配网状态
#define MSINFO_CON_WIFI2	0x40	//WIFI网络状态2
#define MSINFO_WIFI			0x08	//WIFI网络状态

//--------------------------------------------------------------------------------------------end

#define ERR_STA_PICK_UP		(1<<0)		//


#define BUM_V_WALL			(1<<0)		//虚拟墙
#define BUM_DOOR			(1<<1)		//门槛

#define LEFT_SIDE_FALL		1
#define RIGHT_SIDE_FALL		2

#define MAX_YAW_FIFO		1000
#define MAX_FRONT_HW		20

#define ERP_LEN			12

#define WIFI_STA_STANDBY		0x00
#define WIFI_STA_AUTO			0x01
#define WIFI_STA_SPOT			0x02
#define WIFI_STA_PAUSE			0x04
#define WIFI_STA_FULL2GO		0x08
#define WIFI_STA_FINISH			0x10
#define WIFI_STA_CHARGE			0x20


struct sys_t
{
	//存储的数据
	uint32_t 	t_area;		//总面积,单位:平方米
	uint32_t	t_clean;	//总清扫时间,单位:分钟

	uint16_t 	m_area;		//本次面积,单位是平方米
	uint16_t	m_worktime;//本次的清扫时间,单位是分钟
#if (1 == SIMPLIFY)
	unsigned char test_line_go_ctrl;
#endif
	FunctionalState dccheck_enable;
	uint8_t	sState;		//系统状太
 	uint8_t 	mState;		//电机状态
 	uint8_t 	auto_sta;	//auto 模式的状态 mpu6500
 	uint8_t voice;			//声音是否开
 	uint8_t	sState_auxi;		//由于sState状态在函数内部会有短时间变化，这里增加一个辅助状态，当短时间变化的情况sState_auxi的最高位为1,低七位为系统状态

	uint8_t	wifi_sta;		//WIFI状态
	
 	uint8_t	spot_flag;
 	uint8_t	pause;			//暂停的标志
 	////uint8_t wifiAreaNoCalc_en;
	uint8_t 	dust_level;
	uint8_t 	gyro_ofs;
	uint8_t		wifi;
	FlagStatus wifi_switch;		//Wi-Fi模组开关
	FlagStatus wifi_switch_old;		//Wi-Fi模组开关(旧)
	uint8_t 	silent;
	uint8_t	power_on;
	uint8_t power_go;
	uint8_t charge_sta;
	uint8_t	dock_near;
	uint8_t fall_sta;		//跌落状态
	uint8_t side_bum;		//侧面碰撞置位
	uint8_t actionwifi;		//wifi 按键响应
	uint8_t  err_sta;
	uint8_t	bum_sta;		//碰撞的状态
	uint8_t walk_bum;		//碰撞
	uint8_t nearwall_signal_lost;
	uint16_t slipping_state;
	uint8_t shut_down_motor;		//????
	uint8_t	navi_run;		//导航清扫
	//调试信息
	uint8_t	near_debug;
#if PID_DEBUG
	uint8_t	walk_pid_debug;
#endif

/*
bit2	湿拖
bit1	预约
bit0	门槛
*/
	uint8_t 	f_work_mode;	//大工作模式  	静音 正常 强力 集中清扫 充满继续 手动
	uint8_t 	work_mod;		//工作模式
	uint8_t 	up_work_mod;	//当前工作模式	静音,正常,强力 充满继续 手动
	uint8_t		work_tmr;		//工作时间，分钟单位
	uint8_t 	full2go;
	uint8_t 	dust_sensor;	//尘检传感器
	uint8_t 	tSta;		//测试的状态
	uint8_t 	gey_key_clear_err;		///按键清除错误标志
	uint8_t 	pair_sta;

//----------------------------------------------------------------------add by wonton2004 20170115
	uint8_t work_options;//工作选项 门槛模式/预约清扫/强力模式/抹布/清扫完成标记等,见宏 MWO_xxx
	uint16_t work_errcode;//工作错误代码 如中扫缠线等,见宏 MWERR_xxx
	uint8_t main_rush_usetime_alarm;
	uint16_t dust_pwm_value;//风机的PWM值,由于有强力模式,用户需要随时调整风机的转速

	uint8_t wifi_led_enable;
//--------------------------------------------------------------------------------------------end
	uint16_t dust_full_check_tmr_min;
	uint32_t strainer_check_tmr_min;
	uint32_t side_rush_check_tmr_min;
	uint32_t main_rush_check_tmr_min;
	uint8_t work_alarmcode;//耗材告警
	uint8_t dust_pwm_lev;	//风机等级
	uint16_t run_speed;
	uint16_t agi_speed;		//滚刷速度

	uint16_t c_escape ;		//脱困次数

	uint32_t t_beep_on;		//告警的喊叫声
	uint32_t t_beep_off;

	uint32_t t_flash_halt;		//暂停键闪烁
	uint32_t t_halt_on;

//----------------------------------------------------------------------add by wonton2004 20170913
#if ROOMA_GYROONLY
 	uint8_t		manState;	//遥控器手动控制模式下的状态
	uint8_t 	man_worktype;//手动行走模式下的计时工作方式,为0时以man_typetimer为计数器计时,为1时表示永不超时

 	uint32_t	man_typetimer;//手动行走模式时的毫秒计数器,为0xffffffff时表示不超时,否则当man_typetimer超过200时跳出动作
 							//如果为-1时表示永不超时
 							//红外遥控时,用超时模式,方向键按下,计数器被重预置为0
 							//wifi遥控时,计数器被写为-1
 	uint32_t	c_cliff_sensor_err;
 	uint32_t	t_cliff_sensor_err;

	uint32_t	sync_loop;//wifi模式下的时钟同步计数器,每24小时同步一次时间

	uint32_t	smt_l_tmout;//左边刷过流后的计时器,当超时后,边刷恢复工作
	uint32_t	smt_r_tmout;//右边刷过流后的计时器,当超时后,边刷恢复工作

	uint8_t	sync_flg;//wifi模式下的时钟同步的标志,这是一个开机同步标识,开机时置0,WIFI正常后,同步一次时间并此值置1
					//之后每隔24小时同步一次时间
	uint8_t charge_done_clean_goon;	//充满电后继续清扫

 	//uint16_t	sWorkTimeMinCt;//工作时间(单为秒)的计数
 	uint16_t	sWorkTimeMin;//工作时间(单为分钟),在非充电/空闲/关机回充等状态下的工作时间,如自动/弓字/随机等
 	//uint16_t	sSleepTimeMin;//睡眠时间
#endif
//--------------------------------------------------------------------------------------------end
		uint16_t 	kdis_yaw;
	uint16_t 	cdis_yaw;
	uint8_t near_wall;	//
	uint8_t slip;
	//uint8_t	from_bum;		//从碰撞转过来的
	uint8_t walk_sta;		//是否走成功

	uint8_t left_dir;
	uint8_t	walk_ctrl;		//运动控制，遥控器控制行走，0不控制行走

	uint8_t nsta;

	//uint8_t low_power;		//低电压

	uint16_t	timeout_navi_work;
	uint32_t 	t_navi_work;	//工作的时间，秒钟为单位




	uint32_t	g_t_walk;
	//	uint16_t 	kdis_yaw;
	//uint16_t 	cdis_yaw;

	//uint8_t 	gIO_ird;

	uint16_t	left_pwm;
	int16_t 	right_pwm;

	uint16_t	c_loop;
	uint16_t 	c_bum;		//碰撞的次数
	uint16_t	lst_distance;
	uint16_t	c_low_power;
	uint16_t	c_charge_power;

	uint16_t 	c_go_bum;
	uint16_t	c_walk_drop;
//	uint16_t	lst_distance;
	uint16_t	tv1;		// 测试参数1
	uint16_t	tv2;		//测试参数2

	uint16_t 	gSta;		//碰撞的状态
	uint16_t	gsta;		//备份的状态
	uint16_t	g_sta[10];	//前撞红外的值
	uint16_t	m_sta[10][MAX_ADC_C];	//红外
	uint16_t	volt;	//电压
	uint16_t 	prs;	//电量百分比
	//uint16_t 	current;	//电流
	uint16_t 	ir_front_1;	//中1 原始值
	uint16_t	ir_front_3;//中2 原始值
	uint16_t	g_buton[2][3];	// g_buton[0][0~3];跌落红外  g_buton[1][0~3] 门槛

	uint16_t c_right_drop;
	uint16_t c_left_drop;
	//int16_t 	gyro_ofs;

	unsigned short ground_ul;

	int16_t  max_pwm;
	int16_t  min_pwm;
	int16_t	 max_front_stop;

	int16_t 	i_c_near;
	uint16_t 	c_near;
	uint8_t 	dist_ok;
	uint8_t		small_dist_ok;

	uint8_t		init_prs;

	//uint8_t c_lost_wall_right;
//	uint8_t	c_near_wall_right;
	//uint8_t c_lost_wall_left;
	//uint8_t	c_near_wall_left;
	// uint8_t	c_lost_wall_right	;
	// uint8_t	c_near_wall_right 	;
	//	uint8_t c_lost_wall_left;

	//充电 算法
	//uint8_t    dock_discovery; 		//


	uint8_t		g_fall_sta;//跌落的状态,bit0为前方 bit1左边 bit2右边
	//uint8_t	near_pid;		//是否在贴墙走
	uint8_t	c_ir_adc;			//adc红外
	uint8_t	motor_power_off;
    uint8_t t_adj_run;

	


	//uint8_t 	lStopSta;		//最后超时停止的状态
		uint8_t		auto_lamp_sta;	//光感自控灯状态
//	uint8_t	c_near_wall_right;
	//uint8_t	c_near_wall_left;
	//uint16_t	c_load;
	//uint16_t  c_load2;
	uint16_t  chw_next;
	uint16_t  gyro_stop;
	uint16_t  c_pid_hw;
	uint16_t  c_front_stop;
	uint16_t  c_front_hw;
	uint16_t  front_hw[MAX_FRONT_HW];

	int16_t		gyro_short;		//陀螺仪原始数据

	//uint8_t	needStop;

	uint8_t	c_hw_diff;	//两个轮子之间的差距次数
	//uint8_t	caleHW;		//是否要计算霍尔数 0:不计算霍尔，1:电机1(左电机)计算，2，电机2计算

	//int		trg_speed;		//目标速度
	//int		trg_speed2;		//目标速度
	uint32_t 	c_hw;//1=0,c_hw2=0;		//

	int		nextPWM;			//保留，做下一步动作的时候可能用到。
	int		next_c_hw;			//保留，做下一步动作的时候可能用到。

	int		pwm1;
	int 		pwm2;
	int		rpwm,lpwm,pwm;

	int setminpwm,setmaxpwm;
	uint16_t	c_left;
	uint16_t  c_right;
	//uint16_t	c_near_angle;

	uint16_t 	c_turn_navi;//mpu6500
	uint16_t c_ajust_agle;
	uint16_t c_fw_rd;		//中间传感器最大值的个数
	uint32_t t_loop;
	uint16_t	c_yaw;

	uint16_t  left_speed;		//记录左轮速度
	uint16_t  right_speed;
	uint16_t  turn_speed;


	volatile uint32_t 	m_cHw1;
	volatile uint32_t	m_cHw2;
	uint32_t	m_speed1;
	uint32_t  m_speed2;
	//uint32_t	tStop;			//中间短暂的暂停时间;
	//uint32_t	t_ss;
	uint32_t	irda[9];
	uint32_t 	tird;
	uint32_t	timeout;
	uint32_t 	t_gyro_ofst;		//陀螺仪初始化的时间超时
	uint32_t	t_idle;				//空闲的时间，空闲时间太长，则关红外
	int 	m_dist;		//记录开始运行时的距离


	uint32_t	adjust_gyro_time;	///调整陀螺仪时间

	//int 	cross_yaw;			//临时变量
	uint16_t c_yaw_ok1;//MPU6500 mjw 20191129
	uint8_t sensens_err;	///传感器错误标志
	uint32_t sys_charge_cur;	//系统充电电流

	

	uint32_t	t_work;		//开始工作的时间
	int16_t	gyro_offset;
	float   langle;
	float	yaw;
	float	angle;
	float	gyro;
	float lspeed,rspeed;
	float	ajust_agle;		//每次修正的角度
	float  total_ajust_agle;
//	float	yaw_ofset;			//角度矫正
//	float 	yaw_fifo[MAX_YAW_FIFO];

	//struct cfg_t	*m_cfg;
//	navigat_t		*m_navi;
	//near_wall_t	near_wall;
	//float  last_near_angle;

//	float	tyaw;

	uint8_t upmap_en_flag;		//上报地图使能标志
	uint8_t uphismap_en_flag;	//上报地图使能标志
	uint8_t upmap_en_flag_first;	//开启上报实时历史地图使能标志
	uint8_t clean_map_flag;		//清除地图标志

	uint8_t sync_wifi;			//wifi时间同步
	uint8_t dis_statechange;	//显控状态变化
	uint8_t wifistatechange;	//WIFI状态变化
	uint8_t wifi_config_state;	//WIFI配网模式
	uint8_t wifi_state;			//WIFI联网状态
	uint8_t wifi_old_state;
	uint8_t wifiAreaNoCalc_en;
	uint32_t wifistatechange_tmr;	//WIFI状态改变时间

	uint8_t dusk_usetime_alarm;			//尘盒使用时间到了告警
	uint8_t strainer_usetime_clear;		//滤网使用时间到了告警
	uint8_t side_rush_usetime_clear;			//边耍使用时间到了告警
	uint8_t main_rush_usetime_clear;		//主刷使用时间到了告警

	uint16_t 	agi_curr;
	uint16_t	left_curr;
	uint16_t	right_curr;
	uint16_t	i_whele_cnt;
	uint32_t	i_left_whele;
	uint32_t 	i_right_whele;

	uint32_t 	t_agitator_inspection;
	uint32_t 	t_agitator_replacemen;

	uint8_t gyro_init_ok;
	////uint32_t wifistatechange_tmr;	//WIFI状态改变时间
//	uint8_t stream_state;
//周期判断左右轮是否堵转相关变量
	uint8_t motor_primary_locked;	//电机好像卡住了标志

	uint8_t motor_locked_flag;	//电机堵转标志


	FlagStatus upgrade_mode;	//升级模式

	FlagStatus Blankets_ground;		//毛毯地面

		FlagStatus wifi_time_sync;
	float dis_yaw[5];
	float vleft;
	float vright;
	float vagi;
};

struct sta_bump_t
{
	unsigned int sec_bum_execute;
	FlagStatus sta_bum_keep;
	FlagStatus sta_bump_op;
};
extern struct sta_bump_t g_sta_bump;


extern struct sys_t *sys;
extern gyro_t kgyro;

//extern struct irda_t  rx_irda,rx_irda1,rx_irda8;

void init_sys(void);
char check_io(uint16_t valu);
char check_io_h(uint16_t valu);

//void insert_irda(uint16_t ird);
void printf_irda(void);

int disXY(int x,int y);
void en_pwm(uint8_t chnl,uint8_t en);
void send_irda(uint8_t chnl,uint16_t bt);
void init_tim3_camp(void);
 //uint8_t  dock_check_dir(void);

void  docking_init(void);
void docking(void);
//void insert_irda1(uint16_t ird);
void rx_ird(struct r_ird_t *rx_ird);

void waiting_stop(void);
//void get_sensers(uint16_t *g_sta);

void init_near_wall(void);
void near_wall_mode(uint8_t gSta);

void irda_timer_cfg(void);
void infra_sensers(uint16_t *g_sta);
void set_walk_angle(uint8_t dir,float agle);
void charging_turn_round(void);
float disf_xy(float x,float y);

int find_big_count( int *n ,int nNum );
void init_sys_status(uint8_t sta,uint8_t old_sta);
void robot_run_ctrl(void);
void init_robt_run_ctrl(int deg,int ctrl);
int dis_xy(short x,short y);
uint8_t in_area(int16_t x,int16_t bx,int16_t ex);
void init_gyro_g(void);

#if ROOMA_GYROONLY
uint8_t sys_state_convert2wifi(void);
uint8_t wifi_state_convert2sys(uint8_t wifi_mode);

extern uint16_t sWorkTimeMinCt,sWorkTimeMin,sSleepTimeMin;
#endif

#endif
