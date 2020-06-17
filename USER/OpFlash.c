#include "OpFlash.h"
#include "Stm32f10x_flash.h"
#include "usart.h"

void Write_FlashPage0(unsigned long from,unsigned long to)
{
	unsigned short i;
	unsigned long fadd,tadd;
	unsigned long tempt[512];

	for(i=0,fadd=from;i<512;i++)
	{
		tempt[i]=*((unsigned long *)fadd);
		fadd+=4;

	}
	
//	uuRwrite((unsigned char *)&to,4);
//	uuRwrite("\r\n",2);
	FLASH_Unlock();//FLASH解锁
	FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);//清标志位
	FLASH_ErasePage(to);	
//	uuRwrite("a3\r\n",4);
	for(i=0,tadd=to;i<512;)
	{
		if(FLASH_ProgramWord(tadd, tempt[i])==FLASH_COMPLETE)
		{
			i++;
			tadd+=4;
		}
	//	else
	//		uuRwrite("write err\r\n",11);
	}
	FLASH_Lock();//FLASH锁
}
/*	
void Write_FlashPage1(unsigned long from,unsigned long to)
{
	unsigned short i;
	unsigned long tadd;
	CHR_L inptr;

	inptr.chrs[0]='a';
	inptr.chrs[1]='b';
	inptr.chrs[2]='c';
	inptr.chrs[3]='d';

	FLASH_Unlock();//FLASH解锁
	FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);//清标志位
	FLASH_ErasePage(to);
	for(i=0,tadd=to;i<512;)
	{
		if(FLASH_ProgramWord(tadd, inptr.lons)==FLASH_COMPLETE)
		{
			tadd+=4;
			i++;
		}
	}
	FLASH_Lock();//FLASH锁	
}

*/
unsigned char flashup_check(void)
{
	unsigned char len;
	unsigned long checksta;

//	FLASH_ReadOutProtection(DISABLE);
	checksta=*((unsigned long *)0x0801F800);
	if(checksta!=0x58585858)
	{
	//	log_printf("not upgr\r\n");
	
	//	FLASH_ReadOutProtection(ENABLE);
		return 0;
	}
	len=*((unsigned char *)0x0801F810);
//	log_printf("upgr len=%d\r\n",len);
	if(len)
	{
		//FLASH_ReadOutProtection(DISABLE);
		FLASH_Unlock();//FLASH解锁
		FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);//清标志位
		FLASH_ErasePage(0x0801F800);
		FLASH_Lock();//FLASH锁
		return len;
	}
	return 0;
}
void flash_to_flash(unsigned char pnum)
{
	unsigned char num;
	unsigned long faddr,taddr;
	
	faddr=(unsigned long)FLASH_BACKUP;
	taddr=(unsigned long)0x08000000;
	FLASH_ReadOutProtection(DISABLE);
//	IWDG_ReloadCounter();
	for(num=0;num<pnum;num++)
	{
		Write_FlashPage0(faddr,taddr);
		faddr+=0x800;
		taddr+=0x800;	
	}
	SCB->AIRCR = ((u32)0x05FA0000) | (u32)0x04;
	while(1);
}
void pp_tvv(unsigned long addr,unsigned char *p,unsigned short n)
{
	unsigned short i,a,b;
	unsigned long addrs;
	union char_l inptr;
		
	a=n/4;
	b=n%4;
	for(i=0,addrs=addr;i<a;i++,addrs+=4,p+=4)
	{
		inptr.chrs[0]=*(p);
		inptr.chrs[1]=*(p+1);
		inptr.chrs[2]=*(p+2);
		inptr.chrs[3]=*(p+3);
		FLASH_ProgramWord(addrs, inptr.lons);
	}
	inptr.lons=0xFFFFFFFF;
	if(b)
	{
		b--;
		inptr.chrs[0]=*(p);
		if(b)
		{
			b--;
			inptr.chrs[1]=*(p+1);
			if(b)
			{
				b--;
				inptr.chrs[2]=*(p+2);
				if(b)
				{
					inptr.chrs[3]=*(p+3);
				}
			}
		}
		FLASH_ProgramWord(addrs, inptr.lons);
	}
}

void write_Flasfbuff1(unsigned long addr,unsigned char *p,unsigned short n)
{

	//asm("CPSID  I");   //关中断 
	__enable_irq() ;
	FLASH_Unlock();//FLASH解锁
	FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);//清标志位
	FLASH_ErasePage(addr);
	pp_tvv(addr, p, n);
	FLASH_Lock();
	//asm("CPSIE  I");   //开中断
	__disable_irq();
	
}

/*	
NVIC_SETFAULTMASK()；   //关闭总中断
NVIC_RESETFAULTMASK()；//开放总中断

void flash_to_flash1(unsigned char pnum)
{
	unsigned char num;
	unsigned long faddr,taddr;
	
	FLASH_ReadOutProtection(DISABLE);
	faddr=(unsigned long)0x08020000;
	taddr=(unsigned long)0x08000000;
	for(num=0;num<pnum;num++)
	{
		Write_FlashPage1(faddr,taddr);
		faddr+=0x800;
		taddr+=0x800;	
	}
}
*/
