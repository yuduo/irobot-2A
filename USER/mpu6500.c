

#include "mpu6500.h"
#include "sys.h"
//#include "delay.h"

uint8_t	mpu6500_buf[14];					//spi读取MPU6500后存放数据
uint8_t offset_flag = 0;						//校准模式标志位，为0未进行校准，为1进行校准

S_INT16_XYZ	MPU6500_Acc_Offset	=	{0,0,0};		
S_INT16_XYZ	MPU6500_Gyro_Offset	=	{0,0,0};	
S_INT16_XYZ MPU6500_Acc = {0,0,0};
S_INT16_XYZ MPU6500_Gyro = {0,0,0};

float	mpu6500_tempreature = 0;
int16_t mpu6500_tempreature_temp = 0;
int16_t mpu6500_tempreature_Offset = 0;


#if 1//GYRO_ON_BORD && ICLEAN_ROUND_CC_V12
static volatile uint32_t mpu_ext_tick_us = 0;//微秒级的计数器

#define MPU6500_SAVE2EXTTICK()	mpu_ext_tick_us += GYRO_TIM->CNT

#if 0
 static void mpu6500_save2exttick(void)
{
	mpu_ext_tick_us += GYRO_TIM->CNT;
}
#endif

uint32_t mpu6500_getexttick(void)
{
	return (mpu_ext_tick_us + GYRO_TIM->CNT);
}

void mpu6500_add2exttickperiod(void)
{
	mpu_ext_tick_us += 65535;
}
#endif

/*
 * 函数名：MPU6500_Write_Reg
 * 描述  ：SPI写入寄存器
 * 输入  ：reg:指定的寄存器地址；value：写入的值
 * 输出  ：status：返回状态值
 */ 
u8 MPU6500_Write_Reg(uint8_t reg,uint8_t value)
{
	uint8_t status;
	SPI_CS(0);  										//使能SPI传输
	status = SPI1_ReadWriteByte(reg); //发送写命令+寄存器号
	SPI1_ReadWriteByte(value);				//写入寄存器值
	SPI_CS(1);  										//禁止MPU9500
	return(status);											//返回状态值
}


/*
 * 函数名：MPU6500_Read_Reg
 * 描述  ：SPI读取寄存器
 * 输入  ：reg:指定的寄存器地址
 * 输出  ：reg_val：reg寄存器地址对应的值
 */ 
u8 MPU6500_Read_Reg(uint8_t reg)
{
	uint8_t reg_val;
	SPI_CS(0);  										//使能SPI传输
	SPI1_ReadWriteByte(reg | 0x80); 	//发送读命令+寄存器号
	reg_val = SPI1_ReadWriteByte(0xff); //读取寄存器值
	SPI_CS(1);  									  //禁止MPU9250
//	log_printf("read:%d\r\n",reg_val);
	return(reg_val);
}

/*
 * 函数名：MPU6500_Date_Offset
 * 描述  ：MPU6500数据校准
 * 输入  ：校准次数
 * 输出  ：无
 */ 
