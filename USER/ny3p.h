#ifndef __NY3P_H__
#define __NY3P_H__


#define VOICE_BEE_SHORT		 99	//	短音

//------------------------------------------modified by wonton2004 2016.12.28
#define NY3P_VOICE_TYPE 1
// 语音芯片型号
// 0是原(最早)语音
// 1是新语音
#ifndef NY3P_VOICE_TYPE
#define NY3P_VOICE_TYPE 0
#endif
#if NY3P_VOICE_TYPE == 1
//新语音
#define VOICE_DIDI			2		//	滴
#define VOICE_DRIP			3		//水滴音效
#define VOICE_SELECT_M		4		//苹果line提示音铃声
#define VOICE_M_START		5		//	现在开始自动清扫
#define VOICE_M_STOP		6		//	现在暂停清扫
#define VOICE_M_NAVI		7	//现在开始导航清扫
#define VOICE_M_MOP			8	//现在开始湿拖清扫
#define VOICE_M_EX_MOP		9	//现在切换到湿拖模式
#define VOICE_M_CLN_HEART	10	//	现在开始集中清扫
#define VOICE_M_NEAR		11	//	现在开始沿墙清扫
#define VOICE_M_SPEED_MODE	12	//	强力模式开启
#define VOICE_MEN_CLEAN		13	//	现在开始手动清扫
#define VOICE_MSCH_CLEAN	14	//  现在开始预约清扫
#define VOICE_M_FINISHED	15	//	清扫已完成
#define VOICE_M_Z_CLEAN		16	//	现在开始弓字清扫(Zig-zag)
#define VOICE_M_ORDER		17	//	现在进入预约模式
#define VOICE_M_ORDER_OUT	18	//	退出预约模式
#define VOICE_M_ORDER_SAVE	19	//	预约保存成功
#define VOICE_M_SLEEP		20	//	机器人进入睡眠
#define VOICE_M_DOCK		21	//	现在返回充电座
#define VOICE_CHARING		22	//	现在开始开始充电
#define VOICE_CHARGE_M		23	//	请手动充电
#define VOICE_START_CHARGE 	24	//	请检查电源开关是否打开
#define VOICE_CHECK_BOX		25	//	请清洁集尘盒与吸尘口
#define VOICE_CHECK_M_MTR	26	//	请检查中扫是否缠线
#define VOICE_BOX_OK		27	//	请检查集尘盒是否放到位
#define VOICE_CHECK_LEAVE	28	//	请检查左右轮是否离地
#define VOICE_CHECK_LINE	29		//	请检查左右轮是否缠线
#define VOICE_CHECKLEAVE2	30	//	请检查地面感应器是否离地或尘污
#define VOICE_CHECK_LINE3	31	//	请检查边扫是否缠线
#define VOICE_CHECK_F_LINE	32	//	请检查前轮是否缠线
#define VOICE_CHECK_M_LINE	33	//	请检查中扫边扫是否缠线
#define VOICE_FAC_MODE		34	//	现在进入工程模式

