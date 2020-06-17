#ifndef H_NAVIGAT_
#define H_NAVIGAT_

#define ROOMA_GYROONLY	1//陀螺仪版本的宏,此宏为非0时表示为单陀螺仪版本,对应PCB系列板号为RE811/RE810A 硬件版本号>=1.03
						//较RE810取消了吸嘴检测PD12,增加三个按键PA8 PA15 PB14,增加LED PA11 PA12
						//显控串口改为WIFI模块串口,波特率9600bps

#define ROOMA_RELEASE				1		// 芮魅正式释放版本




#if (1)////ROOMA_RELEASE
#define BUM_LINE_ADJ				0

#define EDGE_DRAW_MAP				0		//沿边画地图	micony2019-03-05
#define JUDGE_STOP_FRONT			0		//wy190615 根据前轮判断是否停止。生产为1，生产为0
#define ADJ_END						1		//矫正使能
#define TURN_V_ADJ					1
#define BOTTOM_IR_EN				1		//对地红外使能
#define GYRO_MODUL					0		//采用第三方陀螺仪模块
#define NEAR_DRAW_MAP				0		//延边画地图
#define PRINTF_BY_BUFFER			0		//中断		正式为0
#define CALE_ADJ_RIGHT				0		//右轮矫正坐标
#define CALE_BY_FRONT				0
#define MOP_EN						0		//湿拖模式	正式为1
#define GYRO_ADJ_ONBOARD			0
#define LASER_IR					1		//三个测距红外
#define LASER_IR_ONLY_ONE			0		//只有1个测距红外
#define CURR_ALA_TABLE				0		//查表法电流告警

#define TURN_CHECK_BUM				1		//wy190528 旋转时加入碰撞判断

#define	ADJ_GYRO_IDX				1
                                             //micony20190306 陀螺仪不能矫正0度方向，新的陀螺仪程序可以矫正，新的陀螺仪程序为0

#define THE_ORIGINAL_VALUE          1        //中1,中3 原始值   2019 03 11


#define DUST_BOX_CHECK				1		//尘合检测
#define UNINTERRUPTED_CLEAN			0		//不间断清扫			正式为0
#define WALK_PRINTF					1		//打印走过的路
#define WALK_PRINTF_CALC			0
#define PRINTF_SCAN_END				0		//结束的时候打印扫描点
#define NEARWALL_PID				0
#define CLEAN_OK_LOW_LEVEL			1		//低级别的清扫完成
#define Z_GO_NEAR_REAL	   			1		//延边测试
#define TARGE_RUN					0
#define ADJ_V_90					0
#define LINEX_EN					1
#define WALK_1D5_HOUR				0		//只走一个半小时。
#define LAB_DEMO_VERSION			0		//------------------------  1实验室演示版本   0正常生产版本


#define AUTO_LAMP_EN				1		// 光感灯

#if LAB_DEMO_VERSION				//实验室版本不校准陀螺仪
#define ADJ_GYRO					0		///校准陀螺仪					需要校准陀螺仪的时候开启,不需要校准陀螺仪的时候为0
#else
#define ADJ_GYRO					0		///校准陀螺仪					需要校准陀螺仪的时候开启,不需要校准陀螺仪的时候为0
#endif

#define MOTOER_SENSER_CHECK			1	///正式要为1
#define LOCKED_MOTOR_PARA 			0		//正式要为1   不好用
#define JUDGE_PID_CTRL_WHEEL_STOP	0		//通过判断PID函数内HW值,得到左右轮是否缠线标志
#define MID_MOTOR_CUR_OVER_GO_ON_EN	1		//中扫电机过流 停掉继续跑
#define SIDE_MOTOR_CUR_OVER_GO_ON_EN	1		//边刷电机过流 停掉继续跑
#define TURN_TIMEOUT_EN				0		//旋转超时判断  1使能  0一直在旋转
#define CONSUMABLES_CHECK_EN		1		//耗材告警使能
#define DUST_MOTOR_SAME_2B			1		//与2B相同为1,与2B不同为0
#define POWER_ON_RUN_VER			0		//开机就跑



#if CONSUMABLES_CHECK_EN
#define DUST_BOX_FULL_CHECK			0		//尘盒满累积时间计算使能   1尘盒会告警  0 尘盒不会告警
#define STRAINER_ALARM_CHECK		0
#define SIDE_RUSH_ALARM_CHECK		0
#define MAIN_RUSH_ALARM_CHECK		0

#endif

#define DP_PANEL_EN					4	//165显控888使能   	2:显控版本的170版本  3:陀螺仪板本的170版本   4:显控版本180版本
#define NEARWALL_SCAN_OBST_RECORD	0		//1,沿边清扫和障碍点被记录在坐标里,   0,不记录

#if (0)//LASER_IR_ONLY_ONE
#define TURN_90_V					0		//TURN_90_V与LASER_IR_ONLY_ONE状态要一致
#else
#define TURN_90_V					0		//TURN_90_V与LASER_IR_ONLY_ONE状态要一致
#endif

#define CAPACITY_4400_VER			0


#else

#define JUDGE_STOP_FRONT			0		//根据前轮判断是否停止。
#define ADJ_END						0		//矫正使能
#define BOTTOM_IR_EN				1		//对地红外使能
#define GYRO_MODUL					0		//采用第三方陀螺仪模块
#define NEAR_DRAW_MAP				0		//延边画地图
#define PRINTF_BY_BUFFER			0		//中断			//正式为0，调试可以用1
#define CALE_ADJ_RIGHT				1		//右轮矫正坐标
#define CALE_BY_FRONT				0
#define MOP_EN						0		//湿拖模式
#define GYRO_ADJ_ONBOARD			0
#define LASER_IR					1		//三个测距红外
#define LASER_IR_ONLY_ONE			0		//只有1个测距红外
//#define TURN_90_V					1
#define TURN_V_ADJ					0

#define	ADJ_GYRO_IDX				1		//micony20190306 陀螺仪不能矫正0度方向，新的陀螺仪程序可以矫正，新的陀螺仪程序为0


#define DUST_BOX_CHECK				0		//尘合检测
#define UNINTERRUPTED_CLEAN			0		//不间断清扫
#define WALK_PRINTF					1		//打印走过的路
#define WALK_PRINTF_CALC			0
#define PRINTF_SCAN_END				0		//结束的时候打印扫描点
#define NEARWALL_PID				0
#define CLEAN_OK_LOW_LEVEL			1		//低级别的清扫完成
#define Z_GO_NEAR_REAL	   			1		//延边测试
#define TARGE_RUN					0
#define ADJ_V_90					0
#define LINEX_EN					1
#define MOTOER_SENSER_CHECK			0		///中扫缠线
#define LOCKED_MOTOR_PARA 			0		//定时器判断左右轮是否缠线
#define JUDGE_PID_CTRL_WHEEL_STOP	0		//通过判断PID函数内HW值,得到左右轮是否缠线标志

#define MID_MOTOR_CUR_OVER_GO_ON_EN	1		//中扫电机过流 停掉继续跑
#define SIDE_MOTOR_CUR_OVER_GO_ON_EN	1		//边刷电机过流 停掉继续跑
#define TURN_TIMEOUT_EN				0		//旋转超时判断  1使能  0一直在旋转

#define DP_PANEL_EN					2		///1 165显控888使能   2 170

