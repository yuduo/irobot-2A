
#ifndef _TASK_RX_H_
#define _TASK_RX_H_

#define PID_PEC	100.0f

#define BEGIN_IMAGE				0xA5
#define MSG_IMU_RX				0x01
#define MSG_IMU_SET_XY			0x02
#define MSG_IMU_SET_YAW			0x03
#define MSG_TURN_DEG			0x04
#define MSG_TURN_TO_DEG			0x05
#define MSG_WALK_DIST			0x06		//直线行走
#define MSG_WORK				0x07		//开始工作，开电机，变量初始化
#define MSG_WALK_DEG			0x08		//修改机器行进中的角度，非行进状态无效
#define MSG_NEAR_WALL			0x09		//沿边
#define MSG_ZGO_CLEAN			0x0A		//工字清扫
#define MSG_ARC_TURN			0x0B		//弧线转弯
#define MSG_GO_STOP				0x0C		//停止走路
#define MSG_NAVI_POINT			0x0D		//导航的路基点
#define MSG_NEAR_WALK			0x0E		//沿边画地图时的直行
#define MSG_CLEAN_OK			0x0F		//清扫完成
#define MSG_WALK_POINT			0x10		//单点运行
#define MSG_GO_BUM				0x11		//激光碰撞消息
#define MSG_ZMAP_OBST			0x12		//下发地图障碍点坐标
#define MSG_DOOR				0x13		//接收门信息
#define MSG_WALK_PATH			0x14		//接收新路径
#define MSG_ROTA_WALK_PATH		0x15		//旋转路径点命令
#define MSG_NAVI_BORAD_INIT_OK	0x16		//激光板上电启动初始化OK
#define MSG_VIRTUAL_WALL		0x17		//接收虚拟墙
#define MSG_EMPTY_CLEAN_ZMAP	0x18		//接收清空清扫的地图和路径
#define MSG_ZGO_STOP			0x1A		//工字清扫停止
#define MSG_IMU_SET_XY_OFS		0x1B		//设置XY偏移量 XY_OFS
#define MSG_HEARTBEAT			0x1D		//导航板心跳包
#define MSG_GO_DIS_BUM			0x1E		//距离前方多少距离碰撞
#define MSG_APP_CMD				0x1C		//APP命令 透传信息
#define MSG_WIFI_STA			0x1F		//WIFI模块状态
#define MSG_APP_NAVI_POINT		0x20		//指哪扫(去)哪
#define MSG_APP_AREA_CLEAN		0x21		//划区清扫		划区清扫模式
#define MSG_APP_FORBIDDEN_AREA	0x22		//禁区设置
#define MSG_APP_VIRTUAL_WALL	0x23		//虚拟墙设置
#define MSG_APP_CONF_INFO		0x24		//获取所有配置
#define MSG_VW_BUM				0x25		//虚拟墙碰撞消息
#define MSG_ROOM_AREA_POINT		0x26		//划分房间区域清扫范围点,正常弓字清扫模式
#define MSG_LIDAR_DIST   		0x27  		//激光雷达的数据
#define MSG_FIRMWARE_UPGRADE	0x30		//固件升级
#define MSG_IMU_SET_LEAVE_DIS	0x40
#define MSG_WALK_OK				0x50		//直行动作完成
#define MSG_TURN_OK				0x51		//旋转OK
#define MSG_ROBOT_TURN   		0x52		// 1若机器在弓字清扫,停下来进入空闲状态,扫地图
#define WALK_RES_OK				0x00		//直行完成，没有碰撞
#define WALK_RES_BUM			0x01		//直行碰撞了
#define WALK_RES_IR				0x02		//直行红外碰撞了
#define WALK_RES_STOP			0x03		//激光导航要求停止
#define INFO_STOP				0x00
#define INFO_WALK				0x01
#define INFO_TURN				0x02
#define INFO_NAVI_OK			0x03		//导航完毕
#define INFO_NEAR				0x04		//延边
#define INFO_NAVI_POINT			0x05		//导航目标点
#define INFO_NAVI_BOARD_START_WORK		0x06		//底盘启动导航板开始工作 按键/app开始启动清扫
#define	INFO_HALT				0x07		//暂停
#define	INFO_OVER				0x08		//清扫完成
#define	INFO_HANG				0x09		//机器人悬空
#define	INFO_CLEAN_CONDITION	0x0A		//清扫情况信息
#define	INFO_CONSUMABLES		0x0B		//耗材信息
#define	INFO_ALARM				0x0C		//告警信息
#define INFO_NAVI_BUM			0x0D		//导航碰撞
#define INFO_DOOR_NAVI			0x0E		//导航到门那边去，重新扫地图
#define INFO_ZGO_STA			0x0F		//开始新的清扫,要求上位机扫地图
#define INFO_WIFI_CONFIG		0x10		//wywy 配网模式上报
#define INFO_DOCK_POINT			0x11		//充电桩坐标
#define INFO_DOCK_STA			0x12		//返回充电座模式
#define INFO_LINE_WALK			0x13		//弓字直线行走
#define INFO_NY3P_PLAY 			0x14		//语音播报
#define INFO_IMU_VER   			0x15		//IMU版本
#define EAI_DIST_OFFSET	8//距离数据在数组中的偏移量
#define EAI_DIST_BUFFERLENTH	64//0x40(&0x3f)
#define EAI_DIST_ANDHEX	0x3f//循环队列余数
struct imu_rx_data_t
{
	uint8_t		begin;
	uint8_t		crc;
	uint8_t		msg;		//命令码
	uint8_t		sta;
	uint8_t		ret;		//直行的反馈
	uint8_t		sSta;		//系统状态
	uint8_t		r;			//预留
	uint8_t		mSta;		//运动状态clean_sta 		清扫状态
	int16_t		x;
	int16_t		y;
	float		yaw;
};

typedef struct h_ctrl_t
{
	//uint8_t begin[2];
	uint8_t msg;
	uint8_t crc;
	uint16_t pwm;
	uint16_t hw;
	uint16_t speed;
}ctrl_t;

typedef struct h_pid_set_t
{
	uint8_t 	msg;
	uint8_t 	crc;
	uint16_t	kp1;
	uint16_t	ki1;
	uint16_t 	kd1;
	uint16_t	kp2;
	uint16_t	ki2;
	uint16_t 	kd2;	
}pid_set_t;
typedef struct h_rx_cfg_t
{
	uint8_t 	msg;
	uint8_t 	crc;
	uint16_t	cfg[40];	
}rx_cfg_t;
void rx_usart(uint8_t chr);

void tx_msg(uint8_t *buff,int len);
void test_seft_task(uint8_t type);

#endif
