
/** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*      S M A L T E R M  --  D e m o   D r i v e r   f o r   M C o m m 5     *
*                          A s y n c   R o u t i n e s                      *
*                                                                           *
*              Mike Dumdei, 6 Holly Lane, Texarkana TX 75503                *
*                 North East Texas DataLink, 903 838-6713                   *
*                                                                           *
*               (Link with comm_s.lib for external functions)               *
*               cl smalterm.c /link comm_s          (Microsoft C)           *
*               tcc smalterm.c comm_s.lib           (Turbo C)               *
*               ztc -b smalterm.c comm_s.lib        (Zortech C)             *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
#include <stdio.h>
#include <fcntl.h>
#if !defined(__TURBOC__)
    #include <sys\types.h>
#endif
#include <sys\stat.h>
#include <io.h>
#include <stdlib.h>
#include <conio.h>
#include <ctype.h>
#include <string.h>
#include <process.h>
#include <stdarg.h>
#if defined(__ZTC__)
  #define   MSDOS           1
  #define   stricmp         strcmpl
#endif
#include <dos.h>
#include <bios.h>

#include "comm.h"                            /* header for async functions */
#include "ansidrv.h"                   /* header for ANSI display routines */
#define KEY_INT
#include "keys.h"                                 /* key definition header */
#if defined(RED)
    #undef RED
#endif
#include "colors.h"                  /* vid mode & color definition header */
#include "extra.h"                               /* extra functions header */

#if defined (__TURBOC__)                                        /* Turbo C */
  #define   NO_FILES(x)     findfirst((x), &findbuf, 0)
  #define   find_t          ffblk
  #define   KBHIT           bioskey(1)
  #define   KBREAD          bioskey(0)
  #include  <dir.h>
#else                                            /* Microsoft C, Zortech C */
  #include  <direct.h>
  #define   NO_FILES(x)     _dos_findfirst((x), _A_NORMAL, &findbuf)
  #define   KBHIT           _bios_keybrd(_KEYBRD_READY)
  #define   KBREAD          _bios_keybrd(_KEYBRD_READ)
#endif

#define POP         0
#define PUSH        1              /* used in screen save/restore function */
#define RXBUFSZ     4096                            /* rx ring buffer size */
#define TXBUFSZ     1050                            /* tx ring buffer size */
#define LOST_CARRIER -1
#define NO_INPUT     -1
#define ESC_PRESSED  -2
#define TIMED_OUT    -3

#define TRUE          1
#define FALSE         0
#define REG     register

typedef unsigned char uchar;
typedef unsigned long ulong;
typedef unsigned int uint;


/*      f u n c t i o n   d e c l a r a t i o n s     */
extern int main(int argc,char * *argv);
static int proc_keypress(int ch);
static int proc_rxch(int ch);
static int proc_fkey(int keycd);
static int watch_cd(void );
static int rcls(void);
static int toggle_log(void );
static int toggle_echo(void );
static int toggle_lfs(void );
static int toggle_dtr(void );
static int dial_nbr(void );
static int shell_to_dos(void );
static int host_mode(void );
static int shell_remote(void );
static int run_batch(int keycd);
static int exit_term(void );
static int do_help(void );
static int async_tx_str(char *str);
static int async_tx_echo(char *str);
static int screen_pushpop(int flag);
static int chg_parms(void );
static int rx_timeout(int Tenths);
static int waitfor(char *str, int ticks);
static int waitforOK(int ticks );
static int prompt(char *kbuf,char *promptmsg,int maxchars);
static int comprompt(char *kbuf,char *promptmsg,int maxchars);
static int hang_up(void );

/*      g l o b a l   v a r i a b l e s     */

ASYNC   *port;                               /* pointer to async structure */
 /* default to COM1 */
char    com_str[5] = "COM1";           /* text for port ("COM1" or "COM2") */
int     IOadrs = 0x3f8;                      /* comm port base I/O address */
char    irqm = IRQ4;                         /* mask for selected IRQ line */
char    vctrn = VCTR4;                        /* comm interrupt vector nbr */

FILE    *logfil = NULL;                                 /* log file handle */
char    dialstr[40];                           /* dial string storage area */
int     ChkgKbd = 1;              /* watch for ESC during rcv with timeout */
int     ChkgCarrier = 0;        /* monitor carrier during rcv with timeout */
char    buf[200];                                    /* gen purpose buffer */
int     lfs = 0, echo = 0, dtr = 1, autoans = 0;          /* various flags */
struct find_t findbuf;                        /* struct for NO_FILES macro */
int     cyn, hred, grn, hgrn, wht, hblu;           /* text color variables */
int     hostcnt = 0;                        /* toggle to host mode counter */

