#include "sys.h"

enum ENUM_TWI_REPLY
{
	TWI_NACK=0,
	TWI_ACK=1,
};
enum ENUM_TWI_BUS_STATE
{
	TWI_READY=0,
	TWI_BUS_BUSY=1,
	TWI_BUS_ERROR=2,
};

void I2C_EE_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE);
	// Configure I2C1 pins: SCL and SDA
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
	GPIO_Init(GPIOE, &GPIO_InitStructure);
}

void TWI_delay(void)
{
	uint8_t i=10; //i=10延时1.5us//这里可以优化速度 ，经测试最低到5还能写入
	while(i--);
}

/**************************************************************************
延时ms：延时的毫秒数CYCLECOUNTER / 72000000
***************************************************************************/
void DelayMs(uint16_t ms)
{
	uint16_t iq0;
	uint16_t iq1;
	for(iq0 = ms; iq0 > 0; iq0--)
	{
		for(iq1 = 11998; iq1 > 0; iq1--); // ( (6*iq1+9)*iq0+15 ) / 72000000
	}
}

uint8_t TWI_Start(void)
{
	EE_SDAH;
	EE_SCLH;
	TWI_delay();
	if(!EE_SDAread)
		return TWI_BUS_BUSY; //SDA线为低电平则总线忙,退出?
	EE_SDAL;
	TWI_delay();
	if(EE_SDAread) 
		return TWI_BUS_ERROR; //SDA线为高电平则总线出错,退出?
	EE_SCLL;
	TWI_delay();
	return TWI_READY;
}

void TWI_Stop(void)
{
	EE_SDAL;
	EE_SCLL;
	TWI_delay();
	EE_SCLH;
	TWI_delay();
	EE_SDAH;
	TWI_delay();
}

void TWI_Ack(void)
{
	EE_SCLL;
	TWI_delay();
	EE_SDAL;
	TWI_delay();
	EE_SCLH;
	TWI_delay();
	EE_SCLL;
	TWI_delay();
}

void TWI_NoAck(void)
{
	EE_SCLL;
	TWI_delay();
	EE_SDAH;
	TWI_delay();
	EE_SCLH;
	TWI_delay();
	EE_SCLL;
	TWI_delay();
}

uint8_t TWI_WaitAck(void) //返回为:=1有ACK,=0无ACK
{
	EE_SCLL;
	TWI_delay();
	EE_SDAH;
	TWI_delay();
	EE_SCLH;
	TWI_delay();
	if(EE_SDAread)
	{
		EE_SCLL;
		return 0;
	}
	EE_SCLL;
	return 1;
}
void TWI_SendByte(uint8_t SendByte) //数据从高位到低位//
{
	uint8_t i=8;
	while(i--)
	{
		EE_SCLL;
		TWI_delay();
		if(SendByte&0x80)
			EE_SDAH;
		else
			EE_SDAL;
		SendByte<<=1;
		TWI_delay();
		EE_SCLH;
		TWI_delay();
	}
	EE_SCLL;
}

uint8_t TWI_ReceiveByte(void) //数据从高位到低位//
{
	uint8_t i=8;
	uint8_t ReceiveByte=0;
	EE_SDAH;
	while(i--)
	{
		ReceiveByte <<= 1;
		EE_SCLL;
		TWI_delay();
		EE_SCLH;
		TWI_delay();
		if(EE_SDAread)
		{
			ReceiveByte |= 0x01;
		}
	}
	EE_SCLL;
	return ReceiveByte;
}

//返回：3写入成功；0写器件地址出错，1总线忙，2出错
//写入1字节数据SendByte：待写入数据WriteAddress：待写入地址
uint8_t TWI_WriteByte(uint8_t SendByte, uint8_t WriteAddress)
{
	uint8_t i;
//	uint16_t j;

	i = TWI_Start();
	if(i)
		return i;
	TWI_SendByte( ADDR_24CXX & 0xFE);
	//写器件地址 ?写入：地址最低位是0，读取：地址最低位是1
	if(!TWI_WaitAck())
	{
		TWI_Stop();
		return 0;
	}
	TWI_SendByte(WriteAddress);	//设置起始地址
	TWI_WaitAck();
	TWI_SendByte(SendByte);	//写数据
	TWI_WaitAck();
	TWI_Stop();	
	//注意：因为这里要等待EEPROM写完，可以采用查询或延时方式(10ms)
	DelayMs(12);	//写入延时 12ms? 写周期大于10ms即可
	return 3;
}

