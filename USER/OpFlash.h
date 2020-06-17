#ifndef OPF_H
#define OPF_H

typedef enum
{ 
  op_FLASH_BUSY = 1,
  op_FLASH_ERROR_PG,
  op_FLASH_ERROR_WRP,
  op_FLASH_COMPLETE,
  op_FLASH_TIMEOUT
}op_FLASH_Status;

union char_l
{
	unsigned char chrs[4];
	unsigned int lons;
};

/*
typedef union chr_l
{
	unsigned char chrs[4];
	unsigned long lons;
}CHR_L;
*/
#define FLASH_BACKUP 0x08020000	//ADDRESS


extern void flash_to_flash(unsigned char pnum);
extern void flash_to_flash1(unsigned char pnum);
extern unsigned char flashup_check(void);

void write_Flasfbuff1(unsigned long addr,unsigned char *p,unsigned short n);

#endif
