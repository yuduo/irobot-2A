/****************************************Copyright (c)*************************
**                               版权所有 (C), 2015-2020, 涂鸦科技
**
**                                 http://www.tuya.com
**
**--------------文件信息-------------------------------------------------------
**文   件   名: protocol.c
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

//#include "wifi.h"
#include "sys.h"

#include "rtc_lib.h"
#include "libatcharge.h"
#include "mcu_api.h"///

/******************************************************************************
                                移植须知:
1:MCU必须在while中直接调用mcu_api.c内的wifi_uart_service()函数
2:程序正常初始化完成后,建议不进行关串口中断,如必须关中断,关中断时间必须短,关中断会引起串口数据包丢失
3:请勿在中断/定时器中断内调用上报函数
******************************************************************************/


void wifi_sync_state_pro(uint8_t wifi_state);




/******************************************************************************
                              第一步:初始化
1:在需要使用到wifi相关文件的文件中include "wifi.h"
2:在MCU初始化中调用mcu_api.c文件中的wifi_protocol_init()函数
3:将MCU串口单字节发送函数填入protocol.c文件中uart_transmit_output函数内,并删除#error
4:在MCU串口接收函数中调用mcu_api.c文件内的uart_receive_input函数,并将接收到的字节作为参数传入
5:单片机进入while循环后调用mcu_api.c文件内的wifi_uart_service()函数
******************************************************************************/

/******************************************************************************
                        1:dp数据点序列类型对照表
          **此为自动生成代码,如在开发平台有相关修改请重新下载MCU_SDK**
******************************************************************************/
const DOWNLOAD_CMD_S download_cmd[] =
{
  {DPID_START, DP_TYPE_VALUE},
  {DPID_TYPE, DP_TYPE_VALUE},
  {DPID_CLEAN_AREA,DP_TYPE_VALUE},     //  清扫面积    (数值型) 20200520
  {DPID_CLEAN_TIME,DP_TYPE_VALUE},    //   清扫时间上报(数值型) 20200508
  {DPID_TOTAL_AREA,DP_TYPE_VALUE},    //   总清扫面积  (数值型) 20200520
  {DPID_TOTAL_TIME,DP_TYPE_VALUE},    //   总清扫时间  (数值型) 20200521

  {DPID_FAN, DP_TYPE_VALUE},
  {DPID_SPEAKER, DP_TYPE_VALUE},
  {DPID_WALL, DP_TYPE_RAW},
  {DPID_MAP, DP_TYPE_RAW},
  {DPID_EDGE_CLEANING, DP_TYPE_BOOL},
  {DPID_SENSITIVITY, DP_TYPE_VALUE},
  {DPID_WHEEL, DP_TYPE_VALUE},
  {DPID_CHARGE, DP_TYPE_VALUE},
  {DPID_DIRECTION_CONTROL, DP_TYPE_VALUE},
  {DPID_PAUSE_RESTART, DP_TYPE_VALUE},
  {DPID_FIND_SOUND, DP_TYPE_VALUE},
  {DPID_RETURN_TO_BASE, DP_TYPE_VALUE},
  {DPID_FLOOR_SENSOR_STATUS, DP_TYPE_VALUE},

  {DPID_LIGHT_STATUS, DP_TYPE_BOOL},
  {DPID_DOUBLECLEAN_MODE, DP_TYPE_BOOL},
  {DPID_FLOORSENSITIVITY_LEVEL, DP_TYPE_VALUE},
  {DPID_FLOORSENSOR_STATUS, DP_TYPE_BOOL},
  {DPID_SCHEDULE, DP_TYPE_RAW},
};

//地图参数
const unsigned char map_config[3] = {0x01, 0x00, 0xFF};

/******************************************************************************
                           2:串口单字节发送函数
请将MCU串口发送函数填入该函数内,并将接收到的数据作为参数传入串口发送函数
******************************************************************************/
wifistreamdata_t *pwifistream,wifistreamdata; 		//导航数据

uint16_t errcode_mapping[]={0,//MWERR_NONE
						  0,//请清洁集尘盒与吸尘口
						  0,//请检查中扫是否缠绕
						  0,//请检查集尘盒是否正确安装
						  0,//请检查左右轮是否离地
						  0x08,//清检查左右轮是否缠绕
						  0,//请检查地面感应器
						  0,//请检查边扫是否缠绕
						  0,//请检查前轮是否缠绕
						  0,//请检查电源开关是否打开
						  0,//摄像头错误
						  };

//static uint8_t wifi_workmode_data = 0;//APP面板下发的机器工作模式(自动/弓字型/随机/重点清扫/沿边清扫/回充)

/*****************************************************************************
函数名称 : uart_transmit_output
功能描述 : 发数据处理
输入参数 : value:串口收到字节数据
返回参数 : 无
使用说明 : 请将MCU串口发送函数填入该函数内,并将接收到的数据作为参数传入串口发送函数
*****************************************************************************/
void uart_transmit_output(unsigned char value)
{
  //#error "请将MCU串口发送函数填入该函数,并删除该行"
  put_usart3(value);
  //PRINTF("%02X ",value);

/*
  //示例:
  extern void Uart_PutChar(unsigned char value);
  Uart_PutChar(value);	                                //串口发送函数
*/
}
/**************** **************************************************************
                           第二步:实现具体用户函数
1:APP下发数据处理
2:数据上报处理
******************************************************************************/
////备注:00休眠、01待机、02工作中、03回充中、04充电完成、05座充中、06直充中、07故障
//系统工作状态，sys->sSta
//#define SYS_IDLE			0x00		//空闲
//#define SYS_NAVIGAT		0x01		//导航
//#define SYS_FOCUS		0x02		//重点
//#define SYS_RANDOM		0x03		//随机
//#define SYS_DOCK			0x04		//回充
//#define SYS_NEAR			0x05		//沿边
//#define SYS_CHARGE		0x06		//充电
//#define SYS_TEST_BD		0x07		//?÷°?2aê??￡ê?


struct tm m_time_now;

/******************************************************************************
                            1:所有数据上报处理
当前函数处理全部数据上报(包括可下发/可上报和只上报)
  需要用户按照实际情况实现:
  1:需要实现可下发/可上报数据点上报
  2:需要实现只上报数据点上报
此函数为MCU内部必须调用
用户也可调用此函数实现全部数据上报
******************************************************************************/

//自动化生成数据上报函数

/*****************************************************************************
函数名称 : all_data_update
功能描述 : 系统所有dp点信息上传,实现APP和muc数据同步
输入参数 : 无
返回参数 : 无
使用说明 : 此函数SDK内部需调用;
           MCU必须实现该函数内数据上报功能;包括只上报和可上报可下发型数据
*****************************************************************************/
//     { SYS_NAVIGAT 0x01, SYS_FOCUS 0x02,SYS_IDLE 0x04,SYS_IDLE 0x00,SYS_DOCK 0x20,0x08};
//const unsigned char mode_config[] = {  0x01,  0x02, 0x04, 0x00, 0x20, 0x08 } ;
//SYS_NAVIGAT= mode_config[1],
//= mode_config[2],
//SYS_IDLE= mode_config[3]，
//SYS_IDLE= mode_config[4]，
//SYS_DOCK= mode_config[5]，
void all_data_update(void)
{
//	uint8_t tempstate2;//,tempstate3,tempstate4;tempstate,
	static unsigned short errorcode = 0xffff;
////	static unsigned char versionupdate = 0xff;
	static unsigned char n_sstate = SYS_IDLE;
	static unsigned short n_wheelspeed = 0;
////    static unsigned long t_worktime = 0;
    static uint8_t n_onoffstatus = 0;
    static uint32_t n_minute1_tmr = 0;
    static uint8_t n_minute3_tmr = 0;

    static uint8_t n_bat_persent = 0;//电池电量
    static uint8_t n_dustmotor_power = DUST_MODE_NORMAL;//风机工作模式上报
    static uint8_t n_floorsensor_en = 0,n_dustsensor_en = 0,n_lightstatus_en = 0,n_voiceonoffstatus = 0;

	unsigned short n_worktime = 0;
	//log_printf("\r\n[Wi-Fi]all update!!\r\n");

	//if(mcu_get_wifi_work_state() != WIFI_CONN_CLOUD)return;
	if(mcu_get_wifi_work_state() != WIFI_CONN_READY)return;


	uint8_t n_nowonoffstatus;
	if(sys->sState == SYS_IDLE || sys->sState == SYS_CHARGE)
	{
		n_nowonoffstatus = 0;
		//LED2_OFF();
	}
	else
	{
		n_nowonoffstatus = 1;
	}
	if(mstimeout(&n_minute1_tmr,60000))
	{
		n_onoffstatus = n_nowonoffstatus;
		mcu_dp_value_update(DPID_START, n_nowonoffstatus);

        	//mcu_dp_value_update(DPID_WARNING, sys->work_errcode); //VALUE型数据上报;
		n_worktime = sys->t_navi_work/60;
		if(sys->m_worktime != n_worktime)
		{
			sys->m_worktime = n_worktime;
			//n_worktime ++;//= sys->t_navi_work;
			mcu_dp_value_update(DPID_CLEAN_TIME,n_worktime);	//VALUE型数据上报;	实时清扫时间
		}
		//mcu_dp_value_update(DPID_CLEAN_TIME,n_worktime);	//VALUE型数据上报;	实时清扫时间
	    mcu_dp_value_update(DPID_CLEAN_AREA,sys->m_area);  //实时面积上报
		log_printf("wifi_updata_tmr_data0:%d,%d,%d\r\n",sys->t_navi_work/60,sys->m_area,n_worktime);
		delay_ms_tm5(20);
		//mcu_dp_value_update(DPID_BATTERY, (unsigned long)ctrl->pwm);
		//delay_ms_tm5(10);

		if(n_minute3_tmr ++ > 2)
		{
			n_minute3_tmr = 0;
        }
        if(n_minute3_tmr == 1)
        {
		    //t_worktime = sys->t_navi_work;
	    	mcu_dp_value_update(DPID_TOTAL_AREA,sys->t_area);  //总面积上报 20200520
	    	mcu_dp_value_update(DPID_TOTAL_TIME,sys->t_clean); //总清扫时间上报
	        log_printf("wifi_updata_tmr_data1:%d,%d\r\n",sys->t_area,sys->t_clean);
			delay_ms_tm5(20);
        }

        uint8_t n_bat_prs;
		n_bat_prs = get_libat_percent(sys->volt);  //目前这里是测试的程序，还未上APP，就一个大致的数值

        if((n_bat_persent == 0)
        	|| ((sys->sState == SYS_CHARGE) && (n_bat_persent < n_bat_prs))
        	|| ((sys->sState != SYS_CHARGE) && (n_bat_persent > n_bat_prs)))
        {
        	n_bat_persent = n_bat_prs;
        }
		mcu_dp_value_update(DPID_BATTERY, (unsigned long)n_bat_persent);
		//log_printf("DPID_BATTERY=%d\r\n",n_bat_prs);
		//mcu_dp_value_update(DPID_BATTERY, (unsigned long)(TIM6->CNT%100));

	//	mcu_dp_value_update(DPID_CHARGE,1);
		delay_ms_tm5(20);

	}


	//有变化时的异步上报,
	if(n_nowonoffstatus != n_onoffstatus)
	{
		n_onoffstatus = n_nowonoffstatus;
		mcu_dp_value_update(DPID_START, n_nowonoffstatus);
	}

	#if 0
	//mcu_dp_float_update(DPID_VERSION, SYS_VERSION); //float型数据上报
	if(0/*versionupdate == 0xff*/)//系统联网成功后上报版本号      //20200529 音量 地毯等级 版本号 不能上报 犹大要求
	{
		versionupdate = 0;
		mcu_dp_float_update(DPID_VERSION, SYS_VERSION); //float型数据上报
		delay_ms_tm5(20);
	}
	#endif
	//上报错误代码
	if(errorcode == 0xffff)
	{
		errorcode = 0;
		mcu_dp_value_update(DPID_WARNING, errorcode); //VALUE型数据上报;
		delay_ms_tm5(20);
	}
	else if(sys->work_errcode != errorcode)
	{
		errorcode = sys->work_errcode;
		mcu_dp_value_update(DPID_WARNING, errorcode); //VALUE型数据上报;
		log_printf("DPID_WARNING:%d,%d\r\n",errorcode);
		delay_ms_tm5(20);
	}

	if(n_sstate != sys->sState)
	{
		n_sstate = sys->sState;

		mcu_dp_value_update(DPID_TYPE, sys->wifi_sta); //VALUE型数据上报;
		delay_ms_tm5(20);
	}

	if(n_wheelspeed != sys->run_speed)
	{
		n_wheelspeed = sys->run_speed;
		update_wheelspeed();
		mcu_dp_value_update(DPID_FAN, sys->dust_pwm_lev); //VALUE型数据上报;
		delay_ms_tm5(20);
	}

	//上报风机工作模式
	if(n_dustmotor_power != cfg->dust_pwm_lev)
	{
		uint8_t n_dustmotor_power_wifi;

		n_dustmotor_power = cfg->dust_pwm_lev;
		if(cfg->dust_pwm_lev == DUST_MODE_SILENT)
			n_dustmotor_power_wifi = 2;
		else if(cfg->dust_pwm_lev <= DUST_MODE_TURBO)
			n_dustmotor_power_wifi = cfg->dust_pwm_lev - 1;
  		mcu_dp_value_update(DPID_FAN,n_dustmotor_power_wifi);
		delay_ms_tm5(10);
	}

	cfg->r[IC_FLOORSENSOR_EN] = (cfg->r[IC_FLOORSENSOR_EN] != 0)?1:0;
	if(n_floorsensor_en != (uint8_t)(cfg->r[IC_FLOORSENSOR_EN]))
	{
		n_floorsensor_en = ((uint8_t)(cfg->r[IC_FLOORSENSOR_EN]));
		mcu_dp_value_update(DPID_FLOOR_SENSOR_STATUS,n_floorsensor_en);
		delay_ms_tm5(10);
	}

	cfg->r[IC_DUSTSENSOR_EN] = (cfg->r[IC_DUSTSENSOR_EN] != 0)?1:0;
	if(n_dustsensor_en!= ((uint8_t)(cfg->r[IC_DUSTSENSOR_EN])))
	{
		n_dustsensor_en = ((uint8_t)(cfg->r[IC_DUSTSENSOR_EN]));
		mcu_dp_value_update(DPID_SENSITIVITY,n_dustsensor_en);
		delay_ms_tm5(10);
	}

	if(n_lightstatus_en!= (cfg->r[IC_LED_EN] != 0))
	{
		n_lightstatus_en = ((uint8_t)(cfg->r[IC_LED_EN]) != 0);
		mcu_dp_value_update(DPID_LIGHT_STATUS,n_lightstatus_en);
		delay_ms_tm5(10);
	}

	if(n_voiceonoffstatus != (cfg->speaker_able != 0))
	{
		n_voiceonoffstatus = ((uint8_t)(cfg->speaker_able) != 0);
		mcu_dp_value_update(DPID_SPEAKER,(n_voiceonoffstatus == 0));
		delay_ms_tm5(10);
	}

	#if 0
	if((sys->t_navi_work - n_worktime) > 60)        //  60实时上报清扫时间  20200508
	{
		n_worktime = sys->t_navi_work;
		mcu_dp_value_update(DPID_CLEAN_TIME,n_worktime/60);	//VALUE型数据上报;	实时清扫时间
	    mcu_dp_value_update(DPID_CLEAN_AREA,sys->m_area);  //实时面积上报
		log_printf("wifi_updata_tmr_data0:%d,%d\r\n",sys->t_navi_work/60,sys->m_area);
		delay_ms_tm5(20);
	}
	if((sys->t_navi_work-t_worktime ) > 180)
	{
	    t_worktime = sys->t_navi_work;
    	mcu_dp_value_update(DPID_TOTAL_AREA,sys->t_area);  //总面积上报 20200520
    	mcu_dp_value_update(DPID_CLEAN_AREA,sys->t_clean); //总清扫时间上报
        log_printf("wifi_updata_tmr_data1:%d,%d\r\n",sys->t_area,sys->t_clean);

    }
    #endif
	//电池电量在sensor.c里上报
	///4剩余电量
	//mcu_dp_value_update(DPID_BATTERY, (unsigned long)sys->prs); //VALUE型数据上报;
	//mcu_dp_value_update(DPID_BATTERY, 53); //VALUE型数据上报;
	//delay_ms_tm5(100);

	///5寻找机器
}

