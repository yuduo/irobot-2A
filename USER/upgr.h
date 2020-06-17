
#ifndef _UPGR_H
#define _UPGR_H

#define  BYTE unsigned char


#define CCITT 0x1021 

/*----- Receiver -> Sender -----*/ 
#define XM_ACK 0x06 /* Packet got successfully */ 
#define XM_NAK 0x15 /* Packet got incorrectly */ 
#define XM_CAN 0x18 /* Terminate the transfer operation */ 
#define XM_EOF 0x1A /* End Of File */ 

/*----- Receiver <- Sender -----*/ 
#define XM_EOT 0x04 /* Inform receiver end of file occurred */ 
#define XM_SOH 0x01 /* Start symbol of one packet */ 

#define XM_NOT_EOF -1 /* end of file not yet reached */ 
#define XM_MAXERROR 50 /* max no. of errors */ 
#define XM_BLKSIZE 128 /* size of data block */ 


#define XM_NON			0x00
#define XM_WAIT			0x01
#define XM_RECVBLOCK	0x02
#define XM_FINISH		0x03

BYTE calcrc(BYTE *, int);
void SavetoFlash(BYTE *buffer,int len,int pNum,BYTE flage);
int upgrade(void);
void init_upgrade(void);
#endif
