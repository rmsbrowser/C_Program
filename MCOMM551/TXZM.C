
/*/////////////////////////////////////////////////////////////////////
//                                                                   //
//  TXZM.C -- zmodem protocol driver (formerly ZMP)                  //
//                                                                   //
//    (c) 1991, Mike Dumdei, 6 Holly Lane, Texarkana TX, 75503       //
//                                                                   //
//////////////////////////////////////////////////////////////////// */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <process.h>
#include <ctype.h>
#include <io.h>
#include <dos.h>
#include <bios.h>
#include "comm.h"
#include "ansidrv.h"
#include "extra.h"
#include "colors.h"
#include "zmdos.h"

#ifdef __TURBOC__
  #include <dir.h>
  #define _chdrive(d) setdisk((d)-1)
#endif

#ifdef __ZTC__
  #include <direct.h>
#endif

#define ALT_C       0x2e00
#define ALT_D       0x2000
#define ALT_E       0x1200
#define ALT_H       0x2300
#define ALT_L       0x2600
#define ALT_P       0x1900
#define ALT_R       0x1300
#define ALT_S       0x1f00
#define ALT_X       0x2d00
#define FK1         0x3b00
#define PGUP        0x4900
#define PGDN        0x5100
#define ALT_EQU     0x8300
#define X_ESC       0x011b

void AddToList(char *subdir);
char *CaptureBaudRate(void);
char *ConvertSecs(long secs);
long ConvertTicks(long ticks);
int Dial(void);
void DrawBox(int row, int col, int nrows, int ncols, int color, int style);
void DspZmodemScrn(void);
char *ExpandSubDirs(char *fnames);
int FileTransfer(void);
void HelpFunc(void);
void InitDefaults(void);
void MiniTermMode(void);
void ProcCmdLine(int argc, char * *argv);
int prompt(char *buf, int maxlen);
int RecurseSubDirs(char *sd);
int RepeatDial(void);
void SetFIFOLevels(int rxFIFOlevel, int txFIFOlevel);
int SetParams(char *newparams);
void usage(void);
void vDisplay(int row, int col, char *format, ...);
int waitfor(int ticks, ...);
void ZMsg(int type, ...);

/*/////////////////////////////////////////////////////////
//  Configuration structure.  Structure is used to allow //
//  the defaults to be changed modifying the EXE.        //
//  "TXZMCFG:" is the tag to search for to find the start//
//  of the structure in the EXE.                         //
//////////////////////////////////////////////////////// */
struct PROTCONFIG
{
    char    tag[8], DLPath[80], ULPath[80];
    long    LocBaud, MinFifoBaud;
    int     ComBase, IRQ, Vctr;
    int     h_VBufSize, h_BufSize, b_VBufSize, b_BufSize;
    int     TxWindow, ZExistOpts, XYExistOpts;
    int     FifoTxLvl, FifoRxLvl;
    char    IgnCarrier, MsrFlow, KeepTime, EscCtl, OvlyIO;
    char    Color[10], Mono[10];
} cfg =
{
    "TXZMCFG:", "", "",
    0L, 1L,
    0x3f8, IRQ4, VCTR4,
    2048, 0, 20508, 20480,
    0, 1, 2,
    8, 8,
    0, 0, 1, 0, 1,
    {  WHT|BLU_B,  H_GRN|BLU_B,  H_RED|BLU_B,    YLW|BLU_B,
       GRY|BLU_B,    YLW|BLU_B,  H_RED|BLU_B,  H_MAG|BLU_B,
       CYN,              CYN_B  },
    { WHT, WHT, RVRS, H_WHT, WHT, H_WHT, H_WHT, H_WHT, WHT, WHT_B }
};

/*/////////////////////////////////////////////////////////
//  Screen data structure                                //
//////////////////////////////////////////////////////// */
typedef struct
{
    int row, col, color, count;
    char *text;
} SCREENDATA;

/*/////////////////////////////////////////////////////////
//  Global variables                                     //
//////////////////////////////////////////////////////// */
ASYNC port;                 /* ASYNC port structure */
int combase, irq, vctr;     /* port address, IRQ number, & vector */
int openmask = 0;           /* mask for forcing no FIFOs, no MSR intrpts */
long LocBaud = 0L;          /* CPU to modem/device baud rate */
char params[12] = "";       /* port parameters */
char lockedbaud[12] = "";   /* locked baud parameter */
char fnames[256] = "";      /* list of files to send if sending */
char minsecs[10];           /* ticks to min:secs buffer */
char *color;                /* pointer to list of colors */
int txtcolor;               /* color of most screen message output */
char buf[256];              /* general purpose buffer */
int goodargs = 0;           /* got an 'r' or an 's' on command line */
char *node = NULL;          /* for bbs use: LASTUSER.BBS format file name */
int miniterm = 0;           /* mini-terminal mode selected */
char OvlyIO;                /* overlay disk and serial I/O flag */
char *flist;                /* used when expanding subdirectories */
char mask[14];              /* used when expanding subdirectories */
int plen;                   /* used when expanding subdirectories */
int tryDV = 0;              /* cmdline switch to test for DesqView */
int checkcarrier = 0;       /* check for carrier during 'waitfor' flag */
char phone[40] = "";        /* phone number to dial */

long tfBytes, tCPS;         /* total bytes transferred, average CPS */
int tFiles;                 /* number of files transferred */