void wifi_updata_area_data(void)
{
#if MAP_VERSION
////	//整理清扫点 计算面积
//	uint16_t tempx,tempy;
	uint32_t /*scancount=0,*/area=0;
	if(mcu_get_wifi_work_state() == WIFI_CONN_CLOUD)
	{
		if((sys->sState == SYS_NAVIGAT || sys->sState == SYS_FOCUS ) && sys->sState_auxi != 1)
		{
			if(pwifistream->wifiStreamState == WIFI_STREAM_STATE_STREAM)
			{
				if(pwifistream->wifiStreamStart == 1)
				{
					//if(sys->sState_auxi != 1)		//非回充状态上报面积
					{
						mcu_dp_value_update(DPID_CLEAN_AREA,area); //VALUE型数据上报;	实时清扫时间
						////log_printf("[bum]wifi_updata_area_data area:%d\r\n",area);
					}//其他状态上报时间
				}
			}
		}
	}
#endif
	return;
}

void update_map_over(void)
{
	log_printf("[update_map_over]OK!\r\n");
	updata_stream_cache_data(navigat->tx,navigat->ty,0,POINT_SCAN);
	pwifistream->cleanover = 1;
}

void wifi_updata_clean_record(void)
{
#if 1
	//整理清扫点 计算面积
	uint16_t tempx,tempy,templen;
	uint32_t scancount = 0,area = 0;
	char clean_record[25];
#if (0)
	if(pwifistream->wifiAreaNoCalc == 0)
	{
		for(tempx=navigat->min_tx-2;tempx<=(navigat->max_tx +3);tempx++)
		{
			for(tempy=navigat->min_ty-2;tempy<=(navigat->max_ty+3);tempy++)
			{
				if(SCANE_XY(tempx,tempy))	//已经清扫过的个数
					scancount++;			//45格为1平方
					////
				//else if(OBST_XY(tempx,tempy)) //障碍点也算清扫点
				//	scancount++;
					////
			}
		}	 //"20180411020080" 更改为020 080 2018 0411 0800  代表清扫了20分钟，80平方米，清扫日期2018年4月11日，清扫时间8:00

		//area = (scancount/45) + (scancount%45 > 18? 1:0);
		//清扫记录的麻烦你那面按照这个格式调整下2018 12 15 18 18 001 001 00001	分别代表年 月 日 时 分 清扫时间 清扫面积 记录id
		area = (scancount/43) + (scancount%43 > 16? 1:0);	//1.2的理论值
	}
	else
		area = 0;
#else
////	uint8_t sta ;
	if(pwifistream->wifiAreaNoCalc == 0)
	{
		/*for(tempx=0;tempx<=255;tempx++)
		{
			for(tempy=0;tempy<=255;tempy++)*/
		for(tempx=navigat->min_tx-2;tempx<=(navigat->max_tx +3);tempx++)
		{
			for(tempy=navigat->min_ty-2;tempy<=(navigat->max_ty+3);tempy++)
			{

				//if(get_map_sta(tempx,tempy) != MAP_STA_UNKN)	//已经清扫过的个数
					scancount++;			//45格为1平方
			}
		}
		////area = (scancount/43) + (scancount%43 > 16? 1:0);	//1.2的理论值
		unsigned int test_area = POINT_TO_AREA(scancount);
		area = test_area /10000 +  (test_area %10000) / 5000;
	}
	else
		area = 0;
	////area = POINT_TO_AREA(navigat->scan_num) /10000;
#endif
#if 0
	templen = sprintf(clean_record,"%04d%02d%02d%02d%02d%03d%03d%05d",
															//sys->t_navi_work_updata/60,area,
															pwifistream->clean_time_record.year,
															pwifistream->clean_time_record.month,
															pwifistream->clean_time_record.day,
															pwifistream->clean_time_record.hour,
															pwifistream->clean_time_record.min,
															sys->t_navi_work/60,area,cfg->steam_clean_id
															);
#else			///清扫记录的麻烦你那面按照这个格式调整下2018 12 15 18 18 001 001 00001  分别代表年 月 日 时 分 清扫时间 清扫面积 记录id
	templen = sprintf(clean_record,"%04d%02d%02d%02d%02d%03d%03d%05d",
															//sys->t_navi_work_updata/60,area,
															pwifistream->clean_time_record.year,
															pwifistream->clean_time_record.month,
															pwifistream->clean_time_record.day,
															pwifistream->clean_time_record.hour,
															pwifistream->clean_time_record.min,
															(msWifiTmr - pwifistream->record_navi_work_time)/60000,area,cfg->steam_clean_id
															);																														//sys->t_navi_work_updata/60,area,															pwifistream->clean_time_record.year,															pwifistream->clean_time_record.month,															pwifistream->clean_time_record.day, 														pwifistream->clean_time_record.hour,															pwifistream->clean_time_record.min, 														sys->t_navi_work/60,area,cfg->steam_clean_id															);

#endif//uint32_t tmr = ((msWifiTmr - pwifistream->record_navi_work_time)/1000)/60;
	log_printf("[wifi_updata_clean_record]clean_record:%s,area:%d,scancount:%d,point:%d\r\n",clean_record,area,scancount,scancount%45);
	//log_printf("clean_record:%s,area:%d,scancount:%d,point:%d\r\n",clean_record,area,scancount,scancount%30);
	////mcu_dp_value_update(DPID_CLEAN_AREA,area); //VALUE型数据上报;	实时清扫时间
	mcu_dp_raw_update(DPID_COORDINATE,(uint8_t *)clean_record,templen ); //STRING型数据上报;//当前清扫记录指针,当前清扫记录数据长度); //STRING型数据上报;
	//mcu_dp_raw_update(DPID_DIRTYMAP,(uint8_t *)clean_record,templen ); //脏地图上报
	log_printf("DPID_CLEAN_RECORD over\r\n");
	////mcu_dp_value_update(DPID_CLEAN_TIME,(msWifiTmr - pwifistream->record_navi_work_time)/60000); //VALUE型数据上报;	实时清扫时间		//9
#endif



}



