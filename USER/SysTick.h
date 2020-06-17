#ifndef __SYSTICK_H
#define __SYSTICK_H

//#include "stm32f10x.h"

#if ROOMA_GYROONLY
extern volatile uint32_t	msTmr,msledTmr,msWifiTmr,msChargeTmr,msledwifitmr;
#else
extern volatile uint32_t	msTmr,msledTmr,msWifiTmr,,msChargeTmr,msledwifitmr;
#endif

void SysTick_Init(void);
void SysTick_Handler(void);
// void assignment1(void);
// void assignment2(void);
// void assignment3(void);

uint8_t mstimeout(uint32_t *timep,uint32_t msec);

void delay_ms(uint32_t ms);
void delay_sensor(uint32_t msec);
void charge_turn_delay_sensor(uint32_t msec);

void delay_key_wifi_sensers(uint32_t msec);
//ÑÓ³Ùº¯Êý
void delay(uint32_t msec);
void timer5_config(void);
void delay_ms_sensers(uint32_t ms);
void delay_ms_for_dist(uint32_t ms,int dis);
void delay_ms_tm5(uint32_t ms);
void turn_delay(float angle,int speed);


#endif /* __SYSTICK_H */