/*/////////////////////////////////////////////////////////
//                                                       //
//      Main                                             //
//                                                       //
//:mai////////////////////////////////////////////////// */
void cdecl main(int argc, char *argv[])
{
    int i;
    char *p1;

    color = ((initvid() & 0xff) == CO80) ? cfg.Color : cfg.Mono;
    txtcolor = (int)color[1] & 0xff;
    InitDefaults();
    ProcCmdLine(argc, argv);
    if (!goodargs)
        usage();

    if (tryDV && TestDesqView() != 0)
    {
        i = DV_VideoSeg(v_seg);
        if (v_seg != i)
            v_seg = i, v_snow = 0;
    }
    if (*params)
        ConnectBaud = atol(params);
    if (*lockedbaud)
        LocBaud = atol(lockedbaud);
    if (LocBaud == 0L)
        LocBaud = ConnectBaud;
    if (LocBaud != 0L)
    {
        sprintf(params, "%ldN81", LocBaud);
        if (LocBaud < cfg.MinFifoBaud)
            openmask |= 0x4000;
    }

    if (OvlyIO)
        VBufSize = cfg.h_VBufSize, BufSize = cfg.h_BufSize;
    else
        VBufSize = cfg.b_VBufSize, BufSize = cfg.b_BufSize;
    if (BufSize)
        ZFR0 &= ~CANOVIO;

    AllocRingBuffer(&port, 2048, 4096, 0);
    while (1)
    {
        i = async_open(&port, combase|openmask, irq, vctr, params);
        if (i != 0)
        {
            printf("\nSerial port open error, Error code = %d\n\a", i);
            exit(i - 20); /* -20 so exit code don't clash with zm result */
        }
        if (ConnectBaud == 0L)
        {
            ConnectBaud = atol(port.BPDSstr);
            strcpy(params, port.BPDSstr);
            if (ConnectBaud < cfg.MinFifoBaud && async_16550(&port)
             && !(openmask & 0x4000))
            {
                async_close(&port);
                openmask |= 0x4000;
                continue;
            }
        }
        break;
    }
    async_msrflow(&port, cfg.MsrFlow);
    tickhookset(1);

    if (miniterm)
    {
        pushscrn(0, 0, 25, 80);
        SetFIFOLevels(1, 1);
        MiniTermMode(), i = 0;
        popscrn();
    }
    else
    {
        i = FileTransfer();
        sprintf(buf, "TXZM exit code = %d", i);
        v_color = WHT;
        d_strat(23, 0, buf);
    }
    async_close(&port);
    tickhookset(0);
    exit(i);
}

/*/////////////////////////////////////////////////////////
//  AddToList                                            //
//:add////////////////////////////////////////////////// */
void AddToList(char *subdir)
{
    static char bkslsh[2] = " ";
    int i, j;

    if (*(strchr(subdir, '\0') - 1) == '\\' || subdir == GetNameExt(subdir))
        j = 1, *bkslsh = '\0';
    else
        j = 2, *bkslsh = '\\';
    i = plen - 1;
    plen += (strlen(subdir) + strlen(mask) + j);
    flist = realloc(flist, plen);
    sprintf(&flist[i], "\n%s%s%s", subdir, bkslsh, mask);
}

/*/////////////////////////////////////////////////////////
//  CaptureBaudRate                                      //
//:cap////////////////////////////////////////////////// */
char *CaptureBaudRate(void)
{
    int i;
    static char *b[] = { "300", "300", "1200", "2400", "4800", "7200",
        "9600", "14400", "19200" };

    checkcarrier = 0;
    i = waitfor(18, "\r", b[1],b[2],b[3],b[4],b[5],b[6],b[7],b[8], NULL);
    return ((i >= 0 && i <= 8) ? b[i] : NULL);
}

/*///////////////////////////////////////////////
//  ConvertSecs                                //
//:con//////////////////////////////////////// */
char *ConvertSecs(long secs)
{
    sprintf(minsecs, "%01ld:%02ld   " , secs / 60L, secs % 60L);
    return(minsecs);
}

/*///////////////////////////////////////////////
//  ConvertTicks                               //
//:con//////////////////////////////////////// */
long ConvertTicks(long ticks)
{
    long secs = ((ticks * 10L) / 182L) + 1L;
    ConvertSecs(secs);
    return(secs);
}

/*/////////////////////////////////////////////////////////
//  Dial                                                 //
//:dia////////////////////////////////////////////////// */
int Dial(void)
{
    int rval;
    char *p1;

    async_txblk(&port, phone, strlen(phone));
    checkcarrier = 0;
    rval = waitfor(45 * 18, "CONNECT", "BUSY", "NO CARRIER", "VOICE",
     "ERROR", "NO DIALTONE", NULL);
    if (rval == 0)
    {
        if ((p1 = CaptureBaudRate()) != NULL)
            SetParams(p1);
    }
    return (rval);
}

/*/////////////////////////////////////////////////////////
//  DrawBox                                              //
//:dra////////////////////////////////////////////////// */
void DrawBox(int row, int col, int nrows, int ncols, int color, int style)
{
    static char boxchars[][6] =
    {
        'Ú', '¿', 'À', 'Ù', 'Ä', '³',
        'É', '»', 'È', '¼', 'Í', 'º',
        'Õ', '¸', 'Ô', '¾', 'Í', '³',
        'Ö', '·', 'Ó', '½', 'Ä', 'º'
    };
    char *box = boxchars[style];
    int i, temp = v_color;

    v_color = color;
    --ncols;
    scrlupat(row, col, row + nrows - 1, col + ncols, nrows);
    --nrows;
    ++col, --ncols, --ncols;
    d_nchat(row, col, box[4], color, ncols, 1);
    d_nchat(row + nrows, col, box[4], color, ncols, 1);
    d_nchat(row, col, box[5], color, nrows, 0);
    d_nchat(row, col + ncols, box[5], color, nrows, 0);
    d_chat(row, col, box[0]);
    d_chat(row, col + ncols, box[1]);
    d_chat(row + nrows, col, box[2]);
    d_chat(row + nrows, col + ncols, box[3]);
    v_color = temp;
}