//add by jzz 2018-06-25
void wifi_daemon_pro(unsigned char sta)
{
    static uint8_t area = 0;
	static uint32_t wifisync_timeout = 0;
	static uint32_t count=0;
	static uint32_t waittime = 0;
	int i=0;
	uint8_t send_en = 0 ;
	static uint8_t send_points = 0;
	static uint8_t /*stream_open_flag = 0,*/stream_start_flag = 0,stream_start_flag_no = 0, /*stream_startdelay_flag = 0,*/stream_stop_flag = 0;
	static uint32_t /*stream_open_count = 0,*/stream_start_count = 0,stream_startdelay_count = 0,stream_stop_count = 0;

	if(sta == 5)
		wifisync_timeout = msWifiTmr;
	if(sta == 1)
	{
		wifisync_timeout = msWifiTmr;
		memset((char *)pwifistream,0x00,sizeof(wifistreamdata));
		pwifistream->wifiStreamState = WIFI_STREAM_STATE_IDEL;
		/*stream_open_flag = 0,*/stream_start_flag = 0,/*stream_startdelay_flag = 0,*/stream_stop_flag = 0;
		/*stream_open_count = 0,*/stream_start_count = 0,stream_startdelay_count = 0,stream_stop_count = 0;
		log_printf("wifi_daemon_pro init 1 ok wifistate:%d, \r\n",mcu_get_wifi_work_state());
	}
	else if(sta == 2)
	{
		wifisync_timeout = msWifiTmr;
		pwifistream->wifiStreamState = WIFI_STREAM_STATE_IDEL;
		/*stream_open_flag = 0,*/stream_start_flag = 0,stream_stop_flag = 0;
		/*stream_open_count = 0,*/stream_start_count = 0,stream_stop_count = 0;
		mcu_dp_value_update(DPID_CLEAN_AREA,area); //VALUE型数据上报;	实时清扫时间
		log_printf("[bum]wifi_updata_area_data:area:%d\r\n",area);
		mcu_dp_value_update(DPID_CLEAN_TIME,0); //VALUE型数据上报;	实时清扫时间		//9
		log_printf("wifi_daemon_pro init 2 ok wifistate:%d, \r\n",mcu_get_wifi_work_state());
	}

	//log_printf("[wifi_daemon_pro]wifistate:%d\r\n",mcu_get_wifi_work_state());
	if(mcu_get_wifi_work_state() == WIFI_CONN_CLOUD)
	{

		if(mcu_get_wifi_work_state() == WIFI_CONN_CLOUD)
		{
			if ( pwifistream->sync_flg == 0)
			{
				pwifistream->sync_flg = 1;
				//log_printf("mcu_get_system_time\r\n");
				mcu_get_system_time();
			}
		}

		if((msWifiTmr - wifisync_timeout) > 1000*10 || sys->wifistatechange ==1) 	//6s同步一次数据
		{
			wifisync_timeout = msWifiTmr;
			sys->wifistatechange = 0;
			////log_printf("wifi_all_data_update,stream :%d\r\n",pwifistream->wifiStreamState);
			all_data_update();			///联网后上报
		}
#if MAP_VERSION
		wifi_uart_service();			////

		switch(pwifistream->wifiStreamState)
		{
			case WIFI_STREAM_STATE_IDEL:	//需要开启流数据传输功能
/*
				if(stream_open_flag == STATE_NULL)
				{
					stream_open_flag = STATE_START;
					stream_open_count = msWifiTmr + 3000;
					stream_open();		///1s
					log_printf("[wifi_stream]stream_open\r\n");
				}

				if(stream_status == 0)
				{
					//stream_open_flag = STATE_SUSS;		///成功
					pwifistream->wifiStreamState = WIFI_STREAM_STATE_OPEN;
					stream_open_flag = STATE_NULL;
					stream_start_flag = STATE_NULL;

					//stream_startdelay_flag = STATE_NULL;
					stream_startdelay_count = msWifiTmr + 5000;	//5s后才可以start
					log_printf("[wifi_stream]stream_open_ok\r\n");
				}
				if(msWifiTmr >= stream_open_count )
				{
					log_printf("[wifi_stream]stream_open_err 0X%02x\r\n",stream_status);
					stream_open_flag = STATE_NULL;
				}
*/
			break;

			case WIFI_STREAM_STATE_OPEN:
				if(pwifistream->stream_work_start == 1 || pwifistream->repeat == 1 || stream_stop_flag == STATE_ERR)
				{
					if(msWifiTmr > stream_startdelay_count)			//必须大于5S后才可以start
					{
						{
							if(stream_start_flag == STATE_NULL)
							{
								stream_start_flag = STATE_START;
								stream_start_count = msWifiTmr + 3000;
								stream_start();		///2
								log_printf("[wifi_stream]stream_start:SID:%d\r\n",cfg->steam_clean_id);
							}

							//	wifi_uart_service();			////
							if(stream_status == 0)
							{
								stream_start_flag = STATE_NULL;		///成功
								log_printf("[wifi_stream]stream_start_ok\r\n");
								if(stream_stop_flag == STATE_ERR)
									pwifistream->wifiStreamState = WIFI_STREAM_STATE_OVER;
								else
									pwifistream->wifiStreamState = WIFI_STREAM_STATE_START;
								//if(pwifistream->repeat == 1)
								//	pwifistream->repeat = 0;
								//stream_open_flag = STATE_NULL;
							}
							if( msWifiTmr >= stream_start_count )
							{
								if(stream_start_flag_no++ >3)
								{
									stream_start_flag_no = 0;
									stream_start_flag = STATE_NULL;		///失败
									//if()
									log_printf("[wifi_stream]stream_start_err 0X%02x\r\n",stream_status);
//									stream_open_flag = STATE_NULL;
									pwifistream->wifiStreamState = WIFI_STREAM_STATE_IDEL;
								}
								else
									stream_start_flag = STATE_NULL;		///失败
							}


						}
					}
				}
			break;

			case WIFI_STREAM_STATE_START:			//条件1:app启动
				//if(sys->sState == SYS_NAVIGAT || sys->sState == SYS_FOCUS || sys->sState == SYS_RANDOM)
				{
					pwifistream->wifiStreamState = WIFI_STREAM_STATE_STREAM; 	//开始流传输了
				}
			break;
			case WIFI_STREAM_STATE_STREAM:
				//------------启动清扫,若没有同步时间，同步一下服务器时间
				if(pwifistream->sync_flg == 0)
				{
					pwifistream->sync_flg = 1;
					//log_printf("mcu_get_system_time\r\n");
					mcu_get_system_time();
				}
				if(count++>5000)
				{
					count = 0;
					if(pwifistream->sync_flg != 2)
					{
						pwifistream->sync_flg = 1;
						//log_printf("mcu_get_system_time_again\r\n");
						mcu_get_system_time();
					}
				}
				///1,判断数据是否够了，够了更改pwifistream->wifiStreamState = WIFI_STREAM_STATE_STREAMEN;
				///2,最后一包
				///3,无最后一包，更改pwifistream->wifiStreamState = WIFI_STREAM_STATE_OVER;
				//if((pwifistream->send != pwifistream->write) )		//有可能有数据发送
				{
					if(pwifistream->write - pwifistream->send >=STREAM_MAX_PACK)
					{
						send_en = 1;
						send_points = STREAM_MAX_PACK;
					}
					else if(pwifistream->write < pwifistream->send)
					{
						send_en = 1;
						send_points = STREAM_MAX_PACK;
					}
					else if(pwifistream->cleanover == 1 /*|| sys->sState == SYS_IDLE || sys->sState == SYS_DOCK*/)
					{
						send_points = pwifistream->write - pwifistream->send ;
						if(send_points)
						{
							send_en = 1;
							pwifistream->lastpack =1;				//有最后一包
						}
						else
						{
							if(pwifistream->pack_offset !=0)
								pwifistream->pack_offset -= 1;			//无最后一包
							else
								pwifistream->pack_offset = 0;
							#if 1			///2018-11-26
								pwifistream->wifiStreamState = WIFI_STREAM_STATE_OVER;
							#else
								pwifistream->wifiStreamState = WIFI_STREAM_STATE_WAIT;
							#endif
							send_en = 0;
						}

					}
				}

				if(send_en)
				{
#if CURRENT_POINT_UNREPORT //不上报当前点 坐标属性
//#if 0
					send_en = 0;
					stream_trans(cfg->steam_clean_id,
								pwifistream->pack_offset,
								&pwifistream->stream_cache[pwifistream->send].x,3*send_points
								);
					if(pwifistream->repeat == 1)
					{
						pwifistream->repeat = 0;
						log_printf("SID:%d,repeat_stream_trans_pack_%d:\r\n",cfg->steam_clean_id,pwifistream->pack_offset);
					}
					log_printf("SID:%d,stream_trans_pack_%d:(%d,%d)\r\n",cfg->steam_clean_id,pwifistream->pack_offset,pwifistream->write,pwifistream->send);
#if 0
					log_printf("STREAM_TRANS:\r\n");
					for(i=0;i<send_points;i++)
						log_printf(":(%d,%d,%d)(%02x,%02x,%02x)\r\n",
											pwifistream->stream_cache[pwifistream->send+i].x,
											pwifistream->stream_cache[pwifistream->send+i].y,
											pwifistream->stream_cache[pwifistream->send+i].spot_attrib,
											pwifistream->stream_cache[pwifistream->send+i].x,
											pwifistream->stream_cache[pwifistream->send+i].y,
											pwifistream->stream_cache[pwifistream->send+i].spot_attrib
											);
#endif
					pwifistream->wifiStreamState = WIFI_STREAM_STATE_STREAMEN;
					//waittime = 0;
					waittime = msWifiTmr + 2500;			//启动计数
#else	//上报当前点属性
					send_en = 0;
					spotattribute_t stream_cache[11];
					memcpy(&stream_cache[0].x,&pwifistream->stream_cache[pwifistream->send].x,3*send_points);
					stream_cache[send_points].x = pwifistream->stream_cache[pwifistream->send+send_points-1].x;
					stream_cache[send_points].y = pwifistream->stream_cache[pwifistream->send+send_points-1].y;
					stream_cache[send_points].spot_attrib = POINT_CURRENT;
					stream_trans(cfg->steam_clean_id,
								pwifistream->pack_offset,							///3
								&stream_cache[0].x,3*(send_points+1)
								);

					if(pwifistream->repeat == 1)
					{
						pwifistream->repeat = 0;
						log_printf("SID:%d,repeat_stream_trans_pack_%d:\r\n",cfg->steam_clean_id,pwifistream->pack_offset);
					}
					log_printf("SID:%d,stream_trans_pack_%d:(%d,%d)\r\n",cfg->steam_clean_id,pwifistream->pack_offset,pwifistream->write,pwifistream->send);
#if 0
					log_printf("STREAM_TRANS:\r\n");
					//for(i=0;i<(send_points);i++)
					for(i=0;i<(send_points+1);i++)
						log_printf(":(%d,%d,%d)(%02x,%02x,%02x)\r\n",
											stream_cache[i].x,
											stream_cache[i].y,
											stream_cache[i].spot_attrib,
											stream_cache[i].x,
											stream_cache[i].y,
											stream_cache[i].spot_attrib
											);
#endif
					pwifistream->wifiStreamState = WIFI_STREAM_STATE_STREAMEN;
					//waittime = 0;
					waittime = msWifiTmr + 2500;			//启动计数
#endif
				}
			break;

			case WIFI_STREAM_STATE_STREAMEN:
			//判断发送成功
				//if(waittime == 0)
				{
				//	waittime = msWifiTmr + 2500;
				}
			//	else
				{
					/*
					if(msWifiTmr > waittime)
					{
						i=0;

						while(1)
						{
							wifi_uart_service();			////
							if(stream_status == 0)		//开始传输流数据
								break;
							delay_sensor(10);
							if(i++>20)
								break;
						}
						if(i>20)
						{
							log_printf("SID:%d,stream_trans_pack_%d: _err\r\n",cfg->steam_clean_id,pwifistream->pack_offset);
							pwifistream->repeat = 1;
#if WIFI_STREAM_RECONNECTION_EN//Reconnect
							pwifistream->wifiStreamState = WIFI_STREAM_STATE_OPEN;	//重新走流程
#else
							pwifistream->wifiStreamState = WIFI_STREAM_STATE_STREAM;
#endif
						}
						else
						{
							log_printf("SID:%d,stream_trans_pack_%d: _ok\r\n",cfg->steam_clean_id,pwifistream->pack_offset);

							if(pwifistream->lastpack != 1)
							{
								pwifistream->pack_offset++;
								pwifistream->send += send_points;
								if(pwifistream->send>= STREAM_MAX_PACK*20)
									pwifistream->send = 0;
								pwifistream->wifiStreamState = WIFI_STREAM_STATE_STREAM;

							}
							else
							{
								pwifistream->wifiStreamState = WIFI_STREAM_STATE_OVER;
							}
						}
					}
					*/
					//if(msWifiTmr > waittime)
					{
						i=0;

						//while(1)
						{
							//wifi_uart_service();			////
							if(stream_status == 0)		//传输数据OK
								i = 20;

							if(msWifiTmr > waittime)		//传输数据超时
								i = 30;

						}
						if(i==30)
						{
							log_printf("SID:%d,stream_trans_pack_%d: _err\r\n",cfg->steam_clean_id,pwifistream->pack_offset);
							pwifistream->repeat = 1;
#if WIFI_STREAM_RECONNECTION_EN//Reconnect
							pwifistream->wifiStreamState = WIFI_STREAM_STATE_OPEN;	//重新走流程
#else
							pwifistream->wifiStreamState = WIFI_STREAM_STATE_STREAM;
#endif
						}
						else if(i==20)
						{
							log_printf("SID:%d,stream_trans_pack_%d: _ok\r\n",cfg->steam_clean_id,pwifistream->pack_offset);

							if(pwifistream->lastpack != 1)
							{
								pwifistream->pack_offset++;
								pwifistream->send += send_points;
								if(pwifistream->send>= STREAM_MAX_PACK*20)
									pwifistream->send = 0;
								pwifistream->wifiStreamState = WIFI_STREAM_STATE_STREAM;

							}
							else
							{
								pwifistream->wifiStreamState = WIFI_STREAM_STATE_OVER;
							}
						}
					}
				}
			break;
			case WIFI_STREAM_STATE_OVER:			///结束流数据传输
				if(stream_stop_flag == STATE_NULL || stream_stop_flag == STATE_ERR)
				{
					log_printf("SID:%d,stream_stop,offset:%d\r\n",cfg->steam_clean_id,pwifistream->pack_offset);
					stream_stop(cfg->steam_clean_id,pwifistream->pack_offset);		///4
					stream_stop_flag = STATE_START;
					stream_stop_count = msWifiTmr +3000;
				}
					wifi_uart_service();		//有发现state一直为03
				if(stream_status == 0)
				{
					stream_stop_flag = STATE_SUSS;
					pwifistream->stream_work_start = 0;
					log_printf("[wifi_stream]stream_stop_ok\r\n");
					sys->wifistatechange = 1;
					sys->dis_statechange = 1;

					//stream_stop_flag = STATE_NULL;
				}
				if(msWifiTmr > stream_stop_count )
				{
					log_printf("[wifi_stream]stream_stop_err%d\r\n",stream_status);
#if WIFI_STREAM_RECONNECTION_EN//Reconnect
					stream_stop_flag = STATE_ERR;
					pwifistream->wifiStreamState = WIFI_STREAM_STATE_OPEN;
#else
					pwifistream->wifiStreamState = WIFI_STREAM_STATE_OVER;
#endif
					break;
				}
				sys->power_go = 0;
				//sys->wifistatechange = 1;
				if(stream_stop_flag == STATE_SUSS)
				{
					stream_stop_flag = STATE_NULL;

					wifi_updata_clean_record();
#if 0
					//整理清扫点 计算面积
					uint16_t tempx,tempy,templen;
					uint32_t scancount,area;
					char clean_record[20];
					for(tempx=navigat->min_tx-2;tempx<=(navigat->max_tx +3);tempx++)
					{
						for(tempy=navigat->min_ty-2;tempy<=(navigat->max_ty+3);tempy++)
						{
							if(SCANE_XY(tempx,tempy))	//已经清扫过的个数
								scancount++;			//45格为1平方
								////
							//else if(OBST_XY(tempx,tempy))	//障碍点也算清扫点
							//	scancount++;
								////
						}
					}	 //"20180411020080"	更改为020 080 2018 0411 0800  代表清扫了20分钟，80平方米，清扫日期2018年4月11日，清扫时间8:00
					//area = (scancount/45) + (scancount%45 > 18? 1:0);
					area = (scancount/43) + (scancount%43 > 16? 1:0);	//1.2的理论值

					templen = sprintf(clean_record,"%03d%03d%04d%02d%02d%02d%02d",
																			/*sys->t_navi_work/60,area,*/
																			sys->t_navi_work_updata/60,area,
																			pwifistream->clean_time_record.year,
																			pwifistream->clean_time_record.month,
																			pwifistream->clean_time_record.day,
																			pwifistream->clean_time_record.hour,
																			pwifistream->clean_time_record.min
																			);

					log_printf("clean_record:%s,area:%d,scancount:%d,point:%d\r\n",clean_record,area,scancount,scancount%45);
					//log_printf("clean_record:%s,area:%d,scancount:%d,point:%d\r\n",clean_record,area,scancount,scancount%30);
					mcu_dp_float_update(DPID_CLEAN_RECORD,(uint8_t *)clean_record,templen ); //STRING型数据上报;//当前清扫记录指针,当前清扫记录数据长度); //STRING型数据上报;
					log_printf("DPID_CLEAN_RECORD over\r\n");
#endif
					pwifistream->wifiStreamState = WIFI_STREAM_STATE_OPEN;		//在OPEN状态下就可以了
					pwifistream->send = 0;
					pwifistream->write = 0;
					pwifistream->pack_offset = 0;
					pwifistream->record_navi_work_time = 0;
					pwifistream->stream_work_start = 0;
					pwifistream->cleanover = 0;
					pwifistream->lastpack = 0;
					pwifistream->repeat = 0;
					pwifistream->wifiStreamStart = 0;
				}
			break;
			case WIFI_STREAM_STATE_UPDATA_ALL:								//忽略
				pwifistream->wifiStreamState = WIFI_STREAM_STATE_IDEL;
				//wifi_updata_all_map_data();
			break;
			default:
			break;

		}

#endif
	}
	else		//初始化流状态
	{

		wifisync_timeout = msWifiTmr;
#if WIFI_STREAM_RECONNECTION_EN//Reconnect
		pwifistream->wifiStreamState = WIFI_STREAM_STATE_IDEL;
#else
		if(pwifistream->stream_work_start)
		//	pwifistream->wifiStreamState = WIFI_STREAM_STATE_STREAM;		//保持断网前wifiStreamState的状态即可
#endif
		/*stream_open_flag = 0,*/stream_start_flag = 0,stream_stop_flag = 0;
		/*stream_open_count = 0,*/stream_start_count = 0,stream_startdelay_count = 0,stream_stop_count = 0;

	}
}

