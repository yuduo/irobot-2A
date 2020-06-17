#include "sys.h"

#include "stm32f10x.h"
/**************************************************µÚ1²½£º·½ÏòĞÅºÅ²É¼¯£¬ÅäÖÃËùÓÃµÄADC***************************************/

#if 1
#define ADC1_DR_Address    ((u32)0x4001244C)

__IO u16 adc_converted_value[ADC_CHANNEL_NUM+1];
//__IO u16 ADC_ConvertedValueLocal;


/*
 * º¯ÊıÃû£ºADC1_GPIO_Config
 * ÃèÊö  £ºÊ¹ÄÜADC1ºÍDMA1µÄÊ±ÖÓ£¬³õÊ¼»¯PC0£¬pc1
 * ÊäÈë  : ÎŞ
 * Êä³ö  £ºÎŞ
 * µ÷ÓÃ  £ºÄÚ²¿µ÷ÓÃ
 */
static void ADC1_GPIO_Config(void)
{
/*
µÚ¶şÓ²¼ş°æ±¾
14	PA3	×ó±ßºìÍâÅö×²´«¸ĞÆ÷	ADC
15	PA4	×óÖĞºìÍâÅö×²´«¸ĞÆ÷	ADC
16	PA5	×óÖĞºìÍâÅö×²´«¸ĞÆ÷	ADC
17	PA6	ÖĞ¼äºìÍâÅö×²´«¸ĞÆ÷	ADC
18	PA7	ÓÒÖĞºìÍâÅö×²´«¸ĞÆ÷	ADC
19	PB0	ÓÒÖĞºìÍâÅö×²´«¸ĞÆ÷	ADC
20	PB1	ÓÒ±ßºìÍâÅö×²´«¸ĞÆ÷	ADC
µÚÈıÓ²¼ş°æ±¾
PB1	×ó±ßºìÍâÅö×²´«¸ĞÆ÷	ADC9
PB0	×óÖĞºìÍâÅö×²´«¸ĞÆ÷	ADC8
PB0	×óÖĞºìÍâÅö×²´«¸ĞÆ÷	ADC8
PA7	×óÖĞºìÍâÅö×²´«¸ĞÆ÷	ADC7
PA3	ÖĞ¼äºìÍâÅö×²´«¸ĞÆ÷	ADC3
PA4	ÓÒÖĞºìÍâÅö×²´«¸ĞÆ÷	ADC4
PA5	ÓÒÖĞºìÍâÅö×²´«¸ĞÆ÷	ADC5
PA6	ÓÒ±ßºìÍâÅö×²´«¸ĞÆ÷	ADC6

PA0	×ó±ß·ÀµøÂä	ADC 		ADC0
PA1	ÖĞ¼ä·ÀµøÂä	ADC			ADC1
PA2	ÓÒ±ß·ÀµøÂä	ADC			ADC2

¾Ã²ı°æ±¾
/--------------------------
PB1 ×ó±ßºìÍâ²â¾à´«¸ĞÆ÷	ADC9
PA5	×ó±ßºìÍâÅö×²´«¸ĞÆ÷	ADC5
PA3	ÖĞ¼äºìÍâ²â¾à´«¸ĞÆ÷	ADC3
PA4	ÓÒ±ßºìÍâÅö×²´«¸ĞÆ÷	ADC4
PA6	ÓÒ±ßºìÍâ²â¾à´«¸ĞÆ÷	ADC6

PA0	×ó±ß·ÀµøÂä			ADC0
PA2	ÓÒ±ß·ÀµøÂä			ADC2
----------------------------/

	PC4	ÖĞÉ¨µç»úµÄ¹ıÁ÷¼ì²â£¨H/LµçÆ½¼ì²â£©	ADC
	PC5	ÊÇÎü³¾µç»úµÄ¹ıÁ÷¼ì²â£¨H/LµçÆ½¼ì²â£©


PC3		AD13 	³äµçµçÁ÷²ÉÑù
PC2		AD12 	³äµçµçÑ¹

PC0		AD10 	ºóÂÖÓÒµç»úµçÁ÷
PC1     AD11	ºó×óÂÖµç»úµçÁ÷

*/
	GPIO_InitTypeDef GPIO_InitStructure;

	/* Enable DMA clock */
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

	/* Enable ADC1 and GPIOC clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 | RCC_APB2Periph_GPIOC, ENABLE);

	/* Configure PC.01  as analog input */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1 |GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init(GPIOA, &GPIO_InitStructure);				// PC1,ÊäÈëÊ±²»ÓÃÉèÖÃËÙÂÊ


	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1|GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	//PC4	ÖĞÉ¨µç»úµÄ¹ıÁ÷¼ì²â£  PC5	ÊÇÎü³¾µç»úµÄ¹ıÁ÷¼ì²â
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4|GPIO_Pin_3|GPIO_Pin_2|GPIO_Pin_0|GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

}


