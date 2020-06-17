

#include "sys.h"
 int I2Cerror;		//need them to make local!;
 int I2Cerrorcount;	//need to make them local

//I2C1的模拟IO口初始化
void I2C1_GPIO_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;


	GPIO_InitStructure.GPIO_Pin =I2C_ULTRASONIC_CLK_PIN;//SDA1_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(I2C_ULTRASONIC_CLK_PORT, &GPIO_InitStructure);//SDA1_PORT

		GPIO_InitStructure.GPIO_Pin = I2C_ULTRASONIC_DATA_PIN ; //KEY1 KEY2 KEY3 KEY4
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(I2C_ULTRASONIC_DATA_PORT, &GPIO_InitStructure);
	GPIO_ResetBits(I2C_ULTRASONIC_DATA_PORT, I2C_ULTRASONIC_DATA_PIN);
	

}

void I2C_delay(void)
{
//   uint8_t i = 10;  //3 for 400khz
//
//   while(i)
//   {
//		i--;
//   }
	int8_t i;
	for(i = 11; i > 0; i--);//3->8->11->20
}

void I2C1_Start(void)
{
	SDAH;
	SCLH;
	I2C_delay();
	SDAL;
	I2C_delay();
	SCLL;
	I2C_delay();
	//while(1);

}

void I2C1_Stop(void)
{
	SCLL;
	I2C_delay();
	SDAL;
	I2C_delay();
	SCLH;
	I2C_delay();
	SDAH;
	I2C_delay();
}

void I2C1_Ack(void)
{ 
	SCLL;
	I2C_delay();
	SDAL;
	I2C_delay();
	SCLH;
	I2C_delay();
	SCLL;
	I2C_delay();
}


void I2C1_NoAck(void)
{ 
	SCLL;
	I2C_delay();
	SDAH;
	I2C_delay();
	SCLH;
	I2C_delay();
	SCLL;
	I2C_delay();
}

void I2C1_SendByte(unsigned char SendByte) 
{
//    int8_t i = 8;
	unsigned char i = 8;
    while(i--)
//	for(; i > 0; i--)	
    {
        SCLL;
        I2C_delay();
        if(SendByte & 0x80) 
		{
			SDAH;
		}

        if(!(SendByte & 0x80))
		{
			SDAL;
		}  
        SendByte<<=1;
        I2C_delay();
		SCLH;
        I2C_delay();
    }
    SCLL;
}

uint8_t I2C1_ReceiveByte(void)  
{ 
//	int InputLevel[9];
  /*  unsigned char i=8;
    unsigned char ReceiveByte=0;


    SDAH;    
    while(i--)
    {
      ReceiveByte<<=1;      
      SCLL;
      I2C_delay();
      SCLH;
      I2C_delay(); 
      if(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_11)==1)
      {
        ReceiveByte|=0x01;
      }
    }
    SCLL;
    return ReceiveByte;*/
	
	unsigned char i=8;
	unsigned char ReceiveByte=0;
	uint8_t t;
	uint8_t data;

	SDAH;    
	while(i--)
	{
		ReceiveByte<<=1;      
		SCLL;
		I2C_delay();
		SCLH;
		/*
		InputLevel[0]=GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_11);
		InputLevel[1]=GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_11);
		InputLevel[2]=GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_11);
		InputLevel[3]=GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_11);
		InputLevel[4]=GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_11);
		InputLevel[5]=GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_11);
		InputLevel[6]=GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_11);
		InputLevel[7]=GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_11);
		InputLevel[8]=GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_11);
		if((InputLevel[0]+InputLevel[1]+InputLevel[2]+InputLevel[3]+InputLevel[4]+InputLevel[5]+InputLevel[6]+InputLevel[7]+InputLevel[8])>=4)
		{
			ReceiveByte|=0x01;
		}
		*/ 
		data = 0;
		for(t = 0; t < 8; t++)
		{
			//data += GPIO_ReadInputDataBit(SDA1_PORT, SDA1_PIN);
			//data += GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_15);
			data += (GPIOE->IDR & GPIO_Pin_15)?1:0;//GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_15);
		}
		if(data >= 4)
		{
			ReceiveByte |= 0x01;
		}

	}
	SCLL;
	return ReceiveByte;	
}

