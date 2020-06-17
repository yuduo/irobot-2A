#include "sys.h"
#include <stdarg.h>
#include <stdio.h>
#include "test_mode.h"


#if PRINTF_BY_BUFFER
static tRingBuffer RingBuffer_USART1_TX;
#endif
static tRingBuffer RingBuffer_USART1_RX;
#if 1//!ROOMA_GYROONLY
static tRingBuffer RingBuffer_WIFI_RX;
static tRingBuffer RingBuffer_WIFI_TX;

static tRingBuffer RingBuffer_USART4_RX;
#endif


void USART1_Enable_Tx_Interrupt(void)
{
    USART_ITConfig(USART1, USART_IT_TXE, ENABLE);
}


void Init_Usart1_Buffer(void)
{
#if PRINTF_BY_BUFFER

	RingBufferInit(&RingBuffer_USART1_TX, &USART1_Enable_Tx_Interrupt);
#endif
	RingBufferInit(&RingBuffer_USART1_RX, 0L);
}



void USART_PutChar(uint8_t ch)
{
 // while(!(USART4->SR & USART_SR_TXE));
  //USART4->DR = ch;


 #if PRINTF_BY_BUFFER
 	RingBufferPut(&RingBuffer_USART1_TX, ch, 0);
 #else
	uint16_t i;
   // while(!(UART4->SR & USART_SR_TXE));
	//while(!(USART1->SR & USART_SR_TXE));
	while(!(USART1->SR & USART_SR_TXE));
	USART1->DR = ch;
	//while( USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET );
	for(i=0;i<890;i++);
	//delay_ms(1);
 #endif
}

void USART_PutString(uint8_t * str)
{
	if(RESET == sys->upgrade_mode)
	{
	while(*str != 0)
	{
#if PRINTF_BY_BUFFER
		USART_PutChar(*str);
#else
		uint16_t i;
		//while(!(USART1->SR & USART_SR_TXE));
		USART_SendData(USART1, (unsigned char) *str);// USART1 ???? USART2 ?
		for(i=0;i<890;i++);
		//while( USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET );;
#endif
		str++;

	}
	}
}
void usart_write(uint8_t *buff,int len)
{
	int i;
	for(i=0;i<len;i++)
	{
		USART_PutChar(*buff++);
	}
}

/*******************************************************************************
* Name  : usart1_init  串口初始化
* Deion        : Configures the usartx
* Input                    : None
* Output                 : None
* Return                 : None
*******************************************************************************/
void usart1_init(void)
{

	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;


	RCC_APB2PeriphClockCmd( RCC_APB2Periph_AFIO,ENABLE);//开启端口B和复用功能时钟
 	GPIO_PinRemapConfig(GPIO_Remap_USART2,ENABLE);//使能端口重映射


//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE);




	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);




	//串口参数配置：9600,8,1，无奇偶校验，无硬流量控制 ，使能发送和接收
	 USART_InitStructure.USART_BaudRate =115200;//115200; 	//
	 USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	 USART_InitStructure.USART_StopBits = USART_StopBits_1;
	 USART_InitStructure.USART_Parity = USART_Parity_No ;
	 USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	 USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	 USART_Init(USART1, &USART_InitStructure);
	 USART_ITConfig(USART1, USART_IT_RXNE,ENABLE);//串口接收中断
#if PRINTF_BY_BUFFER
	 USART_ITConfig(USART1, USART_IT_TXE, ENABLE);
#endif
	 USART_Cmd(USART1, ENABLE);


	 Init_Usart1_Buffer();

}

void USART1_IRQHandler(void)
{
   int sr = USART1->SR;


		int i;
    if (sr & USART_FLAG_TXE)
    {
 #if PRINTF_BY_BUFFER
        tRingBuffer *rb = &RingBuffer_USART1_TX;

        if (rb->Read != rb->Write)
        {
            USART1->DR = rb->Buffer[rb->Read];

            if (rb->Read + 1 == RingBufferSize(rb))
            {
                rb->Read = 0;
            }
            else
            {
                rb->Read++;
            }
        }
        else
#endif
        {
            USART_ITConfig(USART1, USART_IT_TXE, DISABLE);
            i=0;
            i++;i++;
          //  asm volatile("nop");
           // asm volatile("nop");
        }
    }

    if (sr & USART_FLAG_RXNE)
    {
        tRingBuffer *rb = &RingBuffer_USART1_RX;

        unsigned char c = USART1->DR;


        if (RingBufferFillLevel(rb) + 1 == RingBufferSize(rb))
        {
            rb->Overrun++;
            return;
        }

        rb->Buffer[rb->Write] = c;

        if (rb->Write + 1 == RingBufferSize(rb))
        {
            rb->Write = 0;
        }
        else
        {
            rb->Write++;
        }
    }

}

