
#if !USE_ON_COMPUTER
#include "sys.h"
#include "task_rx.h"
#include "libatcharge.h"
#endif

#include "test_mode.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

uint8_t tm_mode = 0;//1;//测试模式选择,0为板测模式,即主板在测试架上的程序,1为整机测试模式,即机器装完后的自测程序

TBDIO_UART rec_mac_proc_dat,test_mac_proc_dat;


//测试板上传的数据存放的临时变量
struct __testbd_dat{
	uint8_t unread_flag;//未读标志,如果有新的数据上报,此值置1,在被读取过后,被置0

	uint8_t pwr_sw;//电源开关是否打开 1为已打开 0为未打开
	uint8_t mtoc_whl_l;//左后轮电机的短路开关是否通电 1为已短路 0为未短路
	uint8_t mtoc_whl_r;//右后轮电机的短路开关是否通电 1为已短路 0为未短路
	uint8_t mtoc_smt_l;//左边刷电机的短路开关是否通电
	uint8_t mtoc_smt_r;//右边刷电机的短路开关是否通电
	uint8_t mtoc_mt_fan;//风机的短路开关是否打开(未使用)
	uint8_t mtoc_mt_mid;//左边刷电机的短路开关是否通电

	uint8_t bum_sw;//前碰撞光耦模拟输入,bit0为左 bit1为右,1表示关闭(收到为低电平),0表示打开(收到为高电平)
	uint8_t bum_sw_led;//前碰撞光耦的LED光源检测,正常为0,不正常为1,bit0为左,bit1为右
	uint8_t sensors_sw;//其他开关式传感器的模拟输入信号bit0-bit5依次为抹布/虚拟墙/尘盒/集尘盒满/左落轮/右落轮
	uint8_t sensors_sw_led;//其他开关式传感器LED光源检测bit0-bit3依次为抹布/无/无/集尘盒满/无/无

	uint8_t bat_info;//电池的接入相关信息:0电池与测试板断开 1电池接入主板,24V接到DC座 2电池接入主板,24V接到充电触片
					//3电池接入主板,24V未接入主板
	uint8_t nc;//未使用

	uint16_t cd_smt_l;//左边刷码盘数
	uint16_t cd_smt_r;//右边刷码盘数
	uint16_t cd_mt_mid;//中扫码盘数

};
struct __testbd_dat tmd_rxbd_dat;

void tx_dp_msg(uint8_t *buff,int len);
void proc_bd_test_mode(uint8_t *buf,uint8_t len);
void proc_com_test_mode(uint8_t *buf,uint8_t len);
void tx_com_dat(uint8_t code, uint8_t progress, uint8_t error_code, uint8_t ret_value);
void tx_com_mac_dat(TBDIO_UART *proc_dat);
void mac_dat_pro(uint8_t type);
void tx_com_beep(uint8_t progress, uint8_t error_code, uint8_t ret_value,uint16_t *irvalue,uint16_t *irbuttom);
//void tx_dp_dat(uint8_t code, uint8_t para, uint8_t res_out, uint8_t res_in);
void tx_dp_dat(uint8_t code, uint8_t para, uint8_t para1);
void tx_dp_pc_dat(uint8_t code, uint8_t para, uint8_t para1);
void data_print(uint8_t * pdat,uint16_t len);
uint8_t get_chksum(uint8_t *pdata, uint16_t length);

void tx_com_burnin(uint8_t side_dir, uint16_t timer,uint32_t bum_left_ct,uint32_t ir_left_ct,uint32_t bum_right_ct,uint32_t ir_right_ct);
void tx_sn(uint8_t code ,uint8_t *sn);

#define TMOD_COMRX_BUFFSIZE	64

static uint8_t tmd_buf[TMOD_COMRX_BUFFSIZE];
static uint8_t tmd_bd_buf[TMOD_COMRX_BUFFSIZE];

static uint8_t tmod_rx_sta = 0;
static uint8_t tmod_rx_len = 0;
static uint8_t tmod_bd_rx_sta = 0;
static uint8_t tmod_bd_rx_len = 0;

//UART接收服务函数
//com_dp: 函数同时被两个接口调用,即与上位机通信的UART1以及与测试板通信的UART4(原DP口),需要增加此变量以适应
//        不同的协议解析需求,与上位机通信,值为0,与测试板通信值为1
void tmod_rx_uart(uint8_t com_dp, uint8_t chr)
{
	uint8_t *n_pbuf;
	uint8_t *n_prxsta;
	uint8_t *n_polen;
		//log_printf("%02X ",chr);

	if(!com_dp)
	{
		n_pbuf = tmd_buf;
		n_prxsta = &tmod_rx_sta;
		n_polen = &tmod_rx_len;
	}
	else
	{
		n_pbuf = tmd_bd_buf;
		n_prxsta = &tmod_bd_rx_sta;
		n_polen = &tmod_bd_rx_len;
	}

	switch(*n_prxsta)
	{
		case 0:
			if(chr == 0x7E)
			{
				*n_prxsta=1;
			}
			break;
		case 1:
			if(chr ==0x5D)
			{
				*n_prxsta=2;
				memset(n_pbuf,0x00,TMOD_COMRX_BUFFSIZE);
				*n_polen=0;
			}
			else
			 *n_prxsta =0;
			break;
		case 2:
			n_pbuf[(*n_polen)++]=chr;
			if(*n_polen >=TMOD_COMRX_BUFFSIZE)
			{
				log_printf("rx error\r\n");
				*n_polen=0;
				*n_prxsta=0;
				break;
			}
			//7E 5D 05 00 6D 7D
			if(chr == 0x7D && n_pbuf[*n_polen-2]==0x6D)	//接受完毕
			{
				//log_printf("\r\nfjd\r\n\r\n");
				if(!com_dp)//与上位机的通信
					proc_com_test_mode(n_pbuf,*n_polen);
				else
					proc_bd_test_mode(n_pbuf,*n_polen);
				*n_polen=0;
				*n_prxsta=0;
			}
			break;

		default:
				*n_polen=0;
				*n_prxsta=0;
				break;


	}
}

#define BDTEST_LOGPRINT	0

#define BDTEST_SMT_PWMVALUE	900//边刷PWM转速值,倒向,值越大转速越小,范围为1000-0

uint8_t bdtest_fantest = 0;//测试模式下风机测试使能,风机的转速检测脚和前轮码盘的A相检测脚中断号重叠,故以此作为标记
uint16_t tmod_fancd = 0;//风机的码盘计数器,最大计到30000,超出则保持30000
static uint8_t bdtest_sta = TPC_STOP_ALL;//此变量代表的是测试过程中的某个工作环节,而非指令,此变量与串口指令无关,只是使用了与之相同的宏定义
static uint8_t bdtest_sta_child = 0;//测试步骤的小状态,0为初始

#define BDTEST_CHG_SUCC_MIN	20//充电电流在正常范围内的计数最小值,连续计数超过此值,认为充电正常
#define BDTEST_CHG_FAIL_MAX	10//充电电流超限的计数最大值,超过此值,认为充电失败
#define BDTEST_CHG_START_MAX	200//开始充电到达到预定电流的限定时间,如果超过这个时间还未达到预定值,表示充电失败
uint16_t bdtest_chgcurr = 0;//充电电流的记录,当充电电流达到预定值±100ma时,连续20个以上的范围内计数认为充电正常
									//如果有总计超过10个以上的电流超限计数,认为充电失败
									//当超过30秒时,超限计数与正常连续计数都不足认为充电正常
//static uint8_t bdtest_chgsucceed_counter = 0xff;//充电正常计数
//static uint8_t bdtest_chgfailed_counter = 0xff;//充电超限计数
//static uint16_t bdtest_chgstart_counter = 0;//充电开始到电流正常值之前的计数,此值如果大于BDTEST_CHG_START_MAX,表示一直都未达到预定值,充电失败

#if 0
//记录当前的充电电流,外部函数使用
void bdtest_set_curr(uint16_t curr)
{bdtest_chgcurr = curr;}
#endif
//对地红外测试用的临时变量
//static uint8_t tm_ir_bot_cc;//,tm_ir_bot_j;//,tm_ir_bot_i
///static int32_t tm_ir_bot_k1;//,tm_ir_bot_k2;
ErrorStatus test_bd_com = ERROR;		//主板测试时，显控/遥控相关串口的状态
uint8_t r_key_err = 0x03;

void test_mode_gpio(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

       GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
       GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
       GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
       GPIO_Init(GPIOE,&GPIO_InitStructure);

       GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
       GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
       GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
       GPIO_Init(GPIOD,&GPIO_InitStructure);
}