/*/////////////////////////////////////////////////////////
//  DspZmodemScrn                                        //
//:dsp////////////////////////////////////////////////// */
void DspZmodemScrn(void)
{
    static SCREENDATA zscrn[] =
    {
        {  1,  6, 2,   0, " Zmodem File " },
        {  2, 71, 0, -20, "³" },
        { 15,  5, 0,  64, "Ä" },
        {  2, 73, 0,   0, "F T" },
        {  3, 73, 4, -19, "±" },
        {  3, 75, 4, -19, "±" },
        {  3,  5, 0,   0, "File name :" },
        {  5,  5, 0,   0, "Estimated time :" },
        {  6,  5, 0,   0, "Elapsed time   :" },
        {  7,  5, 0,   0, "File CPS rate  :" },
        {  5, 37, 0,   0, "File position      :" },
        {  6, 37, 0,   0, "Expected file size :" },
        {  7, 37, 0,   0, "Beginning offset   :" },
        {  9, 51, 0,   0, "ÄÄÄ Hdr Data ÄÄÄ" },
        { 10, 23, 0,   0, "Header Name" },
        { 10, 40, 0,   0, "Type" },
        { 10, 52, 0,   0, "Hex" },
        { 10, 61, 0,   0, "Decimal" },
        { 11,  5, 0,   0, "Last hdr recvd :" },
        { 12,  5, 0,   0, "Last hdr sent  :" },
        { 14,  5, 0,   0, "Crc :" },
        { 14, 37, 0,   0, "Flow :" },
        { 17,  5, 0,   0, "Total files queued :" },
        { 18,  5, 0,   0, "Total bytes queued :" },
        { 17, 42, 0,   0, "Estimated time   :" },
        { 18, 42, 0,   0, "Accumulated time :" },
        { 20,  5, 0,   0, "Files transferred  :" },
        { 21,  5, 0,   0, "Bytes transferred  :" },
        { 21, 42, 0,   0, "Average CPS rate :" },
        { -1, -1, 0,   0, "" }
    };
    register SCREENDATA *sd;

    v_color = WHT;
    cls();
    v_color = txtcolor;
    DrawBox(1, 1, 22, 78, color[0], 0);
    for (sd = zscrn; sd->row >= 0; ++sd)
    {
        if (sd->count == 0)
            d_msgat(sd->row, sd->col, color[sd->color], sd->text);
        else if (sd->count > 0)
        {
            d_nchat(sd->row, sd->col, *sd->text, color[sd->color],
             sd->count, 1);
        }
        else
        {
            d_nchat(sd->row, sd->col, *sd->text, color[sd->color],
             -sd->count, 0);
        }
    }
    d_msgat(1, 19, color[2], (TFlag.F.Receiving) ? "Receive " : "Send ");
    loc(23, 0);
}

/*/////////////////////////////////////////////////////////
//  ExpandSubDirs                                        //
//:exp////////////////////////////////////////////////// */
char *ExpandSubDirs(char *fnames)
{
    DF fbuf;
    char *p1, *lstptr, *wrkname, *savdir, *orgdir;
    int i, dosub;

    orgdir = malloc(_MAX_PATH), getcwd(orgdir, _MAX_PATH);
    wrkname = malloc(_MAX_PATH), savdir = malloc(_MAX_PATH);
    plen = 1, lstptr = SkipSpaces(fnames), flist = calloc(1, 1);
    while (1)
    {
        if (lstptr != fnames)
        {
            if (wrkname[1] == ':')
                chdir(savdir);
            _chdrive(toupper(*orgdir) - 'A' + 1);
            chdir(orgdir);
        }
        if (*lstptr == '\0')
        {
            free(wrkname), free(orgdir), free(savdir);
            return(flist);
        }
        p1 = lstptr, i = SkipChars(lstptr) - lstptr, dosub = 0;
        lstptr = (SkipSpaces(SkipChars(lstptr)));
        if (*p1 == '(' && p1[i - 1] == ')')
            ++p1, dosub = 2;
        strncpy(wrkname, p1, i), wrkname[i - dosub] = '\0';
        if (wrkname[1] == ':')
        {
            _chdrive(toupper(*wrkname) - 'A' + 1);
            getcwd(savdir, _MAX_PATH);
        }
        p1 = GetNameExt(wrkname);
        strcpy(mask, "*.*");
        if (*p1)
        {
            fbuf.attrib = 0;
            if (!strchr(p1, '*') && !strchr(p1, '?'))
                DosFindFirst(wrkname, -1, &fbuf);
            if (fbuf.attrib & _A_SUBDIR)
                p1 = strchr(p1, '\0');
            else
            {
                strupr(strncpy(mask, p1, 12));
                p1[0] = mask[12] = '\0';
            }
        }
        if (p1 != wrkname && *(--p1) != ':')
        {
            if (p1 != wrkname && *p1 == '\\' && *(p1 - 1) != ':')
                *p1 = '\0';
            if (chdir(wrkname) != 0)
                continue;
        }
        getcwd(wrkname, _MAX_PATH);
        if (dosub == 0)
            AddToList(wrkname);
        else
            RecurseSubDirs(wrkname);
    }
}

/*/////////////////////////////////////////////////////////
//  FileTransfer - file send / receive caller            //
//:fil////////////////////////////////////////////////// */
int FileTransfer(void)
{
    FILE *fh;
    int rval, temp = v_color;
    long efficiency = 0L;

    if (miniterm)
        pushscrn(0, 0, 24, 80);
    DspZmodemScrn();
    if (node != NULL)           /* BBS support -- Maximus */
    {
        if ((fh = fopen(node, "rb")) != NULL)
        {
            memset(buf, 0, sizeof(buf));
            fread(buf, 1, sizeof(buf), fh);
            fclose(fh);
            buf[32] = buf[68] = '\0';
            d_msgat(23, 1, WHT, &buf[0]);
            d_msgat(23, 40, WHT, &buf[36]);
        }
    }
    SetFIFOLevels(cfg.FifoRxLvl, cfg.FifoTxLvl);
    ZMsg(M_RESET);
    if (TFlag.F.Receiving)
        rval = ZmodemRecv(&port);
    else
    {
        rval = ZmodemSend(&port, flist);
        free(flist);
    }
    if (ConnectBaud)
        efficiency = (tCPS * 1000L) / ConnectBaud;
    sprintf(buf, "CPS: %ld (%d files, %ld bytes)  Efficiency %ld%% \r\n",
     tCPS, tFiles, tfBytes, efficiency);
    d_msgat(23, 1, WHT, buf);
    tdelay(4);
    if (node != NULL && async_carrier(&port) && tFiles)
    {                           /* send other end result report */
        async_txblk(&port, buf, strlen(buf));
        while (!async_txempty(&port))
            ;
    }
    async_rxflush(&port);

    v_color = temp;
    if (miniterm)
    {
        SetFIFOLevels(1, 1);
        d_str("  Press Enter to continue ..");
        KBREAD;
        popscrn();
        d_str(buf);
    }
    return (rval);
}

