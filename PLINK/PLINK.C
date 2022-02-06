#include <alloc.h>
#include "comm.h"
#include <conio.h>
#include <io.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "plinkcrc.c"
#include "plinkrle.c"

#define SOH 0x01
#define EOT 0x04
#define ACK 0x06
#define DLE 0x10
#define XON 0x11
#define XOFF 0x13
#define NAK 0x15
#define SYN 0x16
#define CAN 0x18
#define ESC 0x1b

/*
    unsigned long crc32(register char *buf,int size);

    unsigned short rle(unsigned short len,unsigned char *in_buffer,
            unsigned char *out_buffer);

    unsigned short unrle(unsigned short len,unsigned char *in_buffer,
            unsigned char *out_buffer);

    SOH dle(altcharset(rle(data[1024]+crc[4])))
    STX dle(altcharset(rle((pos[4])+data[1024]+crc[4])))
    NAK dle(altcharset(rle(pos[4])))

*/

short alloc_ring_buffer(ASYNC *port,short rxsize,short txsize);
short computs(unsigned char *buf,unsigned short bytes);
short sendblock(FILE *file,unsigned long pos);
short sendnak(unsigned long pos);
short send(char *fname);
char *comgets(char *buf,short bytes);
short receive(char *fname);
short escape(void);
short no_carrier(void);
void usage(void);

ASYNC *port;
short possent=0;

unsigned short main(short argc,char *argv[])
{
	port=(ASYNC *)malloc(sizeof(port));
	cputs("\r\nPlink v1.00d  Copyright (c) 1992  Christopher Peterson\r\n\n");
/*
	if (argc!=3) usage();
*/
	cputs("\r\nargv[1] = "); gets(argv[1]);
	cputs("\r\nargv[2] = "); gets(argv[2]);

	switch(*argv[1]) {
      case('s'):
      case('S'):
			send(strupr(argv[2]));
         break;
      case('r'):
      case('R'):
			receive(strupr(argv[2]));
         break;
      default:usage();
	}
	async_close(port);
    return 0;
}

short computs(unsigned char *inbuf,unsigned short bytes)
{
    unsigned short i;
    
	for(i=0; i<=bytes; i++,*inbuf++) {
        if (*inbuf<0x20) {
            if (async_tx(port,DLE)<0) return -1;
            if (async_tx(port,*inbuf^0x40)<0) return -1;
        }
        else if (async_tx(port,*inbuf)<0) return -1;
    }
    return 0;
}

short sendblock(FILE *file,unsigned long pos)
{
	char *buf=(char *)malloc(1024);
    unsigned short blocklen;
	unsigned long crc;

	if (pos!=possent+1024)
        if (fsetpos(file,(fpos_t *)pos)) return 1;
    blocklen=(unsigned short)(fread(buf,(size_t)1,(size_t)1024,file));
	possent=pos+blocklen;
    if (async_tx(port,SOH)<0) return -1;
    if (computs(buf,blocklen)==1) return -1;
	crc=crc32(buf,blocklen);
    if (computs((char *)crc,4)==1) return -1;
    return 0;
}

short sendnak(unsigned long pos)
{
    if (async_tx(port,NAK)<0) return -1;
    if (computs((char *)pos,4)==-1) return -1;
    return 0;
}

short send(char *fname)
{
	FILE *fp=(FILE *)malloc(sizeof(fp));
	unsigned long flen,pos,possent;
	short cancel=0,ch;

	alloc_ring_buffer(port,4096,2048);
	if ((async_open(port,COM1,""))!=R_OK) {
		cputs("\r\nERROR: Cannot open COM1\r\n");
		return(-1);
	}
	async_ignerr(port,1);

	if ((fp=fopen(fname,"rb"))==NULL) {
		cprintf("\r\nERROR: Cannot open '%s'\r\n",fname);
		return(-1);
   }
   flen=filelength(fileno(fp));

   /* get file info */

	cputs("\r\nSynchronizing\r\n");
	async_tx(port,SYN);
	async_tx(port,SYN);
	while ((ch=async_rx(port))!=SYN) {
		if (ch!=NAK) async_tx(port,SYN);
		if (ch==CAN) {
			cancel++;
			if (cancel>3) {
				cputs("\r\nERROR: Transfer aborted remotely\r\n");
				return(-1);
			}
		}
		if (escape()) return(-1);
		if (no_carrier()) return(-1);
	}

	/* send block zero */
   /* send file blocks */
   /* NAK? yes, then backtrack */
   /* no, then continue */
   /* send EOF */
   /* end transfer */

	if (fclose(fp)==EOF) {
		cprintf("\r\nERROR: Error closing %s",fname);
		return(-1);
	}
	return(0);
}

char *comgets(char *buf,short bytes)
{
   short i;

	buf=(char *)malloc(sizeof(buf));
	for(i=0; i<=bytes; i++) {
		*buf=async_rx(port);
      if (*buf==DLE)
			*buf=async_rx(port)|0x40;
   }
   return(buf);
}

short receive(char *fname)
{
	FILE *fp=(FILE *)malloc(sizeof(fp));
	unsigned long pos,possent;

	fname=(char *)malloc(sizeof(fname));
   alloc_ring_buffer(port,4096,2048);
	if ((async_open(port,COM1,""))!=R_OK) {
		cputs("\r\nERROR: Cannot open COM1\r\n");
		exit(1);
	}
	async_ignerr(port,1);

	cputs("\r\nSynchronizing\r\n");
	while (async_rx(port)!=SYN) {
		async_tx(port,SYN);

			cputs("\r\nERROR: Cannot synchronize\r\n");
         return(-1);

   }
	async_tx(port,SYN);
	while (async_rx(port)!=SOH) {
		async_tx(port,SOH);

			cputs("\r\nERROR: Cannot synchronize\r\n");
         return(-1);

   }
	async_tx(port,SOH);

	if ((fp=fopen(fname,"rb"))==NULL) {
		cprintf("\r\nERROR: Cannot open '%s'\r\n",fname);
      return(-1);
   }

   /* get block zero */
   /* get file info */
	/* receive file blocks */
   /* blocks okay? no, then NAK */
   /* yes, then continue */
   /* EOF? */
   /* end transfer */
	return(0);
}

short escape(void)
{
	if (kbhit())
		if (getch()==ESC) {
			cputs("\r\nERROR: Transfer aborted locally\r\n");
			return(1);
		}
	return(0);
}

short no_carrier(void)
{
	if (!async_carrier(port)) {
		cputs("\r\nERROR: No carrier\r\n");
		return(1);
	}
	return(0);
}

void usage(void)
{
	cputs("Usage: dm s filename\r\n");
	cputs("       dm r filename\r\n");
   exit(1);
}

short alloc_ring_buffer(ASYNC *port,short rxsize,short txsize)
{
	short memsize=rxsize+txsize;
	unsigned long memptr=(unsigned long)malloc(memsize);

	port->RxSize=rxsize;
	port->TxSize=txsize;
	port->RingSeg=(short)(memptr>>16);
	port->RingOfst=(short)(memptr);
	return(memptr==0L?0:1);
}
