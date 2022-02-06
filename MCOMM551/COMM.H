
/****************************************************************************
 COMM.H -- header file for C programs using MCOMM5 Async Lib
   Mike Dumdei,  6 Holly Lane,  Texarkana TX  75503    (c) 1989,1990
 New COMM structure implemented in 5.42 <<< ALWAYS USE CURRENT COMM.H >>>
****************************************************************************/

#ifndef MCOMM
  #define _C_ cdecl
  #define _V_ volatile
  #define _F_ far
  #if (__TURBOC__ && __HUGE__)
    #define _N_
  #else
    #define _N_ near
  #endif
  #define MCOMM
#endif

/*  ---- Return codes ----  */
#ifndef __ZTC__
  #define       R_OK           0x0000
#endif
#define         R_NOMEM        0x0001
#define         R_BAUDERR      0x0002
#define         R_PARITYERR    0x0004
#define         R_DTABITERR    0x0008
#define         R_STPBITERR    0x0010
#define         R_IRQUSED      0x0020
#define         R_VCTRUSED     0x0040
#define         R_NOPORT       0x0080
#define         R_PORTUSED     0x0100
#define         R_UARTERR      0x0200

#define         R_TXERR        0xffff

/* ---- Interrupt bit masks ---- */
#define         IRQ3         0x08
#define         IRQ4         0x10

/* ---- Vector numbers ---- */
#define         VCTR3          11
#define         VCTR4          12

/* ---- Standard Comm Ports ---- */
#define         COM1        0x3f8, 0x10, 12
#define         COM2        0x2f8, 0x08, 11

/* ---- Defines for Async_regs() ---- */
#define         RXreg       0
#define         TXreg       0
#define         IERreg      1
#define         IIDreg      2
#define         FCRreg      2
#define         LCRreg      3
#define         MCRreg      4
#define         LSRreg      5
#define         MSRreg      6
#define         LObaud      0
#define         HIbaud      1
#define         RDreg      (-1)

/* ---- Miscellaneous bits ---- */
#define         B_DTR        0x0001
#define         B_RTS        0x0002

/*  ---- Status bit masks ----  */
                        /*    S T A T 2    */
#define         B_XUSE       0x0001         /* using XON/XOFF protocol bit */
#define         B_XRXD       0x0002                   /* XOFF received bit */
#define         B_XSENT      0x0004                       /* XOFF sent bit */
#define         B_TXEMPTY    0x0008             /* nothing to transmit bit */
#define         B_CTS        0x0010                   /* clear to send bit */
#define         B_DSR        0x0020                  /* data set ready bit */
#define         B_FLOWHALT   0x0040             /* flow halt indicator bit */
#define         B_MNTR_CD    0x0080                      /* monitor CD bit */

                        /*    S T A T 1    */
#define         B_RXOVF      0x0100        /* rx buffer overflow indic bit */
#define         B_OVERRUN    0x0200              /* char overrun error bit */
#define         B_PARITY     0x0400                    /* parity error bit */
#define         B_FRAMING    0x0800                   /* framing error bit */
#define         B_BREAK      0x1000          /* line break signal detected */
#define         B_THRE       0x2000      /* used by int handler (always 0) */
#define         B_RXEMPTY    0x4000                 /* rx buffer empty bit */
#define         B_CD         0x8000             /* carrier detect bit mask */

                        /*    S T A T 3    */
#define         B_IGNERR     0x0001  /* discard chars with par, frm'g errs */
#define         B_16550      0x0002                      /* 16550 detected */
#define         B_FIFO       0x0004                 /* 16550 FIFOs enabled */
#define         B_ORGFIFO    0x0008  /* FIFOs were on when port was opened */
#define         B_XUSET      0x0020     /* using XON/XOFF on txmt side bit */
#define         B_XUSER      0x0040     /* using XON/XOFF on recv side bit */
#define         B_XONANY     0x0080          /* accept any char for an XON */


/*  ----  P O R T   S T R U C T U R E   ---- */

#pragma pack(1)      /* be sure and use byte align to reference port struc */