/*
            * * * *    M A I N    * * * *
*/
main(int argc, char **argv)
{
    static char params[10] = "1200N81";              /* default parameters */
    char        buf[100];
    int         got_port = FALSE;
    unsigned    i, ch;

    initvid();                                /* initialize video routines */
    if (v_mode == CO80)
    {
        cyn = CYN, hred = H_RED, grn = GRN, hgrn = H_GRN,
        wht = WHT, hblu = H_BLU;
    }
    else
    {
        cyn = WHT, hred = H_WHT, grn = WHT, hgrn = H_WHT,
        wht = WHT, hblu = H_WHT;
    }
    v_color = cyn;
    cls();                                             /* clear the screen */

    /*  D I S P L A Y   S I G N O N  */
    v_color = hred;
    d_str("SMALTERM - Sample driver for MCOMM5 comm routines\n");
    v_color = grn;
    d_str("    Mike Dumdei, 6 Holly Lane, Texarkana TX 75503\n");
    d_str("    North East Texas DataLink   903 838-6713\n");
    v_color = hred;
    d_str("Usage: smalterm {COM1 | COM2} {param str EX: 2400N81}\n\n");
    v_color = cyn;

    /*  P R O C E S S   C O M M A N D   L I N E   A R G S  */
    for (i = 1; i < 3; i++)        /* do it twice so args can be any order */
    {                              /*  or have one arg and not the other   */
        if (argc > i)
        {
            if (!got_port && !stricmp(argv[i], "COM1"))
                got_port = TRUE;
            else if (!got_port && !stricmp(argv[i], "COM2"))
            {
                got_port = TRUE, strcpy(com_str, "COM2");
                IOadrs = 0x2f8, irqm = IRQ3, vctrn = VCTR3;
            }
            else if (strlen(argv[i]) < 9)   /* assume a valid param string */
                strcpy(params, argv[i]);
        }
    }

    /*  O P E N   T H E   C O M M   P O R T  */
    port = malloc(sizeof(ASYNC));
    AllocRingBuffer(port, RXBUFSZ, TXBUFSZ, 1);    /* alloc FAR ring bufrs */
    if ((ch = async_open(port, IOadrs, irqm, vctrn, params)) != 0)
    {
        /* couldn't open the port code */
        strsum(buf, "SMALTERM: Couldn't open ", com_str, " using ",
              params, " for parameters -- ABORTING\n\n", NULL);
        d_str(buf);
        strsum(buf, "Error code = ", itoa(ch, &buf[50], 10), "\n\n", NULL);
        d_str(buf);
        exit(ch);
    }

    /* opened OK code */

    d_str(strsum(buf, "SMALTERM: ", com_str, " -- PORT OPENED\n\n", NULL));
    d_str("Press F1 for HELP screen\n");           /* tell how to get help */
    /* display status line */
    SETWND(0, 0, 23, 79);                           /* set the window size */
    d_strnat(24, 0, 'Í', cyn, 80);                          /* paint a bar */
    d_msgat(24, 60, hgrn, strupr(com_str));                /* display port */
    d_msgat(24, 65, hgrn, strupr(params));               /* display params */
    d_msgat(24, 10, hgrn, "DTR");                           /* DTR is high */
    *dialstr = '\0';                              /* clear out dial string */
    tickhookset(1);        /* enable 'ticker' variable for timer functions */
    if (!async_carrier(port))
    {
        async_tx_str("ATE1V1\r");                       /* reset the modem */
        waitforOK(9);
    }
    /*
        T H I S   I S   T H E   M A I N   I N G R E D I E N T
    */
    while (1)
    {
        /* turn off auto answer if enabled by Host mode */
        if (autoans && !async_carrier(port))
        {
            DELAY_TENTHS(2);
            async_tx_str("ATS0=0\r");
            waitforOK(9);
            autoans = 0;
        }
         /* check the keyboard */
        if (KBHIT)
            proc_keypress(KBREAD);
         /* check the serial port */
        if (async_rxcnt(port))
            proc_rxch(async_rx(port));

    }
}

int proc_rxch(int ch)
{
    REG int ch2;

    ch2 = ch & 0xff;

    if (ch2 == '\r' && lfs)
        ch2 = '\n';              /* translate to nl if supplying lin feeds */
    if (ch2 != '~')
        hostcnt = 0;
    else
    {
        if (++hostcnt >= 10)               /* ten ~'s puts it in host mode */
        {
            host_mode();
            hostcnt = 0;
            return (ch);
        }
    }
    d_ch((char)ch2);     /* dsply if got one -- 'd_ch' supports ANSI codes */
    if (logfil && !v_ansiseq)
        fputc(ch2, logfil);               /* write char to logfile if open */
    return (ch);
}