#if LASER_IR_ONLY_ONE
#define TURN_90_V					0		//TURN_90_V与LASER_IR_ONLY_ONE状态要一致
#else
#define TURN_90_V					0		//TURN_90_V与LASER_IR_ONLY_ONE状态要一致
#endif


#if JUDGE_PID_CTRL_WHEEL_STOP			//通过判断PID函数内HW值,得到左右轮是否缠线标志
#define GET_OUT_OF_TROUBLE_EN		0	//		Get out of trouble

#define GETOUT_TROUBLE_ANGLE   		44.59f
#define INTO_CHECK_SENSER_GETOUT_TROUBLE   0				///在serner里阻塞脱困
#else
#define GET_OUT_OF_TROUBLE_EN		0
#define GETOUT_TROUBLE_ANGLE   		44.59f
#define INTO_CHECK_SENSER_GETOUT_TROUBLE   0				///在serner里阻塞脱困

#endif

#endif

#define DOCK_SIGNAL_UNBLE			0						///1 顶灯+红外一起判断充电座距离  0 顶灯判断 正式为1

#if (1)////ROOMA_GYROONLY
#define WIFIAPP_EN					1			//正式为1

#if SYS_VER == VER_BIGWHITEMACH	|| VER_SMALLBLACKMACH || VER_WHITEBLUEMACH
#define ARC_Z_GO					0			//正式为1 /弧线行走			状态必须与ADJ_END	保持一致
#else
#define ARC_Z_GO					0///1			//正式为1 /弧线行走			状态必须与ADJ_END	保持一致
#endif

#else

#define WIFIAPP_EN 					0
#define ARC_Z_GO					0

#endif


#if (0)////SYS_VER == VER_BIGWHITEMACH
#define BUM_STA_OLD					1		//老版本的碰撞  无摇臂为1  有摇臂为0
#else
#define BUM_STA_OLD					0		//老版本的碰撞  无摇臂为1  有摇臂为0
#endif
#define		FRONT_IR_VERSION_1		1		//红外的版本，		2018-07-11 由1改为0
#define  PID_DEBUG					0		///PID调试为1  正式为0
#define  GYRO_TO_LIB				0

#define GYRO_VERSION				1			//陀螺仪版本---   0:EPSON	1:BOSIH BMI055
#define MAP_VERSION					1			//地图版本  生产为量产0无地图   新图牙为1 测试版本
#if MAP_VERSION
#define MAP_VERSION_V				3			//地图版本1 2    //2为新UI定制测试产品   //3马来西亚地图版本
#define CURRENT_POINT_UNREPORT		0

#endif
#define LAGER_ROUND_MODE			1			//绕大桩判断模式

#define CALE_BY_RIGHT_STOP_EN		0			//HW值出错,是否停电机  1 停机 0 不停机


#define MAX_CLEAN_FRONT				2		//前方还有3个方可继续
#define MAX_UNCLEAN_IN				4		//最少有几个未清扫，才进去
#define CALE_BY_FRON				0

#define FORCE_SIDE_NAVI				0		//强制扫完一边
#define EXTEND_ROUTER				1
#define PARTITION					0		//分区清扫
#define ROBOT_TO_WHELE				1		//
#define NAVI_V_AJUST				0		//导航的时候，是否要矫正一下
#define CALE_BY_BKHW				1		//后轮计算坐标
#define HW_AB_EN					0		//后轮霍尔使用AB相计算
#define HW_AB_OLD					0		//后轮霍尔AB相，插件式SK1816为老版本码盘，贴片式SK1816为新版本码盘，新旧码盘的AB相是相反的，需要区别一下 1为旧码盘 0为新码盘
											//需要注意的是主板测试架的码盘AB相是与老版本相同，所以在主板测试时需要加以区别

#define BORD_FRONT_100				1		//20190307 于上海办公室。2019年3月份，惠州生产的100套前撞，缩进太多，前部红外测试有问题。1为该前撞版本，未来都不用该版本



#define NOT_TURN_DEG_V				0
#define PRINTF_PWR_STA				1		//打印电压

//#define COOR_CALC_TEST				0		//坐标计算测试

#define WHELE_RESV					1		//后来的码盘是否反的，以后边的为正，之前的是反的

#define LASER_EN					0		//前部激光版本
#define LASER_SIDE					0		//侧面红外
//#define NEAR_DRAW_MAP				0		//延边画地图
#define VERIFY_MAP					0		//是否校验地图
#define WALK_NAV_FOR_AJD			1		//导航走的时候矫正地图
#define NAVI_WALK_NEAR_LINE			0		//延边只走一条线
#define BORD_VERSION				3		// 3 最新版本 ，2上一版本
#define ADJ_NAER_TURN_V				0		//ADJ延边的时候，转到垂直的位置，该函数写了，未测试，先关闭
#define EN_GO_UNCLR					1		//是否去未清扫区域
#define GYRO_SHORT_ORG				0		//取陀螺仪原始数据
#define GYRO_OFSET_ONBORD			0		//在本机实现陀螺仪偏移矫正

//#define PID_ANGLE_TEST				1		//PID角度调试

#define LASER_DRAW			0		//激光画图

#define WALK_AJUST			0		//行走的时候矫正地图

#define NAVI_NEAR_LINE		0		//导航回来延边回来

#define LASER_RED			0		//红光激光
#define GYRO_LASER_V1		0		//陀螺仪，激光版本1
#define GRYO_SPI  			1

#define TEST_VERSION		0		//测试
#define TEST_FOR_LABO		0
#define LASER_FORMART_LINE	0		//方格内的测试版本，用于格式化障碍物成一条直线。仅用于方格内的演示。
#define LASER_SCAN_TEST		0		//方格内的扫描测试


#define DELAY_MS(X)		delay_ms_sensers(X)

#define LASER_PWM			650		//激光旋转的时候的速度

#define AJUST_NEAR_ANGLE	0
#define AJUST_VITAL_ANGLE	1
#define SIGE_UNCLAER		0		//标记未清扫区域的方式
#define BOARD_V1			0
#define ADJ_COR_WHELL		0		//后轮矫正坐标

#define OS_WINDWOS			0

#define EN_AJUST_BY_LINE	1		//是否根据旁边的直线来判断本次碰撞时直线
#define EN_SIDE_BUM			1		//是能侧面碰撞，是否拐外斜过去
#define NAVI_LINE_Y			0
#define LAND_IR_EN			1		//对地红外是否有作用，要新的磨具才能用

//micony20170726 电压补偿       13887
#define LOW_POW_PRS_TWS				20


#define CHARGE_POWER		13800////14100
#define CHARGE_POWER_2		12800////13500
#define CHARGE_POWER_3     	12400
#define LOW_POWER			12800


#define AJUST_NOW_OBST		0		//碰撞的时候检测是否越过障碍物,这个目前有问题，再碰撞的时候，老是矫正，导致坐标乱掉，尤其是Y轴的90?270°碰撞的时候，经常出问题。

#define ANGLE_VALID()		((sys->angle <= 20 && sys->angle >=-20)	 ||		\
							 (sys->angle >= 70 && sys->angle <=110)  ||		\
							 (sys->angle >= 165 && sys->angle <=195) ||		\
							 (sys->angle >= 250 && sys->angle <=290) ||		\
							 (sys->angle >= 340 && sys->angle <=360))