#if 1//GYRO_ON_BORD
#if 0//ICLEAN_HW_VERSION == 0x1203
#define MPU6500_SHAKEVALUE_MAX	200
#define MPU6500_SHAKEVALUE_MIN	-100
#else
#define MPU6500_SHAKEVALUE_MAX	100
#define MPU6500_SHAKEVALUE_MIN	-100
#endif
void mpu6500_date_offset(uint16_t cnt)
{
	int i;
	int v=0;
	int16_t vv;

	for(i = cnt; i > 0; i--)
	{
		vv = mpy6500_read_gyro();
		
		v += vv;
	//	log_printf("%d,%d\r\n",vv,v);
		delay_ms_tm5(2);

	}
	v /=cnt;
//	MPU6500_Acc_Offset.X 	=	Temp_Acc.X	/	cnt;
	//MPU6500_Acc_Offset.Y 	=	Temp_Acc.Y	/	cnt;
	//MPU6500_Acc_Offset.Z  =	Temp_Acc.Z	/	cnt;	
	//MPU6500_Gyro_Offset.X	= Temp_Gyro.X	/	cnt;
	//MPU6500_Gyro_Offset.Y	= Temp_Gyro.Y	/	cnt;
	
	if(cnt <300)
	{
		int16_t ofs=	sys->gyro_offset;
		if(disxy(ofs,sys->gyro_offset) < 20)
		{
			sys->gyro_offset =(ofs + sys->gyro_offset)/2;
		}else
			log_printf("ofs err=%d,%d\r\n",ofs,sys->gyro_offset);
		//MPU6500_Gyro_Offset.Z =	(Temp_Gyro.Z	/	cnt) + MPU6500_Gyro_Offset.Z ) /2;
		
	}
	else
		sys->gyro_offset = v;
	log_printf("[mpu6500_date_offset]gyro_z:%d,%d\r\n",sys->gyro_offset,v);
	
	sys->gyro_ofs = 0;
}
#endif
void MPU6500_get_Offset(uint16_t cnt)
{
	static S_INT32_XYZ Temp_Gyro ;//, Temp_Acc;
	int i = 0;
	
	Temp_Gyro.X =	0;
	Temp_Gyro.Y =	0;
	Temp_Gyro.Z =	0;
	
//	Temp_Acc.X = 0;
//	Temp_Acc.Y = 0;
//	Temp_Acc.Z = 0;
	
	offset_flag = 1;//进入MPU6500校准模式
	for(i = cnt; i > 0; i--)
	{
#if AHRS_EN				
    MPU6500_ReadValue();
		
		Temp_Acc.X	+=	MPU6500_Acc.X;
		Temp_Acc.Y	+=	MPU6500_Acc.Y;		
		//Temp_Acc.Z	+=	MPU6500_Acc.Z;
		Temp_Gyro.X	+=	MPU6500_Gyro.X;
		Temp_Gyro.Y	+=	MPU6500_Gyro.Y;
		Temp_Gyro.Z	+=	MPU6500_Gyro.Z;
#else
		Temp_Gyro.Z +=	mpy6500_read_gyro();
#endif
	}
#if AHRS_EN			
	MPU6500_Acc_Offset.X 	=	Temp_Acc.X	/	cnt;
	MPU6500_Acc_Offset.Y 	=	Temp_Acc.Y	/	cnt;
	//MPU6500_Acc_Offset.Z  =	Temp_Acc.Z	/	cnt;	
	MPU6500_Gyro_Offset.X	= Temp_Gyro.X	/	cnt;
	MPU6500_Gyro_Offset.Y	= Temp_Gyro.Y	/	cnt;
	MPU6500_Gyro_Offset.Z =	Temp_Gyro.Z	/	cnt;
#endif	
	MPU6500_Gyro_Offset.Z =	Temp_Gyro.Z	/	cnt;
	offset_flag = 0;//退出MPU6500校准模式
}


/*
 * 函数名：MPU6500_ReadValue
 * 描述  ：读取MPU6500原始数据
 * 输入  ：无
 * 输出  ：无
 */ 
