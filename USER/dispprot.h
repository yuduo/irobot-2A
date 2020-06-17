#ifndef _DISPPORT_H_
#define _DISPPORT_H_
//串口通信协议

//接收缓存大小
#define DP_RXBUFFER_LEN	512

//键值宏
#define DP_KEY_PWR	TM_KEY_PWR//0x02
#define DP_KEY_MODE	TM_KEY_MODE//0x01
#define DP_KEY_PLAY	TM_KEY_PLAY//0x04


//#define PD_MSG_TIMER_SYNC		0x81
#define PD_MSG_WORK_STA		0x84
#define DP_MSG_BATY			0x85
#define DP_SYS_STA			0x83



#define ROBOT_STOP	0x00
#define ROBOT_WORK	0x01

#define FAN_MODE_LOW	0x01
#define FAN_MODE_MID	0x02
#define FAN_MODE_HIG	0x00
//命令集
//bit7=1为主板发送的命令
//bit7=0为显控发送的命令

//按键事件(显控=>主板)
#define DP_CMD_INVALID		0x00	//无效命令
#define DP_CMD_KEY_POWER	0x01	//开关机按键,带参为1字节,0表示此键按下后为关机,1表示开机
#define DP_CMD_KEY_MODE		0x02	//模式切换按键,带参为机器此时的工作模式,及选项,共2字节,见MachineWorkMode&MachineWorkOptions
#define DP_CMD_KEY_PLAY		0x03	//执行或停止按键,带参为3字节,byte0:0表示此键按下后为停止,1表示执行当前的模式
																//byte1:当前的工作模式,见MachineWorkMode
																//byte2:当前的工作模式,见MachineWorkOptions
#define DP_CMD_KEY_MANUAL	0x04	//手动清扫模式,带参为1字节,byte0:0/1/2/3分别表示前/后/左/右	
#define DP_CMD_KEY_SETTIME		0x06	//设置当前时间
#define DP_CMD_KEY_SETSCHEDULE	0x07	//设置预约时间,这里就用来放提示音,预约数据是存在显控板的,不用发到主控上
#define DP_CMD_VIOC			0x08	//静音
#define DP_CMD_KEY_DIDI		0x09	//让主板的喇叭叮一声,用于一些无指令按键的声音播放,提高用户体验	
#define DP_CMD_KEY_STATUS		0x0A	//显控下发摄像头或者其他一些板载设备的状态信息
#define DP_CMD_KEY_SELFTEST		0x0B	//触发整机自检命令

#define DP_CMD_KEY_WIFI		0x0C		//WIFI配网
#define DP_CMD_FAN_MODE		0x0D		//风力模式设置

//设置事件
#define DP_CMD_SET_TIME			0x81	//设置时间,带参3字节 0-2分别为 时(24小时格式)/分/星期(星期为位的格式,即bit0表周一,bit6表周日)
#define DP_CMD_SET_ORDERTIME	0x82	//设置预约时间,带参3字节,内容同DP_CMD_SET_TIME,星期位使能:如bit2=1表示周三需要清扫
#define DP_CMD_SET_STATUS		0x83	//发送机器信息(包括错误),带参2字节,byte0: 信息的内容,见MachineStatus
																		// byte1: 错误的内容,从1排到99,如果MachineStatus的error位为0,此参无效
#define DP_CMD_SET_MODE			0x84	//设置模式,带参2字节,见DP_CMD_KEY_MODE
#define DP_CMD_SET_BTRY			0x85	//发送电池电量,带参1字节,电量为0-100,单位为%


#define DP_PROTHEAD_1	0x7e
#define DP_PROTHEAD_2	0X5d

#define DP_PROTEND_2	0X6d
#define DP_PROTEND_1	0x7d
//通信数据的总长度
#define DP_PROT_COMMLENGTH	8
#define DP_PROT_DATALENGTH	5

#if 1			///显控协议2.0
////DISP_CMD_REMOTE--------------------遥控器相关
#define REMOTE_CODE_INVALID		0xff//无效码
#define REMOTE_CODE_POWER		0x01//电源
#define REMOTE_CODE_UP			0x02//向上键
#define REMOTE_CODE_LEFT		0x03//向左键
#define REMOTE_CODE_RIGHT		0x05//向右键
#define REMOTE_CODE_DOWN		0x06//向下键
//#define REMOTE_CODE_OK			0x30//OK
#define REMOTE_CODE_OK			0x04//OK
#define REMOTE_CODE_DOCK		0x07//回充
#define REMOTE_CODE_SCHDL		0x08//预约
#define REMOTE_CODE_FULL_GO		0x09//满电续扫
#define REMOTE_CODE_FOCUS		0x0A//重点清扫
#define REMOTE_CODE_TIM			0x0C//时间
//#define REMOTE_CODE_FORCE		0x1B//强力模式
#define REMOTE_CODE_LIGHTS		0xb0//氛围灯
#define REMOTE_CODE_MUTE		0x2B//静音
#define REMOTE_CODE_WIFI		0x40//WIFI
#define REMOTE_CODE_MODE		0x0B//模式选择
#define REMOTE_CODE_START		0x04//开始/停止
////DISP_CMD_ALL_LED_LIGHT--------------------显控led相关
//显控指示灯编号
#define LED_CONTRAL_NUB				12
#define DP_LED_ALL	0
#define DP_LED_1	1
#define DP_LED_2	2
#define DP_LED_3	3
#define DP_LED_4	4
#define DP_LED_5	5
#define DP_LED_6	6
#define DP_LED_7	7
#define DP_LED_8	8
#define DP_LED_9	9
#define DP_LED_10	10
#define DP_LED_11	11
//主板模式及状态对应指示灯
#define STAT_LED_ALL		DP_LED_ALL
#define MODE_LED_SILENT		DP_LED_1
#define MODE_LED_NORMAL		DP_LED_2
#define MODE_LED_TURBO		DP_LED_3
#define MODE_LED_MANUAL		DP_LED_4
#define MODE_LED_FULL2GO	DP_LED_5
#define STAT_LED_BTYPRS		DP_LED_6
#define STAT_LED_ERROR		DP_LED_7
#define STAT_LED_WIFI		DP_LED_8
#define STAT_LED_DUSTFULL	DP_LED_9
#define STAT_LED_888		DP_LED_10
#define STAT_LED_NA			DP_LED_11
//指示灯动作码
#define LED_OFF				0x00
#define LED_ON				0x01
#define LED_TW_0_5HZ		0x02
#define LED_TW_1HZ			0x03
#define LED_TW_2HZ			0x04
#define LED_TW_4HZ			0x05
#define LED_UNACTION		0x0F
#define LED_UNKNOW			0xFF
//指示灯动作时间
#define KEEP_TMR_0_S			0
#define KEEP_TMR_10_S			10
#define KEEP_TMR_120_S			120
#define KEEP_TMR_600_S			600
#define KEEP_TMR_30_MIN			1800
#define TMR_CHARGEIDLE_INTO_LOWPOWER	180000//60000	///180000

