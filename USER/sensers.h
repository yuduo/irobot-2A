
#ifndef _SENSERS_H_
#define _SENSERS_H_

#if (1 == SIMPLIFY)
#define SP_TIMEOUT	12960000.000000
#define SP_T_MIN	0.210069444
#endif

#define MOTOR_LEAVE_H_L		1	//轮子的高低拎起的高低电平

#define T_IDLE_IR		2880//480		//红外在3分钟后，关闭

#if (1 == SINGLE_LED_DOCK)
typedef enum{
	IR_LEFT = 1,
	IR_RIGHT = 2,
	IR_MID = 3,
	IR_TOP = 4,
	IR_TOP_NEAR = 5,
	IR_WALL = 6,
}ir_dir_data;
extern uint8_t debug_ir_original_bk[3];
#elif (0 == SINGLE_LED_DOCK)
#define IR_LEFT			1
#define IR_M_RIGHT		2
#define IR_M_LEFT		4
#define IR_RIGHT		8

#define IR_LEFT_SIDE	(IR_LEFT | IR_M_LEFT)
#define IR_RIGHT_SIDE	(IR_RIGHT | IR_M_RIGHT)
#define IR_MID		(IR_M_LEFT |IR_M_RIGHT)
#endif

#define LEFT_IR_DROP		cfg->cliff_max//1500//3300
#define RIGHT_IR_DROP		cfg->cliff_max//1500//3300
#define FRONT_IR_DROP		cfg->cliff_max//1500//3300


#define IR_L_CHANNEL	0x1
#define IR_M_CHANNEL	0x2
#define IR_R_CHANNEL	0x3

#define BOTOM_IDX		0
#define DOOR_IDX		1
#define MID_ADC_DEBUG 	1

#define BOTOM_LEFT_IEX		0
#define BOTOM_FRONT_IDX		1
#define BOTOM_RIGHT_IDX		2


#define INTERRUPT_PRIORITY		5

#define IDX_RIGHT_ADC		1		//PC1 右
#define IDX_LEFT_ADC		2		//pc2 左
#define IDX_MIDLE_ADC		3		//PC3 中

//左边红外
#define IR_L_SOURCE			GPIO_PortSourceGPIOB
#define IR_L_PIN			GPIO_PinSource15
#define IR_L_LINE			EXTI_Line15
#define IR_L_IRQ			EXTI15_10_IRQn //EXTI10_15_IRQn
#define IR_L_NUM			15
#define IR_L_GPIO_PORT		GPIOB
#define IR_L_GPIO_PIN		GPIO_Pin_15

#define IR_L_PIN_NUM		0

//中间红外 PE7
#define IR_R_SOURCE			GPIO_PortSourceGPIOC
#define IR_R_PIN			GPIO_PinSource8
#define IR_R_LINE			EXTI_Line8
#define IR_R_IRQ			EXTI9_5_IRQn
#define IR_R_NUM			8
#define IR_R_GPIO_PORT		GPIOC
#define IR_R_GPIO_PIN		GPIO_Pin_8

#define IR_R_PIN_NUM		2

#if(0)		//1,正式为1//3#		2#0
//右边红外
#define IR_M2_SOURCE		GPIO_PortSourceGPIOE
#define IR_M2_PIN		GPIO_PinSource7
#define IR_M2_LINE		EXTI_Line7
#define IR_M2_IRQ		EXTI9_5_IRQn
#define IR_M2_NUM		7
#define IR_M2_GPIO_PORT	GPIOE	
#define IR_M2_GPIO_PIN	GPIO_Pin_7

#define IR_M_SOURCE		GPIO_PortSourceGPIOE
#define IR_M_PIN			GPIO_PinSource9
#define IR_M_LINE			EXTI_Line9
#define IR_M_IRQ			EXTI9_5_IRQn
#define IR_M_NUM			9
#define IR_M_GPIO_PORT	GPIOE
#define IR_M_GPIO_PIN	GPIO_Pin_9
#else//V1.3F之前的硬件版本,双灯左右搞反了,中左和中右要对调一下
#define IR_M2_SOURCE		GPIO_PortSourceGPIOE
#define IR_M2_PIN			GPIO_PinSource9
#define IR_M2_LINE			EXTI_Line9
#define IR_M2_IRQ			EXTI9_5_IRQn
#define IR_M2_NUM			9
#define IR_M2_GPIO_PORT	GPIOE	
#define IR_M2_GPIO_PIN	GPIO_Pin_9