void MPU6500_ReadValue(void)
{
	uint8_t i;
	
	SPI_CS(0); 																	//使能SPI传输

	SPI1_ReadWriteByte(ACCEL_XOUT_H|0x80); 				//从加速度计的寄存器开始进行读取陀螺仪和加速度计的值//发送读命令+寄存器号
	
	for(i	=	0;i	<	14;i++)														//一共读取14字节的数据
	{
		mpu6500_buf[i]	=	SPI1_ReadWriteByte(0xff);	//输入0xff,因为slave不识别
	}	
	if(offset_flag == 0)
	{
		MPU6500_Acc.X = BYTE16(int16_t, mpu6500_buf[0],  mpu6500_buf[1]) - MPU6500_Acc_Offset.X;
		MPU6500_Acc.Y = BYTE16(int16_t, mpu6500_buf[2],  mpu6500_buf[3]) - MPU6500_Acc_Offset.Y;
		MPU6500_Acc.Z = BYTE16(int16_t, mpu6500_buf[4],  mpu6500_buf[5]);
		MPU6500_Gyro.X = BYTE16(int16_t, mpu6500_buf[8],  mpu6500_buf[9]) - MPU6500_Gyro_Offset.X;
		MPU6500_Gyro.Y = BYTE16(int16_t, mpu6500_buf[10],  mpu6500_buf[11]) - MPU6500_Gyro_Offset.Y;
		MPU6500_Gyro.Z = BYTE16(int16_t, mpu6500_buf[12],  mpu6500_buf[13]) - MPU6500_Gyro_Offset.Z;
		
		mpu6500_tempreature_temp	=	BYTE16(int16_t, mpu6500_buf[6],  mpu6500_buf[7]);
		mpu6500_tempreature	=	(float)(35000+((521+mpu6500_tempreature_temp)*100)/34); // 原来分母为340，现在分子*100，即：扩大1000倍；
		mpu6500_tempreature = mpu6500_tempreature/1000;    
	//	log_printf("%d,%d,%d\r\n",MPU6500_Gyro.X,MPU6500_Gyro.Y,MPU6500_Gyro.Z);
		if(( -15	<	MPU6500_Gyro.X ) && (MPU6500_Gyro.X < 15) ) MPU6500_Gyro.X = 0;
		if(( -15	<	MPU6500_Gyro.Y ) && (MPU6500_Gyro.Y < 15) ) MPU6500_Gyro.Y = 0;
		if(( -15	<	MPU6500_Gyro.Z ) && (MPU6500_Gyro.Z < 15) ) MPU6500_Gyro.Z = 0;
		sys->gyro = MPU6500_Gyro.Z;
		if(MPU6500_Gyro.Z < 100)
		{
				if(sys->c_yaw_ok1 < 50)
					sys->c_yaw_ok1++;
		}else
			sys->c_yaw_ok1 = 0;
	}
	else if(offset_flag)  //MPU6500处于校准模式
	{
		MPU6500_Acc.X = BYTE16(int16_t, mpu6500_buf[0],  mpu6500_buf[1]);
		MPU6500_Acc.Y = BYTE16(int16_t, mpu6500_buf[2],  mpu6500_buf[3]);
		MPU6500_Acc.Z = BYTE16(int16_t, mpu6500_buf[4],  mpu6500_buf[5]);
		MPU6500_Gyro.X = BYTE16(int16_t, mpu6500_buf[8],  mpu6500_buf[9]);
		MPU6500_Gyro.Y = BYTE16(int16_t, mpu6500_buf[10],  mpu6500_buf[11]);
		MPU6500_Gyro.Z = BYTE16(int16_t, mpu6500_buf[12],  mpu6500_buf[13]);
	}
	
	SPI_CS(1);  	    //禁止SPI传输
}



int16_t mpy6500_read_accel(void)
{
	uint8_t i;
	int16_t ret;
	
	SPI_CS(0); 																	//使能SPI传输

	SPI1_ReadWriteByte(ACCEL_XOUT_H|0x80); 				//从加速度计的寄存器开始进行读取陀螺仪和加速度计的值//发送读命令+寄存器号

	for(i	=	0;i	<	2;i++)														//一共读取14字节的数据
	{
		mpu6500_buf[i]	=	SPI1_ReadWriteByte(0xff);	//输入0xff,因为slave不识别
	}	

	ret = BYTE16(int16_t, mpu6500_buf[0],  mpu6500_buf[1]);//- MPU6500_Gyro_Offset.Z;
		
	SPI_CS(1);  	    //禁止SPI传输
	return ret;
}

int16_t mpy6500_read_gyroy(void)
{
	uint8_t i;
	int16_t ret;
	
	SPI_CS(0); 																	//使能SPI传输

	SPI1_ReadWriteByte(GYRO_YOUT_H|0x80); 				//从加速度计的寄存器开始进行读取陀螺仪和加速度计的值//发送读命令+寄存器号

	for(i	=	0;i	<	2;i++)														//一共读取14字节的数据
	{
		mpu6500_buf[i]	=	SPI1_ReadWriteByte(0xff);	//输入0xff,因为slave不识别
	}	

	ret = BYTE16(int16_t, mpu6500_buf[0],  mpu6500_buf[1]);//- MPU6500_Gyro_Offset.Z;
		
	SPI_CS(1);  	    //禁止SPI传输
	return ret;
}

