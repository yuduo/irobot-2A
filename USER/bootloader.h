#ifndef _BOOTLOADER_H_
#define _BOOTLOADER_H_

#include "stm32f10x_flash.h"

#define FLASH_ADDRESS_START		0x08000000
#define FLASH_ADDRESS_BACKUP 	0x08040000

#define FLASH_PAGE 	0x800
#define FIFO_NUM		3

#define NACK_FREQ		300		//3s / 10ms
#define RECV_FREQ		10		//0.1s / 10ms
#define RECV_TIMEOUT		50		//0.5s / 10ms

#define XMODEM_SOH		0X01
#define XMODEM_STX		0X02
#define XMODEM_NACK		0X15
#define XMODEM_ACK		0X06
#define XMODEM_EOT		0X04
#define XMODEM_EOF		0X1A

#define STAGE_READY		0		//准备，校验和方式
#define STAGE_DOWNLOAD	1		//下载数据到备份flash
#define STAGE_PROGRAM	2		//拷贝备份到主程序

//包头
#define SIZE_HEAD		3
//校验和
#define SIZE_CRC			1
//两种长度格式
#define SIZE_128_B		128
#define SIZE_1K_B		1024

#define BYTES_PER_FRAME		SIZE_1K_B
#define FRAME_NUM_PER_PAGE	(FLASH_PAGE / BYTES_PER_FRAME)

//数据帧接受缓存长度
#define FRAME_SIZE		(SIZE_HEAD + SIZE_1K_B + SIZE_CRC)

typedef struct
{
	unsigned char frame_buffer[FRAME_SIZE*2];
	unsigned char flash_fifo_buff[FIFO_NUM][FLASH_PAGE];
}
UPGRADE_BUFF_T;

//传入系统TIM的CNT,设置成10ms,不需要中断
extern volatile unsigned short *timer_sync;

//升级的串口通道

extern void xmodem_default(void);						//进入升级模式时的初始化
extern void upgrade_self_task(void);						//升级主线程，内部循环堵塞
extern void befor_usart_init(FlagStatus intelchannel);
extern void dma_recevie_from_usart(void);
#endif