/*/////////////////////////////////////////////////////////
//  HelpFunc - display help screen                       //
//:hel////////////////////////////////////////////////// */
void HelpFunc(void)
{
    static char helpscrn[] = "\n\
    -- TXZM COMMANDS --\n\
  ALT X:  Exit program         ALT E:  Echo On\n\
  ALT S:  Shell to DOS         ALT P:  Change Baud\n\
  ALT D:  Dial Number          ALT =:  Doorway Mode\n\
  ALT R:  Redial Number        PGUP :  Send Zmodem\n\
  ALT H:  Hang Up              PGDN :  Receive Zmodem\n\
  ALT L:  Toggle Log File (TXZM.CAP)\n";

    d_str(helpscrn);
}

/*/////////////////////////////////////////////////////////
//  InitDefaults - init protocol global variables        //
//:ini////////////////////////////////////////////////// */
void InitDefaults(void)
{
    char *p1;
    int i;

    combase = cfg.ComBase, irq = cfg.IRQ, vctr = cfg.Vctr;
    LocBaud = cfg.LocBaud;
    OvlyIO = cfg.OvlyIO;
    TFlag.F.ExistOpts = cfg.ZExistOpts;
    TFlag.F.IgnCarrier = cfg.IgnCarrier;
    TFlag.F.KeepTime = cfg.KeepTime;
    TFlag.F.EscCtl = cfg.EscCtl;
    TxWindow = cfg.TxWindow;
}

/*/////////////////////////////////////////////////////////
//  MiniTermMode - mini-terminal mode                    //
//:min////////////////////////////////////////////////// */
void MiniTermMode(void)
{
    static char rxtripstr[6] = { '*', '*', ZDLE, 'B', '0', '0' };
    static FILE *flog = NULL;
    int ch, rxtrip = 0, echo = 0, doorwaymode = 0;
    char *cmd;
    static char statline[] = "\
 Help F1 | Exit ALT-X | Shell ALT-S | Hangup ALT-H | Send PGUP | Recv PGDN";

    cmd = getenv("COMSPEC");
    if (cmd == NULL)
        cmd = "COMMAND";
    v_color = color[8];
    cls();
    sprintf(buf,
     "TXZM 2.14 Mini-Terminal Mode : %s : (c) 1991 Mike Dumdei\n\n",
     port.BPDSstr);
    d_strat(1, 0, buf);
    d_nchat(24, 0, ' ', color[9], 80, 1);
    d_msgat(24, 1, color[9], statline);
    SETWND(0, 0, 23, 79);
    while (1)
    {
        if (KBHIT)
        {
            if ((unsigned)(ch = KBREAD) == ALT_EQU)
            {
                if ((doorwaymode ^= 1) != 0)
                {
                    v_btm = 24;
                    d_nchat(24, 0, ' ', v_color, 80, 1);
                }
                else
                {
                    if (((ch = getcurloc()) >> 8) == 24)
                        d_ch('\n'), loc(23, (char)(ch & 0xff));
                    v_btm = 23;
                    d_nchat(24, 0, ' ', color[9], 80, 1);
                    d_msgat(24, 1, color[9], statline);
                }
            }
            else if (doorwaymode)
            {
                if (!(ch & 0xff))
                {
                    async_tx(&port, '\0');
                    ch >>= 8;
                }
                async_tx(&port, (char)ch);
            }
            else switch (ch)
            {
              case FK1:
                HelpFunc();
                break;
              case ALT_C:
                v_color = 7;
                cls();
                break;
              case ALT_X:
                return;
              case ALT_H:
                async_dtr(&port, 0);
                tdelay(9);
                async_dtr(&port, 1);
                break;
              case ALT_D:
                d_str("\nEnter number to dial (ESC to abort) :\n");
                if ((ch = prompt(buf, 32)) == 0)
                    break;
                sprintf(phone, "ATDT%s\r", buf);
                Dial();
                break;
              case ALT_R:
                if (*phone)
                    RepeatDial();
                break;
              case ALT_P:
                SetParams(NULL);
                break;
              case ALT_L:
                if (flog)
                {
                    fclose(flog);
                    flog = NULL;
                    d_str("\nTXZM.CAP file closed\n");
                }
                else
                {
                    if ((flog = fopen("TXZM.CAP", "ab")) != NULL)
                        d_str("\nTXZM.CAP file opened\n");
                    else
                        d_str("\nTXZM.CAP file open error\n");
                }
                break;
              case ALT_E:
                echo ^= 1;
                break;
              case ALT_S:
                pushscrn(0, 0, 25, 80);
                SETWND(0, 0, 24, 79);
                cls();
                d_str("Type EXIT and press ENTER to return to TXZM MiniTerm\n");
                spawnlp(P_WAIT, cmd, cmd, NULL);
                popscrn();
                SETWND(0, 0, 23, 79);
                break;
              case PGUP:
                d_str("\nEnter filenames to send (ESC to abort) :\n");
                if ((ch = prompt(fnames, 255)) == 0)
                    break;
                flist = ExpandSubDirs(fnames);
                TFlag.F.Receiving = 0;
                FileTransfer();
                break;
              case PGDN:
                TFlag.F.Receiving = 1;
                FileTransfer();
                rxtrip = 0;
                break;
              default:
                if (ch & 0xff)
                {
                    async_tx(&port, (char)ch);
                    if (echo)
                    {
                        d_ch((char)ch);
                        if (flog)
                            fputc(ch, flog);
                    }
                }
                break;
            }
        }
        else if (async_rxcnt(&port))
        {
            ch = async_rx(&port) & 0xff;
            d_ch((char)ch);
            if (ch != rxtripstr[rxtrip++])
                rxtrip = 0;
            if (flog)
                fputc(ch, flog);
            if (rxtrip == 6)
            {
                TFlag.F.Receiving = 1;
                FileTransfer();
                rxtrip = 0;
            }

        }
        else if (tryDV)
            DV_TimeSlice();
    }
}