int16_t mpy6500_read_gyro(void)
{
	uint8_t i;
	int16_t ret;
	uint8_t n_mpu6050_buf[2];
	
	SPI_CS(0); 																	//使能SPI传输

	SPI1_ReadWriteByte(GYRO_ZOUT_H|0x80); 				//从加速度计的寄存器开始进行读取陀螺仪和加速度计的值//发送读命令+寄存器号

	for(i	=	0;i	<	2;i++)														//一共读取14字节的数据
	{
		n_mpu6050_buf[i]	=	SPI1_ReadWriteByte(0xff);	//输入0xff,因为slave不识别
	}	

	if(n_mpu6050_buf[0] == 0 && n_mpu6050_buf[1] == 0 ||
		n_mpu6050_buf[0] == 0xff && n_mpu6050_buf[1] == 0xff)
	{	
		ret = BYTE16(int16_t, mpu6500_buf[0],  mpu6500_buf[1]);//- MPU6500_Gyro_Offset.Z;
	}
	else
	{
		ret = BYTE16(int16_t, n_mpu6050_buf[0],  n_mpu6050_buf[1]);//- MPU6500_Gyro_Offset.Z;
		mpu6500_buf[0] = n_mpu6050_buf[0];
		mpu6500_buf[1] = n_mpu6050_buf[1];
	}
		
	SPI_CS(1);  	    //禁止SPI传输
	if(ret > 2000 || ret < -2000)
		ret = 0;
	//if(ret > 1000 || ret < -1000)
	//	log_printf("g=%d\r\n",ret);
	return ret;
}



void MPU6500_tmr_init(void)
{
	TIM_DeInit(TIM8);
	TIM_InternalClockConfig(TIM8);
	TIM8->PSC = 71;	// Set prescaler (PSC + 1)
	TIM8->ARR = 65535;//TIM2_AUTORELOADVALUE;           // Auto reload value 2000
	TIM8->DIER = TIM_DIER_UIE; // Enable update interrupt (timer level)
	TIM8->CNT=0;
	TIM8->CR1 = TIM_CR1_CEN;   // Enable timer

/*
	TIM_TimeBaseInitTypeDef   TIM_TimeBaseStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB2Periph_TIM8, ENABLE);
	TIM_TimeBaseStructure.TIM_Period = 65535;//100ms 80 MS 0xFFFF; //éè??×??ˉ×°????′??÷//×?′ó2aá?65000us =65ms
	TIM_TimeBaseStructure.TIM_Prescaler = 72 - 1;

	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; //?????òé???êy
	TIM_TimeBaseStructure.TIM_RepetitionCounter =0;
	TIM_TimeBaseInit(TIM8, &TIM_TimeBaseStructure);

	TIM_Cmd(TIM8, ENABLE);
	*/
}
/*
 * 函数名：MPU6500_Init
 * 描述  ：MPU6500初始化函数
 * 输入  ：无
 * 输出  ：0：初始化失败 1：初始化成功
 */ 
uint8_t MPU6500_Init(void)
{
	//uint8_t cc,i;
	//SPI1_Init();																//MPU6500 IO口和SPI初始化
	MPU6500_tmr_init();

	if(MPU6500_Read_Reg(WHO_AM_I) == 0x70)			//正确读取到6500的地址
	{		
		sys->gyro_init_ok = 1;	
		MPU6500_Write_Reg(PWR_MGMT_1,0X80);   		//电源管理,复位MPU6500
		delay_ms(100);
		MPU6500_Write_Reg(SIGNAL_PATH_RESET,0X07);//陀螺仪、加速度计、温度计复位
		delay_ms(100);
		MPU6500_Write_Reg(PWR_MGMT_1,0X01);   		//选择时钟源
		MPU6500_Write_Reg(PWR_MGMT_2,0X00);   		//使能加速度计和陀螺仪
		MPU6500_Write_Reg(CONFIG,0X02);						//低通滤波器 0x02 92hz (3.9ms delay) fs=1khz
		MPU6500_Write_Reg(SMPLRT_DIV,0X00);				//采样率1000/(1+0)=1000HZ
#if 1//AHRS_EN		
		MPU6500_Write_Reg(GYRO_CONFIG,0X18);  		//陀螺仪测量范围 0X18 正负2000度
#else
		MPU6500_Write_Reg(GYRO_CONFIG,0X00); //0x08->0x00
#endif		
		MPU6500_Write_Reg(ACCEL_CONFIG,0x10); 		//加速度计测量范围 0X00 正负8g
		MPU6500_Write_Reg(ACCEL_CONFIG2,0x00);		//加速度计速率1khz 滤波器460hz (1.94ms delay)
		return 1;
	}
	else
	{
		//log_printf("mpu6500 init error!\r\n");
		//log_printf("mpu6500 init error!\r\n");
	}
	return 0;
}