void test_mode_ul(FlagStatus able)
{
	GPIO_InitTypeDef  GPIO_InitStructure;

	if(SET == able)
	{
		GPIO_InitStructure.GPIO_Pin =I2C_ULTRASONIC_CLK_PIN;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(I2C_ULTRASONIC_CLK_PORT, &GPIO_InitStructure);
	
		GPIO_InitStructure.GPIO_Pin =I2C_ULTRASONIC_DATA_PIN ;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(I2C_ULTRASONIC_DATA_PORT, &GPIO_InitStructure);
	}
	else
	{
		GPIO_InitStructure.GPIO_Pin =I2C_ULTRASONIC_CLK_PIN;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(I2C_ULTRASONIC_CLK_PORT, &GPIO_InitStructure);

		GPIO_InitStructure.GPIO_Pin =I2C_ULTRASONIC_DATA_PIN ;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(I2C_ULTRASONIC_DATA_PORT, &GPIO_InitStructure);
		GPIO_ResetBits(I2C_ULTRASONIC_DATA_PORT, I2C_ULTRASONIC_DATA_PIN);
	}
}
void proc_bdtest_task(void)
{
	static uint8_t bd_timer = 0xff;
	static uint8_t bd_heart_timer = 0;
	static uint8_t bd_heart_ct = 0;
//	static uint8_t bdtest_sta_old = TPC_STOP_ALL;
	static uint16_t bd_batcharge_timer = 0;
	static uint8_t bd_stopmode_trigger = TPC_STOP_ALL;//TPC_STOP_ALL状态的新触发标志,用来判断新置了TPC_STOP_ALL状态
	static uint8_t dustbox_order = 0;
	uint32_t mid_close_adc=0,mid_open_adc=0,mid_du_adc=0;
	uint8_t key_err = 0;
	uint16_t adc_t = 0;

	if(TIM5->CNT < 50000)//50ms定时
		return;
	TIM5->CNT = 0;

	if(bd_heart_timer ++ >= 20)//发送心跳包,每秒一个
	{
		if(READ_DUSTBOX_DET() && (0 == dustbox_order))
		{
			dustbox_order = 1;
		}
		else if((!READ_DUSTBOX_DET()) && (1 == dustbox_order))
		{
			dustbox_order = 2;
		}
		bd_heart_timer = 0;
		tx_com_dat(TPC_HEART_BEEP, bd_heart_ct ++, TPC_ERR_NOERROR, 0);
	}
	//return;
	if(bdtest_sta != TPC_STOP_ALL)
		bd_stopmode_trigger = bdtest_sta;
	////log_printf("\r\n[wywy]bd test:%d-%d\r\n",bdtest_sta,bdtest_sta_child);
	switch(bdtest_sta)
	{
		case TPC_STOP_ALL:
			//if(bdtest_sta_old != bdtest_sta)
			if(bd_stopmode_trigger != TPC_STOP_ALL)
			{
				//if(bd_stopmode_trigger == TPC_IDLE)
				{
					tx_com_dat(TPC_STOP_ALL, 100 , TPC_ERR_NOERROR, 0);//上发测试完成
					tx_dp_dat(TBD_STOP_ALL, 0, 0);//将24V打到DC插座接口
				}
				bd_stopmode_trigger = TPC_STOP_ALL;
				TargetSysReset();
				sys->sState = SYS_IDLE;
			}
			MOTOR_POWER_OFF_NPRI();//关闭电机电源
			break;
		case TPC_START_ALL://这里要检查陀螺仪是否正常/与测试板的通信是否正常
			//MOTOR_CTRL(1000, 1000, 0, 0);
			MOTOR_POWER_OFF_NPRI();
			tx_dp_dat(TBD_START_ALL, 0, 0);//将24V打到DC插座接口
			bdtest_sta = TPC_MT_FAN;//TPC_WHL_L;////TPC_WHL_L;//TPC_GYRO_TST;//TPC_WHL_L;//TPC_SMT_L;;
			bdtest_sta_child = 0;
			test_bd_com= ERROR;
			r_key_err = 0x03;
			dustbox_order = 0;
			#if BDTEST_LOGPRINT
			log_printf("\r\n[wywy]bd test start!\r\n");
#endif
			break;
		case TPC_MT_FAN:
			switch(bdtest_sta_child)
			{
				case 0://开始测试风机
					{
						bdtest_sta_child = 1;
						bdtest_fantest = 1;
						tmod_fancd = 0;						

						tx_com_dat(TPC_MT_FAN, 25 , TPC_ERR_NOERROR, 0);
						tx_dp_dat(TBD_FAN, 0, 0);
						delay_ms_tm5(300);

						tx_com_dat(TPC_MT_FAN, 50 , TPC_ERR_NOERROR, 0);

						DUST_MOTOR_RANK3();
						delay_ms(300);
						tx_com_dat(TPC_MT_FAN, 75 , TPC_ERR_NOERROR, 0);
										
						DUST_MOTOR_OFF();
						delay_ms(300);
						tx_com_dat(TPC_MT_FAN, 100 , TPC_ERR_NOERROR, 0);
						//log_printf("\r\n[wywy]FAN success!\r\n");

						tmod_fancd=0;
						bd_timer = 0;
						bdtest_sta_child = 1;
					}
				#if BDTEST_LOGPRINT
					bdtest_sta_child = 2;
					break;
				case 2:				
					TEST_MOTOR_POWER_OFF_NPRI();
					break;
				#endif
				default:
					bdtest_sta = TPC_WHL_L;
					bdtest_sta_child = 0;
					bdtest_fantest = 0;
					break;
			}
			break;			
		case TPC_WHL_L://左轮检测
			switch(bdtest_sta_child&0x0f)
			{
				case 0://开始测试左轮
					bdtest_sta_child = 1;
					MOTOR_POWER_ON();
					motor.c_left_hw = 0;
					motor_wheel_stop(RIGHT_WHEEL);
					motor_wheel_forward(LEFT_WHEEL,600);

					tx_com_dat(TPC_WHL_L, 25 , TPC_ERR_NOERROR, 0);//上报进度
					tx_dp_dat(TBD_MTOC_WHL_L, 0, 0);//下发左后轮过载测试,将短路的电阻断开
					bd_timer = 0;
					break;
				case 1://向前转
					if(bd_timer ++ < 20)break;
					motor_wheel_stop(LEFT_WHEEL);
					#if BDTEST_LOGPRINT
					log_printf("forward:%d\r\n",motor.c_left_hw);
					#endif

					if(motor.c_left_hw < 200)//霍尔数不足
					{
						MOTOR_POWER_OFF_NPRI();
						tx_com_dat(TPC_WHL_L, 50, TPC_ERR_FORWARD, 7);
						bdtest_sta = TPC_WHL_R;//跳到下一个步骤
						bdtest_sta_child = 0;
						break;
					}
					tx_com_dat(TPC_WHL_L, 50, TPC_ERR_NOERROR, 8);
					bdtest_sta_child = 2;
					motor.c_left_hw = 0;
					motor_wheel_backward(LEFT_WHEEL,600);
					bd_timer = 0;
					break;
				case 2://向后转
					if(bd_timer ++ < 20)break;
					motor_wheel_stop(LEFT_WHEEL);

					#if BDTEST_LOGPRINT
					log_printf("back:%d\r\n",motor.c_left_hw);
					#endif
					//log_printf("\r\n[wywy]Left Whell back:%d\r\n",motor.c_left_hw);

					if(motor.c_left_hw < 200)//霍尔数不足
					{
						MOTOR_POWER_OFF_NPRI();
					#if BDTEST_LOGPRINT
						log_printf("TPC_ERR_BACK\r\n");
						delay_ms(10);
					#endif
						tx_com_dat(TPC_WHL_L, 75, TPC_ERR_BACK, (uint8_t)(motor.c_left_hw >> 8));
						bdtest_sta = TPC_WHL_R;//跳到下一个步骤
						bdtest_sta_child = 0;
						break;
					}
					tx_com_dat(TPC_WHL_L, 75, TPC_ERR_NOERROR, 0);//上报进度

					bdtest_sta_child = 3;
					//MOTOR_POWER_ON();
					motor.c_left_hw = 0;

					#if BDTEST_LOGPRINT
						log_printf("BACK PASS\r\n");
						delay_ms(10);
					#endif
					tx_dp_dat(TBD_MTOC_WHL_L, 1, 0);//下发左后轮过载测试,将短路的电阻接入
					//bd_timer = 0;
					tmd_rxbd_dat.unread_flag = 0;//将数据接收的缓存未读标志清0
					break;
				case 3:
					if(bd_timer ++ < 20 && !tmd_rxbd_dat.unread_flag)break;//超时未收到数据
					bd_timer = 0;
					if(!tmd_rxbd_dat.unread_flag)
					{
						tx_com_dat(TPC_DPPORT_TST, 100, TPC_ERR_COMM, 0);//上报进度或上报过流检测错误
					}
					tmd_rxbd_dat.unread_flag = 0;
					if(tmd_rxbd_dat.mtoc_whl_l)
					{
						uint8_t i;
						uint16_t n_mt_curr;

						tx_com_dat(TPC_DPPORT_TST, 100, TPC_ERR_NOERROR, 0);//上报进度或上报过流检测错误
						motor_wheel_forward(LEFT_WHEEL,600);
						delay_ms(100);
						for(i = 0;i < 18;i ++)
						{
							delay_ms(5);
							n_mt_curr = I_MOTER_LEFT();
						#if BDTEST_LOGPRINT
							//log_printf("curr:%d-%d-%d\r\n",n_mt_curr,MAX_MOTOR_I,i);
							log_printf("curr:%d\r\n",n_mt_curr);
							delay_ms(10);
						#endif

							if(n_mt_curr >= 40)
							{
								motor_wheel_stop(LEFT_WHEEL);
								break;
							}
						}

						tx_com_dat(TPC_WHL_L, 100, (i < 18) ? (TPC_ERR_NOERROR) : (TPC_ERR_OC), i);//上报进度或上报过流检测错误
						//if(18 > i)
						//	log_printf("\r\n[wywy]Left Whell success\r\n");
						//else
						//	log_printf("\r\n[wywy]Left Whell error\r\n");
						if(i < 18)
						{
						#if BDTEST_LOGPRINT
							delay_ms(100);
							bdtest_sta_child = 4;
							log_printf("oc succeeded\r\n");
						#else
							bdtest_sta = TPC_WHL_R;//跳到下一个步骤
							bdtest_sta_child = 0;
						#endif
						}
						#if BDTEST_LOGPRINT
						else
						{
							bdtest_sta_child = 4;
							log_printf("\r\n11111\r\n");
							tx_dp_dat(TBD_MTOC_WHL_L, 0, 0);//下发左后轮过载测试,将短路的电阻断开
							break;
						}
						#endif
					}
					else
					{
						tx_com_dat(TPC_DPPORT_TST, 100, TPC_ERR_COMM, 0);//上报进度或上报过流检测错误
					}
					tx_dp_dat(TBD_MTOC_WHL_L, 0, 0);//下发左后轮过载测试,将短路的电阻断开
				#if BDTEST_LOGPRINT
					break;
				case 4:
					log_printf("$");
					motor_wheel_stop(LEFT_WHEEL);
					MOTOR_POWER_OFF_NPRI();
					break;
				#endif
				default:
					bdtest_sta = TPC_WHL_R;
					bdtest_sta_child = 0;
					motor_wheel_stop(LEFT_WHEEL);
					MOTOR_POWER_OFF_NPRI();
					break;
			}
			break;
		case TPC_WHL_R://右轮检测
			switch(bdtest_sta_child&0x0f)
			{
				case 0://开始测试右轮
					bdtest_sta_child = 1;
					MOTOR_POWER_ON();
					motor.c_right_hw = 0;
					motor_wheel_stop(LEFT_WHEEL);
					motor_wheel_forward(RIGHT_WHEEL,600);

					tx_com_dat(TPC_WHL_R, 25 , TPC_ERR_NOERROR, 0);//上报进度
					tx_dp_dat(TBD_MTOC_WHL_R, 0, 0);//下发右后轮过载测试,将短路的电阻断开
					bd_timer = 0;
					break;
				case 1://向前转
					if(bd_timer ++ < 20)break;
					motor_wheel_stop(RIGHT_WHEEL);
					#if BDTEST_LOGPRINT
					log_printf("forward:%d\r\n",motor.c_right_hw);
					#endif
					//log_printf("\r\n[wywy]Right Whell forward:%d\r\n",motor.c_right_hw);

					if(motor.c_right_hw < 200)//霍尔数不足
					{
						MOTOR_POWER_OFF_NPRI();
						tx_com_dat(TPC_WHL_R, 50, TPC_ERR_FORWARD, 0);
						#if BDTEST_LOGPRINT
						bdtest_sta_child = 4;
						#else
						bdtest_sta = TPC_MT_MID;//跳到下一个步骤
						bdtest_sta_child = 0;
						#endif
						break;
					}
					tx_com_dat(TPC_WHL_R, 50, TPC_ERR_NOERROR, 0);
					bdtest_sta_child = 2;
					motor.c_right_hw = 0;
					motor_wheel_backward(RIGHT_WHEEL,600);
					bd_timer = 0;
					break;
				case 2://向后转
					if(bd_timer ++ < 20)break;
					motor_wheel_stop(RIGHT_WHEEL);

					#if BDTEST_LOGPRINT
					log_printf("back:%d\r\n",motor.c_right_hw);
					log_printf("\r\n[wywy]Right Whell back:%d\r\n",motor.c_right_hw);
					#endif

					if(motor.c_right_hw < 200)//霍尔数不足
					{
						MOTOR_POWER_OFF_NPRI();
						tx_com_dat(TPC_WHL_R, 75, TPC_ERR_BACK, 0);//上报电机反向不转

						#if BDTEST_LOGPRINT
						bdtest_sta_child = 4;
						#else
						bdtest_sta = TPC_MT_MID;//跳到下一个步骤
						bdtest_sta_child = 0;
						#endif
						break;
					}
					tx_com_dat(TPC_WHL_R, 75, TPC_ERR_NOERROR, 0);//上报进度

					bdtest_sta_child = 3;
					motor.c_left_hw = 0;

					tx_dp_dat(TBD_MTOC_WHL_R, 1, 0);//下发左后轮过载测试,将短路的电阻接入
					bd_timer = 0;
					tmd_rxbd_dat.unread_flag = 0;//将数据接收的缓存未读标志清0
					break;
				case 3:
					if(bd_timer ++ < 20)break;//超时未收到数据
					bd_timer = 0;
					tmd_rxbd_dat.unread_flag = 0;
					if(1)////(tmd_rxbd_dat.mtoc_whl_r)
					{
						uint8_t i;
						uint16_t n_mt_curr;

						motor_wheel_forward(RIGHT_WHEEL,600);
						delay_ms(100);
						for(i = 0;i < 18;i ++)
						{
							delay_ms(5);
							n_mt_curr = I_MOTER_RIGHT();
						#if BDTEST_LOGPRINT
							//log_printf("curr:%d-%d-%d\r\n",n_mt_curr,MAX_MOTOR_I,i);
						log_printf("\r\n[wywy]Right Whell curr:%d\r\n",n_mt_curr);
						#endif
							//if(I_MOTER_LEFT() >= MAX_MOTOR_I)
							//if(n_mt_curr >= 100)
							if(n_mt_curr >= 40)
							{
								motor_wheel_stop(RIGHT_WHEEL);
								break;
							}
						}

						tx_com_dat(TPC_WHL_R, 100, (i < 18) ? (TPC_ERR_NOERROR):(TPC_ERR_OC), 0);//上报进度或上报过流检测错误
						#if BDTEST_LOGPRINT
						if(18 > i)
							log_printf("\r\n[wywy]Right Whell success\r\n");
						else
							log_printf("\r\n[wywy]Right Whell error\r\n");
						#endif
						//log_printf("\r\n 55555\r\n");
						if(i < 8)
						{
						#if BDTEST_LOGPRINT
							bdtest_sta_child = 4;
							log_printf("oc succeeded\r\n");
						#else
							bdtest_sta = TPC_MT_MID;//跳到下一个步骤
							bdtest_sta_child = 0;
						#endif
						}
						#if BDTEST_LOGPRINT
						else
						{
							bdtest_sta_child = 4;
							log_printf("\r\n11111\r\n");
							tx_dp_dat(TBD_MTOC_WHL_R, 0, 0);//下发左后轮过载测试,将短路的电阻断开
							break;
						}
						#endif
					}

					tx_dp_dat(TBD_MTOC_WHL_R, 0, 0);//下发左后轮过载测试,将短路的电阻断开
					//bdtest_sta = TPC_WHL_R;
					//bdtest_sta_child = 0;
					//MOTOR_POWER_OFF_NPRI();
				#if BDTEST_LOGPRINT
					break;
				case 4:
					log_printf("$");
					motor_wheel_stop(RIGHT_WHEEL);
					MOTOR_POWER_OFF_NPRI();
					break;
				#endif
				default:
					bdtest_sta = TPC_MT_MID;
					bdtest_sta_child = 0;
					motor_wheel_stop(DOUBLE_WHEEL);
					MOTOR_POWER_OFF_NPRI();
					break;
			}
			break;
		case TPC_MT_MID://测试滚刷电机
			
			
			bdtest_sta_child = 0;
			navigat->gun_cnt=0;
			tx_dp_dat(TBD_MTOC_MT_MID, 0, 0);
			tx_com_dat(TPC_MT_MID, 33, TPC_ERR_NOERROR, 0);
			TEST_MOTOR_POWER_ON();
			SET_MID_MOTER(500);
			delay_ms(500);
			for(uint8_t i=0;i<10;i++)	
			{
				delay_ms(100);
				adc_t = I_MID_ADC();
				mid_open_adc += adc_t;
			}
			#if BDTEST_LOGPRINT
			log_printf("\r\n[wywy]Mid open adc 0:%d\r\n",mid_open_adc);
			#endif
			tx_com_dat(TPC_MT_MID, 67, TPC_ERR_NOERROR, 0);

			tx_dp_dat(TBD_MTOC_MT_MID, 1, 0);
			delay_ms(500);
			for(uint8_t i=0;i<10;i++)	
			{
				delay_ms(100);
				adc_t = I_MID_ADC();
				mid_du_adc += adc_t;
			}
			#if BDTEST_LOGPRINT
			log_printf("\r\n[wywy]Mid open adc 1:%d\r\n",mid_du_adc);
			#endif
			tx_com_dat(TPC_MT_MID, 67, TPC_ERR_NOERROR, 0);
			
			////TEST_MOTOR_POWER_OFF_NPRI();
			tx_dp_dat(TBD_MTOC_MT_MID, 0, 0);
			SET_MID_MOTER(0);
			delay_ms(500);		
			for(uint8_t i=0;i<10;i++)
			{
				delay_ms(100);
				adc_t = I_MID_ADC();				
				mid_close_adc += adc_t;
			}	//未运行电流	
			#if BDTEST_LOGPRINT
			log_printf("\r\n[wywy]Mid close adc:%d\r\n",mid_close_adc);

			log_printf("\r\n[wywy]Mid code:%d\r\n",navigat->gun_cnt);
			#endif
			if(disXY(mid_open_adc,mid_close_adc)<1000 &&disXY(mid_open_adc,mid_close_adc)>0)////200)
			{
				tx_com_dat(TPC_MT_MID, 100, TPC_ERR_NOERROR, 0);//OK
			#if BDTEST_LOGPRINT
				log_printf("\r\n[wywy]Mid success\r\n");
			#endif
			}
			else
			{
				tx_com_dat(TPC_MT_MID, 100, TPC_ERR_FORWARD, 0);//过流
			#if BDTEST_LOGPRINT
				log_printf("\r\n[wywy]Mid error\r\n");
			#endif
			}
			tmod_fancd=0;					
			bdtest_sta = TPC_IR_FRT;//跳到下一个步骤
			bdtest_sta_child = 0;
			break;
		case TPC_IR_FRT://前撞测距(碰撞)红外检测,对地红外在这里一起检查
			switch(bdtest_sta_child)
			{
				case 0:
					{
						uint8_t cc,bot;
						int32_t k1,k2;
						uint8_t i,j;

						sys->c_ir_adc = 0;
						for(cc = 0;cc < 9;cc ++)
						{
							for(i=0;i<10;i++)
							{
								sys->m_sta[i][cc] = adc_converted_value[i];
								//log_printf(",%d",sys->m_sta[i][sys->c_ir_adc] );
							}
							//log_printf("\r\n");

							if(cc <=3 )
							{
								GPIO_ResetBits(PORT_IR_CTRL,PIN_IR_CTRL);	//低电平，关灯
								GPIO_ResetBits(GPIOD,GPIO_Pin_14);	//低电平，关灯
								//GPIO_ResetBits(PORT_IR_CTRL2,PIN_IR_CTRL2); //低电平，关灯
								//门槛
								//NEAR_LAN_ON();
								//FAR_LAN_OFF();
							}
							else
							{

								GPIO_SetBits(PORT_IR_CTRL,PIN_IR_CTRL);
								GPIO_SetBits(GPIOD,GPIO_Pin_14);	//低电平，关灯
								//GPIO_SetBits(PORT_IR_CTRL2,PIN_IR_CTRL2);
								//NEAR_LAN_OFF();
								//FAR_LAN_ON();
							}
							delay_ms(5);
						}

						cc = bot = 0;//cc用来记录前撞红外的检测结果,j用来记录对地红外的检测结果
						for(i=0;i<10;i++)
						{
							k1 = k2 = 0;
							for(j=0;j<4;j++)
								k1 +=sys->m_sta[i][j];
							for(j=4;j<8;j++)
								k2 +=sys->m_sta[i][j];


							if(i >=7)
							{
								sys->g_buton[0][i-7] = k2 / 4; //sys->g_sta[i];
								//if(sys->g_butoncg_buton > 300)//近端红外
								if(sys->g_buton[0][i-7] < 3300 || sys->g_buton[0][i-7] > 3700)//近端红外
								{
									bot |= 1 << (i - 7);
								}
								/*sys->g_buton[1][i-7] = k1 / 4;
								if(sys->g_buton[1][i-7] > 1500)//远端红外
								{
									bot |= 1 << (i - 7 + 3);
								}*/
								//log_printf("\r\n[wywy]G Ir:(%d)%d-%d\r\n",i,sys->g_buton[0][i-7],sys->g_buton[1][i-7]);
								//log_printf("\r\n%d\t%d\r\n",sys->g_buton[0][i-7],sys->g_buton[1][i-7]);
							}else
							{
								if(i == 1 || i == 5)
									continue;
								if(i == 2)
								{
									if(k1 > k2 && ((k1 - k2) /4) < 5000)
										sys->g_sta[i] = (k1 - k2) >> 2;
									else
										sys->g_sta[i] = 0;
									if(sys->g_sta[i] < 1500 || sys->g_sta[i] > 3000)//近端红外
									{
										cc |= 1 << i;
										//log_printf("err:%d-%d\r\n",i,sys->g_sta[i]);
										//delay_ms(10);
									}
								}
								else
								{
									sys->g_sta[i] = (k1) >> 2;
									if(sys->g_sta[i] < 3300 || sys->g_sta[i] > 3700)//近端红外
									{
										cc |= 1 << i;
										//log_printf("err:%d-%d\r\n",i,sys->g_sta[i]);
										//delay_ms(10);
									}
								}

							}
						//log_printf("%d,%d,%d,%d\r\n",i,k1 / 4,k2 /4,(k1-k2 ) / 4);
						}
						#if 0 //调试的打印信息
						log_printf("\r\nirda:%02x\r\nirbt:%02x\r\n",cc,bot);
						delay_ms(500);
						#endif
						//delay_ms(500);
						#if 1
						#if 0
						delay_ms(500);
						log_printf("\r\nir_frt:\r\n");
						for(i = 0;i < 7;i ++)
						{
							log_printf("\t%d",sys->g_sta[i]);
						}
						log_printf("-%02X\r\nir_bot:\r\n",cc);
						for(i = 0;i < 3;i ++)
						{
							log_printf("\t%d",sys->g_buton[0][i]);
						}
						log_printf("-\t-");
						for(i = 0;i < 3;i ++)
						{
							log_printf("\t%d",sys->g_buton[1][i]);
						}
						log_printf("-%02X\r\n\r\n",bot);
						#else

						if(cc)
						{
							tx_com_dat(TPC_IR_FRT, 100 , TPC_ERR_IRLED, cc);//上报进度及错误信息
						}
						else
						{
							tx_com_dat(TPC_IR_FRT, 100 , TPC_ERR_NOERROR, 0);//上报进度及错误信息
						}

						if(bot)
						{
							tx_com_dat(TPC_IR_BOT, 100 , TPC_ERR_IRLED, bot);//上报进度及错误信息
						}
						else
						{
							tx_com_dat(TPC_IR_BOT, 100 , TPC_ERR_NOERROR, 0);//上报进度及错误信息
						}
						#endif
						#endif
					}
					//关闭所有红外LED,以降低功耗
					FAR_LAN_OFF();
					NEAR_LAN_OFF();
					GPIO_ResetBits(PORT_IR_CTRL,PIN_IR_CTRL);
					GPIO_ResetBits(GPIOD,GPIO_Pin_14);	//低电平，关灯
					//GPIO_ResetBits(PORT_IR_CTRL2,PIN_IR_CTRL2); //低电平，关灯
					#if 1//单功能测试状态下,此状态转换需要关闭一下
					bdtest_sta = TPC_IRDA_CHRG;
					bdtest_sta_child = 0;
					#endif
					break;
				case 1:
				default:
					bdtest_sta_child = 0;
					bdtest_sta = TPC_IRDA_CHRG;
					break;
			}
			break;
		case TPC_IRDA_CHRG:
			switch(bdtest_sta_child)
			{
			case 0:
				{
					////uint8_t n_irdata[3] = {0};

					////read_ir_original_data(n_irdata);//读一次红外,把之前的值清掉
				}
				bdtest_sta_child = 1;
				bd_batcharge_timer = 0;//把变量临时借过来用一下,用来做接收不到红外信号的计数
				break;
			case 1:
				{
					uint8_t n_irdata[4] = {0};
					uint8_t n_irres = 0;
					//uint8_t n_irknk = 0;
					//uint8_t i,cc;
					read_ir_original_data(n_irdata);
					//read_ir_data(n_irdata,0);
			#if BDTEST_LOGPRINT
					log_printf("\r\n[wywy]Dock Ir:(%d)%d,%d,%d\r\n",
					bd_batcharge_timer,
					n_irdata[0],n_irdata[1],n_irdata[2]);
			#endif
					//n_irres = (n_irdata[0] | n_irdata[1] | n_irdata[2]) & 0x01;//取顶灯信号
					//n_irres = (n_irdata[0] | n_irdata[1] | n_irdata[2] | n_irdata[3]);//取顶灯信号
					//n_irres ^= 0x01;
					if((n_irdata[0]&0x1f) == 0)	n_irres |= 0x01;
					if((n_irdata[1]&0x1f) == 0)	n_irres |= 0x02;
					if((n_irdata[2]&0x1f) == 0)	n_irres |= 0x04;
					if((n_irdata[3]&0x1f) == 0)	n_irres |= 0x08;
					
					if((n_irdata[0]&0xe0) != 0xe0)n_irres |= 0x01;
					if((n_irdata[1]&0xe0) != 0xe0)n_irres |= 0x02;
					if((n_irdata[2]&0xe0) != 0xe0)n_irres |= 0x04;
					if((n_irdata[3]&0xe0) != 0xe0)n_irres |= 0x08;

					#if 0
					log_printf("\r\n %02x %02x %02x res:%02x\r\n",n_irdata[0],n_irdata[1],n_irdata[2],n_irres);
					//log_printf("\r\n ---%02x---\r\n",n_irdata[0]);
					delay_ms(200);
					#else
					if(n_irres != 0 && bd_batcharge_timer ++ > 2)
					{
						#if 0
						log_printf("\r\n %02x %02x %02x res:%02x\r\n",n_irdata[0],n_irdata[1],n_irdata[2],n_irres);
						//log_printf("\r\n ---%02x---\r\n",n_irdata[0]);
						delay_ms(200);
						#else
						bd_batcharge_timer = 0;
						bdtest_sta_child = 0;
						bdtest_sta = TPC_BUM_SW;
						tx_com_dat(TPC_IRDA_CHRG, 100 , (n_irres!=0) ? (TPC_ERR_IRLED):(TPC_ERR_NOERROR), n_irres);//上报进度及错误信息
						#endif
					}
					else if(n_irres == 0)
					{
						bd_batcharge_timer = 0;
						bdtest_sta_child = 0;
						bdtest_sta = TPC_BUM_SW;
						tx_com_dat(TPC_IRDA_CHRG, 100 , (n_irres!=0) ? (TPC_ERR_IRLED):(TPC_ERR_NOERROR), n_irres);//上报进度及错误信息
					}
					#endif
				}
				#if 1//单功能测试状态下,此状态转换需要关闭一下
				#else
				bdtest_sta_child = 1;
				#endif
				break;
			case 2:
			default:
				break;
			}
			break;
		case TPC_BUM_SW://读取碰撞开关数据
			switch(bdtest_sta_child&0x0f)
			{
				case 0://读取光耦无光时的GPIO开关输入数据,如果是正常状态下,此时应该有碰撞(高电平)
					tx_dp_dat(TBD_BUM_SW, 0, 0);//左右碰撞放开
					delay(1000);
					{
						uint8_t n_res = 0;
						uint8_t i;

						////左1
						for(i=0;i<20;i++)
						{
							if(((GPIOD->IDR) & GPIO_Pin_0)!=0)
								break;
						}
						if(i==20)
							n_res |= 0x01;

						//左2
						for(i=0;i<20;i++)
						{
							if(((GPIOE->IDR) & GPIO_Pin_12) !=0)
								break;
						}
						if(i==20)
							n_res |= 0x02;

						//右1
						for(i=0;i<20;i++)
						{
							if(((GPIOE->IDR) & GPIO_Pin_5) !=0)
								break;
						}
						if(i==20)
							n_res |= 0x04;

						//右2
						for(i=0;i<20;i++)
						{
							if(((GPIOE->IDR) & GPIO_Pin_6) !=0)
								break;
						}
						if(i==20)
							n_res |= 0x08;

			#if BDTEST_LOGPRINT
						log_printf("\r\n[wywy]bum:(%d)%d-%d-%d-%d\r\n",bdtest_sta_child,
						((GPIOD->IDR) & GPIO_Pin_0),
						((GPIOE->IDR) & GPIO_Pin_12),
						((GPIOE->IDR) & GPIO_Pin_5),
						((GPIOE->IDR) & GPIO_Pin_6));
			#endif
						#if 0
						log_printf("\r\n double bum:%02x\r\n",n_res);
						delay_ms(100);
						#endif
						if(n_res)
						{
							tx_com_dat(TPC_BUM_SW, 25 , TPC_ERR_DIGI_IN, n_res);//上报进度及错误信息
						}
						else
						{
							tx_com_dat(TPC_BUM_SW, 20 , TPC_ERR_NOERROR, 0);//上报进度
						}
					}

					bdtest_sta_child = 1;
					tx_dp_dat(TBD_BUM_SW, 1, 0x01);//左碰撞短接到地
					bd_timer = 0;
					break;
				case 1:
					if(bd_timer ++ < 20)break;//超时未收到数据
					bd_timer = 0;

					if(1)////(tmd_rxbd_dat.bum_sw == 0x01)
					{
						uint8_t i;
						uint8_t n_res = 0;

						////左1
						for(i=0;i<20;i++)
						{
							if(((GPIOD->IDR) & GPIO_Pin_0)==0)
								break;
						}
						if(i==20)
							n_res |= 0x01;

						//左2
						for(i=0;i<20;i++)
						{
							if(((GPIOE->IDR) & GPIO_Pin_12) !=0)
								break;
						}
						if(i==20)
							n_res |= 0x02;

						//右1
						for(i=0;i<20;i++)
						{
							if(((GPIOE->IDR) & GPIO_Pin_5) !=0)
								break;
						}
						if(i==20)
							n_res |= 0x04;

						//右2
						for(i=0;i<20;i++)
						{
							if(((GPIOE->IDR) & GPIO_Pin_6) !=0)
								break;
						}
						if(i==20)
							n_res |= 0x08;
			#if BDTEST_LOGPRINT
						log_printf("\r\n[wywy]bum:(%d)%d-%d-%d-%d\r\n",bdtest_sta_child,
						((GPIOD->IDR) & GPIO_Pin_0),
						((GPIOE->IDR) & GPIO_Pin_12),
						((GPIOE->IDR) & GPIO_Pin_5),
						((GPIOE->IDR) & GPIO_Pin_6));
			#endif
						#if 0
						log_printf("\r\nleft bum:%02x\tled:%d\r\n",n_res,tmd_rxbd_dat.bum_sw_led);
						delay_ms(100);
						#endif
						if(n_res)
						{
							tx_com_dat(TPC_BUM_SW, 40 , TPC_ERR_DIGI_IN, n_res);//上报进度及错误信息
						}
						else
						{
							tx_com_dat(TPC_BUM_SW, 40 , TPC_ERR_NOERROR, 0);//上报进度
						}
					}
					//else
					//	tx_com_dat(TPC_BUM_SW, 50, TPC_ERR_TB, 0);//上报进度或上报过流检测错误

					tx_dp_dat(TBD_BUM_SW, 1, 0x02);//左碰撞放开,右碰撞短接到地
					bdtest_sta_child = 2;
					break;
				case 2:
					if(bd_timer ++ < 20)break;//超时未收到数据
					bd_timer = 0;

					if(1)////(tmd_rxbd_dat.bum_sw == 0x02)
					{
						uint8_t i;
						uint8_t n_res = 0;

						////左1
						for(i=0;i<20;i++)
						{
							if(((GPIOD->IDR) & GPIO_Pin_0)!=0)
								break;
						}
						if(i==20)
							n_res |= 0x01;

						//左2
						for(i=0;i<20;i++)
						{
							if(((GPIOE->IDR) & GPIO_Pin_12) ==0)
								break;
						}
						if(i==20)
							n_res |= 0x02;

						//右1
						for(i=0;i<20;i++)
						{
							if(((GPIOE->IDR) & GPIO_Pin_5) !=0)
								break;
						}
						if(i==20)
							n_res |= 0x04;

						//右2
						for(i=0;i<20;i++)
						{
							if(((GPIOE->IDR) & GPIO_Pin_6) !=0)
								break;
						}
						if(i==20)
							n_res |= 0x08;
			#if BDTEST_LOGPRINT
						log_printf("\r\n[wywy]bum:(%d)%d-%d-%d-%d\r\n",bdtest_sta_child,
						((GPIOD->IDR) & GPIO_Pin_0),
						((GPIOE->IDR) & GPIO_Pin_12),
						((GPIOE->IDR) & GPIO_Pin_5),
						((GPIOE->IDR) & GPIO_Pin_6));
			#endif
						#if 0
						log_printf("\r\nright bum:%02x\r\n",n_res);
						delay_ms(100);
						#endif
						if(n_res)
						{
							tx_com_dat(TPC_BUM_SW, 60 , TPC_ERR_DIGI_IN, n_res);//上报进度及错误信息
						}
						else
						{
							tx_com_dat(TPC_BUM_SW, 60 , TPC_ERR_NOERROR, 0);//上报进度
						}
					}
					else
						tx_com_dat(TPC_BUM_SW, 60, TPC_ERR_TB, 0);//上报进度或上报过流检测错误

					tx_dp_dat(TBD_BUM_SW, 1, 0x04);
					bdtest_sta_child = 3;
					break;
				case 3:
					if(bd_timer ++ < 20)break;//超时未收到数据
					bd_timer = 0;

					if(1)////(tmd_rxbd_dat.bum_sw == 0x04)
					{
						uint8_t i;
						uint8_t n_res = 0;

						////左1
						for(i=0;i<20;i++)
						{
							if(((GPIOD->IDR) & GPIO_Pin_0) !=0)
								break;
						}
						if(i==20)
							n_res |= 0x01;

						//左2
						for(i=0;i<20;i++)
						{
							if(((GPIOE->IDR) & GPIO_Pin_12) !=0)
								break;
						}
						if(i==20)
							n_res |= 0x02;

						//右1
						for(i=0;i<20;i++)
						{
							if(((GPIOE->IDR) & GPIO_Pin_5) ==0)
								break;
						}
						if(i==20)
							n_res |= 0x04;

						//右2
						for(i=0;i<20;i++)
						{
							if(((GPIOE->IDR) & GPIO_Pin_6) !=0)
								break;
						}
						if(i==20)
							n_res |= 0x08;
			#if BDTEST_LOGPRINT
						log_printf("\r\n[wywy]bum:(%d)%d-%d-%d-%d\r\n",bdtest_sta_child,
						((GPIOD->IDR) & GPIO_Pin_0),
						((GPIOE->IDR) & GPIO_Pin_12),
						((GPIOE->IDR) & GPIO_Pin_5),
						((GPIOE->IDR) & GPIO_Pin_6));
			#endif
						#if 0
						log_printf("\r\nright bum:%02x\r\n",n_res);
						delay_ms(100);
						#endif
						if(n_res)
						{
							tx_com_dat(TPC_BUM_SW, 80 , TPC_ERR_DIGI_IN, n_res);//上报进度及错误信息
						}
						else
						{
							tx_com_dat(TPC_BUM_SW, 80 , TPC_ERR_NOERROR, 0);//上报进度
						}
					}
					else
						tx_com_dat(TPC_BUM_SW, 80, TPC_ERR_TB, 0);//上报进度或上报过流检测错误

					tx_dp_dat(TBD_BUM_SW, 1, 0x08);//左右碰撞放开
					bdtest_sta_child = 4;
					break;
				case 4:
					if(bd_timer ++ < 20)break;//超时未收到数据
					bd_timer = 0;

					if(1)////(tmd_rxbd_dat.bum_sw == 0x08)
					{
						uint8_t i;
						uint8_t n_res = 0;

						////左1
						for(i=0;i<20;i++)
						{
							if(((GPIOD->IDR) & GPIO_Pin_0) !=0)
								break;
						}
						if(i==20)
							n_res |= 0x01;

						//左2
						for(i=0;i<20;i++)
						{
							if(((GPIOE->IDR) & GPIO_Pin_12) !=0)
								break;
						}
						if(i==20)
							n_res |= 0x02;

						//右1
						for(i=0;i<20;i++)
						{
							if(((GPIOE->IDR) & GPIO_Pin_5) !=0)
								break;
						}
						if(i==20)
							n_res |= 0x04;

						//右2
						for(i=0;i<20;i++)
						{
							if(((GPIOE->IDR) & GPIO_Pin_6) ==0)
								break;
						}
						if(i==20)
							n_res |= 0x08;
			#if BDTEST_LOGPRINT
						log_printf("\r\n[wywy]bum:(%d)%d-%d-%d-%d\r\n",bdtest_sta_child,
						((GPIOD->IDR) & GPIO_Pin_0),
						((GPIOE->IDR) & GPIO_Pin_12),
						((GPIOE->IDR) & GPIO_Pin_5),
						((GPIOE->IDR) & GPIO_Pin_6));
			#endif
						#if 0
						log_printf("\r\nright bum:%02x\r\n",n_res);
						delay_ms(100);
						#endif
						if(n_res)
						{
							tx_com_dat(TPC_BUM_SW, 100 , TPC_ERR_DIGI_IN, n_res);//上报进度及错误信息
						}
						else
						{
							tx_com_dat(TPC_BUM_SW, 100 , TPC_ERR_NOERROR, 0);//上报进度
						}
					}
					else
						tx_com_dat(TPC_BUM_SW, 100, TPC_ERR_TB, 0);//上报进度或上报过流检测错误

					tx_dp_dat(TBD_BUM_SW, 0, 0);//左右碰撞放开
				#if 0
					bdtest_sta_child = 3;
					break;
				case 3:
					break;
				#endif
				default:
					bdtest_sta = TPC_SWSENSOR;
					bdtest_sta_child = 0;
					bdtest_fantest = 0;
					break;
			}
			break;
		case TPC_SWSENSOR://读取其他开关式传感器数据
			switch(bdtest_sta_child&0x0f)
			{
				case 0://读取未被短接到地时的各传感器数据
					test_mode_ul(SET);
					delay_ms_tm5(500);
					SCLL;
					{
						uint8_t n_res = 0;
						uint8_t i;

						for(i=0;i<20;i++)//超声波
						{
							if(READ_DATA() == 0)//放开时应为高电平,如果检测到低电平就判定为检测异常
								break;
						}
						if(i==20)
							n_res |= 0x01;

						for(i=0;i<20;i++)//虚拟墙,宏定义是反的...
						{
							if(READ_VWALL_DET() == 0)//放开时应为高电平,如果检测到低电平就判定为检测异常
								break;
						}
						if(i==20)
							n_res |= 0x02;

						for(i=0;i<20;i++)//左落轮
						{
							if(LEFT_MOTOR_LEAVE() != 0)//放开时应为高电平,如果检测到低电平就判定为检测异常
								break;
						}
						if(i==20)
							n_res |= 0x10;

						for(i=0;i<20;i++)//右落轮
						{
							if(RIGHT_MOTOR_LEAVE() != 0)//放开时应为高电平,如果检测到低电平就判定为检测异常
								break;
						}
						if(i==20)
							n_res |= 0x20;
						#if BDTEST_LOGPRINT
						log_printf("\r\n[wywy]qi ta:(%d)%d,%d,%d-%d\r\n",bdtest_sta_child,
						READ_DATA(),
						READ_VWALL_DET(),
						LEFT_MOTOR_LEAVE(),
						RIGHT_MOTOR_LEAVE());
						log_printf("\r\n case 0:%02x\r\n",n_res);
						delay_ms(100);
						#else
						if(n_res)
						{
							tx_com_dat(TPC_SWSENSOR, 25 , TPC_ERR_DIGI_IN, n_res);//上报进度及错误信息
						}
						else
						{
							tx_com_dat(TPC_SWSENSOR, 25 , TPC_ERR_NOERROR, 0);//上报进度
						}
						#endif
					}

					////tx_dp_dat(TBD_SWSENSOR, 1, 0x01);//超声波
					SCLH;
					bd_timer = 0;
					bdtest_sta_child = 1;
					break;
				case 1:
					if(bd_timer ++ < 20)break;//超时未收到数据
					bd_timer = 0;

					if(1)
					{
						uint8_t n_res = 0;
						uint8_t i;

						for(i=0;i<20;i++)//超声波
						{
							if(READ_DATA() != 0)//放开时应为高电平,如果检测到低电平就判定为检测异常
								break;
						}
						if(i==20)
							n_res |= 0x01;

						for(i=0;i<20;i++)//虚拟墙
						{
							if(READ_VWALL_DET() == 0)//放开时应为高电平,如果检测到低电平就判定为检测异常
								break;
						}
						if(i==20)
							n_res |= 0x02;

						for(i=0;i<20;i++)//左落轮
						{
							if(LEFT_MOTOR_LEAVE() != 0)//放开时应为高电平,如果检测到低电平就判定为检测异常
								break;
						}
						if(i==20)
							n_res |= 0x10;

						for(i=0;i<20;i++)//右落轮
						{
							if(RIGHT_MOTOR_LEAVE() != 0)//放开时应为高电平,如果检测到低电平就判定为检测异常
								break;
						}
						if(i==20)
							n_res |= 0x20;
						#if BDTEST_LOGPRINT
						log_printf("\r\n[wywy]qi ta:(%d)%d,%d,%d-%d\r\n",bdtest_sta_child,
						READ_DATA(),
						READ_VWALL_DET(),
						LEFT_MOTOR_LEAVE(),
						RIGHT_MOTOR_LEAVE());
						log_printf("\r\n case 2:%02x\r\n",n_res);
						delay_ms(100);
						#else
						if(n_res)
						{
							tx_com_dat(TPC_SWSENSOR, 45 , TPC_ERR_DIGI_IN, n_res);//上报进度及错误信息
						}
						else
						{
							tx_com_dat(TPC_SWSENSOR, 45 , TPC_ERR_NOERROR, 0);//上报进度
						}
						#endif
					}

					tx_dp_dat(TBD_SWSENSOR, 1, 0x02);//虚拟墙
					SCLL;
					bd_timer = 0;
					bdtest_sta_child = 2;
					break;
				case 2:
					//bdtest_sta_child = (bdtest_sta_child & 0xf0) + 3;
					if(bd_timer ++ < 20)break;//超时未收到数据
					bd_timer = 0;

					if(1)////(tmd_rxbd_dat.sensors_sw== 0x02)
					{
						uint8_t n_res = 0;
						uint8_t i;

						for(i=0;i<20;i++)//超声波
						{
							if(READ_DATA() == 0)//放开时应为高电平,如果检测到低电平就判定为检测异常
								break;
						}
						if(i==20)
							n_res |= 0x01;

						for(i=0;i<20;i++)//虚拟墙
						{
							if(READ_VWALL_DET() != 0)//放开时应为高电平,如果检测到低电平就判定为检测异常
								break;
						}
						if(i==20)
							n_res |= 0x02;

						for(i=0;i<20;i++)//左落轮
						{
							if(LEFT_MOTOR_LEAVE() != 0)//放开时应为高电平,如果检测到低电平就判定为检测异常
								break;
						}
						if(i==20)
							n_res |= 0x10;

						for(i=0;i<20;i++)//右落轮
						{
							if(RIGHT_MOTOR_LEAVE() != 0)//放开时应为高电平,如果检测到低电平就判定为检测异常
								break;
						}
						if(i==20)
							n_res |= 0x20;
						#if BDTEST_LOGPRINT
						log_printf("\r\n[wywy]qi ta:(%d)%d,%d,%d-%d\r\n",bdtest_sta_child,
						READ_DATA(),
						READ_VWALL_DET(),
						LEFT_MOTOR_LEAVE(),
						RIGHT_MOTOR_LEAVE());
						log_printf("\r\n case 2:%02x\r\n",n_res);
						delay_ms(100);
						#else
						if(n_res)
						{
							tx_com_dat(TPC_SWSENSOR, 60 , TPC_ERR_DIGI_IN, n_res);//上报进度及错误信息
						}
						else
						{
							tx_com_dat(TPC_SWSENSOR, 60 , TPC_ERR_NOERROR, 0);//上报进度
						}
						#endif
					}
					else
						tx_com_dat(TPC_SWSENSOR, 60, TPC_ERR_TB, tmd_rxbd_dat.sensors_sw);//上报进度或上报过流检测错误

					tx_dp_dat(TBD_SWSENSOR, 1, 0x10);//左落轮检测短拉到地
					bd_timer = 0;
					bdtest_sta_child = 3;
					break;
				case 3:
					//bdtest_sta_child = (bdtest_sta_child & 0xf0) + 6;
					if(bd_timer ++ < 20)break;//超时未收到数据
					bd_timer = 0;

					if(1)////(tmd_rxbd_dat.sensors_sw== 0x10)
					{
						uint8_t n_res = 0;
						uint8_t i;

						for(i=0;i<20;i++)//超声波
						{
							if(READ_DATA() == 0)//放开时应为高电平,如果检测到低电平就判定为检测异常
								break;
						}
						if(i==20)
							n_res |= 0x01;

						for(i=0;i<20;i++)//虚拟墙
						{
							if(READ_VWALL_DET() == 0)//放开时应为高电平,如果检测到低电平就判定为检测异常
								break;
						}
						if(i==20)
							n_res |= 0x02;

						for(i=0;i<20;i++)//左落轮
						{
							if(LEFT_MOTOR_LEAVE() == 0)//放开时应为高电平,如果检测到低电平就判定为检测异常
								break;
						}
						if(i==20)
							n_res |= 0x10;

						for(i=0;i<20;i++)//右落轮
						{
							if(RIGHT_MOTOR_LEAVE() != 0)//放开时应为高电平,如果检测到低电平就判定为检测异常
								break;
						}
						if(i==20)
							n_res |= 0x20;
						#if BDTEST_LOGPRINT
						log_printf("\r\n[wywy]qi ta:(%d)%d,%d,%d-%d\r\n",bdtest_sta_child,
						READ_DATA(),
						READ_VWALL_DET(),
						LEFT_MOTOR_LEAVE(),
						RIGHT_MOTOR_LEAVE());
						log_printf("\r\n case 5:%02x\r\n",n_res);
						delay_ms(100);
						#else
						if(n_res)
						{
							tx_com_dat(TPC_SWSENSOR, 90 , TPC_ERR_DIGI_IN, n_res);//上报进度及错误信息
						}
						else
						{
							tx_com_dat(TPC_SWSENSOR, 90 , TPC_ERR_NOERROR, 0);//上报进度
						}
						#endif
					}
					else
						tx_com_dat(TPC_SWSENSOR, 90, TPC_ERR_TB, tmd_rxbd_dat.sensors_sw);//上报进度或上报过流检测错误

					tx_dp_dat(TBD_SWSENSOR, 1, 0x20);//右落轮检测短拉到地
					bd_timer = 0;
					bdtest_sta_child = 4;
					break;
				case 4:
					//bdtest_sta_child = (bdtest_sta_child & 0xf0);
					if(bd_timer ++ < 20)break;//超时未收到数据
					bd_timer = 0;

					if(1)////(tmd_rxbd_dat.sensors_sw== 0x20)
					{
						uint8_t n_res = 0;
						uint8_t i;

						for(i=0;i<20;i++)//超声波
						{
							if(READ_DATA() == 0)//放开时应为高电平,如果检测到低电平就判定为检测异常
								break;
						}
						if(i==20)
							n_res |= 0x01;

						for(i=0;i<20;i++)//虚拟墙
						{
							if(READ_VWALL_DET() == 0)//放开时应为高电平,如果检测到低电平就判定为检测异常
								break;
						}
						if(i==20)
							n_res |= 0x02;

						for(i=0;i<20;i++)//左落轮
						{
							if(LEFT_MOTOR_LEAVE() != 0)//放开时应为高电平,如果检测到低电平就判定为检测异常
								break;
						}
						if(i==20)
							n_res |= 0x10;

						for(i=0;i<20;i++)//右落轮
						{
							if(RIGHT_MOTOR_LEAVE() == 0)//放开时应为高电平,如果检测到低电平就判定为检测异常
								break;
						}
						if(i==20)
							n_res |= 0x20;
						#if BDTEST_LOGPRINT
						log_printf("\r\n[wywy]qi ta:(%d)%d,%d,%d-%d\r\n",bdtest_sta_child,
						READ_DATA(),
						READ_VWALL_DET(),
						LEFT_MOTOR_LEAVE(),
						RIGHT_MOTOR_LEAVE());
						log_printf("\r\n case 6:%02x\r\n",n_res);
						delay_ms(100);
						#else
						if(n_res)
						{
							tx_com_dat(TPC_SWSENSOR, 100 , TPC_ERR_DIGI_IN, n_res);//上报进度及错误信息
						}
						else
						{
							tx_com_dat(TPC_SWSENSOR, 100 , TPC_ERR_NOERROR, 0);//上报进度
						}
						#endif
					}
					else
						tx_com_dat(TPC_SWSENSOR, 100, TPC_ERR_TB, tmd_rxbd_dat.sensors_sw);//上报进度或上报过流检测错误

					tx_dp_dat(TBD_SWSENSOR, 0, 0);//虚拟墙检测短拉到地
					bd_timer = 0;
					bdtest_sta_child = 0;
				#if BDTEST_LOGPRINT
					bdtest_sta_child = 7;
					break;
				case 7:
					break;
				#endif
				default:
					test_mode_ul(RESET);
					bdtest_sta = TPC_CHRG_TST;
					bdtest_sta_child = 0;
					MOTOR_POWER_OFF_NPRI();
					bdtest_fantest = 0;
					break;
			}
			break;
		case TPC_CHRG_TST://充电测试
			switch(bdtest_sta_child&0x0f)
			{
				#if 1
				case 0:
					tx_dp_dat(TBD_CHRG_TST, 3, 0);//将24V打到DC插座接口
					bdtest_sta_child = 1;
					break;
				case 1://下发给测试板,要求测试板将电池的相关脚接入到主板上
					bdtest_sta_child = 2;
					//先采样电池的温度,看电池是否存在或过温,检测DC座及充电座的信号脚是否正常
					{
						uint16_t n_bat_temp = LiBat_GetBatTemper();//获取电池温度
						uint8_t n_error_code = TPC_ERR_NOERROR;
						#if 0
						if(n_bat_temp == 1500)//电池不存在,退出测试模式
						{
							//bdtest_sta_child = 0;
							//bdtest_sta = TPC_STOP_ALL;
							n_error_code = TPC_ERR_BATNE;
							//break;
						}
						else if(/*n_bat_temp < 0 || */n_bat_temp > 400)//电池过温,退出测试模式
						{
							//bdtest_sta_child = 0;
							//bdtest_sta = TPC_STOP_ALL;
							n_error_code = TPC_ERR_BATOT;
							//break;
						}
						#else
						n_bat_temp = 200;
						#endif
						//检测充电座或DC插座的电路是否正常,未接入的状态下,均应为低电平
						if(DOCK_DETECT())
						{
							//bdtest_sta_child = 0;
							//bdtest_sta = TPC_STOP_ALL;
							n_error_code = TPC_ERR_DOCK;
							//break;
						}
						if(EXTERAL_AC_DETECT())
						{
							n_error_code = TPC_ERR_DCJACK;
							//break;
						}

						#if BDTEST_LOGPRINT
						log_printf("\r\n 1:%d,DOCK:%d-DC:%d\r\n",n_error_code,DOCK_DETECT(),EXTERAL_AC_DETECT());
						#endif
						delay_ms(100);
						tx_com_dat(TPC_CHRG_TST, 25 , n_error_code, 0);
						if(n_error_code != TPC_ERR_NOERROR)
						{
							bdtest_sta_child = 0;
							bdtest_sta = TPC_GYRO_TST;
							//bdtest_sta_child = 4;
							break;
						}
					}
					tmd_rxbd_dat.unread_flag = 1;
					tx_dp_dat(TBD_CHRG_TST, 1, 0);//将24V打到DC插座接口
					bd_timer = 0;
					break;
				case 2://下发给测试板,要求测试板将电池的相关脚接入到主板上
					if(bd_timer ++ < 20 && !tmd_rxbd_dat.unread_flag)break;//超时未收到数据
					bd_timer = 0;
					if(!tmd_rxbd_dat.unread_flag)//超时未收到数据,与测试板(显控端口)通信失败
					{
						tx_com_dat(TPC_DPPORT_TST, 0, TPC_ERR_COMM, 0);//数据通信错误
						tx_dp_dat(TBD_CHRG_TST, 1, 0);//将24V打到DC插座接口
						bdtest_sta_child = (bdtest_sta_child & 0xf0) + 2;
						bdtest_sta_child += 0x10;
						if(bdtest_sta_child > 0x50)
							bdtest_sta_child = 0x0f;
						//tmd_rxbd_dat.unread_flag = 0;
						//bdtest_sta_child = 4;
						break;
					}
					else
					{
						uint8_t n_error_code = TPC_ERR_NOERROR;

						delay_ms(200);
						tmd_rxbd_dat.unread_flag = 0;
						
						if(DOCK_DETECT())//插座信号检测错误
							n_error_code = TPC_ERR_DOCK;
						if(EXTERAL_AC_DETECT() == 0)
							n_error_code = TPC_ERR_DCJACK;
						
						//log_printf("\r\n 1:%d,DOCK:%d-DC:%d\r\n",n_error_code,DOCK_DETECT(),EXTERAL_AC_DETECT());
						//delay_ms(100);
						tx_com_dat(TPC_CHRG_TST, 50 , n_error_code, 0);
					}
					bdtest_sta_child = 3;
					tx_dp_dat(TBD_CHRG_TST, 2, 0);//将24V打到充电座触片接口
					bd_timer = 0;
					break;
				case 3://检测DC插座信号检测是否正常
					if(bd_timer ++ < 20 && !tmd_rxbd_dat.unread_flag)break;//超时未收到数据
					bd_timer = 0;
					if(!tmd_rxbd_dat.unread_flag)//超时未收到数据,与测试板(显控端口)通信失败
					{
						tx_com_dat(TPC_DPPORT_TST, 0, TPC_ERR_COMM, 0);//数据通信错误
						//tx_dp_dat(TBD_CHRG_TST, 2, 0);//将24V打到DC插座接口
						bdtest_sta_child = (bdtest_sta_child & 0xf0) + 3;
						bdtest_sta_child += 0x10;
						if(bdtest_sta_child > 0x50)
							bdtest_sta_child = 0x0f;
						//tmd_rxbd_dat.unread_flag = 0;
						//bdtest_sta_child = 4;
						break;
					}
					else
					{
						uint8_t n_error_code = TPC_ERR_NOERROR;
						
						delay_ms(200);
						tmd_rxbd_dat.unread_flag = 0;
						
						if(DOCK_DETECT() == 0)//插座信号检测错误
							n_error_code = TPC_ERR_DOCK;
						if(EXTERAL_AC_DETECT())
							n_error_code = TPC_ERR_DCJACK;
						
						//log_printf("\r\n 2:%d\r\n",n_error_code);
						//log_printf("\r\n 1:%d,DOCK:%d-DC:%d\r\n",n_error_code,DOCK_DETECT(),EXTERAL_AC_DETECT());
						//delay_ms(100);
						tx_com_dat(TPC_CHRG_TST, 100 , n_error_code, 0);
						//LiBat_HalInit();
//						bdtest_chgfailed_counter = 0xff;
//						bdtest_chgsucceed_counter = 0xff;
//						bdtest_chgstart_counter = 0;
						bd_batcharge_timer = 0;
						//delay_ms(500);
					}
					
					tx_dp_dat(TBD_CHRG_TST, 0, 0);//将24V打到DC插座接口
					bdtest_sta_child = 0;//0
					//log_printf("\r\nLB_CHS_ERROR:%d\r\n",n_charge_res);
					bdtest_sta = TPC_GYRO_TST;
					//bdtest_sta_child = 4;
					bd_timer = 0;
					break;
				#if 0
				case 4:
					log_printf("^");
					delay_ms(1000);
					break;
				#endif
				default:
					bdtest_sta = TPC_GYRO_TST;
					bdtest_sta_child = 0;
					MOTOR_POWER_OFF_NPRI();
					bdtest_fantest = 0;
					break;
				#else
				case 0:
					bdtest_sta_child = 1;
					tx_com_dat(TPC_CHRG_TST, 100, TPC_ERR_NOERROR, 0);//上报进度
					break;
					if(DOCK_DETECT())
					{				
						if(CHARGE_DONE_DETECT() == 0 || CHARGE_ING_DETECT() == 0)	
							tx_com_dat(TPC_CHRG_TST, 100, TPC_ERR_NOERROR, 0);//上报进度
						else
							tx_com_dat(TPC_CHRG_TST, 100, TPC_ERR_DOCK, 0);//上报进度
					}	
					else
					{
						tx_com_dat(TPC_CHRG_TST, 100, TPC_ERR_DCJACK, 0);//上报进度				
					}
					log_printf("\r\n[wywy]chong dian:%d\r\n",DOCK_DETECT());
					break;
				default:
					bdtest_sta = TPC_GYRO_TST;
					bdtest_sta_child = 0;
					MOTOR_POWER_OFF_NPRI();
					bdtest_fantest = 0;
					break;
				#endif
			}
			break;
		case TPC_GYRO_TST:
			switch(bdtest_sta_child)
			{
			case 0:
				#if 0
				{
					uint8_t n_gyro_res = 0;
					uint8_t n_err_res = TPC_ERR_DIGI_IN;
					SPI_CS(0);
					SPI1_ReadWriteByte(0xB5);
					n_gyro_res = SPI1_ReadWriteByte(0);
					SPI_CS(1);

					if(n_gyro_res == 0x70)//功能正常
					{
						n_err_res = TPC_ERR_NOERROR;
					}
					else if(n_gyro_res == 0)//陀螺仪板异常或未插
					{
						n_err_res = TPC_ERR_COMM;//数据通信异常
					}
					else if(n_gyro_res == 0x7A)//陀螺仪IC异常
					{
						n_err_res = TPC_ERR_DIGI_IN;//数据通信异常
					}
					else
					{
						n_err_res = TPC_ERR_DIGI_OUT;//数据通信异常
					}
					#if 0
					bdtest_sta_child = 1;
					log_printf("n_gyro_res:%02x\t err:%d\r\n",n_gyro_res,n_err_res);
					#else
					//bdtest_sta = TPC_STOP_ALL;
					tx_com_dat(TPC_GYRO_TST, 100 , n_err_res, 0);//上发测试完成
					tx_com_dat(TPC_STOP_ALL, 100 , TPC_ERR_NOERROR, 0);//上发测试完成
					tx_dp_dat(TBD_STOP_ALL, 0, 0);//将24V打到DC插座接口
					#endif
				}
				#else
				{
					if(sys->gyro_init_ok)
						tx_com_dat(TPC_GYRO_TST, 100, TPC_ERR_NOERROR, 0);//上报进度
					else
						tx_com_dat(TPC_GYRO_TST, 50, TPC_ERR_TYRO, 1);//上报进度
					bdtest_sta = TPC_CT_KEY;
					bdtest_sta_child = 0;
				}
				#endif
			break;
			case 1:
				break;
			default:
				break;
			}
			break;
		//case TPC_IDLE:
				
		//	break;
		case TPC_CT_KEY:
			switch(bdtest_sta_child)
			{
				case 0:
					tx_dp_dat(TBD_KEY, 0, 0);
					delay(1000);
					key_err = 0;
					if(0 != KEY_RD_SPOT())		//定点清扫
					{
						key_err |= 0x04;
					}

					if(0 != KEY_RD_WIFI())		//调风机
					{
						key_err |= 0x08;
					}
					#if BDTEST_LOGPRINT
					log_printf("\r\n[wywy]key:(%d)%d-%d\r\n",bdtest_sta_child,
					KEY_RD_SPOT(),
					KEY_RD_WIFI());
					#endif
					//key_err |= r_key_err;
					if(0 != key_err)
					{
						tx_com_dat(TPC_CT_KEY, 30, TPC_ERR_DIGI_IN, key_err);//上报进度
						////bdtest_sta_child = 3;
					}
					else
					{
						tx_com_dat(TPC_CT_KEY, 30, TPC_ERR_NOERROR, 0);//上报进度
						////tx_dp_dat(TBD_KEY, 1, 0x04);
						////bdtest_sta_child = 1;
						////delay(500);
					}
					tx_dp_dat(TBD_KEY, 1, 0x04);
					bdtest_sta_child = 1;
					delay(1000);
				break;
				case 1:
					key_err = 0;
					if(0 == KEY_RD_SPOT())		//定点清扫
					{
						key_err |= 0x04;
					}

					if(0 != KEY_RD_WIFI())		//调风机
					{
						key_err |= 0x08;
					}
					#if BDTEST_LOGPRINT
					log_printf("\r\n[wywy]key:(%d)%d-%d\r\n",bdtest_sta_child,
					KEY_RD_SPOT(),
					KEY_RD_WIFI());
					#endif
					//key_err |= r_key_err;
					if(0 != key_err)
					{
						tx_com_dat(TPC_CT_KEY, 70, TPC_ERR_DIGI_IN, key_err);//上报进度
						////bdtest_sta_child = 3;
					}
					else
					{
						tx_com_dat(TPC_CT_KEY, 70, TPC_ERR_NOERROR, 0);//上报进度
						////tx_dp_dat(TBD_KEY, 1, 0x08);
						////bdtest_sta_child = 2;
						////delay(500);
					}
					tx_dp_dat(TBD_KEY, 1, 0x08);
					bdtest_sta_child = 2;
					delay(1000);
				break;
				case 2:
					key_err = 0;
					if(0 != KEY_RD_SPOT())		//定点清扫
					{
						key_err |= 0x04;
					}

					if(0 == KEY_RD_WIFI())		//调风机
					{
						key_err |= 0x08;
					}
					#if BDTEST_LOGPRINT
					log_printf("\r\n[wywy]key:(%d)%d-%d\r\n",bdtest_sta_child,
					KEY_RD_SPOT(),
					KEY_RD_WIFI());
					#endif
					//key_err |= r_key_err;
					if(0 != key_err)
						tx_com_dat(TPC_CT_KEY, 100, TPC_ERR_DIGI_IN, key_err);//上报进度
					else
						tx_com_dat(TPC_CT_KEY, 100, TPC_ERR_NOERROR, 0);//上报进度
					bdtest_sta_child = 3;
				break;
				default:
					tx_dp_dat(TBD_KEY, 0, 0);
					bdtest_sta_child = 0;
					bdtest_sta = TPC_CT_REMOTE;
				break;
			}
			break;
		case TPC_CT_REMOTE:
			//log_printf("\r\n[wywy]Remote;%d\r\n", test_bd_com);
			if(SUCCESS == test_bd_com)
			{
				tx_com_dat(TPC_CT_REMOTE, 100 , TPC_ERR_NOERROR, 0);
				tx_com_dat(TPC_COM_DISPLAY, 100 , TPC_ERR_NOERROR, 0);
			}
			else
			{
				tx_com_dat(TPC_CT_REMOTE, 100 , TPC_ERR_COMM, 0);
				tx_com_dat(TPC_COM_DISPLAY, 100 , TPC_ERR_COMM, 0);
			}
			bdtest_sta = TPC_STOP_ALL;
			bdtest_sta_child = 0;
			break;
		default:
			bdtest_sta = TPC_STOP_ALL;
			break;
	}
//	bdtest_sta_old = bdtest_sta;
}