#define IR_M_SOURCE		GPIO_PortSourceGPIOE
#define IR_M_PIN		GPIO_PinSource7
#define IR_M_LINE			EXTI_Line7
#define IR_M_IRQ			EXTI9_5_IRQn
#define IR_M_NUM			7
#define IR_M_GPIO_PORT		GPIOE
#define IR_M_GPIO_PIN		GPIO_Pin_7
#endif
#define IR_ML_PIN_NUM	1
#define IR_MR_PIN_NUM	3


//#define VVV 1
//#if VVV
//红外参数
//右边红外PID

#define IRD_RIGHT_PID_MIN	cfg->min_right_adc//右边做PID最小的红外值
#define IRD_RIGHT_PID_MIN2	1000	//右边第二个红外
#define IRD_RIGHT_PID_LOCK	cfg->lock_right_adc//PID锁定的值
#define IRD_RIGHT_PID_MAX   cfg->max_right_adc//右边最大的值，超过这个，要往外拐了
#define IRD_RIGHT_PID_MAX2	5000//cfg->max_right2_adc//1500	//右边第二个红外的最大值，超过这置，要往外拐了

#define IRD_RIGHT_ADJ_MIN	100//cfg->lost_right_adc//右边红外特征物沿边
#define IRD_RIGHT_OBST		cfg->side_right_adc		//右边障碍物
#define IRD_RIGHT_OBST1		350
#define SIDE_LOST_RIGHT		100//cfg->lost_right_adc

//左边红外参数
#define IRD_LEFT_PID_MIN	cfg->min_left_adc//右边做PID最小的红外值
#define IRD_LEFT_PID_MIN2	1300	//右边第二个红外
#define IRD_LEFT_PID_LOCK	cfg->lock_left_adc//PID锁定的值
#define IRD_LEFT_PID_MAX	cfg->max_left_adc
#define IRD_LEFT_PID_MAX2	5000//cfg->max_left2_adc//2000	//右边第二个红外的最大值，超过这置，要往外拐了


#define IRD_LEFT_ADJ_MIN	100//cfg->lost_left_adc//右边红外特征物沿边
#define IRD_LEFT_OBST		cfg->side_left_adc//右边障碍物
#define IRD_LEFT_OBST1		200
#define SIDE_LOST_LEFT		100//cfg->lost_left_adc


/*
#define DOCK_BUM_IR()		((sys->g_sta[2] >= 300 &&  sys->g_sta[3] >= 100 && sys->g_sta[4] >= 300) && \
							 (sys->g_sta[2] < 1300 &&  sys->g_sta[3] < 1300 && sys->g_sta[4] < 1300) && \
							 (sys->g_sta[2] >= 400 ||  sys->g_sta[3] >= 170 || sys->g_sta[4] >= 300))
*/
#if LASER_IR
#define DOCK_BUM_IR()		(sys->ir_front_1 >= 400 ||  sys->g_sta[3] >= 700 || sys->ir_front_3 >= 400)   //200  600 200

#else
#define DOCK_BUM_IR()		(sys->g_sta[2] >= 100 ||  sys->g_sta[3] >= 100 || sys->g_sta[4] >= 100)
#endif

#define BUM_RIGHT(X)			((X & MASK_BUM_RIGHT) && (X & MASK_BUM_LEFT) != MASK_BUM_LEFT)
#define BUM_LEFT(X)				((X & MASK_BUM_LEFT) && (X & MASK_BUM_RIGHT) != MASK_BUM_RIGHT)