#if ADJ_END
#define AJUST_RUN			1		//特征物对比使能

#define ANGLE_NEAR_RUN_ADJ	1		//沿边走的时候矫正角度
#define ANGLE_PEER			30		//角度校准的比例
#define ANGLE_ADJ_DIS		20.0f

#define TARGER_REC_AJUST	0		//特征物被覆盖的矫正，目前还是有问题，如果特征物的两侧都有清扫区域，会有问题，目前还没有判定特征物的方向
#define NAVI_TO_TARGET		1		//是否要导航到特征点
#define FORMART_ADJ_RUN		1		//格式化特征物走法时，X轴的坐标。如果角度变化不大，则X轴坐标不让动，要不然会有一条曲线出来。
#define ADJ_Y_BUM			0		//y轴碰撞的时候，根据特征物矫正		目前演示暂时先不开，
#define FORCE_AJD_LONG_TIME	0		//长时间走后，强制校准一次
#define TO_ADJ_BEFOR_NAV	0		//导航前，是否走一下特征物
#define NEAR_ADJ_NAV		1		//近距离的矫正


#else
#define AJUST_RUN			0		//特征物对比使能

#if ROOMA_GYROONLY
#define ANGLE_NEAR_RUN_ADJ	0		//沿边走的时候矫正角度
#else
#define ANGLE_NEAR_RUN_ADJ	0		//沿边走的时候矫正角度
#endif
#define ANGLE_PEER			80		//角度校准的比例

#define ANGLE_ADJ_DIS		20.0f

#define TARGER_REC_AJUST	0		//特征物被覆盖的矫正，目前还是有问题，如果特征物的两侧都有清扫区域，会有问题，目前还没有判定特征物的方向
#define NAVI_TO_TARGET		0		//是否要导航到特征点
#define FORMART_ADJ_RUN		0		//格式化特征物走法时，X轴的坐标。如果角度变化不大，则X轴坐标不让动，要不然会有一条曲线出来。
#define ADJ_Y_BUM			0		//y轴碰撞的时候，根据特征物矫正		目前演示暂时先不开，
#define FORCE_AJD_LONG_TIME	0		//长时间走后，强制校准一次
#define TO_ADJ_BEFOR_NAV	0		//导航前，是否走一下特征物
#define NEAR_ADJ_NAV		0		//近距离的矫正

#endif


#define OFFSET_180_PID		(0)		//180°走的时候偏移一下角度，否则Y轴导航不准

#define TURN_TO_DEG			2		//0 - 左转 ，1-右转，2，左右转

#define TURN_VERTIC_WALL	0		//贴墙转角度的时候，是否要垂直转

#define PRINTF_SCAN			1
#define PRINTF_BUM_DIS		0		//打印碰撞时的距离和坐标

#define NAVI_TYPE_CLEAN		0		//前方已经清扫导致的导航
#define NAVI_TYPE_BUM		1		//碰撞导致的导航
#define NAVI_TYPE_NJST		2		//导航不校准

#define NO_ANGLE_V			1000		//不是角度


#define SCANE_XY(X,Y)		read_compress_mem(X, Y, navigat->scan)
#define OBST_XY(X,Y)		read_compress_mem(X, Y, navigat->obstacle)


#define BUM_NO				0x00
#define BUM_BUM				0x01
#define BUM_SIDE			0x02

#define ADJ_NAVI_NO_RUN		0x00	//不符合导航到特征点的条件，没动，没导航过去
#define ADJ_NAVI_RUN_OK		0x01	//符合导航到特征点的条件，且导航过去，矫正成功
#define ADJ_NAVI_RUN_ERR	0x02	//符合导航到特征点的条件，且导航过去，不过是否，但机子动了

#define NAV_ADJ_TYPE_NO		0x00
#define NAV_ADJ_FORCE_H		0x01	//强制去第一个，第二个。
#define NAV_ADJ_FORCE_ALL	0x02	//强制去，只要能导航过去，都去，一般发现坐标错误的时候，要强制矫正一下
#define NAVA_ADJ_NEAR		0x03
#define NAV_ADJ_ANGLE		0x04		//强制去延边矫正角度


#define NADJ_RET_NO  		0x00	//不符合校验的标准，不是七字形或长条的一字型
#define NADJ_RET_BUM 		0x01	//七字形校验
#define NADJ_RET_LINE 		0x02	//一字型校验

#define GO_ADJ_ERROR	   	NADJ_RET_NO		//测滑等错误
#define GO_ADJ_OVER	   		NADJ_RET_NO		//步数走完
#define GO_ADJ_BUM	   		NADJ_RET_BUM		//碰撞
#define GO_ADJ_LOST	   		NADJ_RET_LINE		//延边丢失

#if SYS_VER == VER_ROMMAT || VER_SMALLBLACKMACH || VER_BIGWHITEMACH || VER_WHITEBLUEMACH
#define WALL_LOST_DEG		90
#else
#define WALL_LOST_DEG		40

#endif

#define WALL_LOST_DEG2		15

#define GO_TYPE_SAVE	0x00
#define GO_TYPE_NO_SAVE	0x01

#define OUT_TYPE_Y				0x01		//工字型的方式
#define OUT_TYPE_SIDE			0x02		//侧面斜过去
#define OUT_TYPE_Z				0x03		//反向工字型
#define OUT_NEAR_BACK			0x04		//延边回去画未清扫区域
#define OUT_TARGET_RUN			0x05		//特征物延边走法
#define OUT_TARGET_BACK			0x06

#define Y_ORG_OFFSET			0			//工字型的时候补偿的霍尔数，要不然工字型出来Y轴的坐标回变大,0 就不补偿

#define T_GO_FW_PWM			0
#define T_GO_NEAR_PWM		1
//#define T_GO_SLOW_PWM		2
#define T_GO_TURN_DEG		3

#define FW_SLOW_PWM			650//sys->pwm//620
#define BACK_OFF_PWM		700//550//650//(sys->pwm+80)//cfg->go_back_pwm
#define GO_FORWARD_PWM		cfg->walk_pwm//650//sys->pwm//cfg->go_forward_pwm
#define GO_ROUTINT_PWM		680//sys->pwm//cfg->go_route_pwm
#define TURN_DEG_PWM		cfg->turn_pwm//680//700//sys->pwm  600
#define GO_NEAR_PWM			650//500
#define GO_NEAR_PWM_FAST	650//570

/*
#define FW_SLOW_PWM			get_pwm(T_GO_FW_PWM)//780//sys->pwm//620
#define BACK_OFF_PWM		get_pwm(T_GO_FW_PWM)//780//650//(sys->pwm+80)//cfg->go_back_pwm
#define GO_FORWARD_PWM		get_pwm(T_GO_FW_PWM)//780//sys->pwm//cfg->go_forward_pwm
#define GO_ROUTINT_PWM		get_pwm(T_GO_FW_PWM)//780//sys->pwm//cfg->go_route_pwm
#define TURN_DEG_PWM		get_pwm(T_GO_TURN_DEG)//700//sys->pwm
#define GO_NEAR_PWM			get_pwm(T_GO_NEAR_PWM)//700
#define GO_NEAR_PWM_FAST	630
*/
#define WAIT_STOP_COUNT		150
#define WAIT_STOP_CNT		2000