//测试板的通信程序
void proc_bd_test_mode(uint8_t *buf,uint8_t len)
{
	TEST_BD_PROC *proc_dat;

	proc_dat = (TEST_BD_PROC *)buf;

	//data_print(buf,len);
	if(
#if TBD_STOP_ALL > 0
	proc_dat->code >= TBD_STOP_ALL &&
#endif
		proc_dat->code <= TBD_KEY)
		tmd_rxbd_dat.unread_flag = 1;//设置未读标签
	switch(proc_dat->code)
	{
		case TBD_STOP_ALL://退出测试模式,测试板将把主板的电源开关关闭,上位机软件如果检测到心跳包超时,则认为主板已顺利关闭,测试完成,如果还有心跳包上报就认为电源开关有问题
			tx_msg(buf,sizeof(TEST_PROC));
			TargetSysReset();
			break;
		case TBD_START_ALL://进入测试模式
			tx_msg(buf,sizeof(TEST_PROC));
			break;
		case TBD_MTOC_WHL_L:
			tmd_rxbd_dat.mtoc_whl_l = proc_dat->t_res_output;
			//log_printf("Get Data TBD_MTOC_WHL_L\r\n");
			break;
		case TBD_MTOC_WHL_R:
			tmd_rxbd_dat.mtoc_whl_r = proc_dat->t_res_output;
			break;
		case TBD_MTOC_SMT_L:
			tmd_rxbd_dat.mtoc_smt_l = proc_dat->t_res_output;
			break;
		case TBD_MTOC_SMT_R:
			tmd_rxbd_dat.mtoc_smt_r = proc_dat->t_res_output;
			break;
		case TBD_MTOC_MT_MID:
			tmd_rxbd_dat.mtoc_mt_mid = proc_dat->t_res_output;
			break;
		case TBD_SMT_L:
			tmd_rxbd_dat.cd_smt_l = proc_dat->t_res_input;
			//log_printf("\r\n cd_smt_l:%d-%d\r\n",tmd_rxbd_dat.cd_smt_l,proc_dat->t_res_input);
			break;
		case TBD_SMT_R:
			tmd_rxbd_dat.cd_smt_r = proc_dat->t_res_input;
			break;
		case TBD_MT_MID:
			tmd_rxbd_dat.cd_mt_mid = proc_dat->t_res_input;
			break;
		case TBD_BUM_SW:
			tmd_rxbd_dat.bum_sw = proc_dat->t_res_output;
			tmd_rxbd_dat.bum_sw_led = proc_dat->t_res_input;
			break;
		case TBD_SWSENSOR:
			tmd_rxbd_dat.sensors_sw = proc_dat->t_res_output;
			tmd_rxbd_dat.sensors_sw_led = proc_dat->t_res_input;
			break;
		default:
			break;
	}
}

