#include "stm32f10x.h"
#include"upgr.h"
#include "usart.h"
#include "SysTick.h"
#include "OpFlash.h"
#include "string.h"
#include "motor.h"
//flash_word upgrade_addr;
int upgrade_len;
int FILE_SIZE;

unsigned long addr;

int pagenum;


BYTE sBuf[2048];


//***************************************************************//
//                   \                      //
//***************************************************************//

int upgrade(void)
{
	static BYTE StockBuffer[140];
	static int XMODEM_STATUS=XM_NON;
	static int SB_len;
	static int NakRetryTime;
	static unsigned int upgr_time;
	BYTE msg[200];
	BYTE	chr;

	static int PackageNum=0;
	static BYTE canSave=0;
	int LastPkgLen;

	

	
	if(XMODEM_STATUS==XM_NON)
	{
		TIM2->CR1=0;
	//	motor_run(GO_STOP,0,0,0);

		XMODEM_STATUS=XM_WAIT;
		memset(StockBuffer,0x00,140);
		PackageNum=0;
		NakRetryTime=0;
		SB_len=0;
		psend(XM_NAK);
		mstimeout(&upgr_time,0);   //reset the timer
		canSave=0;
	}
	
	if(XMODEM_STATUS==XM_WAIT)//等待数据包第一个字节
	{
		if (read_pb(&chr))
		{
				
			if(chr==XM_SOH)//收到包头进入接收数据块模块
			{
				NakRetryTime=0;
				XMODEM_STATUS=XM_RECVBLOCK;
				if (PackageNum>0){
					if (PackageNum==1 && ((*(StockBuffer+3)!=0x23 || *(StockBuffer+4)!=0x00))){
							//退出xmodem
							psend(XM_CAN);
							NakRetryTime=XM_MAXERROR;
					} else if(canSave)
					{
						SavetoFlash(StockBuffer+3,XM_BLKSIZE,PackageNum,0);
						canSave=0;
					}
					
				}				
				
				StockBuffer[SB_len]=chr;
				SB_len++;
			}
			else if(chr==XM_EOT && canSave)//Finish send
			{
				LastPkgLen=XM_BLKSIZE;
				while(StockBuffer[LastPkgLen+2]==XM_EOF)
					LastPkgLen--;
				
				psend(XM_ACK);
				XMODEM_STATUS=XM_FINISH;
				SavetoFlash(StockBuffer+3,LastPkgLen,PackageNum,1);
				canSave=0;
			}
			else if(chr==XM_CAN)//Server Cancel
			{
				psend(XM_ACK);
				NakRetryTime=XM_MAXERROR;
			}
			mstimeout(&upgr_time,0);   //reset the timer
		}else if(mstimeout(&upgr_time,200)){
			psend(XM_NAK);
			NakRetryTime++;
		}
	}
	
	if(XMODEM_STATUS==XM_RECVBLOCK)//接收完整数据包
	{
		if (read_pb(&chr)){
			StockBuffer[SB_len]=chr;
			SB_len++;
			if((SB_len>=XM_BLKSIZE+4))
			{//收到数据包
				if((calcrc(StockBuffer+3,XM_BLKSIZE)==StockBuffer[XM_BLKSIZE+3])&&
					((BYTE)(StockBuffer[1])+(BYTE)(StockBuffer[2])==0xff)&&
					((BYTE)(StockBuffer[1])==( (PackageNum+1)% 256)))
				{//数据包校验通过
					psend(XM_ACK);
					memset(msg,0x00,200);
					
					PackageNum++;
					SB_len=0;
					XMODEM_STATUS=XM_WAIT;
					canSave=1; //能存FLASH
				}
				else
				{
					NakRetryTime++;
					psend(XM_NAK);
					SB_len=0;
					XMODEM_STATUS=XM_WAIT;
					canSave=0;
				}
			}
			mstimeout(&upgr_time,0);   //reset the timer
		}else if(mstimeout(&upgr_time,1000)){
			NakRetryTime++;
			SB_len=0;
			psend(XM_NAK);
			XMODEM_STATUS=XM_WAIT;
			canSave=0;
		}
	}
	
	if(NakRetryTime>=XM_MAXERROR){//接收失败退到命令模式
		XMODEM_STATUS=XM_NON;
		memset(StockBuffer,0x00,140);
		NakRetryTime=0;
		SB_len=0;
		log_printf("[UPGR]upgr error\r\n") ;
		return 0;
	}
	
	if(XMODEM_STATUS==XM_FINISH)//接收成功退到命令模式
	{
		XMODEM_STATUS=XM_NON;
		if (PackageNum==0){
			log_printf("[UPGR]Error file,please check and try again",46);
			
		}else{

			log_printf("Receive file success.update myselft...\r\n");
			//这里更新，重启
			flash_to_flash(pagenum);
			SCB->AIRCR = ((u32)0x05FA0000) | (u32)0x04;
			while(1);			
				
		}
	}
	return 0;
}

void SavetoFlash(BYTE *buffer,int len,int pNum,BYTE flage)
{
	static char cPack=0;
	if(cPack>=2048)
	   cPack=0 ;
	if(len >128)
		len=128;
	memcpy((char *)(sBuf+cPack),(char *)buffer,len);
	cPack+=len;
	if((flage==1) || (cPack>=2048))
	{
		 //save to flash
		write_Flasfbuff1(addr, (u8 *)sBuf, cPack);
		addr+=cPack;
		pagenum++;
	}
	

}
BYTE calcrc(BYTE * ptr,int count)
{

	int crc ;

	crc = 0;
	while(--count >= 0) {
		crc = crc + (int)*ptr++ ;
	}
	return (crc & 0xFF);
}


//**************************************************************************//
//                       程序在线升级模式                                   //
//**************************************************************************//
//升级模式目前只支持小于128K的文件的升级

void init_upgrade(void){
//	upgrade_addr=0;
	addr=FLASH_BACKUP;
	pagenum=0;
	upgrade_len=0;
	FILE_SIZE=0;
}


