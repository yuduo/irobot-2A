
#ifndef USART_H
#define USART_H

#define LOG_PRINTF    1 //0关闭打印 1 log_printf 2 调用系统printf,不使用ram缓存

//#define SYS_DEBUG 1

typedef struct __r1371t_dat{
	uint8_t head2;//0xAA后面的字节(永远都是0x00)
	uint8_t index;//计数器数据
	int16_t angle;//当前角度
	int16_t rate;//当前角速度
	int16_t accx;//加速度x
	int16_t accy;//加速度y
	int16_t accz;//加速度z
	uint8_t resv;//未使用
	uint8_t chsum;//校验和
}R1371T_FMT;

extern R1371T_FMT r1371t_dat;




typedef char OUT[64];



//#define UART2RXBUFFMAX_SIZE  512  //缓冲区长度
#define UART1RXBUFFMAX_SIZE  64  //缓冲区长度

#define UART3RXBUFFMAX_SIZE  64 
#define UART4RXBUFFMAX_SIZE	 64
#define UART5RXBUFFMAX_SIZE	 64

//extern uint8_t rxBuf[UART1RXBUFFMAX_SIZE];

extern int u1_len;

extern uint8_t g_buf[32];
extern int g_len;
		
void usart1_init(void);


#if LOG_PRINTF == 2
#define log_printf printf
#elif LOG_PRINTF == 1
void log_printf(const char *format,...);
#endif

extern char read_pb(uint8_t *chr);
extern void psend(uint8_t chr);
extern int read_uart1(uint8_t *str);
extern void proc_uart_task(void);
extern void usart_write(uint8_t *buff,int len);

extern void usart2_init(void) ;
extern void usart2_write(uint8_t *str,int len);
extern int usart2_read(uint8_t *data,int len);

extern void usart4_init(void);
extern int usart4_read(uint8_t *data,int len);
extern void usart4_write(uint8_t *str,int len);

extern void usart_init_info(void);

extern void usart3_init(void);
extern void usart3_write(uint8_t *str,int len);
extern void put_usart3(uint8_t ch);
#endif