#if DP_PANEL_EN == 1 || DP_PANEL_EN == 2 || DP_PANEL_EN == 4
#define MAX_US2_LEN		128
uint8_t dp_buff[MAX_US2_LEN];
int w_dp=0,r_dp=0;
#endif

#if (0)
void usart2_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD ,ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);
	GPIO_PinRemapConfig(GPIO_Remap_USART2, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOD, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOD, &GPIO_InitStructure);


	//串口参数配置：9600,8,1，无奇偶校验，无硬流量控制 ，使能发送和接收
#if ROOMA_GYROONLY
	#if MAP_VERSION
		 USART_InitStructure.USART_BaudRate =115200;//老涂鸦模块是9600;//新涂鸦模块是115200; 	//
	#else
		USART_InitStructure.USART_BaudRate =9600;//老涂鸦模块是9600;//新涂鸦模块是115200;    //
	#endif
#else
	 USART_InitStructure.USART_BaudRate =115200;//115200; 	//
#endif
	 USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	 USART_InitStructure.USART_StopBits = USART_StopBits_1;
	 USART_InitStructure.USART_Parity = USART_Parity_No ;
	 USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	 USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	 USART_Init(USART2, &USART_InitStructure);
	 USART_ITConfig(USART2, USART_IT_RXNE,ENABLE);//串口接收中断

	 USART_Cmd(USART2, ENABLE);
}

void usart2_write(uint8_t *str,int len)
{
	int i;
	for(i=0;i<len;i++)
	{
		while(!(USART2->SR & USART_SR_TXE));
		USART2->DR = *str++;
	}
}

void put_usart2(uint8_t ch)
{
/*
 	RingBufferPut(&RingBuffer_WIFI_TX, ch, 0);
 */
}

int usart2_read(uint8_t *data,int len)
{
	int rlen=0;
	return rlen;
}

void USART2_IRQHandler(void)
{
/*
#if ROOMA_GYROONLY
	uint8_t cChar;

    if (USART_GetITStatus( USART2, USART_IT_TXE ) == SET)
    {
        tRingBuffer *rb = &RingBuffer_WIFI_TX;

        if (rb->Read != rb->Write)
        {
            USART2->DR = rb->Buffer[rb->Read];
            //cChar = rb->Buffer[rb->Read];

            if (rb->Read + 1 == RingBufferSize(rb))
            {
                rb->Read = 0;
            }
            else
            {
                rb->Read++;
            }
        }
        else
        {
            USART_ITConfig(USART2, USART_IT_TXE, DISABLE);
            //i=0;
            //i++;i++;
          //  asm volatile("nop");
           // asm volatile("nop");
        }
        //log_printf("%02x ",cChar);
    }
	if( USART_GetITStatus( USART2, USART_IT_RXNE ) == SET )
	{
		cChar = USART_ReceiveData( USART2 );
	}
#else
	uint8_t c;
	if (USART2->SR & USART_FLAG_RXNE)
	{
		c = USART2->DR;

		if(sys->sState == SYS_TEST_BD)
		{
	        tRingBuffer *rb = &RingBuffer_USART3_RX;

	        if (RingBufferFillLevel(rb) + 1 == RingBufferSize(rb))
	        {
	            rb->Overrun++;
	            return;
	        }

	        rb->Buffer[rb->Write] = c;

	        if (rb->Write + 1 == RingBufferSize(rb))
	        {
	            rb->Write = 0;
	        }
	        else
	        {
	            rb->Write++;
	        }
	    }
		else
		{
			//按键线程
			if(w_dp >=MAX_US2_LEN)
				w_dp = 0;
			dp_buff[w_dp++]= c;
		}
	}
#endif
*/
}
#endif

void usart_init_info(void)
{
	w_dp = 0;
	r_dp = 0;
}

void usart4_sndbyte(uint8_t ch)
{
	while(!(UART4->SR & USART_SR_TXE));
	UART4->DR = ch;
}

void usart4_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No ;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	//Write USART2 parameters
	USART_Init(UART4, &USART_InitStructure);

	//Enable UART4 Receive interrupt
	USART_ITConfig(UART4, USART_IT_RXNE, ENABLE);
	//Enable USART2
	USART_Cmd(UART4, ENABLE);
	usart4_sndbyte(0xaa);

	RingBufferInit(&RingBuffer_USART4_RX, 0L);
}

int usart4_read(uint8_t *data,int len)
{
	int rlen=0;
	while(w_dp!=r_dp)
	{
		if(r_dp >=MAX_US2_LEN)
			r_dp = 0;
		data[rlen++] = dp_buff[r_dp++];

		if(rlen >= len)
			break;
	}
	return rlen;
}

