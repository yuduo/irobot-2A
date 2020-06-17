
#include "sys.h"



void mdelay(u16 m)
{
	// portTickType mcnt = xTaskGetTickCount();

	// vTaskDelayUntil(&mcnt, TASK_DELAY_MS(m));
	u16 i,j;
	for(i = 0;i<m;i++){
		for(j=0;j<400;j++);
	}
	return;
}

//------------------------------------------modified by wonton2004 2016.12.27
u16 ny3p_play(u8 num)
{
#if (0 == ENABLE_VOICE)
	if(1 == cfg->speaker_able)
	{
		log_printf("ny3p_play=%d\r\n",num);
		
		if(num == VOICE_BEE_SHORT)
		{
			BEEP_SHORT_RING();
		}	
		else
			BEEP_RING();
	}
#else
#if 0//¿œ”Ô“Ù
	unsigned char i;

	log_printf("[ny3p_play]%d\r\n",num);
	//log_printf("567\r\n");
	//delay_ms(100);
	NY3P_REST(0);
	NY3P_REST(1);
	//mdelay(20);
	delay_ms(1);
	//mdelay(20);
	NY3P_REST(0);
	delay_ms(1);

	for (i = 0; i < num; i++) {
		//log_printf("123\r\n");
		NY3P_DATA(0);
		//mdelay(2);
		delay_ms(1);
		NY3P_DATA(1);
		delay_ms(1);
		//mdelay(2);
		NY3P_DATA(0);
	}
	
	return 0;
#else//–¬”Ô“Ù
	unsigned char i;

	log_printf("[ny3p_play]%d,%d\r\n",num,sys->voice);
	if(sys->voice == 1)
	{
		log_printf("Silent!");
		return 0;
	}
		
	
	//delay_ms(100);
	NY3P_REST(0);
	delay_ms(1);
	NY3P_REST(1);
	//mdelay(20);
	delay_ms(1);
	//mdelay(20);
	NY3P_REST(0);
	delay_ms(1);

	for (i = 0; i < num + 1; i++) {
		//log_printf("123\r\n");
		NY3P_DATA(0);
		//mdelay(2);
		delay_ms(1);
		NY3P_DATA(1);
		delay_ms(1);
		//mdelay(2);
		NY3P_DATA(0);
	}
	//delay_ms(1);
	

#endif
#endif
	return 0;
}
//------------------------------------------------------------------------end

void ny3p_init(void)
{
#if (1 == ENABLE_VOICE)
  	GPIO_InitTypeDef  GPIO_InitStructure; 

	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

  	GPIO_InitStructure.GPIO_Pin = NY3P_DATA_PIN;
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  	GPIO_Init(NY3P_DATA_PORT, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = NY3P_REST_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(NY3P_REST_PORT, &GPIO_InitStructure);

/*
	GPIO_InitStructure.GPIO_Pin = NY3P_POWER1_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
	GPIO_Init(NY3P_POWER1_PORT, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = NY3P_POWER2_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
	GPIO_Init(NY3P_POWER2_PORT, &GPIO_InitStructure);
*/
	NY3P_REST(0);
	NY3P_DATA(0);
	NY3P_MUTE(1);
	NY3P_POWER(1);
#endif
}