void init_stream_clean_id(void)
{
#if MAP_VERSION
//	int i=0;
	if ((/*mcu_get_wifi_work_state() == WIFI_CONNECTED ||*/mcu_get_wifi_work_state() == WIFI_CONN_CLOUD ))
	{	///必须网络状态下
		read_Flashbuf(CFG_ADDER,(u8 *)cfg_buf,MAX_CFG_LEN);
		cfg->steam_clean_id++;
		save_cfg(); 		//保证ID不会重复	只有启动清扫初始化这里获取一次

		//if(pwifistream->wifiStreamState == WIFI_STREAM_STATE_OPEN)		//open状态下
		{
			//pwifistream->wifiStreamState = 0;
			pwifistream->send = 0;
			pwifistream->write = 0;
			pwifistream->sync_flg = 0;									//需要同步一下开始清扫的时间
			pwifistream->pack_offset = 0;
			pwifistream->record_navi_work_time = msWifiTmr;
			pwifistream->stream_work_start = 1;

			pwifistream->cleanover = 0;
			pwifistream->lastpack = 0;
			pwifistream->repeat = 0;
			pwifistream->updata_dock_flag = 0;
			pwifistream->wifiStreamStart = 1;
			//pwifistream->wifiStreamStart = 1;
			if(sys->wifiAreaNoCalc_en == 1 )
			{
				sys->wifiAreaNoCalc_en = 0;
				pwifistream->wifiAreaNoCalc = 1;
			}
			else
				pwifistream->wifiAreaNoCalc = 0;
			//updata_stream_cache_data(100,100,0,POINT_DOCK);		//假设为充电座
			wifi_daemon_pro(2);

			//pwifistream->wifiStreamState = WIFI_STREAM_STATE_START;		//START状态
		}
	}
#endif
return;
}
void updata_stream_cache_data(unsigned char tx,unsigned char ty,unsigned char state,unsigned char spot_attrib)
{
	static uint8_t updata_state=0,update_en = 0;
//	uint16_t i;
	if(state ==1)
	{
		updata_state = 0;
		update_en = 0;
		log_printf("updata_stream_cache_data init\r\n");
		return;
	}
	#if 0
	if(sys->sState_auxi != 1 )		//非回充状态才可以丢数据进去
	{
		pwifistream->stream_cache[pwifistream->write].x= (uint8_t)(tx);
		pwifistream->stream_cache[pwifistream->write].y = (uint8_t)(ty);
		pwifistream->stream_cache[pwifistream->write++].spot_attrib= spot_attrib;//0x02;
		//pwifistream->stream_cache_count++;
		//log_printf("%d:(%d,%d,%d)\r\n",pwifistream->write,tx,ty,spot_attrib);
		if(pwifistream->write>=STREAM_MAX_PACK*20)
		{
	/*
			log_printf("==all cache data %d:\r\n",count++);
			for(i=0;i<STREAM_MAX_PACK*20;i++)
			{
				log_printf("(%02d,%02d,%02d)\r\n",pwifistream->stream_cache[i].x,pwifistream->stream_cache[i].y,pwifistream->stream_cache[i].spot_attrib);
			}
	*/
			pwifistream->write = 0;
		}
	}
	#endif
	if(updata_state == 0)
	{
		updata_state = 1;
		if(sys->sState_auxi != 1 )		//非回充状态才可以丢数据进去
		{
			update_en = 1;
		}
		else		//回充模式上报有两个条件
		{
			uint16_t tempx,tempy;
			uint32_t scancount=0;
			for(tempx=60;tempx<=140;tempx++)
			{
				for(tempy=60;tempy<=140;tempy++)
				{
					if(SCANE_XY(tempx,tempy))	//已经清扫过的个数
					{
						if(scancount++>200)			//45格为1平方
							break;
					}
				}
				if(scancount>200)			//45格为1平方
					break;
			}
			if(scancount<200)
			{
				log_printf("all date need updata\r\n");
				update_en = 1;
			}
			else
			{
				log_printf("all date need select scan updata\r\n");
				update_en = 2;
			}
		}
	}
	if(update_en == 1 )		//非回充状态才可以丢数据进去
	{
		pwifistream->stream_cache[pwifistream->write].x= (uint8_t)(tx);
		pwifistream->stream_cache[pwifistream->write].y = (uint8_t)(ty);
		log_printf("%d:(%02x,%02x,%02x)\r\n",pwifistream->write,tx,ty,spot_attrib);
		pwifistream->stream_cache[pwifistream->write++].spot_attrib= spot_attrib;//0x02;

		if(pwifistream->write>=STREAM_MAX_PACK*20)
		{
	/*
			log_printf("==all cache data %d:\r\n",count++);
			for(i=0;i<STREAM_MAX_PACK*20;i++)
			{
				log_printf("(%02d,%02d,%02d)\r\n",pwifistream->stream_cache[i].x,pwifistream->stream_cache[i].y,pwifistream->stream_cache[i].spot_attrib);
			}
	*/
			pwifistream->write = 0;
		}
	}
	else if(update_en == 2)
	{//当前点必须为清扫点才丢进来..
		//tx,ty,//是否为清扫点或者障碍,

		if(SCANE_XY(tx,ty))	//已经清扫过的个数
		{
			pwifistream->stream_cache[pwifistream->write].x= (uint8_t)(tx);
			pwifistream->stream_cache[pwifistream->write].y = (uint8_t)(ty);
			log_printf("select %d:(%d,%d,%d)\r\n",pwifistream->write,tx,ty,spot_attrib);
			pwifistream->stream_cache[pwifistream->write++].spot_attrib= spot_attrib;//0x02;
			//pwifistream->stream_cache_count++;

			if(pwifistream->write>=STREAM_MAX_PACK*20)
			{
		/*
				log_printf("==all cache data %d:\r\n",count++);
				for(i=0;i<STREAM_MAX_PACK*20;i++)
				{
					log_printf("(%02d,%02d,%02d)\r\n",pwifistream->stream_cache[i].x,pwifistream->stream_cache[i].y,pwifistream->stream_cache[i].spot_attrib);
				}
		*/
				pwifistream->write = 0;
			}
		}

	}
}

void updata_stream_data_map(unsigned char tx,unsigned char ty)
{
#if MAP_VERSION
//	uint8_t i;


#endif
return;
}

/*****************************************************************************
函数名称 : dp_download_power_go_handle
功能描述 : 针对DPID_START的处理函数
输入参数 : value:数据源数据
        : length:数据长度
返回参数 : 成功返回:SUCCESS/失败返回:ERROR
使用说明 : 可下发可上报类型,需要在处理完数据后上报处理结果至app
*****************************************************************************/
static ErrorStatus dp_download_power_go_handle(const unsigned char value[], unsigned short length)
{
	//示例:当前DP类型为BOOL
	ErrorStatus ret;
	//0:关/1:开
	unsigned long power_go;

	power_go = mcu_get_dp_download_value_littleindian(value,length);

	if(0 == power_go)
	{
		log_printf("[DPID_START]start off\r\n");
		//开关关
		if(sys->sState != SYS_DOCK && sys->sState != SYS_CHARGE)
		{
			if(sys->power_go == 1)
			{
				ny3p_play(VOICE_M_STOP);
			}
			sys->power_go = 0;
			init_sys_sta(SYS_IDLE);
			STOP_ALL_MOTOR();
			log_printf("[dp_download_power_go_handle]:OFF\r\n");
		}
	}
	else if(1 == power_go)
	{
		log_printf("[DPID_START]start on\r\n");
		if(sys->work_errcode == MWERR_NONE)
		{
			//开关开
			sys->power_go = 1;
			ny3p_play(VOICE_M_START);
			//init_sys_sta(SYS_NAVIGAT);
			log_printf("[dp_download_power_go_handle]:ON\r\n");

			if(sys->power_on != 0)
			{
				log_printf("[DPID_TYPE]Auto clean\r\n");
				if(EXTERAL_AC_DETECT())
				{
					log_printf("but now ac charge...\r\n");
				}
				else
				{
					if(sys->sState != SYS_NAVIGAT)
					{
						all_key_led_off();
						log_printf("key0 %d..\r\n",sys->sState);
						//LED1_ON();
						LED3_ON();
						init_sys_sta(SYS_NAVIGAT);
					}
					sys->full2go = 0;
					log_printf("[DPID_TYPE]Auto clean\r\n");
				}
			}
		}
		else
		{
			power_go = 0;
		}
	}

	//处理完DP数据后应有反馈
	ret = mcu_dp_value_update(DPID_START,power_go);

	return ret;
}