/* º¯ÊıÃû£ºADC1_Mode_Config
 * ÃèÊö  £ºÅäÖÃADC1µÄ¹¤×÷Ä£Ê½ÎªMDAÄ£Ê½
 * ÊäÈë  : ÎŞ
 * Êä³ö  £ºÎŞ
 * µ÷ÓÃ  £ºÄÚ²¿µ÷ÓÃ
 */
static void ADC1_Mode_Config(void)
{
	DMA_InitTypeDef DMA_InitStructure;
	ADC_InitTypeDef ADC_InitStructure;

	/* DMA channel1 configuration */
  DMA_DeInit(DMA1_Channel1);
  DMA_InitStructure.DMA_PeripheralBaseAddr = ADC1_DR_Address;
  DMA_InitStructure.DMA_MemoryBaseAddr = (u32)adc_converted_value;
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
  DMA_InitStructure.DMA_BufferSize = ADC_CHANNEL_NUM;
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;    /*Éè¼ÆÄÚ´æµØÖ·¼ÓÒ»*/
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;
  DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
  DMA_Init(DMA1_Channel1, &DMA_InitStructure);

  /* Enable DMA channel1 */
  DMA_Cmd(DMA1_Channel1, ENABLE);

  /* ADC1 configuration */
  ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
  ADC_InitStructure.ADC_ScanConvMode = ENABLE;
  ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
  ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
  ADC_InitStructure.ADC_NbrOfChannel = ADC_CHANNEL_NUM;
  ADC_Init(ADC1, &ADC_InitStructure);

  /* ADC1 regular channel11 configuration
	//ADC_RegularChannelConfig(ADC1, ADC_Channel_10, 1, ADC_SampleTime_55Cycles5);
	//ADC_RegularChannelConfig(ADC1, ADC_Channel_11, 2, ADC_SampleTime_55Cycles5);
	//ADC_RegularChannelConfig(ADC1, ADC_Channel_12, 3, ADC_SampleTime_55Cycles5);

	PC3		AD13 ³äµçµçÁ÷²ÉÑù
	PC2		AD12 ³äµçµçÑ¹
	PC0		ºóÂÖÓÒµç»úµçÁ÷
	PC1     ºó×óÂÖµç»úµçÁ÷
 */

	//ÖĞÉ¨µç»úºÍÎü³¾µç»úµÄAD¿Ú
	ADC_RegularChannelConfig(ADC1, ADC_Channel_13, 16, ADC_SampleTime_55Cycles5);	//³äµçµçÁ÷
	ADC_RegularChannelConfig(ADC1, ADC_Channel_12, 15, ADC_SampleTime_55Cycles5);	//³äµçµçÑ¹

	ADC_RegularChannelConfig(ADC1, ADC_Channel_11, 14, ADC_SampleTime_55Cycles5);	//ºóÂÖÓÒµç»úµçÁ÷
	ADC_RegularChannelConfig(ADC1, ADC_Channel_10, 13, ADC_SampleTime_55Cycles5);	//ºóÂÖ×óµç»úµçÁ÷

	ADC_RegularChannelConfig(ADC1, ADC_Channel_1, 12, ADC_SampleTime_55Cycles5);	//»Ò¼ì
	////ADC_RegularChannelConfig(ADC1, ADC_Channel_15, 12, ADC_SampleTime_55Cycles5);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_14, 11, ADC_SampleTime_55Cycles5);//ÖĞÉ¨µç»ú¹ıÁ÷¼ì²â

	/*
	ADC_RegularChannelConfig(ADC1, ADC_Channel_3, 7, ADC_SampleTime_55Cycles5);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_4, 6, ADC_SampleTime_55Cycles5);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_5, 5, ADC_SampleTime_55Cycles5);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_6, 4, ADC_SampleTime_55Cycles5);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_7, 3, ADC_SampleTime_55Cycles5);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_8, 2, ADC_SampleTime_55Cycles5);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_9, 1, ADC_SampleTime_55Cycles5);
	*/
	//ÆßÂ·Ç°ÖÃºìÍâ
#if VER_BORD_2
/*
14	PA3	×ó±ßºìÍâÅö×²´«¸ĞÆ÷	ADC
15	PA4	×óÖĞºìÍâÅö×²´«¸ĞÆ÷	ADC
16	PA5	×óÖĞºìÍâÅö×²´«¸ĞÆ÷	ADC
17	PA6	ÖĞ¼äºìÍâÅö×²´«¸ĞÆ÷	ADC
18	PA7	ÓÒÖĞºìÍâÅö×²´«¸ĞÆ÷	ADC
19	PB0	ÓÒÖĞºìÍâÅö×²´«¸ĞÆ÷	ADC
20	PB1	ÓÒ±ßºìÍâÅö×²´«¸ĞÆ÷	ADC

*/
	ADC_RegularChannelConfig(ADC1, ADC_Channel_3, 1, ADC_SampleTime_55Cycles5);	//×ó±ß
	ADC_RegularChannelConfig(ADC1, ADC_Channel_4, 2, ADC_SampleTime_55Cycles5);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_5, 3, ADC_SampleTime_55Cycles5);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_6, 4, ADC_SampleTime_55Cycles5);	//ÖĞ¼ä
	ADC_RegularChannelConfig(ADC1, ADC_Channel_7, 5, ADC_SampleTime_55Cycles5);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_8, 6, ADC_SampleTime_55Cycles5);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_9, 7, ADC_SampleTime_55Cycles5);	//ÓÒ±ß