//===============================================================================================================
//老化测试程序段

#define BURNIN_LOG_EN	0
#define BURNIN_EX_TIME	600//左右沿边的切换周期,单位为秒

static uint16_t burnin_work_near_timer;//单边延边的工作时间计数器
static uint16_t burnin_work_timer;//工作时间计数器,单位为秒,前20分钟为左沿边,后20分钟为右沿边,总共工作40分钟后流程结束,跳回到idle模式
static uint32_t bummech_ct,bumir_ct;//机械碰撞(即接触式碰撞)计数器 红外碰撞计数器
static uint32_t bummech_left_ct,bumir_left_ct;//左沿边计数器
static uint32_t bummech_right_ct,bumir_right_ct;//右沿边计数器

#if 1
//后退N霍尔
void burnin_back_off(int hw)
{
	//float dt;
//	int c;

	int i,tmr;
//	int dist=0
#if CALE_BY_BKHW
	int distance=motor.c_left_hw - hw * WHELE_HW;
#else
	int distance=navigat->distance - hw;
#endif

	CHECK_NAVI_STA();
	motor_run(GO_STOP,0,0,0);
	delay_ms(5);
	TIM5->CNT = 0;
	//motor.c_left_hw = 0;
	tmr = hw > 50 ?2000:500;

	motor_run(GO_BACK, BACK_OFF_PWM, 0, 0);
//	navigat->wheel_dir =-1;
	//navigat->is_walk = 1;

	i=0;

//	log_printf("befor bk,left(%3.1f,%3.1f)right(%3.1f,%3.1f)org(%d,%d)\r\n",navigat->x_org_f,navigat->y_org_f,navigat->x_org_r,,navigat->y_org_r,navigat->x_org,navigat->y_org);
	while(1)
	{

		if(TIM5->CNT >=10000)
		{
			TIM5->CNT = 0;
		//	www_idleintel_com();
			//log_printf("k");
			if(WALK_DIST()< distance || i++ >= tmr)
			{
				motor_run(GO_STOP,0,0,0);
				break;
			}
		}
	}
	//navigat->is_walk = 0;
	motor_run(GO_STOP,0,0,0);
	www_idleintel_com();
	//calc_gyro();
	burnin_coordinate_calcu(0);					//计算新的坐标
	//log_printf("c2\r\n");
	coord_org2map(navigat->x_org,navigat->y_org,&navigat->tx,&navigat->ty);	//坐标系转换
	//log_printf("o&");
	//gyro_whlmap();
	//log_printf("\r\n");
	//while(1);
//	log_printf("last bk,left(%3.1f,%3.1f)right(%3.1f,%3.1f)org(%d,%d)\r\n",navigat->x_org_f,navigat->y_org_f,navigat->x_org_r,,navigat->y_org_r,navigat->x_org,navigat->y_org);
	//log_printf("*(%d,%d,%f,0)-[%d,%d,%3.1f]\r\n\r\n",navigat->tx,navigat->ty,navigat->angle,navigat->x_org,navigat->y_org,sys->angle);
	//log_printf("hw=%d,x:%d,y%d,angle:%f,tx:%d,ty:%d\r\n",hw,navigat->x_org,navigat->y_org,navigat->angle,navigat->tx,navigat->ty);
	save_line_xy(navigat->x1_org,navigat->y1_org);
}


//计算是否360度转圈圈
uint8_t burnin_near_round_360(float *m_angle,int16_t c_m_angle)
{
	int16_t i;
	uint8_t quadrant_1 = 0,quadrant2=0,quadrant3=0,quadrant4=0;
	uint8_t c_quadrant=0;
	//log_printf("[near_round_360]\r\n");
	for(i=0;i<MAX_C_M_ANGLE;i++)
	{

		if( m_angle[i] == 0)
			continue;
		if(m_angle[i] >=0 && m_angle[i] <=90)
			quadrant_1++;
		if(m_angle[i] >90 && m_angle[i] <=180)
			quadrant2++;
		if(m_angle[i] >180 && m_angle[i] <=270)
			quadrant3++;
		if((m_angle[i] >270 && m_angle[i] <=360) || m_angle[i] < 0)
			quadrant4++;
		//log_printf("quadrant=%3.1f,%d,%d,%d,%d\r\n",m_angle[i],quadrant_1,quadrant2,quadrant3,quadrant4);
		if(c_quadrant >=4 && i > 3)
		{
 			if((get_quadrant(m_angle[0]) == get_quadrant(m_angle[i]) && get_quadrant(m_angle[0]) == get_quadrant(m_angle[i-1])) ||
 			   (get_quadrant(m_angle[1]) == get_quadrant(m_angle[i]) && get_quadrant(m_angle[0]) == get_quadrant(m_angle[i-1]))  ||
 			   (get_quadrant(m_angle[2]) == get_quadrant(m_angle[i]) && get_quadrant(m_angle[0]) == get_quadrant(m_angle[i-1]))  ||
 			    (get_quadrant(m_angle[4]) == get_quadrant(m_angle[i]) && get_quadrant(m_angle[0]) == get_quadrant(m_angle[i-1]))||
 			     (get_quadrant(m_angle[5]) == get_quadrant(m_angle[i]) && get_quadrant(m_angle[0]) == get_quadrant(m_angle[i-1])))
			{
				//log_printf("found round...\r\n");
				for(c_m_angle = 0;c_m_angle<MAX_C_M_ANGLE;c_m_angle++)
					m_angle[c_m_angle] = 0;
				c_m_angle = 0;
				return 1;
			}
		}else
		{
			c_quadrant =count_quadrant(quadrant_1,quadrant2,quadrant3,quadrant4);	//取象限数

		}

	}
	//log_printf("quadrant=%d,%d,%d,%d\r\n",quadrant_1,quadrant2,quadrant3,quadrant4);
	/**/
	if(c_quadrant >=4 && (quadrant_1 >=3 || quadrant2 >=3 || quadrant3 >=3 || quadrant4>=3))
	{
		//log_printf("found round2...\r\n");
		for(c_m_angle = 0;c_m_angle<MAX_C_M_ANGLE;c_m_angle++)
				m_angle[c_m_angle] = 0;
		c_m_angle = 0;
		return 1;
	}

	return 0;
}

//????????
extern GRYO_T *gyro;
extern gyro_t	*mgyro;
void burnin_gyro_mapwhl(void)
{

	if(gyro->isOK ==0)
	{
		*(mgyro->angle)  = 0;
		//g_printf("Warning!!No copy allowed.Copyright idleintel\r\n");
		return ;
	}
//	disable_irq();

	*(mgyro->x_org_f)  = *(mgyro->x_org) - *(mgyro->radius) * gyro->sinA ; // WHELE_HW;
	*(mgyro->y_org_f)  =  *(mgyro->y_org) + *(mgyro->radius) * gyro->cosA ;// WHELE_HW;

	*(mgyro->x_org_r)  = *(mgyro->x_org) + *(mgyro->radius) * gyro->sinA ; // WHELE_HW;
	*(mgyro->y_org_r)  =  *(mgyro->y_org) - *(mgyro->radius) * gyro->cosA ;// WHELE_HW;

	*(mgyro->x_org_t)  = (*(mgyro->x_org) + *(mgyro->radius) * gyro->cosA)*ORG_TO_FRONT ; // WHELE_HW;
	*(mgyro->y_org_t)  =  (*(mgyro->y_org) + *(mgyro->radius) * gyro->sinA)*ORG_TO_FRONT ;// WHELE_HW

	//*(mgyro->x_org_t)  = (navigat->k_x_org + *(mgyro->radius) * gyro->cosA)*ORG_TO_FRONT ; // WHELE_HW;
	//*(mgyro->y_org_t)  =  (navigat->k_y_org + *(mgyro->radius) * gyro->sinA)*ORG_TO_FRONT ;// WHELE_HW
//	enable_irq();

}

void burnin_robot_turn_deg(uint8_t dir,int pwm,float agle)
{
//	int pwm=600;
	//int dg_turn=deg - 2.5;
//	int c=0;
	float dis_agle,b_agle;
	float out_deg;
	int midl=0;
	uint32_t t;
	www_idleintel_com();
	//calc_gyro();
	printf_power_sta();
	//log_printf("turn_charge,dir=%s,agle=%f\r\n",get_motor_sta(dir),agle);
	motor.c_left_hw = motor.c_right_hw = 0;
	//log_printf("1\r\n");
	CHECK_NAVI_STA();

	b_agle = sys->angle;
	//log_printf("2\r\n");
	//burnin_coordinate_calcu();
	//gyro_whlmap();
	navigat->is_walk = 0;
	//log_printf("3\r\n");
	turn_round_pid(0);
	//log_printf("4\r\n");
	motor_run(dir,pwm,0,0);
	sys->right_pwm = sys->left_pwm = pwm;
	TIM5->CNT = 0;
	//从中间碰到红外转过来
	if(MIDLE_HAVE_IRDA2() || *(navigat->near.pid->adc2) >= MAX_IR_SIDE2 )
		midl = 1;

	out_deg = NO_ANGLE_V;
	t=0;
	burnin_coordinate_calcu(0);
	while(1)
	{
		get_sensers(&sys->gSta);
		CHECK_NAVI_STA();
		//laser_calc_move(navigat,MAX_LASR_DIST);
#if LASER_SIDE
		laser_scan_dist(navigat,LASER_CALC_GO_FW);
#endif
		if(TIM5->CNT >=5000)
		{

			TIM5->CNT = 0;
			turn_round_pid(TURN_SPEED);
			www_idleintel_com();
			//calc_gyro();

			dis_agle = b_agle -sys->angle;
			if(dis_agle > 180)
				dis_agle = 360 - dis_agle;
			if(dis_agle <-180)
				dis_agle += 360;
			if(dis_agle >=agle || dis_agle <=-agle)
				break;
			//中间没有红外，就退出
			if(out_deg == NO_ANGLE_V)
			{
				if(agle == WALL_LOST_DEG && midl && (dis_agle >=20 || dis_agle <=-20))
				{
					if(sys->g_sta[2] < 300 && sys->g_sta[3] < 150 && sys->g_sta[4] < 280 &&
						*(navigat->near.pid->adc2) < MAX_IR_SIDE2)
					{

						//out_deg = sys->angle;

						if(*(navigat->near.pid->adc2) < 1700)
						{
							//log_printf("found V_angle=%3.1f,%d\r\n",sys->angle,*(navigat->near.pid->adc2));
							break;
						}
						//log_printf("\r\n");
					}
				}
			}else
			{
				if(disfloat(sys->angle,out_deg) > 5)	//再转10°
				{
					//log_printf("VOK,v_agle=%3.1f\r\n",sys->angle);
					break;
				}
			}
			if(t++>=3000)
			{
				//log_printf("timeout\r\n");
				break;
			}


		}

		//log_printf("%3.1f,%3.1f\r\n",sys->angle,dis_agle);
	}
	motor_run(GO_STOP,0,0,0);
	DELAY_MS(100);
	//delay_ms_sensers(200);
#if LASER_SIDE
		calc_scan_laser(navigat,1);
#endif
//	www_idleintel_com();
	//gyro_whlmap();
	//gyro_mapwhl();
	burnin_gyro_mapwhl();
	//navigat->is_walk =1;
	//log_printf("..OK,c=%d,%d,agl=%3.1f\r\n",motor.c_left_hw,motor.c_right_hw,sys->angle);

}
#define BURN_MAX_KXXYY		80
static int16_t b_kxx[BURN_MAX_KXXYY];
static int16_t b_kyy[BURN_MAX_KXXYY];
char burnin_check_round_bum(uint8_t type)
{
	static uint16_t idx=0;
	short i;
	if(type ==0)
	{
		idx = 0;
		return 0;
	}
	if(idx >=BURN_MAX_KXXYY)
		idx = 0;
	b_kxx[idx] = X_NOW;
	b_kyy[idx++] = Y_NOW;

	for(i=1;i<BURN_MAX_KXXYY;i++)
	{
		if(dis_xy(b_kxx[0],b_kxx[i]) > 5)
			return 0;
		if(dis_xy(b_kyy[0],b_kyy[i]) > 5)
			return 0;
	}
	return 1;

}
#if CALE_BY_FRON
int burnin_coordinate_calcu(uint8_t type)
{

	static int cc=0;
//	static int16_t cc=0;
	int32_t x_org1;//,x_org2;//,
	int32_t y_org1;//,y_org2;//,;
#if WALK_PRINTF_CALC
	static  short xx=0,yy=0;
#endif

	int32_t x_org2;	//右轮
	int32_t y_org2;	//右轮
	int32_t x_org3,y_org3;	//前轮
	float x_org_t,y_org_t;


	float x_org_f,y_org_f;
	float x_org_r,y_org_r;
	cc++;
	if(cc <2 && type==1)
		return 1;
	cc = 0;
//
	disable_irq();
	x_org_f = navigat->x_org_f ;
	y_org_f = navigat->y_org_f;

	x_org_r = navigat->x_org_r;
	y_org_r = navigat->y_org_r;

 	x_org_t= navigat->x_org_t;
 	y_org_t= navigat->y_org_t;
 	enable_irq();
 	//左轮
 	x_org1	=	x_org_calc(x_org_f);
 	y_org1	= 	y_org_calc(y_org_f);
 	//使用前轮
	x_org3 = x_org_calc_f(x_org_t);
	y_org3 = y_org_calc_f(y_org_t);

	//使用右轮
	x_org2	=	x_org_calc_r(x_org_r);
	y_org2	= 	y_org_calc_r(y_org_r);


	/**/
	if(motor.c_left_hw > 100)
	{
		if(disXY(x_org1,x_org2) > 30 || disXY(y_org1,y_org2) > 30)
		{
			navigat->x_org	= 	x_org1;
			navigat->y_org	=  	y_org1;
			//log_printf("[coor_calcu]err x=%d,%d,y=%d,%d,%d\r\n",x_org1,x_org2,y_org1,y_org2,motor.c_left_hw);
			navigat->x_org	= 	(x_org1+x_org2+x_org3) / 3;
			navigat->y_org	=  	(y_org1+y_org2+y_org3) / 3;
			gyro_whlmap();
			return 0;
		}



		if(disXY(x_org1,x_org3) > 30 || disXY(y_org1,y_org3) > 30)
		{
			//log_printf("[coor_calcu]errX=%d,%d,Y=%d,%d,%d\r\n",x_org1,y_org1,x_org3,y_org3,motor.c_left_hw);
			navigat->x_org	= 	(x_org1+x_org2+x_org3) / 3;
			navigat->y_org	=  	(y_org1+y_org2+y_org3) / 3;
			gyro_whlmap();
			return 0;
		}
	}
	/**/
	//if(cc++>=100)
	{
				//log_printf("%d %d|%d %d|%d %d\r\n",x_org1,y_org1,x_org2,y_org2,x_org3,y_org3);
				//	cc = 0;
	}



	navigat->x_org	= 	x_org1;
	navigat->y_org	=  	y_org1;
	gyro_whlmap();

/*
	log_printf("*(%d,%d,%3.1f)[%d,%d|%d,%d|%d,%d]*\r\n",navigat->tx,navigat->ty,sys->angle,
					navigat->x_org,navigat->y_org,x_org1,y_org1,x_org2,y_org2);

	log_printf("*%3.3f[%d,%d|%d,%d|%d,%d](%d,%3.1f,%3.1f)(%d,%3.1f,%3.1f)*\r\n",sys->angle,
						navigat->x_org,navigat->y_org,x_org1,y_org1,x_org2,y_org2,
						motor.c_left_hw,navigat->x_org_f,navigat->y_org_f,
						motor.c_right_hw,navigat->x_org_r,navigat->y_org_r);


	log_printf("*(%d,%d,%3.3f)[%d,%d|%d,%d](%d,%d,%d)*\r\n",navigat->tx,navigat->ty,sys->angle,
						navigat->x_org,navigat->y_org,x_org2,y_org2,
						motor.c_right_hw,motor.c_right_hw,navigat->distance);
	*/
#if WALK_PRINTF_CALC
	if(xx != X_NOW || yy != Y_NOW)
	{
		xx 	= X_NOW;
		yy	= Y_NOW;
		//log_printf("*(%d,%d,%3.1f,0)[%d,%d,%d,%d,%d]*\r\n",navigat->tx,navigat->ty,sys->angle,navigat->x_org,navigat->y_org,
		//												x_org2,y_org2,navigat->distance);
	}

#endif



	return 1;



}
#else
int burnin_coordinate_calcu(uint8_t type)
{


	int32_t x_org1;//,x_org2;//,
	int32_t y_org1;//,y_org2;//,;
#if WALK_PRINTF_CALC
	static  short xx=0,yy=0;
#endif

#if CALE_ADJ_RIGHT
	int32_t x_org2;//,
	int32_t y_org2;//,;
#endif
#if CALE_BY_FRONT
//	static int distanc=0;
	int32_t x_org_front,y_org_front;
	float x_org_t,y_org_t;
	//int ret=0; asd

#endif

	float x_org_f,y_org_f;
#if CALE_ADJ_RIGHT
	float x_org_r,y_org_r;
#endif
//
	disable_irq();
	x_org_f = navigat->x_org_f ;
	y_org_f = navigat->y_org_f;
#if CALE_ADJ_RIGHT
	x_org_r = navigat->x_org_r;
	y_org_r = navigat->y_org_r;
#endif
 #if CALE_BY_FRONT
 	x_org_t= navigat->x_org_t;
 	y_org_t= navigat->y_org_t;
 #endif
 	enable_irq();

 	x_org1	=	x_org_calc(x_org_f);
 	y_org1	= 	y_org_calc(y_org_f);
 	//使用前轮
#if CALE_BY_FRONT
	x_org_front = x_org_calc_f(x_org_t);
	y_org_front = y_org_calc_f(y_org_t);

	navigat->k_x_org	= 	x_org_front;
	navigat->k_y_org	=  	y_org_front;
	coord_org2map(navigat->k_x_org,navigat->k_y_org,&navigat->kx,&navigat->ky);
#endif
	//log_printf("%d %d %d %d %d %d\r\n",);
	//使用右轮校准
#if CALE_ADJ_RIGHT
		if(navigat->adj_run == FALSE)
		{
			x_org2	=	x_org_calc_r(x_org_r);
			y_org2	= 	y_org_calc_r(y_org_r);

			//TEST MICONY
			//micony 2017-12-27不知道为何，差速延边的时候坐标差异好大
			//if(sys->nsta == NO_SIDE_NEAR)
			{
				if(disXY(x_org1,x_org2) > 160 || disXY(y_org1,y_org2) > 160)
				{
					navigat->x_org	= 	x_org1;
					navigat->y_org	=  	y_org1;
					//log_printf("[coor_calcu]err x=%d,%d,y=%d,%d\r\n",x_org1,x_org2,y_org1,y_org2);
					return 0;
				}
			}
		}


#if CALE_BY_FRONT		//使用前轮参与校准
		//navigat->x_org	= 	(x_org1 + x_org_front)>> 1;
		//navigat->y_org	=  	(y_org1 + y_org_front)>> 1;
		if(disXY(x_org1,x_org_front) > 20 || disXY(y_org1,y_org_front) > 20)
		{
			//log_printf("[coor_calcu]errX=%d,%d,Y=%d,%d\r\n",x_org1,x_org2,x_org_front,y_org_front);
			return 0;
		}
		gyro_fwhlmap();
#endif

		//navigat->x_org	= 	(x_org1 +x_org2)>>1 ;// + (int32_t)((float)(x_org2 - x_org1)*0.6f);
		//navigat->y_org	=  	(y_org1 +y_org2)>>1 ;//+ (int32_t)((float)(y_org2 - y_org1)*0.6f);
		navigat->x_org	= 	x_org1;
		navigat->y_org	=  	y_org1;

#else
		navigat->x_org	= 	x_org1;
		navigat->y_org	=  	y_org1;

#endif

/*
	log_printf("*(%d,%d,%3.1f)[%d,%d|%d,%d|%d,%d]*\r\n",navigat->tx,navigat->ty,sys->angle,
					navigat->x_org,navigat->y_org,x_org1,y_org1,x_org2,y_org2);

	log_printf("*%3.3f[%d,%d|%d,%d|%d,%d](%d,%3.1f,%3.1f)(%d,%3.1f,%3.1f)*\r\n",sys->angle,
						navigat->x_org,navigat->y_org,x_org1,y_org1,x_org2,y_org2,
						motor.c_left_hw,navigat->x_org_f,navigat->y_org_f,
						motor.c_right_hw,navigat->x_org_r,navigat->y_org_r);


	log_printf("*(%d,%d,%3.3f)[%d,%d|%d,%d](%d,%d,%d)*\r\n",navigat->tx,navigat->ty,sys->angle,
						navigat->x_org,navigat->y_org,x_org2,y_org2,
						motor.c_right_hw,motor.c_right_hw,navigat->distance);
	*/
#if WALK_PRINTF_CALC
	if(xx != X_NOW || yy != Y_NOW)
	{
		xx 	= X_NOW;
		yy	= Y_NOW;
		//log_printf("*(%d,%d,%3.1f,0)[%d,%d,%d,%d,%d]*\r\n",navigat->tx,navigat->ty,sys->angle,navigat->x_org,navigat->y_org,
		//												x_org2,y_org2,navigat->distance);
	}

#endif



	return 1;



}


//判断是否超时(与设定的时间)
//注意：msec输入参数为0时的情况
uint8_t burnin_mstimeout(uint32_t *timep,uint32_t msec)
{
	uint32_t time,diff;
	uint8_t result=0;
	time=msTmr;
	diff=time- *timep;
	if(msec==0)
	{
		*timep=time;
		result=1;
	}
	else if(diff>=msec)
	{
		//diff += (msec + (msec >> 1));
		result = (uint8_t)(diff/msec);
		*timep=time;
	}
	return result;
}

#endif

int burnin_near_wall_pid(float *agle,int c_lost)
{
	static int c_pid=0;
	NEAR_WALL *near;

	near = &navigat->near;
	if(c_pid ++ >=2)
	{
		c_pid = 0;
		//if(near->n_sta !=NO_SIDE_NEAR && near->pid != NULL)		//强制延边
		{
			return (near_wall_pid(1,near->pid,agle));
		}/*else
		{
			if(near_wall_pid(0,&l_near_pid,agle) ==0)
			{
				return (near_wall_pid(0,&r_near_pid,agle));
			}
			return 1;
		}*/

	}
	return 1;

}
#endif
/*****************************************************************************
 * 函数名称:
 * 入    参:	type  :
 *						 GO_NEAR_TYPE_NO		0x00		//普通延边
 *						 GO_NEAR_TYPE_NAVI		0x01		//中间如果能导航到终点，则退出去导航
 *						 GO_NEAR_TYPE_ADJ	   0x02
 *函数功能:	延边导航，找地方矫正，如果到目的地，则退出
 *
 *****************************************************************************/
 static uint32_t burninbeep_timer = 0;