/*****************************************************************************
函数名称 : dp_download_mode_handle
功能描述 : 针对DPID_TYPE的处理函数
输入参数 : value:数据源数据
        : length:数据长度
返回参数 : 成功返回:SUCCESS/失败返回:ERROR
使用说明 : 可下发可上报类型,需要在处理完数据后上报处理结果至app
*****************************************************************************/
static ErrorStatus dp_download_mode_handle(const unsigned char value[], unsigned short length)
{
  //示例:当前DP类型为VALUE
  ErrorStatus ret;
  unsigned long mode;

  mode = mcu_get_dp_download_value_littleindian(value,length);

	if(1)//(1 == sys->power_go)
	{
	switch(mode)
	{
		case 0x1:

			log_printf("[DPID_TYPE]Auto clean\r\n");
			if(EXTERAL_AC_DETECT())
			{
				log_printf("but now ac charge...\r\n");
				break;
			}
			if(sys->sState != SYS_NAVIGAT)
			{
				all_key_led_off();
				log_printf("key0 %d..\r\n",sys->sState);
				//LED1_ON();
				LED3_ON();
				init_sys_sta(SYS_NAVIGAT);
			}
			sys->full2go = 0;
			log_printf("[DPID_TYPE]Auto clean\r\n");
		break;
		case 0x02:
			log_printf("[DPID_TYPE]Spot clean\r\n");
			all_key_led_off();
			if(sys->sState == SYS_IDLE || sys->sState == SYS_CHARGE)
				ny3p_play(VOICE_DIDI);
			else
				ny3p_play(VOICE_BEE_SHORT);
			dp_led_light_contrl(0,sys->sState,0,sys->charge_sta,sys->prs,sys->work_errcode,sys->wifi_state);

		   sys->t_beep_on = 0;
		   if(sys->sState == SYS_CHARGE)   //充电黑屏模式下,强制屏幕开机亮起,充电下没有关机状态
		   {
			   dp_led_light_contrl(0,sys->sState,0,sys->charge_sta,sys->prs,sys->work_errcode,sys->wifi_state);
		   }
		   LED3_OFF();
		   LED4_OFF();
		   log_printf("--------------------------\r\n");
		  // sys->power_on = 1;
		   if (sys->spot_flag++ >=1)
		   {
			   sys->spot_flag=0;
			   log_printf("key spot_flag 11 %d,%d..\r\n",sys->sState,sys->spot_flag);
		   }
		   if(1 == sys->spot_flag)
		   {
			   log_printf("key spot_flag 12 %d,%d..\r\n",sys->sState,sys->spot_flag);
			   // init_focus_task();
			   // sys->sState  = SYS_FOCUS;
			   //sys->sState_auxi = 0;
			   //proc_focus_task();
			   // sys->power_go = 1;
			   //ny3p_play(VOICE_M_CLN_HEART);

			   //init_focus_task();
			   //s->sState = SYS_FOCUS;

			   //init_sys_status(SYS_FOCUS,SYS_IDLE);
			   //init_sys_status(SYS_FOCUS,sys->sState);
			   if(sys->sState == SYS_NAVIGAT)
			   {
				   log_printf("navi stop...\r\n");
				   motor_run(GO_STOP,0,0,0);
				   delay_ms(200);
			   }
			   if(sys->sState == SYS_CHARGE)//充电状态不允许集中清扫
			   {
				   log_printf("charge...out\r\n");
				   break;
			   }
				//LED3_ON();
			   LED1_ON();
			   sys->gey_key_clear_err = 1;
			   sys->sState = SYS_FOCUS;
			   sys->f_work_mode = MWO_SPOT;
			   log_printf("set f_work_mode:%d\r\n",sys->f_work_mode);
			   log_printf("key12  focus  %d..\r\n",sys->sState);
			   //if(sys->sState != SYS_IDLE)
			   //sys->sState = SYS_FOCUS;
			   break;
		   }
		   if(0 == sys->spot_flag)
		   {
		  // if (sys->sState  != SYS_FOCUS)
				{
				 //LED3_OFF();
				   LED1_OFF();
				   sys->sState	= SYS_IDLE;
				   log_printf("key spot_flag 13 %d,%d..\r\n",sys->sState,sys->spot_flag);
				   if(sys->full2go)
					   sys->f_work_mode = MWO_FULL2GO;
				   else
					   sys->f_work_mode = sys->work_mod;	   //集中清扫模式 按键恢复
				   log_printf("set f_work_mode:%d\r\n",sys->f_work_mode);
				}
		   }

		break;
		case 0x04:
		case 0x00:
			//if(SYS_CHARGE != sys->sState)
			all_key_led_off();
			init_sys_sta(SYS_IDLE);
			sys->full2go = 0;
			log_printf("[DPID_TYPE]standby&pause\r\n");
		break;
		case 0x20:

			log_printf("[DPID_TYPE]return to charging base\r\n");
			if(EXTERAL_AC_DETECT())
			{
				log_printf("but now ac charge...\r\n");
				break;
			}


			log_printf("_DOCK\r\n");
			if(sys->sState == SYS_IDLE || sys->sState == SYS_CHARGE)
				ny3p_play(VOICE_DIDI);
			else
				ny3p_play(VOICE_BEE_SHORT);
			all_key_led_off();
			//dp_led_light_contrl(0,sys->sState,0,sys->charge_sta,sys->prs,sys->work_errcode,sys->wifi_state);
			sys->t_beep_on = 0;
			if(sys->sState == SYS_CHARGE)	//充电黑屏模式下,强制屏幕开机亮起,充电下没有关机状态
			{
				dp_led_light_contrl(0,sys->sState,0,sys->charge_sta,sys->prs,sys->work_errcode,sys->wifi_state);
			}
			/*
			if(sys->sState == SYS_NAVIGAT)
			{
				log_printf("clean now,please stop fisrt\r\n");
				break;
			}
			*/
			if(sys->sState == SYS_DOCK || sys->sState == SYS_DOCK_NEAR)
			{
				log_printf("I'm in charge...\r\n");
				break;
			}else if(sys->sState == SYS_CHARGE)
			{
				log_printf("key charge2,%d..\r\n",sys->sState);
				break;
			}else
			{
				all_key_led_off();
				// LED2_ON();
				  LED4_ON();
				  sys->gey_key_clear_err = 1;
				 log_printf("key charge3,%d..\r\n",sys->sState);
				init_sys_sta(SYS_DOCK);

			}
		break;
		case 0x08:
			//sys->full2go = 1;
			log_printf("[DPID_TYPE]full&go\r\n");
			if(sys->sState == SYS_IDLE || sys->sState == SYS_CHARGE)
				ny3p_play(VOICE_DIDI);
			else
				ny3p_play(VOICE_BEE_SHORT);
			dp_led_light_contrl(0,sys->sState,0,sys->charge_sta,sys->prs,sys->work_errcode,sys->wifi_state);

			if(sys->full2go == 0)
			{
				log_printf("full and go\r\n");
				sys->full2go = 1;
				sys->f_work_mode = MWO_FULL2GO;
			//增加只有在充电和清扫模式下才响应full2go按键,1,充电使能充满开始清扫,2,清扫状态直接进入回充状态.
				if(sys->sState == SYS_CHARGE)
				{
					log_printf("charge now,wait charge done\r\n");
					break;
				}
				// LED2_ON();
				if((sys->sState != SYS_DOCK))
				{
					all_key_led_off();
					LED4_ON();
					log_printf("key charge3,%d..\r\n",sys->sState);
					init_sys_sta(SYS_DOCK);
				}
			}
			/*else
			{
				log_printf("no full and go\r\n");
				sys->full2go = 0;
				sys->f_work_mode = sys->work_mod;		//full2go遥控器恢复
				break;
			}*/
		break;
		default:
			log_printf("[DPID_TYPE]DPID_TYPE error type value %d\r\n",mode);
		break;
	}
	}
  //处理完DP数据后应有反馈
   mcu_dp_value_update(DPID_TYPE,mode);
  return ret;
}
/*****************************************************************************
函数名称 : dp_download_fan_handle
功能描述 : 针对DPID_FAN的处理函数
输入参数 : value:数据源数据
        : length:数据长度
返回参数 : 成功返回:SUCCESS/失败返回:ERROR
使用说明 : 可下发可上报类型,需要在处理完数据后上报处理结果至app
*****************************************************************************/
static ErrorStatus dp_download_fan_handle(const unsigned char value[], unsigned short length)
{
  ErrorStatus ret;
  unsigned long fan_value = 0xFF;
  fan_value = mcu_get_dp_download_value_littleindian(value,length);
	if(sys->sState == SYS_IDLE || sys->sState == SYS_CHARGE)
		ny3p_play(VOICE_DIDI);
	else
		ny3p_play(VOICE_BEE_SHORT);
	switch(fan_value)
	{
		case 0:
			sys->f_work_mode = sys->work_mod = MWO_NORMAL;
		break;
		case 1:
			sys->f_work_mode = sys->work_mod = MWO_TURBO;
		break;
		case 2:
			sys->f_work_mode = sys->work_mod = MWO_SILENT;
		break;
		default:
			log_printf("[Wi-Fi DP]DPID_FAN error type value %d\r\n",fan_value);
		break;
	}
	dp_led_light_contrl(0,sys->sState,0,sys->charge_sta,sys->prs,sys->work_errcode,sys->wifi_state);
  ret = mcu_dp_value_update(DPID_FAN,fan_value);
  return ret;
}
/*****************************************************************************
函数名称 : dp_download_fan_handle
功能描述 : 针对DPID_SPEAKER的处理函数
输入参数 : value:数据源数据
        : length:数据长度
返回参数 : 成功返回:SUCCESS/失败返回:ERROR
使用说明 : 可下发可上报类型,需要在处理完数据后上报处理结果至app
*****************************************************************************/
static ErrorStatus dp_download_speaker_handle(const unsigned char value[], unsigned short length)
{
  ErrorStatus ret;
  unsigned long speaker_value = 0xFF;
  speaker_value = mcu_get_dp_download_value_littleindian(value,length);
	if(sys->sState == SYS_IDLE || sys->sState == SYS_CHARGE)
		ny3p_play(VOICE_DIDI);
	else
		ny3p_play(VOICE_BEE_SHORT);
	switch(speaker_value)
	{
		case 1:
			if(0 != cfg->speaker_able)
			{
				cfg->speaker_able = 0;
				save_cfg();
				log_printf("speaker mute\r\n");
			}
		break;
		case 0:
			if(1 != cfg->speaker_able)
			{
				cfg->speaker_able = 1;
				save_cfg();
				log_printf("speaker beep\r\n");
			}
		break;
		default:
			log_printf("[Wi-Fi DP]DPID_SPEAKER error type value %d\r\n",speaker_value);
		break;
	}
  ret = mcu_dp_value_update(DPID_SPEAKER,speaker_value);
  return ret;
}

/*****************************************************************************
函数名称 : dp_download_directioncontrol_handle
功能描述 : 针对DPID_DIRECTION_CONTROL的处理函数
输入参数 : value:数据源数据
        : length:数据长度
返回参数 : 成功返回:SUCCESS/失败返回:ERROR
使用说明 : 只下发类型,需要在处理完数据后上报处理结果至app
*****************************************************************************/
static ErrorStatus dp_download_directioncontrol_handle(const unsigned char value[], unsigned short length)
{
  //示例:当前DP类型为VALUE
  ErrorStatus ret;
  unsigned long directioncontrol;

  /*
  forward:0x00
  backward:0x01
  turnleft:0x02
  turnright:0x03
  stop:0x04
  */
  directioncontrol = mcu_get_dp_download_value_littleindian(value,length);

	  switch(directioncontrol)
	  {
		case 1:		//前进
		case 2:		//后退
		case 3:		//左转
		case 4:		//右转
			log_printf("manual mode:%d\r\n",directioncontrol);
			if(sys->work_errcode != MWERR_NONE)
			{
				log_printf("have err!\r\n");
				break;
			}
			if(sys->sState != SYS_IDLE || //只有在空闲模式下
				(sys->sState == SYS_CHARGE && ((sys->charge_sta & AC_CHARGE) != 0)))//或者在充电座上才做能方向盘控制
			{
				log_printf("I'm busy now!\r\n");
				break;
			}
			if(sys->prs<3)
			{
				log_printf("low power!\r\n");
				break;
			}
			sys->wifistatechange = 1;
			sys->dis_statechange = 1;
			all_key_led_off();
			sys->f_work_mode = MWO_MANUAL;
			////wifi_motor_single_run(directioncontrol+1);
			wifi_motor_single_run(directioncontrol);
			log_printf("[dp_download_directioncontrol_handle]:%d\r\n",directioncontrol);
			break;
		case 0:		//停止
			log_printf("manual mode:%d\r\n",directioncontrol);
			if(sys->sState != SYS_IDLE || //只有在空闲模式下
				(sys->sState == SYS_CHARGE && ((sys->charge_sta & AC_CHARGE) != 0)))//或者在充电座上才做能方向盘控制
			{
				log_printf("I'm busy now!\r\n");
				break;
			}
			sys->wifistatechange = 1;
			sys->dis_statechange = 1;
			wifi_motor_single_run(GO_STOP);
			sys->f_work_mode = 0;
		  	log_printf("[dp_download_directioncontrol_handle]:stop\r\n");
		  break;
		default:
		  break;
	  }

	if(5 > directioncontrol)
	{
  		log_printf("[Wi-Fi DP]directioncontrol:%d\r\n",directioncontrol);
	}
	else
	{
  		log_printf("[Wi-Fi DP]dir error:%d\r\n",directioncontrol);
	}

  //处理完DP数据后应有反馈
  ret = mcu_dp_value_update(DPID_DIRECTION_CONTROL,directioncontrol);
  return ret;
}

/*****************************************************************************
函数名称 : dp_download_findsound_handle
功能描述 : 针对DPID_SEEK的处理函数
输入参数 : value:数据源数据
        : length:数据长度
返回参数 : 成功返回:SUCCESS/失败返回:ERROR
使用说明 : 可下发可上报类型,需要在处理完数据后上报处理结果至app
*****************************************************************************/
static ErrorStatus dp_download_findsound_handle(const unsigned char value[], unsigned short length)
{
	//示例:当前DP类型为VALUE
	ErrorStatus ret = SUCCESS;
	//0:关/1:开
	unsigned long sound_able = 0xFF;

  	sound_able = mcu_get_dp_download_value_littleindian(value, length);
  	if(1 == sound_able)
  	{
  		if((1 == cfg->speaker_able) && (0 == sys->t_beep_on))
			sys->t_beep_on = 10000;                             //叫10秒   20200529
		log_printf("\r\nxun zhao jiao\r\n");
  	}

	if(SUCCESS == ret)
	{
		//处理完DP数据后应有反馈
		ret = mcu_dp_value_update(DPID_FIND_SOUND, sound_able);
	}

	return ret;
}

/*****************************************************************************
函数名称 : dp_download_edge_clean_handle
功能描述 : 针对DPID_EDGE_CLEANING的处理函数
输入参数 : value:数据源数据
        : length:数据长度
返回参数 : 成功返回:SUCCESS/失败返回:ERROR
使用说明 : 可下发可上报类型,需要在处理完数据后上报处理结果至app
*****************************************************************************/
static ErrorStatus dp_download_edge_clean_handle(const unsigned char value[], unsigned short length)
{
	//示例:当前DP类型为BOOL
	ErrorStatus ret = SUCCESS;
	//0:关/1:开
	unsigned char ctl_able = 0xFF;

  	ctl_able = mcu_get_dp_download_bool(value,length);

	if(0 == ctl_able)
		log_printf("[Wi-Fi DP]edge cleaning disable\r\n");
	else
		log_printf("[Wi-Fi DP]edge cleaning enable\r\n");


	cfg->r[IC_EDGE_STATUS] = (ctl_able != 0);
	log_printf("set edge=%d\r\n",ctl_able);
	save_cfg();

	ret = mcu_dp_bool_update(DPID_EDGE_CLEANING,ctl_able);
	return ret;
}