#else

#if (1 == HOOVER)
/*
PA5	×ó±ßºìÍâ²â¾à´«¸ĞÆ÷	ADC9	1
PB1	×óÖĞºìÍâÅö×²´«¸ĞÆ÷	ADC8	2
PA3	ÖĞ¼äºìÍâÅö×²´«¸ĞÆ÷	ADC3	4
PA6	ÓÒÖĞºìÍâÅö×²´«¸ĞÆ÷	ADC4	5
PA4	ÓÒ±ßºìÍâ²â¾à´«¸ĞÆ÷	ADC6	7
*/
	ADC_RegularChannelConfig(ADC1, ADC_Channel_9, 1, ADC_SampleTime_55Cycles5); //×ó±ß
	ADC_RegularChannelConfig(ADC1, ADC_Channel_5, 3, ADC_SampleTime_55Cycles5);	//Åö×²×ó
	//ADC_RegularChannelConfig(ADC1, ADC_Channel_3, 4, ADC_SampleTime_55Cycles5); //LAOBANBENÖĞ¼ä
	ADC_RegularChannelConfig(ADC1, ADC_Channel_7, 4, ADC_SampleTime_55Cycles5); //ÖĞ¼ä
	ADC_RegularChannelConfig(ADC1, ADC_Channel_4, 5, ADC_SampleTime_55Cycles5);	//Åö×²ÓÒ
	ADC_RegularChannelConfig(ADC1, ADC_Channel_6, 7, ADC_SampleTime_55Cycles5); //ÓÒ±ß
	ADC_RegularChannelConfig(ADC1, ADC_Channel_8, 9, ADC_SampleTime_55Cycles5);//Ç°ÃæÈı¸öµÆ