char motor_go_burnin(uint8_t n_sta ,short tox,short toy,uint8_t type,uint8_t is_save)
{
//	int xx=0,yy=0,x1=0,y1=0;
	int calue=0;
//	int by = navigat->y_org;
//	uint8_t gSta;
	uint8_t sta;
//	int	lx=0,ly=0;
	short llx=0,lly=0;
//	short lx=X_NOW,ly=Y_NOW;
//	int bx = X_NOW;
	short by = Y_NOW;
//	short bx = X_NOW;
//	short nx1,ny1;
	float lagle;		//记录最后的延边角度，用于计算是否转弯过多
	uint16_t	gSta;
	uint32_t	t_begin;//开始的时间
	int ret_calc=1;
	uint16_t c_dock_data=0;
	int16_t c_lost=0;
	uint16_t c_round=0;		//转圈的次数


	short x_begin_line=0,y_begin_line = 0;		//一条线开始的X，Y的坐标

	uint8_t sec1s_ct = 0;// 1秒的计数器
	///uint8_t time5ms_flag = 0;//5ms时间标志


	u8 irData_bak[6];

	NEAR_WALL *near = &navigat->near;
	float m_angle[MAX_C_M_ANGLE];			//保存最近的20个碰撞的角度
	int16_t c_m_angle=0;
	int16_t c_near_wall = 0;

	for(c_m_angle = 0;c_m_angle<MAX_C_M_ANGLE;c_m_angle++)
		m_angle[c_m_angle] = 0;
	c_m_angle = 0;
	CHECK_NAVI_STA_RT(0);
	pd_gyro_int(GO_NEAR_PWM_FAST);
	navigat->out =navigat->angle;
	navigat->distance = 0;
	motor.c_left_hw = motor.c_right_hw = 0;
	cord_calc_store(0);
	gyro_whlmap();
	robot_whele_stop(0);
	motor_run(GO_FORWARD, GO_NEAR_PWM_FAST, 0, 0);
	if(n_sta & 0x80)
		init_near_wall_navi(n_sta & 0x7F);
	else
		init_near_wall_navi(NO_SIDE_NEAR);
	n_sta &=0x7F;
	navigat->is_walk =1;
#if BURNIN_LOG_EN
	log_printf("[motor_go_edgeways]sta=%d(%d),type=%d,is_save=%d,now=(%d,%d,)to=(%d,%d,)\r\n",navigat->near.n_sta,n_sta,type,is_save,X_NOW,Y_NOW,tox,toy);
#endif
	//navigat->wheel_dir = 1;
	sta = sys->sState;
	lagle = sys->angle;
	mstimeout(&t_begin,0);
	MOTOR_CTRL(NORM_SIDE_PWM_L,NORM_SIDE_PWM_R,NORM_MID_PWM,sys->dust_pwm_value)
#if BURNIN_LOG_EN
	printf_power_sta();
#endif
	burnin_mstimeout(&burninbeep_timer,0);
	while(1)
	{

		sec1s_ct = burnin_mstimeout(&burninbeep_timer,1000);
		proc_uart_task();
		get_sensers(&sys->gSta);
		if(sys->sState !=sta)
		{
#if BURNIN_LOG_EN
			log_printf("\r\nmode changed\r\n");
#endif
			tx_com_burnin(NEAR_BY_IRDA, burnin_work_timer, bummech_left_ct, bumir_left_ct, bummech_right_ct, bumir_right_ct);
			return 0;
			//log_printf("\r\nerr:%d\r\n",sys->work_errcode);
			//sys->sState = SYS_RANDOM;
			//MOTOR_POWER_ON();
			//motor_run(GO_FORWARD, GO_NEAR_PWM_FAST, 0, 0);
			//MOTOR_CTRL(NORM_SIDE_PWM_L,NORM_SIDE_PWM_R,NORM_MID_PWM,sys->dust_pwm_value)
		}

		if(sec1s_ct)
		//if(mstimeout(&sys->t_loop,1000))
		//if(TIM5->CNT >=5000)
		{
			//TIM5->CNT = 0;
			//time5ms_flag = 1;

			//if(sec1s_ct ++ > 199)
			{
				//sec1s_ct = 0;
				if(near->n_sta == LEFT_SIDE_NEAR)
				{
					bummech_left_ct = bummech_ct;
					bumir_left_ct = bumir_ct;
				}
				else if(near->n_sta == RIGHT_SIDE_NEAR)
				{
					bummech_right_ct = bummech_ct;
					bumir_right_ct = bumir_ct;
				}
				//log_printf("--------------------------------------\r\n");
				burnin_work_timer += sec1s_ct;
				burnin_work_near_timer += sec1s_ct;
				if(sys->work_errcode != 0)
					log_printf("\r\nerr:%d\r\n",sys->work_errcode);
				tx_com_burnin(near->n_sta, burnin_work_timer, bummech_left_ct, bumir_left_ct, bummech_right_ct, bumir_right_ct);
				if(burnin_work_near_timer > BURNIN_EX_TIME)//测试中,暂定5分钟
				{
					burnin_work_near_timer = 0;
					motor_run(GO_STOP,0,0,0);
					return RET_NEAR_TIMEOUT;
				}
			}
		}
		//if(sys->work_errcode != 0 && sys->c_left==0 && sys->c_right == 0)
		//{
		//}
			//return RET_NEAR_ERROR;
		CHECK_NAVI_STA_RT(0);
		//碰撞
		if(sys->gSta & (MASK_BUM_MIDL) ||(sys->gSta & (MASK_BUM_LEFT | MASK_BUM_LEFT2 | MASK_BUM_RIGHT | MASK_BUM_RIGHT2))||  ret_calc==0 ||  c_lost >=MAX_NEAR_LOST || MIDLE_IRDA())
		{
//			gSta = sys->gSta;	//记录碰撞的状态

			motor_run(GO_STOP,0,0,0);
#if BURNIN_LOG_EN
			log_printf("BUM\r\n");
#endif
			//delay_ms(200);
			if(sys->gSta & (MASK_BUM_MIDL))
			{
				if(sys->fall_sta)		//跌落，则
					burnin_back_off(BACK_HW*8);
				else
					burnin_back_off(BACK_HW);
				bummech_ct ++;//机械碰撞或跌落的计数

			}
			if(MIDLE_IRDA())
				bumir_ct ++;//正前方红外碰撞的计数
			//delay_ms_sensers(200);
			burnin_coordinate_calcu(0);														//计算出原始的坐标系
			coord_org2map(navigat->x_org,navigat->y_org,&navigat->tx,&navigat->ty); //坐标系转换
			motor_run(GO_STOP,0,0,0);
#if BURNIN_LOG_EN
			log_printf("\r\n-----go_edgeways bum(%d,%d,%d,%f,%f),gsta=%d,irda=(%d,%d,%d,%d)angle=%3.1f,ret=%d,lost=%d,%d\r\n",navigat->tx,navigat->ty,motor.c_left_hw,navigat->x_org_f,navigat->y_org_f,
							sys->gSta,sys->g_sta[0],sys->g_sta[1],sys->g_sta[5],sys->g_sta[6],sys->angle,ret_calc,c_lost,c_m_angle);
#endif

			if(ret_calc==0)
					burnin_gyro_mapwhl();
			if(c_m_angle >=MAX_C_M_ANGLE)
				c_m_angle = 0;
			m_angle[c_m_angle++] = sys->angle;		//保持角度

			if(burnin_near_round_360(m_angle,c_m_angle))
			{
#if BURNIN_LOG_EN
				log_printf("found round...go line...\r\n");
#endif

				init_near_wall_navi(NO_SIDE_NEAR);
				for(c_m_angle = 0;c_m_angle<MAX_C_M_ANGLE;c_m_angle++)
					m_angle[c_m_angle] = 0;
				c_m_angle = 0;
				goto l_mgo_edeways_burnin;
			}



			if(!(ret_calc==0 || c_lost >=MAX_NEAR_LOST))
				c_round = 0;
			if(ret_calc==0)
					gyro_mapwhl();
			gSta = sys->gSta;
			//motor_run(GO_STOP,0,0,0);
//			if(sys->work_mod & MWO_MOP)
		//		delay_ms(200);
			//www_idleintel_com();
			lagle = sys->angle;
			//init_near_wall_navi(n_sta);		//碰撞后，才开始一直沿边

			navigat->distance = 0;
			motor.c_left_hw = 0;

			if( type == GO_NEAR_TYPE_DOCK)
			{
				//找充电桩
				read_ir_data_timeout(irData_bak,0);

				if(irData_bak[IR_L_PIN_NUM] ||  irData_bak[IR_ML_PIN_NUM] || irData_bak[IR_R_PIN_NUM] || ir_dock_insight(1))
				{
					//int16_t i;
					//for(i=0;i<3;i++)
					{
						//delay_ms(80);
						//read_ir_data(irData_bak,0);

#if BURNIN_LOG_EN
						log_printf("[motor_go_edgeways]bum  dock(%d,%d,%d)%d\r\n",
						irData_bak[IR_L_PIN_NUM] ,irData_bak[IR_ML_PIN_NUM],irData_bak[IR_R_PIN_NUM],c_dock_data);
#endif
						//if(irData_bak[IR_L_PIN_NUM] ||  irData_bak[IR_M_PIN_NUM] || irData_bak[IR_R_PIN_NUM])
						if(1)
						{

							c_dock_data++;
#if BURNIN_LOG_EN
							log_printf("ir=%d,%d,%d,c=%d\r\n",irData_bak[IR_L_PIN_NUM] , irData_bak[IR_ML_PIN_NUM] , irData_bak[IR_R_PIN_NUM],c_dock_data);
#endif
							if(c_dock_data >=3)
							{
#if BURNIN_LOG_EN
								log_printf("found dock..\r\n");
#endif
								goto l_go_out_for_dock_burnin;
							}
						}else
							c_dock_data=0;
					}

				}else
					c_dock_data = 0;
			}


			/**************************************************************************
				如果tox，toy有设置了置，则到点了，就停下来。
			*****************************************************************************/
			if(  (((X_NOW == tox || tox ==0) && (Y_NOW == toy || toy==0)) && tox && toy) || 	//到点了
				(type == GO_NEAR_TYPE_ADJ &&  ((by > toy && Y_NOW < toy) || (by < toy && Y_NOW > toy)))) //adj回去，Y轴超过了
			{
#if BURNIN_LOG_EN
				log_printf("xy ok(%d,%d,%3.1f)\r\n",X_NOW,Y_NOW,sys->angle);
#endif
				motor_run(GO_STOP,0,0,0);
				//delay_ms_sensers(200);
				//burnin_coordinate_calcu();														//计算出原始的坐标系
				//coord_org2map(navigat->x_org,navigat->y_org,&navigat->tx,&navigat->ty);	//坐标系转换
				return RET_NEAR_OK;
			}


			if(near->n_sta ==NO_SIDE_NEAR)
			{
				init_near_wall_navi(n_sta);
			}

			if( (ret_calc==0 || c_lost >= MAX_NEAR_LOST)&& *(navigat->near.pid->adc) < navigat->near.pid->min_adc )
			{
				c_round++;
#if BURNIN_LOG_EN
				log_printf("lost or calc error(%d,%d),cround=%d\r\n",ret_calc,c_lost,c_round);
#endif
				/*
				if(c_round >=3)
				{
					log_printf("big round not near\r\n");
					init_near_wall_navi(NO_SIDE_NEAR);
					goto l_go_edeways;
				}
				*/
				if(ret_calc)
				{
					gyro_whlmap();
				}
				if(near->n_sta == RIGHT_SIDE_NEAR)
				{
					burnin_robot_turn_deg(GO_RIGTH,DEG_TURN_PWM,NEAR_LOST_DEG);
					goto l_mgo_edeways_burnin;
				}else if(near->n_sta == LEFT_SIDE_NEAR)
				{
					burnin_robot_turn_deg(GO_LEFT,DEG_TURN_PWM,NEAR_LOST_DEG);
					goto l_mgo_edeways_burnin;
				}

			}

			if(near->n_sta == RIGHT_SIDE_NEAR)
			{
				if(burnin_check_round_bum(1))
					burnin_robot_turn_deg(GO_LEFT,DEG_TURN_PWM,12);
				else
				{
					if(BUM_LEFT(gSta) || MIDLE_HAVE_IRDA2() || RIGHT_IR_BUM2())
					{
						burnin_robot_turn_deg(GO_LEFT,DEG_TURN_PWM,WALL_LOST_DEG);
					}else if(sys->g_sta[6] > 900 && sys->g_sta[5] > 900)//贴墙了，则可以延边，不要转太多
					{
						burnin_robot_turn_deg(GO_LEFT,DEG_TURN_PWM,12);
					}else
						burnin_robot_turn_deg(GO_LEFT,DEG_TURN_PWM,25);
				}

			}else if(near->n_sta == LEFT_SIDE_NEAR)
			{
				if(burnin_check_round_bum(1))
					burnin_robot_turn_deg(GO_RIGTH,DEG_TURN_PWM,12);

				else
				{
					if(BUM_RIGHT(gSta) || MIDLE_HAVE_IRDA2() || LEFT_IR_BUM2())
					{
						burnin_robot_turn_deg(GO_RIGTH,DEG_TURN_PWM,WALL_LOST_DEG);
					}
					else if(sys->g_sta[1] > 900 && sys->g_sta[2] > 900)
					{
						burnin_robot_turn_deg(GO_RIGTH,DEG_TURN_PWM,12);
					}
					else
					{
						burnin_robot_turn_deg(GO_RIGTH,DEG_TURN_PWM,25);
					}
				}

			}
			else
			{
#if BURNIN_LOG_EN
				log_printf("RET_NEAR_ERROR,nsta=%d\r\n",n_sta);
#endif
				return RET_NEAR_ERROR;
			}
l_mgo_edeways_burnin:

			//转圈了。。
			c_lost=0;
			//burnin_coordinate_calcu();
			motor_run(GO_STOP,0,0,0);
		//	log_printf("after bk(%d,%d,%d,%3.3f,%3.3f,%3.3f)\r\n==============\r\n",navigat->tx,navigat->ty,motor.c_left_hw,navigat->x_org_f,navigat->y_org_f,sys->angle);
			//if(ccc++ >=5)
			//	while(1);
			navigat->distance = 0;
			navigat->is_walk = 1;
			pd_gyro_int(GO_NEAR_PWM_FAST);
			navigat->out =sys->angle;
			cord_calc_store(0);
			gyro_whlmap();
			motor.c_left_hw = motor.c_right_hw = navigat->distance = 0;

			motor_run(GO_FORWARD, GO_NEAR_PWM_FAST, 0, 0);
			navigat->near.pid->c_lost = 0;

			navigat->near.pid->c_lost = 0;
			navigat->near.pid->c_lost_flag = 0;			//失去墙的标志     2019 02 15 add
			sys->g_t_walk = 0;
			ret_calc = 1;
			//记录起点的位置
			x_begin_line = X_NOW;
			y_begin_line = Y_NOW;
			c_near_wall = 0;
		//	motor_run(GO_RIGHT_RD,0,0,0);
		//	navigat->near_sta = LOST_WALL_RIGHT;
		}

		if(TIM5->CNT >=5000)
		//if(time5ms_flag)
		{
			TIM5->CNT = 0;
			//time5ms_flag = 0;
			navigat->out = format_agle(navigat->out,ANGLE_360);
			proc_line_pid(navigat->out);
			burnin_near_wall_pid(&navigat->out,5);
			//navigat_near_wall_pid(&navigat->out,5);
			if(near->n_sta ==NO_SIDE_NEAR)
			{
				if(RIGHT_ADC() >= cfg->lock_right_adc)
					init_near_wall_navi(RIGHT_SIDE_NEAR);
				else if(LEFT_ADC() >= cfg->lock_left_adc)
					init_near_wall_navi(LEFT_SIDE_NEAR);
			}

			if(robot_whele_stop(1))
					continue;			//直接出来，给碰撞做准备



			if( type == GO_NEAR_TYPE_DOCK )
			{
				//找充电桩
			//	c_dock = 0;
				read_ir_data_timeout(irData_bak,0);
				if(irData_bak[IR_L_PIN_NUM] ||	irData_bak[IR_ML_PIN_NUM] || irData_bak[IR_R_PIN_NUM] || ir_dock_insight(1))
				{
#if BURNIN_LOG_EN
					log_printf("[motor_go_edgeways]found dock(%d,%d,%d)%d\r\n",
						irData_bak[IR_L_PIN_NUM] ,irData_bak[IR_ML_PIN_NUM],irData_bak[IR_R_PIN_NUM],c_dock_data);
#endif
					//if(c_dock_data++ >=3)
					{
#if BURNIN_LOG_EN
						log_printf("[motor_go_edgeways]found dock\r\n");
#endif
						motor_run(GO_STOP,0,0,0);
						//delay_ms_sensers(200);
						burnin_coordinate_calcu(0); 													//计算出原始的坐标系
						coord_org2map(navigat->x_org,navigat->y_org,&navigat->tx,&navigat->ty); //坐标系转换
						goto	l_go_out_for_dock_burnin;
					}

				}else
					c_dock_data = 0;
			}


			if( *(navigat->near.pid->adc) > navigat->near.pid->min_adc) //延边
			  lagle = sys->angle;
			else		//延边消失，转的角度超过180度，则失败退出
			{
				if(disfloat( lagle , sys->angle) > 180)
				{
#if BURNIN_LOG_EN
					log_printf("lost over(%d,%d,%3.1f,%3.1f)\r\n",X_NOW,Y_NOW,sys->angle,lagle);
#endif
					motor_run(GO_STOP,0,0,0);
					//delay_ms_sensers(200);
					burnin_coordinate_calcu(0);														//计算出原始的坐标系
					coord_org2map(navigat->x_org,navigat->y_org,&navigat->tx,&navigat->ty);	//坐标系转换
					return RET_NEAR_ERROR;
				}
			}
#if CALE_BY_FRON
				ret_calc = burnin_coordinate_calcu(1);														//计算出原始的坐标系
#endif
			if(calue++ >=40)
			{

				if(c_near_wall ++ >=30)
				{

					for(c_m_angle = 0;c_m_angle<MAX_C_M_ANGLE;c_m_angle++)
						m_angle[c_m_angle] = 0;
					c_m_angle = 0;
					c_near_wall = 0;
#if BURNIN_LOG_EN
					log_printf("log near\r\n");
#endif
				}



				calue = 0;
#if !CALE_BY_FRON
				ret_calc = burnin_coordinate_calcu(0);														//计算出原始的坐标系
#endif
				coord_org2map(navigat->x_org,navigat->y_org,&navigat->tx,&navigat->ty);	//坐标系转换
				//不是0度，90度，80度的方向,刷新坐标
				/**/
				if(x_begin_line != X_NOW && y_begin_line != Y_NOW)
				{
//					ajust_xy_by_near_line(x_begin_line,y_begin_line,X_NOW,Y_NOW,sys->angle,LINE_TYPE_LOST,n_sta);
					x_begin_line = X_NOW;
					y_begin_line = Y_NOW;
				}


				if( *(navigat->near.pid->adc) > navigat->near.pid->min_adc) //延边
				{
					//c_near++;
					c_round = 0;

				}
				else if(near->n_sta !=NO_SIDE_NEAR)	//强制延边，则计数丢失的个数
					c_lost ++;

				if(llx!=navigat->tx || lly!=navigat->ty)
				{
#if WALK_PRINTF	&& BURNIN_LOG_EN
					log_printf("*(%d,%d,%3.1f,0)-[%3.1f,%3.1f,0]*\r\n",navigat->tx,navigat->ty,sys->angle,navigat->x_org_f,navigat->y_org_f);
#endif
					llx = navigat->tx;
					lly = navigat->ty;
				}
				/**************************************************************************
					如果tox，toy有设置了置，则到点了，就停下来。
				*****************************************************************************/
				if(  (((X_NOW == tox || tox ==0) && (Y_NOW == toy || toy==0)) && (tox || toy)) || 	//到点了
				(type == GO_NEAR_TYPE_ADJ &&  ((by > toy && Y_NOW < toy) || (by < toy && Y_NOW > toy)))) //adj回去，Y轴超过了
				{
#if BURNIN_LOG_EN
						log_printf("xy ok(%d,%d,%3.1f)\r\n",X_NOW,Y_NOW,sys->angle);
#endif
						motor_run(GO_STOP,0,0,0);
						//delay_ms_sensers(200);
						burnin_coordinate_calcu(0);														//计算出原始的坐标系
						coord_org2map(navigat->x_org,navigat->y_org,&navigat->tx,&navigat->ty);	//坐标系转换
						return RET_NEAR_OK;
				}

			

				if(type == GO_NEAR_DRAW_MAP)		//画地图
				{
					if(Y_NOW < 100 &&  Y_NOW > 98 && disXY(X_NOW,100) < 3)
					{
#if BURNIN_LOG_EN
						log_printf("[motor_go_edgeways]to begin point2\r\n",X_NOW,Y_NOW);
#endif
						motor_run(GO_STOP,0,0,0);
						//delay_ms_sensers(200);
						burnin_coordinate_calcu(0);														//计算出原始的坐标系
						coord_org2map(navigat->x_org,navigat->y_org,&navigat->tx,&navigat->ty);	//坐标系转换
						return RET_NEAR_OK;
					}
				}

			}
		}

	}


//	return 0;

l_go_out_for_dock_burnin:
	if(near->n_sta == RIGHT_SIDE_NEAR)
	{
		burnin_robot_turn_deg(GO_LEFT,DEG_TURN_PWM,60);
	}else if(near->n_sta == LEFT_SIDE_NEAR)
	{
		burnin_robot_turn_deg(GO_RIGTH,DEG_TURN_PWM,60);
	}
#if BURNIN_LOG_EN
	log_printf("[motor_go_fw]...\r\n");
#endif
	CHECK_IDLE_STA_RT(0);
	sys->sState = SYS_DOCK;
	motor_go_fw(2000,NO_SIDE,0);
	return 0;
	//delay_ms(1000);
}

//老化测试任务初始化函数
void proc_burn_in_init(void)
{
	//log_printf("dir = %d,pwm=%d,speed=%d,hw=%d\r\n",dir,ctrl->pwm,ctrl->speed,ctrl->hw);
	sys->shut_down_motor = 0;
	navigat_init(0);

	MOTOR_POWER_ON();
	MOTOR_CTRL(NORM_SIDE_PWM_L,NORM_SIDE_PWM_R,NORM_MID_PWM,sys->dust_pwm_value)
	sys->sState = SYS_RANDOM;
	burnin_work_near_timer = 0;
	burnin_work_timer = 0;
	bummech_ct = bumir_ct = 0;

	bummech_left_ct = bummech_right_ct = 0;
	bumir_left_ct = bumir_right_ct = 0;
}
//老化测试任务
void proc_burn_in_task(void)
{
	uint8_t near_dir = LEFT_SIDE_NEAR;


	tx_com_burnin(near_dir, burnin_work_timer, bummech_left_ct, bumir_left_ct, bummech_right_ct, bumir_right_ct);
	motor_go_forwark(0,NO_SIDE_NEAR,NULL);		//直行
	motor_go_burnin(LEFT_SIDE_NEAR,0,0,GO_NEAR_TYPE_NO,0);
	bummech_left_ct = bummech_ct;
	bummech_ct = 0;
	bumir_left_ct = bumir_ct;
	bumir_ct = 0;

	tx_com_burnin(near_dir, burnin_work_timer, bummech_left_ct, bumir_left_ct, bummech_right_ct, bumir_right_ct);
	near_dir = RIGHT_SIDE_NEAR;
	motor_go_forwark(0,NO_SIDE_NEAR,NULL);		//直行
	motor_go_burnin(RIGHT_SIDE_NEAR,0,0,GO_NEAR_TYPE_NO,0);
	bummech_right_ct = bummech_ct;
	bummech_ct = 0;
	bumir_right_ct = bumir_ct;
	bumir_ct = 0;

	tx_com_burnin(NO_SIDE_NEAR, burnin_work_timer, bummech_left_ct, bumir_left_ct, bummech_right_ct, bumir_right_ct);
	STOP_ALL_MOTOR();
	sys->sState = SYS_IDLE;
	ny3p_play(VOICE_M_FINISHED);
}

//=========================================================================================end


void sn_print(uint8_t * pdat,uint16_t len)
{
	uint16_t i;

	len --;
	log_printf("\r\n[sn %d]:",len);
	for(i = 0;i < len;i ++)
	{
		log_printf("%c",pdat[i]);
	}
	log_printf("\r\n");
	for(i = 0;i < len;i ++)
	{
		log_printf("%02X ",pdat[i]);
	}
	log_printf("\r\n%02X\r\n",pdat[i]);
}

void data_print(uint8_t * pdat,uint16_t len)
{
	uint16_t i;

	log_printf("\r\n[data_print %d]:",len);
	for(i = 0;i < len;i ++)
	{
		log_printf("%02x ",pdat[i]);
	}
	log_printf("\r\n\r\n");
}