/*****************************************************************************
函数名称 : dp_download_wall_handle
功能描述 : 针对DPID_WALL的处理函数
输入参数 : value:数据源数据
        : length:数据长度
返回参数 : 成功返回:SUCCESS/失败返回:ERROR
使用说明 : 可下发可上报类型,需要在处理完数据后上报处理结果至app
*****************************************************************************/
static ErrorStatus dp_download_wall_handle(const unsigned char value[], unsigned short length)
{
	uint8_t i,wall_cnt;
	//示例:当前DP类型为raw
	ErrorStatus ret = SUCCESS;
	union{
		struct{
				unsigned long x1;
				unsigned long y1;
				unsigned long x2;
				unsigned long y2;
		} uls;
		unsigned char bt[16];
	} wall_data[5];

  	wall_cnt = length/16;
  	if(wall_cnt > 5)wall_cnt = 5;
  	length = wall_cnt*16;
	//unsigned char frame_data[16] = {0};
  	mcu_get_dp_download_raw(value, (uint8_t *)wall_data, length);
	/*
  	for(uint8_t i = 0;i < 4;i++)
  	{
		wall_data.bt[4 * i + 0] = frame_data[4 * i + 3];
		wall_data.bt[4 * i + 1] = frame_data[4 * i + 2];
		wall_data.bt[4 * i + 2] = frame_data[4 * i + 1];
		wall_data.bt[4 * i + 3] = frame_data[4 * i + 0];
  	}
  	*/
  	log_printf("[Wi-Fi DP]wall cnt-%d,len-%d:\r\n",wall_cnt,length);
  	for(i = 0;i < wall_cnt;i ++)
  	{
		log_printf("(%d,%d),(%d,%d)\r\n",
					wall_data[i].uls.x1,wall_data[i].uls.y1,wall_data[i].uls.x2,wall_data[i].uls.y2);
  	}
	if(SUCCESS == ret)
	{
		//处理完DP数据后应有反馈
		ret = mcu_dp_raw_update(DPID_WALL, (uint8_t *)wall_data, length);
	}

	return ret;
}
/*****************************************************************************
函数名称 : dp_download_wall_handle
功能描述 : 针对DPID_MAP的处理函数
输入参数 : value:数据源数据
        : length:数据长度
返回参数 : 成功返回:SUCCESS/失败返回:ERROR
使用说明 : 可下发可上报类型,需要在处理完数据后上报处理结果至app
*****************************************************************************/
static ErrorStatus dp_download_map_handle(const unsigned char value[], unsigned short length)
{
	ErrorStatus ret = SUCCESS;
	union{
		struct{
				unsigned int x1;
				unsigned int y1;
				unsigned int x2;
				unsigned int y2;
		} uls;
		unsigned char bt[16];
	} map_data;
	//unsigned char frame_data[16] = {0};
  	mcu_get_dp_download_raw(value, map_data.bt, length);
  	/*for(uint8_t i = 0;i < 4;i++)
  	{
		map_data.bt[4 * i + 0] = frame_data[4 * i + 3];
		map_data.bt[4 * i + 1] = frame_data[4 * i + 2];
		map_data.bt[4 * i + 2] = frame_data[4 * i + 1];
		map_data.bt[4 * i + 3] = frame_data[4 * i + 0];
  	}*/
	log_printf("[Wi-Fi DP]map:(%d,%d),(%d,%d),len:%d\r\n",
				map_data.uls.x1,map_data.uls.y1,map_data.uls.x2,map_data.uls.y2,length);
	if(SUCCESS == ret)
	{
		ret = mcu_dp_raw_update(DPID_MAP, map_data.bt, length);
	}
	return ret;
}
/*****************************************************************************
函数名称 : dp_download_fan_handle
功能描述 : 针对DPID_SENSITIVITY的处理函数
输入参数 : value:数据源数据
        : length:数据长度
返回参数 : 成功返回:SUCCESS/失败返回:ERROR
使用说明 : 可下发可上报类型,需要在处理完数据后上报处理结果至app
*****************************************************************************/
static ErrorStatus dp_download_sensitivity_handle(const unsigned char value[], unsigned short length)
{
  ErrorStatus ret;
  unsigned short sen_lv = 0xFFFF;
  sen_lv = (unsigned short)mcu_get_dp_download_value_littleindian(value,length);

	cfg->r[IC_DUSTSENSOR_EN] = sen_lv;
	log_printf("set dustsensor=%d\r\n",sen_lv);
	save_cfg();

	if(sen_lv == 0)
	{
		sys->dust_sensor = DUST_LOW;
	}

  ret = mcu_dp_value_update(DPID_SENSITIVITY,sen_lv);
  return ret;
}
/*****************************************************************************
函数名称 : dp_download_fan_handle
功能描述 : 针对DPID_WHEEL的处理函数
输入参数 : value:数据源数据
        : length:数据长度
返回参数 : 成功返回:SUCCESS/失败返回:ERROR
使用说明 : 可下发可上报类型,需要在处理完数据后上报处理结果至app
*****************************************************************************/
static ErrorStatus dp_download_wheel_handle(const unsigned char value[], unsigned short length)
{
  ErrorStatus ret;
  unsigned long speed_sw = 0xFF;
  speed_sw = mcu_get_dp_download_value_littleindian(value,length);
	switch(speed_sw)
	{
		case 1:
			//sys->pwm = TURN_DEG_PWM;
			sys->run_speed = LOW_SPEED;
			log_printf("\r\nSlow\r\n");
		break;
		case 2:
			//sys->pwm = FW_SLOW_PWM;
			sys->run_speed = MIDLE_SPEED;
			log_printf("\r\nMiddle\r\n");
		break;
		case 3:
			//sys->pwm = BACK_OFF_PWM;
			sys->run_speed = HIGHT_SPEED;
			log_printf("\r\nFast\r\n");
		break;
		default:
			log_printf("[Wi-Fi DP]DPID_WHEEL error type value %d\r\n",speed_sw);
		break;
	}
  ret = mcu_dp_value_update(DPID_WHEEL,speed_sw);
  return ret;
}
/*****************************************************************************
函数名称 : dp_download_fan_handle
功能描述 : 针对DPID_CHARGE的处理函数
输入参数 : value:数据源数据
        : length:数据长度
返回参数 : 成功返回:SUCCESS/失败返回:ERROR
使用说明 : 可下发可上报类型,需要在处理完数据后上报处理结果至app
*****************************************************************************/
static ErrorStatus dp_download_charge_mode_handle(const unsigned char value[], unsigned short length)
{
  ErrorStatus ret;
  unsigned long mode;
  mode = mcu_get_dp_download_value_littleindian(value,length);
	switch(mode)
	{
		case 1:
			log_printf("\r\nStandard charge\r\n");
		break;
		case 2:
			log_printf("\r\nExpress charge\r\n");
		break;
		case 3:
			log_printf("\r\nAdvanced charge\r\n");
		break;
		default:
			log_printf("[Wi-Fi DP]DPID_CHARGE error type value %d\r\n",mode);
		break;
	}
  ret = mcu_dp_value_update(DPID_CHARGE,1);
  return ret;
}
/*****************************************************************************
函数名称 : dp_download_fan_handle
功能描述 : 针对DPID_PAUSE_RESTART的处理函数
输入参数 : value:数据源数据
        : length:数据长度
返回参数 : 成功返回:SUCCESS/失败返回:ERROR
使用说明 : 可下发可上报类型,需要在处理完数据后上报处理结果至app
*****************************************************************************/
static ErrorStatus dp_download_pause_handle(const unsigned char value[], unsigned short length)
{
  ErrorStatus ret;
  unsigned long mode;
  mode = mcu_get_dp_download_value_littleindian(value,length);
	switch(mode)
	{
		case 0:
			if(sys->sState == SYS_NAVIGAT)
			{
				sys->sState = SYS_IDLE;
				init_sys_sta(SYS_IDLE);
				STOP_ALL_MOTOR();
				delay_sensor(2000);
				log_printf("\r\npause\r\n");
			}
		break;
		case 1:
			if(sys->sState == SYS_IDLE)
			{
				init_sys_sta(SYS_NAVIGAT);
				delay_ms(500);
				log_printf("\r\nclean\r\n");
			}
		break;
		default:
			log_printf("[Wi-Fi DP]DPID_PAUSE_RESTART error type value %d\r\n",mode);
		break;
	}
  ret = mcu_dp_value_update(DPID_PAUSE_RESTART,mode);
  return ret;
}
/*****************************************************************************
函数名称 : dp_download_findsound_handle
功能描述 : 针对DPID_RETURN_TO_BASE的处理函数
输入参数 : value:数据源数据
        : length:数据长度
返回参数 : 成功返回:SUCCESS/失败返回:ERROR
使用说明 : 可下发可上报类型,需要在处理完数据后上报处理结果至app
*****************************************************************************/
static ErrorStatus dp_download_dock_handle(const unsigned char value[], unsigned short length)
{
	ErrorStatus ret = SUCCESS;
	unsigned long dock_able = 0xFF;
  	dock_able = mcu_get_dp_download_value_littleindian(value, length);
  	if((1 == dock_able) && (SYS_DOCK != sys->sState) && (SYS_CHARGE != sys->sState))
  	{
  		init_sys_sta(SYS_DOCK);
  	}
  	else if((0 == dock_able) && (SYS_DOCK == sys->sState))
  	{
		init_sys_sta(SYS_IDLE);
  	}
	//if(SUCCESS == ret)
	{
		ret = mcu_dp_value_update(DPID_RETURN_TO_BASE, 1);  //dock_able   ES需要返回0  20200529
		delay_ms_tm5(20);
		mcu_dp_value_update(DPID_RETURN_TO_BASE, 0);
	}
	return ret;
}
/*****************************************************************************
函数名称 : dp_download_findsound_handle
功能描述 : 针对DPID_FLOOR_SENSOR_STATUS的处理函数
输入参数 : value:数据源数据
        : length:数据长度
返回参数 : 成功返回:SUCCESS/失败返回:ERROR
使用说明 : 可下发可上报类型,需要在处理完数据后上报处理结果至app
*****************************************************************************/
static ErrorStatus dp_download_floor_handle(const unsigned char value[], unsigned short length)
{
	//示例:当前DP类型为VALUE
	ErrorStatus ret = SUCCESS;
	//0:关/1:开
	unsigned short floor_sen_able = 0xFFFF;

  	floor_sen_able = (unsigned short)mcu_get_dp_download_value_littleindian(value, length);

	cfg->r[IC_FLOORSENSOR_EN] = floor_sen_able;
	log_printf("set floorsensor=%d\r\n",floor_sen_able);
	save_cfg();

	ret = mcu_dp_value_update(DPID_FLOOR_SENSOR_STATUS, floor_sen_able);

	return ret;
}
/*****************************************************************************
函数名称 : dp_download_light_handle
功能描述 : 针对DPID_FLOOR_SENSOR_STATUS的处理函数
输入参数 : value:数据源数据
        : length:数据长度
返回参数 : 成功返回:SUCCESS/失败返回:ERROR
使用说明 : 可下发可上报类型,需要在处理完数据后上报处理结果至app
*****************************************************************************/
static ErrorStatus dp_download_light_handle(const unsigned char value[], unsigned short length)
{
	//示例:当前DP类型为VALUE
	ErrorStatus ret = SUCCESS;
	//0:关/1:开
	unsigned char action = 0;

  	action = mcu_get_dp_download_bool(value, length);

	cfg->r[IC_LED_EN] = action;
	log_printf("set led=%d\r\n",action);
	save_cfg();
	if(SUCCESS == ret)
	{
		//处理完DP数据后应有反馈
		ret = mcu_dp_bool_update(DPID_LIGHT_STATUS, action);
	}

	return ret;
}

/*****************************************************************************
函数名称 : dp_download_doublecleanmode_handle
功能描述 : 针对DPID_FLOOR_SENSOR_STATUS的处理函数
输入参数 : value:数据源数据
        : length:数据长度
返回参数 : 成功返回:SUCCESS/失败返回:ERROR
使用说明 : 可下发可上报类型,需要在处理完数据后上报处理结果至app
*****************************************************************************/
static ErrorStatus dp_download_doublecleanmode_handle(const unsigned char value[], unsigned short length)
{
	//示例:当前DP类型为VALUE
	ErrorStatus ret = SUCCESS;
	//0:关/1:开
	unsigned char action = 0;

  	action = mcu_get_dp_download_bool(value, length);

	if(SUCCESS == ret)
	{
		//处理完DP数据后应有反馈
		ret = mcu_dp_bool_update(DPID_DOUBLECLEAN_MODE, action);
	}

	return ret;
}
/*****************************************************************************
函数名称 : dp_download_wall_handle
功能描述 : 针对DPID_MAP的处理函数
输入参数 : value:数据源数据
        : length:数据长度
返回参数 : 成功返回:SUCCESS/失败返回:ERROR
使用说明 : 可下发可上报类型,需要在处理完数据后上报处理结果至app
*****************************************************************************/
static ErrorStatus dp_download_schedule_handle(const unsigned char value[], unsigned short length)
{
	ErrorStatus ret = SUCCESS;
	unsigned char n_cnt,n_powermode;
	T_WIFI_SCH_DATA dp_schedule_data[7];
	//unsigned char frame_data[16] = {0};
  	mcu_get_dp_download_raw(value, (unsigned char *)dp_schedule_data, length);
  	/*for(uint8_t i = 0;i < 4;i++)
  	{
		map_data.bt[4 * i + 0] = frame_data[4 * i + 3];
		map_data.bt[4 * i + 1] = frame_data[4 * i + 2];
		map_data.bt[4 * i + 2] = frame_data[4 * i + 1];
		map_data.bt[4 * i + 3] = frame_data[4 * i + 0];
  	}*/
  	n_cnt = length / 5;

  	for(unsigned char i = 0;i < n_cnt;i ++)
  	{
  		n_powermode = (dp_schedule_data[i].powermode != 0)?(dp_schedule_data[i].powermode - 1):2;
  		dp_set_schdata(dp_schedule_data[i].weekday,dp_schedule_data[i].hour,dp_schedule_data[i].minute,n_powermode,dp_schedule_data[i].en);
		log_printf("%d,%02d:%02d,%d\r\n",dp_schedule_data[i].weekday,dp_schedule_data[i].hour,dp_schedule_data[i].minute,dp_schedule_data[i].en);
  	}
	//log_printf("[Wi-Fi DP]map:(%d,%d),(%d,%d),len:%d\r\n",
	//			map_data.uls.x1,map_data.uls.y1,map_data.uls.x2,map_data.uls.y2,length);
	if(SUCCESS == ret)
	{
		ret = mcu_dp_raw_update(DPID_MAP, (unsigned char *)dp_schedule_data, length);
	}
	dp_send_schdata();
	return ret;
}
//double clean mode 状态上报
ErrorStatus update_doublecleanmode(unsigned char action)
{
	ErrorStatus ret = SUCCESS;

	log_printf("[DPID_DOUBLECLEAN_MODE]:%d\r\n",action);

	if(SUCCESS == ret)
	{
		ret = mcu_dp_bool_update(DPID_DOUBLECLEAN_MODE, action);
	}
	return ret;
}

//map status 状态上报
ErrorStatus update_mapstatus(unsigned char action)
{
	ErrorStatus ret = SUCCESS;

	log_printf("[DPID_DOUBLECLEAN_MODE]:%d\r\n",action);

	if(SUCCESS == ret)
	{
		ret = mcu_dp_bool_update(DPID_MAP_STATUS, action);
	}
	return ret;
}


//robot position(机器人当前坐标上报)
ErrorStatus update_robotposition(signed short coord_x,signed short coord_y)
{
	ErrorStatus ret = SUCCESS;
	union{
		struct{
				unsigned long x1;
				unsigned long y1;
		} uls;
		unsigned char bt[8];
	} dock_data;
	//unsigned char frame_data[8];

	dock_data.uls.x1 = coord_x;
	dock_data.uls.y1 = coord_y;
	log_printf("[DPID_ROBOT_POSITION]:(%d,%d)\r\n",dock_data.uls.x1,dock_data.uls.y1);
#if 0
  	for(uint8_t i = 0;i < 2;i++)
  	{
		frame_data[4 * i + 3] = dock_data.bt[4 * i + 0];
		frame_data[4 * i + 2] = dock_data.bt[4 * i + 1];
		frame_data[4 * i + 1] = dock_data.bt[4 * i + 2];
		frame_data[4 * i + 0] = dock_data.bt[4 * i + 3];
  	}
#endif
	if(SUCCESS == ret)
	{
		ret = mcu_dp_raw_update(DPID_ROBOT_POSITION, dock_data.bt, 8);
	}
	return ret;
}

