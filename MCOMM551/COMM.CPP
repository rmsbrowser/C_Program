
/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//  COMM.CPP -- source for common functions for MCOMM AsyncPort class for  //
//    Zortech C++ 2.0, Turbo C++ 1.0.                                      //
//    Compiler specific code:  MK_FP, SPTR (true if NEAR data), farmalloc  //
//                                                                         //
//    Mike Dumdei,  6 Holly Lane,  Texarkana TX  75503    (c) 1989,1990    //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#include "comm.hpp"
#include <string.h>
#include <stdlib.h>
#include <dos.h>

#if defined(__TURBOC__)
  #include <alloc.h>
  #if (__TINY__ || __SMALL__ || __MEDIUM__)
    #define SPTR 1
  #endif
#endif

/////////////////////////////////////////
//  Default ring buffer size variables //
/////////////////////////////////////////
int AsyncPort::DefRxBufSz = 1024;   // 1K receive buffer
int AsyncPort::DefTxBufSz = 1024;   // 1K transmit buffer
int AsyncPort::DefMemType = 0;      // default to NEAR, NZ to default to FAR

/////////////////////////////////////////
//  Constructor - Simple               //
//    Presets port variables to 0      //
//://////////////////////////////////////
AsyncPort::AsyncPort()
{
    memset((void *)this, '\0', sizeof(AsyncPort));
}

/////////////////////////////////////////
//  Constructor - Allocs ring buffers  //
//    Presets port variables to 0      //
//    Allocates ring buffers           //
//://////////////////////////////////////
AsyncPort::AsyncPort(int RxBufSz, int TxBufSz, int UseFarMem)
{
    memset((void *)this, '\0', sizeof(AsyncPort));
    AllocBuffers(RxBufSz, TxBufSz, UseFarMem);
}

/////////////////////////////////////////
//  Destructor                         //
//    If necessary, calls Close and    //
//    frees ring buffer memory.        //
//://////////////////////////////////////
AsyncPort::~AsyncPort()
{
    if (HdlrUsed != '\0')           // true if port not closed yet
        Close();
}

/////////////////////////////////////////
//  Open Port - Detailed parameters    //
//    Initializes port for interrupt   //
//    driven operation.  Allocates     //
//    default ring buffers if memory   //
//    has not already been allocated.  //
//    If Open fails default buffers    //
//    are automatically deallocated.   //
//    User defined buffers are not     //
//    automatically deallocated.       //
//://////////////////////////////////////
int AsyncPort::Open(int ComBase, int IRQNbr, int InterruptVctr, char *Params)
{
    int rval;
    int defbuffers = 0;

    if (!RingSeg && !RingOfst)          // if ring buffers not allocated,
    {                                   // use defaults
        AllocBuffers(DefRxBufSz, DefTxBufSz, DefMemType);
        defbuffers = 1;
    }
     // open the port
    rval = async_open(this, ComBase, IRQNbr, InterruptVctr, Params);
    if (rval != R_OK && defbuffers == 1)     // if port open failed & default
        FreeBuffers();                       // buffers, free default buffers
    return (rval);                      // return port open result
}

/////////////////////////////////////////
//  Open Port -  COM1/COM2 abbrev      //
//    Short method to open either COM1 //
//    or COM2.  Calls detailed Open    //
//    after selecting port address,    //
//    irq, and interrupt vector.       //
//://////////////////////////////////////
int AsyncPort::Open(int ComN, char *Params)
{
    int base, irq, vector;

    if (ComN == COM1)
        base = 0x3f8, irq = IRQ4, vector = VCTR4;
    else if (ComN == COM2)
        base = 0x2f8, irq = IRQ3, vector = VCTR3;
    else
        return R_NOPORT;
    return (Open(base, irq, vector, Params));
}

/////////////////////////////////////////
//  Close port - No parameters         //
//    Resets port to the state it was  //
//    in prior to being opened.  Frees //
//    ring buffer memory.              //
//://////////////////////////////////////
int AsyncPort::Close()
{
    int rval = async_close(this);   // call low level function to close port
    if (rval == R_OK)               // free ring buffer memory if closed OK
        FreeBuffers();
    return rval;
}