void I2C1_WaitAck(void)   
{
	SCLL;
	I2C_delay();
	SDAH;   
	I2C_delay();
	SCLH;
	I2C_delay();
	if(GPIOE->IDR & GPIO_Pin_15)
	//if(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_15)==1)
	//if(GPIO_ReadInputDataBit(SDA1_PORT, SDA1_PIN)==1)
	{
		I2Cerror=1; 
		//DEBUG_LEDon; 
		I2Cerrorcount++;
	}
	SCLL;
 
}


uint8_t I2C1_ReadByte(uint8_t i2c_addr, uint8_t reg_addr)
{
	uint8_t res;
	
    I2C1_Start();
    I2C1_SendByte((i2c_addr & 0xfe));//fe-0(Write)
    I2C1_WaitAck();
    I2C1_SendByte(reg_addr);//fe-0(Write)
    I2C1_WaitAck();
	
    //I2C1_Stop();
    
    I2C1_NoAck();
    //I2C_delay();
    //I2C_delay();
    //I2C_delay();
    
    I2C1_Start();
    I2C1_SendByte((i2c_addr | 0x01));//fe-0(Write)
    I2C1_WaitAck();
    

    res = I2C1_ReceiveByte(); //receive
    
    //I2C1_WaitAck();
    I2C1_NoAck();
	I2C1_Stop();
	return res;
}


void I2C1_WriteByte(uint8_t i2c_addr, uint8_t reg_addr, uint8_t data)
{
//	uint8_t res;
	
	
    I2C1_Start();
    I2C1_SendByte((i2c_addr & 0xfe));//fe-0(Write)
    I2C1_WaitAck();
    I2C1_SendByte(reg_addr);//fe-0(Write)
    I2C1_WaitAck();
    
    //I2C1_Start();
    //I2C1_SendByte((i2c_addr | 0x01));//fe-0(Write)
    //I2C1_WaitAck();
    I2C1_SendByte(data);//fe-0(Write)
    
    I2C1_NoAck();
	I2C1_Stop();
}
#if 1
///////////////////////////////////////////////////////////////////////20200520
uint8_t i2c_read_byte(uint8_t dev_addr, uint16_t addr)
{
	uint8_t data = 0;

    data = I2C1_ReadByte(dev_addr,addr);

	return data;
}

uint8_t i2c_read_data(uint8_t dev_addr, uint16_t addr, uint16_t no, uint8_t *ret_data)
{
	uint8_t data = 0;


    I2C1_Start();
    I2C1_SendByte((dev_addr & 0xfe));//fe-0(Write)
    I2C1_WaitAck();
   // I2C1_SendByte((uint8_t)(addr >> 8));//fe-0(Write)
   // I2C1_WaitAck();
    I2C1_SendByte((uint8_t)(addr & 0xff));//fe-0(Write)
    I2C1_WaitAck();


    //I2C2_NoAck();

    I2C1_Start();
    I2C1_SendByte((dev_addr | 0x01));//fe-0(Write)
    I2C1_WaitAck();

    while (no)
    {
	    *ret_data = I2C1_ReceiveByte();
	    if(no == 1)
	    {
		    I2C1_NoAck();
			I2C1_Stop();
	    }
	    else
	    {
	    	I2C1_Ack();
	    }
		//printf("%d-%02X\r\n",no,*ret_data);
	    ret_data++;
	    /* Decrement the read bytes counter */
	    no--;
    }

    //printf("[i2c_read_data]out\r\n");

	return data;
}

/***************************************************
**oˉêy??:I2C_ReadS
**1|?ü:?áè?24C02?à??×??ú
**×￠òaê???:24C02ê?256×??ú,8??μ??·,A0-A21ì?¨?a0,′ó?÷?tμ??·?aEEPROM_ADDR
***************************************************/
void I2C_ReadS_24C(uint16_t addr ,uint8_t* pBuffer,uint16_t no)
{
    if(no==0)
		return;
	i2c_read_data(EEPROM_ADDR, addr, no,  pBuffer);
}

