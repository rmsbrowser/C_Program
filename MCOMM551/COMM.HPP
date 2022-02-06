
/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//  COMM.HPP -- header file for C++ programs using MCOMM async routines.   //
//    Zortech C++ 2.0, Turbo C++ 1.00 version.                             //
//                                                                         //
//    Mike Dumdei,  6 Holly Lane,  Texarkana TX  75503    (c) 1989,1990    //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#if !defined(COMM_HPP)
#define COMM_HPP

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

//////////////////////////////////////////////
//  Return codes                            //
//////////////////////////////////////////////
#ifndef __ZTC__
  #define   R_OK           0x0000
#endif
#define     R_NOMEM        0x0001
#define     R_BAUDERR      0x0002
#define     R_PARITYERR    0x0004
#define     R_DTABITERR    0x0008
#define     R_STPBITERR    0x0010
#define     R_IRQUSED      0x0020
#define     R_VCTRUSED     0x0040
#define     R_NOPORT       0x0080
#define     R_PORTUSED     0x0100
#define     R_UARTERR      0x0200

#define     R_TXERR        (-1)

//////////////////////////////////////////////
//  Interrupt bit masks                     //
//////////////////////////////////////////////
#define     IRQ3           0x08
#define     IRQ4           0x10

//////////////////////////////////////////////
//  Vector numbers                          //
//////////////////////////////////////////////
#define     VCTR3          0x0b
#define     VCTR4          0x0c

//////////////////////////////////////////////
//  Standard Comm Ports                     //
//   Note - due to the ability to overload  //
//   the open call, COM1 and COM2 are def-  //
//   ined differently in the C++ version.   //
//////////////////////////////////////////////
#define     COM1        0
#define     COM2        1

//////////////////////////////////////////////
//  Defines for async_regs function         //
//////////////////////////////////////////////
#define     RXreg       0
#define     TXreg       0
#define     IERreg      1
#define     IIDreg      2
#define     FCRreg      2
#define     LCRreg      3
#define     MCRreg      4
#define     LSRreg      5
#define     MSRreg      6
#define     LObaud      0
#define     HIbaud      1

//////////////////////////////////////////////
//  Miscellaneous bit masks                 //
//////////////////////////////////////////////
#define     B_DTR        0x0001
#define     B_RTS        0x0002

/////////////////////////////////////////////////////////////////////////
//  Status bit masks                                                   //
/////////////////////////////////////////////////////////////////////////
  //  S T A T 2  //
#define     B_XUSE       0x0001   // using XON/XOFF protocol bit
#define     B_XRXD       0x0002   // XOFF received bit
#define     B_XSENT      0x0004   // XOFF sent bit
#define     B_TXEMPTY    0x0008   // nothing to transmit bit
#define     B_CTS        0x0010   // clear to send bit
#define     B_DSR        0x0020   // data set ready bit
#define     B_FLOWHALT   0x0040   // flow halt indicator bit
#define     B_MNTR_CD    0x0080   // monitor CD bit

  //  S T A T 1  //
#define     B_RXOVF      0x0100   // rx buffer overflow indic bit
#define     B_OVERRUN    0x0200   // char overrun error bit
#define     B_PARITY     0x0400   // parity error bit
#define     B_FRAMING    0x0800   // framing error bit
#define     B_BREAK      0x1000   // line break signal detected
#define     B_THRE       0x2000   // used in interrupt hdlr (always 0)
#define     B_RXEMPTY    0x4000   // rx buffer empty bit
#define     B_CD         0x8000   // carrier detect bit mask

  //  S T A T 3  //
#define     B_IGNERR     0x0001   // discard chars with par, frm'g errs
#define     B_16550      0x0002   // 16550 detected
#define     B_FIFO       0x0004   // 16550 FIFOs enabled
#define     B_ORGFIFO    0x0008   // FIFOs were on when port was opened
#define     B_FARBUFFER  0x0010   // using FAR ring buffers
#define     B_XUSET      0x0020   // using XON/XOFF on txmt side bit
#define     B_XUSER      0x0040   // using XON/XOFF on recv side bit
#define     B_XONANY     0x0080   // accept any char for an XON


