/****************************************Copyright (c)*************************
**                               版权所有 (C), 2015-2020, 涂鸦科技
**
**                                 http://www.tuya.com
**
**--------------文件信息-------------------------------------------------------
**文   件   名: protocol.h
**描        述: 下发/上报数据处理函数
**使 用 说 明 :

                  *******非常重要，一定要看哦！！！********

** 1、用户在此文件中实现数据下发/上报功能
** 2、DP的ID/TYPE及数据处理函数都需要用户按照实际定义实现
** 3、当开始某些宏定义后需要用户实现代码的函数内部有#err提示,完成函数后请删除该#err
**
**--------------当前版本修订---------------------------------------------------
** 版  本: v2.3.8
** 日　期: 2018年1月17日
** 描　述: 1:变量添加volatile防止编译器优化
           2:添加#error提示
		   
** 版  本: v2.3.7
** 日　期: 2017年4月18日
** 描　述: 1:优化串口队列接收处理
		   
** 版  本: v2.3.6
** 日　期: 2016年7月21日
** 描　述: 1:修复获取本地时间错误
           2:添加hex_to_bcd转换函数
		   
** 版  本: v2.3.5
** 日　期: 2016年6月3日
** 描　述: 1:修改返回协议版本为0x01
           2:固件升级数据偏移量修改为4字节

** 版  本: v2.3.4
** 日　期: 2016年5月26日
** 描　述: 1:优化串口解析函数
           2:优化编译器兼容性,取消enum类型定义

** 版  本: v2.3.3
** 日　期: 2016年5月24日
** 描　述: 1:修改mcu获取本地时间函数
           2:添加wifi功能测试

** 版  本: v2.3.2
** 日　期: 2016年4月23日
** 描　述: 1:优化串口数据解析
           2:优化MCU固件升级流程
           3:优化上报流程

** 版  本: v2.3.1
** 日　期: 2016年4月15日
** 描　述: 1:优化串口数据解析

** 版  本: v2.3
** 日　期: 2016年4月14日
** 描　述: 1:支持MCU固件在线升级

** 版  本: v2.2
** 日　期: 2016年4月11日
** 描　述: 1:修改串口数据接收方式

** 版  本: v2.1
** 日　期: 2016年4月8日
** 描　述: 1:加入某些编译器不支持函数指针兼容选项

** 版  本: v2.0
** 日　期: 2016年3月29日
** 描　述: 1:优化代码结构
           2:节省RAM空间
**
**-----------------------------------------------------------------------------
******************************************************************************/
#ifndef __PROTOCOL_H_
#define __PROTOCOL_H_

/******************************************************************************
                            用户相关信息配置
******************************************************************************/
/******************************************************************************
                            1:修改产品信息                
******************************************************************************/
#if MAP_VERSION
#if MAP_VERSION_V == 1
#define PRODUCT_KEY "HDCHze2wkUzxKr4I"    //"hBP3VvF9DMkXQ3FO"//    //开发平台创建产品后生成的16位字符产品唯一标识
#elif MAP_VERSION_V == 2
#define PRODUCT_KEY "SQlCb5OAF7c8ABgz"
#elif MAP_VERSION_V == 3
#define PRODUCT_KEY "lggtlhsevxavadx5"//"sq3iuotbitcz3gyk"//
#endif
#else
#define PRODUCT_KEY "hBP3VvF9DMkXQ3FO"
#endif
//配网方式选择,默认为CONFIG_MODE_DEFAULT,只能三选一
#define CONFIG_MODE     CONFIG_MODE_DEFAULT             //默认配网方式
//#define CONFIG_MODE     CONFIG_MODE_LOWPOWER            //低功耗配网方式
//#define CONFIG_MODE     CONFIG_MODE_SPECIAL             //特殊配网方式