#define AJUST_FLOAT		1		//浮点校准




//*1.0096f
#if ICLEAN_MAC_TYPE == 1 || ICLEAN_MAC_TYPE == 3
uint8_t m_mpu_accelknock_ct = 0;//计算加速度碰撞的计数器,6ms读一次,calc_gyro_on_board()的调用周期为2ms,故此值门限为3
#endif
//static int16_t gyro_short_old = 0;
void calc_gyro_on_board(void)
{
		float dt;//,dt1;

		if(sys->gyro_ofs )						//矫正陀螺仪时，不可积分
			return ;
		int16_t gyro_short;
		gyro_short = mpy6500_read_gyro();		//获取陀螺仪角速度

		//计算两次的时间差，GYRO_TIM->CNT 1微妙加1，+80是修正一下时间，理论依据，根据实际情况去掉
		dt= (float)(GYRO_TIM->CNT) /  1000000.0;	//计算时间，换算成秒
		
		MPU6500_SAVE2EXTTICK();					//获取时钟，这个数值跟陀螺仪无关
		GYRO_TIM->CNT = 0;						//定时器数据清零
	
		
		gyro_short -= sys->gyro_offset;			



	 	if(gyro_short <=80 && gyro_short>=-80)
		{
				if(sys->c_yaw_ok1 < 100)
						sys->c_yaw_ok1++;
		}else
			sys->c_yaw_ok1=0;

			

		idle_gyro(&sys->yaw,&sys->gyro, dt,gyro_short);
		sys->angle = sys->yaw;			//最终的角度

}


#define MPU6500_CALMDOWM_VALUE	150
#define MPU6500_CALMDOWN_CTER	50

uint8_t mpu6500_waitformpucalm(uint16_t timeout)
{
	uint8_t n_calmdown = 0;
	int16_t gyro_short;

	while(timeout > 0)
	{
		gyro_short = mpy6500_read_gyro();
		gyro_short -= MPU6500_Gyro_Offset.Z;
		if(gyro_short < MPU6500_CALMDOWM_VALUE && gyro_short >-MPU6500_CALMDOWM_VALUE )//40->80
		{
			if(n_calmdown ++ > MPU6500_CALMDOWN_CTER)
				return 1;
			gyro_short = 0;
		}
		timeout --;
		//feed_dog();
		delay_ms(10);
	}

	return 0;
}

#if 0
rx_agl_t *g_rx;
uint8_t grxBuf[32];
void calc_gyro(void)
{
	int i;
	uint8_t crc=0;
	int c = 0;
	uint32_t tmr;
l_c_gyro:

	if(c++ >=3)
		return ;
	g_len = 0;
	
	memset(g_buf,0x00,32);

	//usart4_sndbyte(0xAA);
	tmr = msTmr;
	
	while(1)
	{
		if((msTmr - tmr) >= 3000)
		{
			log_printf("-gyro er,len=%d,%02X,%02X\r\n",g_len,grxBuf[0],grxBuf[1]);
			goto l_c_gyro;
		}	
		if(g_len >=9 && g_len <32)
		{

			
			memcpy((char *)&grxBuf,(char *)g_buf,g_len);
			g_rx = (rx_agl_t *)grxBuf;

			for(i=0;i<8;i++)
			
			{
				crc += grxBuf[i];
			}
			//log_printf("crc:%02X-%02X \r\n",crc,g_rx->crc);
			if(crc == g_rx->crc)
			{
			//	
#if GYRO_RESET			
				sys->angle = format_agle(sys->yaw + g_rx->agle,ANGLE_360);
#else
				sys->angle = format_agle(g_rx->agle, ANGLE_360);
#endif
				sys->gyro  = g_rx->gyro;


				/*
				for(i=0;i<7;i++)
					sys->g_sta[i] = g_rx->g_sta[i];
				*/
	
			}
			return ;
			
		}
		/*
		if(rx_agl.begin==1)
		{
			sys->angle = rx_agl.agle;
			sys->gyro_offset = rx_agl.agle2;
			TIM5->CNT = 0;
			return ;
		}
		*/
	}

}
#endif

