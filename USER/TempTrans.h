
extern s16 temper_trans(u16 resistor, s16 T95, s16 T40);
extern u16 ad_to_resistor(u16 adc);
s16 GetTemp(u16 adcvalue);
#if 0
extern void GetCalcTemp(float * pCalcTemp,s16 * pBlockTemp, s16 * pTargetTemp[]);
#endif

extern s16 Temps[10];