#define MCU_VER "1.0.0"                                 //用户的软件版本,用于MCU固件升级,MCU升级版本需修改
/******************************************************************************
                          2:MCU是否需要支固件升级                  
如需要支持MCU固件升级,请开启该宏
MCU可调用mcu_api.c文件内的mcu_firm_update_query()函数获取当前MCU固件更新情况
                        ********WARNING!!!**********
当前接收缓冲区为关闭固件更新功能的大小,固件升级包为256字节
如需要开启该功能,串口接收缓冲区会变大
******************************************************************************/
//#define         SUPPORT_MCU_FIRM_UPDATE                 //开启MCU固件升级功能(默认关闭)
/******************************************************************************
                         3:定义收发缓存:
                    如当前使用MCU的RAM不够,可修改为24
******************************************************************************/
//#ifndef SUPPORT_MCU_FIRM_UPDATE
//#define WIFI_UART_QUEUE_LMT             16              //数据接收队列大小,如MCU的RAM不够,可缩小
//#define WIFI_UART_RECV_BUF_LMT          24              //根据用户DP数据大小量定,必须大于24
//#else
#define WIFI_UART_QUEUE_LMT             256             //数据接收队列大小,如MCU的RAM不够,可缩小
#define WIFI_UART_RECV_BUF_LMT          300             //固件升级缓冲区,需大缓存,必须大于260
//#endif

#define WIFIR_UART_SEND_BUF_LMT         256//24              //根据用户DP数据大小量定,必须大于24
/******************************************************************************
                        4:定义模块工作方式
模块自处理:
          wifi指示灯和wifi复位按钮接在wifi模块上(开启WIFI_CONTROL_SELF_MODE宏)
          并正确定义WF_STATE_KEY和WF_RESET_KEY
MCU自处理:
          wifi指示灯和wifi复位按钮接在MCU上(关闭WIFI_CONTROL_SELF_MODE宏)
          MCU在需要处理复位wifi的地方调用mcu_api.c文件内的mcu_reset_wifi()函数,并可调用mcu_get_reset_wifi_flag()函数返回复位wifi结果
          或调用设置wifi模式mcu_api.c文件内的mcu_set_wifi_mode(WIFI_CONFIG_E mode)函数,并可调用mcu_get_wifi_work_state()函数返回设置wifi结果
******************************************************************************/
//#define         WIFI_CONTROL_SELF_MODE                       //wifi自处理按键及LED指示灯;如为MCU外界按键/LED指示灯请关闭该宏
//#ifdef          WIFI_CONTROL_SELF_MODE                      //模块自处理
  #define     WF_STATE_KEY            14                    //wifi模块状态指示按键，请根据实际GPIO管脚设置
  #define     WF_RESERT_KEY           0                     //wifi模块重置按键，请根据实际GPIO管脚设置
//#endif

/******************************************************************************
                      5:MCU是否需要支持校时功能                     
如需要请开启该宏,并在Protocol.c文件内mcu_write_rtctime实现代码
mcu_write_rtctime内部有#err提示,完成函数后请删除该#err
mcu在wifi模块正确联网后可调用mcu_get_system_time()函数发起校时功能
******************************************************************************/
//#define         SUPPORT_MCU_RTC_CHECK                //开启校时功能

/******************************************************************************
                      6:MCU是否需要支持wifi功能测试                     
如需要请开启该宏,并且mcu在需要wifi功能测试处调用mcu_api.c文件内mcu_start_wifitest
并在protocol.c文件wifi_test_result函数内查看测试结果,
wifi_test_result内部有#err提示,完成函数后请删除该#err
******************************************************************************/
//#define         WIFI_TEST_ENABLE                //开启WIFI产测功能



#define WIFI_COOR_MAX_NUM	10
/******************************************************************************
                      9:是否支持流服务功能                 
******************************************************************************/
//#define         WIFI_STREAM_ENABLE              //支持流服务相关功能
//#ifdef WIFI_STREAM_ENABLE
#define         STREM_PACK_LEN                 256
//#endif

#define 		STREAM_MAX_PACK					10




/******************************************************************************
                        1:dp数据点序列号重新定义
          **此为自动生成代码,如在开发平台有相关修改请重新下载MCU_SDK**         
******************************************************************************/
//清扫开关(只下发)
#define DPID_START				1

//工作模式(可下发可上报)
//数值型
//standby：待机模式， chargego:回充模式，dry_rub:干擦,wet_wipe:湿擦
#define DPID_TYPE				2

//清扫面积(只上报)
//数值型
//需要兼容一位小数显示
#define DPID_CLEAN_AREA 			3

//清扫时间(只上报)
//数值型
//MCU上报本次清扫时间，实时更新
#define DPID_CLEAN_TIME			4

//总清扫面积(只上报)
//数值型
//需要兼容一位小数显示
#define DPID_TOTAL_AREA 			5

//总清扫时间(只上报)
//数值型
//MCU上报本次清扫时间，实时更新
#define DPID_TOTAL_TIME			6

