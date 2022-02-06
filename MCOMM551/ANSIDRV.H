
/****************************************************************************
 ANSIDRV.H -- FUNCTION DECLARATIONS AND MACROS FOR ANSI DISPLAY ROUTINES
   Mike Dumdei,  6 Holly Lane,  Texarkana TX  75503    (c) 1989,90,91
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

/* * COMPATIBILITY WITH ANSI8MSC MACRO DUE TO d_strnat FUNCTION CHANGE * */
#define d_strnat(r,c,ch,atr,n)  d_nchat((r),(c),(ch),(atr),(n),1)
#define HORZ    1
#define VERT    0            /* defines for direction for d_nchat function */

#define SCRBUF(r, c) (16 + (2 * (r) * (c))) /* storage for a pushed screen */

/* * MACROS TO DEFINE CURRENT WINDOW (startup value is 0,0,24,79) * */
#define SETWND(t,l,b,r) v_wndsz = 16777216L*(t) + 65536L*(l) + 256L*(b) + (r)
#define SETTOPLFT(t,l)  v_toplft = 256*(t) + (l)
#define SETBTMRGT(b,r)  v_btmrgt = 256*(b) + (r)
/* * MACROS TO DEFINE ENTIRE SCREEN (startup value is 0,0,24,79) * */
#define SETsWND(t,l,b,r) vs_wndsz = 16777216L*(t) + 65536L*(l) + 256L*(b) + (r)
#define SETsTOPLFT(t,l)  vs_toplft = 256*(t) + (l)
#define SETsBTMRGT(b,r)  vs_btmrgt = 256*(b) + (r)

#ifdef __TURBOC__
  #ifdef __HUGE__
    #undef _N_
    #define _N_
  #endif
#endif

/* * GLOBAL VIDEO VARIABLES DECLARED IN THE 'INITVID' FUNCTION * */
extern int _N_ v_struct;              /* pointer to global video variables */
extern int _N_ v_seg;                   /* seg address of video ram memory */
extern char _N_ v_mode;                              /* current video mode */
extern char _N_ v_page;                              /* current video page */
extern unsigned long _N_ v_wndsz;  /* current window dimensions (TLBR hex) */
extern unsigned int _N_ v_btmrgt;              /* current window btm right */
extern unsigned char _N_ v_rgt;         /* current window rightmost column */
extern unsigned char _N_ v_btm;                  /* current window btm row */
extern unsigned int _N_ v_toplft;               /* current window top left */
extern unsigned char _N_ v_lft;          /* current window leftmost column */
extern unsigned char _N_ v_top;                  /* current window top row */
extern unsigned long _N_ vs_wndsz;     /* max dimensions for entire screen */
extern unsigned int _N_ vs_btmrgt;               /* max btm right position */
extern unsigned int _N_ vs_toplft;                /* max top left position */
extern char _N_ v_scrlm;          /* scroll mode (1=scroll, 0=cursor wrap) */
extern char _N_ v_cntrlm;        /* control mode (1=exec cntrls, 0=dsplay) */
extern char _N_ v_textm;         /* text mode (1= \n == \n\r, 0= \n == \n) */
extern char _N_ v_ansi;           /* ansi mode (1=use ANSI, 0=see garbage) */
extern char _N_ v_color;      /* current atrib to use when displying chars */
extern char _N_ v_snow;        /* snow flag (1=wait for retrace, 0=no wait */
extern char _N_ v_bios;         /* bios flag (1=use BIOS, 0=direct writes) */
extern char _N_ v_ansiseq;       /* flag showing ANSI sequence in progress */
extern char _N_ v_wrel;    /* winfunc locs relative to window upr/lft if 1 */
extern char _N_ vs_wrel;  /* scrnfunc locs relative to window upr/lft if 1 */
extern char _N_ v_cga;                 /* set to 1 if CGA adaptor detected */
extern char _N_ v_bksp;           /* 1=destructive bksp, 0=non-destructive */

#ifdef __TURBOC__
  #ifdef __HUGE__
    #undef _N_
    #define _N_ near
  #endif
#endif

#if __cplusplus
  extern "C" {
#endif

/* *  FUNCTION DECLARATIONS  * */
int _C_ initvid(void);                                 /* initialize video */
void _C_ loc(int r, int c);                        /* locate cursor at r,c */
void _C_ d_strat(int r, int c, char *str);        /* display string at r,c */
void _C_ d_str(char *str);                        /* disp string at curpos */
void _C_ d_chat(int r, int c, char ch);                /* disp char at r,c */
void _C_ d_ch(char ch);                             /* disp char at curpos */
void _C_ d_msgat(int r, int c, int atrb, char *str);    /* disp msg at r,c */
void _C_ d_atrbat(int r, int c, int atrb, int n);   /* disp atrb,nlocs,r,c */
void _C_ d_nchat(int, int, char, int, int, int);      /* r,c,ch,atrb,n,dir */
int _C_ rd_scrn(void);                         /* read atrb/char at curpos */
int _C_ cls(void);                                         /* clear window */
void _C_ scrlup(int n);                                /* scroll up n rows */
void _C_ scrldn(int n);                              /* scroll down n rows */
void _C_ setcurloc(int);                   /* set cur loc, hibyt=r,lobyt=c */
int _C_ getcurloc(void);                  /* get cur loc, hibyt=r, lobyt=c */
void _C_ setcursiz(int);                               /* set cursor shape */
int _C_ getcursiz(void);                               /* get cursor shape */
void _C_ setvmode(int);                                  /* set video mode */
int _C_ getvmode(void);                                  /* get video mode */
void _C_ setvpage(int);                                  /* set video page */
int _C_ getvpage(void);                                  /* get video page */
void _C_ setvbordr(int);                               /* set video border */
 /*
  *  Functions formerly declared in EXTRA.H.  They were move here
  *  to get all the video related variables in one header file.
  */
void _C_ moveblk(int row, int col, int nrows, int ncols, char *buf, int dir);
void _C_ pu_scrnd(int r, int c, int nrows, int ncols, char *buf);
void _C_ po_scrnd(char *buf);
int _C_ pushscrn(int r, int c, int nrows, int ncols);
int _C_ popscrn(void);
void _C_ fpu_scrnd(int r, int c, int nrows, int ncols, char _F_ *buf);
void _C_ fpo_scrnd(char _F_ *buf);
int _C_ fpushscrn(int r, int c, int nrows, int ncols);
int _C_ fpopscrn(void);
void _C_ d_msgatd(int r, int c, int atrb, char *str);
void _C_ scrlupat(int top, int lft, int btm, int rgt, int nrows);
void _C_ scrldnat(int top, int lft, int btm, int rgt, int nrows);
void _C_ d_scrd(int r, int c, char *scrn);
char * _C_ rd_scrnd(int r, int c, int nbytes, char *buffer);

#ifdef __cplusplus
  }
#endif