/////////////////////////////////////////
//  Close port - Force DTR/RTS         //
//    Manipulates class variable that  //
//    'remembers' initial port DTR or  //
//    RTS setting then calls normal    //
//    Close to close the port and free //
//    ring buffer memory.              //
//://////////////////////////////////////
int AsyncPort::Close(int DTR, int RTS)
{
    if (DTR != 0)                   // if ZR, close with DTR in pre-open state
    {
        if (DTR > 0)                // if > ZR, force DTR to remain high
            OldMCR |= B_DTR;
        else
            OldMCR &= ~B_DTR;       // if < ZR, force DTR to go low
    }
    if (RTS != 0)                   // if ZR, close with RTS in pre-open state
    {
        if (RTS > 0)                // if > ZR, force RTS to remain high
            OldMCR |= B_RTS;
        else
            OldMCR &= ~B_RTS;       // if < ZR, force RTS to go low
    }
    return Close();
}

/////////////////////////////////////////
//  Set XON/XOFF trip levels           //
//    Returns R_OK (0) if success, or  //
//    NZ if invalid parameters.  The   //
//    Xon/Xoff 'Levels' are for bytes  //
//    free in the buffer, therefore,   //
//    the XoffLevel must be less than  //
//    the XonLevel.  The Repeat level  //
//    is the number of bytes that will //
//    be accepted after sending an     //
//    Xoff before sending a 2nd Xoff.  //
//://////////////////////////////////////
int AsyncPort::XTrip(int XoffLevel, int XonLevel, int RepeatLevel)
{
    if (XoffLevel >= XonLevel)
        return (-1);
    XoffTrip = XoffLevel;
    XonTrip = XonLevel;
    XTxRptInit = RepeatLevel;
    return 0;
}

/////////////////////////////////////////
//  Transmit a C style string          //
//    Returns bytes left available in  //
//    transmit ring buffer.            //
//://////////////////////////////////////
int AsyncPort::Tx(char *String)
{
    return (async_txblk(this, String, strlen(String)));
}

/////////////////////////////////////////
//  AllocBuffers                       //
//    Function to allocate ring buffer //
//    memory.  Returns 1 if success,   //
//    0 if memory already allocated or //
//    no memory available.             //
//://////////////////////////////////////
int AsyncPort::AllocBuffers(int RxBufSz, int TxBufSz, int UseFarMem)
{
    unsigned long memptr;
    int memsize = RxBufSz + TxBufSz;

    if (RingSeg || RingOfst)
        return 0;                   // don't alloc if already allocated

#if SPTR  ////////////////////////////////////
          //  SMALL OR MEDIUM MEMORY MODEL  //
          ////////////////////////////////////
    if (UseFarMem != 0)
    {                               // if ring buffers in FAR mem
        memptr = (unsigned long)farmalloc(memsize);
        Stat3 |= (char)B_FARBUFFER;
    }
    else                            // if ring buffers in NEAR mem
        memptr = (unsigned long)(unsigned int)new char[memsize];

#else     /////////////////////////////////////
          //  LARGE OR COMPACT MEMORY MODEL  //
          /////////////////////////////////////
    memptr = (unsigned long)new char[memsize];

#endif    ////////////////////////////////////
          //    END MODEL SPECIFIC CODE     //
          ////////////////////////////////////
    RxSize = RxBufSz;               // receive buffer size
    TxSize = TxBufSz;               // transmit buffer size
    RingSeg = (int)(memptr >> 16);  // seg of allocated memory (0 if NEAR)
    RingOfst = (int)memptr;         // offset of allocated memory
    if (memptr == 0L)
        return 0;                   // return 0 if no memory available
    return 1;                       // return 1, had some memory
}

/////////////////////////////////////////
//  FreeBuffers                        //
//    Function to release ring buffer  //
//    memory.  Returns 1 if success,   //
//    0 if port not closed or no mem-  //
//    ory to free.                     //
//://////////////////////////////////////
int AsyncPort::FreeBuffers()
{
    if ((!RingSeg && !RingOfst) || (HdlrUsed != '\0'))
        return 0;                   // exit if port busy or nothing to free

#if SPTR  ////////////////////////////////////
          //  SMALL OR MEDIUM MEMORY MODEL  //
          ////////////////////////////////////
    if (Stat3 & (char)B_FARBUFFER)
    {                               // if FAR memory was allocated
        farfree(MK_FP(RingSeg, RingOfst));
        Stat3 &= (char)~B_FARBUFFER;
    }
    else
        delete (char *)RingOfst;    // if NEAR memory was allocated

#else     /////////////////////////////////////
          //  LARGE OR COMPACT MEMORY MODEL  //
          /////////////////////////////////////
    delete MK_FP(RingSeg, RingOfst);

#endif    ////////////////////////////////////
          //    END MODEL SPECIFIC CODE     //
          ////////////////////////////////////
    RingSeg = RingOfst = 0;         // show ring buffers not allocated
    return 1;
}


