
#ifndef __TEST_MODE__
#define __TEST_MODE__

//定义系统软件版本，用来上报到测试软件
#define FW_VERSION	180

#define USE_ON_COMPUTER 0

#if 0
enum __tproc_code{
	TPC_STOP_ALL = 0,//停止所有检测
	TPC_START_ALL,//开始所有检测
	TPC_CD_FRT,//前轮码盘检测
	TPC_WHL_L,//左后轮检测
	TPC_WHL_R,//右后轮检测
	TPC_SMT_L,//左边刷检测
	TPC_SMT_R,//右边刷检测
	TPC_MT_MID,//中扫电机检测
	TPC_MT_FAN,//风机检测
	TPC_IR_FRT,//前撞红外检测
	TPC_IR_BOT,//对地红外检测
	TPC_IRDA_CHRG,//回充红外检测
	TPC_BUM_SW,//前撞传感器检测
	TPC_SWSENSOR,//其他传感器检测(抹布/虚拟墙/尘盒/集尘盒满)
	TPC_CHRG_TST,//充电检测
};
#else
#define TPC_STOP_ALL	0//停止所有检测
#define TPC_START_ALL	1//开始所有检测
#define TPC_CD_FRT	2//前轮码盘检测
#define TPC_WHL_L	3//左后轮检测y
#define TPC_WHL_R	4//右后轮检测y
#define TPC_SMT_L	5//左边刷检测n
#define TPC_SMT_R	6//右边刷检测n
#define TPC_RESET	8//与其他状态下的重启指令一致
#define TPC_MT_MID	7//中扫电机检测
#define TPC_IR_FRT	9//前撞红外检测
#define TPC_IR_BOT	10//对地红外检测
#define TPC_IRDA_CHRG	11//回充红外检测
#define TPC_BUM_SW		12//前撞传感器检测
#define TPC_SWSENSOR	13//其他传感器检测(虚拟墙/尘盒/集尘盒满/左落轮/右落轮/超声波/后轮码盘)
#define TPC_CHRG_TST	14//充电检测
#define TPC_DPPORT_TST	15//显控端口检测
#define TPC_GYRO_TST	16//陀螺仪检测
#define TPC_MT_FAN		17//风机检测

#define TPC_CT_KEY		18//按键检测
#define TPC_CT_REMOTE		19//遥控检测
#define TPC_COM_DISPLAY		20//显控检测
#define TPC_IDLE	0xfa//检测空闲状态,当所有检测已完成，切换到这个状态，等待写入SN
#define TPC_SETSN	0xfb//写入SN码,长度为20字节的字符串,经过转义后才能存储
#define TPC_GETSN	0xfc//读出SN码
#define TPC_TRANS_PACK	0xfd//转由PC下发给工装的各通路控制字
						//para = 0 正常控制,t_res_output中每1位代表一个电机的动作(照着U形光耦的限位转一圈)
						//para = 1 清0磁编码盘的计数
						//t_res_output 设置各电机的工作状态(1为正在动作(正轮) 0为停机 2为反转),
						//			t_res_output 	bit15-14 前轮电机 
						//							bit13-12 红外测距挡板电机(1为低位光耦 2为高位光耦 0为静止)
						//							bit11-8 模拟碰撞电机1-4
						//							bit7-bit3 继电器输出1-5的状态
						//							bit2-bit0 地检电机1-3状态
						//上发给PC时使用的是TEST_PROC结构体 由t_res及t_ret_value组成uint16_t 位功能排列如t_res_output,为小端格式
						
#define TPC_HEART_BEEP	0xfe//心跳信号,每秒发一次
#endif
#define TPC_BUM_SW_L (TPC_BUM_SW+38)
#define TPC_BUM_SW_ML (TPC_BUM_SW+39)	//中左
#define TPC_BUM_SW_M (TPC_BUM_SW+40)
#define TPC_BUM_SW_MR (TPC_BUM_SW+41)	//中右
#define TPC_BUM_SW_R (TPC_BUM_SW+42)

#define IR_FRT		1