/*/////////////////////////////////////////////////////////
//  ProcCmdLine -- process commnad line args             //
//:pro////////////////////////////////////////////////// */
void ProcCmdLine(int argc, char *argv[])
{
                        /*   COM1   COM2   COM3   COM4  */
    static int bases[4] = { 0x3f8, 0x2f8, 0x3e8, 0x2e8 };
    static int irqs[4]  = {  IRQ4,  IRQ3,  IRQ4,  IRQ3 };
    static int vctrs[4] = { VCTR4, VCTR3, VCTR4, VCTR3 };
    FILE *fh;
    int parg = 0, i, j;
    char *p1;

    for (i = 1; i < argc; i++)
    {
        p1 = strupr(argv[i]);
        if (*p1 == '-' || *p1 == '/')
            ++p1;
        if (parg == 0)
        {
            if (strncmp(argv[i], "COM", 3) == 0)
            {
                j = atoi(isdigit(*(p1 += 3)) ? p1 : argv[++i]);
                if (j < 1 && j > 4)
                    usage();
                --j, parg = 1;
                combase = bases[j], irq = irqs[j], vctr = vctrs[j];
                continue;
            }
        }
        switch (*p1)
        {
          case 'C':             /* custom comm port */
            if (!isxdigit(*++p1))
                p1 = argv[++i];
            for (combase = 0; isxdigit(*p1); ++p1)
            {
                combase = (combase << 4) | (*p1 - ((*p1 > '9' ) ?
                 ('A' - 0xA) : '0'));
            }
            irq = atoi(&p1[1]);
            if (combase < 0 || combase > 0xff8 || irq < 2 || irq > 7 || parg)
                usage();
            vctr = irq + 8;
            irq = 1 << irq;
            parg = 1;
          case 'B':             /* connect baud rate */
            strcpy(params, (isdigit(*++p1)) ? p1 : argv[++i]);
            break;
          case 'L':             /* locked baud rate */
            strcpy(lockedbaud, (isdigit(*++p1)) ? p1 : argv[++i]);
            break;
          case 'D':             /* disable FIFOs */
            openmask |= 0x4000;
            break;
          case 'M':             /* disable MSR interrupts */
            openmask |= 0x2000;
            break;
          case 'H':             /* enable hardware handshake */
            if (!isdigit(*++p1) || (j = atoi(p1)) < 0 || j > 3)
                j = 3;
            if (j & 1)
                cfg.MsrFlow = B_CTS;    /* sender monitors CTS */
            if (j & 2)
                cfg.MsrFlow |= B_RTS;   /* recvr drops RTS if rxbuf fills */
            break;
          case 'P':             /* send/accept complete pathnames */
            TFlag.F.FullPath = 1;
            break;
          case 'I':             /* ignore loss of carrier */
            TFlag.F.IgnCarrier = 1;
            break;
          case 'E':             /* option to take if file exists */
            if (isdigit(*++p1) && (j = atoi(p1)) >= 0 && j <= 3)
                TFlag.F.ExistOpts = j;
            break;
          case 'X':             /* escape control characters */
            if (!isdigit(*++p1) && !isdigit(*argv[i + 1]))
                TFlag.F.EscCtl = 1;
            else
            {
                if (!isdigit(*p1))
                    p1 = argv[++i];
                do {
                    j = atoi(p1);
                    if ((j & 0xff60) == 0)
                        ZTable[j] &= 0xf7;
                    else if (j < 0 && (-j & 0xff60) == 0)
                        ZTable[-j] |= 0x08;
                    p1 = SkipSpaces(SkipChars(p1));
                } while (*p1);
            }
            break;
          case 'V':             /* disable disk writes during serial I/O */
            OvlyIO = 0;
            break;
          case 'W':             /* maximum tx bytes in transit */
            TxWindow = atoi(isdigit(*++p1) ? p1 : argv[++i]);
            break;
          case 'N':             /* bbs use, node number follows */
            node = (*++p1) ? p1 : argv[++i];
            break;
          case '6':             /* 16 bit CRCs only */
            ZFR0 &= ~CANFC32;
            break;
          case '0':
            v_bios = 1;         /* use bios for video */
            break;
          case 'Q':             /* test for DesqView environment */
            tryDV = 1;
            break;
          case 'U':             /* miniterm mode */
            miniterm = goodargs = 1;
            break;
          case 'R':             /* receiving -- download path may follow */
            TFlag.F.Receiving = 1;
            if (*++p1 > ' ')
                DfltPath = p1;
            else if (++i < argc)
                DfltPath = argv[i];
            else
                DfltPath = cfg.DLPath;
            i = argc;
            ++goodargs;
            break;
          case 'S':             /* sending -- file names next args */
            TFlag.F.Receiving = 0;
            DfltPath = cfg.ULPath;
            if (*++p1 > ' ')
                argv[i--] = p1;
            if (*argv[i + 1] == '@')
            {
                if ((fh = fopen(&argv[++i][1], "rb")) == NULL)
                    break;
                j = (int)filelength(fileno(fh));
                if ((p1 = calloc(j + 1, 1)) == NULL)
                {
                    fclose(fh);
                    break;
                }
                fread(p1, 1, j, fh);
                fclose(fh);
                flist = ExpandSubDirs(p1);
                free(p1);
                i = argc;
                --goodargs;
            }
            else
            {
                while (++i < argc)
                {
                    strcat(fnames, " ");
                    strcat(fnames, argv[i]);
                    flist = ExpandSubDirs(fnames);
                }
                if (fnames[1])
                    --goodargs;
            }
            break;
        }
    }
}

/*/////////////////////////////////////////////////////////
//  Prompt - prompt for input string                     //
//:pro////////////////////////////////////////////////// */
int prompt(char *buf, int maxlen)
{
    char *p1 = buf;
    int ch = 0;

    while (ch != '\r' && ch != '\x1b')
    {
        if ((ch = (KBREAD & 0xff)) == '\r')
            continue;
        if (ch == '\b')
        {
            if (p1 > buf)
                d_ch((char)ch), --p1;
        }
        else if (p1 >= &buf[maxlen])
            d_ch('\a');
        else if (ch != '\x1b' && isprint(ch))
            d_ch((char)ch), *p1++ = ch;
    }
    d_ch('\n');
    if (ch == '\x1b')
        p1 = buf;
    *p1 = '\0';
    return (*buf);
}

/*/////////////////////////////////////////////////////////
//  RecurseSubDirs                                       //
//:rec////////////////////////////////////////////////// */
int RecurseSubDirs(char *sd)
{
    DF fbuf;
    static char subdir[_MAX_PATH];

    if (chdir(sd) != 0)
        return 0;
    getcwd(subdir, _MAX_PATH);
    AddToList(subdir);
    if (!DosFindFirst("*.*", _A_SUBDIR, &fbuf))
    {
        do {
            if (fbuf.attrib & _A_SUBDIR && fbuf.name[0] != '.')
                RecurseSubDirs(fbuf.name);
        } while (!DosFindNext(&fbuf));
    }
    chdir("..");
    return 1;
}