#else
/*
PB1	×ó±ßºìÍâÅö×²´«¸ĞÆ÷	ADC9	1
PB0	×óÖĞºìÍâÅö×²´«¸ĞÆ÷	ADC8	2
PA7	×óÖĞºìÍâÅö×²´«¸ĞÆ÷	ADC7	3
PA3	ÖĞ¼äºìÍâÅö×²´«¸ĞÆ÷	ADC3	4
PA4	ÓÒÖĞºìÍâÅö×²´«¸ĞÆ÷	ADC4	5
PA5	ÓÒÖĞºìÍâÅö×²´«¸ĞÆ÷	ADC5	6
PA6	ÓÒ±ßºìÍâÅö×²´«¸ĞÆ÷	ADC6	7

*/
	ADC_RegularChannelConfig(ADC1, ADC_Channel_9, 1, ADC_SampleTime_55Cycles5); //×ó±ß
	ADC_RegularChannelConfig(ADC1, ADC_Channel_8, 2, ADC_SampleTime_55Cycles5);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_7, 3, ADC_SampleTime_55Cycles5);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_3, 4, ADC_SampleTime_55Cycles5); //ÖĞ¼ä
	ADC_RegularChannelConfig(ADC1, ADC_Channel_4, 5, ADC_SampleTime_55Cycles5);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_5, 6, ADC_SampleTime_55Cycles5);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_6, 7, ADC_SampleTime_55Cycles5); //ÓÒ±ß
#endif
#endif

#if (1 == HOOVER)
	ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 8, ADC_SampleTime_55Cycles5);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_2, 10, ADC_SampleTime_55Cycles5);

#else
	ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 8, ADC_SampleTime_55Cycles5);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_1, 9, ADC_SampleTime_55Cycles5);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_2, 10, ADC_SampleTime_55Cycles5);
#endif
  /* Enable ADC1 DMA */
  ADC_DMACmd(ADC1, ENABLE);

  /* Enable ADC1 */
  ADC_Cmd(ADC1, ENABLE);

  /* Enable ADC1 reset calibaration register */
  ADC_ResetCalibration(ADC1);
  /* Check the end of ADC1 reset calibration register */
  while(ADC_GetResetCalibrationStatus(ADC1));

  /* Start ADC1 calibaration */
  ADC_StartCalibration(ADC1);
  /* Check the end of ADC1 calibration */
  while(ADC_GetCalibrationStatus(ADC1));

  /* Start ADC1 Software Conversion */
  ADC_SoftwareStartConvCmd(ADC1, ENABLE);
}

/*
 * º¯ÊıÃû£ºADC1_Init
 * ÃèÊö  £ºÎŞ
 * ÊäÈë  £ºÎŞ
 * Êä³ö  £ºÎŞ
 * µ÷ÓÃ  £ºÍâ²¿µ÷ÓÃ
 */
void adc1_init(void)
{
	ADC1_GPIO_Config();
	ADC1_Mode_Config();
}


#else
#define ADC1_DR_Address    ((u32)0x40012400+0x4c)

__IO uint16_t adc_converted_value[ADC_CHANNEL_NUM+1];//adÖµÔİ´æÊı×é