static uint8_t mact_remote_tst_reg = 0;//bit0 显控 bit1 remote controller, 0xff 清零
void set_remote_flag(uint8_t set)
{
	if(set == 0xff)
		mact_remote_tst_reg = 0;
	else
		mact_remote_tst_reg |= (set < 4)?set:0;
}
uint8_t get_remote_flag(void)
{
	return mact_remote_tst_reg;
}
static uint8_t norm_sensor_false_reg = 0;//触发型传感器/开关量检测的未触发的检测成功标记，即如果检测到检测按键未按键时，IO读入为1,即未触发,那么对应的位置1,位说明参考宏NORM_SENSOR_XXX
static uint8_t norm_sensor_true_reg = 0;//触发型传感器/开关量检测的未触发的检测成功标记，即如果检测到检测按键按键时，IO读入为0,即已被触发,那么对应的位置1,位说明参考宏NORM_SENSOR_XXX
//在测试模式下获取各传感器的输入数据,各种数据均用位表示,
// 位设置与bit0-bit7分别为:抹布/虚拟墙/尘盒/集尘盒满/左落轮/右落轮/左碰撞/右碰撞
uint16_t get_mactest_sensors(void)
{
	uint8_t i;
	uint16_t n_res = 0;

	if((norm_sensor_false_reg & NORM_SENSOR_BIN) == 0 && (READ_DUSTBOX_DET() != 0))//没有尘盒
	{
		for(i=0;i<20;i++)//抹布检测
		{
			if(READ_DUSTBOX_DET() == 0)
			break;
		}
		if(i == 20)
			norm_sensor_false_reg |= (1 << NORM_SENSOR_BIN);
	}
	if(((norm_sensor_true_reg & NORM_SENSOR_BIN) == 0) && (READ_DUSTBOX_DET() == 0))//有尘盒
	{
		for(i = 0;i < 20;i ++)
		{
			if(READ_DUSTBOX_DET() != 0)
				break;
		}
		if(i==20)
			norm_sensor_true_reg |= (1 << NORM_SENSOR_BIN);
	}

	if(((norm_sensor_false_reg & (1<<NORM_SENSOR_KEY_WIFI)) == 0) && !KEY_RD_WIFI())
	{
		for(i=0;i<20;i++)//虚拟墙
		{
			if(KEY_RD_WIFI())
				break;
		}
		if(i==20)
			norm_sensor_false_reg |= (1 << NORM_SENSOR_KEY_WIFI);
	}
	if(((norm_sensor_true_reg & (1<<NORM_SENSOR_KEY_WIFI)) == 0) && KEY_RD_WIFI())
	{
		for(i=0;i<20;i++)//尘盒检测
		{
			if(!KEY_RD_WIFI())
				break;
		}
		if(i==20)
			norm_sensor_true_reg |= (1 << NORM_SENSOR_KEY_WIFI);
	}

	if(((norm_sensor_false_reg & (1<<NORM_SENSOR_KEY_SPOT)) == 0) && !KEY_RD_SPOT())
	{
		//log_printf("NORM_SENSOR_KEY_SPOT\r\n");
		for(i = 0;i < 20;i ++)
		{
			if(KEY_RD_SPOT())
				break;
		}
		if(i == 20)
			norm_sensor_false_reg |= (1 << NORM_SENSOR_KEY_SPOT);
	}
	if(((norm_sensor_true_reg & (1 << NORM_SENSOR_KEY_SPOT)) == 0) && KEY_RD_SPOT())
	{
		for(i=0;i<20;i++)//左落轮
		{
			if(!KEY_RD_SPOT())
				break;
		}
		if(i==20)
			norm_sensor_true_reg |= (1 << NORM_SENSOR_KEY_SPOT);
	}
	if(((norm_sensor_false_reg & (1 << NORM_SENSOR_KEY_START)) == 0) && !KEY_RD_CLEAN())
	{
		for(i = 0;i < 20;i ++)
		{
			if(KEY_RD_CLEAN())
				break;
		}
		if(i == 20)
			norm_sensor_false_reg |= (1 << NORM_SENSOR_KEY_START);
	}
	if(((norm_sensor_true_reg & (1 << NORM_SENSOR_KEY_START)) == 0) && KEY_RD_CLEAN())
	{
	for(i=0;i<20;i++)//右落轮
	{
			if(!KEY_RD_CLEAN())
				break;
		}
		if(i == 20)
			norm_sensor_true_reg |= (1 << NORM_SENSOR_KEY_START);
	}

	if(((norm_sensor_false_reg & (1 << NORM_SENSOR_KEY_DOCK)) == 0) && !KEY_RD_DOCK())
	{
		for(i = 0;i < 20;i ++)
		{
			if(KEY_RD_DOCK())
				break;
		}
		if(i == 20)
			norm_sensor_false_reg |= (1 << NORM_SENSOR_KEY_DOCK);
	}
	if(((norm_sensor_true_reg & (1 << NORM_SENSOR_KEY_DOCK)) == 0) && KEY_RD_DOCK())
	{
		for(i = 0;i < 20;i ++)
		{
			if(!KEY_RD_DOCK())
				break;
		}
		if(i == 20)
			norm_sensor_true_reg |= (1 << NORM_SENSOR_KEY_DOCK);
	}

	uint8_t com_flag = get_remote_flag();
	////if((norm_sensor_true_reg & NORM_SENSOR_REMOTE) == 0 && (get_remote_flag()&0x02))//遥控器按键下发,这里不管true和false只要收到遥控器信号就测试通过
	if(com_flag &0x02)
	{
		norm_sensor_true_reg |= (1 << NORM_SENSOR_REMOTE);
		norm_sensor_false_reg |= (1 << NORM_SENSOR_REMOTE);
	}

	if(com_flag &0x01)
	{
		norm_sensor_true_reg |= (1 << NORM_SENSOR_DISPLAY);
		norm_sensor_false_reg |= (1 << NORM_SENSOR_DISPLAY);
	}
	n_res = ((uint16_t)(norm_sensor_true_reg & norm_sensor_false_reg) << 8) + (norm_sensor_true_reg & norm_sensor_false_reg);
	//log_printf("n_res:%04X-%02X,%02X\r\n",n_res,norm_sensor_true_reg,norm_sensor_false_reg);
	return n_res;
}





////////整机测试	tm_mode = 1
tm_irmaxmin irmaxmin[7] = {{0,1000},
							{0,3800},
							{0,1200},
							{0,1200},
							{0,1200},
							{0,3800},
							{0,1000}};



tm_irmaxmin irtestdata[7] = {{1000,4200},
							{0,4200},
							{700,4200},
							{500,4200},
							{700,4200},
							{0,4200},
							{1000,4200}};
////////半成品测试 tm_mode = 2
tm_irmaxmin asse_irmaxmin[7] = {{0,1500},
							{0,3800},
							{0,1200},
							{0,1200},
							{0,1200},
							{0,3800},
							{0,1500}};
//tm_irmaxmin asse_irtestdata[7] = {{1000,4200},
tm_irmaxmin asse_irtestdata[7] = {{800,4200},
							{0,3800},
							{700,4200},
							{500,4200},
							{700,4200},
							{0,3800},
							//{1000,4200}};
							{800,4200}};

uint8_t const pn_number[] = {0x10,0x01,0x18,0x45,0x12,0x34,0x56,0x67,0x78,0x90};
static uint8_t bum_order = 0;
static uint8_t bum_over = 0;
static uint8_t bum_err = 0;
void check_bum(void)
{
	#define MAX_C_BUM		20
	uint8_t i =0;

	if(0 == bum_order)
	{
		if(((GPIOE->IDR) & GPIO_Pin_12) )	//L2	
		{
			for(i=0;i<MAX_C_BUM;i++)
			{
				if((GPIOE->IDR & GPIO_Pin_12)==0)
					break;
			}
			if(MAX_C_BUM > i)
				bum_over |= 0x01;
		}

		if(( (GPIOD->IDR) & GPIO_Pin_0))//L1
		{						
			for(i=0;i<MAX_C_BUM;i++)
			{
				if(((GPIOD->IDR) & GPIO_Pin_0)==0)
					break;
			}
			if(MAX_C_BUM > i)
				bum_over |= 0x02;
		}

		if( ((GPIOE->IDR) & GPIO_Pin_5)  )//R1
		{
			for(i=0;i<MAX_C_BUM;i++)
			{
				if((GPIOE->IDR) & GPIO_Pin_5==0)
					break;
			}
			if(MAX_C_BUM > i)
				bum_over |= 0x04;
		}

		if(((GPIOE->IDR) & GPIO_Pin_6)) //R2
		{
			for(i=0;i<MAX_C_BUM;i++)
			{
				if(((GPIOE->IDR) & GPIO_Pin_6) ==0)

					break;
			}
			if(MAX_C_BUM > i)
				bum_over |= 0x08;
		}

		bum_err = bum_over;
		bum_order= 1;
	}
	else if(1 == bum_order)
	{
			if(0 == (bum_over & 0x01))	//L2	
			{
				if(((GPIOE->IDR) & GPIO_Pin_12) ==0)
				{
					for(i=0;i<MAX_C_BUM;i++)
					{
						if((GPIOE->IDR) & GPIO_Pin_12)
							break;
					}
					if(i>=MAX_C_BUM)
					{
						bum_err &= 0xFE;	
						bum_over |= 0x01;
					}
					else
						bum_err |= 0x01;
				}
				else
					bum_err |= 0x01;
			}
			


			if(0 == (bum_over & 0x02))		//L1
			{
				if(((GPIOD->IDR) & GPIO_Pin_0) ==0)
				{
					for(i=0;i<MAX_C_BUM;i++)
					{
						if((GPIOD->IDR) & GPIO_Pin_0)
							break;
					}
					if(i>=MAX_C_BUM)
					{
						bum_err &= 0xFD;	
						bum_over |= 0x02;
					}
					else
						bum_err |= 0x02;
				}
				else
					bum_err |= 0x02;
			}	


			if(0 == (bum_over & 0x04))	//R1
			{
				if(((GPIOE->IDR) & GPIO_Pin_5) ==0)
				{
					for(i=0;i<MAX_C_BUM;i++)
					{
						if((GPIOE->IDR) & GPIO_Pin_5)
							break;
					}
					if(i>=MAX_C_BUM)
					{
						bum_err &= 0xFB;
						bum_over |= 0x04;
					}
					else
						bum_err |= 0x04;
				}
				else
					bum_err |= 0x04;
			}	


			if(0 == (bum_over & 0x08))	//R2
			{
				if(((GPIOE->IDR) & GPIO_Pin_6) ==0)
				{
					for(i=0;i<MAX_C_BUM;i++)
					{
						if((GPIOE->IDR) & GPIO_Pin_6)
							break;
					}
					if(i>=MAX_C_BUM)
					{
						bum_err &= 0xF7;	
						bum_over |= 0x08;
					}
					else
						bum_err |= 0x08;
				}
				else
					bum_err |= 0x08;
			}
	}
}

void bum_com(void)
{
			uint8_t g_err = 0;
			if(bum_err&0x01)
				g_err|=BIT_STA(ERR_LEFT_BUM);
			if(bum_err&0x02)
			{
				g_err|=BIT_STA(ERR_MLEFT_BUM);		
				g_err|=BIT_STA(ERR_MID_BUM);		
			}	
			if(bum_err&0x04)
			{
				g_err|=BIT_STA(ERR_MRIGHT_BUM);
				g_err|=BIT_STA(ERR_MID_BUM);	
			}	
			if(bum_err&0x08)
				g_err|=BIT_STA(ERR_RIGHT_BUM);	

			tx_com_dat(TPC_BUM_SW, 20 , g_err?(TPC_ERR_DIGI_IN):(TPC_ERR_NOERROR), (uint8_t)g_err);//上报进度及错误信息	

}