//返回：0写器件地址出错，1总线忙，2出错,
//读出1字节数据
//ReadAddress：待读出地址
uint8_t TWI_ReadByte( uint8_t ReadAddress)
{
uint8_t i,temp;

i = TWI_Start();

if(i)
	return i;
TWI_SendByte((ADDR_24CXX & 0xFE));
//写器件地址，先执行一次伪写操作
if(!TWI_WaitAck())
{
	TWI_Stop();
	return 0;
}
TWI_SendByte(ReadAddress);//设置起始地址
TWI_WaitAck();
TWI_Start();
TWI_SendByte((ADDR_24CXX & 0xFE)|0x01);//读器件地址写入：地址最低位是0，读取：地址最低位是1
TWI_WaitAck();//*pDat = TWI_ReceiveByte();
temp = TWI_ReceiveByte();
TWI_NoAck();
TWI_Stop();
return temp;//返回的如果是0，1，2则与错误代码相同了，再考虑一下
}


/***************************************************************************
向24c256中写多个字节psrc_data：指向要写入数据数组的指针adr：24c256中要写入数据的首地址nbyte：写入的字节数返回值:? 0：执行完毕；1：执行出现错误形参中：C02只有一个地址adr；C256中有高位地址hadr和低位地址ladr
***************************************************************************/
uint8_t I2C_EE_BufferWrite(uint8_t *psrc_data,uint8_t adr,uint8_t nbyte)
{
uint8_t i;
for(;nbyte!=0;nbyte--)
{
i = TWI_Start();

if(i)
return i;
TWI_SendByte( ADDR_24CXX & 0xFE);//写器件地址
if(!TWI_WaitAck())
{
TWI_Stop();
return 0;
}
TWI_SendByte(adr);	//设置起始地址
TWI_WaitAck();
TWI_SendByte(*psrc_data);		//写数据
TWI_WaitAck();
psrc_data++;//指向待写数据的指针加1
adr++;//对24C08的操作地址加1
TWI_Stop();//注意：因为这里要等待EEPROM写完，可以采用查询或延时方式(10ms)
DelayMs(12);//写入延时 12ms? 写周期大于10ms即可
}
return 0;
}

/***************************************************************************
从24c02读多个字节pdin_data：指向要保存读出数据的数组的指针
adr：24c02中要读出数据的首地址
nbyte：读出的字节数返回值:? 0：执行完毕；1：执行出现错误
***************************************************************************/
uint8_t I2C_EE_BufferRead(uint8_t *pdin_data,uint8_t adr,uint8_t nbyte)
{
	uint8_t i;
	i = TWI_Start();
	if(i)
		return i;

	TWI_SendByte((ADDR_24CXX & 0xFE));//写器件地址，先执行一次伪写操作
	if(!TWI_WaitAck())
	{
		TWI_Stop();
		return 0;
	}
	TWI_SendByte(adr);	//设置起始地址
	TWI_WaitAck();
	TWI_Start();
	TWI_SendByte((ADDR_24CXX & 0xFE)|0x01); //读器件地址写入：地址最低位是0，读取：地址最低位是1
	TWI_WaitAck();
	while(nbyte!=1)//读入前(nbyte-1)个字节
	{
		*pdin_data = TWI_ReceiveByte();//循环从24C02中读数据，存入pdin_data所指的存储器中
		TWI_Ack();//IIC应答
		pdin_data++;//指向存储读入数据的存储器指针加1
		nbyte--;//剩余要读入的字节减1
	};
	*pdin_data = TWI_ReceiveByte();//读入最后一个字节
	TWI_NoAck();//IIC无应答操作
	TWI_Stop();
	return 0;
 }

 /*
 void TWI_24CXX_Write(uint8_t* pDat, uint8_t nAddr, uint8_t nLen)
 {
	 uint16_t i;
	 for(i=0;i<nLen;i++)
	 {
	 	TWI_WriteByte(*(pDat+i), nAddr+i);
	 }
 }

 void TWI_24CXX_Read(uint8_t* pDat, uint8_t nAddr, uint8_t nLen)
 {
	 uint16_t i;
	 for(i=0; i<nLen; i++)
	 	*(pDat+i) = TWI_ReadByte(nAddr+i);
 }
 */
