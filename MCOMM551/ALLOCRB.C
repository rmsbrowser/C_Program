
/*
    ALLOCRB.C -- source file of code to allocate ring buffers and initialize
     the required port structure members for MCOMM5 async routines.

    Mike Dumdei, 6 Holly Lane, Texarkana TX 75503  (c) 1989
*/

#if defined(__TURBOC__)
    #include <alloc.h>
    #define _fmalloc farmalloc
#elif defined(__ZTC__)
    #include <dos.h>
    #include <stdlib.h>
    #define _fmalloc farmalloc
#else
    #include <malloc.h>
#endif
#include <comm.h>

int AllocRingBuffer(
 ASYNC *port,                                 /* pointer to port structure */
 int rxsize,                     /* number bytes to use for receive buffer */
 int txsize,                    /* number bytes to use for transmit buffer */
 int useFARmem)                   /* flag set if using FAR mem for buffers */
{
    unsigned long memptr;
    int memsize;

    memsize = rxsize + txsize;

    if (useFARmem || sizeof(char *) == 4)              /* if FAR Ring bufs */
        memptr = (unsigned long)_fmalloc(memsize);
    else                                /* if Ring buffers use NEAR memory */
        memptr = (unsigned long)(unsigned int)malloc(memsize);

     /* pre-initialize 4 required structure members */
    port->RxSize = rxsize;                          /* receive buffer size */
    port->TxSize = txsize;                         /* transmit buffer size */
    port->RingSeg = (int)(memptr >> 16);                        /* SEG adr */
    port->RingOfst = (int)memptr;                          /* OFST address */
    if (memptr == 0L)
        return 0;                       /* return 0 if no memory available */
    return 1;                                 /* return 1, had some memory */
}