#define IDR_PRINTF()	log_printf("    [%d,%d,%d]\r\n",LEFT_ADC(),MIDLE_ADC(),RIGHT_ADC())

#if (1)
#define BACK_HW		8
#else
#define BACK_HW		4
#endif
#if CALE_BY_BKHW		//后轮计算


	#define CM_PER_HW		29//31			//每29个霍尔一厘米
	#define HW_HF_GRID		225
	#define HW_GRID			450//30	 15m一格		//多少个霍尔，一格	 200,7mm左右一格
	#define INC_HW			600//(50*WHELE_HW)
	#define RADIUS			338.0f		// 半径 ,两轮子直径226mm，计算的结果336.6左右，反正都不准，就去340


/*
速度设置，250ms一个速度环，估算290的速度，是0.4m/s

*/
//吸尘电机速度
#define DUST_MODE_SILENT		0x00
#define DUST_MODE_NORMAL		0x01
#define DUST_MODE_TURBO			0x02

//滚刷速度
#define AGI_H_SPEED			 (cfg->h_agi_speed)//		  AGITATOR_PWM 999 									//高速
#define AGI_L_SPEED			(cfg->l_agi_speed)//											//低速
#define AGI_M_SPEED			((cfg->h_agi_speed-cfg->l_agi_speed)/2)//(AGI_L_SPEED + (AGI_H_SPEED - AGI_L_SPEED)/2)//				//中速
#define AGI_MH_SPEED		((cfg->h_agi_speed-cfg->l_agi_speed)/2)//(AGI_H_SPEED - (AGI_H_SPEED - AGI_M_SPEED)/2)//				//中高速
#define AGI_ML_SPEED		((cfg->h_agi_speed-cfg->l_agi_speed)/2)//(AGI_L_SPEED + (AGI_M_SPEED - AGI_L_SPEED)/2)//				//中低速
//行走速度
#define HIGHT_SPEED			cfg->h_run_speed//350			//高速
#define MIDLE_SPEED			cfg->m_run_speed//300			//中速
#define LOW_SPEED			cfg->l_run_speed//250			//低速

#define MIDLE_H_SPEED		cfg->m_run_speed//(cfg->m_run_speed+25)//300		//中高速
#define MIDLE_M_SPEED		cfg->m_run_speed//(cfg->m_run_speed-17)//300		//中中速
#define MIDLE_L_SPEED		cfg->m_run_speed//(cfg->m_run_speed-34)//300		//中低速

//165显控板(888)的使能开关,
//1时为只有显控板,是基于PCB V1.6的改装版本 165
//2时是基于PCB V1.7的新版本,中扫电机的电流监控为AD采样 888
//3时为无888的PCB V1.7版本,安装的是按键板,电流为AD采样,按键与LED与V1.6不同,前撞测踏板以红外电流开关脚也不相同
//4时是基于PCB V1.8的版本，较V1.7，增加左右边刷电流ADC采样,ADC采样通道为原前撞红外左2与右2，现左2与右2分别为IO模式的PE9与PE15
#ifndef DP_PANEL_EN
#define DP_PANEL_EN 1
#endif

#define LASER_RES				15		//分辨率，15mm一格
#define ORG_AJUST_HW	1

#define MAX_X		7000
#define MAX_Y		7000

#define RC_GRID		0			//25宫格，每边有3个格子
#define WALK_DIST()     motor.c_left_hw//navigat->distance
//右轮
#else
#define HW_HF_GRID		225
#define HW_GRID			450//30	 15m一格		//多少个霍尔，一格	 200,7mm左右一格
#define INC_HW			600//(50*WHELE_HW)
#define MAX_X			7000
#define MAX_Y			7000

#define WALK_DIST()     navigat->distance

#endif

#define save_line_xy(x,y)
#define printf_clr_are()
#define	BLACK			0
#define	BLUE			0xAA0000
#define	GREEN			0x00AA00
#define	CYAN			0xAAAA00
#define	RED				0x0000AA
#define	MAGENTA			0xAA00AA
#define	BROWN			0x0055AA
#define	LIGHTGRAY		0xAAAAAA
#define	DARKGRAY		0x555555
#define	LIGHTBLUE		0xFF5555
#define	LIGHTGREEN		0x55FF55
#define	LIGHTCYAN		0xFFFF55
#define	LIGHTRED		0x5555FF
#define	LIGHTMAGENTA	0xFF55FF
#define	YELLOW			0x55FFFF
#define	WHITE			0xFFFFFF
#define draw_dot(X,Y,K)


#define D2R		0.0174533f

#define CROSS_XY_NAVI		0x00		//越过去了，需要导航
#define CROSS_XY_OK			0x01		//导航到位，不再导航
#define CROSS_XY_ERR		0x02		//地图错乱了，需要反工字型
#define CROSS_XY_POINT_OK	0x03		//导航到点了。
//typedef 	 int 	ORG_T
//typedef 	short   MAP_T;
#define CHECK_NAVI_STA2()		{if(sys->sState !=SYS_NAVIGAT  && sys->sState != SYS_DOCK && sys->sState != SYS_FOCUS && sys->sState !=SYS_RANDOM && sys->sState !=SYS_DOCK_NEAR) { motor_run(GO_STOP,0,0,0); STOP_ALL_MOTOR() ; log_printf("sta(%d) err ,stop all motor\r\n",sys->sState); return 0;}}
#define CHECK_NAVI_STA()		{if(sys->sState !=SYS_NAVIGAT  && sys->sState != SYS_DOCK && sys->sState != SYS_FOCUS && sys->sState !=SYS_RANDOM && sys->sState !=SYS_DOCK_NEAR) { motor_run(GO_STOP,0,0,0); STOP_ALL_MOTOR() ; log_printf("sta(%d) err ,stop all motor\r\n",sys->sState); return ;}}
#define CHECK_NAVI_STA_RT(X)		{if(sys->sState !=SYS_NAVIGAT && sys->sState != SYS_DOCK && sys->sState != SYS_FOCUS && sys->sState !=SYS_RANDOM && sys->sState !=SYS_DOCK_NEAR) { motor_run(GO_STOP,0,0,0); STOP_ALL_MOTOR() ;log_printf("sta(%d) err2,stop all motor\r\n",sys->sState);return (X);}}

#define CHECK_NAVI_STA_ONLY()		{if(sys->sState !=SYS_NAVIGAT) { motor_run(GO_STOP,0,0,0); /*STOP_ALL_MOTOR() */; return ;}} //不能停掉所以电机，给回充留点电机

#define CHECK_IDLE_STA()		{if(sys->sState == SYS_IDLE)return;}
#define CHECK_IDLE_STA_RT(X)		{if(sys->sState == SYS_IDLE)return X;}
/*

 每一格(15cm)大约35个霍尔，最大200个 200 * 35 = 7000
*/
#define DONT_SEARCH_UNCLR	0x00		//不搜索未清扫区域，直接导航到目的地
#define SEARCH_UNCLR		0x01		//搜索未清扫区域，然后再导航过去

#define NO_SIDE			0x00
#define LEFT_SIDE		0x01
#define RIGHT_SIDE		0x02
#define MIDLE_SIDE		0x03

#define ROALD_LEN		20
#define NAVI_LEN		20

