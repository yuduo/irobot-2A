/*********ÎÄ¼þÃû£ºi2c_ee.h**********/
/* Define to prevent recursive inclusion ------------------------------------ */
#ifndef __I2C_EE_H
#define __I2C_EE_H
/* Includes ------------------------------------------------------------------*/
////#include "sys.h"
/* Exported macro ------------------------------------------------------------*/
#define ADDR_24CXX		0xA0
#define EE_SCLH				GPIOB->BSRR |= GPIO_Pin_10
#define EE_SCLL				GPIOB->BRR   |= GPIO_Pin_10
#define EE_SDAH				GPIOE->BSRR |= GPIO_Pin_15
#define EE_SDAL				GPIOE->BRR  |= GPIO_Pin_15
#define EE_SCLread			GPIOB->IDR  & GPIO_Pin_10
#define EE_SDAread			GPIOE->IDR  & GPIO_Pin_15
/* Exported functions ------------------------------------------------------- */
extern uint8_t I2C_EE_BufferWrite(uint8_t *psrc_data,uint8_t adr,uint8_t nbyte);
extern void I2C_EE_Init(void);uint8_t I2C_EE_BufferWrite(uint8_t *psrc_data,uint8_t adr,uint8_t nbyte);
extern uint8_t I2C_EE_BufferRead(uint8_t *pdin_data,uint8_t adr,uint8_t nbyte);
#endif /* __I2C_EE_H */