/*
        p r o c e s s   k e y p r e s s
*/
int proc_keypress(REG int ch)
{
    REG int ch2;

    if ((ch2 = ch & 0xff) == 0)
        proc_fkey(ch);                  /* process if extended key pressed */
    else
    {
    async_tx(port, (char)ch2);                          /* send ch to port */
        if (echo && ch != X_ESC)
        {
            if (ch2 == '\r' && lfs)
                ch2 = '\n';                  /* make it a nl if adding lfs */
        d_ch((char)ch2);                               /* display the char */
            if (logfil && !v_ansiseq)
                fputc(ch2, logfil);            /* write to logfile if open */
        }
    }
    return (ch);
}

/*
        p r o c e s s   e x t e n d e d   k e y s
*/
int     proc_fkey(REG int keycd)
{
    static struct
    {
    int jval;
    int (*fun)();
    } *f, funtbl[] = {
        { ALT_C, rcls },            { ALT_E, toggle_echo },
        { ALT_X, exit_term },       { ALT_S, shell_to_dos },
        { ALT_D, dial_nbr },        { PGUP, run_batch },
        { PGDN, run_batch },        { ALT_H, hang_up },
        { ALT_L, toggle_lfs},       { F2, toggle_log },
        { F10, host_mode },         { ALT_P, chg_parms },
        { F9, watch_cd },    { 0, NULL }             };

    if (keycd == F1)
        keycd = do_help();         /* do a help screen first if F1 pressed */
    for (f = funtbl; f->jval != keycd && f->jval; f++)
        ;                                       /* lookup function for key */
    if (f->jval)
        return ((f->fun)(keycd));/* execute it if found & return exit code */
    else
        return (0);                      /* else do nothing and return a 0 */
}

/*
        w a t c h   c a r r i e r  d e t e c t
*/
int     watch_cd(void)
{
    return(watchdogset(1, port->ComBase));
}

/*
        r e s e t   c o l o r,   c l e a r   s c r e e n
*/
int     rcls(void)
{
    v_color = cyn;
    cls();
    return 0;
}

/*
        t o g g l e   l o g   f i l e
*/
int     toggle_log(void)
{
    if (logfil == NULL)
    {
        if (NULL != (logfil = fopen("smalterm.log", "ab")))
            d_msgat(24, 25, hgrn, "LOG");              /* LOG file is open */
    }
    else
    {
        fclose(logfil);
        logfil = NULL;
        d_msgat(24, 25, cyn, "ÍÍÍ");                   /* LOG file is open */
    }
    return 0;
}

/*
        t o g g l e   e c h o    f u n c t i o n
*/
int     toggle_echo(void)
{
    if (echo ^= 1)                                          /* toggle flag */
        d_msgat(24, 15, hgrn, "ECHO");                       /* ECHO is on */
    else
        d_msgat(24, 15, cyn, "ÍÍÍÍ");                       /* ECHO is off */
    return 0;
}

/*
        t o g g l e   l f ' s    f u n c t i o n
*/
int     toggle_lfs(void)
{
    if (lfs ^= 1)                                           /* toggle flag */
        d_msgat(24, 21, hgrn , "LF");                       /* LF's are on */
    else
        d_msgat(24, 21, cyn, "ÍÍ");                        /* LF's are off */
    return 0;
}

/*
        t o g g l e   d t r    f u n c t i o n
*/
int     toggle_dtr(void)
{
    async_dtr(port, (dtr ^= 1));                             /* toggle DTR */
    if (dtr)
        d_msgat(24, 10, hgrn, "DTR");                       /* DTR is high */
    else
        d_msgat(24, 10, (hred | BLNK), "DTR");               /* DTR is low */
    return 0;
}

/*
        d i a l   a   n u m b e r
*/
int     dial_nbr(void)
{
    char    lbuf[25];
    static char dmsg[] = "Enter number (proceed with P if pulse dial) ->";

    /* input a phone number */
    if (prompt(lbuf, dmsg, 24))                        /* input the number */
        return 0;
    if (*lbuf == 'p' || *lbuf == 'P')
        strsum(dialstr, "ATDP", &lbuf[1], "\r", NULL);
    else                                          /* build the dial string */
        strsum(dialstr, "ATDT", lbuf, "\r", NULL);
    async_tx_str(dialstr);                              /* dial the number */
    return 0;
}