/*/////////////////////////////////////////////////////////
//  RepeatDial                                           //
//:rep////////////////////////////////////////////////// */
int RepeatDial(void)
{
    int rval, dials = 0, curpos = getcurloc();
    char buf[40];

    while ((rval = Dial()) != 0 && rval != X_ESC)
    {
        tdelay(36);
        while (async_rxcnt(&port))
            d_ch((char)async_rx(&port));
        sprintf(buf, "Dial Attempts = %d", ++dials);
        d_str(buf);
        setcurloc(curpos);
    }
    return (rval);
}

/*/////////////////////////////////////////////////////////
//  SetFIFOLevels - set FIFO levels                      //
//:set////////////////////////////////////////////////// */
void SetFIFOLevels(int rxFIFOlevel, int txFIFOlevel)
{
    if (port.Stat3 & B_FIFO)
    {
        async_FIFOrxlvl(&port, rxFIFOlevel);
        async_FIFOtxlvl(&port, txFIFOlevel);
    }
}

/*/////////////////////////////////////////////////////////
//  SetParams - set comport parameters                   //
//:set////////////////////////////////////////////////// */
int SetParams(char *newparams)
{
    char oldparams[14];

    if (newparams != NULL)
        strcpy(buf, newparams);
    else
    {
        d_str("\nEnter modem parameters (ESC to abort) :\n");
        if (prompt(buf, 9) == 0)
            return 0;
    }
    ConnectBaud = atol(buf);
    if (*lockedbaud == '\0')
    {
        if (*(strrchr(buf, '0') + 1) == '\0')
            strcat(buf, strrchr(params, '0') + 1);
        strupr(buf);
        if (async_setbpds(&port, buf) != 0) /* if no good */
        {
            ConnectBaud = atol(params);
            return 0;
        }
        strcpy(params, buf);
        LocBaud = ConnectBaud;
    }
    sprintf(buf,"\nModem Parameters: %s, ConnectBaud: %ld, LockedBaud: %ld\n",
      port.BPDSstr, ConnectBaud, (*lockedbaud) ? LocBaud : 0L);
    d_str(buf);
    return 1;
}

/*///////////////////////////////////////////////
//  Usage                                      //
//:usa//////////////////////////////////////// */
void usage()
{
    static char msg1[] = "\n\
\
TXZM -- Zmodem Protocol Driver 2.14\n\
 (c) 1991, Mike Dumdei, 6 Holly Lane, Texarkana Tx 75503\n\
\n\
This program is a demo of the MCOMM5 'C' serial communications library.\n\
It may be used free of charge for non-commercial purposes.  It is not\n\
public domain and may not be modified, sold, or distributed for a fee\n\
with the exception of normal and reasonable shareware distribution fees.\n\
\n\
If you have a need for a C communications library, the latest shareware\n\
version of MCOMM5 (freq = MCOMM) is available from:\n\
  North East Texas DataLink BBS, Texarkana TX  (903) 838-6713  1:19/128\n\
Features of MCOMM5 include:\n\
  Fully interrupt driven            16550 FIFO support\n\
  Up to 115200 baud                 Fast - 95%% ASM code\n\
  Library version: $25, Source included: $45\n\
\n\
<< TXZM USAGE: >>\n\
  recv> txzm { port -b# -l# -t# -d -m -h -i -p -q -x -v -e# } -r { directory }\n\
  send> txzm { port -b# -l# -t# -d -m -h -i -p -q -x -w#    } -s file1 file2\n\
  term> txzm { port -b# -l# -t# -d -m -h -i -p -q -x -w# -v -e# } -u\n\
Arguments in braces are optional.  If a baud rate is not specified, the\n\
current baud rate will be used.  The -r or -s switches must come last\n\
on the command line.\n\
Press any key for command line switch descriptions.....";

    static char msg2[] = "\n\
\n\
 port : set to COM1 (default), COM2, COM3, COM4, or -c#,#\n\
         -c#,# : base port address of comm port in hex,IRQ (2-7)\n\
         Ex: -c2E8,5    Addrs=2E8 IRQ=5  (no spaces in arg)\n\
  -b# : baud rate -- if using a fixed DTE link, CONNECT baud rate\n\
  -l# : baud rate of fixed DTE link (locked baud rate)\n\
  -t# : maximum characters to send to 16550 FIFOs per interrupt\n\
         1-16, default is 8  (some modems may require lower setting)\n\
  -d  : if 16550 UART detected do not enable FIFOs, default is to enable\n\
  -m  : disable modem status register interrupts\n\
  -h  : use RTS/CTS hardware handshake\n\
  -i  : ignore absence of carrier detect\n\
  -p  : send/accept full pathnames (will create subdirs)\n\
  -q  : DesqView mode (use DV video buffer, timeslice when idle)\n\
  -x  : zdle escape all control characters, -x#,.. to esc particular chars\n\
  -v  : disable serial I/O during disk writes\n\
  -e# : duplicate file handling options (default = 1)\n\
         0=skip file, 1=resume transfer, 2=create dup name, 3=overwrite\n\
  -w# : transmit window size (must be multiple of 128)\n\
  -u  : start up in mini-terminal mode\n\
  -r  : receive,  directory is optional download directory\n\
  -s  : send, file names follow - recurses subdirs of names in parenthensis\n\
txzm com2 -b2400 -h -e2 -r   (COM2, 2400 baud, CTS hndshk, dup name if exist)\n\
txzm -l38400 -b9600 -s *.zip (38400 locked rate, 9600 CONNECT, send all ZIPs)\n\
txzm -p -s (c:\\subdir)       (send all files in all subdirs in c:\\subdir)";

    cls();
    printf(msg1);
    KBREAD;
    printf(msg2);
    exit(-100);
}

/*///////////////////////////////////////////////
//  vDisplay                                   //
//:vdi//////////////////////////////////////// */
void vDisplay(int row, int col, char *format, ...)
{
    va_list arg_ptr;
    char lbuf[80];

    va_start(arg_ptr, format);
    vsprintf(lbuf, format, arg_ptr);
    d_msgat(row, col, txtcolor, lbuf);
}