#define TYPE_ENDPOINT	0x00	//导航终点,导航终点始终不删
#define TYPE_TEMPOINT	0x01	//临时点，导航过程中，有碰撞，则删掉
#define TYPE_DOCK_AJUST	0x02	//导航点
#define SIDE_BUM_LEN	3

#define UC_LEN		30
#define DOOR_LEN		20

#define CROSS_X			0x01
#define CROSS_Y			0x02

#define INC_RAISE		1
#define INC_FAILL		-1

#define AJUST_X			1
#define AJUST_Y			2
#define AJUST_MOVE		4		//移动了。
//导航状态
#define NAV_STA_NORMORL			0	//正在弓字形
#define NAV_STA_NAVIGATE		1	//导航
#define NAV_STA_REVEV			2	//反向弓字形
#define NAV_STA_DOCK_AJUST		3	//90度。270度的根据座子的中线的校准
//#define NAV_STA_ADJ				4	//特征物走法


#define NO_NEAR					0x00
#define NEAR_WALL_RIGHT			0x01
#define LOST_WALL_RIGHT			0x02

#define NEAR_WALL_LEFT			0x03
#define LOST_WALL_LEFT			0x04

#define NO_SIDE_NEAR		0
#define LEFT_SIDE_NEAR		1
#define RIGHT_SIDE_NEAR		2
#define NEAR_BY_IRDA		20


#define  MAX_OBST			1000			//miconydrawmap		1000	->10
#define  MAX_J_OBST			200

#define NAVIGAT_OK			0x01
#define NAVIGAT_ERR			0x02
#define NAVIGAT_AJUST		0x03
#define NAVIGAT_SERCH_ERR	0x04


#define MAX_DROP_COUNT		5


#define WHELE_HW			9


#define		DOCK_ERR     	0
#define 	DOCK_NO_FOUND	1
#define 	DOCK_OK			2
#define 	DOCK_TO_X		3		//到了指定的调整的X值

#define 	MAX_TARGET		50
#define		MAX_ADGE_LEN	1000


#define ADJ_FAIL  			0x01	//特征物走法失败
#define ADJ_MID_BUM			0x02	//中间碰撞,100%校准
#define ADJ_BUM				0x03	//距离相差1个格子内，100校准,
#define ADJ_BUM_SIDE		0x04	//
#define ADJ_SIDE_LOST		0x05	//侧面丢失,两个格子内80%校准

#define GO_NEAR_TYPE_NO 	   0x00 	   //普通延边
#define GO_NEAR_TYPE_NAVI	   0x01 	   //中间如果能导航到终点，则退出去导航
#define GO_NEAR_TYPE_ADJ	   0x02			//adj特征点回去
#define GO_NEAR_DRAW_MAP	   0x03		  	//延边画地图
#define GO_NEAR_TYPE_DOCK	   0x04			//回充


#define RET_NEAR_ERROR		0x00
#define RET_NEAR_OK			0x01
#define RET_NEAR_TIMEOUT	0x02
#define RET_NEAR_FITT		0x03	//拟合地图存储用完
#define RET_NEAR_ADJ		0x04	//拟合地图
#define RET_NEAR_HW_OVER	0x05	//行程用完

#define STORT_XY()		{bx = X_NOW;by = Y_NOW;}


#define Y_LENE		0X00	// X轴不变，Y轴一条线
#define X_LINE		0x01	//Y轴不变，X轴一条线

#define LINE_TYPE_BUM		0x00
#define LINE_TYPE_LOST		0x01

#define  NEAR_GO_CLEAN_IN_FRONT		0x10

#define X_NOW				navigat->tx
#define Y_NOW				navigat->ty
#define A_NOW				sys->angle

#define drow_point(X,Y,z)

#define  FRONT_TO_ORG		1.2//1.17f
#define  ORG_TO_FRONT		0.83//0.85f

#define CELL_AREA			225		//15cm*15cm
#define POINT_TO_AREA(x)		(x * CELL_AREA)
typedef struct obst_t
{
	short 		x;
	short 		y;
	int 		x_org;		//激光版本，是1mm级别
	int 		y_org;		//激光版本，是1mm级别

	short 		tx;
	short 		ty;
//	unsigned short	ir;		//侧面的红外

}OBST;
#define MAX_LINEX		50
//一条X线，
typedef struct linex_t
{
	signed short tx;
	signed short ty;
	signed short dir;
}LINEX;
//边缘
typedef struct edge_t
{
	signed short 		x;
	signed short 		y;
	int 		x_org;		//激光版本，是1mm级别
	int 		y_org;		//激光版本，是1mm级别

	/**/
	//unsigned short	ir;		//侧面的红外		高位 1，则清扫方位
	//int8_t		x_dir;
	//int8_t		y_dir;

}EDGE;


typedef struct tobst_t
{
	//开始的点
	short bx;
	short by;
	//结束的点
	short ex;
	short ey;
	//测试的点的坐标
	//short tx;
	//short ty;
	//开始和结尾的时间的状态
	unsigned char	bret;
	unsigned char eret;
	short 	x_dir;	//X的方向，在0度方向做adj，x_dir=1,也就是说，是障碍物相对于清扫点的方向，障碍物>清扫点，则为1
	int 	x_org;
/*
	short x;
	short y;
	int x_org;
	int y_org;
	short x_dir;	//X,轴可去的方向， 是改点对于障碍物的方向， (x-x_dir,y)就是障碍物的点
	short y_dir;
	unsigned char side;
	unsigned char sta;
	short idx;		//序号
//	int dist;
	float  angle;
	*/
}TOBST;

typedef struct majust_t
{
	//点位
	short x;
	short y;
	//原始坐标
	short x_org;
	short y_org;
	//结束点
	short x_end;
	short y_end;

	short x_begin;
	short y_begin;
//	int dist;
	float  angle;
}MAJST;

#define  MAX_NAVI_ERR		200
#define  MAX_NAVI_REP		10

//存储导航失败的点
typedef struct navi_err_t
{
	signed short x;
	signed short y;
	signed short c;
}NVIER;

typedef struct near_wall_t
{
	//unsigned char 	sta;
	unsigned char		n_sta;		//沿边的状态 0 - 不沿边，1-作沿边，2-右沿边
	unsigned char		turn_dir;	//碰撞时的转向
	//unsigned char		side_sta;	//
	unsigned char		c_left;
	unsigned char 	c_right;
	unsigned char		c_adc;

//	unsigned short 	lost_dist;		//计算失去多少个霍尔后，才转弯
//	unsigned short 	c_near;		//沿边的次数
//	unsigned short 	c_lost;		//计算失去墙的时间
	//float		l_angle;	//丢失时候的角度

	NEAR_PID 	*pid;

//	unsigned short	count;		//计数角度的累计和
	//unsigned short	previ_adc;	//上次的ADC
//	float		angle;
//	float		yaw;

}NEAR_WALL;
//一个点的数据
typedef struct point_t
{
	short x;
	short y;
	int x_org;
	int y_org;
	float angle;
}POINT;

struct laser_point_t
{
	short m_dist;
	float m_deg;
};
//
//激光数据
#if LASER_EN
#define MAX_LASER_SCAN		500
#else
#define MAX_LASER_SCAN 	1

