
/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//  ANSIVID.HPP - header file for C++ programs using MCOMM video routines. //
//    Zortech C++ 2.0, Turbo C++ 1.00 version.                             //
//                                                                         //
//    Mike Dumdei,  6 Holly Lane,  Texarkana TX  75503    (c) 1989,1990    //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#if !defined(ANSIVID_HPP)
#define ANSIVID_HPP

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

  extern "C" {
/////////////////////////////////////////////////////////////////
//  GLOBAL VIDEO VARIABLES DECLARED IN THE 'INITVID' FUNCTION  //
/////////////////////////////////////////////////////////////////
extern int _N_ v_struct;            // pointer to global video variables
extern int _N_ v_seg;               // SEG address of video ram memory
extern char _N_ v_mode;             // current video mode
extern char _N_ v_page;             // current video page
extern unsigned long _N_ v_wndsz;   // current window dimensions (TLBR hex)
extern unsigned int _N_ v_btmrgt;   // current window btm right
extern unsigned char _N_ v_rgt;     // current window rightmost column
extern unsigned char _N_ v_btm;     // current window btm row
extern unsigned int _N_ v_toplft;   // current window top left
extern unsigned char _N_ v_lft;     // current window leftmost column
extern unsigned char _N_ v_top;     // current window top row
extern unsigned long _N_ vs_wndsz;  // max dimensions for entire screen
extern unsigned int _N_ vs_btmrgt;  // max btm right position
extern unsigned int _N_ vs_toplft;  // max top left position
extern char _N_ v_scrlm;      // scroll mode (1=scroll, 0=cursor wrap)
extern char _N_ v_cntrlm;     // control mode (1=exec cntrls, 0=dsplay)
extern char _N_ v_textm;      // text mode (1= \n == \r\n, 0= \n == \n)
extern char _N_ v_ansi;       // ansi mode (1=use ANSI, 0=see garbage)
extern char _N_ v_color;      // current atrib to use when displying chars
extern char _N_ v_snow;       // snow flag (1=wait for retrace, 0=no wait
extern char _N_ v_bios;       // bios flag (1=use BIOS, 0=direct writes)
extern char _N_ v_ansiseq;    // flag showing ANSI sequence in progress
extern char _N_ v_wrel;       // window locs relative to window upr/lft if 1
extern char _N_ vs_wrel;      // screen locs relative to window upr/lft if 1
extern char _N_ v_cga;        // set to 1 if CGA adaptor detected
extern char _N_ v_bksp;       // 1=destructive backspace, 0=non-destructive

/////////////////////////////////////////////////////////////////
//  LOW LEVEL FUNCTION DECLARATIONS                            //
/////////////////////////////////////////////////////////////////
int initvid(void);                      // initialize video
void loc(int r, int c);                 // locate cursor at r,c
void d_strat(int r, int c, char *str);  // display string at r,c
void d_str(char *str);                  // disp string at curpos
void d_chat(int r, int c, char ch);     // disp char at r,c
void d_ch(char ch);                     // disp char at curpos
void d_msgat(int r, int c, int atrb, char *str);  // disp msg at r,c
void d_atrbat(int r, int c, int atrb, int n);     // r,c,atrb,nlocs
void d_nchat(int, int, char, int, int, int);      // r,c,ch,atrb,n,dir
int rd_scrn(void);                      // read atrb/char at curpos
void cls(void);                         // clear window
void scrlup(int n);                     // scroll up n rows
void scrldn(int n);                     // scroll down n rows
void setcurloc(int);                    // set curloc, hibyte=r,lobyte=c
int getcurloc(void);                    // get curloc, hibyte=r, lobyte=c
void setcursiz(int);                    // set cursor shape
int getcursiz(void);                    // get cursor shape
void setvmode(int);                     // set video mode
int getvmode(void);                     // get video mode
void setvpage(int);                     // set video page
int getvpage(void);                     // get video page
void setvbordr(int);                    // set video border
void moveblk(int, int, int, int, char *, int);  // r,c,nrow,ncols,buf,dir
void pu_scrnd(int, int, int, int, char *);      // r,c,nrows,ncols,buf
void po_scrnd(char *);                  // buf
int pushscrn(int, int, int, int);       // r,c,nrows,ncols
int popscrn(void);
void fpu_scrnd(int, int, int, int, char _F_ *); // r,c,nrows,ncols,farbuf
void fpo_scrnd(char _F_ *);             // farbuf
int fpushscrn(int, int, int, int);      // r,c,nrows,ncols
int fpopscrn(void);
void d_msgatd(int, int, int, char *);   // r,c,atrib,string
void scrlupat(int, int, int, int, int); // top,left,btm,rgt,nrows
void scrldnat(int, int, int, int, int); // top,left,btm,rgt,nrows
void d_scrd(int, int, char *);          // r,c,screendata
char *rd_scrnd(int, int, int, char *);  // r,c,nbtyes,buf
  }

/////////////////////////////////////////////////////////////////
//  C++ SPECIFIC DECLARATIONS                                  //
/////////////////////////////////////////////////////////////////

inline void SetWindowSize(int Top, int Lft, int Btm, int Rgt)
 { v_toplft = (Top << 8) | Lft, v_btmrgt = (Btm << 8) | Rgt; }
inline void SetScreenSize(int Top, int Lft, int Btm, int Rgt)
 { vs_toplft = (Top << 8) | Lft, vs_btmrgt = (Btm << 8) | Rgt; }
inline void Scroll(int NbrLines = 1, int Up = 1)
 { if (Up >= 0) scrlup(NbrLines); else scrldn(NbrLines); }
inline void PosCursor(int Row, int Col = -1)
 { if (Col >= 0) loc(Row, Col); else setcurloc(Row); }
inline int PosCursor()
 { return getcurloc(); }
inline void CursorShape(int ScanHiLo)
 { setcursiz(ScanHiLo); }
inline int CursorShape()
 { return getcursiz(); }
inline void ClearWindow()
 { cls(); }
inline void Display(int Row, int Col, char *String)
 { d_strat(Row, Col, String); }
inline void Display(char *String)
 { d_str(String); }
inline void Display(int Row, int Col, char Ch)
 { d_chat(Row, Col, Ch); }
inline void Display(char Ch)
 { d_ch(Ch); }
inline void Display(int Row, int Col, int Attrib, char *String)
 { d_msgat(Row, Col, Attrib, String); }
inline void Display(int Color, char *String)
 { v_color = (char)Color, d_str(String); }
inline void Fill(int Row, int Col, char Ch, int Attrib, int NbrLocs,
  int Direction = 1)
 { d_nchat(Row, Col, Ch, Attrib, NbrLocs, Direction); }
inline void Paint(int Row, int Col, int Attrib, int NbrLocs)
 { d_atrbat(Row, Col, Attrib, NbrLocs); }

#endif          // ANSIVID_HPP