/****************************************************
**oˉêy??:I2C_Standby_24C
**1|?ü:24Cê?·?×?±?o??ùD′è?μ??D??
**×￠òaê???:±?oˉêy?éò?àí?a?a:?D?|
****************************************************/
void I2C_Standby_24C(void)
{
  volatile uint16_t SR1_Tmp;
  do
  {
    /*?eê???*/
	//i2c_ack_config(I2C0, I2C_ACK_ENABLE);
    I2C1_Start();
    I2C1_SendByte((EEPROM_ADDR & 0xfe));//fe-0(Write)
    I2Cerror = 0;
    I2C1_WaitAck();
  }while(I2Cerror);
}

/*************************************************
**oˉêy??:I2C_ByteWrite_24C
**1|?ü:D′ò???×??ú
**×￠òaê???:×?D′è?í??ùDèòaμ÷ó??|?D??
*************************************************/
void I2C_ByteWrite_24C(uint16_t addr,uint8_t dat)
{
    I2C1_Start();
    I2C1_SendByte((EEPROM_ADDR & 0xfe));//fe-0(Write)
    I2C1_WaitAck();
   // I2C1_SendByte((uint8_t)(addr >> 8));//fe-0(Write)
   // I2C1_WaitAck();
    I2C1_SendByte((uint8_t)(addr & 0xff));//fe-0(Write)
    I2C1_WaitAck();

    //I2C2_Start();
    //I2C2_SendByte((EEPROM_ADDR & 0xfe));//fe-0(Write)
    //I2C2_WaitAck();

    I2C1_SendByte(dat);//fe-0(Write)

    I2C1_NoAck();
	I2C1_Stop();

  	I2C_Standby_24C();
}

/*************************************************
**oˉêy??:I2C_PageWrite_24C
**1|?ü:D′è?ò?ò3(ò??ú)
**×￠òaê???:′?oˉêy1?èoD′è?μ÷ó?
*************************************************/
void I2C_PageWrite_24C(uint16_t addr,uint8_t* pBuffer, uint16_t no)
{
    I2C1_Start();
    I2C1_SendByte((EEPROM_ADDR & 0xfe));//fe-0(Write)
    I2C1_WaitAck();
   // I2C1_SendByte((uint8_t)(addr >> 8));//fe-0(Write)
   // I2C1_WaitAck();
    I2C1_SendByte((uint8_t)(addr & 0xff));//fe-0(Write)
    I2C1_WaitAck();


    //I2C2_Start();
    //I2C2_SendByte((EEPROM_ADDR & 0xfe));//fe-0(Write)
    //I2C2_WaitAck();

	while(no--)
	{
	    I2C1_SendByte(*pBuffer++);//fe-0(Write)
	    //if(no != 1)
	    	I2C1_WaitAck();
	}
	I2C1_Stop();
}



/*************************************************
**oˉêy??:I2C_WriteS_24C
**1|?ü:ò3D′è?24C
**×￠òaê???:24C02×??à?êDíò?′?D′è?8??×??ú
*************************************************/
void I2C_WriteS_24C(uint16_t addr,uint8_t* pBuffer,  uint16_t no)
{
	uint16_t temp;

	//1.?è°?ò32?????μ?2?·?D′è?
	temp=addr % I2C_PAGESIZE;
	if(temp)
	{
		temp=I2C_PAGESIZE-temp;
		if(no < temp)
			temp = no;
		I2C_PageWrite_24C(addr,pBuffer,  temp);
		no-=temp;
		addr+=temp;
		pBuffer+=temp;
		I2C_Standby_24C();
	}
	//2.′óò3?????aê?D′
	while(no)
	{
		if(no>=I2C_PAGESIZE)
		{
			I2C_PageWrite_24C(addr,pBuffer,  I2C_PAGESIZE);
			no-=I2C_PAGESIZE;
			addr+=I2C_PAGESIZE;
			pBuffer+=I2C_PAGESIZE;
			I2C_Standby_24C();
		}
		else
		{
			I2C_PageWrite_24C(addr,pBuffer,  no);
			no=0;
			I2C_Standby_24C();
		}
        	delay_ms(10);
	}
}
////////////////////////////////////////////////////////////////////////////////////20200520
#endif