/////////////////////////////////////////////////////////////////////////
//                                                                     //
//  A s y n c P o r t   C l a s s                                      //
//                                                                     //
//                    * * *  W A R N I N G  * * *                      //
//     This class must have the port variables byte aligned.  As       //
//     written, ZTC 2.0 will byte align this structure without the     //
//     need to direct it to pack structures.  If you are using         //
//     a different implementation of C++ be sure this data is          //
//     byte aligned by your compiler.                                  //
/////////////////////////////////////////////////////////////////////////

class AsyncPort
{
      // port variables
  private:
    int ComBase;          // base port address of for comm port
    char IRQMask;         // mask for IRQ used by this port
    char Vector;          // interrupt vector used by this port
    char BPDSstr[10];     // current baud,parity,data,stop parms
    int RxSize;           // size of receive bufr
    int TxSize;           // size of transmit bufr
    int RingSeg;          // segment of rx/tx bufrs (leave 0 for NEAR)
    int RingOfst;         // offset of rx/tx bufrs, alias TxTop
    int RxTop;            // ptr to begin of rx bufr, alias TxBtm
    int RxBtm;            // ptr to end of rx bufr (end + 1)
    int _V_ TxIn;         // ptr to where next 'put' goes
    int _V_ TxOut;        // ptr to where next tx'd comes from
    int _V_ TxFree;       // remaining tx bufr space
    int _V_ RxIn;         // pointer to where next rx'd goes
    int _V_ RxOut;        // ptr to where next 'get' comes from
    int _V_ RxFree;       // remaining bytes in rx bufr space
    int BaudDvsr;         // baud rate divisor
    int XoffTrip;         // send XOFF when this many bytes left in rxbuf
    int XonTrip;          // send XON when this many bytes left in rxbuf
    int OldVctrOfst;      // OFST address of original interrupt vector
    int OldVctrSeg;       // SEG address of original interrupt vector
    int OldDvsr;          // original value of Baud Divisor Regs
    char _V_ Stat2;       // secondary status byte
    char _V_ Stat1;       // primary status byte
    char _V_ TxStat;      // determines when tx_intrpts need turned on
    char MSRMask;         // slcts bits in MSR montrd for flow cntrl
    char _V_ MSRVal;      // present value of modem status register
    char _V_ Stat3;       // another flag, used for XONANY char
    char PDSinfo;         // parity, # dta bits, # stop bits
    char StripMask;       // mask for high bit stripping
    char XTxRptInit;      // initialization value for XTxRpt below
    char _V_ XTxRpt;      // countdown val befr repeat tx of XOFF
    char _V_ TxImmedChar; // char to be tx'd 1st opportunity
    char HdlrUsed;        // assigned interrupt handler for this port
    char Old8259Msk;      // original 8259 interrupt mask
    char OldLCR;          // original Line Control Reg value
    char OldMCR;          // original Modem Cntrl Reg value
    char OldIER;          // original Interrupt Enable Reg value
    int TxByteCnt;        // number bytes to send per tx interrupt
    char FCRVal;          // FIFO control register data byte
    char IERVal;          // interrupt enable register value
    char _V_ MCRVal;      // modem control register value

      // declaration of default ring buffer variables
    static int DefRxBufSz;      // default receive ring buffer size
    static int DefTxBufSz;      // default transmit buffer size
    static int DefMemType;      // default to NEAR or FAR memory flag

  public:
    AsyncPort();                    // constructors
    AsyncPort(int RxBufSz, int TxBufSz, int UseFarMem = 0);

    ~AsyncPort();                   // destructor

                                    // allocate ring buffers function
    int AllocBuffers(int RxBufSz, int TxBufSz, int UseFarMem = 0);
    int FreeBuffers();              // free ring buffers function
                                    // reset default buffers function
    static void SetDefaultBuffers(int RxDefSz, int TxDefSz, int DefMem = 0);

                                    // open with full port definition
    int Open(int ComBase, int IRQNbr, int InterruptVctr, char *Params = "");
    int Open(int ComN, char *Params = ""); // open COM1 or COM2

    int Close();                    // close & leave DTR/RTS in pre-open state
    int Close(int DTR, int RTS);    // close with option to force DTR/RTS

    void Release();                 // disable interrupt operation
    void Restart();                 // resume interrupt mode of operation