/* ////////////////////////////////////////////////////////////////
//  waitfor -  waits for one of a list of up to 10 different     //
//   strings to come in on the serial port.  The list must end   //
//   with NULL.  The ticks argument is how long to wait and is   //
//   in 1/18s of a second.  Calls MCOMM serial and timer LIB     //
//   functions.  Returns the index of the first targeted string  //
//   found or one of the above error values.                     //
//:wai////////////////////////////////////////////////////////// */
int waitfor(int ticks, ...)
{
    struct { int len; char *str; } list[11], *wp;
    va_list argptr;
    char ch, *p1, *buf;
    long to;
    int i, j, rxcnt = 0, longest = 0;

     /* load 'list' array with target strings and their lengths */
    va_start(argptr, ticks);
    for (i = 0, wp = list; i < 10; ++i, ++wp)
    {
        if ((wp->str = va_arg(argptr, char *)) == NULL)
            break;
        if ((wp->len = strlen(wp->str)) == 0)
            return (i);
        if (wp->len > longest)
            longest = wp->len;
    }
    wp->str = NULL;

     /* allocate buffer for incoming chars, start timer */
    p1 = buf = malloc(longest);   /* buf size is length of longest target */
    set_timeout(&to, ticks);

     /* wait for target string or error */
    while (1)
    {
        if (async_rxcnt(&port))         /* if character available */
        {
            ch = (char)async_rx(&port);
            d_ch(ch);

            if (rxcnt < longest)            /* if "incoming" buf not full */
            {                                       /* load ch in buf,  */
                *p1 = ch;                           /* advance positon  */
                if (++rxcnt < longest)
                    ++p1;
            }
            else                            /* if "incoming" buf is full */
            {                                       /* shuffle left 1 pos, */
                memmove(buf, &buf[1], longest);     /* load ch in last pos */
                *p1 = ch;
            }
            for (wp = list; wp->str; ++wp)  /* check for found target */
            {                               /* j selects pos in buf to look */
                if ((j = rxcnt - wp->len) >= 0
                 && memicmp(wp->str, &buf[j], wp->len) == 0)
                {
                    free(buf);
                    return (wp - list);     /* return index if found */
                }
            }
        }
        else if (KBHIT && KBREAD == X_ESC)  /* check for local abort */
        {
            free(buf);
            return (X_ESC);
        }
        else if (timed_out(&to))            /* check for max time */
        {
            free(buf);
            return (TIMED_OUT);
        }                                   /* check for lost carrier */
        else if (checkcarrier && !async_carrier(&port)) /* see comment */
        {
            free(buf);
            return (LOST_CARRIER);
        }
    }
}

/*--------------------------------------------+
|  zmodem message types (defined in ZMDOS.H)  |
+---------------------------------------------/
#define M_RHDR      0       // received header
#define M_SHDR      1       // sent header
#define M_BLK0      2       // block 0 data processed (name, size, etc.)
#define M_CLASH     3       // file name clash occurred (use ExistOpts)
#define M_FILE      4       // start of transfer, FilePos = 1st position
#define M_EOF       5       // end of transfer (1 file)
#define M_DATA      6       // sent or received file data packet
#define M_FLOW      7       // change in XOFF or CTS flow status
#define M_IDLE      8       // waiting for character or for tx to empty
#define M_RESET     9       // reset to 'first file' condition */

