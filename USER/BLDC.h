#ifndef BLDC_H
#define BLDC_H


#define BLDC_POLES		2
extern FlagStatus Direction;

extern void BLDC_GPIOConfig(void);
extern void BLDC_TIM1Config(void);
extern void BLDC_TIM3Config(void);
extern void huanxiang(void);
extern void BLDC_Start(void);
extern void BLDC_Stop(void);
void bdlc_set_pwm(uint16_t pwm);
void set_direction(FlagStatus dir);
void check_run_ok(void);

#endif