/*
        s h e l l   t o   D O S   f u n c t i o n
*/
int     shell_to_dos(void)
{
    char    savdcolr, lbuf[2];
    char    cmdcom[60];

    if (screen_pushpop(PUSH))                       /* save current screen */
    {
        cls();
        savdcolr = v_color, v_color = hred;
        *cmdcom = '\0';
        strcpy(cmdcom, getenv("COMSPEC"));
        if (!*cmdcom)
            strcpy(cmdcom, "COMMAND.COM");
        d_str("Type EXIT to return to SMALTERM."); /* tell how to get back */
        v_color = savdcolr;
        spawnlp(P_WAIT, cmdcom, cmdcom, NULL);        /* spawn COMMAND.COM */
        screen_pushpop(POP);     /* fix up screen & ANSI sequence variable */
    }
    else
        prompt(lbuf, "Not enough memory, Press ENTER to continue\a", 0);
    screen_pushpop(POP);         /* fix up screen & ANSI sequence variable */
    return 0;
}

/*
        h o s t   m o d e
*/
int     host_mode(void)
{
    long    rto;
    char    i, lbuf[62], ch = '\0';
    int     savdcolr;

    static char menu[] =
"\r\n\r\nU)pload    D)ownload    S)hell to DOS    P)age operator    H)ang up\
\r\n      Enter selection ---> ";

    static char zmrcvmsg[] =
"u\r\n\r\nReady to receive files using Zmodem, start your upload\r\n";

    static char zmsndmsg1[] =
"d\r\n\r\nEnter the filename(s) you wish to download at the prompt, one file\r\n\
per prompt.  Wild cards are accepted.  Press ENTER when all files have\r\n\
been entered or press ESC to abort the transfer.\r\n";

    static char zmsndmsg2[] =
"\r\nReady to transmit files using Zmodem, start your download\r\n";

    static char pagemsg[] =
"p\r\n\r\nPaging remote, press ESC if no answer\r\n";
    
    d_str("\nHost mode activated, Press ESC to exit Host,\n");
    d_str("Press the space bar to chat\n");
    async_txflush(port);
    async_rxflush(port);                             /* clean out the port */
    if (lfs)
        toggle_lfs();
    if (echo)
        toggle_echo();
    while (ch != ESC && ch != ' ')
    {
        if (!async_carrier(port))
        {
            DELAY_TENTHS(2);
            async_tx_str("ATS0=1\r");         /* set modem for auto answer */
            waitforOK(9);                             /* throw away the OK */
            autoans = 1;       /* set flag showing AA enabled by Host mode */
            d_str("\r\nAwaiting caller\r\n");
            while (1)         /* loop till carrier detected or ESC pressed */
            {
                if (KBHIT && ((ch = KBREAD) == ESC))
                    break;    
                if (async_carrier(port))
                {
                    DELAY_TENTHS(5);/* half sec delay to let things settle */
                    async_rxflush(port);             /* clean out the port */
                    async_tx_echo("Welcome to Smalterm Host\r\n");
                    break;
                }
            }
        }
        if (async_carrier(port))
            async_tx_echo(menu);
        while (async_carrier(port) && ch != ESC && ch != ' ')
        {
            if (KBHIT)
            {
                switch (ch = KBREAD)
                {
                  case ESC:
                    hang_up();                        /* hang up on caller */
                    d_str("\r\nCaller aborted");
                    continue;                         /* break out of loop */
                  case ' ':
                    async_tx_echo("\r\nEntering chat mode\r\nHello\r\n");
                    toggle_lfs();
                    toggle_echo();
                    continue;
                }
            }
            switch (async_rx(port) & 0xff)
            {
              case 'u':
              case 'U':
                async_tx_echo(zmrcvmsg);
                while (!async_stat(port, B_TXEMPTY | B_CD))
                    ;
                DELAY_TENTHS(2);
                if (!async_carrier(port))
                    break;
                if (screen_pushpop(PUSH))           /* save current screen */
                {
                    cls();
                    savdcolr = v_color, v_color = hred;
                    d_str("Remote is uploading ...");
                    v_color = savdcolr;
                    system("RCV.BAT");                /* receive the files */
                }
                screen_pushpop(POP);                 /* restore the screen */
                DELAY_TENTHS(2);
                break;
              case 'd':
              case 'D':
                async_tx_echo(zmsndmsg1);
                while (!async_stat(port, B_TXEMPTY | B_CD))
                    ;
                strcpy(buf, "SND.BAT");
                i = '0';
                while(++i < '9' && async_carrier(port))
                {
                    strcpy(lbuf, "\r\nEnter filename #  : ");
                    lbuf[18] = i;
                    comprompt(lbuf, lbuf, 60);
                    if (!*lbuf || *lbuf == ESC)
                        break;
                    if (NO_FILES(lbuf))
                    {
                        async_tx_echo("\r\nFile not found");
                        --i;
                        continue;
                    }
                    if (strlen(buf) + strlen(lbuf) > 125)
                    {
                        async_tx_echo("\r\nFile buffer full");
                        continue;
                    }
                    strcat(buf, " ");
                    strcat(buf, lbuf);
                }
                if (*lbuf == ESC || !async_carrier(port))
                    break;
                async_tx_echo(zmsndmsg2);
                while (!async_stat(port, B_TXEMPTY | B_CD))
                    ;
                DELAY_TENTHS(2);
                if (screen_pushpop(PUSH))           /* save current screen */
                {
                    cls();
                    savdcolr = v_color, v_color = hred;
                    d_str("Remote is downloading ...");
                    v_color = savdcolr;
                    system(buf);                         /* send the files */
                }
                screen_pushpop(POP);                 /* restore the screen */
                break;
              case 's':
              case 'S':
                async_tx_echo("s\r\n");
                watchdogset(1, port->ComBase);          /* enable watchdog */
                shell_remote();                         /* do remote shell */
                watchdogset(0, 0);                     /* disable watchdog */
                break;
              case 'p':
              case 'P':
                ch = '\0';
                async_tx_echo(pagemsg);
                d_str("\a\a\a");
                SET_TO_SECS(rto, 15);
                while (async_carrier(port) && async_rx(port) != ESC
                  && (!KBHIT || (ch = KBREAD) != ' '))
                {
                    if (timed_out(&rto))
                    {
                        d_str("\a\a\a");
                        SET_TO_SECS(rto, 15);
                    }
                }
                if (ch == ' ')
                {
                    async_tx_echo("\r\nEntering chat mode\r\nHello\r\n");
                    toggle_lfs();
                    toggle_echo();
                    continue;
                }
                break;
              case 'h':
              case 'H':
                async_tx_echo(
                  "h\r\nSmalterm Host going offline ...\r\n");
                while (!async_stat(port, B_TXEMPTY))
                    ;
                hang_up();
                d_str("\r\nCaller offline");
                continue;                             /* break out of loop */
              default:
                continue;                                /* skips the menu */
            }
            async_tx_echo(menu);
        }
    }
    return 0;
}