#define SWSENSOR_ULTRA	0
#define SWSENSOR_VW		1
#define SWSENSOR_LEV_L	2
#define SWSENSOR_LEV_R	3
#define SWSENSOR_DUSTFULL	4//尘满


#define NORM_SENSOR_BIN		0//尘盒开关
#define NORM_SENSOR_KEY_WIFI	1//WIFI按键
#define NORM_SENSOR_KEY_SPOT	2//集中清扫按键
#define NORM_SENSOR_KEY_START	3//开始/停止按键
#define NORM_SENSOR_KEY_DOCK	4//回充按键
#define NORM_SENSOR_REMOTE		5//遥控器
#define NORM_SENSOR_DISPLAY		6//显控


#define TPC_ERR_NOERROR	0//无错误
#define TPC_ERR_FORWARD	1//电机正转或转动错误
#define TPC_ERR_BACK	2//电机反转错误
#define TPC_ERR_DIGI_IN	3//开关量输入错误, 如:碰撞没反应
#define TPC_ERR_DIGI_OUT	4//开关量输出错误,如:红外LED不供电
#define TPC_ERR_COMM	5//通信错误
#define TPC_ERR_OC		6//电机过流检测错误
#define TPC_ERR_IRLED	7//红外接收错误
#define TPC_ERR_TB		8//测试板错误
#define TPC_ERR_BATNE	9//电池不存在
#define TPC_ERR_BATOT	10//电池过温
#define TPC_ERR_DOCK	11//充电座检测电路错误
#define TPC_ERR_DCJACK	12//DC座检测电路错误
#define TPC_ERR_BATCH	13//电池充电失败
#define TPC_ERR_TYRO	14//陀螺仪错误

//上位机与主板通信的结构体
typedef struct __test_mode_proc{
	unsigned char code;//控制指令码
	unsigned char t_progress;//检测进度,上位机下发0为开始检测,下位机回复到100为结束检测/或为心跳包的计数(循环)
	unsigned char t_res;//检测结果,见宏定义 TPC_ERR_XX
	unsigned char t_ret_value;//返回的附带参数,只有在红外等多路同时检测中使用,用于显示出错误的通道号(bit0-bit7代表通道0-7)
								// 如果是对地红外,bit0-2为近端的各路红外,bit3-5为远端的各路红外
								//如果是开关量,碰撞信号:bit0 1 2 3 4//左 左1 中 右1 右
								//回充 bit0 1 2 左右中
								//其他传感器: bit0-bit6分别为:(超声波/虚拟墙/左落轮/右落轮/集尘盒满)
	unsigned char chksum;//校验位,现在没有被使用
}TEST_PROC;

typedef struct __testbdio_uart{	
	uint16_t output_sta_1;//其他开关式传感器的信号模拟输出端口
	uint16_t output_sta_2;//高8为代表测试版电机状态1运动中 0到位,低8位为目标状态0:B 1:A, //A:01 B:00 ->b11 ->a10		
}TBDIO_UART;



//上位机与主板通信的心跳包结构体
typedef struct __test_mode_mac_beep_proc{
	unsigned char code;//控制指令码
	unsigned char t_progress;//检测进度,上位机下发0为开始检测,下位机回复到100为结束检测/或为心跳包的计数(循环)
	unsigned char t_res;//检测结果,见宏定义 TPC_ERR_XX
	unsigned char t_ret_value;//返回的附带参数,只有在红外等多路同时检测中使用,用于显示出错误的通道号(bit0-bit7代表通道0-7)
								// 如果是对地红外,bit0-2为近端的各路红外,bit3-5为远端的各路红外
								//如果是开关量,碰撞信号:bit0:左碰撞异常,bit1:右碰撞异常
								//			   其他传感器: bit0-bit5分别为:抹布/虚拟墙/尘盒/集尘盒满/左落轮/右落轮
	unsigned short ir_frt_value[10];//前撞红外值上传,后3个是对地红外的远光
	unsigned short bat_volt;//电池电压
	unsigned short bat_curr;//电池的充电电流
	unsigned short fw_ver;//机器软件版本号
	unsigned char bat_chstate;//电池的充电状态 0为未充电 1为DC插孔充电 2为充电桩充电
	unsigned char chksum;//校验位,现在没有被使用
}TEST_BEEP_PROC;


