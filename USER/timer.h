#ifndef _TIMER_H
#define _TIMER_H

#include <stdint.h>

#define MAX_PWM 2000

#define DIS_CH1()			(TIM1->CCER &=0xFFFC)		//PA8 PWM½ûÖ¹)
#define DIS_CH2()			(TIM1->CCER &=0xFFCF)
#define DIS_CH3()			(TIM1->CCER &=0xFCFF)
#define DIS_CH4()			(TIM1->CCER &=0xCFFF)

#define EN_CH1()			(TIM1->CCER|=3<<0)//OC1 Êä³öÊ¹ÄÜ)
#define EN_CH2()			(TIM1->CCER|=3<<4)
#define EN_CH3()			(TIM1->CCER|=3<<8)
#define EN_CH4()			(TIM1->CCER|=3<<12)




#define CYC_HW		240		

//extern char motorStr[6][20];

//extern int ccr3_1;
//extern int c_hw;//1,c_hw2;

//extern uint32_t m_cHw1,m_cHw2;
void timer1_init(void);
void motor_timer_init(void);
void timer3_pwm_init(void);
void l_motor_set_pwm(uint8_t HL,int pwm);
void r_motor_set_pwm(uint8_t HL,int pwm);
void timr3_cfg(void);
void r_motor_stop(void);
void l_motor_stop(void);
void motor_run(uint8_t dir ,int pwm,int cHw,uint8_t next);
char * get_motor_sta(uint8_t indx);
void printf_motor_sta(void);
void init_tim4_camp(uint16_t psc, uint16_t arr, uint8_t way, uint8_t dir);

void rl_motor_stop_v(uint8_t v);

void init_tim5_camp(uint16_t psc, uint16_t arr, uint8_t way, uint8_t dir);
void tx_timer_cfg(void);
void motor_turn(uint8_t dir,int pwm,float agle);
char proc_line_pid(float set_angle);
//char proc_near_pid(float ird,float n_ird);

void motor_turn_360(uint8_t dir);
void motor_turn_circle(uint8_t dir,int hw);
void motor_wheel_forward(uint8_t wheel,uint16_t speed);
void motor_wheel_backward(uint8_t wheel,uint16_t speed);
void motor_wheel_stop(uint8_t wheel);
void timer2_init(void);
void timer1_pwm_init(void);
#endif