////888 错误码指示
#define DISP_ERR_UN		0x00
#define DISP_ERR_EN		0x01
//协议命令码对应包体长度
#define LEN_CMD_ALL_LED_LIGHT 		(LED_CONTRAL_NUB/2)	//发送所有灯的控制数据长度
#define LEN_CMD_SINGLE_LED_LIGHT	4
#define LEN_CMD_ERROR_DIS			5
#define LEN_CMD_SET_TIME			4
#define LEN_CMD_PAIR_REMOTE			1

//协议命令码
//	下发<-  上报->  双向<->
#define DISP_CMD_REMOTE				0X01		//遥控器			<-
#define DISP_CMD_ALL_LED_LIGHT		0X02		//所有灯的控制	->	-o
#define DISP_CMD_SCHEDULE_CLEAN		0X03		//预约清扫下发	<-	
#define DISP_CMD_SINGLE_LED_LIGHT	0X04		//单个灯控制		->	-o
#define DISP_CMD_ERROR_DIS			0X05		//错误显示		->	-o
#define DISP_CMD_SET_TIME			0X06		//设置时间		<->	-o
#define DISP_CMD_SET_SCHEDULE_TIME	0X07		//设置预约时间	<->
#define DISP_CMD_PAIR_REMOTE		0X08		//配对遥控器		<->
#define DISP_CMD_DPINFO				0x09	//主板<->显控 0为向显控设置关机,显控会拉低PA5作关机输出信号,0xff为主板从显控读取状态,此功能未加
#endif



//遥控器配对状态
#define	REMOTE_PAIR_NU		0	//非配对模式	->
#define	REMOTE_PAIR_ING 	1	//配对模式	<->
#define	REMOTE_PAIR_OK		2	//配对成功	<-

typedef struct __sch_data{
	uint8_t week_en;//使用的位为bit0-6 bit0为周日 bit1-bit6分别表示周一到周六
	uint8_t hour[7];
	uint8_t min[7];
}T_SCH_DATA;


/*
union __data_union
{
	DP_PROTTIME timedata;
	DP_PROTWMODE workmode;
	DP_PROTWSTATUS morkstatus;
	DP_POWERSTATUS powerstatus;
	DP_BATTERY battery;
	DP_WORK	   work;
	DP_OPINT	opt;
	DP_TIME_T	time;
};
*/


typedef struct __dp_protdata{
	uint8_t head1;
	uint8_t head2;//包头 0xaa,0x99
	uint8_t len;
	uint8_t chksum;//校验和,为此字节后的所有字节之和取低8位
	uint8_t cmd;//命令(不能与包头重码--0xaa 0x99)
	uint8_t data[20];
}DP_PROTDAT;

enum __dp_parseerrors{
	DP_PARSE_NOERR = 0,
	DP_PARSE_LENTH,//数据长度错误
	DP_PARSE_CHKSUM,//校验错误
	DP_PARSE_NODAT//无有效数据,即未找到包头
};

extern void dp_sendkey(uint8_t keycode,uint8_t pwr, uint8_t workmode, uint8_t workoptions);
extern void dp_commrxtask(void);

extern uint8_t dp_weekbit2dec(uint8_t weekbit);
//extern void dp_tx_data(uint8_t msg,uint8_t option1,uint8_t option2,uint8_t option3,uint8_t option4);
void dp_tx_data(uint8_t msg,uint8_t pack_len,uint8_t *data);
void dp_led_light_contrl(uint8_t type,uint8_t sSta,uint8_t work_mode,uint8_t change_sta,uint8_t prs,uint16_t work_errcode,uint8_t wifi_state);
void dp_led_err_contrl(uint8_t type,uint8_t work_sta,uint16_t work_errcode);
extern uint8_t dp_camera_error;

void dp_send_schdata(void);
void dp_set_schdata(uint8_t wday,uint8_t hour,uint8_t minute,uint8_t powermode,uint8_t en);
void dp_get_schdata_conv2wifi(uint8_t *wifi_sch);
#endif