void usart4_write(uint8_t *str,int len)
{
	int i,ii;
	if(sys->sState != SYS_TEST_BD)
	log_printf("[U4]:");
	for(i=0;i<len;i++)
	{
		////while(!(UART4->SR & USART_SR_TXE));
		if(sys->sState != SYS_TEST_BD)
		log_printf("%02X ", *str);
		UART4->DR = *str++;
		for(ii=0;ii<2000;ii++);
	}
	if(sys->sState != SYS_TEST_BD)
	log_printf("\r\n");
}

extern ErrorStatus test_bd_com;

void UART4_IRQHandler(void)
{
	uint8_t c;
	if (UART4->SR & USART_FLAG_RXNE)
	{
		c = UART4->DR;

		if((sys->sState == SYS_TEST_BD) && (TM_MODE_BOARD == tm_mode))
		{
			test_bd_com = SUCCESS;
			tRingBuffer *rb = &RingBuffer_USART4_RX;

			if (RingBufferFillLevel(rb) + 1 == RingBufferSize(rb))
			{
				rb->Overrun++;
				return;
			}

			rb->Buffer[rb->Write] = c;

			if (rb->Write + 1 == RingBufferSize(rb))
			{
				rb->Write = 0;
			}
			else
			{
				rb->Write++;
			}
	    	}
		else
		{
			//按键线程
			if(w_dp >=MAX_US2_LEN)
				w_dp = 0;
			dp_buff[w_dp++]= c;
		}
	}
}



#if ROOMA_GYROONLY
static tRingBuffer RingBuffer_WIFI_TX;
void USART3_Enable_Tx_Interrupt(void)
{
    USART_ITConfig(USART3, USART_IT_TXE, ENABLE);
}
#endif

void usart3_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD ,ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);

	USART_DeInit(USART3);

	GPIO_PinRemapConfig(GPIO_FullRemap_USART3, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOD, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOD, &GPIO_InitStructure);

	//串口参数配置：9600,8,1，无奇偶校验，无硬流量控制 ，使能发送和接收
#if ROOMA_GYROONLY
	#if MAP_VERSION
		 USART_InitStructure.USART_BaudRate =115200;//老涂鸦模块是9600;//新涂鸦模块是115200; 	//
	#else
		USART_InitStructure.USART_BaudRate =115200;//老涂鸦模块是9600;//新涂鸦模块是115200;    //
	#endif
#else
	 USART_InitStructure.USART_BaudRate =115200;//115200; 	//
#endif
	 USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	 USART_InitStructure.USART_StopBits = USART_StopBits_1;
	 USART_InitStructure.USART_Parity = USART_Parity_No ;
	 USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	 USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	 USART_Init(USART3, &USART_InitStructure);
	 USART_ITConfig(USART3, USART_IT_RXNE,ENABLE);//串口接收中断
	 USART_ITConfig(USART3, USART_IT_TXE, ENABLE);
	 USART_Cmd(USART3, ENABLE);

	RingBufferInit(&RingBuffer_WIFI_TX, &USART3_Enable_Tx_Interrupt);
}

void usart3_write(uint8_t *str,int len)
{
	int i;
	for(i=0;i<len;i++)
	{
		while(!(USART3->SR & USART_SR_TXE));
		USART3->DR = *str++;
	}
}

#if ROOMA_GYROONLY
void put_usart3(uint8_t ch)
{
 	RingBufferPut(&RingBuffer_WIFI_TX, ch, 0);
}
#endif

void USART3_IRQHandler(void)
{
	uint8_t cChar;

	if (USART_GetITStatus(USART3, USART_IT_TXE ) == SET)
	{
		tRingBuffer *rb = &RingBuffer_WIFI_TX;

		if (rb->Read != rb->Write)
		{
			USART3->DR = rb->Buffer[rb->Read];

			if (rb->Read + 1 == RingBufferSize(rb))
			{
				rb->Read = 0;
			}
			else
			{
				rb->Read++;
			}
		}
		else
		{
			USART_ITConfig(USART3, USART_IT_TXE, DISABLE);
		}
		//log_printf("%02x ",cChar);
	}
	if( USART_GetITStatus(USART3, USART_IT_RXNE ) == SET )
	{
		cChar = USART_ReceiveData(USART3);
		uart_receive_input(cChar);
		//log_printf("%02X ",cChar);
	}
}