#endif
typedef struct laser_t
{

	//上次测试的数据，用来矫正本次的数据
	short 		last_x_org;
	short		last_y_org; 	//上次出的的圆心坐标
	unsigned int 	last_timer;	//上次测距的时间
	short 		last_dist;
	float		last_deg;

	short 		c_scan;
	short		c_dist;			//角度，距离的计数
	short		m_dist[MAX_LASER_SCAN];			//距离
	float		s_angle[MAX_LASER_SCAN];
} LASER;

#define MAX_IR_SCAN		100
typedef struct ir_scan_t
{
	int 		len;
	unsigned short	ir[MAX_IR_SCAN][7];
	float		angle[MAX_IR_SCAN][7];
}IR_SCAN;
#define ORG_SAVE_LEN		20
#define MAX_WALK_GRID		50 //30米的行走距离

//直行的时候，临时的侧面障碍物，先临时存储，用于矫正 大约耗费5K 14*14 =5600
#define MAX_SIDE_OBST		200
typedef struct side_obst_t
{
	unsigned short  len;
	OBST obst[MAX_SIDE_OBST];
}SIDEOBST;
#define MAX_Z_XLINE		200

/*
清扫线路
*/
typedef struct x_line_t
{
	unsigned short bx;
	unsigned short by;
	unsigned short ex;
	unsigned short idx;
	unsigned char	 clean;
	unsigned char  r;
	//struct x_line_t *prev;
	//unsigned short ey;
}XLINE_T;
//反向工字型回来时的矫正地图数据
typedef struct xline_z_t
{
	unsigned short len;
	XLINE_T  xline[MAX_Z_XLINE];
}Z_XLINE;

//#define MAX_WALK_GRID		30

typedef struct func_arg_t
{
	unsigned char type;
	unsigned char	type1;
	unsigned char	arg8_1;
	signed short	arg16_1;
	signed short arg16_2;
}FUNC_ARG;

typedef struct xy_t
{
	signed short	x;
	signed short y;
}XY_T;


#define MAX_M_EDGE		500

typedef struct h_targe_t
{
	//对应的XY队列里的开始结束
	signed short bidx;
	signed short eidx;
	signed short	deg;
	//对应的开始的xline的序号
	//signed short xline;
}TARGET_T;
#define MAX_TARGET_T		100

//地图的边缘
typedef struct m_edge_t
{
	unsigned short 	len;
	signed short 		tx[MAX_M_EDGE];
	signed short 		ty[MAX_M_EDGE];
	signed short 		ox[MAX_M_EDGE];		//障碍物
	signed short 		oy[MAX_M_EDGE];		//障碍物
}MEDGE;



