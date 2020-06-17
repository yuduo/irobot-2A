/*
 * 	gyro.h
 
 *	Created on: Jun 26, 2013
 *		Author: Denis aka caat
 *
 * MPU6050 gyro via i2c bus.
 */

#ifndef GYRO_H_
#define GYRO_H_

#define MPU6050_I2C I2C2 //MPU6050 Bus
typedef struct _gyro_t
{
		float *angle;
		int		*rpwm;
		int		*lpwm;
		int		*pwm;
		int16_t  *gyro_ofs1;
		int16_t  *gyro_ofs2;
		uint16_t *kp2;
		uint16_t *ki2;
		uint16_t *kd2;
	float *x_org_f,*y_org_f;		//浮点d坐标系
	float *x_org_r,*y_org_r;		//右轮计算的
	float *x_org_t,*y_org_t;		//前轮计算的坐标
	int	*x_org,*y_org;		//本点的原始坐标
	float *radius; 
	int	*hw_grid;
	pid_t *pid;
}gyro_t;


typedef u8 (*m_spi_read_write)(uint8_t data);
typedef void (*m_spi_cs)(uint8_t cs);
		
typedef void (*m_func_printf)(const char *format,...);
typedef	void (*m_func_delay)(uint32_t msec);




//左轮
#define X_ORG_CALC(X) 		(int32_t)(X+ *(mgyro->radius) * gyro->sinA) // WHELE_HW;
#define Y_ORG_CALC(Y)		(int32_t)(Y- *(mgyro->radius) * gyro->cosA) // WHELE_HW;
//右轮
#define X_ORG_CALC_R(X) 	(int32_t)(X - *(mgyro->radius) * gyro->sinA) // WHELE_HW;
#define Y_ORG_CALC_R(Y)		(int32_t)(Y + *(mgyro->radius) * gyro->cosA) // WHELE_HW;
//前轮
#define X_ORG_CALC_F(X)		(int32_t)(X*FRONT_TO_ORG- *(mgyro->radius) * gyro->cosA)
#define Y_ORG_CALC_F(Y)		(int32_t)(Y*FRONT_TO_ORG - *(mgyro->radius) * gyro->sinA)

//左轮



#define GYRO_RESET				0
#define GYRO_SET_OFS_MSG		0x05		//éè????ò?á?
#define GYRO_RESET_MSG			0x08
#define GYRO_REST_VALUE			0x88


#define RESET_GYRO()			illegal_copy_tell_13757122544_gsend(0x08,0x88,0x88);



#define SYS_AGL_360()		format_agle(sys->angle, ANGLE_360)
#define SYS_AGL_180()		format_agle(sys->angle, ANGLE_180)



#define ANGLE_360		0x00		
#define ANGLE_180		0x01	


typedef struct h_rx_agl_t
{

		float		agle;
		float 	gyro;
		

		uint8_t	crc;	
}rx_agl_t;

#define MAX_GRID		200
#define MAX_GRID_LIN	5000  //miconydrawmap		5000->500

typedef struct h_gyto_t
{
	uint8_t isOK;
	float	sinA;			//角度计算
	float	cosA;			//角度计算三角函数
	uint8_t obstacle[MAX_GRID_LIN];		// 5K
	uint8_t scan[MAX_GRID_LIN];			// 5K
}GRYO_T;







void illegal_copy_tell_13757122544_gsend(uint8_t msg,uint16_t p1,uint16_t p2);
float format_agle(float angle2,uint8_t type);


uint8_t test_gyro_org(void);
void adj_gyro_onboard(void);

void calc_xy_org(int dis,float *x_org,float *y_org);
void gyro_whlmap(void);
void gyro_mapwhl(void);
//int coordinate_calcu(void);
void www_idleintel_com(void);
short get_gyro_ofs(void);
uint8_t get_gyro_org(void);
//uint8_t reset_gyro(void);

int32_t x_org_calc(float x);
int32_t y_org_calc(float y)	;
int32_t x_org_calc_r(float x);
int32_t y_org_calc_r(float y);
extern int32_t x_org_calc_f(float x);
extern int32_t y_org_calc_f(float y);

float X_ORGF_CALC(void) ;		
float  Y_ORGF_CALC(void) ;
uint8_t read_compress_mem(short x,short y,uint8_t byte);
void set_compress_mem(short x,short y,uint8_t value,uint8_t byte);

void inti_gyro_org(void);
int p_gyro_calce(float set_speed,float speed);
extern void gyro_fwhlmap(void);


extern void copyright_idleintel_illegal_copy_tell_13757122544(void);
extern void copyright_idleintel_no_one_is_allowed_to_copy(const char *cp,const char *webside);
extern void copyright_www_idleintel_com(const char *str,gyro_t *msys,m_spi_read_write func_spi_wr,m_spi_cs func_cs,m_func_printf func_printf,m_func_delay	func_delay);
extern void init_gyro_l(void);

extern void idle_gyro(float *yaw,float *y1,float dt,int16_t vgyro);



#endif /* GYRO_H_ */