//整机的测试程序,全局变量延用proc_bdtest_task(),因为两个函数不会同时被使用
//心跳包当中要实时上报各种开关量的状态,以用作显示,开关量不会在测试模式中专门检测
void proc_mactest_task(void)
{
	static uint8_t bd_timer = 0xff;
	static uint8_t bd_heart_timer = 0;
	static uint8_t bd_heart_ct = 0;
	static uint8_t bd_bum_timer = 0;
//	static uint8_t bdtest_sta_old = TPC_STOP_ALL;
//	static uint16_t bd_batcharge_timer = 0;
//	static uint16_t g_sta_bum=0,g_sta_idle=0,g_err=0;
	static uint8_t sw_en=0,sw_un=0;
//	static uint16_t test_ground[3][2] = {0};
	static uint8_t cc=0,bot=0;
	int32_t k1,k2;
	uint8_t charge_result = 1;

	get_irda(&sys->gSta);
	uint8_t tmp_cc=0;

	if(TIM5->CNT < 50000)//50ms定时
		return;
	TIM5->CNT = 0;
	get_mactest_sensors();
	if(bd_heart_timer ++ >= 15)//发送心跳包,每秒一个
	{
		uint16_t n_sensors;
		bd_heart_timer = 0;
		n_sensors = get_mactest_sensors();
		#if BDTEST_LOGPRINT == 0
		tx_com_beep(bd_heart_ct ++, (uint8_t)(n_sensors >> 8), (uint8_t)(n_sensors & 0x00ff),sys->g_sta,&sys->g_buton[0][0]);
		#endif
		//log_printf("beep:%04X\r\n",n_sensors);
	}

	switch(bdtest_sta)
	{
		case TPC_STOP_ALL:
			ny3p_play(VOICE_FAC_MODE);
			delay_ms(500);
			ny3p_play(VOICE_FAC_MODE);
			delay_ms(500);
			
			MOTOR_POWER_OFF_NPRI();//关闭电机电源
			TargetSysReset();
			break;
		case TPC_START_ALL://这里要检查陀螺仪是否正常/与测试板的通信是否正常
			//MOTOR_CTRL(1000, 1000, 300, 0);
			MOTOR_POWER_OFF_NPRI();
			bdtest_sta = TPC_MT_FAN;//TPC_MT_FAN;//TPC_BUM_SW;//TPC_MT_FAN;//TPC_IR_BOT;//TPC_MT_FAN;//TPC_SMT_L;;//风机开机时序的原因，需要先开风机
			bdtest_sta_child = 0;
			navigat->distance = 0;
			norm_sensor_false_reg = norm_sensor_true_reg = 0;
			bum_order = 0;
			bum_over = 0;
			bum_err = 0;
			////set_remote_flag(0xff);
			#if BDTEST_LOGPRINT
			log_printf(">>>>>>>>>>>>\r\n");
			#endif
			//tx_com_dat(test_mac_proc_dat);//上报进度
	
			mac_dat_pro(TPC_STOP_ALL);
			break;
		case TPC_MT_FAN:
			switch(bdtest_sta_child)
			{
				case 0://开始测试风机
					{
						bdtest_sta_child = 1;
						//GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource13);						
						bdtest_fantest = 1;
						tmod_fancd = 0;						
						uint32_t dust_close_adc=0,dust_close_v = 0,dust_open_adc=0,dust_open_v=0;
						TEST_MOTOR_POWER_ON();
						DUST_MOTOR_OFF();	//停止电流		
						//delay_ms(2000);	
						delay_ms(500);	
						for(uint8_t i=0;i<10;i++)
						{
							delay_ms(100);
							dust_close_adc += V_DUST_ADC();
							dust_close_v += LiBat_GetBatVolt();
						}	//未运行电流
						TEST_MOTOR_POWER_ON();
						//初始化风机转速中断的IO口
						DUST_MOTOR_RANK1();				
						delay_ms(500);		//运行电流
						for(uint8_t i=0;i<10;i++)	
						{
							delay_ms(300);
							dust_open_adc += V_DUST_ADC();
							dust_open_v += LiBat_GetBatVolt();
						}	
										
						TEST_MOTOR_POWER_OFF_NPRI();
						DUST_MOTOR_OFF();	//停止电流		
						#if 0
						if(disXY(dust_open_adc,dust_close_adc)>1000)
						{
							tmod_fancd=5000;
						}
						else
							tmod_fancd=0;
						#else
						////if((dust_close_v>dust_open_v) && ((dust_close_v - dust_open_v)>2000) )
						if(dust_close_v>dust_open_v)
							tmod_fancd=5000;
						else
							tmod_fancd=0;

						#endif
						bd_timer = 0;
						bdtest_sta_child = 1;
					}
					break;					
				case 1://读取霍尔数	
					bdtest_fantest = 0;				
					//检测电流  有电流就过
					tx_com_dat(TPC_MT_FAN, 7 , tmod_fancd < 100 ? TPC_ERR_FORWARD:TPC_ERR_NOERROR, 1);//上报进度及错误信息

					bd_timer = 0;
					bdtest_sta_child = 3;
				#if BDTEST_LOGPRINT
					bdtest_sta_child = 2;
					break;
				case 2:				
					TEST_MOTOR_POWER_OFF_NPRI();
					break;
				#endif
				default:
					bdtest_sta = TPC_WHL_L;
					bdtest_sta_child = 0;
					bdtest_fantest = 0;
					break;
			}
			break;
		case TPC_WHL_L://左轮检测
			switch(bdtest_sta_child)
			{
				case 0://开始测试左轮
					bdtest_sta_child = 1;
					TEST_MOTOR_POWER_ON();
					motor.c_left_hw = 0;
					motor_wheel_stop(RIGHT_WHEEL);
					motor_wheel_forward(LEFT_WHEEL,600);

					//////tx_com_dat(TPC_WHL_L, 25 , TPC_ERR_NOERROR, 0);//上报进度
					bd_timer = 0;
					break;
				case 1://向前转
					if(bd_timer ++ < 20)break;
					motor_wheel_stop(LEFT_WHEEL);
					#if BDTEST_LOGPRINT
					log_printf("forward:%d\r\n",motor.c_left_hw);
					#endif
					if(motor.c_left_hw < 200)//霍尔数不足
					{
						tx_com_dat(TPC_WHL_L, 14, TPC_ERR_FORWARD, 0);
						#if BDTEST_LOGPRINT
						log_printf("TPC_WHL_L:FAIL\r\n");
						#else
						bdtest_sta = TPC_WHL_R;//跳到下一个步骤
						bdtest_sta_child = 0;
						break;
						#endif
					}
					//////tx_com_dat(TPC_WHL_L, 40, TPC_ERR_NOERROR, 0);
					bdtest_sta_child = 2;
					//MOTOR_POWER_ON();
					motor.c_left_hw = 0;
					motor_wheel_backward(LEFT_WHEEL,600);
					bd_timer = 0;
					break;
				case 2://向后转
					if(bd_timer ++ < 20)break;
					motor_wheel_stop(LEFT_WHEEL);

					#if BDTEST_LOGPRINT
					log_printf("back:%d\r\n",motor.c_left_hw);
					#endif
					if(0)////(motor.c_left_hw < 200)
					{
						//MOTOR_POWER_OFF_NPRI();
						//motor_wheel_forward(LEFT_WHEEL,GO_FORWARD,600);
						tx_com_dat(TPC_WHL_L, 14, TPC_ERR_BACK, 0);//上报电机反向不转

						#if BDTEST_LOGPRINT
						log_printf("TPC_WHL_L:FAIL\r\n");
						#else
						bdtest_sta = TPC_WHL_R;//跳到下一个步骤
						bdtest_sta_child = 0;
						break;
						#endif
					}
					tx_com_dat(TPC_WHL_L, 14, TPC_ERR_NOERROR, 0);//上报进度

					#if BDTEST_LOGPRINT
					delay_ms(50);
					bdtest_sta_child = 3;
					motor_wheel_stop(LEFT_WHEEL);
					log_printf("TPC_WHL_L STOP!\r\n");
					#else
					bdtest_sta = TPC_WHL_R;//跳到下一个步骤
					bdtest_sta_child = 0;
					#endif
					//MOTOR_POWER_ON();
					motor.c_left_hw = 0;

					bd_timer = 0;
					tmd_rxbd_dat.unread_flag = 0;//将数据接收的缓存未读标志清0
					break;
				#if BDTEST_LOGPRINT
				case 3:
					log_printf("TPC_WHL_L STOP!\r\n");
					motor_wheel_stop(LEFT_WHEEL);
					//MOTOR_POWER_OFF_NPRI();
					break;
				#endif
				default:
					bdtest_sta = TPC_WHL_R;
					bdtest_sta_child = 0;
					motor_wheel_stop(LEFT_WHEEL);
					//MOTOR_POWER_OFF_NPRI();
					break;
			}
			break;
		case TPC_WHL_R://右轮检测
			switch(bdtest_sta_child)
			{
				case 0://开始测试左轮
					bdtest_sta_child = 1;
					TEST_MOTOR_POWER_ON();
					motor.c_right_hw = 0;
					motor_wheel_stop(LEFT_WHEEL);
					motor_wheel_forward(RIGHT_WHEEL,600);

					//////tx_com_dat(TPC_WHL_R, 25 , TPC_ERR_NOERROR, 0);//上报进度
					bd_timer = 0;
					break;
				case 1://向前转
					if(bd_timer ++ < 20)break;
					motor_wheel_stop(RIGHT_WHEEL);
					#if BDTEST_LOGPRINT
					log_printf("forward:%d\r\n",motor.c_right_hw);
					#endif
					if(motor.c_right_hw < 200)//霍尔数不足
					{
						tx_com_dat(TPC_WHL_R, 21, TPC_ERR_FORWARD, 0);
						#if BDTEST_LOGPRINT
						log_printf("TPC_WHL_R:FAIL\r\n");
						#else
						bdtest_sta = TPC_MT_MID;//跳到下一个步骤
						bdtest_sta_child = 0;
						break;
						#endif
					}
					//////tx_com_dat(TPC_WHL_R, 40, TPC_ERR_NOERROR, 0);
					bdtest_sta_child = 2;
					//MOTOR_POWER_ON();
					motor.c_right_hw = 0;
					motor_wheel_backward(RIGHT_WHEEL,600);
					bd_timer = 0;
					break;
				case 2://向后转
					if(bd_timer ++ < 20)break;
					//motor_wheel_stop(RIGHT_WHEEL);
					motor_wheel_stop(DOUBLE_WHEEL);

					#if BDTEST_LOGPRINT
					log_printf("back:%d\r\n",motor.c_right_hw);
					#endif
					if(0)////(motor.c_right_hw < 200)
					{
						//MOTOR_POWER_OFF_NPRI();
						//motor_wheel_forward(LEFT_WHEEL,GO_FORWARD,600);
						tx_com_dat(TPC_WHL_R, 21, TPC_ERR_BACK, 0);//上报电机反向不转
						motor_wheel_stop(DOUBLE_WHEEL);

						#if BDTEST_LOGPRINT
						log_printf("TPC_WHL_R:FAIL\r\n");
						#else
						bdtest_sta = TPC_MT_MID;//跳到下一个步骤
						bdtest_sta_child = 0;
						break;
						#endif
					}
					tx_com_dat(TPC_WHL_R, 21, TPC_ERR_NOERROR, 0);//上报进度

					#if BDTEST_LOGPRINT
					delay_ms(50);
					bdtest_sta_child = 3;
					motor_wheel_stop(DOUBLE_WHEEL);
					log_printf("TPC_WHL_L STOP!\r\n");
					#else
					bdtest_sta_child = 0;
					bdtest_sta = TPC_MT_MID;//跳到下一个步骤
					#endif
					//MOTOR_POWER_ON();
					motor.c_right_hw = 0;

					bd_timer = 0;
					tmd_rxbd_dat.unread_flag = 0;//将数据接收的缓存未读标志清0
					motor_wheel_stop(DOUBLE_WHEEL);
					break;
				#if BDTEST_LOGPRINT
				case 3:
				break;
				#endif
				default:
					bdtest_sta = TPC_MT_MID;//TPC_SMT_L;
					bdtest_sta_child = 0;
					motor_wheel_stop(DOUBLE_WHEEL);
					//MOTOR_POWER_OFF_NPRI();
					break;
			}
			break;
		case TPC_SMT_L://测试左边刷电机
			switch(bdtest_sta_child&0x0f)
			{
				case 0://开始测试左轮
					bdtest_sta_child = 1;
					TEST_MOTOR_POWER_ON();
					SET_RSIDE_MOTER(1000);
					SET_LSIDE_MOTER(BDTEST_SMT_PWMVALUE);
					//////tx_com_dat(TPC_SMT_L, 25 , TPC_ERR_NOERROR, 0);//上报进度
					//////tx_dp_dat(TBD_SMT_L, 1, 0);//下发左边刷开始霍尔计数,即原霍尔数清0
					//////tx_dp_dat(TBD_MTOC_SMT_L, 0, 0);//下发左后轮过载测试,将短路的电阻断开
					bd_timer = 0;
					break;
				case 1://读取霍尔数
					if(bd_timer ++ < 20)break;
					//////tx_com_dat(TPC_SMT_L, 50 , TPC_ERR_NOERROR, 0);//上报进度
					//////tx_dp_dat(TBD_SMT_L, 0, 0);//下发左边刷停止霍尔计数,即原霍尔数不清0
					//////tx_dp_dat(TBD_SMT_L, 3, 0);//下发左边刷停止霍尔计数,即原霍尔数不清0
					bd_timer = 0;
					bdtest_sta_child = 2;
					break;
				case 2:
					if(bd_timer ++ < 20 && !tmd_rxbd_dat.unread_flag)break;//超时未收到数据
					bd_timer = 0;
					{
						tmd_rxbd_dat.unread_flag = 0;
					}
					//log_printf("get mtoc_smt_l:%d\r\n",tmd_rxbd_dat.mtoc_smt_l);
					{
						uint8_t i;//, j = 0;
						uint16_t n_mt_curr;
						
						SET_LSIDE_MOTER(BDTEST_SMT_PWMVALUE);
						delay_ms(10);
						for(i = 0;i < 8;i ++)
						{
							delay_ms(5);
							n_mt_curr = I_SMTL_ADC();
						#if BDTEST_LOGPRINT
							log_printf("curr:%d-%d\r\n",n_mt_curr,i);
						#endif
							////if(n_mt_curr > 600)
							if(n_mt_curr> I_SMTL_CUR_MAX)
							{
								//j ++;
								break;
							}
						}

						tx_com_dat(TPC_SMT_L, 28, i >= 8 /*i < 5*/ ? TPC_ERR_NOERROR:TPC_ERR_OC, 0);//上报进度或上报过流检测错误
						if(i < 5)
						{
						#if 0
							//bdtest_sta = TPC_SMT_R;//跳到下一个步骤
							bdtest_sta_child = 4;
							log_printf("*****\r\n");
						#else
							bdtest_sta = TPC_SMT_R;//跳到下一个步骤
							bdtest_sta_child = 0;
						#endif
							//break;
						}
					}
				
					//////tx_dp_dat(TBD_MTOC_SMT_L, 0, 0);//下发左后轮过载测试,将短路的电阻断开
					//bdtest_sta = TPC_WHL_R;
					//bdtest_sta_child = 0;
					//MOTOR_POWER_OFF_NPRI();
				#if 0
					break;
				case 4:
					log_printf("^");
					SET_LSIDE_MOTER(1000);
					MOTOR_POWER_OFF_NPRI();
					break;
				#endif
				default:
					bdtest_sta = TPC_SMT_R;
					bdtest_sta_child = 0;
					SET_LSIDE_MOTER(1000);
					TEST_MOTOR_POWER_OFF_NPRI();
					break;
			}
			break;
		case TPC_SMT_R://测试打右边刷电机
			switch(bdtest_sta_child&0x0f)
			{
				case 0:
					bdtest_sta_child = 1;
					TEST_MOTOR_POWER_ON();
					SET_LSIDE_MOTER(1000);
					SET_RSIDE_MOTER(BDTEST_SMT_PWMVALUE);
					//////tx_com_dat(TPC_SMT_R, 25 , TPC_ERR_NOERROR, 0);//上报进度
					//////tx_dp_dat(TBD_MTOC_SMT_R, 0, 0);//下发左后轮过载测试,将短路的电阻断开
					//////tx_dp_dat(TBD_SMT_R, 1, 0);//下发左边刷开始霍尔计数,即原霍尔数清0
					bd_timer = 0;
					break;
				case 1://读取霍尔数
					if(bd_timer ++ < 20)break;
					//////tx_com_dat(TPC_SMT_R, 50 , TPC_ERR_NOERROR, 0);//上报进度
					//////tx_dp_dat(TBD_SMT_R, 3, 0);//下发左边刷停止霍尔计数,即原霍尔数不清0
					bd_timer = 0;
					bdtest_sta_child = 2;
					break;
				case 2:
				{
					if(bd_timer ++ < 20 && !tmd_rxbd_dat.unread_flag)break;//超时未收到数据
					bd_timer = 0;
					{
						tmd_rxbd_dat.unread_flag = 0;
					}

					
						uint8_t i;//, j = 0;
						uint16_t n_mt_curr;
						
						SET_RSIDE_MOTER(BDTEST_SMT_PWMVALUE);
						delay_ms(10);
						for(i = 0;i < 8;i ++)
						{
							delay_ms(5);
							n_mt_curr = I_SMTR_ADC();
						#if BDTEST_LOGPRINT
							log_printf("curr:%d-%d\r\n",n_mt_curr,i);
						#endif
							////if(n_mt_curr > 600)
							if(n_mt_curr > I_SMTR_CUR_MAX)
							{
								//j ++;
								break;
							}
							//log_printf("i:%d\r\n",i);
						}

						tx_com_dat(TPC_SMT_R, 35, i >= 8/*i < 5*/ ? TPC_ERR_NOERROR:TPC_ERR_OC, 0);//上报进度或上报过流检测错误
						if(i < 5)
						{
						#if 0
							//bdtest_sta = TPC_SMT_R;//跳到下一个步骤
							bdtest_sta_child = 4;
							log_printf("**6**%d\r\n",i);
						#else
							bdtest_sta = TPC_MT_MID;//跳到下一个步骤
							bdtest_sta_child = 0;
						#endif
							//break;
						}
					
					
					//////tx_dp_dat(TBD_MTOC_SMT_R, 0, 0);//下发左后轮过载测试,将短路的电阻断开
					//bdtest_sta = TPC_WHL_R;
					//bdtest_sta_child = 0;
					//MOTOR_POWER_OFF_NPRI();
				}
				break;
				default:
					bdtest_sta = TPC_MT_MID;
					bdtest_sta_child = 0;
					SET_RSIDE_MOTER(1000);
					TEST_MOTOR_POWER_OFF_NPRI();
					break;
			}
			break;
		case TPC_MT_MID://测试中扫电机		//2A中扫电机检测电流   2B中扫电机检测码盘
			switch(bdtest_sta_child&0x0f)
			{
				case 0:
				{
					uint32_t mid_close_adc=0,mid_open_adc=0;
					bdtest_sta_child = 1;
					navigat->gun_cnt=0;		
					mac_dat_pro(TPC_STOP_ALL);
					TEST_MOTOR_POWER_ON();
					SET_MID_MOTER(300);////800
							
					delay_ms(1000);		//运行电流
					for(uint8_t i=0;i<10;i++)	
					{
						delay_ms(100);
						mid_open_adc += I_MID_ADC();
					}										
					TEST_MOTOR_POWER_OFF_NPRI();
					SET_MID_MOTER(1000);
					////DUST_MOTOR_OFF();	//停止电流		
					////SET_MID_MOTER(0);////800
					delay_ms(500);		
					for(uint8_t i=0;i<10;i++)
					{
						delay_ms(100);
						mid_close_adc += I_MID_ADC();
					}	//未运行电流						
					if(disXY(mid_open_adc,mid_close_adc)<1000 &&disXY(mid_open_adc,mid_close_adc)>200)
					{
						////tx_com_dat(TPC_MT_MID, 42, TPC_ERR_NOERROR, 0);//OK
						bdtest_sta_child = 1;
					}
					else if(disXY(mid_open_adc,mid_close_adc)>=1000)
					{
						tx_com_dat(TPC_MT_MID, 42, TPC_ERR_FORWARD, 0);//过流
						tmod_fancd=0;
						bdtest_sta = TPC_IR_FRT;//跳到下一个步骤
						bdtest_sta_child = 0;
					}
					else
					{
						tx_com_dat(TPC_MT_MID, 43, TPC_ERR_FORWARD, 0);//不转
						tmod_fancd=0;
						bdtest_sta = TPC_IR_FRT;//跳到下一个步骤
						bdtest_sta_child = 0;
					}									
				}
					break;
				case 1:				
				default:
				{
					uint32_t mid_open_adc_d =0;
					////bdtest_sta_child = 0;
					navigat->gun_cnt=0;		
					mac_dat_pro(TPC_MT_MID);
					delay_ms(1000);
					delay_ms(1000);		//运行电流
					for(uint8_t i=0;i<10;i++)	
					{
						delay_ms(100);
						mid_open_adc_d += I_MID_ADC();
					}										
					TEST_MOTOR_POWER_OFF_NPRI();
					SET_MID_MOTER(1000);
					mac_dat_pro(TPC_STOP_ALL);
					delay_ms(500);		
					tx_com_dat(TPC_MT_MID, 42, TPC_ERR_NOERROR, 0);//OK
					tmod_fancd=0;					
					//////tx_dp_dat(TBD_MTOC_MT_MID, 0, 0);//下发左后轮过载测试,将短路的电阻断开
					bdtest_sta = TPC_IR_FRT;//跳到下一个步骤
					bdtest_sta_child = 0;		

					GPIO_SetBits(PORT_IR_CTRL,PIN_IR_CTRL);
					delay_ms(500);
					//mac_dat_pro(TPC_IR_FRT);	
					//	delay(500);
				}	
				break;
			}
			break;			
		case TPC_IR_FRT://前撞测距(碰撞)红外检测
			switch(bdtest_sta_child)
			{
				case 0:	
					{
						uint8_t ccc;			
						//#if BDTEST_LOGPRINT
						///uint8_t bot;
						//#endif
						//int32_t k1,k2;
						uint8_t i,j;
						sys->c_ir_adc = 0;
						for(ccc = 0;ccc < 9;ccc ++)
						{
							for(i=0;i<10;i++)
							{
								//sys->m_sta[i][ccc] = adc_converted_value[i];
								if(ccc == 0)
									sys->m_sta[i][ccc] = 0;
								else
									sys->m_sta[i][ccc] = adc_converted_value[i];
								//log_printf(",%d",sys->m_sta[i][sys->c_ir_adc] );
							}
							if(ccc<=3)
							{
								GPIO_ResetBits(PORT_IR_CTRL,PIN_IR_CTRL);	//低电平，关灯
								GPIO_ResetBits(GPIOD,GPIO_Pin_14);	//低电平，关灯
								
							}
							else
							{
								GPIO_SetBits(PORT_IR_CTRL,PIN_IR_CTRL);	
								GPIO_SetBits(GPIOD,GPIO_Pin_14);	
							}
							delay(1);
							
						}
						#if 0
						for(i= 0;i < 9;i ++)
						{
							log_printf("%d\r\n",sys->m_sta[0][i]);
						}
						#endif
						cc /*= bot */= 0;//cc用来记录前撞红外的检测结果,j用来记录对地红外的检测结果
						//log_printf("\r\ncc>>%d\r\n",cc);
						for(i=0;i<10;i++)//把对地红外也算进来,目前检测对地红外在这里获取数据，检测环境为一个定高的障碍物
						{
							k1 = k2 = 0;
							for(j=0;j<4;j++)
								k1 +=sys->m_sta[i][j];
							for(j=5;j<9;j++)
								k2 +=sys->m_sta[i][j];

							if(i < 7)//前撞红外
							{
								/*
								if(i == 0 || i == 3 || i == 6)
								{
									sys->g_sta[i] = k1 >> 2;
								}
								else
								*/
								{
									if(k1 > k2 && ((k1 - k2) /4) < 5000)
										sys->g_sta[i] = (k1 - k2) >> 2;
									else
										sys->g_sta[i] = 0;
								}
								//if(sys->g_sta[i] < 500 || sys->g_sta[i] > 3500)//前撞红外,在红定距离读取值超出范围认为不合格
								//if(sys->g_sta[i] < 1000 || sys->g_sta[i] > 3800)//前撞红外,在红定距离读取值超出范围认为不合格
								//哪一路没有使用,将哪一路数据填充为中间值
								if(tm_mode == 1)		//整机测试
								{
									if((sys->g_sta[i] < irmaxmin[i].min) || (sys->g_sta[i] > irmaxmin[i].max))
									{
										cc |= BIT8_STA(i);
									}
								}
								else if(tm_mode == 2)	//半成品
								{
									//if(i == 0 || i == 4)
									{
										if((sys->g_sta[i] < asse_irmaxmin[i].min) || (sys->g_sta[i] > asse_irmaxmin[i].max))
										{
											cc |= BIT8_STA(i);
										}
									}
								}
										//log_printf("\r\n%02X-%d\r\n",cc,i);
							}
							else
							{//这个CASE只获取对地红外值，到TPC_IR_BOT中判断对地红外是否正常
								//sys->g_buton[0][i-7] = k2 >> 2; //sys->g_sta[i];
								//sys->g_buton[1][i-7] = k1 >> 2;								
								/*if(sys->g_buton[0][i-7] > 300)//近端红外
								{
									bot |= 1 << (i - 7);
								}
								if(sys->g_buton[1][i-7] > 1500)//远端红外
								{
									bot |= 1 << (i - 7 + 3);
								}*/
							}
							//log_printf("%d\t%d\r\n",i,sys->g_sta[i]);
							//delay(5);
						}
						tmp_cc = cc;
						if(cc)
						{
							if(tmp_cc&BIT8_STA(2))
								cc |= BIT8_STA(4);
							else
								cc &= ~(BIT8_STA(4));
							if(tmp_cc&BIT8_STA(4))
								cc |= BIT8_STA(2);
							else
								cc &= ~(BIT8_STA(2));				
							tx_com_dat(TPC_IR_FRT, 50 , TPC_ERR_IRLED, cc);//上报进度及错误信息
						#if BDTEST_LOGPRINT
							delay(50);
							log_printf("\r\ncc------:%02X:%d(%d-%d),%d(%d-%d)\r\n",cc,sys->g_sta[0],asse_irmaxmin[0].min,asse_irmaxmin[0].max,sys->g_sta[6],asse_irmaxmin[6].min,asse_irmaxmin[6].max);
							#endif
							////bdtest_sta_child = 1;				
							////mac_dat_pro(TPC_IR_FRT);	
							//delay(500);
							////break;
						}
						/*else
						{
							bdtest_sta_child = 1;						
							mac_dat_pro(TPC_IR_FRT);	
							//delay(500);
							break;
						}
						*/
						cc = tmp_cc;	//恢复cc

						#if BDTEST_LOGPRINT
						delay(500);
						log_printf("\r\nir_frt:\r\n");
						for(i = 0;i < 7;i ++)
						{
							log_printf("\t%d",sys->g_sta[i]);
						}
						log_printf("\r\n\t cc:%02X\r\nir_bot:\r\n",cc);
						for(i = 0;i < 3;i ++)
						{
							log_printf("\t%d",sys->g_buton[0][i]);
						}
						log_printf("\r\n");
						for(i = 0;i < 3;i ++)
						{
							log_printf("\t%d",sys->g_buton[1][i]);
						}
						log_printf("\r\n");
						#endif
					}					
					bdtest_sta_child = 1;						
					mac_dat_pro(TPC_IR_FRT);	
					delay(500);
					
					break;
				case 1:		//遮挡后测试
					{
						uint8_t ccc;
						//#if BDTEST_LOGPRINT
						///uint8_t bot;
						//#endif
						//int32_t k1,k2;
						uint8_t i,j;
						
						
						for(ccc = 0;ccc < 9;ccc ++)
						{			
							if(ccc<=3)
							{
								GPIO_ResetBits(PORT_IR_CTRL,PIN_IR_CTRL);	//低电平，关灯
								GPIO_ResetBits(GPIOD,GPIO_Pin_14);	//低电平，关灯
								
							}
							else
							{
								GPIO_SetBits(PORT_IR_CTRL,PIN_IR_CTRL);	
								GPIO_SetBits(GPIOD,GPIO_Pin_14);	
							}
							delay(1);
							for(i=0;i<10;i++)
							{
								if(ccc == 0)
									sys->m_sta[i][ccc] = 0;
								else
									sys->m_sta[i][ccc] = adc_converted_value[i];
								//log_printf(",%d",sys->m_sta[i][sys->c_ir_adc] );
							}	
						}
						
						bot = 0;//cc用来记录前撞红外的检测结果,j用来记录对地红外的检测结果
						//cc = 0;
						for(i=0;i<10;i++)//把对地红外也算进来,目前检测对地红外在这里获取数据，检测环境为一个定高的障碍物
						{
							k1 = k2 = 0;
							for(j=0;j<4;j++)
								k1 +=sys->m_sta[i][j];
							for(j=5;j<9;j++)
								k2 +=sys->m_sta[i][j];

							if(i < 7)//前撞红外
							{
								/*
								if(i == 0 || i == 3 || i == 6)
								{
									sys->g_sta[i] = k1 >> 2;
								}
								else
								*/
								{
									if(k1 > k2 && ((k1 - k2) /4) < 5000)
										sys->g_sta[i] = (k1 - k2) >> 2;
									else
										sys->g_sta[i] = 0;
								}
								//if(sys->g_sta[i] < 500 || sys->g_sta[i] > 3500)//前撞红外,在红定距离读取值超出范围认为不合格
								//if(sys->g_sta[i] < 1000 || sys->g_sta[i] > 3800)//前撞红外,在红定距离读取值超出范围认为不合格
								//哪一路没有使用,将哪一路数据填充为中间值
								if(tm_mode == 1)		//整机测试
								{
									if((sys->g_sta[i] < irtestdata[i].min) || (sys->g_sta[i] > irtestdata[i].max))
									{
										cc |= BIT8_STA(i);
									}
								}
								else if(tm_mode == 2)	//半成品
								{
									if((sys->g_sta[i] < asse_irtestdata[i].min) || (sys->g_sta[i] > asse_irtestdata[i].max))
									{
										cc |= BIT8_STA(i);
									}
								}
							}
							else
							{
								sys->g_buton[0][i-7] = k2 >> 2; //sys->g_sta[i];
								//sys->g_buton[1][i-7] = k1 >> 2;
								if(i==7 || i==9)
								{
									if(sys->g_buton[0][i-7] < 1800)//近端红外								
										bot |= BIT8_STA(i - 7);	
								}	
								///if(sys->g_buton[1][i-7] < 1800)//远端红外								
								//	bot |= 1 << (i - 7 + 3);								
							}
							//log_printf("%d\t%d\r\n",i,sys->g_sta[i]);
						}

						//中左和中右位置反了,需要转成正确的位置上报给上位机软件
						#if 1
						tmp_cc = cc;
						if(tmp_cc&BIT8_STA(2))
							cc |= BIT8_STA(4);
						else
							cc &= ~(BIT8_STA(4));
						if(tmp_cc&BIT8_STA(4))
							cc |= BIT8_STA(2);
						else
							cc &= ~(BIT8_STA(2));						
						#endif
						tx_com_dat(TPC_IR_FRT, 100 , cc?TPC_ERR_IRLED:TPC_ERR_NOERROR, cc);//上报进度及错误信息

						if(bot)	//遮挡情况下,有错误上报
							tx_com_dat(TPC_IR_BOT, 100 , TPC_ERR_IRLED, bot);//上报进度及错误信息
						#if BDTEST_LOGPRINT
							delay(50);
							log_printf("\r\ncc------:%02X:%d(%d-%d),%d(%d-%d)\r\n",cc,sys->g_sta[0],asse_irmaxmin[0].min,asse_irmaxmin[0].max,sys->g_sta[6],asse_irmaxmin[6].min,asse_irmaxmin[6].max);
							#endif
							////测试
							//{
							//	tx_com_dat(TPC_IR_BOT, (sys->g_buton[0][0] /100) , TPC_ERR_IRLED, (sys->g_buton[0][0] %100));//上报进度及错误信息
							//	tx_com_dat(TPC_IR_BOT, (sys->g_buton[0][2] /100) , TPC_ERR_IRLED, (sys->g_buton[0][2] %100));//上报进度及错误信息
							//	bot = 0;
							//}
						#if BDTEST_LOGPRINT
						delay(500);
						log_printf("\r\n case 2ir_frt:\r\n");
						for(i = 0;i < 7;i ++)
						{
							log_printf("\t%d",sys->g_sta[i]);
						}
						log_printf("\r\n\t cc:%02X\r\nir_bot:\r\n",cc);
						for(i = 0;i < 3;i ++)
						{
							log_printf("\t%d",sys->g_buton[0][i]);
						}
						log_printf("\r\n");
						for(i = 0;i < 3;i ++)
						{
							log_printf("\t%d",sys->g_buton[1][i]);
						}
						log_printf("\r\n");
						#endif

							///测试结束
						bdtest_sta_child = 2;													
					}
				break;
				case 2:
				bdtest_sta_child = 0;
				bdtest_sta = TPC_IR_BOT;
					mac_dat_pro(TPC_STOP_ALL);	
				break;
				default:
					break;
			}
			break;
		case TPC_IR_BOT://前撞测距(碰撞)红外检测,对地红外在这里一起检查
			switch(bdtest_sta_child)
			{
				case 0:		//检测遮挡,正常状态
					{
						bdtest_sta_child = 3;
						///tm_ir_bot_k1 /*= tm_ir_bot_k2 */= 0;
						
//						tm_ir_bot_cc = 4;
						//tx_com_dat(TPC_IR_BOT, 50 , TPC_ERR_NOERROR, 0);//上报开始对地红外测试,等待用户将近端
						delay_ms(500);
						k1 = k2 = 0;
						for(uint8_t ccc = 0;ccc < 4;ccc ++)
						{
							k1 += adc_converted_value[7];
							k2 += adc_converted_value[9];
							delay(20);
						}
												
						
						k1 /=4;
						k2 /=4;
						if(k1 > 1500) //左边
						{
							bot |= BIT8_STA(0);
						}
						if(k2 > 1500) //左边
						{
							bot |= BIT8_STA(2);
						}
						/*
						//for(i=0;i<10;i++)//把对地红外也算进来,目前检测对地红外在这里获取数据，检测环境为一个定高的障碍物
						{
							//k1 = k2 = 0;			
			
							{//这个CASE只获取对地红外值，到TPC_IR_BOT中判断对地红外是否正常
								sys->g_buton[0][i-7] = k2 >> 2; //sys->g_sta[i];
								//sys->g_buton[1][i-7] = k1 >> 2;
								if(sys->g_buton[0][i-7] > 1500)//近端红外
								{
									bot |= 1 << (i - 7);
								}
								//if(sys->g_buton[1][i-7] > 1500)//远端红外
								//{
								//	bot |= 1 << (i - 7 + 3);
								//}
							}
							//log_printf("%d\t%d\r\n",i,sys->g_sta[i]);
						}
						*/
						tx_com_dat(TPC_IR_BOT, 50 , bot ? TPC_ERR_IRLED:TPC_ERR_NOERROR, bot);//上报进度及错误信息

				
						bdtest_sta = TPC_BUM_SW;
						bdtest_sta_child = 0;
					}
					break;
				case 5:
					break;
				default:
					bdtest_sta = TPC_BUM_SW;
					bdtest_sta_child = 0;
					bd_timer = 0;
					break;
			}
			break;
		case TPC_BUM_SW:
		{
		
					bdtest_sta = TPC_SWSENSOR;					
					bdtest_sta_child = 0;
					bdtest_fantest = 0;
						
		}	
			break;
		case TPC_SWSENSOR://读取其他开关式传感器数据
		{
				
			//switch(bdtest_sta_child)
			{
				
					mac_dat_pro(TPC_STOP_ALL);
					delay_ms(1000);
												
					if(LEFT_MOTOR_LEAVE()==0)			//非出发状态	
						sw_un &= ~(BIT8_STA(SWSENSOR_LEV_L));						
					else
						sw_un |= BIT8_STA(SWSENSOR_LEV_L);	
					if(RIGHT_MOTOR_LEAVE()==0)		//非出发状态			
						sw_un &= ~(BIT8_STA(SWSENSOR_LEV_R));	
					else
						sw_un |= BIT8_STA(SWSENSOR_LEV_R);	
		
					
					mac_dat_pro(TPC_MT_MID);//区分左右中
					delay_ms(1000);
			/*
					//(超声波/虚拟墙/左落轮/右落轮)
					if(READ_DATA() == 0)
						sw_en &= ~(BIT8_STA(SWSENSOR_ULTRA));				//非触发状态
					else						
						sw_en |= BIT8_STA(SWSENSOR_ULTRA);	
					if(!READ_VWALL_DET())							//虚拟墙记录非触发状态
						sw_en &= ~(BIT8_STA(SWSENSOR_VW));	
					else 
						sw_en |= BIT8_STA(SWSENSOR_VW);	
			*/		
		#if 1				
					if(LEFT_MOTOR_LEAVE() == 0) 		//非出发状态			
						sw_en &= ~(BIT8_STA(SWSENSOR_LEV_L));	
					else
						sw_en |= BIT8_STA(SWSENSOR_LEV_L);
					if(RIGHT_MOTOR_LEAVE() == 0)		//非出发状态	
						sw_en &= ~(BIT8_STA(SWSENSOR_LEV_R));	
					else
						sw_en |= BIT8_STA(SWSENSOR_LEV_R);	
		#endif		
					sw_en|=sw_un;
					if(sw_en)
					{						
						tx_com_dat(TPC_SWSENSOR, 50, TPC_ERR_DIGI_IN, sw_en);//上报进度及错误信息
					}
					else
						tx_com_dat(TPC_SWSENSOR, 100, TPC_ERR_NOERROR, sw_en);//上报进度及错误信息
					//bdtest_sta_child = 3;	
				//break;
				//case 3:
					bdtest_sta = TPC_IRDA_CHRG;
					bdtest_sta_child = 0;
					MOTOR_POWER_OFF_NPRI();
					bdtest_fantest = 0;				
					
			}
		}	
		break;
		case TPC_IRDA_CHRG:
			mac_dat_pro(TPC_STOP_ALL);
			switch(bdtest_sta_child)
			{
			case 0:
			/*
				{
					uint8_t n_irdata[3] = {0};
					read_ir_original_data(n_irdata);//读一次红外,把之前的值清掉
				}
				*/
				GPIO_ResetBits(PORT_IR_CTRL,PIN_IR_CTRL);	//低电平，关灯
				bdtest_sta_child = 1;
				delay_ms(500);
				break;
			case 1:
				{
					uint8_t n_irdata[4] = {0};
					uint8_t n_irres=0;
					//uint8_t n_irknk = 0;
					//uint8_t i,cc;
					////delay_ms(500);
					read_ir_original_data(n_irdata);		
					#if 0
					n_irres = (n_irdata[0] | n_irdata[1] | n_irdata[2]) & 0x01;//取顶灯信号

					if(n_irres == 0x01) 
						n_irres&=~0x01;
					else
						n_irres=0x01;
					#endif	
					
					if(!n_irdata[0])n_irres |= BIT8_STA(0);
					if(!n_irdata[1])n_irres |= BIT8_STA(1);
					if(!n_irdata[2])n_irres |= BIT8_STA(2);
					if(!n_irdata[3])n_irres |= BIT8_STA(3);
					if(n_irres == 0)////if(n_irres != 0 )
					{		
						bdtest_sta_child = 0;
//						bd_batcharge_timer = 0;
						bdtest_sta = TPC_GYRO_TST;
						tx_com_dat(TPC_IRDA_CHRG, 100 , TPC_ERR_NOERROR, n_irres);//上报进度及错误信息					
					}
					else//// if(n_irres == 0)
					{
						bdtest_sta_child = 0;
						bdtest_sta = TPC_GYRO_TST;					
						tx_com_dat(TPC_IRDA_CHRG, 100 , TPC_ERR_IRLED, n_irres);//上报进度及错误信息
					}
					delay_ms(500);
				}
			
				break;
			case 2:
			default:
				bdtest_sta = TPC_GYRO_TST;	
				bdtest_sta_child = 0;
				delay_ms(100);
				break;
			}
			break;
		
		case TPC_GYRO_TST:		
			delay_ms(500);
			switch(bdtest_sta_child)
			{
				case 0:
				if(sys->angle>=0 && sys->angle<=360)////if(sys->gyro_init_ok)
					tx_com_dat(TPC_GYRO_TST, 100, TPC_ERR_NOERROR, 0);//上报进度
				else
					tx_com_dat(TPC_GYRO_TST, 50, TPC_ERR_TYRO, 1);//上报进度
				bdtest_sta = TPC_CHRG_TST;
				delay_ms(100);
				break;	
			}	
#if 0			
		case TPC_CD_FRT://测试前轮码盘,前轮码盘的测试电机开机就会转，直接读前轮码盘计数器就可以
			{
				int n_cd_frt = navigat->distance;
				if(n_cd_frt < 0)n_cd_frt = -n_cd_frt;
				tx_com_dat(TPC_CD_FRT, 100 , (n_cd_frt > 20) ? TPC_ERR_NOERROR:TPC_ERR_FORWARD, 0);//上报进度
				//tx_com_dat(TPC_CD_FRT, 100 , TPC_ERR_NOERROR, 0);//上报进度
				bdtest_sta = TPC_CHRG_TST;
				bdtest_sta_child = 0;
				bd_batcharge_timer = 0;
			}
			break;
#endif			
		break;
		case TPC_CHRG_TST://充电测试
			
			delay_ms(1000);
			if(DOCK_DETECT())
			{				
				if((1 == CHARGE_DONE_DETECT()) && (1 == CHARGE_ING_DETECT()))
				{
					tx_com_dat(TPC_CHRG_TST, 63, TPC_ERR_DOCK, 2);//上报进度
					charge_result= 0;
				}
			}
				else
			{
					tx_com_dat(TPC_CHRG_TST, 63, TPC_ERR_DOCK, 1);//上报进度
				charge_result= 0;
			}	

			delay_ms(1000);
			if(0 == EXTERAL_AC_DETECT())
			{
				tx_com_dat(TPC_CHRG_TST, 63, TPC_ERR_DCJACK, 1);//上报进度				
				charge_result= 0;
			}				
			if(1 == charge_result)
			{
				tx_com_dat(TPC_CHRG_TST, 63, TPC_ERR_NOERROR, 0);//上报进度
			}				
			bdtest_sta = TPC_HEART_BEEP;	
			break;
		case TPC_HEART_BEEP:
			delay_ms(20);
			check_bum();
			if(20 < bd_bum_timer++)
			{
				bd_bum_timer = 0;
				bum_com();
			}
		break;
		default:
			bdtest_sta = TPC_HEART_BEEP;
			delay_ms(2);
			break;
	}
//	bdtest_sta_old = bdtest_sta;
}