/*****************************************************************************
函数名称 : dp_download_floorsensitivitylevel_handle
功能描述 : 针对DPID_FLOOR_SENSOR_STATUS的处理函数
输入参数 : value:数据源数据
        : length:数据长度
返回参数 : 成功返回:SUCCESS/失败返回:ERROR
使用说明 : 可下发可上报类型,需要在处理完数据后上报处理结果至app
*****************************************************************************/
static ErrorStatus dp_download_floorsensitivitylevel_handle(const unsigned char value[], unsigned short length)
{
	//示例:当前DP类型为VALUE
	ErrorStatus ret = SUCCESS;
	//0:关/1:开
	unsigned long floor_sen_able = 0x0;

  	floor_sen_able = mcu_get_dp_download_value_littleindian(value, length);

	if(SUCCESS == ret)
	{
		//处理完DP数据后应有反馈
		ret = mcu_dp_value_update(DPID_FLOORSENSITIVITY_LEVEL, floor_sen_able);
	}

	return ret;
}

//floorSensitivityLevel 地板检测灵敏度
ErrorStatus update_floorsensitivitylevel(unsigned short level)
{
	ErrorStatus ret = SUCCESS;

	log_printf("[DPID_FLOORSENSITIVITY_LEVEL]:(%d)\r\n",level);


	if(SUCCESS == ret)
	{
		//ret = mcu_dp_raw_update(DPID_FLOORSENSITIVITY_LEVEL, frame_data, 4);
		ret = mcu_dp_value_update(DPID_FLOORSENSITIVITY_LEVEL, level);
	}
	return ret;
}
//machMode 机器人工作模式上报
ErrorStatus update_machmode(unsigned short mode)
{
	ErrorStatus ret = SUCCESS;

	log_printf("[DPID_TYPE]:(%d)\r\n",mode);


	if(SUCCESS == ret)
	{
		//ret = mcu_dp_raw_update(DPID_FLOORSENSITIVITY_LEVEL, frame_data, 4);
		ret = mcu_dp_value_update(DPID_TYPE, mode);
	}
	return ret;
}

//robot position(机器人当前坐标上报)
ErrorStatus update_scheduleconfig(void)
{
	ErrorStatus ret = SUCCESS;
	T_WIFI_SCH_DATA dp_schedule_data[7];
	//unsigned char frame_data[16] = {0};

	dp_get_schdata_conv2wifi((unsigned char *)dp_schedule_data);
	log_printf("read schedule data\r\n");

	if(SUCCESS == ret)
	{
		ret = mcu_dp_raw_update(DPID_MAP, (unsigned char *)dp_schedule_data, SIZEOF_WIFISCHDATA_ARRAY);
	}
	return ret;
}


/*****************************************************************************
函数名称 : dp_download_floorsensorstatus_handle
功能描述 : 针对DPID_FLOOR_SENSOR_STATUS的处理函数
输入参数 : value:数据源数据
        : length:数据长度
返回参数 : 成功返回:SUCCESS/失败返回:ERROR
使用说明 : 可下发可上报类型,需要在处理完数据后上报处理结果至app
*****************************************************************************/
static ErrorStatus dp_download_floorsensorstatus_handle(const unsigned char value[], unsigned short length)
{
	//示例:当前DP类型为VALUE
	ErrorStatus ret = SUCCESS;
	//0:关/1:开
	unsigned char action = 0;

  	action = mcu_get_dp_download_bool(value, length);

	if(SUCCESS == ret)
	{
		//处理完DP数据后应有反馈
		ret = mcu_dp_bool_update(DPID_FLOORSENSOR_STATUS, action);
	}

	return ret;
}


/*****************************************************************************************
//  2020 05 08
//上报清扫时间
// {DPID_CLEAN_TIME,DP_TYPE_VALUE}
//  mcu_dp_value_update(DPID_CLEAN_TIME,n_worktime/60);	//VALUE型数据上报;	实时清扫时间

*****************************************************************************************/
/*
ErrorStatus update_n_worktime(unsigned short n_worktime)
{
	ErrorStatus ret = SUCCESS;

	log_printf("[n_worktime]:(%04X)\r\n",n_worktime);


	if(SUCCESS == ret)
	{
		//ret = mcu_dp_raw_update(DPID_FLOORSENSITIVITY_LEVEL, frame_data, 4);
		//ret = mcu_dp_value_update(DPID_WARNING, errorcode);
		ret =  mcu_dp_value_update(DPID_CLEAN_TIME,n_worktime/60);	//VALUE型数据上报;	实时清扫时间

	}
	return ret;
}
*/
//上报错误代码
ErrorStatus update_errorcode(unsigned short errorcode)
{
	ErrorStatus ret = SUCCESS;

	log_printf("[DPID_WARNING]:(%04X)\r\n",errorcode);


	if(SUCCESS == ret)
	{
		//ret = mcu_dp_raw_update(DPID_FLOORSENSITIVITY_LEVEL, frame_data, 4);
		ret = mcu_dp_value_update(DPID_WARNING, errorcode);
	}
	return ret;
}
//上报轮速 1/2/3 slow/middle/fast
ErrorStatus update_wheelspeed(void)
{
	ErrorStatus ret = SUCCESS;
	unsigned char speed;

	if(sys->run_speed == LOW_SPEED)
		speed = 1;
	else if(sys->run_speed == HIGHT_SPEED)
		speed = 3;
	else
		speed = 2;

	log_printf("[DPID_WHEEL]:(%d)\r\n",speed);


	if(SUCCESS == ret)
	{
		//ret = mcu_dp_raw_update(DPID_FLOORSENSITIVITY_LEVEL, frame_data, 4);
		ret = mcu_dp_value_update(DPID_WHEEL, speed);
	}
	return ret;
}


//充电座坐标
ErrorStatus update_dock_coord(signed short coord_x,signed short coord_y)
{
	ErrorStatus ret = SUCCESS;
	union{
		struct{
				unsigned long x1;
				unsigned long y1;
		} uls;
		unsigned char bt[8];
	} dock_data;
	//unsigned char frame_data[8];

	dock_data.uls.x1 = coord_x;
	dock_data.uls.y1 = coord_y;
	log_printf("[DPID_CHARGESTATION]:(%d,%d)\r\n",dock_data.uls.x1,dock_data.uls.y1);
#if 0
  	for(uint8_t i = 0;i < 2;i++)
  	{
		frame_data[4 * i + 3] = dock_data.bt[4 * i + 0];
		frame_data[4 * i + 2] = dock_data.bt[4 * i + 1];
		frame_data[4 * i + 1] = dock_data.bt[4 * i + 2];
		frame_data[4 * i + 0] = dock_data.bt[4 * i + 3];
  	}
#endif
	if(SUCCESS == ret)
	{
		ret = mcu_dp_raw_update(DPID_CHARGESTATION, dock_data.bt, 8);
	}
	return ret;
}

ErrorStatus dp_download_spotarea_handle(const unsigned char value[], unsigned short length)
{
	ErrorStatus ret = SUCCESS;
	union{
		struct{
				unsigned long x1;
				unsigned long y1;
				unsigned long x2;
				unsigned long y2;
		} uls;
		unsigned char bt[16];
	} map_data;
	//unsigned char frame_data[16] = {0};
  	mcu_get_dp_download_raw(value, map_data.bt, length);
  	#if 0
  	for(uint8_t i = 0;i < 4;i++)
  	{
		map_data.bt[4 * i + 0] = frame_data[4 * i + 3];
		map_data.bt[4 * i + 1] = frame_data[4 * i + 2];
		map_data.bt[4 * i + 2] = frame_data[4 * i + 1];
		map_data.bt[4 * i + 3] = frame_data[4 * i + 0];
  	}
  	#endif
	log_printf("[Wi-Fi DP]DPID_SPOT:(%d,%d),(%d,%d)\r\n",
				map_data.uls.x1,map_data.uls.y1,map_data.uls.x2,map_data.uls.y2);
	if(SUCCESS == ret)
	{
		ret = mcu_dp_raw_update(DPID_SPOT, map_data.bt, length);
	}
	return ret;
}

//区域清扫 对角坐标
ErrorStatus update_spot_area(XY_T coord[])
{
	ErrorStatus ret = SUCCESS;
	union{
		struct{
				unsigned long x1;
				unsigned long y1;
				unsigned long x2;
				unsigned long y2;
		} uls;
		unsigned char bt[16];
	} spot_data;
	//unsigned char frame_data[16] = {0};
	spot_data.uls.x1 = coord[0].x;
	spot_data.uls.y1 = coord[0].y;
	spot_data.uls.x2 = coord[1].x;
	spot_data.uls.y2 = coord[1].y;
	log_printf("[DPID_SPOT]:(%d,%d),(%d,%d)\r\n",
				spot_data.uls.x1,spot_data.uls.y1,spot_data.uls.x2,spot_data.uls.y2);
	#if 0
  	for(uint8_t i = 0;i < 4;i++)
  	{
		frame_data[4 * i + 3] = spot_data.bt[4 * i + 0];
		frame_data[4 * i + 2] = spot_data.bt[4 * i + 1];
		frame_data[4 * i + 1] = spot_data.bt[4 * i + 2];
		frame_data[4 * i + 0] = spot_data.bt[4 * i + 3];
  	}
#endif
	if(SUCCESS == ret)
	{
		ret = mcu_dp_raw_update(DPID_SPOT, spot_data.bt, 16);
	}
	return ret;
}

struct __coor_mcu_dpid{
	long x;
	long y;
}wifi_coor[5] = {{1,1},{2,2},{3,3},{4,4},{5,5},};

uint16_t wifi_coor_type[5] = {0,1,2,0,2};

uint8_t wifi_coor_buff[128];
//robot position(机器人当前坐标上报)
ErrorStatus update_coordinate(void)
{
	ErrorStatus ret = SUCCESS;
	uint8_t i,j,k;
	uint8_t *byteptr_coor,*byteptr_type;

	byteptr_coor = (uint8_t *)wifi_coor;
	byteptr_type = (uint8_t *)wifi_coor_type;
	k = 0;
	for(i = 0;i < 5;i ++)
	{
		for(j = 0;j < 8;j ++)
		{
			wifi_coor_buff[k ++] = *byteptr_coor ++;
		}
		wifi_coor_buff[k ++] = *byteptr_type++;
		wifi_coor_buff[k ++] = *byteptr_type++;
	}
	//log_printf("[DPID_ROBOT_POSITION]:(%d,%d)\r\n",dock_data.uls.x1,dock_data.uls.y1);
	if(SUCCESS == ret)
	{
		ret = mcu_dp_raw_update(DPID_COORDINATE, wifi_coor_buff, k);
	}
	return ret;
}
/******************************************************************************
                                WARNING!!!
此代码为SDK内部调用,请按照实际dp数据实现函数内部数据
******************************************************************************/
//#ifdef SUPPORT_MCU_RTC_CHECK
/*****************************************************************************
函数名称 : mcu_write_rtctime
功能描述 : MCU校对本地RTC时钟
输入参数 : 无
返回参数 : 无
使用说明 : MCU需要自行实现该功能
*****************************************************************************/

void mcu_write_rtctime(unsigned char time[])
{
  //#error "请自行完成RTC时钟写入代码,并删除该行"
  /*
  time[0]为是否获取时间成功标志，为 0 表示失败，为 1表示成功
  time[1] 为 年 份 , 0x00 表 示2000 年
  time[2]为月份，从 1 开始到12 结束
  time[3]为日期，从 1 开始到31 结束
  time[4]为时钟，从 0 开始到23 结束
  time[5]为分钟，从 0 开始到59 结束
  time[6]为秒钟，从 0 开始到59 结束
  time[7]为星期，从 1 开始到 7 结束，1代表星期一
 */

 log_printf("[mcu_write_rtctime]%s\r\n",time[0] == 1?"suc":"failed");
  if(time[0] == 1)
  {
  /*
     	for(uint8_t i=0;i<3;i++)
	{
		delay_ms_tm5(1000);
		ny3p_play(VOICE_DRIP);
	}
	*/
    //正确接收到wifi模块返回的本地时钟数据
	struct tm time_now;
	time_t unix_time_now;
	time_now.tm_sec = time[6];
	time_now.tm_min = time[5];
	time_now.tm_hour = time[4];
	time_now.tm_mday = time[3];//(time[3]>0)?(time[3] - 1):0;
	time_now.tm_mon = (time[2]>0)?(time[2] - 1):0;
	time_now.tm_year = time[1] + 2000;
	time_now.tm_wday = (time[7] == 7)?0:time[7];


	//RTC_TimeSetCounter(unix_time_now);
	unix_time_now = Time_ConvCalendarToUnix(time_now);
	Time_SetUnixTime(unix_time_now);		/////
	//mw_refreshtime();
	#if WIFIAPP_EN
	pwifistream->sync_flg = 2;
	#endif
	log_printf("[WIFI]RTC Ct=%ld,%02d-%02d,%02d:%02d,%d\r\n",unix_time_now,time_now.tm_mon+1,time_now.tm_mday,
															time_now.tm_hour,time_now.tm_min,time_now.tm_wday);
	pwifistream->clean_time_record.year = time[1] + 2000;
	pwifistream->clean_time_record.month = time[2];
	pwifistream->clean_time_record.day = time[3];			//本地清扫开始时的时间记录
	/*time_now.tm_hour*/pwifistream->clean_time_record.hour= time[4];
	/*time_now.tm_min*/pwifistream->clean_time_record.min= time[5];
	log_printf("[WIFI]RTC Ct=%ld,%02d-%02d %02d:%02d\r\n",
				pwifistream->clean_time_record.year,pwifistream->clean_time_record.month,pwifistream->clean_time_record.day,
				pwifistream->clean_time_record.hour,pwifistream->clean_time_record.min
				);

	//time_now = Time_GetCalendarTime();
	//log_printf("[WIFI](%02d:%02d %d)\r\n",m_time_now.tm_hour,m_time_now.tm_min,m_time_now.tm_wday);
	//log_printf("[WIFI](%02d:%02d %d)\r\n",time_now.tm_hour,time_now.tm_min,time_now.tm_wday);

	unsigned char *temp_time;
	temp_time = (unsigned char *)&unix_time_now;
	dp_tx_data(0x06, 4,temp_time);
	//dp_tx_data(0x06,);
	//unix_time_now = Time_ConvCalendarToUnix(time_now);		/////
	log_printf("%d\r\n",unix_time_now);
  }
  else
  {
  /*
     	for(uint8_t i=0;i<2;i++)
	{
		delay_ms_tm5(1000);
		ny3p_play(VOICE_DRIP);
	}
	*/
  #if WIFIAPP_EN
  	//获取本地时钟数据出错,有可能是当前wifi模块未联网
  	if(pwifistream->sync_flg == 2)
		pwifistream->sync_flg = 0;
  #endif

  }
}
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
void wifi_test_result(unsigned char result,unsigned char rssi)
{
  //#error "请自行实现wifi功能测试成功/失败代码,完成后请删除该行"
  if(result == 0)
  {
    //测试失败
    if(rssi == 0x00)
    {
      //未扫描到名称为tuya_mdev_test路由器,请检查
    }
    else if(rssi == 0x01)
    {
      //模块未授权
    }
  }
  else
  {
    //测试成功
    //rssi为信号强度(0-100, 0信号最差，100信号最强)
  }

}
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
ErrorStatus mcu_firm_update_handle(const unsigned char value[],unsigned long position,unsigned short length)
{
  //#error "请自行完成MCU固件升级代码,完成后请删除该行"
  if(length == 0)
  {
    //固件数据发送完成

  }
  else
  {
    //固件数据处理
  }

  return SUCCESS;
}
//#endif

