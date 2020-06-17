#ifndef CHARGE_H_
#define CHARGE_H_

#define DOCK_DEBUG_PRINT		1	/*回充打印*/
#define BACK_STAR_ORG_POINT		0	//走五角星的时候,是否回到五角星原点

#define NEAR_DOCK_STA		67

#if 0

#define CHARGE_PWM				630
#define DOCK_TURN_PWM			630		//转弯

#define DOCK_PWM_MDL			650		//找到中线
#define DOCK_PWM_GO				600		//行走
#else//---------------------------------------------add by wonton2004 2017.3.3
#define CHARGE_PWM				780//740//830
#define DOCK_TURN_PWM			780//800//780		//转弯

#define DOCK_PWM_MDL			(cfg->walk_pwm+50)		//找到中线
#define DOCK_PWM_GO				cfg->walk_pwm		//行走
#endif



#define CHARGE_ING		0x00	//找座子的的过程，
#define CHARGE_OK		0x01	//找到座子

#define DOCK_STA_NO		0x00
#define DOCK_STA_SIDE	0x01	//侧面过去
#define DOCK_STA_MID	0x02
#define DOCK_STA_TURN	0x03


typedef struct move_list{
	//uint8_t	sta;
//	unsigned char	dock_sta;
	uint8_t	knock_back;		//后退
	uint8_t	first_run;
	uint8_t	dock_side;		//充电桩的位置左中右
	uint8_t	disc_signal;	//发现信号
	//uint16_t find_side_flag ;
	uint16_t	c_midle;		//?????òμ??D??μ?′?êy
	uint16_t	c_turn;
	uint16_t 	c_side;		//侧面收到的次数
	//uint16_t	c_lost;			//????ê§è￥?D??μ?′?êy

	uint32_t 		t_turn;
	float 		out;
	int			walk_dist;
//	FlagStatus dock_fw;	//回充直行路径

	//FlagStatus ba_zi_able;		//8字走法

	uint16_t side_keep_max;
}ChargeInfo;

extern ChargeInfo charge_info;

uint8_t motor_turn_lidar(uint8_t dir,int pwm,float lyaw,float dyaw,float ryaw);
extern unsigned char IrRevData[4];
extern unsigned char IrData_bak[4];
unsigned char do_charging_work(ChargeInfo *Info);
void motor_turn_charge(uint8_t dir,int pwm,float agle);
void motor_fw_charge(void);
char motor_turn_check_midle(uint8_t dir,int pwm,float agle,uint8_t type,uint8_t c_m_f_out);
char dock_found_midle(uint8_t dir,float agle,uint8_t type,uint8_t c_m_f_out);
void init_charge(uint8_t sta);
void proc_charge_task(void);
unsigned char do_navi_charge(ChargeInfo *Info);
char dock_found_dc(float agle);
char dc_charge_ok(void);
uint8_t motor_stop_laser_ranging_hw_to_dock(int *run_dock_hw);

#ifdef KAILY_ROUND_CC_V12
uint8_t get_dockmode_irsw(void);
void set_dockmode_irsw(uint8_t sw);
#endif
uint8_t dock_go_middle(int16_t by);
void charge_back_off(int hw);
uint16_t read_ir_dock_knk(void);
uint8_t motor_turn_check_nosignal(uint8_t dir,int pwm,int hw,char flage);
extern float dock_found_angle(void);
extern void charg_motor_back_off(int hw,unsigned char flag);
uint8_t check_dock_bum(uint16_t grid,float angle);
extern uint8_t detect_dock(void);
#if (0 == SINGLE_LED_DOCK)
extern char motor_go_for_dock(void);
extern void default_ba_zi(void);
#else
extern uint8_t motor_go_for_dock(void);
#endif
uint8_t motor_turn_for_dock(uint8_t dir,int pwm,float agle);

#endif
