
/** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*                                                                           *
*             W A T C H D O G   F U N C T I O N   F O R   M S C             *
*               Mike Dumdei, 6 Holly Lane, Texarkana TX 75503               *
*                    Requires ASM module --> WDOGHOOK.ASM                   *
*                                                                           *
*       IF YOU USE THE TICKHOOK FUNCTION IN COMMx.LIB AND THIS FUNC-        *
*       TION IN THE SAME PROGRAM YOU MUST UNINSTALL THE HOOKS IN THE        *
*       REVERSE ORDER THAT THEY WERE INSTALLED  !!!!!!!!!!!!                *
*       -------------------------------------------------------------       *
*                                                                           *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
#if defined (__TURBOC__)
  #define _dos_getvect  getvect
  #define _dos_setvect  setvect
#endif

#if defined(__ZTC__)
  #include <int.h>
  #define INTERRUPT
#else
  #include <dos.h>
  #define INTERRUPT interrupt
#endif

#define uint    unsigned int
#define TIMER   0x1C                        /* timer tick interrupt vector */

/* these are in the ASM module */
void INTERRUPT far watchdoghook(void);
extern void (INTERRUPT far *oldtimerint)();
extern uint msrportadrs;

int watchdogset(int flag, uint combase)
{
    if (flag)                                         /* enabling watchdog */
    {
        if (msrportadrs != 0)
            return (-1);                       /* error if already enabled */
        /* else set pointers and hook into the timer interrupt */
        msrportadrs = combase + 6;       /* point to modem status register */
#if !defined (__ZTC__)
        oldtimerint = _dos_getvect(TIMER);
        _dos_setvect(TIMER, watchdoghook);               /* hook the timer */
#else
        int_getvector(TIMER, (uint *)&oldtimerint, (uint *)(&oldtimerint+2));
        int_setvector(TIMER, (uint)watchdoghook, *(uint *)(&watchdoghook+2));
#endif
        return (0);
    }
    else                                             /* disabling watchdog */
    {
        if (msrportadrs == 0)
            return (-1);                           /* error if not enabled */
        /* else set timer back to original vector & reset comchip var */
#if !defined (__ZTC__)
        _dos_setvect(TIMER, oldtimerint);      /* reset vector to original */
#else
        int_setvector(TIMER, (uint)oldtimerint, *(uint *)(&oldtimerint+2));
#endif
        msrportadrs = 0;
        return (0);
    }
}

        