/*/////////////////////////////////////////////////////////
//  ZMsg -- zmodem message handler                       //
//:zms///////////////////////////////////////////////:z: */
void ZMsg(int type, ...)
{
    static char *HdrType[] =
    {
        "Garbage Count", "Long Packet",   "Garbled Packet",   "Bad Crc",
        "Timed Out",     "Unknown Hdr",   "Sync Error",       "Memory Error",
        "File Error",    "Lost Carrier",  "Remote Abort",     "Local Abort",
        "ZRQINIT",       "ZRINIT",        "ZSINIT",           "ZACK",
        "ZFILE",         "ZSKIP",         "ZNAK",             "ZABORT",
        "ZFIN",          "ZRPOS",         "ZDATA",            "ZEOF",
        "ZFERR",         "ZCRC",          "ZCHALLENGE",       "ZCOMPL",
        "ZCAN",          "ZFREECNT",      "ZCOMMAND",         "ZSTDERR"
    };
    static char *HdrStyle[] = { "ZBIN", "ZHEX", "ZBIN32", "" };
    static char *CrcStyle[] = { "ZCRCE", "ZCRCG", "ZCRCQ", "ZCRCW" };
    static char MsgMask[] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };

    static long fStartTick, tStartTick, fCPS, fBytes, tBytes;
    static long fBarInc, tBarInc, fBarBytes, tBarBytes;
    static long fSize, fStartPos, updBytes, lastPos, orgTBytes;
    static long lastSec, lastTick, temp, flowhalts;
    static int tBarRow, fBarRow;
    static char gotBlock, crcType = ZCRCG, firstfile = 1;

    FILE *flog;
    va_list argptr;
    int i, j, row, hdr;
    long ticks;
    char *p1;

    if (MsgMask[type] == '\0')
        return;
    switch (type)
    {
      case M_RHDR:              /* received header */
      case M_SHDR:              /* sent header */
        row = (type == M_RHDR) ? 11 : 12;
        va_start(argptr, type);
        if ((hdr = va_arg(argptr, int)) < 0)
            ZHdr.Data = 0L;
        i = hdr + 12;
        j = va_arg(argptr, int) - 'A';
        if (j < 0 || j > 3)
            j = 3;
        vDisplay(row, 22, "%-17s%-9s%08lX%11ld", HdrType[i], HdrStyle[j],
         ZHdr.Data, ZHdr.Data);
        break;
      case M_CLASH:             /* file name clash occurred */
        if (TFlag.F.ExistOpts != 2)
            break;  /* not new name */
      case M_BLK0:              /* sent or received block 0 */
        i = sprintf(buf, "%-52s", PathName) - 52;
        d_msgat(3, 17, color[3], &(strupr(buf))[i]);
        if (type == M_CLASH)
            break;
        v_color = txtcolor;
        flowhalts = 0L;
        scrlupat(5, 22, 6, 32, 0);
        scrlupat(5, 58, 7, 68, 0);
        fSize = (TFlag.F.Receiving) ? RxdFileSize : FileSize;
        vDisplay(6, 58, "%-8ld", fSize);
        fStartTick = get_ticker();
        if (firstfile)
        {
            firstfile = tFiles = 0;
            tBytes = tBarBytes = 0L;
            lastSec = lastTick = tStartTick = fStartTick;
            tBarRow = 21;
            if ((tBarInc = TotalBytes / 19L) == 0)
                ++tBarInc;
            orgTBytes = TotalBytes;
            tCPS = (ConnectBaud * 955L ) / 10000L;
            vDisplay(17, 61, "%-8s",
             ConvertSecs((TotalBytes / tCPS) + (TotalFiles >> 2)));
        }
        if ((fBarInc = fSize / 19L) == 0)
            ++fBarInc;
        fBarBytes = 0L, fBarRow = 21;
        d_nchat(3, 73, '±', color[4], 19, 0);
        if (orgTBytes != 0L)
        {
            j = 3 + (int)((orgTBytes & 0xffff0000L) ?
             (TotalBytes / (orgTBytes / 19L)) :
             ((TotalBytes * 19L) / orgTBytes));
            while (j < tBarRow && tBarRow > 2)
                d_msgat(tBarRow--, 75, color[6], "±");
            vDisplay(17, 26, "%-4d", TotalFiles - 1);
            vDisplay(18, 26, "%-8ld", TotalBytes - fSize);
        }
        else
        {
            tBarInc = fBarInc, tBarBytes = 0L, tBarRow = 21;
            d_nchat(3, 75, '±', color[4], 19, 0);
        }
        break;
      case M_EOF:               /* end of file transfer */
        va_start(argptr, type);
        i = va_arg(argptr, int);
        if (i == ZEOF || i == ZRINIT)
        {
            vDisplay(20, 26, "%d", ++tFiles);
            tfBytes = tBytes;
            if (((p1 = getenv("TXZMLOG")) != NULL)
             && ((flog = fopen(p1, "at")) != NULL))
            {
                sprintf(buf,
                 "%c %6ld %5ld bps %4ld cps %3u errors %5ld %4d %s %d\n",
                 TFlag.F.Receiving ? 'Z' : 'z', FilePos, ConnectBaud, fCPS,
                 ErrCnt, flowhalts, BlkLen, NameExt, (SerNbr) ? SerNbr : -1);
                fputs(buf, flog);
                fclose(flog);
            }
        }
        if (orgTBytes != 0L)
        {
            temp = TotalBytes - fSize;
            j = 3 + (int)((orgTBytes & 0xffff0000L) ?
             (temp / (orgTBytes / 19L)) :
             ((temp * 19L) / orgTBytes));
            while (j < tBarRow && tBarRow > 2)
                d_msgat(tBarRow--, 75, color[6], "±");
        }
        crcType = ZCRCG;
        break;
      case M_FILE:              /* start of transfer, 1st FilePos set */
        lastPos = fStartPos = FilePos;
        fBytes = 0L;
        vDisplay(5, 22, "%-8s", ConvertSecs((fSize - fStartPos) / tCPS));
        vDisplay(7, 58, "%-8ld", fStartPos);
        if (fStartPos == 0L)
            break;
        updBytes = fStartPos;
      case M_DATA:              /* sent or received file data packet */
        if (type != M_FILE)
        {
            gotBlock = 1;
            updBytes = FilePos - lastPos;
            lastPos = FilePos;
            fBytes += updBytes, tBytes += updBytes;
            vDisplay(5, 58, "%-8ld", FilePos);
            vDisplay(21, 26, "%-8ld", tBytes);
            va_start(argptr, type);
            vDisplay(14, 11, "%s-%s", (BinHdr == ZBIN32) ? "32" : "16",
             CrcStyle[(crcType = (char)va_arg(argptr, int)) - ZCRCE]);
        }
        fBarBytes += updBytes;
        tBarBytes += updBytes;
        while (fBarBytes >= fBarInc && fBarRow > 2)
            fBarBytes -= fBarInc, d_msgat(fBarRow--, 73, color[5], "±");
        while (tBarBytes >= tBarInc && tBarRow > 2)
            tBarBytes -= tBarInc, d_msgat(tBarRow--, 75, color[6], "±");
        break;
      case M_FLOW:
        va_start(argptr, type);
        switch (i = va_arg(argptr, int))
        {
          case 0:           /* XOFF cleared (XON received) */
          case 1:           /* XOFF received */
            d_msgat(14, 44, color[3], (i) ? "XOFF" : "    ");
            break;
          case 2:           /* CTS signal raised */
          case 3:           /* CTS signal lowered */
            d_msgat(14, 49, color[3], (i == 3) ? "CTS" : "   ");
            break;
          case 4:           /* port error */
            d_msgat(14, 55, color[3]|BLNK, "PORT RESET");
            tdelay(18);
            d_msgat(14, 55, color[3], "          ");
        }
        flowhalts += (i & 1);
        break;
      case M_IDLE:
        if (firstfile || (ticks = get_ticker()) == fStartTick)
            break;
        if (gotBlock || (crcType == ZCRCE && ticks != lastTick))
        {
            gotBlock = 0;
            lastTick = ticks;
            if (!(fBytes & 0xfff00000L))
                fCPS = ((fBytes * 182L) / (ticks - fStartTick)) / 10L;
            else
                fCPS = fBytes / (((ticks - fStartTick) * 10L) / 182L);
            if (!(tBytes & 0xfff00000L))
                tCPS = ((tBytes * 182L) / (ticks - tStartTick)) / 10L;
            else
                tCPS = tBytes / (((ticks - tStartTick) * 10L) / 182L);
            vDisplay(7, 22, "%-6ld", fCPS);
            vDisplay(21, 61, "%-6ld", tCPS);
        }
        if (ticks > (lastSec + 18))
        {
            lastSec = ticks;
            ConvertTicks(ticks - fStartTick);
            d_msgat(6, 22, txtcolor, minsecs);
            ConvertTicks(ticks - tStartTick);
            d_msgat(18, 61, txtcolor, minsecs);
        }
        if (tryDV)
            DV_TimeSlice();
        break;
      case M_RESET:
        if (cfg.MsrFlow && !async_cts(&port))
            d_msgat(14, 49, color[3], "CTS");
        crcType = ZCRCG, firstfile = 1;
        break;
    }
}