typedef struct
{
    int ComBase;                     /* base port address of for comm port */
    char IRQMask;                        /* mask for IRQ used by this port */
    char Vector;                     /* interrupt vector used by this port */
    char BPDSstr[10];               /* current baud,parity,data,stop parms */
    int RxSize;                                    /* size of receive bufr */
    int TxSize;                                   /* size of transmit bufr */
    int RingSeg;              /* segment of rx/tx bufrs (leave 0 for NEAR) */
    int RingOfst;                    /* offset of rx/tx bufrs, alias TxTop */
    int RxTop;                     /* ptr to begin of rx bufr, alias TxBtm */
    int RxBtm;                          /* ptr to end of rx bufr (end + 1) */
    int _V_ TxIn;                          /* ptr to where next 'put' goes */
    int _V_ TxOut;                    /* ptr to where next tx'd comes from */
    int _V_ TxFree;                             /* remaining tx bufr space */
    int _V_ RxIn;                       /* pointer to where next rx'd goes */
    int _V_ RxOut;                   /* ptr to where next 'get' comes from */
    int _V_ RxFree;                    /* remaining bytes in rx bufr space */
    int BaudDvsr;                                     /* baud rate divisor */
    int XoffTrip;          /* send XOFF when this many bytes left in rxbuf */
    int XonTrip;            /* send XON when this many bytes left in rxbuf */
    int OldVctrOfst;          /* OFST address of original interrupt vector */
    int OldVctrSeg;            /* SEG address of original interrupt vector */
    int OldDvsr;                    /* original value of Baud Divisor Regs */
    char _V_ Stat2;                               /* secondary status byte */
    char _V_ Stat1;                                 /* primary status byte */
    char _V_ TxStat;          /* determines when tx_intrpts need turned on */
    char MSRMask;               /* slcts bits in MSR montrd for flow cntrl */
    char _V_ MSRVal;             /* present value of modem status register */
    char _V_ Stat3;                  /* another flag, used for XONANY char */
    char PDSinfo;                       /* parity, # dta bits, # stop bits */
    char StripMask;                         /* mask for high bit stripping */
    char XTxRptInit;              /* initialization value for XTxRpt below */
    char _V_ XTxRpt;               /* countdown val befr repeat tx of XOFF */
    char _V_ TxImmedChar;               /* char to be tx'd 1st opportunity */
    char HdlrUsed;             /* assigned interrupt handler for this port */
    char Old8259Msk;                       /* original 8259 interrupt mask */
    char OldLCR;                        /* original Line Control Reg value */
    char OldMCR;                         /* original Modem Cntrl Reg value */
    char OldIER;                    /* original Interrupt Enable Reg value */
    int TxByteCnt;                /* number bytes to send per tx interrupt */
    char FCRVal;                        /* FIFO control register data byte */
    char IERVal;                        /* interrupt enable register value */
    char _V_ MCRVal;                       /* modem control register value */
/*
        Anything you want to add to this structure, such as a FILE *
        associated with the port, status flags, buffer pointers, etc.,
        can be added here at the bottom of the primary structure and
        accessed through the same pointer used for the port.
*/
} ASYNC;

#pragma pack()                      /* back to default structure alignment */