    void PortAddress(int BaseAdrs); // set port base address
    int PortAddress();              // get port base address

    int Tx(char Ch);                // transmit character
    int Tx(char *String);           // transmit string
    int Tx(char *Block, int NbrBytes); // transmit block

    int Rx();                       // receive character
    int Rx(char *Block, int MaxNbrBytes); // receive block until maxbytes
                                    // rx blk until delim or maxbytes
    int Rx(char *Block, int MaxNbrBytes, char Delim, int IncludeDelim = 1);

    int Peek(int Offset = 0);       // peek receive buffer (index opt)

    int Status(int Mask = 0xffff);  // get port status (Stat1/Stat2)
    void Reset();                   // reset parity,framing,char/bufr overrun
    char Error(int Mask = 0x0f);    // test for parity,frame,char/bufr overrun

    int Params(char *Params);       // set port parameters
    char *Params();                 // get port parameters

    void RxFlush();                 // flush receive buffer
    void TxFlush();                 // flush transmit buffer

    void HardFlow(int Mask);        // set/reset hardware flow options

    void XFlow(int Mask);           // set/reset XON/XOFF flow control
    char XFlow();                   // read XON/XOFF flow settings (bitmapped)
                                    // set XON/XOFF trip levels
    int XTrip(int XoffLevel, int XonLevel, int RepeatLevel);
    void XoffReset();               // forced reset of XOFF received condition
    void XoffSet();                 // simulate XOFF received
    char XoffRxd();                 // test for XOFF received
    char XoffTxd();                 // test for XOFF transmitted

    void Dtr(int Enable);           // set/reset DTR line
    void Rts(int Enable);           // set/reset RTS line
    char Msr();                     // get value of modem status register
    char Cts();                     // get status of CTS line
    char Dsr();                     // get status of DSR line
    char Carrier();                 // get status of carrier detect line

    void Break(int Enable);         // set or reset breaking condition
    char Break();                   // check if break signal received

    char Regs(int UARTreg, int Value = -1); // raw R/W of UART regs

    int RxLevel();                  // get number bytes in receive buffer
    int TxAvailable();              // get bytes available in transmit buffer
    char TxEmpty();                 // test for transmit buffer empty

    void RxMask(char Mask);         // set strip mask for incoming chars
    void FlushBadChars(int Flag);   // set handling for chars with errs
    char FlushBadChars();           // get setting chars with errs
};

/////////////////////////////////////////////////////////////////////////
//  E X T E R N A L   A S M   F U N C T I O N S                        //
/////////////////////////////////////////////////////////////////////////
  extern "C" {
int async_open(AsyncPort *, int, int, int, char *); // open async port
int async_close(AsyncPort *);              // close async port
int async_tx(AsyncPort *, char);           // transmit character
int async_txblk(AsyncPort *, char *, int); // transmit a block
int async_rx(AsyncPort *);                 // receive a character
int async_rxblk(AsyncPort *, char *, int); // receive a block
int async_rxblkch(AsyncPort *, char *, int, char, int); // rx blk till char
int async_stat(AsyncPort *, int);          // get stat1/stat2 bytes
int async_setbpds(AsyncPort *, char *);    // change line parameters
void async_rxflush(AsyncPort *);           // flush the receive buffer
void async_txflush(AsyncPort *);           // flush the transmit buffer
void async_msrflow(AsyncPort *, int);      // set MSR related flw cntrl
void async_xflow(AsyncPort *, int);        // XON/OFF protocl on/off
void async_xoffclr(AsyncPort *);           // manually clr XOFF rx'd
void async_xoffset(AsyncPort *);           // manually set XOFF rx'd
void async_dtr(AsyncPort *, int);          // DTR on/off
void async_rts(AsyncPort *, int);          // RTS on/off
void async_sndbrk(AsyncPort *, int);       // Set/clr tx modem brk sgnl
void async_stop(AsyncPort *);              // halts interrupt operation
void async_restart(AsyncPort *);           // re-init an already open port
int async_peek(AsyncPort *, int);          // peek indexed char in rx buffer
int async_regs(AsyncPort *, int, int);     // direct read/write of port regs
void async_FIFOrxlvl(AsyncPort *, int);    // set rx FIFO trigger level
void async_FIFOtxlvl(AsyncPort *, int);    // set max sent per tx intrpt
  }