/*
#define IRD_RIGHT_PID_MIN	600		//右边做PID最小的红外值
#define IRD_RIGHT_PID_MIN2	1000	//右边第二个红外
#define IRD_RIGHT_PID_LOCK	1200		//PID锁定的值
#define IRD_RIGHT_PID_MAX   3000	//右边最大的值，超过这个，要往外拐了
#define IRD_RIGHT_PID_MAX2	2400	//右边第二个红外的最大值，超过这置，要往外拐了

#define IRD_RIGHT_ADJ_MIN	300		//右边红外特征物沿边
#define IRD_RIGHT_OBST		800		//右边障碍物
#define IRD_RIGHT_OBST1		350
#define SIDE_LOST_RIGHT		200

//左边红外参数
#define IRD_LEFT_PID_MIN	400		//右边做PID最小的红外值
#define IRD_LEFT_PID_MIN2	1300	//右边第二个红外
#define IRD_LEFT_PID_LOCK	1200		//PID锁定的值
#define IRD_LEFT_PID_MAX	3000
#define IRD_LEFT_PID_MAX2	2000	//右边第二个红外的最大值，超过这置，要往外拐了


#define IRD_LEFT_ADJ_MIN	200		//右边红外特征物沿边
#define IRD_LEFT_OBST		400		//右边障碍物
#define IRD_LEFT_OBST1		200
#define SIDE_LOST_LEFT		100
*/
/*


//2016-10-19演示的码盘版本
#define IRD_RIGHT_PID_MIN	500		//右边做PID最小的红外值
#define IRD_RIGHT_PID_MIN2	1000	//右边第二个红外
#define IRD_RIGHT_PID_LOCK	800		//PID锁定的值
#define IRD_RIGHT_PID_MAX   3200	//右边最大的值，超过这个，要往外拐了
#define IRD_RIGHT_PID_MAX2	2800	//右边第二个红外的最大值，超过这置，要往外拐了

#define IRD_RIGHT_ADJ_MIN	300		//右边红外特征物沿边
#define IRD_RIGHT_OBST		600		//右边障碍物
#define IRD_RIGHT_OBST1		350
#define SIDE_LOST_RIGHT		150

//左边红外参数
#define IRD_LEFT_PID_MIN	100		//右边做PID最小的红外值
#define IRD_LEFT_PID_MIN2	1000	//右边第二个红外
#define IRD_LEFT_PID_LOCK	320		//PID锁定的值
#define IRD_LEFT_PID_MAX	3200
#define IRD_LEFT_PID_MAX2	1600	//右边第二个红外的最大值，超过这置，要往外拐了


#define IRD_LEFT_ADJ_MIN	120		//右边红外特征物沿边
#define IRD_LEFT_OBST		200		//右边障碍物
#define IRD_LEFT_OBST1		150
#define SIDE_LOST_LEFT		70


*/
#define MIN_IRDA			800
#define SIDE_LOST_ADC		200
#define HAVE_IRDA_N			550



#define NEAR_PID_IRDA	800


#define MAX_MOTOR_L		cfg->i_whele_max//65	//正常的都是10左右乱晃，堵住了，都达到40以上
#define MAX_MOTOR_R		cfg->r[IC_WHEEL_R]//65	//正常的都是10左右乱晃，堵住了，都达到40以上

#define MAX_COUNT		cfg->t_whele_max//2000
#define MIX_I_COUNT		2

#define MAX_LEAVE_TIME		cfg->t_drop_max//2000	// 4秒钟
#define MAX_LEAVE_TIME2		4000	

#define MAX_LEAVE_COUNT		2000
#define MIX_LEAVE_COUNT		500



#define MAX_IR_SIDE2		2000
#define MIDLE_IR_DATA		800

#if LASER_IR
#define SLOW_IRD_MIDL()		((sys->g_sta[2] >= cfg->slow_left|| sys->g_sta[3] >= cfg->slow_mid|| sys->g_sta[4] >= cfg->slow_right))
#define IR_SIDE_BUM()		(sys->g_sta[0] > 800 || sys->g_sta[1] > 1000 || sys->g_sta[5] > 1000 || sys->g_sta[6] > 800)
//#define MIDLE_HAVE_IRDA2() 	     ((sys->g_sta[2] >= 250 || sys->g_sta[3] >= 850 || sys->g_sta[4] >= 250))
#define MIDLE_HAVE_IRDA2() 	((sys->g_sta[2] >= 1500 && sys->g_sta[4] >= 1500) ||  sys->g_sta[3] >= 800 /*|| */)//睿德代工的前100台前撞中1和中3红外静态值较大，出一个临时版本，如果不改前撞模具，可能此参数即是量产值 micony20190307 修改增加 sys->g_sta[2] 和sys->g_sta[4]都有值，也行的，且值改到了中间800
																									// modified by wonton2004 20190121
#define IR_MIDLE_BUM()		(sys->g_sta[2] >= MIDLE_IR_DATA && sys->g_sta[3] >= 800 && sys->g_sta[4] >= MIDLE_IR_DATA)
//宽松的中间碰撞，正常值137,52,181
#define IR_MIDLE_BUM2()		(sys->g_sta[2] >=200 && sys->g_sta[3] >=600 && sys->g_sta[4] >=250)