typedef struct __st_tmirmaxmin
{
	uint16_t min,max;
}tm_irmaxmin;
extern tm_irmaxmin irmaxmin[7];
extern tm_irmaxmin irtestdata[7];
extern tm_irmaxmin asse_irmaxmin[7];
extern tm_irmaxmin asse_irtestdata[7];
#define VALVE_0 0
#define VALVE_1 1//左前碰撞
#define VALVE_2 2//左碰撞
#define VALVE_3 3//滚刷,暂时不知道咋用，先路过
#define VALVE_4 4//中间碰撞
#define VALVE_5 5//右侧碰撞
#define VALVE_6 6//右前碰撞

#define VALVE_F_7 7//左跌落
#define VALVE_F_8 8//右跌落
#define VALVE_F_9 9//虚拟墙 超声波电磁铁

#define MOTER_0 0//左侧沿边红外电机
#define MOTER_1 1//车头红外挡板电机
#define MOTER_2 2//右侧沿边红外电机
#define MOTER_3 3
#define MOTER_4 4
#define MOTER_5 5//尾轮码盘电机


#define ERR_LEFT_BUM	0
#define ERR_MLEFT_BUM	1
#define ERR_MID_BUM		2
#define ERR_MRIGHT_BUM	3
#define ERR_RIGHT_BUM	4







#define BIT_STA(x)         ((uint16_t)(1<<(x)))
#define BIT8_STA(x)         ((uint8_t)(1<<(x)))

/*
//主板与测试板的通信结构体
//测试板需要对主板下发的指令进行返回,无论读取或者设置,设置需要返回设置后的结果,读取。。。肯定是要返回了。。。
//测试板回复主板时,t_res_output及t_res_input如果有值也需要带上来,这样可以省去再读一次的麻烦
typedef struct __test_board_proc{
	unsigned char code;//控制指令码
	unsigned char para;//控制动作 0停止/1开始(正转(前轮码盘电机))/2反转(前轮码盘电机)/3读取参数(码盘霍尔/红外LED亮度接收),返回值即t_res_output和t_res_input
						// 如果是测试过流,1表示将过流测试电阻接入 0表示过流电阻断开
						//如果是测试开关量(开漏模式),0表示放开,即此时如果有上拉电阻时应为高电平,1表示短拉到地,此时应为低电平
	unsigned short t_res_output;//输出返回值,如果当前动作有输出,则返回输出值,没有就空出,比如要求模拟左侧碰撞,则返回值应该为0,当左侧碰撞被置0时,此处的GPIO输出应该也是返回0,否则就是测试板错误
	unsigned short t_res_input;//输入读取返回,如红外LED是否亮,边刷/滚刷等电机的码盘值(最大到30000,防止数据溢出)
	unsigned short nc;
	unsigned char para1;//控制动作 用于多路开关量的同时输出,比如置位前撞的模拟开关,bit0表示左碰撞,1为接地,0为放开,bit1表示右碰撞
						// 如果用在多路开关式传感器测试中,bit0-bit5分别为:抹布/虚拟墙/尘盒/集尘盒满/左落轮/右落轮
						// 当para == 1时,这些开关的模拟触发有效
						// 当para == 0时,这些开关的模拟触发全部关闭,para1的值被忽略
						
	unsigned char chksum;//校验和,暂时没有使用
}TEST_BD_PROC;
*/
#if !USE_ON_COMPUTER

