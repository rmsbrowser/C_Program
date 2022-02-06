
/** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*                                                                           *
*       I N T   1 4   H O O K   T H A T   W A I T S   F O R   C T S         *
*              Mike Dumdei, 6 Holly Lane, Texarkana TX 75503                *
*                   Requires ASM module --> INT14HK.ASM                     *
*                                                                           *
*   This function sets a hook into the BIOS serial interrupt vector that    *
*   waits indefinitely for CTS to go high on the specified port when send-  *
*   ing data.  For the hook to have any effect when INT14 is called:        *
*       1)  DX has to equal the 'portval' you set here                      *
*       2)  AH has to equal '1' -- the send char service ID                 *
*       3)  CTS must be FALSE                                               *
*   If these conditions are all TRUE, then the hook goes into a loop        *
*   until CTS goes high.  The purpose of the function is keep high speed    *
*   modems that CTS handshake from creating timeout errors when redirect-   *
*   ing stdout and stderr to the comm port.                                 *
*                                                                           *
*           To enable:  call with flag = 1                                  *
*                       portval = COM1 (0) or COM2 (1)                      *
*                       combase = base adrs of comm chip (ex: 3F8)          *
*           To disable: call with flag = 0                                  *
*                       portval = don't care                                *
*                       combase = don't care                                *
*                                                                           *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
#include <dos.h>

#if defined (__TURBOC__)
  #define 	_dos_getvect	getvect
  #define	_dos_setvect	setvect
#endif

#if defined(__ZTC__)
  #include <int.h>
  #define INTERRUPT
#else
  #include <dos.h>
  #define INTERRUPT interrupt
#endif

#define uint    unsigned int
#define INT14   0x14                       /* BIOS serial interrupt vector */

/* these are in the ASM module */
void INTERRUPT far int14ctshook(void);
extern void (INTERRUPT far *oldint14)();
extern uint msradr14;
extern uint port14;

int ctshookset(int flag, uint portval, uint combase)
{
    if (flag)                                   /* enabling the INT14 hook */
    {
        if (msradr14 != 0)
            return (-1);                       /* error if already enabled */
        /* else set pointers and hook into BIOS serial port interrupt */
        msradr14 = combase + 6;          /* point to modem status register */
        port14 = portval;                        /* identify port affected */
#if !defined (__ZTC__)
        oldint14 = _dos_getvect(INT14);
        _dos_setvect(INT14, int14ctshook);           /* hook INT 14 vector */
#else
        int_getvector(INT14, (uint *)&oldint14, (uint *)(&oldint14+2));
        int_setvector(INT14, (uint)int14ctshook, *(uint *)(&int14ctshook+2));
#endif
        return (0);
    }
    else                                       /* disabling the INT14 hook */
    {
        if (msradr14 == 0)
            return (-1);                           /* error if not enabled */
        /* else set INT 14 back to original vector & reset comchip var */
#if !defined (__ZTC__)
        _dos_setvect(INT14, oldint14);         /* reset vector to original */
#else
        int_setvector(INT14, (uint)oldint14, *(uint *)(&oldint14+2));
#endif
        msradr14 = 0;                                  /* show not enabled */
        return (0);
    }
}

        