/////////////////////////////////////////////////////////////////////////
//  D E F I N I T I O N    O F   I N L I N E   F U N C T I O N S       //
/////////////////////////////////////////////////////////////////////////

inline void AsyncPort::SetDefaultBuffers(int DefRxSz,int DefTxSz,int DefMem)
 { DefRxBufSz = DefRxSz, DefTxBufSz = DefTxSz, DefMemType = DefMem; }
inline void AsyncPort::PortAddress(int BaseAdrs)
 { ComBase = BaseAdrs; }
inline int AsyncPort::PortAddress()
 { return ComBase; }
inline void AsyncPort::Release()
 { async_stop(this); }
inline void AsyncPort::Restart()
 { async_restart(this); }
inline int AsyncPort::Tx(char Ch)
 { return (async_tx(this, Ch)); }
inline int AsyncPort::Tx(char *Block, int NbrBytes)
 { return (async_txblk(this, Block, NbrBytes)); }
inline int AsyncPort::Rx()
 { return (async_rx(this)); }
inline int AsyncPort::Rx(char *Block, int MaxNbrBytes)
 { return (async_rxblk(this, Block, MaxNbrBytes)); }
inline int AsyncPort::Rx(char *Block, int MaxNbrBytes, char Delim,
 int IncludeDelim)
 { return (async_rxblkch(this, Block, MaxNbrBytes, Delim, IncludeDelim)); }
inline int AsyncPort::Peek(int Offset)
 { return (async_peek(this, Offset)); }
inline int AsyncPort::Status(int Mask)
 { return (async_stat(this, Mask)); }
inline void AsyncPort::Reset()
 { Stat1 &= '\xe0'; }
inline char AsyncPort::Error(int Mask)
 { return (Stat1 & Mask); }
inline int AsyncPort::Params(char *Params)
 { return (async_setbpds(this, Params)); }
inline char * AsyncPort::Params()
 { return BPDSstr; }
inline void AsyncPort::RxFlush()
 { async_rxflush(this); }
inline void AsyncPort::TxFlush()
 { async_txflush(this); }
inline void AsyncPort::HardFlow(int Mask)
 { async_msrflow(this, Mask); }
inline void AsyncPort::XFlow(int Mask)
 { async_xflow(this, Mask); }
inline char AsyncPort::XFlow()
 { return (Stat3 & '\xe0'); }
inline void AsyncPort::XoffReset()
 { async_xoffclr(this); }
inline void AsyncPort::XoffSet()
 { async_xoffset(this); }
inline char AsyncPort::XoffRxd()
 { return (TxStat & '\x02'); }
inline char AsyncPort::XoffTxd()
 { return (Stat2 & '\x04'); }
inline void AsyncPort::Dtr(int Enable)
 { async_dtr(this, Enable); }
inline void AsyncPort::Rts(int Enable)
 { async_dtr(this, Enable); }
inline char AsyncPort::Msr()
 { return MSRVal; }
inline char AsyncPort::Cts()
 { return (MSRVal & '\x10'); }
inline char AsyncPort::Dsr()
 { return (MSRVal & '\x20'); }
inline char AsyncPort::Carrier()
 { return (MSRVal & '\x80'); }
inline void AsyncPort::Break(int Enable)
 { async_sndbrk(this, Enable); }
inline char AsyncPort::Break()
 { return (Stat1 & '\x10'); }
inline char AsyncPort::Regs(int UARTreg, int Value)
 { return async_regs(this, UARTreg, Value); }
inline int AsyncPort::RxLevel()
 { return (RxSize - RxFree); }
inline int AsyncPort::TxAvailable()
 { return (TxFree); }
inline char AsyncPort::TxEmpty()
 { return (TxStat & '\x08'); }
inline void AsyncPort::RxMask(char Mask)
 { StripMask = Mask; }
inline void AsyncPort::FlushBadChars(int Flag)
 { (Flag) ? (Stat3 |= 1) : (Stat3 &= '\xfe'); }
inline char AsyncPort::FlushBadChars()
 { return (Stat3 & 1); }
     
#endif          // COMM_HPP