//音量 0~5 (只上报)
#define DPID_VOICEMODULEVOLUME				7

//风机模式(可下发可上报)
//数值型
#define DPID_FAN					8

//喇叭音量(可下发可上报)
//数值型
#define DPID_SPEAKER				9

//虚拟墙坐标(可下发可上报)
//raw型
#define DPID_WALL				10

//地图范围(只下发)
//raw型
#define DPID_MAP					11

//清扫记录(只上报)
//raw型
#define DPID_COORDINATE			12

//定点清扫(可上报可下发)
//raw型
#define DPID_SPOT				13

//版本号(只上报)
//浮点型
#define DPID_VERSION				14

//沿边清扫开关(可下发可上报)
//bool型
#define DPID_EDGE_CLEANING		15

//吸尘灵敏度(可下发可上报)
//数值型
#define DPID_SENSITIVITY			16

//行走速度(可下发可上报)
//数值型
#define DPID_WHEEL				17

//充电模式(可下发可上报)
//数值型
#define DPID_CHARGE				18

//电量(只上报)
//数值型
#define DPID_BATTERY				19

//告警(只上报)
//数值型
#define DPID_WARNING			20

//方向(只下发)
//数值型
//"前进，后退，左转，右转，停止。
//默认逻辑：按住方向键下发 前/后/左转/右转，手指抬起下发停止"
#define DPID_DIRECTION_CONTROL 	21

//清扫完成(只上报)
//数值型
#define DPID_FINISH				22

//充电座坐标(只上报)
//raw型
#define DPID_CHARGESTATION		23

//暂停清扫(只下发)
//数值型
#define DPID_PAUSE_RESTART 		24

//寻找设备发声(只下发)
//数值型
#define DPID_FIND_SOUND			25

//返回充电座(只下发)
//数值型
#define DPID_RETURN_TO_BASE 		26

//地毯检测开关(只下发)
//数值型
#define DPID_FLOOR_SENSOR_STATUS 	27

//灯光状态(可下发)
//bool(开/关)
#define DPID_LIGHT_STATUS 	28

//double clean mode(可下发可上报)
//bool
#define DPID_DOUBLECLEAN_MODE 	29

//map status(只上报)
//bool
#define DPID_MAP_STATUS 	30

//robot position(只上报)
//int*2(int,int)
#define DPID_ROBOT_POSITION 	31

//floor Sensitivity Level(可下发可上报)
//int(0~5)
#define DPID_FLOORSENSITIVITY_LEVEL 	32

//floor Sensor Status(只下发)
//bool
#define DPID_FLOORSENSOR_STATUS 	33

//dirty Map(只上报)
//结构体
// X1(0~500), Y1(0~500), level(0~2)
#define DPID_DIRTYMAP 	34

//dirty Map(只上报)
//结构体
// (wd,h,m,powermode,enable)
#define DPID_SCHEDULE 	35

//strainer_clear



#define WIFICONFIG 			0
#define WIFI_STREAM_RECONNECTION_EN				0//1


#define WIFI_STREAM_STATE_IDEL					0     //wifi流状态
#define WIFI_STREAM_STATE_OPEN					1     //WiFi流打开
#define WIFI_STREAM_STATE_START					2     //WiFi流开始
#define WIFI_STREAM_STATE_STREAM				3
#define WIFI_STREAM_STATE_STREAMEN				4
#define WIFI_STREAM_STATE_OVER					5
//#define WIFI_STREAM_STATE_WAIT					6



#define WIFI_STREAM_STATE_UPDATA_ALL			9

#define STATE_NULL 		0
#define STATE_START 	1
#define STATE_SUSS	 	2
#define STATE_ERR	 	3


typedef struct h_spotattribute_t
{
	unsigned char x;
	unsigned char y;
	unsigned char spot_attrib;
}spotattribute_t;

typedef struct h_cleantimerecord_t
{
	unsigned int year;
	unsigned char month;
	unsigned char day;
	unsigned char hour;
	unsigned char min;
	//unsigned int natmr;		
}cleantimerecord_t;