static unsigned char real_key_val = 0xFF;
static uint32_t test_led_task_tmr = 0;
void proc_uart_task(void)
{
	static uint8_t led_state = 0;
	unsigned char key_val = 0;
	#if AUTO_LAMP_EN
	static uint8_t auto_lamp_flag=0;//感光灯
    #endif
#if (1)////ROOMA_GYROONLY
    int c;
		//wifi_server_ct = 0;
	if((SYS_TEST_BD == sys->sState) && (TM_MODE_BOARD == tm_mode))
	{

	}
	else
		key_val = proc_key_task(sys->sState);
	if((real_key_val != key_val) && (((KEY_WIFI_NO + 1) == key_val) || ((KEY_DOCK_NO + 1) == key_val)))//按键流程
	{
		real_key_val = key_val;
		test_led_task_tmr = 0;
	}

	test_led_task_tmr++;
	//proc_irctrl_task(sys->sState);
	if((KEY_WIFI_NO + 1) == real_key_val)
	{
		if(0 == (test_led_task_tmr%10000))
		{
			led_state++;
			if(7 <= led_state)
			{
//				LED4_ON();
				test_led_task_tmr = 0;
				led_state = 0;
				real_key_val = 0xFF;
			}
			else if(0 == (led_state%2))
			{
//				LED4_OFF();
			}
			else if(1 == (led_state%2))
			{
	//			LED4_ON();
			}
		}
	}

  else if((KEY_DOCK_NO + 1) == real_key_val)
	{
		if(0 == (test_led_task_tmr%10000))
		{
			led_state++;
			if(7 <= led_state)
			{
//				LED2_ON();
				test_led_task_tmr = 0;
				led_state = 0;
				real_key_val = 0xFF;
			}
			else if(0 == (led_state%2))
			{
//				LED2_OFF();
			}
			else if(1 == (led_state%2))
			{
//				LED2_ON();

			}
		}
	}

    proc_wifiled_task();
	// if(sys->sState != SYS_TEST_BD)
	    dp_commrxtask();
	    dis_mode_pro();
#if AUTO_LAMP_EN
	//1a?D×???μ?
	if(sys->auto_lamp_sta==1 && auto_lamp_flag==0)
	{
		auto_lamp_flag=1;
		AUTO_LAMP_ON();
		log_printf("AUTO_LAMP_ON\r\n");
	}
	else if(sys->auto_lamp_sta==0 && auto_lamp_flag==1)
	{
		auto_lamp_flag=0;
		AUTO_LAMP_OFF();
		log_printf("AUTO_LAMP_OFF\r\n");
	}
#endif

	wifi_uart_service();
	all_data_update();
	//wifi_daemon_pro(0);

    while(1)
    {
		c= RingBufferGet(&RingBuffer_USART1_RX);
	    if (c >= 0)
	    {
		    if(sys->sState == SYS_TEST_BD)								//生产测试函数
		    {
		    	//log_printf("%02x ",c);
		    	tmod_rx_uart(0,c);
		    }
		    else
		       rx_usart(c);												//串口调试
	    }else
	    	break;
    }

    while(sys->sState == SYS_TEST_BD)
    {
		c= RingBufferGet(&RingBuffer_USART4_RX);
	    if (c >= 0)
	    {
	    	//log_printf("%02x ",c);
	    	tmod_rx_uart(1,c);//测试工装主板的通信数据
	    }else
	    	break;
    }
#else
    int c;

   // if(sys->sState != SYS_TEST_BD)
	    dp_commrxtask();												//显控
    while(1)
    {
		c= RingBufferGet(&RingBuffer_USART1_RX);
	    if (c >= 0)
	    {
		    if(sys->sState == SYS_TEST_BD)
		    {
		    	//log_printf("%02x ",c);
		    	tmod_rx_uart(0,c);//PC下发的数据
		    }
		    else
		       rx_usart(c);
	    }else
	    	break;
    }
    while(sys->sState == SYS_TEST_BD)
    {
		c= RingBufferGet(&RingBuffer_WIFI_RX);
	    if (c >= 0)
	    {
	    	//log_printf("%02x ",c);
	    	tmod_rx_uart(1,c);//测试工装主板的通信数据
	    }else
	    	break;
    }
#endif
}




#if LOG_PRINTF == 1
char string[256];
#endif
#if LOG_PRINTF != 2
void log_printf(const char *format,...)
{
	/**/
#if LOG_PRINTF
	va_list ap;
	va_start(ap,format);
	memset(string,0x00,256);
	vsprintf(string,format,ap);	USART_PutString((uint8_t *)string);
	va_end(ap);
#endif

}

void log_printf1(const char *format,...)
{

#if LOG_PRINTF
	va_list ap;
	va_start(ap,format);
	memset(string,0x00,256);
	vsprintf(string,format,ap);
	USART_PutString((uint8_t *)string);
	va_end(ap);
#endif

}

#else
/* retarget the C library printf function to the USART */
int fputc(int ch, FILE *f)
{
	USART_PutChar(ch);
	return ch;
}
#endif

