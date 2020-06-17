#include "sys.h"


void spi_master(void)//
{

	GPIO_InitTypeDef GPIO_InitStructure;
	//RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOC, ENABLE);
	
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;//MOSI
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_0;//CLK CS
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOE, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_3 ;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;//GPIO_Mode_IN_FLOATING;   //MISO
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	SPI_CS(1);
	delay_ms(10);
	SPI_CS(0);
	/*
	while(1)
	{
			GPIO_SetBits(GPIOB, GPIO_Pin_4); 
			delay_ms(10);
			GPIO_ResetBits(GPIOB, GPIO_Pin_4);
		delay_ms(10);
	}
	*/


}
u8 SPI1_ReadWriteByte(uint8_t byte)
{
		uint8_t i; 
		u8 Temp=0x00;
		unsigned char SDI; 
		int k;
	 // SPI_CS(0);
    for (i = 0; i < 8; i++)
    {
				GPIO_SetBits(GPIOE, GPIO_Pin_1);//sclk = 0;//??????
        if (byte & 0x80)      
        {
						GPIO_SetBits(GPIOB, GPIO_Pin_4); //    //SO=1 
        }
        else
        {
						GPIO_ResetBits(GPIOB, GPIO_Pin_4);//     //SO=0
        }
         byte <<= 1;  
				 for(k=0;k<2;k++);
         GPIO_ResetBits(GPIOE, GPIO_Pin_1);//    //sclk = 1; ????
				 for(k=0;k<2;k++);
         SDI = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_3);//??si?????
         Temp<<=1;
        
				if(SDI)       //??1?
				{
						Temp++;  //?1  ??????????1   ????<<=1
				}
				GPIO_SetBits(GPIOE, GPIO_Pin_1);//sclk = 0;//   ???? 
				for(k=0;k<20;k++);
			}
//	SPI_CS(1);
		//for(k=0;k<3;k++);
    return Temp; //????miso????     
 
}      
void spi_cs(uint8_t cs)
{
	SPI_CS(cs);
}