/*
        r e m o t e   s h e l l   t o   D O S   f u n c t i o n
*/
int     shell_remote(void)
{
    char    savdcolr, lbuf[2];
    char    cmdcom[60];
    int      x, inhdl, outhdl, errhdl;

    if (screen_pushpop(PUSH))                       /* save current screen */
    {
        cls();
        savdcolr = v_color, v_color = hred;
        *cmdcom = '\0';
        strcpy(cmdcom, getenv("COMSPEC"));
        if (!*cmdcom)
            strcpy(cmdcom, "COMMAND.COM");
        d_str("Executing remote shell to DOS ...");
        v_color = savdcolr;
        async_txflush(port);
        async_tx_str("\r\nShelling to DOS, Type EXIT when finished.\r\n");
        while (!async_stat(port, B_TXEMPTY))
            ;                             /* wait for message to get there */
        DELAY_TENTHS(1);                         /* wait a little bit more */
        async_stop(port);                       /* let go of the comm port */

        /* make stdin, stdout, & stderr refer to com port */
        x = open(com_str, O_RDWR | O_BINARY);
        inhdl = dup(0);
        outhdl = dup(1);
        errhdl = dup(2);
        dup2(x, 0);
        dup2(x, 1);
        dup2(x, 2);

        spawnlp(P_WAIT, cmdcom, cmdcom, NULL);        /* spawn COMMAND.COM */

        /* restore stdin, stdout, & stderr */
        close(x);
        dup2(inhdl, 0);
        close(inhdl);
        dup2(outhdl, 1);
        close(outhdl);
        dup2(errhdl, 2);
        close(errhdl);

        async_restart(port);                      /* re-init the comm port */
    }
    else
        prompt(lbuf, "Not enough memory, Press ENTER to continue\a", 0);
    screen_pushpop(POP);         /* fix up screen & ANSI sequence variable */
    return 0;
}

/*
        r u n   b a t c h   f i l e
*/
int     run_batch(int keycd)
{
    REG     char *p1;
    char    savdcolr;
    char    b_buf[60];

    /* get batch name and parameters */
    p1 = b_buf;
    strcpy (p1, (keycd == PGUP) ? "SND.BAT " : "RCV.BAT ");
    p1 += strlen(p1);
    if (prompt(p1, "Enter batch parameters ->", 50) == ESC_PRESSED)
        return 0;
    if (screen_pushpop(PUSH))                       /* save current screen */
    {
        cls();
        savdcolr = v_color, v_color = hred;
        d_str(strsum(buf, "Executing: ", b_buf, "\n\n", NULL));
        v_color = savdcolr;
        system(b_buf);                               /* run the batch file */
    }
    else
        prompt(b_buf, "Not enough memory, Press ENTER to continue\a", 0);
    screen_pushpop(POP);         /* fix up screen & ANSI sequence variable */
    d_str("\a\a\a\a");                                  /* signal finished */
    return 0;
}