typedef struct h_wifistreamdata_t
{
	unsigned char wifiStreamState;													//wifi流数据状态
	unsigned char send,write;		
	unsigned char sync_flg;			

	unsigned int pack_offset;														//上报包偏移量
	unsigned int record_navi_work_time;												//上报清扫时间	
	cleantimerecord_t clean_time_record;											//清扫时间
	spotattribute_t stream_cache[STREAM_MAX_PACK*20];								//200个缓存点
	unsigned char stream_work_start;												//流工作标志	
	unsigned char cleanover,lastpack;
	unsigned char repeat;
	unsigned char updata_dock_flag;
	unsigned char wifiStreamStart;
	unsigned char wifiAreaNoCalc;
	
}wifistreamdata_t;

extern wifistreamdata_t *pwifistream,wifistreamdata;		//流数据

#define POINT_CURRENT	0x00//0x00   当前点
#define POINT_OBSTACLE	0x01//0x01   障碍点
#define POINT_SCAN		0x02//0x02   清扫点
#define POINT_DOCK		0x03//0x03   充电座


typedef struct _t_wifi_sch_data{
		unsigned char weekday;
		unsigned char hour;
		unsigned char minute;
		unsigned char powermode;
		unsigned char en;
}T_WIFI_SCH_DATA;
#define SIZEOF_WIFISCHDATA	5	
#define SIZEOF_WIFISCHDATA_ARRAY	(SIZEOF_WIFISCHDATA * 7)	

/*****************************************************************************
函数名称 : all_data_update
功能描述 : 系统所有dp点信息上传
输入参数 : 无
返回参数 : 无
使用说明 : MCU必须实现该函数内数据上报功能
*****************************************************************************/
void all_data_update(void);


//add by jzz 2018-06-25
void wifi_updata_area_data(void);
void wifi_daemon_pro(unsigned char sta);
void init_stream_clean_id(void);
void updata_stream_cache_data(unsigned char tx,unsigned char ty,unsigned char state,unsigned char spot_attrib);


//add by jzz 2018-06-25 end


//#ifdef SUPPORT_MCU_RTC_CHECK
/*****************************************************************************
函数名称 : mcu_write_rtctime
功能描述 : MCU校对本地RTC时钟
输入参数 : 无
返回参数 : 无
使用说明 : MCU需要自行实现该功能
*****************************************************************************/
void mcu_write_rtctime(unsigned char time[]);
//#endif

//#ifdef WIFI_TEST_ENABLE
/*****************************************************************************
函数名称 : wifi_test_result
功能描述 : wifi功能测试反馈
输入参数 : result:wifi功能测试结果;0:失败/1:成功
           rssi:测试成功表示wifi信号强度/测试失败表示错误类型
返回参数 : 无
使用说明 : MCU需要自行实现该功能
*****************************************************************************/
void wifi_test_result(unsigned char result,unsigned char rssi);
//#endif

//#ifdef SUPPORT_MCU_FIRM_UPDATE
/*****************************************************************************
函数名称 : mcu_firm_update_handle
功能描述 : MCU进入固件升级模式
输入参数 : value:固件缓冲区
           position:当前数据包在于固件位置
           length:当前固件包长度(固件包长度为0时,表示固件包发送完成)
返回参数 : 无
使用说明 : MCU需要自行实现该功能
*****************************************************************************/
unsigned char mcu_firm_update_handle(const unsigned char value[],unsigned long position,unsigned short length);
//#endif

/*****************************************************************************
函数名称 : dp_download_handle
功能描述 : dp下发处理函数
输入参数 : dpid:DP序号
value:dp数据缓冲区地址
length:dp数据长度
返回参数 : 成功返回:SUCCESS/失败返回:ERRO
使用说明 : 该函数用户不能修改
*****************************************************************************/
unsigned char dp_download_handle(unsigned char dpid,const unsigned char value[], unsigned short length);
/*****************************************************************************
函数名称 : get_download_cmd_total
功能描述 : 获取所有dp命令总和
输入参数 : 无
返回参数 : 下发命令总和
使用说明 : 该函数用户不能修改
*****************************************************************************/
unsigned char get_download_cmd_total(void);

ErrorStatus update_coordinate(void);
ErrorStatus update_machmode(unsigned short mode);

ErrorStatus update_wheelspeed(void);

void wifi_sync_state_pro(uint8_t wifi_state);

void wifi_updata_clean_record(void);
////void update_dock_point(void);
extern void update_map_over(void);
//充电座坐标
extern ErrorStatus update_dock_coord(signed short coord_x,signed short coord_y);
//区域清扫 对角坐标
extern ErrorStatus update_spot_area(XY_T coord[]);
void wifi_set_apmode(void);
#endif