#ifdef __cplusplus
  extern "C" {
#endif

/* ---- F U N C T I O N   D E C L A R A T I O N S  ---- */
int _C_ async_open(ASYNC *, int, int, int, char *);     /* open async port */
void _C_ async_close(ASYNC *);                         /* close async port */
int _C_ async_tx(ASYNC *, char);                     /* transmit character */
int _C_ async_txblk(ASYNC *, char *, int);             /* transmit a block */
int _C_ async_rx(ASYNC *);                          /* receive a character */
int _C_ async_rxblk(ASYNC *, char *, int);              /* receive a block */
int _C_ async_rxblkch(ASYNC *, char *, int, char, int);/* rx blk till char */
int _C_ async_stat(ASYNC *, int);                 /* get stat1/stat2 bytes */
int _C_ async_setbpds(ASYNC *, char *);          /* change line parameters */
void _C_ async_rxflush(ASYNC *);               /* flush the receive buffer */
void _C_ async_txflush(ASYNC *);              /* flush the transmit buffer */
void _C_ async_msrflow(ASYNC *, int);      /* set MSR related tx flw cntrl */
void _C_ async_xflow(ASYNC *, int);              /* XON/OFF protocl on/off */
void _C_ async_xoffclr(ASYNC *);                 /* manually clr XOFF rx'd */
void _C_ async_xoffset(ASYNC *);                 /* manually set XOFF rx'd */
void _C_ async_dtr(ASYNC *, int);                            /* DTR on/off */
void _C_ async_rts(ASYNC *, int);                            /* RTS on/off */
void _C_ async_sndbrk(ASYNC *, int);          /* Set/clr tx modem brk sgnl */
void _C_ async_stop(ASYNC *);                 /* halts interrupt operation */
void _C_ async_restart(ASYNC *);    /* re-initializes an already open port */
int _C_ async_peek(ASYNC *, int);        /* peek indexed char in rx buffer */
int _C_ async_regs(ASYNC *, int, int);   /* direct read/write of port regs */
void _C_ async_FIFOrxlvl(ASYNC *, int);       /* set rx FIFO trigger level */
void _C_ async_FIFOtxlvl(ASYNC *, int);/* set max bytes sent per tx intrpt */
int _C_ AllocRingBuffer(ASYNC *, int, int, int);     /* alloc ring buffers */

#ifdef __cplusplus
  }
#endif

/* ---- M A C R O   I M P L E M E N T E D   F U N C T I O N S   ---- */
 /* Stat1 is also high byte of async_rx() return value for chks while rx'g */
#define async_breakrxd(x) ((x)->Stat1 & '\x10')       /* rx'd break signal */
#define async_reset(x)   ((x)->Stat1 &= '\xe0') /* rset rx error condition */
#define async_rxerr(x)   ((x)->Stat1 & '\x0f')     /* rx err cond detected */

#define async_rxcnt(x)   ((x)->RxSize - (x)->RxFree) /* get bytes in rxbuf */
#define async_rxfree(x)  ((x)->RxFree)          /* get bytes free in rxbuf */
#define async_txcnt(x)   ((x)->TxSize - (x)->TxFree) /* get bytes in txbuf */
#define async_txfree(x)  ((x)->TxFree)          /* get bytes free in txbuf */
#define async_txempty(x) ((x)->TxStat & '\x08')           /* tx bufr empty */

#define async_strip(x,y) ((x)->StripMask = (y))  /* set bit stripping mask */

#define async_xuse(x)    ((x)->Stat2 & '\x01')   /* test if using XON/XOFF */
#define async_xuset(x)   ((x)->Stat3 & '\x20') /* tst using XON/XOFF on tx */
#define async_xuser(x)   ((x)->Stat3 & '\x40') /* tst using XON/XOFF on rx */
#define async_xany(x)    ((x)->Stat3 & '\x80')       /* all chars are XONs */
#define async_xtxd(x)    ((x)->Stat2 & '\x04')                /* XOFF sent */
#define async_xrxd(x)    ((x)->TxStat & '\x02')           /* XOFF received */

#define async_msr(x)     ((x)->MSRVal)        /* get modem status register */
#define async_cts(x)     ((x)->MSRVal & '\x10')   /* Clear to Send is high */
#define async_dsr(x)     ((x)->MSRVal & '\x20')    /* Data Set Rdy is high */
#define async_carrier(x) ((x)->MSRVal & '\x80')    /* fast chk for carrier */
#define async_16550(x)   ((x)->Stat3 & '\x02')           /* 16550 detected */
#define async_rtsck(x)   ((x)->MCRVal & '\x02')       /* test state of RTS */
#define async_dtrck(x)   ((x)->MCRVal & '\x01')       /* test state of DTR */

 /* set/clr bit that controls handling of chars with parity or frame errs */
#define async_ignerr(x,y) ((y) ? ((x)->Stat3 |= 1) : ((x)->Stat3 &= '\xfe'))
     