/*
        e x i t   f o r   g o o d   f u n c t i o n
*/
int     exit_term(void)
{
    char    lbuf[2];

    switch (prompt(lbuf, "Exit SmalTerm (Y, n)?", 1))
    {
      case ESC_PRESSED:
        return 0;
      case NO_INPUT:
        break;
      default:
        if (*lbuf != 'Y' && *lbuf != 'y')
            return 0;
    }
    tickhookset(0);                    /* disable the TIMER interrupt hook */
    async_close(port);                                   /* close the port */
    SETWND(0, 0, 24, 79);
    cls();                                             /* clear the screen */
    exit (0);                                               /* back to DOS */
}


/*
        h e l p    f u n c t i o n
*/
int     do_help()
{
    /* not the best way to do video but shows ANSI  */
    /*  capabilities of ANSI8MSC display functions  */
    static char *help_scrn[] =
    {
"\33[1;1H\33[0;1;37;40mÚÄÄ   \33[31mSmalTerm HELP  [F1]   \33[37mÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿\n",
"³\33[60C³\n",
"³  \33[0;32mALT-C:  \33[36mClear the screen\33[8C\33[32mPGUP :  \33[36mExecute SND.BAT   \33[1;37m³\n",
"³  \33[0;32mALT-D:  \33[36mDial a number\33[11C\33[32mPGDN :  \33[36mExecute RCV.BAT   \33[1;37m³\n",
"³  \33[0;32mALT-E:  \33[36mToggle echo\33[15C\33[32mF2 :  \33[36mToggle log file   \33[1;37m³\n",
"³  \33[0;32mALT-H:  \33[36mHang Up\33[19C\33[32mF9 :  \33[36mEnable watchdog   \33[1;37m³\n",
"³  \33[0;32mALT-L:  \33[36mToggle line feeds\33[8C\33[32mF10 :  \33[36mEnable host mode  \33[1;37m³\n",
"³  \33[0;32mALT-P:  \33[36mChange parameters\33[33C\33[1;37m³\n",
"³  \33[0;32mALT-S:  \33[36mShell to DOS\33[12C\33[32mALT-X:  \33[36mExit to DOS\33[7C\33[1;37m³\n",
"ÃÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ´\n",
"³  \33[0;37mDemo comm program written using MCOMM5 async routines,    \33[1;37m³\n",
"³  \33[0;37mANSI_xxC video routines, and EXTRA functions.\33[13C\33[1;37m³\n",
"³   \33[0;37mMike Dumdei, 6 Holly Lane, Texarkana TX 75503\33[12C\33[1;37m³\n",
"³   \33[0;37mNorth East Texas DataLink  --  (903) 838-6713\33[12C\33[1;37m³\n",
"ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ\33[0;36m",
NULL
    };
    register char **p1;
    char    *lbuf = 0;
    int     rval = 0, x;

    x = v_ansiseq, v_ansiseq = 0;    /* in case an ANSI seq is in progress */
    if ((lbuf = malloc(SCRBUF(15, 62))) != NULL)/* malloc buf to save scrn */
    {
        pu_scrnd(3, 9, 15, 62, lbuf);            /* push area used by HELP */
        SETWND(3, 9, 17, 70);
        cls();
        v_scrlm = 0;   /* don't want to scroll when at the btm of the wind */
    }
    for (p1 = help_scrn; *p1; p1++)  /* display the ANSI style help screen */
        d_str(*p1);
    rval = KBREAD;                                   /* return key pressed */
    if (lbuf)
    {
        SETWND(0, 0, 23, 79);                     /* reset the window size */
        v_scrlm = 1;                              /* reset the scroll mode */
        po_scrnd(lbuf);                                  /* restore screen */
        free(lbuf);                                      /* release memory */
    }
    v_ansiseq = x;                                  /* restore ANSI status */
    return (rval);               /* return extended key if pressed, else 0 */
}

/*
        a s y n c _ t x _ s t r   f u n c t i o n
*/
int     async_tx_str(char *str)
{
    return(async_txblk(port, str, strlen(str)));
}

/*
        a s y n c _ t x _ e c h o   f u n c t i o n
*/
int     async_tx_echo(register char *str)
{

    while (*str && async_carrier(port))
    {
        d_ch(*str);
        async_tx(port, *str++);
        while (!async_stat(port, B_TXEMPTY | B_CD))
            ;
    }
    return 0;
}


