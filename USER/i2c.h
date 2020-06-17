/*
 * 	i2c.h
 *
 *	Created on: Jun 25, 2013
 *		Author: Denis aka caat
 */

#ifndef I2C_H_
#define I2C_H_

//#include <stdint.h>
//#include "stm32f10x_gpio.h"

#define ULTRASONIC_I2CADDR 0x6c
#define DISTANCE_I2CADDR 	0x5A

extern int I2Cerror;		//need them to make local!;
extern int I2Cerrorcount;	//need to make them local;

#define I2C_ULTRASONIC_DATA_PORT	GPIOE
#define I2C_ULTRASONIC_DATA_PIN		GPIO_Pin_15

#define I2C_ULTRASONIC_CLK_PORT		GPIOB
#define I2C_ULTRASONIC_CLK_PIN		GPIO_Pin_10

#define READ_DATA()  GPIO_ReadInputDataBit(I2C_ULTRASONIC_DATA_PORT, I2C_ULTRASONIC_DATA_PIN)

#define SDAH GPIO_WriteBit(I2C_ULTRASONIC_DATA_PORT, I2C_ULTRASONIC_DATA_PIN,   Bit_SET);
#define SDAL GPIO_WriteBit(I2C_ULTRASONIC_DATA_PORT, I2C_ULTRASONIC_DATA_PIN,   Bit_RESET);

#define SCLH GPIO_SetBits(I2C_ULTRASONIC_CLK_PORT, I2C_ULTRASONIC_CLK_PIN);
#define SCLL GPIO_ResetBits(I2C_ULTRASONIC_CLK_PORT, I2C_ULTRASONIC_CLK_PIN);

void I2C1_GPIO_Init(void);
void I2C_delay(void);
void I2C1_Start(void);
void I2C1_Stop(void);
void I2C1_Ack(void);
void I2C1_NoAck(void);
void I2C1_SendByte(unsigned char SendByte);
void I2C1_WaitAck(void);
uint8_t I2C1_ReceiveByte(void);
uint8_t I2C1_ReadByte(uint8_t i2c_addr, uint8_t reg_addr);
void I2C1_WriteByte(uint8_t i2c_addr, uint8_t reg_addr, uint8_t data);
//////////////////////////////////////////////////////////////////////20200520
#if 1

#define EEPROM_ADDR 0xA0
#define I2C_PAGESIZE	8
#define SCLL2	gpio_bit_reset(SCL1_PORT, SCL1_PIN)
#define SCLH2	gpio_bit_set(SCL1_PORT, SCL1_PIN)

#define SDAL2	gpio_bit_reset(SDA1_PORT, SDA1_PIN)
#define SDAH2	gpio_bit_set(SDA1_PORT, SDA1_PIN)

#define READ_DATA2()	gpio_input_bit_get(SDA1_PORT, SDA1_PIN)

void I2C_Configuration(void);
void I2C_ReadS_24C(uint16_t addr ,uint8_t* pBuffer,uint16_t no);
void I2C_Standby_24C(void);
void I2C_ByteWrite_24C(uint16_t addr,uint8_t dat);
void I2C_PageWrite_24C(uint16_t addr,uint8_t* pBuffer, uint16_t no);
void I2C_WriteS_24C(uint16_t addr,uint8_t* pBuffer,  uint16_t no);
void I2C_Test(uint8_t WriteEn);


uint8_t i2c_read_byte(uint8_t dev_addr, uint16_t addr);
uint8_t i2c_read_data(uint8_t dev_addr, uint16_t addr, uint16_t no, uint8_t *ret_data);
#endif
///////////////////////////////////////////////////////////////////////////////////20200520




#endif /* I2C_H_*/