#define MAX_P_NAVI		100		//最多存储10个导航点
typedef struct h_navigat_t
{
	unsigned int gun_cnt;
	unsigned char sta;			//导航状态
	unsigned char is_walk;		//是否在移动，只有在移动的时候，才计算坐标系
	unsigned char	navi_ok;		//清扫完毕
	unsigned char	adj_run;		//特征物走法
	unsigned char	next_side;		//保存的下一个动作
	unsigned char need_back;		//需要180°掉头
//	unsigned char navi_to_dock;
//	unsigned char	wet_sta;		//湿拖模式
	unsigned char clean_ok;		//清扫完毕
	unsigned char	need_adj_dock;	//需要矫正座子，离座子比较远后，要矫正了。
	unsigned char navi_to_dock;	//导航回充电桩

	unsigned char	whele_stop;		//轮子是否停止了，后边再改，懒得传参数了。

	unsigned char to_side;		//导航的的方向
	unsigned char	m_navi_run;		//导航过去后，第一次走，此时穿过障碍物需要校准
	unsigned char	force_z_go;		//强制工字型的次数

	unsigned char	sheepflod;		//羊圈里
	unsigned char	rever_navi;
	unsigned char navi_near;		//延边走

	unsigned char	c_org_navi;		//导航回原地的次数

	unsigned char can_navi;			//能导航过去
	unsigned char navside;
	unsigned char backsta;
	unsigned char	arc_z_go;			//弧线掉头


	unsigned short	c_reverse_go;		//反向运动
	unsigned short	c_z_go;				//正向运动
	signed short	first_bx;
	signed short	first_ex;
	unsigned char clean_right_record;		//2018-10-27 jzz 记录是否清扫过右侧




	//unsigned char right_side_cln;	//右边是否被清扫了。右边扫完后，再导航到左边去看看有没有未清扫的。
	//unsigned short	gSta;		//直行碰撞的状态，由于直行后得到刚才碰撞的状态

	int x1_org,y1_org;		//上一点的原始坐标
	int	x_org,y_org;		//本点的原始坐标
	int	k_x_org,k_y_org;		//本点的前轮原始坐标

	int x_org_l,y_for_l;
	float x_org_f,y_org_f;		//浮点d坐标系
	float x_org_r,y_org_r;		//右轮计算的
	float x_org_t,y_org_t;		//前轮计算的坐标
//	float x_orgf1,y_orgf1;		//后轮计算的浮点坐标
	//float x_orgf_l,y_orgf_l;	//前轮停钱，最后的时刻的坐标，用于计算前轮是否打滑丢步
	//float x_orgf_pre,y_orgf_pre;	//上一时刻
	//float x_orgf1,y_orgf1;		//后轮计算的浮点坐标
//	int mx,my;				//后轮计算的坐标
	signed short x,y;				//转换过的坐标系
	signed short	tx,ty;
	signed short kx,ky;			// 前轮坐标
	//int	txx,tyy;		//临时的坐标变量
	signed short x_dock;				//需要找座子的位置
	signed short dock_x;		//充电桩的坐标
	signed short dock_y;
	int l_dist;				//上一次行走的距离
	volatile int	distance;	//
	short	tox,toy;			//计算出的导航该去的地点
	signed short	mtox,mtoy;
	int	toAgl;				//到导航点后，应该走的的方向，0或180
	short toxx[4];			//导航路径
	short toyy[4];			//导航路径
	short x_road[ROALD_LEN];	//导航路径
	short y_road[ROALD_LEN];
	short c_road;				//路径
	short	side_obst;		//侧面是否要计算障碍物
	short	c_side_obst;
	short x_avr[2];			//用于X轴的矫正。
	short c_avr[2];
	short osbt_x;
	short osbt_y;
	short scan_xx;
	short mx,my;				//断点续传的点
	short y_min_draw;
	short y_max_draw;

	signed short ntox;	//最后的要导航去的地址
	signed short ntoy;

	//存储原始坐标的矫正前的数据
	/*
	short x_org_s[ORG_SAVE_LEN];
	short y_org_s[ORG_SAVE_LEN];
	*/
	short x_walk[MAX_WALK_GRID];		//行走的坐标点X
	short y_walk[MAX_WALK_GRID];		//行走的坐标点X

	//充电桩的位置。
	short tx_dock[3];
	short ty_dock[3];

	short c_dock_l;		//发现左边的次数
	short c_dock_m;		//发现中间的次数
	short c_dock_r;		//发现右边的次数
//	short org_save_len;
	short walk_len;						//行走的坐标点数
	signed short dock_adj_y;			//记录上次用充电桩矫正的地址，避免重复的矫正
	unsigned short	c_near_0;		//0度的延边次数
	unsigned short	c_near_180;		//180度方向的延边次数
	unsigned short	c_v_go;			//垂直矫正后，走的距离
	unsigned short	c_zgo_bum;		//计算有多少次工字清扫
	unsigned short	c_zgo_err;
	unsigned short	navi_xline_index;		//导航到的点的XLINE
	signed short		c_target_go;
	//unsigned short 	mapedge_len;

#if TEST_FOR_LABO
	short c_side_obst;
#endif
	//int inc_wheel;		//后轮的增长速度。
	/*
	//int ajust_widows;		//矫正滑动窗口
	short dir_0_bum_x[SIDE_BUM_LEN];	//0度方向的碰撞
	short dir_0_bum_y[SIDE_BUM_LEN];	//0度方向的碰撞
	short dir_0_bum_len;
	short dir_180_bum_x[SIDE_BUM_LEN];	//0度方向的碰撞
	short dir_180_bum_y[SIDE_BUM_LEN];	//0度方向的碰撞
	short dir_180_bum_len;
	*/
	short walk_dis;		//走的距离
//	int	wheel_dir;		//后轮方向
	//int x_obst,y_obst;	//暂存的障碍点
	unsigned char to_type[ROALD_LEN];		//中间导航点的类型
//	int navi_x[NAVI_LEN];		//存储的导航点
//	int navi_y[NAVI_LEN];
	short tolen;				//计数导航路径
	//未清扫区域
	signed short 	uc_x[UC_LEN];
	signed short 	uc_y[UC_LEN];
	unsigned char 	uc_side[UC_LEN];
	unsigned short	uc_len;
#if PARTITION
	signed short   m_by;		//本次开始的Y

#endif
	int hw_grid;
	short 	i_obst;
	short 	j_obst;
	short		idx_adj;
	float		adj_x_org;		//特征物走法的Y轴的原始坐标，强制执行
	//short		lst_obst;		//上一个特征物的序号，用了判断本次特征物是否值得取
	//转的圈数，和走的次数
	unsigned short 	c_turn;
	unsigned short		c_go;
	unsigned short	c_go_navi;
	short		c_go_force;
	short	draw_map_bx;
	unsigned short p_navi_len;		//导航的的次数

	/*
	int	door_x[DOOR_LEN];
	int door_y[DOOR_LEN];
	int door_len;
	*/
	//int navilen;
	//int tox1,toy1;			//中间有临时去的点，则先存储导航点。
	short	nextx,nexty;		//计算出的下一点该去的地点
	short max_ty;
	short max_tx;
	short min_tx;
	short min_ty;
	short scan_x;			//本次扫描过的X轴的个数，用于计算侧面有障碍物的时候，直接导航，无需再走过去。
	short ajust_y;		//存储测斜的位置
	float ajust_agle;

	float radius;
	//int	before_uclr_y;	//进入未清扫区域前的y轴备份
	unsigned char	adj_run_ret;		//存储特征物沿边走的结果
	unsigned char side;
	unsigned char old_side;
	unsigned char side_sta;			//边的状态。
	unsigned char	c_navi_bum_y;		//同一条Y轴上碰撞的次数
	unsigned char c_near;				//沿墙的次数，要沿两次的。
	//unsigned char c_side_bum;
	//unsigned char routering;			//正在路径导航
	unsigned char	ret_search;			//搜索路径结果 1-搜索到未清扫区域，0，搜索到中间区域
	unsigned char backClean;
	unsigned char	navi_side;			//导航的方向
	unsigned char	is_bum;				//直行的时候是否碰撞
	unsigned char c_side_bum;			//侧面碰撞的次数
	unsigned char	bum_side;
	unsigned char first_bum;
//	unsigned char	ajust_run;			//校准的走法，1-走，0-不走
	unsigned char from_charge;		//从充电座出来
	unsigned char	found_charge;		//发现充电桩
	unsigned char is_cross_obst;
	//校准，寻找特征物的变量
	unsigned char	adj_nsta;			//沿边的边
	unsigned char	adj_c_lost;
	unsigned char c_navi_err;
	unsigned char c_navi_repeat;

	unsigned char	mside;

	unsigned char	force_obst;		//强制置障碍物
	unsigned char suspend;		//暂停，不清理内存。
	unsigned char draw_map_flage;	//延边画地图的标志，标志是否去过Y-的方向，去过Y-的方向，再回到Y的原点，才算画图完毕
	signed short	arg[4];			//中间的变量参数，在直行函数的时候用到。

	short adj_walk_dis;		//沿边走的路程
	short walk_speed;			//移动速度

	int   	draw_map_bk_angle;		//画地图的时候，矫正时回去的角度
	short	draw_map_adj_x;			//画地图矫正的障碍物

	unsigned short  c_z_bum;			//工字型的碰撞次数
	unsigned short  c_m_bum;			//中间碰撞的次数，

	unsigned short	edge_len;
	unsigned short 	c_adj_navi_near;		//导航延边回来的次数
#if LINEX_EN
	signed short		c_linex;
#endif

	unsigned int	t_navigat;		//导航的时间
	unsigned int 	t_gyro_adj;		//陀螺仪最后矫正的时间
#if  TARGE_RUN
	signed short   	i_target;
#endif
//	unsigned char	near_sta;			//沿边模式
	//unsigned char c_snack;			//蛇形的次数
	//unsigned char force_side;			//强制运行的方向，用于进入到未清扫区域时，强制的方向。
//	unsigned char mask_side;
	float	to_angle;
	float	angle;			//角度
	float	lst_angle;		//最后运行过来的角度
	float  	out;
	float	dis_angle_adj;
	unsigned char obstacle;
	unsigned char scan;
	//unsigned char walkmap[MAX_GRID_LIN];			//扫描地图走的路径
	OBST 	m_obst[MAX_OBST];			// 激光版本 12*1000 = 12K 普通版本  8 * 10000 = 8K
#if AJUST_RUN
	//OBST adj_obst[MAX_J_OBST];
	TOBST	adj_obst[MAX_TARGET];		//特征物 16K
	TOBST	*l_adj_obst;		//最后去校验的特征物
#endif
	NEAR_WALL	near;
	NVIER		e_navi[MAX_NAVI_ERR];
	NVIER		r_navi[MAX_NAVI_REP];		//重复导航的
//#if LASER_EN || LASER_SIDE
//	LASER		laser;
//#endif
	//IR_SCAN 	*ir_scan;
	POINT		poffset;	//用于临时保存的点差值，在校准的时候，保存一下，并且计算出偏移量的差值，用于回滚
	SIDEOBST	*g_obst;	//拟合地图的时候，暂存的障碍点地图

	FUNC_ARG	func_arg;		//运行时候的参数。


}navigat_t;

typedef union
{
	navigat_t navigat_buffer;
	UPGRADE_BUFF_T upgrade_buffer;
}
SHARE_RAM_U;

/*共享*/
extern SHARE_RAM_U share_ram_buff;


typedef signed short (*m_go_func)(void);

extern navigat_t *navigat;
void navigat_init(char flage);
void coord_org2map(int x,int y,short *xx,short *yy);
void coord_map2org(short xx,short yy,int *x,int *y);
void proc_navigat_task(unsigned short sta);
void motor_turn_deg(char dir,int deg);
void motor_back_off(int hw);
char motor_go_forwark(int hw,unsigned char nsta,m_go_func m_go_check);