/******************************************************************************
                                WARNING!!!
以下函数用户请勿修改!!
******************************************************************************/

/*****************************************************************************
函数名称 : dp_download_handle
功能描述 : dp下发处理函数
输入参数 : dpid:DP序号
           value:dp数据缓冲区地址
           length:dp数据长度
返回参数 : 成功返回:SUCCESS/失败返回:ERRO
使用说明 : 该函数用户不能修改
*****************************************************************************/
unsigned char dp_download_handle(unsigned char dpid,const unsigned char value[], unsigned short length)
{
  /*********************************
  当前函数处理可下发/可上报数据调用
  具体函数内需要实现下发数据处理
  完成用需要将处理结果反馈至APP端,否则APP会认为下发失败
  ***********************************/
  unsigned char ret;
  log_printf("DP:%2d\r\n",dpid);

	uint8_t last_wifistate =  sys->wifistatechange;
	uint8_t last_dis_state = sys->dis_statechange;

	 sys->wifistatechange = 1;
	 sys->dis_statechange = 1;
  switch(dpid)
  {
	  case DPID_START:
		 //电源开关处理函数
		 ret = dp_download_power_go_handle(value,length);
		 break;
	   case DPID_TYPE:
		 //清扫模式处理函数
		 ret = dp_download_mode_handle(value,length);
		 break;

		 //case DPID_CLEAN_TIME:
		 //上报清扫时间  20200508
		 //ret = mcu_dp_value_update(value,length);
		 // break;
	   case DPID_FAN:
		 //风机控制处理函数
		 ret = dp_download_fan_handle(value,length);
		 break;
	   case DPID_SPEAKER:
		 //喇叭声音大小处理函数
		 ret = dp_download_speaker_handle(value,length);
		 break;
	   case DPID_WALL:
		 //虚拟墙处理函数
		 ret = dp_download_wall_handle(value,length);
		 break;
	   case DPID_SPOT:
	   	 ret = dp_download_spotarea_handle(value,length);
	   	 break;
	   case DPID_MAP:
		 //地图大小处理函数
		 ret = dp_download_map_handle(value,length);
		 break;
	   case DPID_EDGE_CLEANING:
		 //沿边清扫处理函数
		 ret = dp_download_edge_clean_handle(value,length);
		 break;
	   case DPID_SENSITIVITY:
		 //吸尘灵敏度处理函数
		 ret = dp_download_sensitivity_handle(value,length);
		 break;
	   case DPID_WHEEL:
		 //行走速度处理函数
		 ret = dp_download_wheel_handle(value,length);
		 break;
	   case DPID_CHARGE:
		 //充电模式处理函数
		 ret = dp_download_charge_mode_handle(value,length);
		 break;
	   case DPID_DIRECTION_CONTROL:
		 //方向控制处理函数
		 ret = dp_download_directioncontrol_handle(value,length);
		 break;
	   case DPID_PAUSE_RESTART:
		 //暂停清扫处理函数
 		 ret = dp_download_pause_handle(value,length);
		 break;
	   case DPID_FIND_SOUND:
		 //寻找机器处理函数
		 ret = dp_download_findsound_handle(value,length);
		 break;
	   case DPID_RETURN_TO_BASE:
		 //返回充电座处理函数
		 ret = dp_download_dock_handle(value,length);
		 break;
	   case DPID_FLOOR_SENSOR_STATUS:
		 //地毯检测处理函数
		 ret = dp_download_floor_handle(value,length);
		 break;
	   case DPID_LIGHT_STATUS:
		 ret = dp_download_light_handle(value,length);
		 break;
	   case DPID_DOUBLECLEAN_MODE:
		 ret = dp_download_doublecleanmode_handle(value,length);
		 break;
	   case DPID_FLOORSENSITIVITY_LEVEL:
		 ret = dp_download_floorsensitivitylevel_handle(value,length);
		 break;
	   case DPID_FLOORSENSOR_STATUS:
		 ret = dp_download_floorsensorstatus_handle(value,length);
		 break;
	   case DPID_SCHEDULE:
		 ret = dp_download_schedule_handle(value,length);
		 break;
  	default:
  		sys->wifistatechange = last_wifistate;
		sys->dis_statechange = last_dis_state;
  		log_printf("UNKNOW DPID:%d\r\n",dpid);
    		break;
  }
   sys->t_idle = 0 ;
  return ret;
}
/*****************************************************************************
函数名称 : get_download_cmd_total
功能描述 : 获取所有dp命令总和
输入参数 : 无
返回参数 : 下发命令总和
使用说明 : 该函数用户不能修改
*****************************************************************************/
unsigned char get_download_cmd_total(void)
{
  return(sizeof(download_cmd) / sizeof(download_cmd[0]));
}
//#ifdef WIFI_STREAM_ENABLE
/*****************************************************************************
函数名称 : stream_file_trans
功能描述 : 流服务文件发送
输入参数 : id:ID号
          buffer:发送包的地址
          buf_len:发送包长度
返回参数 : 无
*****************************************************************************/
ErrorStatus stream_file_trans(unsigned int id, unsigned char *buffer, unsigned long buf_len)
{
//  #error "这里仅给出示例，请自行完善天气数据处理代码,完成后请删除该行"
//  unsigned short length = 0;
  unsigned long map_offset = 0;
  unsigned int pack_num = 0;
  unsigned int rest_length = 0;

  if(stop_update_flag == ENABLE)
    return SUCCESS;

  pack_num = buf_len / STREM_PACK_LEN;
  rest_length = buf_len - pack_num * STREM_PACK_LEN;
  if (rest_length > 0)
  {
    pack_num++;
  }

  int this_len = STREM_PACK_LEN;
  for (int cnt = 0; cnt < pack_num; cnt++)
  {
    if (cnt == pack_num - 1 && rest_length > 0)
    {
      this_len = rest_length;
    }
    else
    {
      this_len = STREM_PACK_LEN;
    }

    stream_trans(id, map_offset, buffer + map_offset, this_len);

    //while(stream_status == 0xff);//收到返回

    if(stream_status != 0)
    {
      return ERROR;
    }
  }
  return SUCCESS;
}
void wifi_sync_state_pro(uint8_t wifi_state)
{
	switch(wifi_state)
	{
		case SMART_CONFIG_STATE:		///000
			/*sys->work_status_info &= ~MSINFO_CON_WIFI;		//配网状态ez
			sys->work_status_info &= ~MSINFO_CON_WIFI2; 	//网络状态2 配网状态
			sys->work_status_info &= ~MSINFO_WIFI;			//未连接
			*/
			sys->wifi_state = 0;
			sys->dis_statechange = 1;
			//sys->wifistatechange_tmr = msWifiTmr
			log_printf("cofig_end_tmr:SMART_CONFIG_STATE:%d\r\n",msWifiTmr-sys->wifistatechange_tmr);
		break;
		case AP_STATE:					///001
			/*sys->work_status_info &= ~MSINFO_CON_WIFI;		//配网状态ez
			sys->work_status_info &= ~MSINFO_CON_WIFI2; 	//网络状态2 配网状态
			sys->work_status_info |= MSINFO_WIFI;			//未连接
			*/
			sys->wifi_state = 0;
			sys->dis_statechange = 1;
			log_printf("cofig_end_tmr:AP_STATE:%d\r\n",msWifiTmr-sys->wifistatechange_tmr);
		break;
		case WIFI_NOT_CONNECTED:		///010
			/*sys->work_status_info &= ~MSINFO_CON_WIFI;		//配网状态ez
			sys->work_status_info |= MSINFO_CON_WIFI2;	//网络状态2 配网状态
			sys->work_status_info &= ~MSINFO_WIFI;			//未连接
			*/
			sys->wifi_state = 0;
			sys->dis_statechange = 1;
			log_printf("cofig_temp_tmr:%d\r\n",msWifiTmr-sys->wifistatechange_tmr);
		break;
		case WIFI_CONN_CLOUD: 		///100
			/*sys->work_status_info |= MSINFO_CON_WIFI;		//配网状态ez
			sys->work_status_info &= ~MSINFO_CON_WIFI2; 	//网络状态2 配网状态
			sys->work_status_info &= ~MSINFO_WIFI;			//未连接
			*/
			sys->wifi_state = 1;							//已连接
			sys->dis_statechange = 1;
		break;
		default:
		break;
	}



}

/*
void update_dock_point(void)
{
//	int i=0;
	uint16_t temp_x=0,temp_y=0;

	if(pwifistream->updata_dock_flag != 1)		//没有上报过充电座
	{
		log_printf("updata_dock: \r\n");
		if(DOCK_DETECT())
		{
			///属性应该作为参数传入,暂时先这样2018-09-14 jzz
			if(SCANE_XY(navigat->tx,navigat->ty) || (OBST_XY(navigat->tx,navigat->ty))) //已经清扫过的个数
			{
				log_printf("updata_dock1:");
				temp_x = navigat->tx;
				temp_y = navigat->ty;
				//updata_stream_cache_data(navigat->tx,navigat->ty,0,POINT_DOCK);		//假设为充电座
			}
			else
			{
			#if 0
				if(navigat->tx>100) 			//只判断了X方向>100
				{
					for(i=0;i<100;i++)
					{
						if(SCANE_XY(navigat->tx-i,navigat->ty) || (OBST_XY(navigat->tx-i,navigat->ty)))
						{
							if(navigat->tx-i<200)
							{
								temp_x = navigat->tx-i;
								temp_y = navigat->ty;
								log_printf("updata_dock2:");

							}
								//updata_stream_cache_data(navigat->tx-i,navigat->ty,0,POINT_DOCK); 	//假设为充电座
							else
							{
								temp_x = 200;
								temp_y = navigat->ty;
								log_printf("updata_dock3:");

							}
							//	updata_stream_cache_data(200,navigat->ty,0,POINT_DOCK); 	//假设为充电座
							break;
						}
					}
				}
				else
				{
					for(i=0;i<100;i++)//判断不全，后期增加//只判断了X方向<100
					{
						if(SCANE_XY(navigat->tx+i,navigat->ty) || (OBST_XY(navigat->tx+i,navigat->ty)))
						{
							//还需要增加Y方向上的判断
							temp_x = navigat->tx-i;
							temp_y = navigat->ty;
							log_printf("updata_dock4:");
							//updata_stream_cache_data(navigat->tx-i,navigat->ty,0,POINT_DOCK); 	//假设为充电座
							break;
						}
					}
				}
			#endif
				temp_x = navigat->tx;
				temp_y = navigat->ty;

				log_printf("updata_dock5:");
			}
			//temp_x = 100;
			//temp_y = 100;
			log_printf("%d,%d\r\n",temp_x,temp_y);
			updata_stream_cache_data(temp_x,temp_y,0,POINT_DOCK);		//假设为充电座
			pwifistream->cleanover = 1; 	//增加清扫结束标志
			//当前坐标作为充电座位置上报。
		}
	}
	else
	{
		pwifistream->updata_dock_flag = 0;
		log_printf("updata_dock already\r\n");
		temp_x = 100;
		temp_y = 100;
		log_printf("%d,%d\r\n",temp_x,temp_y);
		updata_stream_cache_data(temp_x,temp_y,0,POINT_DOCK);		//假设为充电座
		pwifistream->cleanover = 1;
	}
}


//#endif
*/
// 55 aa 03 05 00 01 01 09
#define APMODE_CODE_LEN	8
unsigned char apmode_code[] = {0x55,0xaa,0x03,0x05,0x00,0x01,0x01,0x09};
void wifi_set_apmode(void)
{
	unsigned char len = APMODE_CODE_LEN;
	unsigned char *in = apmode_code;
	//wifi_uart_write_data(apmode_code,APMODE_CODE_LEN);
  if((NULL == in) || (0 == len))
  {
    return;
  }

  while(len --)
  {
    uart_transmit_output(*in);
    in ++;
  }
}