//无效指令
#define VOICE_CHECK_BOX2	0	//	请检查尘盒盖是否盖到位
#define VOICE_M_SEFT_CHECK	0	//	机器人进入自检
#define VOICE_OUT_SEFT_TEST	0	//	机器人退出自检
//30	//	第一大项
//31	//	第二大项
//32	//	第三大项
#elif NY3P_VOICE_TYPE == 0
//旧语音
#define VOICE_DIDI			3		//	滴
#define VOICE_SELECT_M		4		//咚咚咚咚2，你好请选择模式
#define VOICE_M_START		5		//	机器人开始工作
#define VOICE_M_STOP		6		//	机器人暂停工作
#define VOICE_M_DOCK		7	//	机器人自动回充
#define VOICE_M_ORDER		8	//	机器人预约模式
#define VOICE_M_ORDER_OUT	9	//	退出预约模式
#define VOICE_M_ORDER_SAVE	10	//	预约保存成功
#define VOICE_M_SLEEP		11	//	机器人进入睡眠
#define VOICE_M_CLN_HEART	12	//	机器人重点清扫
#define VOICE_M_NEAR		13	//	机器人沿墙清扫
#define VOICE_M_SPEED_MODE	14	//	机器人速度模式
#define VOICE_MEN_CLEAN		15	//	机器人手动清扫
#define VOICE_CHECK_BOX		16	//	请清洁尘盒与吸尘口
#define VOICE_CHECK_M_MTR	17	//	请检查中扫是否缠线
#define VOICE_CHECK_BOX2	18	//	请检查尘盒盖是否盖到位
#define VOICE_BOX_OK		19	//	请检查尘盒是否放到位
#define VOICE_CHARING		20	//	开始充电
#define VOICE_CHECK_LEAVE	21	//	请检查左右轮是否离地
#define VOICE_CHECK_LINE	22		//	请检查左右轮是否缠线
#define VOICE_CHECKLEAVE2	23	//	请检查地检是否离地或尘污
#define VOICE_CHECK_LINE3	24	//	请检查边扫是否缠线
#define VOICE_M_SEFT_CHECK	25	//	机器人进入自检
#define VOICE_OUT_SEFT_TEST	26	//	机器人退出自检
#define VOICE_M_Z_CLEAN		27	//	机器人Z字清扫
#define VOICE_CHARGE_M		28//	请手动充电
#define VOICE_CHECK_M_LINE	29	//	请检查中扫边扫是否缠线
//30	//	第一大项
//31	//	第二大项
//32	//	第三大项
#define VOICE_START_CHARGE 33	//	请开机充电
#else
//旧语音
#define VOICE_DIDI			3		//	滴
#define VOICE_SELECT_M		4		//咚咚咚咚2，你好请选择模式
#define VOICE_M_START		5		//	机器人开始工作
#define VOICE_M_STOP		6		//	机器人暂停工作
#define VOICE_M_DOCK		7	//	机器人自动回充
#define VOICE_M_ORDER		8	//	机器人预约模式
#define VOICE_M_ORDER_OUT	9	//	退出预约模式
#define VOICE_M_ORDER_SAVE	10	//	预约保存成功
#define VOICE_M_SLEEP		11	//	机器人进入睡眠
#define VOICE_M_CLN_HEART	12	//	机器人重点清扫
#define VOICE_M_NEAR		13	//	机器人沿墙清扫
#define VOICE_M_SPEED_MODE	14	//	机器人速度模式
#define VOICE_MEN_CLEAN		15	//	机器人手动清扫
#define VOICE_CHECK_BOX		16	//	请清洁尘盒与吸尘口
#define VOICE_CHECK_M_MTR	17	//	请检查中扫是否缠线
#define VOICE_CHECK_BOX2	18	//	请检查尘盒盖是否盖到位
#define VOICE_BOX_OK		19	//	请检查尘盒是否放到位
#define VOICE_CHARING		20	//	开始充电
#define VOICE_CHECK_LEAVE	21	//	请检查左右轮是否离地
#define VOICE_CHECK_LINE	22		//	请检查左右轮是否缠线
#define VOICE_CHECKLEAVE2	23	//	请检查地检是否离地或尘污
#define VOICE_CHECK_LINE3	24	//	请检查边扫是否缠线
#define VOICE_M_SEFT_CHECK	25	//	机器人进入自检
#define VOICE_OUT_SEFT_TEST	26	//	机器人退出自检
#define VOICE_M_Z_CLEAN		27	//	机器人Z字清扫
#define VOICE_CHARGE_M		28//	请手动充电
#define VOICE_CHECK_M_LINE	29	//	请检查中扫边扫是否缠线
//30	//	第一大项
//31	//	第二大项
//32	//	第三大项
#define VOICE_START_CHARGE 33	//	请开机充电
#endif
//------------------------------------------------------------------------end

//PC13 DATA
//PC12 RESET

#if (1 == ENABLE_VOICE)
#define  NY3P_REST_PORT  GPIOC
#define  NY3P_DATA_PORT  GPIOC
#define  NY3P_REST_PIN	 GPIO_Pin_13
#define  NY3P_DATA_PIN	 GPIO_Pin_12

#define NY3P_REST(b)	GPIO_WriteBit(NY3P_REST_PORT, NY3P_REST_PIN, (BitAction)!!b)
#define NY3P_DATA(b)	GPIO_WriteBit(NY3P_DATA_PORT, NY3P_DATA_PIN, (BitAction)!!b)
#define NY3P_MUTE(b)	//GPIO_WriteBit(NY3P_POWER1_PORT, NY3P_POWER1_PIN, (BitAction)!b)
#define NY3P_POWER(b)	//GPIO_WriteBit(NY3P_POWER2_PORT, NY3P_POWER2_PIN, (BitAction)b)
#else
#define BEEP_ON()		{TIM2->CCR4 = 25;}
#define BEEP_OFF()		{TIM2->CCR4 = 0;}
#define BEEP_RING()		{BEEP_ON();delay_ms(500);BEEP_OFF();}
#define BEEP_SHORT_RING()	{BEEP_ON();delay_ms(50);BEEP_OFF();}

#endif

u16 ny3p_play(u8 num);
void ny3p_init(void);
u16 ny3p_volume_set(u8 num);

#define Voice_Init			ny3p_init
#define voice_output		ny3p_play
#define voice_volume_set	ny3p_volume_set

#endif