//上位机的通信程序
void proc_com_test_mode(uint8_t *buf,uint8_t len)
{
	TEST_PROC *proc_dat;

	proc_dat = (TEST_PROC *)buf;

	//log_printf("\r\n\r\n");
	//data_print(buf,len);
	switch(proc_dat->code)
	{
		case TPC_STOP_ALL://退出测试模式,系统重启
			tx_msg(buf,sizeof(TEST_PROC));
			bdtest_sta = TPC_STOP_ALL;
			if(tm_mode == TM_MODE_BOARD)
				tx_dp_dat(TBD_STOP_ALL, 0, 0);
			mac_dat_pro(TPC_STOP_ALL);		
			//TargetSysReset();
			break;
		case TPC_START_ALL://进入测试模式
			//if(bdtest_sta == TPC_STOP_ALL)
			{
				bdtest_sta = TPC_START_ALL;
				tx_msg(buf,sizeof(TEST_PROC));
				if(tm_mode == TM_MODE_BOARD)
					tx_dp_dat(TBD_START_ALL, 0, 0);					
				ny3p_play(VOICE_FAC_MODE);
				delay_ms(500);
		
				
			}
			break;
		case TPC_RESET://进入测试模式
			tx_com_dat(TPC_RESET, 0 , TPC_ERR_NOERROR, 0);
			TargetSysReset();
			break;
		case TPC_IR_BOT://对地红外
			if(proc_dat->t_ret_value < 5)
				bdtest_sta_child = proc_dat->t_ret_value;
			tx_com_dat(TPC_IR_BOT, proc_dat->t_progress , TPC_ERR_NOERROR, proc_dat->t_ret_value);
			break;
		case TPC_CHRG_TST://充电测试
			if(proc_dat->t_ret_value < 5)
				bdtest_sta_child = proc_dat->t_ret_value;
			tx_com_dat(TPC_IR_BOT, proc_dat->t_progress, TPC_ERR_NOERROR, proc_dat->t_ret_value);
			break;
		case TPC_SETSN://PC向主板写入SN码
			//delay_ms(100);
			if(len < 20)//如果数据长度小于20，将被视作无效数据
			{
				#if BDTEST_LOGPRINT
				log_printf("error:data len = %d\r\n",len);
				#else
				tx_sn(TPC_SETSN,NULL);
				#endif
				break;
			}
			else
			{
				uint8_t n_chksum;
				n_chksum = get_chksum(buf, SNLENGTH + 1);
				if(n_chksum == buf[22])
				{
					memcpy(cfg->sn,buf+1,21);

					//log_printf("OK!\r\n");
					//sn_print(cfg->sn, 21);
					save_cfg();
					tx_sn(TPC_SETSN,cfg->sn);
					delay_ms(500);
					TargetSysReset();
				}
				else
				{
				#if BDTEST_LOGPRINT
					log_printf("\r\nerror:chksum = %02X-%02X\r\n",n_chksum,buf[22]);
					sn_print(buf + 1, 21);
					log_printf("\r\n");
				#else
					tx_sn(TPC_SETSN,NULL);
				#endif
					break;
				}
			}
			break;
		case TPC_GETSN://主板向PC发送主板的SN码
			//ny3p_play(VOICE_DIDI);
			tx_sn(TPC_GETSN,cfg->sn);
			break;

		case TPC_TRANS_PACK:
			rec_mac_proc_dat.output_sta_1 = ((((unsigned short)(proc_dat->t_res))&0x00ff)<<8) | (((unsigned short)(proc_dat->t_progress))&0x00ff);	
			rec_mac_proc_dat.output_sta_2 = ((unsigned short)(proc_dat->t_ret_value))&0x00ff;
			break;
		default:
			if(proc_dat->code >= TPC_CD_FRT && proc_dat->code <= TPC_MT_FAN)
			{
				bdtest_sta = proc_dat->code;
				bdtest_sta_child = 0;
				tx_msg(buf,sizeof(TEST_PROC));
				if(tm_mode == TM_MODE_BOARD)
					tx_dp_dat(TBD_START_ALL, 0, 0);
				if(proc_dat->code != TPC_CHRG_TST)
				{
					LiBat_ExitChargeMode();
					delay_ms(100);
					if(tm_mode == TM_MODE_BOARD)
						tx_dp_dat(TBD_CHRG_TST, 0, 0);//将24V打到DC插座接口
				}
			}
			break;
	}
}

//向测试板的端口(即显控端口)usart2发送命令
void tx_dp_msg(uint8_t *buff,int len)
{
	uint8_t tmp[2];

	//发数据之前先把未读标记清掉,以免本次读到上一次的标记
	tmd_rxbd_dat.unread_flag = 0;

	tmp[0] = 0x7E;
	tmp[1] = 0x5D;
	usart3_write(tmp,2);
	usart3_write(buff, len);
	tmp[0] = 0x6D;
	tmp[1] = 0x7D;
	usart3_write(tmp,2);

}


//向上位机发送老化数据,此函数用在机器老化测试中,机器一直在做沿边行走，并上报机器的碰撞数据及一些故障错误
void tx_com_burnin(uint8_t side_dir, uint16_t timer,uint32_t bum_left_ct,uint32_t ir_left_ct,uint32_t bum_right_ct,uint32_t ir_right_ct)
{
	uint8_t i = 0;
	uint8_t tmp[2];
	TEST_BURNIN_PROC test_proc_dat;

	test_proc_dat.code = TPC_HEART_BEEP;
	test_proc_dat.para = side_dir;
	test_proc_dat.worktimer = timer;
	test_proc_dat.bum_ct[i ++] = bum_left_ct;
	test_proc_dat.bum_ct[i ++] = ir_left_ct;
	test_proc_dat.bum_ct[i ++] = bum_right_ct;
	test_proc_dat.bum_ct[i ++] = ir_right_ct;
	test_proc_dat.fw_ver = FW_VERSION;
	test_proc_dat.bat_volt= LiBat_GetBatVolt();
	test_proc_dat.errorcode = sys->work_errcode;


	tmp[0] = 0x7E;
	tmp[1] = 0x5D;
	usart_write(tmp,2);
	usart_write((uint8_t *)&test_proc_dat, sizeof(TEST_BURNIN_PROC));
	tmp[0] = 0x6D;
	tmp[1] = 0x7D;
	usart_write(tmp,2);
}

//向上位机发送心跳包消息
void tx_com_beep(uint8_t progress, uint8_t error_code, uint8_t ret_value,uint16_t *irvalue,uint16_t *irbuttom)
{
	uint8_t i;
	uint8_t tmp[2];
	TEST_BEEP_PROC test_proc_dat;

	test_proc_dat.code = TPC_HEART_BEEP;
	test_proc_dat.t_progress = progress;
	test_proc_dat.t_res = error_code;
	test_proc_dat.t_ret_value = ret_value;
	test_proc_dat.bat_volt= LiBat_GetBatVolt();
	test_proc_dat.fw_ver = FW_VERSION;
	if(EXTERAL_AC_DETECT())
	{
		test_proc_dat.bat_chstate = 1;
		test_proc_dat.bat_curr = LiBat_GetChargeCurrent();
	}
	else
	{
		test_proc_dat.bat_chstate = 0;
		test_proc_dat.bat_curr = 0;
	}
	for(i = 0;i < 7;i ++)
	{
		test_proc_dat.ir_frt_value[i] = irvalue[i];
	}
	for(;i < 10;i ++)
	{
		test_proc_dat.ir_frt_value[i] = irbuttom[i - 7];
	}

	delay_ms(300);
	tmp[0] = 0x7E;
	tmp[1] = 0x5D;
	usart_write(tmp,2);
	usart_write((uint8_t *)&test_proc_dat, sizeof(TEST_BEEP_PROC));
	tmp[0] = 0x6D;
	tmp[1] = 0x7D;
	usart_write(tmp,2);
}

//向上位机发送消息
void tx_com_dat(uint8_t code, uint8_t progress, uint8_t error_code, uint8_t ret_value)
{
	uint8_t tmp[2];
	TEST_PROC test_proc_dat;

	delay(300);
	test_proc_dat.code = code;
	test_proc_dat.t_progress = progress;
	test_proc_dat.t_res = error_code;
	test_proc_dat.t_ret_value = ret_value;
	//test_proc_dat.fw_ver = FW_VERSION;
	for(uint8_t i=0;i<2;i++)
	{
		tmp[0] = 0x7E;
		tmp[1] = 0x5D;
		usart_write(tmp,2);
		usart_write((uint8_t *)&test_proc_dat, sizeof(TEST_PROC));
		tmp[0] = 0x6D;
		tmp[1] = 0x7D;
		usart_write(tmp,2);
		delay(20);
	}
}

#define PRI 1
//通过上位机转发给测试版发送消息
void tx_com_mac_dat(TBDIO_UART *proc_dat)
{
	uint8_t tmp[3];
	delay_ms(300);
#if BDTEST_LOGPRINT
	log_printf("tx_com_mac_dat:");
#endif	
	tmp[0] = 0x7E;
	tmp[1] = 0x5D;
	tmp[2] = TPC_TRANS_PACK;
	usart_write(tmp,3);
#if BDTEST_LOGPRINT		
	for(uint8_t i=0;i<3;i++)
		log_printf("%02X ",tmp[i]);
#endif			
	
	usart_write((uint8_t *)proc_dat, 3);
#if BDTEST_LOGPRINT		
	tmp[0] = (uint8_t)(proc_dat->output_sta_1&0x00ff);
	tmp[1] = (uint8_t)((proc_dat->output_sta_1&0xff00)>>8);
	tmp[2] = (uint8_t)(proc_dat->output_sta_2&0x00ff);
	for(uint8_t i=0;i<3;i++)
		log_printf("%02X ",tmp[i]);
#endif			
	tmp[0] = 0x6D;
	tmp[1] = 0x7D;	
	usart_write(tmp,2);
#if BDTEST_LOGPRINT		
	for(uint8_t i=0;i<2;i++)
		log_printf("%02X ",tmp[i]);
	log_printf("\r\n");	
#endif		
}
void mac_dat_pro(uint8_t type)
{
	uint32_t cc=0,c2=0,c3=0;
	test_mac_proc_dat.output_sta_1 = 0;
	test_mac_proc_dat.output_sta_2 = 0;
	rec_mac_proc_dat.output_sta_1 =	0xffff;
	rec_mac_proc_dat.output_sta_2 = 0xff;
	switch(type)
	{
		case TPC_STOP_ALL:
			test_mac_proc_dat.output_sta_1 = 0;
			test_mac_proc_dat.output_sta_2 = 0;
		break;
		case TPC_IR_FRT:	//前撞红外 电机
		{			
			test_mac_proc_dat.output_sta_1 = BIT_STA(VALVE_F_7)|BIT_STA(VALVE_F_8); 				//跌落电磁阀
			test_mac_proc_dat.output_sta_2 = BIT_STA(MOTER_0)|BIT_STA(MOTER_1)|BIT_STA(MOTER_2);	//外红电机 				
		}			
			break;
		case TPC_IR_BOT:	//前撞红外 电机  add 2020-03-13
			test_mac_proc_dat.output_sta_1 = BIT_STA(VALVE_F_7)|BIT_STA(VALVE_F_8); 				//跌落电磁阀
		break;
		case TPC_BUM_SW_L:	//碰撞12456
			test_mac_proc_dat.output_sta_1 = BIT_STA(VALVE_2);	
		break;
		case TPC_BUM_SW_ML:
			test_mac_proc_dat.output_sta_1 = BIT_STA(VALVE_1);
		break;	
		case TPC_BUM_SW_M:
			test_mac_proc_dat.output_sta_1 = BIT_STA(VALVE_4);
		break;	
		case TPC_BUM_SW_MR:	//碰撞12456			
			test_mac_proc_dat.output_sta_1 = BIT_STA(VALVE_6); 		
		break;
		case TPC_BUM_SW_R:	//碰撞12456			
			test_mac_proc_dat.output_sta_1 = BIT_STA(VALVE_5); 		
		break;
		case TPC_MT_MID:
			test_mac_proc_dat.output_sta_1 = BIT_STA(VALVE_3); 		
		break;
		case TPC_SWSENSOR:	//(虚拟墙/尘盒/集尘盒满/左落轮/右落轮/超声波/后轮码盘)
		{	
			test_mac_proc_dat.output_sta_1 = BIT_STA(VALVE_F_9); 	//虚拟墙/超声波/
			test_mac_proc_dat.output_sta_2 = /*BIT_STA(MOTER_3)|BIT_STA(MOTER_4)|*/BIT_STA(MOTER_5);	///左落轮/右落轮后轮码盘
		}	
		break;
		default:
			return;
		//break;
	}
	tx_com_mac_dat(&test_mac_proc_dat);//上报进度
	cc = 0;
	c2 = 0;
	c3 = 0;
	while(1)	//等待状态同步
	{
		delay_ms(10);
		proc_uart_task();	
		if(rec_mac_proc_dat.output_sta_1 == test_mac_proc_dat.output_sta_1 && rec_mac_proc_dat.output_sta_2 == test_mac_proc_dat.output_sta_2)
		{
#if BDTEST_LOGPRINT			
			log_printf("rec_mac_proc_dat ok\r\n");
#endif			
			break;
		}		
		if(cc++>300)//超过8s
		{
#if BDTEST_LOGPRINT			
			log_printf("rec_mac_proc_dat err\r\n");
#endif				
			break;
		}
		if(c2++>20&&c3<4)
		{	
			c2 = 0;
			c3++;
			tx_com_mac_dat(&test_mac_proc_dat);//上报进度
		}	
	}
}

//向测试板发送消息
void tx_dp_dat(uint8_t code, uint8_t para, uint8_t para1)
{
#if (1)
	delay_ms_tm5(200);
	uint8_t tmp[2];
	TEST_BD_PROC test_bd_dat;
	//print_tbdio_i2c(&tbdio_i2c);
	test_bd_dat.code = code;
	test_bd_dat.para = para;
	test_bd_dat.para1 = para1;
	test_bd_dat.t_res_input = 0;
	test_bd_dat.t_res_output = 0;
	tmp[0] = 0x7E;
	tmp[1] = 0x5D;
	usart4_write(tmp,2);
	usart4_write((uint8_t *)&test_bd_dat, sizeof(TEST_BD_PROC));
	tmp[0] = 0x6D;
	tmp[1] = 0x7D;
	usart4_write(tmp,2);

#else
	tbdio_i2c.package_flag = 0xa5;
	if(code == TBD_STOP_ALL)
		tbdio_i2c.tbstate = TBI2C_TBDSTA_IDLE;
	else
		tbdio_i2c.tbstate = TBI2C_TBDSTA_BUSY;
	switch(code)
	{
		case TBD_STOP_ALL:
			tbdio_i2c.output_sta_1 = tbdio_i2c.output_sta_2 = 0;
			tbdio_i2c.pwm_motor = 0;
			break;
		case TPC_START_ALL:
			tbdio_i2c.output_sta_1 &= ~TBI2C_O1_BAT;
			break;
		case TPC_CD_FRT://前轮码盘暂时不测
			break;
		case TBD_MTOC_WHL_L:
			if(para == 1)
			{
				tbdio_i2c.output_sta_1 |= TBI2C_O1_MTOC_WHL_L;
			}
			else
			{
				tbdio_i2c.output_sta_1 &= ~TBI2C_O1_MTOC_WHL_L;
			}
			break;
		case TBD_MTOC_WHL_R:
			if(para == 1)
				tbdio_i2c.output_sta_1 |= TBI2C_O1_MTOC_WHL_R;
			else
				tbdio_i2c.output_sta_1 &= ~TBI2C_O1_MTOC_WHL_R;
			break;
		case TBD_MTOC_SMT_L:
			if(para == 1)
				tbdio_i2c.output_sta_1 |= TBI2C_O1_MTOC_SMT_L;
			else
				tbdio_i2c.output_sta_1 &= ~TBI2C_O1_MTOC_SMT_L;
			break;
		case TBD_MTOC_SMT_R:
			if(para == 1)
				tbdio_i2c.output_sta_1 |= TBI2C_O1_MTOC_SMT_R;
			else
				tbdio_i2c.output_sta_1 &= ~TBI2C_O1_MTOC_SMT_R;
			break;
		case TBD_MTOC_MT_MID:
			if(para == 1)
				tbdio_i2c.output_sta_1 |= TBI2C_O1_MTOC_MT_MID;
			else
				tbdio_i2c.output_sta_1 &= ~TBI2C_O1_MTOC_MT_MID;
			break;
		case TBD_MTFALL_L:
			if(para == 1)
				tbdio_i2c.output_sta_2 |= TBI2C_O2_MTF_L;
			else
				tbdio_i2c.output_sta_2 &= ~TBI2C_O2_MTF_L;
			break;
		case TBD_MTFALL_R:
			if(para == 1)
				tbdio_i2c.output_sta_2 |= TBI2C_O2_MTF_R;
			else
				tbdio_i2c.output_sta_2 &= ~TBI2C_O2_MTF_R;
			break;
		case TBD_BUM_SW:
			if(para1 & 0x01)
				tbdio_i2c.output_sta_1 |= TBI2C_O1_BUML;
			else
				tbdio_i2c.output_sta_1 &= ~TBI2C_O1_BUML;
			if(para1 & 0x02)
				tbdio_i2c.output_sta_1 |= TBI2C_O1_BUMR;
			else
				tbdio_i2c.output_sta_1 &= ~TBI2C_O1_BUMR;
			break;
		case TBD_SWSENSOR:
				tbdio_i2c.output_sta_2 = para1;
			break;
		case TBD_CHRG_TST:
			tbdio_i2c.output_sta_1 &= ~TBI2C_O1_BAT;
			tbdio_i2c.output_sta_1 |= (uint16_t)(para << 14);
			#if 0
			if(para == 3)//电池断开,以15V开关电源对主板进行供电
			{
				tbdio_i2c.output_sta_1 |= TBI2C_O1_BAT;
			}
			else if(para == 1)//电池接入,且充电电源(24V或其他)接入DC插头
			{
				tbdio_i2c.output_sta_1 |= 0x01 << 14;
			}
			else if(para == 1)//电池接入,且充电电源(24V或其他)接入充电座接口
			{
				tbdio_i2c.output_sta_1 |= 0x02 << 14;
			}
			#endif
			break;
		default:
			return;
	}
	I2C1_writebytes(0x20, 0x00, (uint8_t *)&tbdio_i2c, SIZEOFTBDIO_I2C);
	print_tbdio_i2c(&tbdio_i2c);
	uint8_t *ptr;
	ptr = (uint8_t *)&tbdio_i2c;
	for(uint8_t cc = 0;cc < (SIZEOFTBDIO_I2C);cc ++)
	{
		log_printf("%02X ",ptr[cc]);
	}
	log_printf("\r\n");
#endif
}


//向测试板发送消息,这个数据是由PC中转到测试工装板,这个函数用在整机测试上
void tx_dp_pc_dat(uint8_t code, uint8_t para, uint8_t para1)
{
	uint8_t tmp[2];
	TEST_BD_PROC test_bd_dat;

	test_bd_dat.code = code;
	test_bd_dat.para = para;
	test_bd_dat.para1 = para1;
	test_bd_dat.t_res_input = 0;
	test_bd_dat.t_res_output = 0;

	//data_print((uint8_t *)&test_bd_dat, sizeof(TEST_BD_PROC));

	tmp[0] = 0x7E;
	tmp[1] = 0x5D;
	usart_write(tmp,2);
	usart_write((uint8_t *)&test_bd_dat, sizeof(TEST_BD_PROC));
	tmp[0] = 0x6D;
	tmp[1] = 0x7D;
	usart_write(tmp,2);
}

uint8_t get_chksum(uint8_t *pdata, uint16_t length)
{
	uint16_t i;
	uint8_t ret = 0;

	for(i = 0;i < length;i ++)
	{
		ret += pdata[i];
	}
	return ret;
}

//向PC发送SN,使用的指令码为TPC_GETSN 或 TPC_SETSN
//如果SN为NULL，表示发送的SN无效
void tx_sn(uint8_t code ,uint8_t *sn)
{
	uint8_t tmp[2];
	TEST_SN_PROC sn_proc;

	sn_proc.code = code;
	if(sn)
	{
		memcpy(sn_proc.sn,sn,SNLENGTH);
	}
	else
	{
		sn_proc.sn[0] = 0xff;
	}

	sn_proc.chksum = get_chksum((uint8_t *)&sn_proc, sizeof(TEST_SN_PROC) - 1);
	//data_print((uint8_t *)&sn_proc, sizeof(TEST_SN_PROC));

	tmp[0] = 0x7E;
	tmp[1] = 0x5D;
	usart_write(tmp,2);
	usart_write((uint8_t *)&sn_proc, sizeof(TEST_SN_PROC));
	tmp[0] = 0x6D;
	tmp[1] = 0x7D;
	usart_write(tmp,2);
}
#define DIS_S 50
void walk_tesk_for_whele(void)
{
	uint8_t err=0;
	sys->shut_down_motor = 1;		//关电机
	navigat_init(0);				//初始化
    navigat->angle = 0;
	sys->sState = SYS_NAVIGAT;		//导航状态，否则直行函数会退出
	navigat->distance = motor.c_left_hw = motor.c_right_hw = 0;	//清零
	motor_go_forwark(0,NO_SIDE_NEAR,NULL);	//直行


	float dis = motor.c_left_hw * 0.87f;			//计算出前轮值

	if( disXY(navigat->distance,(int)dis) >= DIS_S)		//前轮和左轮对比
	{
		err = 1;
		log_printf( "Errdis0=%d,%d\r\n",navigat->distance,(int)dis);
	}
	if( disXY(motor.c_left_hw,motor.c_right_hw) >= DIS_S)	//左轮和右轮对比
	{
		err = 1;
		log_printf( "Errdis1=%d,%d\r\n",motor.c_left_hw,motor.c_right_hw);
	}

	turn_to_deg(180);
	//delay_ms(800);//掉头180°
	 if(err == 1)
	{
		for(int i=0;i<20;i++)
		{
			ny3p_play(VOICE_DIDI);
			delay_ms(50);
		}

	}
	else
	{
     delay_ms(800);
	}
	navigat->distance = motor.c_left_hw = motor.c_right_hw = 0;

	motor_go_forwark(0,NO_SIDE_NEAR,NULL);	//直行
	sys->sState = SYS_IDLE;
	 dis = motor.c_left_hw * 0.87f;
	if( disXY(navigat->distance,(int)dis) >= DIS_S)
	{
		err = 1;
		log_printf( "Errdis2=%d,%d\r\n",navigat->distance,(int)dis);
		}
	if( disXY(motor.c_left_hw,motor.c_right_hw) >= DIS_S)
	{
		err = 1;
     log_printf( "Errdis3=%d,%d\r\n",motor.c_left_hw,motor.c_right_hw);
     }
	//错误，则叫
	if(err == 1)
	{
		for(int i=0;i<40;i++)
		{
			ny3p_play(VOICE_DIDI);
			delay_ms(50);
		}

	}

	STOP_ALL_MOTOR();
}