#define IR_D_VALUE 1000      //1000



#else
#define SLOW_IRD_MIDL()		((sys->g_sta[2] >= 400 || sys->g_sta[3] >= 400 || sys->g_sta[4] >= 400))
#define IR_SIDE_BUM()		(sys->g_sta[0] > 300 || sys->g_sta[1] > 1000 || sys->g_sta[5] > 1000 || sys->g_sta[6] > 300)
#define MIDLE_HAVE_IRDA2() 	((sys->g_sta[2] >= 350 || sys->g_sta[3] >= 550 || sys->g_sta[4] >= 350))// modified by wonton2004 20170519

//用红外来计算是否中间碰撞
#define IR_MIDLE_BUM()		(sys->g_sta[2] >= MIDLE_IR_DATA && sys->g_sta[3] >= MIDLE_IR_DATA && sys->g_sta[4] >= MIDLE_IR_DATA)
//宽松的中间碰撞，正常值137,52,181
#define IR_MIDLE_BUM2()		(sys->g_sta[2] >=200 && sys->g_sta[3] >=100 && sys->g_sta[4] >=250)


#endif

#define BASE_ADC_VALUE		150			//基础的值，大于这个值，认为有红外

#define MIN_MIDLE_IDA	  cfg->midle_adc


#define MIDLE_IRDA() 	((sys->g_sta[2] >= cfg->m_left_adc || sys->g_sta[3] >= MIN_MIDLE_IDA || sys->g_sta[4] >= cfg->m_right_adc))  //中间红外
#define MIDLE_HAVE_IRDA() 	((sys->g_sta[2] >= 500 || sys->g_sta[3] >= 500 || sys->g_sta[4] >= 500))
#define MIDLE_HAVE_IRDA3() 	((sys->g_sta[2] >= 100 || sys->g_sta[3] >= 800 || sys->g_sta[4] >= 100))
#define RIGHT_IRDA()		(sys->g_sta[6] >= IRD_RIGHT_OBST)
#define RIGHT_IRDA1()	(sys->g_sta[5] >= IRD_RIGHT_OBST1|| sys->g_sta[6] >= IRD_RIGHT_OBST1)
#define LEFT_IRDA()			(sys->g_sta[0] >=IRD_LEFT_OBST)
#define LEFT_IRDA1()		(sys->g_sta[0] >=IRD_LEFT_OBST1)


#define LEFT_IRDA2()		(sys->g_sta[0] >=1000 || sys->g_sta[2] >=500 )
#define RIGHT_IRDA2()		(sys->g_sta[5] >=1000 || sys->g_sta[6] >=500 )

#define	RIGHT_IR_BUM2()		(sys->g_sta[5] >=2000)
#define	LEFT_IR_BUM2()		(sys->g_sta[1] >=2000)



#define LEFT_ADC()		sys->g_sta[0]
#define LEFT_ADC1()		sys->g_sta[1]
#define IR_MIDE_ADC(X)  (sys->g_sta[2] >= X && sys->g_sta[3] >= X && sys->g_sta[4] >= X)

#define RIGHT_ADC()		sys->g_sta[6]
#define RIGHT_ADC1()	sys->g_sta[5]
#define MIDLE_ADC()		sys->g_sta[3]
#define RIGHT_NEAR()	(RIGHT_ADC() >= 5800)			//右边离得太近了，贴墙了，要退出去一点 目前还做不了贴墙算法
#define LEFT_NEAR()		(LEFT_ADC() >= 5800)			//右边离得太近了，贴墙了，要退出去一点
#define NEAR_PID_LEFT()		(sys->g_sta[0] <= 170)			//没有阳光的情况下，这个值刚好
#define NEAR_PID_RIGHT()	(sys->g_sta[6] <= 170)