#if 0
//检测功能码,在检测光耦类传感器时,需要把红外LED的工作状态一起返回
enum __tbd_proc{
	TBD_STOP_ALL = 0,//退出检测模式
	TBD_START_ALL,//进入检测模式
	TBD_CD_FRT,//前轮码盘测试
	TBD_MTOC_WHL_L,//左轮过流检测
	TBD_MTOC_WHL_R,//右轮过流检测
	TBD_MTOC_SMT_L,//左边刷过流检测
	TBD_MTOC_SMT_R,//右边刷过流检测
	TBD_MTOC_MT_MID,//中扫电机过流检测
	TBD_MTOC_MT_FAN,//风机过流检测,(remark:风机在这里不做过流检测)
	TBD_MTFALL_L,//左轮落轮检测
	TBD_MTFALL_R,//右轮落轮检测
	TBD_BUM_L,//左前撞传感器检测
	TBD_BUM_R,//右前撞传感器检测
	TBD_CHRG_TST,//开始充电测试,需要把原来的充电通道断开,前接入到主板的供电通道,接收24V接入通道选择信号
}
#else
//检测功能码,在检测光耦类传感器时,需要把红外LED的工作状态一起返回
#define	TBD_STOP_ALL 	0//退出检测模式
#define	TBD_START_ALL	1//进入检测模式
#define	TBD_CD_FRT	2//前轮码盘测试
#define	TBD_MTOC_WHL_L	3//左轮过流检测
#define	TBD_MTOC_WHL_R	4//右轮过流检测
#define	TBD_MTOC_SMT_L	5//左边刷过流检测
#define	TBD_MTOC_SMT_R	6//右边刷过流检测
#define	TBD_MTOC_MT_MID	7//中扫电机过流检测
#define	TBD_MTOC_MT_FAN	8//风机过流检测,(remark:风机在这里不做过流检测所以此项不用)
#define	TBD_MTFALL_L	9//左轮落轮检测
#define	TBD_MTFALL_R	10//右轮落轮检测
#define	TBD_BUM_L	11//左前撞传感器检测
#define	TBD_BUM_R	12//右前撞传感器检测
#define	TBD_CHRG_TST	13//开始充电测试,需要把原来的充电通道断开,前接入到主板的供电通道,接收24V接入通道选择信号
						//para = 0 充电测试停止,把电池断开
						//para = 1 将电池接入主板,24V接入到DC插座
						//para = 2 将电池接入主板,24V接入到充电座触片
						//para = 3 将电池接入主板,24V未接入(这个选项一般不会被用到)
						//测试板端除了需要在output返回电池接入及24V的接入状况(bit0:电池/bit1:24V接入到DC座/bit2:24V接入到充电座触片)
						//			还需要返回电池的错误信息,比如电池未接入(未采到电池温度被认为电池未接入)
						
#define TBD_SMT_L	14//测试边刷电机是否在转,此时测试板需要开始记录左边刷的霍尔数,直到有命令读取
#define TBD_SMT_R	15//测试边刷电机是否在转,此时测试板需要开始记录右边刷的霍尔数,直到有命令读取
#define TBD_MT_MID	16//测试中扫电机是否在转,此时测试板需要开始记录右边刷的霍尔数,直到有命令读取
#define TBD_BUM_SW	17//左右碰撞开关模拟,此项中含有红外LED的供电检测,在t_res_input中返回,bit0为左,bit1为右,模拟碰撞的开关输出反馈在output中
#define TBD_SWSENSOR	18//其他开关型传感器的模拟输入,此项中含有红外LED的供电检测,在t_res_input中返回
						//其他传感器: bit0-bit5分别为:抹布/虚拟墙/尘盒/集尘盒满/左落轮/右落轮
#define TBD_FAN		19		//吸尘风机的电机控制
#define TBD_KEY		20		//按键控制
#define TBD_SETSN	0xfb//写入SN码,长度为20字节的字符串,经过转义后才能存储
#define TBD_GETSN	0xfc//读出SN码
#endif