/*
        s c r e e n   s a v e / r e s t o r e   f u n c t i o n
*/
int     screen_pushpop(int flag)
{
    static char *savscrn = NULL;
    static ulong   savdwndsz;
    static int     x;

    if (flag == PUSH)
    {
        x = v_ansiseq, v_ansiseq = 0;/* in case an ANSI seq is in progress */
    if ((savscrn = malloc(SCRBUF(25, 80))) != NULL)
        {
            savdwndsz = v_wndsz;
            SETWND(0, 0, 24, 79);         /* reset window to entire screen */
            pu_scrnd(0, 0, 25, 80, savscrn);/* save current screen in bufr */
            return (1);                                  /* return success */
        }
        else
            return (0);                                   /* return failed */
    }
    else
    {
        v_ansiseq = x;                              /* restore ANSI status */
        if (savscrn != NULL)
        {
            po_scrnd(savscrn);                /* restore the pushed screen */
            free(savscrn);                 /* release screen buffer memory */
            savscrn = NULL;
            v_wndsz = savdwndsz;                  /* reset the screen size */
            return (1);                                  /* return success */
        }
        else
            return (0);                 /* return failed if nothing pushed */
    }
}

/*
        c h a n g e   p a r a m e t e r s
*/
int     chg_parms(void)
{
    char    lbuf[12];

    /* get new parmameters */
    if (prompt(lbuf, "Enter parameters (Ex: 1200N81) ->", 10))
        return 0;
    if (async_setbpds(port, lbuf) == 0)               /* change parameters */
    {
        d_nchat(24, 65, 'Í', cyn, 10, HORZ);
        d_msgat(24, 65, hgrn, strupr(lbuf));             /* display params */
    }
    else
        prompt(lbuf, "Invalid parameters, Press ENTER to continue\a", 0);
    return 0;
}

/*
        r e c e i v e   w i t h   t i m e o u t
            Gets a char from the async port 'port'.  Times out after so many
            tenths of a second.
*/
int     rx_timeout(int Tenths)
{
    uint StatChar;
    long rto;

    /* if char is in buffer return it with no kbd or timeout chks */
    if (!(B_RXEMPTY & (StatChar = async_rx(port))))
        return (StatChar & 0xff);

    /* if no char in rxbufr, set up timeout val & wait for char or timeout */
    SET_TO_TENTHS(rto, Tenths);
    while (1)
    {
        if (!(B_RXEMPTY & (StatChar = async_rx(port))))
            return (StatChar & 0xff);     /* return with char if one ready */
        if (ChkgCarrier && (B_CD & StatChar))
            return (LOST_CARRIER);               /* ret if carrier dropped */
        if (ChkgKbd && KBHIT && (char)KBREAD == ESC)
            return (ESC_PRESSED);     /* ret if watching Kbd & ESC pressed */
        if (timed_out(&rto))
            return (TIMED_OUT);                  /* ret if ran out of time */
    }
}

/*
        w a i t   f o r   m o d e m   t o   s e n d   O K
*/
int     waitforOK(int ticks)
{
    char lbuf[2];

    if (waitfor("OK", ticks))
        prompt(lbuf, "Modem does not respond,  Press ENTER to continue\a", 0);
    return 0;
}

/*
        w a i t f o r   f u n c t i o n
*/
int     waitfor(char *str, int ticks)
{
    REG char *p1, *lbuf;
    long    to;
    char    ch, *end;
    int     matchlen;

    if ((matchlen = strlen(str)) == 0)
        return (0);
    set_timeout(&to, ticks);
    lbuf = calloc(matchlen + 1, 1);
    p1 = lbuf - 1;
    end = p1 + matchlen;
    while (1)
    {
        if (timed_out(&to))
        {
            free(lbuf);
            return (TIMED_OUT);
        }
        if (KBHIT && proc_keypress(KBREAD) == X_ESC)
        {
            free(lbuf);
            return (ESC_PRESSED);
        }
        if (!async_rxcnt(port))
            continue;
        ch = (char)proc_rxch(async_rx(port));
        if (p1 != end)
        {
            *++p1 = ch;
            if (p1 != end)
                continue;
        }
        else
        {
            memmove(lbuf, &lbuf[1], matchlen);
            *p1 = ch;
        }
        if (*lbuf == *str && !memicmp(lbuf, str, matchlen))
        {
            free(lbuf);
            return (0);
        }
    }
}

