
#ifndef _SPI_H_
#define _SPI_H_


#define SPI_CS(X)			((X==0)?GPIO_ResetBits(GPIOE,GPIO_Pin_0):GPIO_SetBits(GPIOE,GPIO_Pin_0)) //MPU6500Æ¬Ñ¡ÐÅºÅ

void spi_master(void);
u8 SPI1_ReadWriteByte(uint8_t byte);
void spi_cs(uint8_t cs);

#endif