//主板与测试板的通信结构体
//测试板需要对主板下发的指令进行返回,无论读取或者设置,设置需要返回设置后的结果,读取。。。肯定是要返回了。。。
//测试板回复主板时,t_res_output及t_res_input如果有值也需要带上来,这样可以省去再读一次的麻烦
typedef struct __test_board_proc{
	unsigned char code;//控制指令码
	unsigned char para;//控制动作 0停止/1开始(正转(前轮码盘电机))/2反转(前轮码盘电机)/3读取参数(码盘霍尔/红外LED亮度接收),返回值即t_res_output和t_res_input
						// 如果是测试过流,1表示将过流测试电阻接入 0表示过流电阻断开
						//如果是测试开关量(开漏模式),0表示放开,即此时如果有上拉电阻时应为高电平,1表示短拉到地,此时应为低电平
	unsigned short t_res_output;//输出返回值,如果当前动作有输出,则返回输出值,没有就空出,比如要求模拟左侧碰撞,则返回值应该为0,当左侧碰撞被置0时,此处的GPIO输出应该也是返回0,否则就是测试板错误
	unsigned short t_res_input;//输入读取返回,如红外LED是否亮,边刷/滚刷等电机的码盘值(最大到30000,防止数据溢出)
	unsigned char para1;//控制动作 用于多路开关量的同时输出,比如置位前撞的模拟开关,bit0表示左碰撞,1为接地,0为放开,bit1表示右碰撞
						// 如果用在多路开关式传感器测试中,bit0-bit5分别为:抹布/虚拟墙/尘盒/集尘盒满/左落轮/右落轮
						// 当para == 1时,这些开关的模拟触发有效
						// 当para == 0时,这些开关的模拟触发全部关闭,para1的值被忽略
						
	unsigned char chksum;//校验和,暂时没有使用
}TEST_BD_PROC;


#define SNLENGTH	21
typedef struct __test_sn_proc{
	unsigned char code;//控制指令码
	unsigned char sn[SNLENGTH];//20个字符长度的SN码，必须是可见字符,在传输过程中此数组中的SN字符为高低4位互换并异或之后再进行发送,FLASH中存储的SN也如此数组中的数据相同,只有需要打印日志或PC端显示时再进行还原
						//最后一个字符表示主板的测试结果,未知为0xff,0为通过,其他小于200的值表示测试不通过的次数
	unsigned char chksum;//校验和,SN的写入与读出必须要带入校验码，以防写错
}TEST_SN_PROC;

//机器老化测试使用的上报结构体
typedef struct __test_burnin_proc{
	unsigned char code;//控制指令码
	unsigned char para;//当前状态,左沿边还是右沿边
	unsigned short worktimer;//工作时间
	unsigned int bum_ct[4];//0-1左沿边碰撞计数 0为机械碰撞 1为红外碰撞 2-3为右沿边碰撞计数,以此类推
	unsigned short cpmt_health;//机器各部件的状态,有错误则对应位置1
						//bit15 左右边刷电机
						//bit14 中扫电机
						//bit13 风机
						//bit12 尘盒 
						//bit11 虚拟墙
						//bit10 前撞红外
						//bit9 对地红外
						//bit8 显控
						//bit7 充电
						//bit6 wifi模块
	unsigned short bat_volt;//电池电压
	unsigned short fw_ver;//系统软件版本号
	unsigned char errorcode;//错误代码
	unsigned char chksum;//校验和,暂时没有使用
}TEST_BURNIN_PROC;
#endif

#define TM_MODE_BOARD	0//主板单独测试模式
#define TM_MODE_MAC	1//整机测试模式
extern unsigned char tm_mode;

extern void tmod_rx_uart(unsigned char com_dp, unsigned char chr);
extern void proc_bdtest_task(void);
extern void proc_mactest_task(void);

extern void proc_burn_in_init(void);
extern void proc_burn_in_task(void);

extern void sn_print(unsigned char * pdat,unsigned short len);
extern void walk_tesk_for_whele(void);

extern unsigned short tmod_fancd;
extern unsigned short bdtest_chgcurr;
extern void tx_sn(unsigned char code ,unsigned char *sn);
extern unsigned char get_chksum(unsigned char *pdata, unsigned short length);
extern int burnin_coordinate_calcu(uint8_t type);
#define BDTST_SET_CURR(C)	bdtest_chgcurr=C

uint16_t get_mactest_sensors(void);
void set_remote_flag(uint8_t set);
#endif