/*
        p r o m p t   f o r   k e y b o a r d   i n p u t
*/
int     prompt(char *kbuf, char *promptmsg, int maxchars)
{
    REG     char *p1;
    char    ch = 0, *scrnbuf, *endkbuf;
    int     i, x, y, boxtop, boxlft, boxlen;

    x = v_ansiseq, v_ansiseq = 0;                /* save ANSI seq variable */
    y = v_color, v_color = hblu;                   /* set the color to use */
    boxlen = strlen(promptmsg) + maxchars + 6;
    boxlft = (80 - boxlen) / 2;
    boxtop = (v_btm - 5) / 2;                  /* calculate box dimensions */
    scrnbuf = malloc(SCRBUF(5, boxlen));
    pu_scrnd(boxtop, boxlft, 5, boxlen, scrnbuf);  /* save the screen area */
    for (i = 1; i < 4; i++)
        d_nchat(boxtop + i, boxlft + 1, ' ', wht, boxlen - 2, HORZ);
    d_chat(boxtop, boxlft, 'É');
    d_nchat(boxtop, boxlft + 1, 'Í', hblu, boxlen - 2, HORZ);
    d_chat(boxtop, boxlft + boxlen - 1, '»');
    d_nchat(boxtop + 1, boxlft, 'º', hblu, 3, VERT);
    d_chat(boxtop + 4, boxlft, 'È');
    d_nchat(boxtop + 4, boxlft + 1, 'Í', hblu, boxlen - 2, HORZ);
    d_chat(boxtop + 4, boxlft + boxlen - 1, '¼');            /* draw a box */
    d_nchat(boxtop + 1, boxlft + boxlen - 1, 'º', hblu, 3, VERT);
    d_msgat(boxtop + 2, boxlft + 3, hred, promptmsg);
    loc(boxtop + 2, boxlft + strlen(promptmsg) + 4); /* display the prompt */
    v_color = wht;
    for (p1 = kbuf, endkbuf = p1 + maxchars; ch != '\r' && ch != ESC;)
    {
        ch = KBREAD & 0xff;                                  /* get a char */
        if (ch != '\r' && ch != ESC)
        {
            if (ch == '\b')
            {
                if (p1 > kbuf)                 /* backspace key processing */
                    --p1, d_ch(ch);
                continue;
            }
            else if (p1 != endkbuf && isprint(ch))
            {
                d_ch(ch), *p1++ = ch;                /* put char in buffer */
                continue;
            }
            d_ch('\a');               /* beep if max chars already entered */
        }
        *p1 = '\0';                                /* terminate the buffer */
    }
    po_scrnd(scrnbuf);
    free(scrnbuf);
    v_ansiseq = x, v_color = y;              /* restore screen & variables */
    if (ch == ESC)
        return (ESC_PRESSED);                /* return this if ESCaped out */
    if (p1 == kbuf)
        return (NO_INPUT);                     /* return val if ENTER only */
    return (0);                            /* return val if got some input */
}

/*
        r e m o t e   p r o m p t   f u n c t i o n
*/
int     comprompt(char *kbuf, char *promptmsg, int maxchars)
{
    REG     char *p1;
    char    ch = 0, *endkbuf;

    async_tx_echo(promptmsg);
    for (p1 = kbuf, endkbuf = p1 + maxchars; ch != '\r' && ch != ESC;)
    {
        if (KBHIT && (KBREAD == X_ESC) || !async_carrier(port))
            ch = ESC;                       /* ck for local operator abort */
        else
            ch = async_rx(port);
        if (!ch)                             /* get a char from the remote */
            continue;
        if (ch != '\r' && ch != ESC)
        {
            if (ch == '\b')
            {
                if (p1 > kbuf)                 /* backspace key processing */
                    --p1, d_ch(ch), async_tx(port, ch);
                continue;
            }
            else if (p1 != endkbuf && isprint(ch))
            {
                d_ch(ch), async_tx(port, ch), *p1++ = ch; /* put ch in bfr */
                continue;
            }
            async_tx(port, '\a');     /* beep if max chars already entered */
        }
        *p1 = '\0';                                /* terminate the buffer */
    }
    if (ch == ESC)
        return (*kbuf = ESC);
    if (p1 == kbuf)
        return (*kbuf = '\0');                 /* return val if ENTER only */
    return (p1 - kbuf);
}

/*
        h a n g   u p   t h e   p h o n e
*/
int     hang_up(void)
{
    long rto;

    if (dtr)
        toggle_dtr();
    async_txflush(port);
    async_rxflush(port);
    SET_TO_TENTHS(rto, 3);
    while (!timed_out(&rto))
    {
        if (!async_carrier(port))
        {
            toggle_dtr();
            return 0;
        }
    }
    toggle_dtr();
    DELAY_TENTHS(11);
    async_tx_str("+++");
    DELAY_TENTHS(13);
    async_tx_str("ATH0\r");
    waitforOK(24);
    return 0;
}

