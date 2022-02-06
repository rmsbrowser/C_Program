
/****************************************************************************
 EXTRA.H -- miscellaneous functions
   Mike Dumdei,  6 Holly Lane,  Texarkana TX  75503    (c) 1989,1990
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

#if __cplusplus
  extern "C" {
#endif
#if __ZTC__
  #define INTERRUPT
#else
  #define INTERRUPT interrupt
#endif

#define SET_TO_TENTHS(to, tenths)   set_timeout((&to), (tenths)*9/5)
#define SET_TO_SECS(to, secs)       set_timeout((&to), (secs)*18)
#define SET_TO_MINS(to, mins)       set_timeout((&to), (mins)*1080)
#define DELAY_TENTHS(tenths)        tdelay((tenths)*9/5)
#define DELAY_SECS(secs)            tdelay((secs)*18)
#define DELAY_MINS(mins)            tdelay((mins)*1080)

/* function declarations */
int _C_ calc_crc(char *blk, int blksz);
int _C_ update_crc(int oldcrc, char ch);
int _C_ strcntch(char *str, char ch_to_cnt);
char * _C_ strrstr(char *str1, char *str2);
char * _C_ strpbrkf(char *str1, char *str2);
char * _C_ strrtrim(char *str);
char * _C_ strltrim(char *str);
char * _C_ strtrim(char *str);
char * _C_ strsum(char *, ...);               /* dest, str1, str2, ..., NULL */
char * _C_ strrjust(char *str, int fldwidth, char pad);
char * _C_ strljust(char *str, int fldwidth, char pad);
char * _C_ strcentr(char *str, int fldwidth, char pad);
int _C_ watchdogset(int flag, int commchipadrs);
void INTERRUPT _F_ watchdoghook(void);
int _C_ ctshookset(int flag, int port, int chipadrs);
void INTERRUPT _F_ int14ctshook(void);

 /*------- NEW TIMER FUNCTIONS ---------*/
#ifndef TIMER_H
int _C_ tickhookset(int flag); /* MUST disable before exiting program !!!!! */
void _C_ set_timeout(long *timer, unsigned ticks);
void _C_ set_longtimeout(long *timer, long ticks);
int _C_ timed_out(long *timer);
void _C_ tdelay(unsigned ticks);
long _C_ get_ticker(void);
#define TIMER_H
#endif

#if __cplusplus
  }
#endif