/*
#define MIDLE_IRDA() 	((sys->g_sta[2] <= MIN_MIDLE_IDA || sys->g_sta[3] <= MIN_MIDLE_IDA || sys->g_sta[4] <= MIN_MIDLE_IDA))  //中间红外
#define MIDLE_IRDA_NEAR() 	((sys->g_sta[2] <= 2000 || sys->g_sta[3] <= 2000 || sys->g_sta[4] <= 2000))  //中间红外

#define RIGHT_IRDA()	(sys->g_sta[5] <=cfg->side_adc|| sys->g_sta[6] <=cfg->side_adc)
#define LEFT_IRDA()		(sys->g_sta[0] <=cfg->side_adc || sys->g_sta[1] <=cfg->side_adc)
#define SLOW_IRD_MIDL()		((sys->g_sta[2] <= cfg->slow_adc|| sys->g_sta[3] <= cfg->slow_adc || sys->g_sta[4] <= cfg->slow_adc) && (sys->g_sta[2] >= 1000 && sys->g_sta[3] >= 1000 &&  sys->g_sta[4] >= 1000 ))

#define LEFT_ADC()		sys->g_sta[0]
#define RIGHT_ADC()		sys->g_sta[6]
#define MIDLE_ADC()		sys->g_sta[3]
#define RIGHT_NEAR()	(RIGHT_ADC() >= 5800)			//右边离得太近了，贴墙了，要退出去一点 目前还做不了贴墙算法
#define LEFT_NEAR()		(LEFT_ADC() >= 5800)			//右边离得太近了，贴墙了，要退出去一点
#define NEAR_PID_LEFT()		(sys->g_sta[0] <= 170)			//没有阳光的情况下，这个值刚好
#define NEAR_PID_RIGHT()	(sys->g_sta[6] <= 170)
*/


#define MAX_DIS_ADC		700
#define MAX_NEAR_ADC	2000

#define AGI_AMP_FACTOR	(float)(cfg->r[IC_AGI_AMP_FACTOR]/100)	//2.0
#define LMOT_AMP_FACTOR	(float)(cfg->r[IC_LMOT_AMP_FACTOR]/100)	//0.75f
#define RMOT_AMP_FACTOR	(float)(cfg->r[IC_RMOT_AMP_FACTOR]/100)	//0.75f


//#define LED5_OFF()		 GPIO_ResetBits(GPIOE,GPIO_Pin_3)
//#define LED5_ON()		 GPIO_SetBits(GPIOE,GPIO_Pin_3)



//void ir_original_bkbuff_test(uint8_t flag);		///debug 2018-07-16 jzz

void  senser_gpio_config(void);
char get_sensers(uint16_t *g_sta);
void hw_timer_cfg(void);
void insert_fw_rd(uint16_t value);
uint16_t max_fw_rd(int max_v);
void reset_fw_fifo(void);
void ir_parse(uint8_t ir_channel);
void read_ir_data(u8 *IR,char flage);
void set_ir_alter_dis(uint8_t dis);
void ir_recv_ext_config(void);
void check_sensens(void);
int ird_to_dist(int side,int ird);
int IR_MIDLE_BUM1(void);
uint8_t ir_dock_insight(uint8_t clr);
char get_irda(uint16_t *g_sta);
void printf_power_sta(void);
void read_ir_data_timeout(u8 *IR,char flage);
void read_ir_original_data(u8 *IR);


#define BKP_CONSUMABLES_CHECK_CODE BKP_DR6  	//耗材检测
#define BKP_CHECK 0x55aa
#define BKP_DUST_DATA 					BKP_DR5			//尘盒
#define BKP_STRAINER_ALARM_DATA 		BKP_DR7			//滤网
#define BKP_SIDE_RUSH_ALARM_DATA 		BKP_DR8			//边刷
#define BKP_MAIN_RUSH_ALARM_DATA 		BKP_DR9			//主刷

#define CFG_DUST_FULL_TIMER_ALARM	cfg->dust_full_time_alarm			//7*60		//7h//超过尘盒尘满时间 3min测试
#define CFG_STRAINER_ALARM			cfg->strainer_use_time_alarm		////4min
#define CFG_SIDE_RUSH_ALARM			cfg->side_rush_use_time_alarm		///5min
#define CFG_MAIN_RUSH_ALARM			cfg->main_rush_use_time_alarm		///6min
//01 尘盒 02 滤网 03 边刷 04 主刷

void dust_full_check_pro(uint8_t flag);
void strainer_alarm_check_pro(uint8_t flag);
void side_rush_alarm_check_pro(uint8_t flag);
void main_rush_alarm_check_pro(uint8_t flag);
void consumables_check_pro(uint8_t flag);
extern void comm_ultrasonic(unsigned short * data);

#endif