void adc_init(void )
{

/* Private function prototypes -----------------------------------------------*/
	ADC_InitTypeDef ADC_InitStructure;
	DMA_InitTypeDef DMA_InitStructure;
	GPIO_InitTypeDef  GPIO_InitStructure;

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE); // Ê¹ÄÜ MDA ÖØÒª£¡£¡£¡
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1,ENABLE);// Ê¹ÄÜ ADC1 ÖØÒª£¡£¡£¡


  //ADC_InitTypeDef  ADC_InitStructure;
  //PC1/2/3 ×÷ÎªÄ£ÄâÍ¨µÀÊäÈëÒı½Å
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;                //Ä£ÄâÊäÈëÒı½Å
  GPIO_Init(GPIOC, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;                //Ä£ÄâÊäÈëÒı½Å
  GPIO_Init(GPIOA, &GPIO_InitStructure);


	/*DMA1 channel1 configuration ----------------------------------------------*/
  	DMA_DeInit(DMA1_Channel1);			   //½«DMAµÄÍ¨µÀ1¼Ä´æÆ÷ÖØÉèÎªÈ±Ê¡Öµ
  	DMA_InitStructure.DMA_PeripheralBaseAddr = ADC1_DR_Address;	  //¶¨ÒåDMAÍâÉè»ùµØÖ·ÎªADC1
  	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)adc_converted_value; //¶¨ÒåDMAÄÚ´æ»ùµØÖ·
  	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;	  //ÍâÉè×÷ÎªÊı¾İ´«ÊäµÄÀ´Ô´
  	DMA_InitStructure.DMA_BufferSize = ADC_CHANNEL_NUM;	  //¶¨ÒåÖ¸¶¨DMAÍ¨µÀµÄDMA»º´æµÄ´óĞ¡
  	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;  //ÍâÉèµØÖ·¼Ä´æÆ÷²»±ä
  	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Disable;		//ÄÚ´æµØÖ·¼Ä´æÆ÷²»±ä
  	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;	//Êı¾İ¿í¶ÈÎª16Î»
  	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;	//Êı¾İ¿í¶ÈÎª16Î»
  	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;	  //¹¤×÷ÔÚÑ­»·»º´æÄ£Ê½
  	DMA_InitStructure.DMA_Priority = DMA_Priority_High;	//DMAÍ¨µÀ1ÓµÓĞ¸ßÓÅÏÈ¼¶
  	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;	//DMAÍ¨µÀ1Ã»ÓĞÉèÖÃÎªÄÚ´æµ½ÄÚ´æ´«Êä
  	DMA_Init(DMA1_Channel1, &DMA_InitStructure);

  	/* Enable DMA1 channel1 */
  	DMA_Cmd(DMA1_Channel1, ENABLE);

  	/* ADC1 configuration ------------------------------------------------------*/
  	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;	 //¶ÀÁ¢¹¤×÷Ä£Ê½
  	ADC_InitStructure.ADC_ScanConvMode = ENABLE;		  //É¨ÃèÄ£Ê½
  	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;	   //Á¬Ğø×ª»»Ä£Ê½
  	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;	//Èí¼ş¿ØÖÆ×ª»»
  	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;	 //ÓÒ¶ÔÆë
  	ADC_InitStructure.ADC_NbrOfChannel =ADC_CHANNEL_NUM;
  	ADC_Init(ADC1, &ADC_InitStructure);

  	/* ADC1 regular channel_8,configuration */
  	ADC_RegularChannelConfig(ADC1, ADC_Channel_10, 1, ADC_SampleTime_55Cycles5); //×ª»»Ê±¼äÎª17.1US
  	ADC_RegularChannelConfig(ADC1, ADC_Channel_11, 2, ADC_SampleTime_55Cycles5);
  	ADC_RegularChannelConfig(ADC1, ADC_Channel_12, 3, ADC_SampleTime_55Cycles5);
  	ADC_RegularChannelConfig(ADC1, ADC_Channel_13, 4, ADC_SampleTime_55Cycles5);

  	/* Enable ADC1 DMA */
  	ADC_DMACmd(ADC1, ENABLE);

  	/* Enable ADC1 */
  	ADC_Cmd(ADC1, ENABLE);

  	/* Enable ADC1 reset calibaration register */
  	ADC_ResetCalibration(ADC1);

  	/* Check the end of ADC1 reset calibration register */
  	while(ADC_GetResetCalibrationStatus(ADC1));

  	/* Start ADC1 calibaration */
  	ADC_StartCalibration(ADC1);

  	/* Check the end of ADC1 calibration */
  	while(ADC_GetCalibrationStatus(ADC1));

  	/* Start ADC1 Software Conversion */
  	ADC_SoftwareStartConvCmd(ADC1, ENABLE);
}
#endif