char go_to_unclr(char sta,short bx);
void save_map(char clac,char scan,char obst);
char check_back_clr(void);
char parallel_forwark_y(int hw1,int dir);
char search_unclear(short *tx,short *ty,float *agnle);
char search_route(short x,short y,short tox,short toy,short *nx,short *ny);
char always_wak_map_cord_y(short y,float angle,int dir);


void turn_to_deg(int deg);
char walk_map_cord(short x,short y,float angle,int bk_dis);
void cord_calc_store(char reset_dist);
char walk_org_cord(short x,short y,float angle,int bk_dis);
char navigat_robot(char type,char t_search,short tox,short toy,float toagle);
void delay_for_gyro(void);
unsigned char clean_sta(short x,short y);
char walk_map_line(short x,short y,float angle,int bk_dis);
int disxy(short x,short y);
char search_cross_y(short x,short y,short tox,short toy,short *nx,short *ny,char flage);
char osbt_sta_x(short y1,short x1,short y2);
char clean_in_front(int cnt);
char search_bum_route(void);
void save_navigat_point(void);
char osbt_sta_x_noscan(short x1,short y1,short x2);
char osbt_sta_y_noscan(short x1,short y1,short y2);
char search_cross_y_noscan(short x,short y,short tox,short toy,short *nx,short *ny);
char search_route_sub(short x,short y,short tox,short toy,short *nx,short *ny);
char judge_obst(unsigned short gsta,int hw);
char walk_for_navigat(void);
unsigned char proc_z_go_bum(unsigned short sta,short bx,short by);
char vertical_wall(unsigned char side_idx);
void insrt_vertical(unsigned short value);
char vertical_big_than(unsigned short min);
unsigned short get_adc(unsigned char side);
char router_p2p(short bx,short by,short tox,short toy,short *nextx,short *nexty);
char proc_walk_line_task(char sta);
void ajust_angle_after_near_pid(void);
char motor_go_forwark_ajust(int hw);
char proc_navigat_bum(short tox,short toy,unsigned char unclr);
void set_osbt_side(void);
void navigat_near_turn(void);
char back_bum_unclr(void);
//char navigat_to_point(int tox,int toy,int toangle);
char search_near_clean(void);
void save_obs_side(char obst);
void sign_unclr(void);
void walk_for_calibration(float to_angle);
void navigat_calibration(void);
void init_navigate_gobal(void);
char robot_moves(int hw);
void set_osbt(short x,short y,unsigned char value,unsigned char type,char *str);
char proc_side_bum(int tdir);
char always_walk(short tox,short toy,unsigned char type,float angle,int dir,char turn,int c_snak);
void ajust_xy_map(short x,short y,unsigned char type);
void ajust_xy_obst(unsigned char adj,short bx,short by,int xdir);

void insert_osbt(short x,short y,unsigned short ir);
OBST *get_obst(short x,short y);
unsigned char robot_turn_deg_getout_trouble(unsigned char dir,int pwm,float agle);
unsigned char  robot_turn_deg(unsigned char dir,int pwm,float agle);
char motor_go_near_wall(uint8_t n_sta);
char nearwall_cross_map(int y,float angle);
void init_near_wall_navi(unsigned char n_sta);
TOBST *get_adj_obst(short x,short y);
char osbt_sta_y(short x1,short y1,short y2);
char walk_for_dock_ajust(int y_org);
char walk_for_dock(short xx,short yy);
int unclear_round(short x,short y);
void go_near_wall(unsigned char side);
char navigat_to_target(unsigned char type);
int ajust_recover_obst(void);
signed short navi_go_forwark_check(void);
void ajust_y_on_move(void);
void printf_scan_obst(void);
int search_navi_err_table(short x,short y);
void calc_timr_upload(void);
NVIER * get_navi_err_stu(short x,short y);
char search_route_depth(short bx,short by,short tox,short toy,short *nx,short *ny);
char router_extend_begin(short bx,short by,short tox,short toy,short *nextx,short *nexty);


int check_now_point(short x,short y,int side_sta);
char navigat_to_point(short tox,short toy,float toagle);
char walk_map_cord_only(short x,short y,float angle,int bk_dis);
char motor_go_forwark_only(int hw,unsigned char nsta);


void save_router(void);
void load_router(void);
int check_adj_line(short nx,short ny,int x_dir,int y_dir,short *xx,short *yy);
int search_adj_line(short nx,short ny,short tox,short toy,short *xxo,short *yyo,short *ax);
int motor_go_for_adj(int hw,unsigned char nsta);
int navi_to_adj_near(short nx,short ny,short tox,short toy);
int search_adj_line_on_road(short nx,short ny,short tox,short toy,short *yyo);
char near_cross_xy(short tox,short toy,unsigned char ctype,unsigned char n_sta);
char motor_go_edgeways(unsigned char n_sta ,short tox,short toy,unsigned char type,unsigned char is_save);
int navi_to_new_adj(short bx,short by,short ex,short ey,short dist);
int check_y_line(short xx,short yy,short *rx,short *ry,short *ox,short *oy,short *deg);
int ajust_y_line(void);
void set_scan(short x,short y,unsigned char value);
void set_scan_point(short x,short y,int scan,int c_grid);

void insert_osbt_l(short x,short y,short x_org,short y_org,short tx_org,short ty_org);
void near_wall_test(void);
//int in_obst_f(struct obst_fifo_t *obst_f,int x,int y);
int ajust_xy_by_near_line(short bx,short by,short ex,short ey,float angle,unsigned char type,unsigned char n_sta);

int proc_walk_road(navigat_t *navi,short bx,short by,unsigned short gsta);

signed short proc_navigat_reverse(void);
unsigned char z_go_for_map(unsigned char side,signed short tx,signed short ty,unsigned short count);
unsigned char insert_navi_err_table(signed short x,signed short y,signed short count);
unsigned char check_xline_sta(signed short bx,signed short by,signed short ex);
unsigned char check_dock(signed short tx,signed short ty,float angle);

unsigned char proc_z_go_navi(unsigned char sta);

unsigned char adj_go_nearwall(signed short tx,signed short ty);
unsigned char i_go_for_navi(unsigned char side);
unsigned char proc_back_unclr(signed short bx,signed short ex);

unsigned char navi_go_dock_adj(unsigned char type);
unsigned char proc_side_near_wall(signed short bx,unsigned char side,float angle);

void init_focus_task(void);
int proc_focus_task(void);
unsigned char navi_to_dock(void);
void go_and_back_test(void);
signed short front_line_unlr(void);

unsigned char walk_to_unclr_line(signed short tox,signed short toy,unsigned char side);
char clean_front_by_walk(void);
unsigned char need_turn_back(signed short tx,signed short ty,float angle);
NVIER * get_navi_repeat_stu(short x,short y);
unsigned char insert_navi_repeat_table(signed short x,signed short y,signed short count);
void clear_navi_repeat_table(void);
int search_navi_repeat_table(short x,short y);
void nearwall_test_12(void);
unsigned char check_go_back(signed short x_dir);
signed short func_nearwall_back_org(void);
char search_unclear_only(void);
void navigat_reset(void);
unsigned char unclr_xline(XLINE_T *xline,signed short *tox,signed short *toy,unsigned char type);
char nearback_for_unclr(signed short bxx,signed short byy,unsigned char type);
char path_navigation(short tox,short toy);


#endif
