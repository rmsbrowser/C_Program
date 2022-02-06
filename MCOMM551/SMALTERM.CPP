
/////////////////////////////////////////////////////////////////////////////
//     S M A L T E R M  --  D e m o   D r i v e r   f o r   M C O M M      //
//                         A s y n c   R o u t i n e s                     //
//                                                                         //
//                           Zortech C++ 2.0 Version                       //
//                           Turbo C++ 1.0 Version                         //
//                                                                         //
//             Mike Dumdei, 6 Holly Lane, Texarkana TX 75503               //
//                North East Texas DataLink, 903 838-6713                  //
//                                                                         //
//              (Link with comm_s.lib for external functions)              //
//                     ztc smalterm.cpp comm.cpp comm_s.lib                //
//                     tcc smalterm.cpp comm.cpp comm_s.lib                //
/////////////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <process.h>
#include <stdarg.h>
#include <dos.h>
#include <bios.h>
#include <signal.h>

#include "comm.hpp"                 // header for AsyncPort class
#include "timer.hpp"                // header for Timer class
#include "ansivid.hpp"              // header for ANSI display functions
#define KEY_INT
#include "keys.h"                   // key definition header
#include "colors.h"                 // vid mode & color definition header
#include "extra.h"                  // extra functions header

#if defined(__TURBOC__)             // Turbo C++
  #define KBHIT   bioskey(1)
  #define KBREAD  bioskey(0)
  #define strcmpl stricmp
  #include <io.h>
#else                               // Zortech C++
  #define KBHIT   _bios_keybrd(1)
  #define KBREAD  _bios_keybrd(0)
#endif

#define NO_INPUT     (-1)
#define ESC_PRESSED  (-2)
#define TIMED_OUT    (-3)
#define NO_CARRIER   (-4)
#define ERROR_LIMIT  (-5)
#define GOT_CANNED   (-6)
#define BLK_COMP_ERR (-7)
#define BLK_SEQ_ERR  (-8)
#define CHECKSUM_ERR (-9)

#define TERMINAL       0
#define HOSTMODE       1            // hostmode isn't in here yet

#define SOH         '\x01'
#define STX         '\x02'
#define EOT         '\x04'
#define ACK         '\x06'
#define NAK         '\x15'
#define CAN         '\x18'

///////////////////////////////////////////////////////////////
//  function declarations in order found in source (random)  //
//:////////////////////////////////////////////////////////////
// void ControlBreakHandler(int sig);
int ProcessKeyPress();
void ProcessExtendedKey(int LongCh);
int ProcessAsyncChar();
int SaveScreen();
void RestoreScreen();
void VideoStatus(int saving);
void ClearScreen();
void ToggleLogFile();
void ToggleEcho();
void ToggleLineFeeds();
void ToggleDtr();
void DialNumber();
void DOS_Shell();
void DOS_Command();
void HostMode();
void MessageUpload();
void DSZUpload();
void DSZDownload();
void RunBatchFile(char *FileName);
void SendFile();
void ReceiveFile();
void ExitProgram();
int DisplayHelp();
void DisplayParameters();
void ChangeParameters();
int DrawBox(int Top, int Lft, int Height, int Length);
void RemoveBox();
int Prompt(char *Response, char *PromptMsg, int MaxResponse);
void HangUp();
int MonitoredDelay(int Ticks, int MonitorCarrier);
int WaitFor(char *String, int Ticks, int MonitorCarrier);
void vDisplay(char *Format, ...);
void vDisplay(int Color, char *Format, ...);
void vDisplay(int Row, int Col, char *Format, ...);
int PushScreen(int Row, int Col, int NbrRows, int NbrCols);
int PopScreen();
void ReceiveXmodem(char *FileName);
void SendXmodem(char *FileName, int BlockSize = 128);
void XmodemExitMsg(int rval);
int RxWithTimeOut(int Ticks);
void SetXmodemMode(char& save_xoff, char& save_flushbad, char *save_params);
void ResetXmodemMode(char& save_xoff, char& save_flushbad, char *save_params);
void SendCANs();
int WaitForBlockToEnd(int Ticks);
int GetHdrChar(int Ticks);
void TransferWindow(int Flag);

////////////////////////
//  global variables  //
////////////////////////
AsyncPort port(4096, 2048);     // port with 4K transmit, 2K receive buffers
char port_name[5];              // ASCII "COM1" or "COM2"
int port_number;                // assigned number COM1==0, COM2==1
int Mode;                       // identifies to subroutines HOST or TERMINAL
FILE *LogFile = NULL;           // handle for log file
char DialString[40];            // storage for dial string
int LineFeeds = 0;              // supply LineFeeds flag
int Echo = 0;                   // Echo on flag
int Dtr = 1;                    // DTR signal level flag
int cyn, hred, grn, hgrn;       // video color variables to enable support
int wht, hblu, ylw;             //   for either color or mono system
char vDispBuf[128];             // buffer for formatted display functions
char *ScreensArray[10];         // array of pointers for Push/Pop Screen
char ScreenIndex = 0;           // index of current pointer in above array


/////////////////////////////////////////////////
//  main                                       //
//://////////////////////////////////////////////
int cdecl main (int argc, char *argv[])
{
    char params[10], *pc;
    int got_port = 0, return_value, i;

    ////////////////////////////////////
    // process command line arguments //
    ////////////////////////////////////
    *params = '\0';                             // preset params to nul string
    for (i = 1; i < argc; i++)
    {
        pc = argv[i];                           // pointer to argument
        if (*pc == '-' || *pc == '/')
            ++pc;                               // skip any switch characters
        if (got_port == 0)
        {
            if (strcmpl(argv[i], "COM1") == 0)      // check for "COM1"
                got_port = 1;
            else if (strcmpl(argv[i], "COM2") == 0) // check for "COM2"
                got_port = 1, port_number = COM2;
        }
        if (isdigit(*pc) && strlen(pc) < 10)    // check for parameter string
            strupr(strcpy(params, argv[i]));
    }
    strcpy(port_name, (port_number == COM1) ? "COM1" : "COM2");

    ///////////////////////////
    // initialize the video  //
    ///////////////////////////
    initvid();                          // initialize lowlevel video variables
    if (v_mode != MONO && v_mode != CO80)
        setvmode(CO80);                 // reset video mode if necessary
    if (v_mode == CO80)
    {                                   // attributes to use if color system
        cyn = CYN, hred = H_RED, grn = GRN, hgrn = H_GRN,
        wht = WHT, hblu = H_BLU, ylw = YLW;
    }
    else
    {                                   // attributes to use if mono system
        cyn = WHT, hred = H_WHT, grn = WHT, hgrn = H_WHT,
        wht = WHT, hblu = H_WHT, ylw = H_WHT;
    }
    SaveScreen();

    /////////////////////////////
    // display sign on message //
    /////////////////////////////
    Display(hred, "SMALTERM - Sample driver for MCOMM library functions\n");
    Display(grn, "    Mike Dumdei, 6 Holly Lane, Texarkana TX 75503\n");
    Display("    North East Texas DataLink   903 838-6713\n");
    Display(hred, "Usage: smalterm {COM1 | COM2} {param str EX: 2400N81}\n\n");
    v_color = cyn;

    //////////////////////////
    // open the serial port //
    //////////////////////////
    return_value = port.Open(port_number, params);
    if (return_value != 0)                      // if port open failed
    {
        vDisplay(
          "SMALTERM: Can't open %s, Parameters: %s, Error code: %04X\n",
           port_name, (*params) ? params : "default", return_value);
        exit(return_value);
    }                                           // if port open succeeded
    vDisplay("SMALTERM: %s -- PORT OPENED\n\n", port_name);
    Display("Press F1 for HELP screen\n");

    //////////////////////////
    // miscellaneous set up //
    //////////////////////////
    tickhookset(1);                     // enable master tick in Timer class
/*  ControlBreakHandler(SIGINT);        // initialize control-break handler */
    SetWindowSize(0, 0, 23, 79);        // set window to protect status line
    Fill(24, 0, 'อ', cyn, 80);          // display status line
    Display(24, 60, hgrn, port_name);
    DisplayParameters();
    Display(24, 10, hgrn, "DTR");
    *DialString = '\0';                 // clear out dial string
    Mode = TERMINAL;                    // terminal, e.g. not host mode

    ///////////////////////////////////
//////       MAIN PROGRAM LOOP       //////
    ///////////////////////////////////
    while (1)
    {
        if (KBHIT)                      // process keyboard character if a
            ProcessKeyPress();          //  key was pressed
        if (port.RxLevel() != 0)        // process character from serial
            ProcessAsyncChar();         //  port if one is available
    }
}

/*
/////////////////////////////////////////////////
//  ControlBreakHandler                        //
//://////////////////////////////////////////////
void ControlBreakHandler(int sig)
{
    signal(SIGINT, ControlBreakHandler);
}
*/

/////////////////////////////////////////////////
//  ProcessKeyPress                            //
//://////////////////////////////////////////////
int ProcessKeyPress()
{
    int LongCh = KBREAD;                // scan code + character
    char Ch = (char)LongCh;             // character without scan code

    if (Ch != '\0')                     // if normal key
    {
        port.Tx(Ch);                    //   send it out the serial port
        if (Echo && Ch != ESC)          //   if echo on & not ESC (don't want
        {                               //    to start an ANSI sequence),
            VideoStatus(1);             //   save video status
            if (Ch == '\r' && LineFeeds)
                Ch = '\n';              //   make it a LF if CR & LF toggle
            Display(Ch);                //    display the char,
            if (LogFile)                //    and write it the log file if
                fputc(Ch, LogFile);     //    its open
            VideoStatus(0);             //   restore video status
        }
    }
    else                                // extended key, e.g. F1, ALT_X, etc,
        ProcessExtendedKey(LongCh);
    return (LongCh);                    // return the key pressed
}

/////////////////////////////////////////////////
//  ProcessExtendedKey                         //
//://////////////////////////////////////////////
void ProcessExtendedKey(int LongCh)
{
    static struct JUMP_TABLE
    {
        int KeyCode;
        void (*Function)();
    }
      TERMINAL_Table[] = {
        { ALT_C, ClearScreen },     { ALT_E, ToggleEcho },
        { ALT_X, ExitProgram },     { ALT_S, DOS_Shell },
        { ALT_D, DialNumber },      { PGUP, SendFile },
        { PGDN, ReceiveFile },      { ALT_H, HangUp },
        { ALT_L, ToggleLineFeeds }, { F2, ToggleLogFile },
        { F10, HostMode },          { ALT_P, ChangeParameters },
        { F4, DOS_Command },        { F3, MessageUpload },
        { F5, DSZUpload },          { F6, DSZDownload },
        { 0, NULL } },
      HOSTMODE_Table[] = {
        { ALT_C, ClearScreen },     { ALT_E, ToggleEcho },
        { ALT_S, DOS_Shell },       { ALT_H, HangUp },
        { ALT_L, ToggleLineFeeds }, { F2, ToggleLogFile },
        { ALT_P, ChangeParameters },{ F4, DOS_Command },
        { F3, MessageUpload },      { F5, DSZUpload },
        { F6, DSZDownload },        { 0, NULL } },
      *JumpTableList[] = { TERMINAL_Table, HOSTMODE_Table };

    JUMP_TABLE *pjt = JumpTableList[Mode];

    if (LongCh == F1)
        LongCh = DisplayHelp();         // run Help if F1 was pressed
    while (pjt->KeyCode != LongCh && pjt->KeyCode)
        ++pjt;                          // look up the key in the table
    if (pjt->KeyCode)
        pjt->Function();                // if key found, run the function
}

/////////////////////////////////////////////////
//  ProcessAsyncChar                           //
//://////////////////////////////////////////////
int ProcessAsyncChar()
{
    static int ToggleHost = 0;

    char Ch = (char)port.Rx();          // read the character
    if (Ch == '\r' && LineFeeds)
        Ch = '\n';                      // translate to LF if supplying LF's
    if (Ch != '~')                      // using ~ to remotely toggle host
        ToggleHost = 0;                 //  mode, reset counter if not ~
    else
    {
        if (++ToggleHost >= 10)         // seq of 10 ~'s toggles host mode
        {
            HostMode();
            ToggleHost = 0;             // reset counter on HostMode exit
            return 0;
        }
    }
    Display(Ch);                        // display the character
    if (LogFile && v_ansiseq == '\0')
        fputc(Ch, LogFile);             // write to log file if its on
    return ((int)Ch & 0xff);            // return received char
}

/////////////////////////////////////////////////
//  SaveScreen                                 //
//://////////////////////////////////////////////
int SaveScreen()
{
    if (PushScreen(0, 0, 25, 80) != 0)  // move screen to memory buffer
        return 0;                       //  return failed if no memory
    VideoStatus(1);                     // save video color & ANSI status
    SetWindowSize(0, 0, 24, 79);
    v_color = wht;
    ClearWindow();                      // reset window size, clear screen
    return 1;                           // return success
}

/////////////////////////////////////////////////
//  RestoreScreen                              //
//://////////////////////////////////////////////
void RestoreScreen()
{
    PopScreen();                        // restore screen
    VideoStatus(0);                     // restore v_color & v_ansiseq
    SetWindowSize(0, 0, 23, 79);        // reset the window size
}

/////////////////////////////////////////////////
//  VideoStatus                                //
//://////////////////////////////////////////////
void VideoStatus(int saving)
{
    static char savedcolor[4], savedseq[4], vsi = 0;

    if (saving)
        savedcolor[vsi] = v_color, savedseq[vsi++] = v_ansiseq,
        v_ansiseq = '\0';
    else
        v_color = savedcolor[--vsi], v_ansiseq = savedseq[vsi];
}

/////////////////////////////////////////////////
//  ClearScreen                                //
//://////////////////////////////////////////////
void ClearScreen()
{
    v_color = cyn;                      // reset screen color
    SetWindowSize(0, 0, 23, 79);        // reset the window size
    ClearWindow();                      // clear the current window
}

/////////////////////////////////////////////////
//  ToggleLogFile                              //
//://////////////////////////////////////////////
void ToggleLogFile()
{
    if (LogFile == NULL)                // if it's off, turn it on
    {
        if ((LogFile = fopen("ST.LOG", "ab")) != NULL)
            Display(24, 25, hgrn, "LOG");
    }
    else                                // if it's on, turn it off
    {
        fclose(LogFile);
        LogFile = NULL;                 // a NULL FILE * shows log is off
        Display(24, 25, cyn, "อออ");
    }
}

/////////////////////////////////////////////////
//  ToggleEcho                                 //
//://////////////////////////////////////////////
void ToggleEcho()
{
    Echo ^= 1;                          // switch state, update status line
    Display(24, 15, (Echo) ? hgrn : cyn, (Echo) ? "ECHO" : "ออออ");
}

/////////////////////////////////////////////////
//  ToggleLineFeeds                            //
//://////////////////////////////////////////////
void ToggleLineFeeds()
{
    LineFeeds ^= 1;                     // switch state, update status line
    Display(24, 21, (LineFeeds) ? hgrn : cyn, (LineFeeds) ? "LF" : "ออ");
}

/////////////////////////////////////////////////
//  ToggleDtr                                  //
//://////////////////////////////////////////////
void ToggleDtr()
{
    Dtr ^= 1;                           // switch state, update status line
    port.Dtr(Dtr);
    Display(24, 10, (Dtr) ? hgrn : (hred | BLNK), "DTR");
}

/////////////////////////////////////////////////
//  DialNumber                                 //
//://////////////////////////////////////////////
void DialNumber()
{
    char Response[25], *pc = Response;

    if (Prompt(pc, "Enter number (p###=Pulse, r=Redial): ", 24) != 0)
        return;                         // get number, exit if ESC pressed
    if (isalpha(*pc))                   // look for 'P' or 'R'
    {
        *pc = toupper(*pc);
        if (*pc == 'R' && *DialString != '\0')
        {
            port.Tx(DialString);        // redial command
            return;
        }
        else if (*pc != 'P')
            return;                     // invalid response
    }
    if (*pc == 'P')
        sprintf(DialString, "%s%s\r", "ATDP", &pc[1]);
    else
        sprintf(DialString, "%s%s\r", "ATDT", pc);
    port.Tx(DialString);
}

/////////////////////////////////////////////////
//  DOS_Shell                                  //
//://////////////////////////////////////////////
void DOS_Shell()
{
    char *comspec;

    comspec = getenv("COMSPEC");        // get command interpreter name
    if (comspec == NULL)
        comspec = "COMMAND.COM";
    if (SaveScreen() == 0)              // save the current screen
        return;                         //  exit if save failed
    Display(hred, "Type EXIT to return to SMALTERM.");
    spawnlp(0, comspec, comspec, NULL); // run command interpreter
    RestoreScreen();                    // restore the original screen
}

/////////////////////////////////////////////////
//  DOS_Command                                //
//://////////////////////////////////////////////
void DOS_Command()
{
    char buf[48];

    if (Prompt(buf, "Enter DOS command: ", 46) != 0)
        return;                         // get the command
    if (SaveScreen() == 0)              // save the current screen
        return;                         //  exit if save failed
    system(buf);                        // execute command
    RestoreScreen();                    // restore the original screen
}

/////////////////////////////////////////////////
//  HostMode                                   //
//://////////////////////////////////////////////
void HostMode()
{
    // do this later
}

/////////////////////////////////////////////////
//  MessageUpload                              //
//://////////////////////////////////////////////
void MessageUpload()
{
    char buf[81];
    FILE *fh;
    char *pc;

    if (Prompt(buf, "Enter ASCII filename: ", 46) != 0)
        return;                         // get the file name
    if ((fh = fopen(buf, "rt")) == NULL)
    {
        Prompt(buf, "File open error, Press ENTER\a", 0);
        return;                         // abort if can't open the file
    }
    port.RxFlush();                     // flush the port
    port.TxFlush();
    while (fgets(buf, 80, fh))          // while more is left
    {
        if (*buf == '\n')               // change blank lines to SPC,CRs
            strcpy(buf, " \r");
        else if ((pc = strchr(buf, '\n')) != NULL)
            *pc = '\r';                 // change LFs to simulate ENTER key
        for (pc = buf; *pc; pc++)
        {
            while (!port.TxEmpty())     // send a char at a time for looks
            {
                if (KBHIT && (KBREAD == X_ESC))
                {                       // monitor for upload aborted
                    fclose(fh);
                    return;
                }
                while (port.RxLevel() != 0)
                    ProcessAsyncChar();
            }
            port.Tx(*pc);
        }
    }
    fclose(fh);                         // close the file
}

/////////////////////////////////////////////////
//  DSZUpload                                  //
//://////////////////////////////////////////////
void DSZUpload()
{
    RunBatchFile("SND.BAT");
}

/////////////////////////////////////////////////
//  DSZDownload                                //
//://////////////////////////////////////////////
void DSZDownload()
{
    RunBatchFile("RCV.BAT");
}

/////////////////////////////////////////////////
//  RunBatchFile                               //
//://////////////////////////////////////////////
void RunBatchFile(char *FileName)
{
    char lbuf[50], *pc = lbuf;

    strcpy(pc, FileName);
    pc += strlen(pc);
    *pc++ = ' ';                        // copy filename to local buffer
    if (Prompt(pc, "Enter batch parameters -> ", 40) == ESC_PRESSED)
        return;                         // add on any args, return if ESC
    if (SaveScreen() == 0)              // save current screen
        return;                         //  return if save failed
    vDisplay(hred, "Executing: %s\n\n", lbuf);
    system(lbuf);                       // run the batch file
    RestoreScreen();                    // restore the screen
}

/////////////////////////////////////////////////
//  SendFile                                   //
//://////////////////////////////////////////////
void SendFile()
{
    char buf[48], *pc;

    if (Prompt(buf, "Enter Filename{,k}: ", 46) != 0)
        return;                         // get the file name
    pc = strchr(buf, ',');              // check if Xmodem-1k selected
    if (pc && (strchr(pc, 'k') || strchr(pc, 'K')))
    {
        *pc = '\0';
        SendXmodem(buf, 1024);
    }
    else                                // send the file
        SendXmodem(buf);
}

/////////////////////////////////////////////////
//  ReceiveFile                                //
//://////////////////////////////////////////////
void ReceiveFile()
{
    char buf[48], *pc;

    if (Prompt(buf, "Enter Filename: ", 46) != 0)
        return;                         // get the file name
    ReceiveXmodem(buf);                 // receive the file
}

/////////////////////////////////////////////////
//  ExitProgram                                //
//://////////////////////////////////////////////
void ExitProgram()
{
    char buf[2];

    switch(Prompt(buf, "Exit SmalTerm (Y, n)? ", 1))
    {                                   // make sure they want to do this
        case ESC_PRESSED:
            return;                     // return if ESC
        default:
            if (*buf != 'y' && *buf != 'Y')
                return;                 // if not 'Yes', then return
        case NO_INPUT:                  // 'Enter key' is a default 'yes'
            break;
    }
    RestoreScreen();                    // replace the original screen
    tickhookset(0);                     // remove the interrupt 1C hook
    exit(0);                            // leave the program
}

/////////////////////////////////////////////////
//  DisplayHelp                                //
//://////////////////////////////////////////////
int DisplayHelp()               // interim help function
{
    int LongCh;
    static char *HelpScrn = "\
    F1 - Help                   ALT C - Clear Screen\n\
    F2 - Toggle Log File        ALT D - Dial Number\n\
    F3 - BBS Message Upload     ALT E - Toggle Echo\n\
    F4 - Execute DOS Command    ALT H - Hang Up\n\
    F5 - DSZ Upload   SND.BAT   ALT L - Toggle Line Feeds\n\
    F6 - DSZ Download RCV.BAT   ALT P - Change Parameters\n\
   F10 - Host Mode (not there)  ALT S - DOS Shell\n\
  PGUP - Xmodem Send            ALT X - Exit Program\n\
  PGDN - Xmodem Receive  << Press ESC or Command >>";

    DrawBox(5, 10, 12, 60);
    SetWindowSize(6, 11, 18, 69);
    v_color = ylw;
    Display(6, 12, "SMALTERM COMMANDS:\n");
    Display(wht, HelpScrn);
    LongCh = KBREAD;
    RemoveBox();
    SetWindowSize(0, 0, 23, 79);
    return LongCh;
}

/////////////////////////////////////////////////
//  DisplayParameters                          //
//://////////////////////////////////////////////
void DisplayParameters()
{
    Fill(24, 65, 'อ', cyn, 10);
    Display(24, 65, hgrn, port.Params());
}

/////////////////////////////////////////////////
//  ChangeParameters                           //
//://////////////////////////////////////////////
void ChangeParameters()
{
    char buf[12];

    if (Prompt(buf, "Enter parameters (Ex: 1200N81) -> ", 10) != 0)
        return;                         // get the new parameters
    if (port.Params(strupr(buf)) == 0)
        DisplayParameters();            // display updated params if success,
    else                                //  else display error message
        Prompt(buf, "Invalid parameters, Press ENTER to continue\a", 0);
}

/////////////////////////////////////////////////
//  DrawBox                                    //
//://////////////////////////////////////////////
int DrawBox(int Top, int Lft, int Height, int Length)
{
    static char BoxChar[] = { 'ษ', 'ป', 'ศ', 'ผ', 'อ', 'บ' };
    int Btm, Rgt;

    if (PushScreen(Top, Lft, Height, Length) != 0)
        return 0;                   // save area for box, return if failed
    Btm = Top + Height - 1;
    Rgt = Lft + Length - 1;
    VideoStatus(1);                 // save video variables
    v_color = wht;
    SetWindowSize(Top, Lft, Btm, Rgt);
    ClearWindow();                  // clear the area for the box
    SetWindowSize(0, 0, 23, 79);
    v_color = hblu;                 // draw the box
    Fill(Top, Lft, BoxChar[4], hblu, Length);
    Fill(Btm, Lft, BoxChar[4], hblu, Length);
    Fill(Top, Rgt, BoxChar[5], hblu, Height, 0);
    Fill(Top, Lft, BoxChar[5], hblu, Height, 0);
    Display(Top, Lft, BoxChar[0]);
    Display(Top, Rgt, BoxChar[1]);
    Display(Btm, Lft, BoxChar[2]);
    Display(Btm, Rgt, BoxChar[3]);
    return 1;
}

/////////////////////////////////////////////////
//  RemoveBox                                  //
//://////////////////////////////////////////////
void RemoveBox()
{
    PopScreen();                        // restore screen
    VideoStatus(0);                     // reset v_color & v_ansiseq
}

/////////////////////////////////////////////////
//  Prompt                                     //
//://////////////////////////////////////////////
int Prompt(char *Response, char *PromptMsg, int MaxResponse)
{
    int Lft, Length;
    char *pc, *end, ch;

    Length = strlen(PromptMsg) + MaxResponse + 6;
    Lft = (80 - Length) / 2;            // calculate box size required
    if (DrawBox(9, Lft, 5, Length) == 0) // draw the box
        return ESC_PRESSED;
    v_color = hred;
    Display(11, Lft + 3, PromptMsg);    // display prompt
    v_color = wht;
    pc = Response, end = pc + MaxResponse;

    while ((ch = (char)KBREAD) != '\r' && ch != ESC)
    {                                   // get the response
        if (ch == '\b')
        {                               // backspace key
            if (pc > Response)
                --pc, Display(ch);
            continue;
        }
        if (pc != end && isprint((int)ch & 0xff))
        {                               // good character
            Display(ch), *pc++ = ch;
            continue;
        }
        Display('\a');                  // illegal character
    }
    *pc = '\0';                         // terminate the string
    RemoveBox();
    if (ch == ESC)
        return ESC_PRESSED;             // return something
    if (*Response == '\0')
        return NO_INPUT;
    return 0;
}

/////////////////////////////////////////////////
//  HangUp                                     //
//://////////////////////////////////////////////
void HangUp()
{
    ToggleDtr();                        // drop DTR
    port.TxFlush();
    port.RxFlush();                     // flush the buffers
    if (MonitoredDelay(24, 1) == 0)     // wait a while for carrier
    {                                   //  to drop
        port.Tx("+++");                 // if dropping Dtr didn't
        if (MonitoredDelay(18, 1) == 0) //  work try modem command
        {
            port.Tx("ATH0\r");
            WaitFor("OK", 54, 1);       // 3 secs, monitor carrier
        }
    }
    ToggleDtr();
}

/////////////////////////////////////////////////
//  MonitoredDelay                             //
//://////////////////////////////////////////////
int MonitoredDelay(int Ticks, int MonitorCarrier)
{
    int LongCh;

    Timer t(Ticks);                     // start the delay
    while (!t.Expired())
    {
        if ((LongCh = KBHIT) != 0)      // check the keyboard
        {
            if (LongCh == X_ESC)
            {
                KBREAD;
                return ESC_PRESSED;     // return if ESC pressed
            }
            ProcessKeyPress();          // process key if not ESC
        }
        if (MonitorCarrier && !port.Carrier())
            return NO_CARRIER;
        if (port.RxLevel() != 0)        // check the async port
            ProcessAsyncChar();
    }
    return 0;
}

/////////////////////////////////////////////////
//  WaitFor                                    //
//://////////////////////////////////////////////
int WaitFor(char *String, int Ticks, int MonitorCarrier)
{
    char ch, *pc, *buf, *end;
    int MatchLen, LongCh, rval;

    if ((MatchLen = strlen(String)) == 0)
        return (0);                     // return if nothing to wait for
    buf = new char[MatchLen];           // allocate & clear incoming chars buf
    memset(buf, '\0', MatchLen);
    pc = buf - 1;
    end = pc + MatchLen;                // preset pointers

    Timer t(Ticks);                     // start a timer for 'Ticks' duration
    while (1)
    {
        while (port.RxLevel() != 0)     // check for characters in the port
        {
            ch = ProcessAsyncChar();
            if (pc != end)
            {                           // if don't have MatchLen chars yet
                *++pc = ch;
                if (pc != end)
                    continue;
            }
            else                        // have MatchLen chars
            {
                memmove(buf, &buf[1], MatchLen);
                *pc = ch;               // put this char in the match buffer
            }                           //  and check for a match
            if (*buf == *String && !memicmp(buf, String, MatchLen))
            {
                delete buf;             // if found wait for string
                return 0;
            }
        }
        if ((LongCh = KBHIT) != 0)      // if got a keyboard entry
        {
            if (LongCh == X_ESC)
            {
                KBREAD;
                rval = ESC_PRESSED;     // return if it was ESC
                break;
            }
            ProcessKeyPress();          //  else process key
        }
        if (t.Expired())                // if max wait time has elapsed
        {
            rval = ESC_PRESSED;
            break;
        }
        if (MonitorCarrier && !port.Carrier())
        {                               // if lost carrier & care
            rval = NO_CARRIER;
            break;
        }
    }
    delete buf;
    return rval;
}

/////////////////////////////////////////////////
//  vDisplay (1 of 3)                          //
//://////////////////////////////////////////////
void vDisplay(char *Format, ...)
{
    va_list arg_ptr;
    va_start(arg_ptr, Format);
    vsprintf(vDispBuf, Format, arg_ptr);
    Display(vDispBuf);
}

/////////////////////////////////////////////////
//  vDisplay (2 of 3)                          //
//://////////////////////////////////////////////
void vDisplay(int Color, char *Format, ...)
{
    va_list arg_ptr;
    va_start(arg_ptr, Format);
    vsprintf(vDispBuf, Format, arg_ptr);
    Display(Color, vDispBuf);
}

/////////////////////////////////////////////////
//  vDisplay (3 of 3)                          //
//://////////////////////////////////////////////
void vDisplay(int Row, int Col, char *Format, ...)
{
    va_list arg_ptr;
    va_start(arg_ptr, Format);
    vsprintf(vDispBuf, Format, arg_ptr);
    Display(Row, Col, vDispBuf);
}

/////////////////////////////////////////////////
//  PushScreen                                 //
//://////////////////////////////////////////////
int PushScreen(int Row, int Col, int NbrRows, int NbrCols)
{
    if (ScreenIndex == 10)              // return if ScreensArray is full
        return (-1);
    if ((ScreensArray[ScreenIndex] = new char[2*NbrRows*NbrCols+16]) == NULL)
        return (-2);                    // return if can't alloc screen buffer
    pu_scrnd(Row, Col, NbrRows, NbrCols, ScreensArray[ScreenIndex]);
    ++ScreenIndex;                      // increment the index
    return 0;
}

/////////////////////////////////////////////////
//  PopScreen                                  //
//://////////////////////////////////////////////
int PopScreen()
{
    if (ScreenIndex == 0)
        return (-1);                    // return if no screens pushed
    --ScreenIndex;
    po_scrnd(ScreensArray[ScreenIndex]); // call low level restore screen
    delete ScreensArray[ScreenIndex];    // release allocated memory
    return 0;
}


/////////////////////////////////////////////////
//  ReceiveXmodem                              //
//://////////////////////////////////////////////
void ReceiveXmodem(char *FileName)
{
    FILE *fh;
    char *pc, *buf, *end;

    /////////////////////////////////////////////
    //  alloc memory, open file, display info  //
    /////////////////////////////////////////////
    if ((buf = new char[1024 + 10]) == NULL) // alloc enough for 1K blocks
    {
        Prompt((char *)&buf, "Not enough memory, Press ENTER\a", 0);
        return;                         // abort if can't alloc memory
    }
    if ((fh = fopen(FileName, "rb")) != NULL)
    {
        Prompt((char *)&buf, "File exists, Press ENTER\a", 0);
        delete buf;
        return;                         // abort if file exists
    }
    if ((fh = fopen(FileName, "wb")) == NULL)
    {
        Prompt((char *)&buf, "File creation error, Press ENTER\a", 0);
        delete buf;
        return;                         // abort if can't open output file
    }

    ///////////////////////////////
    //  pull up transfer window  //
    ///////////////////////////////
    TransferWindow(-1);                 // display receive transfer window
    Display(1, 48, strupr(FileName));

    //////////////////////////////////////////////////////
    //  set xflow off, params to N81, ignore bad chars  //
    //////////////////////////////////////////////////////
    char save_xoff, save_flushbad, save_params[10];
    SetXmodemMode(save_xoff, save_flushbad, save_params);

    //////////////////////////////
    //  intitiate the transfer  //
    //////////////////////////////
    static int BlockSizeAry[] = { 0, 128, 1024 };  // nul, SOH, STX
    int rval, LongCh, CANsRxd, TimeOuts, BlockSize, UseCrc;
    int i, BlockNbr, RxdBlockNbr, Checksum, NeedHdrChar, ConsecErrors;
    long BytesTransferred = 0L;

    TimeOuts = CANsRxd = 0;
    rval = 1;
    while (rval == 1)
    {
        port.Tx((TimeOuts < 6) ? 'C' : NAK);    // try CRC Xmodem first, if
        switch (LongCh = GetHdrChar(90))        // no response try checksum
        {
          case SOH:                     // 128 byte block header
          case STX:                     // 1K byte block header
            BlockSize = BlockSizeAry[LongCh];
            rval = 0;
            break;
          case TIMED_OUT:               // no response
            if (++TimeOuts > 10)
                rval = TIMED_OUT;
            break;
          default:                      // fatal error
            rval = LongCh;
            break;
        }
    }
    UseCrc = (TimeOuts < 5) ? 1 : 0;    // set type of checksum to use
    BlockNbr = 1;                       // initial block number
    NeedHdrChar = TimeOuts = ConsecErrors = 0;

    while (!rval)
    {
        LongCh = 0;
        while (!LongCh)                 // while good blocks
        {
            ////////////////////////////////
            //  get the next header char  //
            ////////////////////////////////
            if (NeedHdrChar)            // if haven't already got SOH
            {
                LongCh = GetHdrChar(180);   // wait up to 10 secs
                if ((char)LongCh != SOH && (char)LongCh != STX)
                    break;
                TimeOuts = 0;
                BlockSize = BlockSizeAry[LongCh];
            }
            else
                NeedHdrChar = 1;
            ////////////////////////////
            //  get the block number  //
            ////////////////////////////
            if ((LongCh = RxWithTimeOut(90)) < 0)   // 90 == 5 secs
                break;
            RxdBlockNbr = LongCh;
            if ((LongCh = RxWithTimeOut(90)) < 0)
                break;
            if (LongCh + RxdBlockNbr != 0xff)
            {                               // block complement error
                LongCh = BLK_COMP_ERR;
                break;
            }
            if ((char)RxdBlockNbr != (char)BlockNbr)
            {                               // not expected block number
                LongCh = BLK_SEQ_ERR;
                break;
            }
            ////////////////////////
            //  receive the data  //
            ////////////////////////
            Checksum = 0;
            pc = buf, end = pc + BlockSize; // initialize variables
            while (pc < end)
            {
                if ((LongCh = RxWithTimeOut(90)) < 0)
                    break;
                *pc++ = (char)LongCh;       // read and store data
                if (UseCrc)                 // update crc if using crc
                    Checksum = update_crc(Checksum, (char)LongCh);
                else
                    Checksum += LongCh;     // else update simple checksum
            }
            if (pc != end)                  // if an error occurred while
                break;                      //  receiving block data
            ///////////////////////////////
            //  receive the check value  //
            ///////////////////////////////
            if ((LongCh = RxWithTimeOut(90)) < 0)
                break;                  // high byte of crc or the checksum
            if (UseCrc)
            {
                if ((Checksum ^= (LongCh << 8)) & 0xff00)
                {                          // crc didn't match, breaking here
                    LongCh = CHECKSUM_ERR; // makes error recovery faster on
                    break;                 // short blocks
                }
                if ((LongCh = RxWithTimeOut(90)) < 0)
                    break;              // low byte of crc
            }
            if ((LongCh ^= Checksum) != 0)
            {                           // crc or checksum didn't match
                LongCh = CHECKSUM_ERR;
                break;
            }
            port.Tx(ACK);                   // good block, send an ACK
            fwrite(buf, 1, BlockSize, fh);  // update the file
            ++BlockNbr;                     // advance the blocknbr
            BytesTransferred += (long)BlockSize;
            vDisplay(3, 48, "%ld", BytesTransferred);
            ConsecErrors = 0;               // reset consecutive error count
        }
        ///////////////////////////////
        //  errors and EOT handling  //
        ///////////////////////////////
        if (LongCh == EOT)                  // end of file indicator
        {
            for (i = 0; i < 3; i++)
            {
                port.Tx(NAK);               // try to confirm real EOT
                if ((char)(LongCh = GetHdrChar(90)) == EOT)
                {
                    port.Tx(ACK);           // if got 2nd EOT after NAKing
                    rval = 1;               // 1st one, it must be real EOT
                    break;
                }
                if ((char)LongCh == SOH     // if got an SOH or STX, 1st EOT
                 || (char)LongCh == STX     // must have been false
                 || (LongCh < 0 && LongCh != TIMED_OUT))
                    break;                  // if < 0, fatal error occurred
            }
            if (LongCh == TIMED_OUT)        // got an EOT but after 3 attempts
                rval = 2;                   // to get confirmation, timed out,
            if (rval)                       // so exit at different level
                break;

            if ((char)LongCh == SOH || (char)LongCh == STX)
            {                                       // got EOT followed by
                Display(5, 48, "False EOT");        // valid start of block
                BlockSize = BlockSizeAry[LongCh];   // char, try to get
                NeedHdrChar = 0;                    // another block
                continue;
            }
        }
        if (++ConsecErrors == 10)       // exit after 10 errors
            LongCh = ERROR_LIMIT;
        vDisplay(4, 48, "%d", ConsecErrors);
        switch (LongCh)                 // switch on error code
        {
          case BLK_COMP_ERR:
            Display(5, 48, "Bad Block Complement");   //  complement error
            if ((rval = WaitForBlockToEnd(18)) != 0)  // wait for port to dry
                break;                                // up, send a NAK, and
            port.Tx(NAK);                             // and try again
            break;
          case BLK_SEQ_ERR:
            Display("Bad Block Sequence  ");
            if ((char)RxdBlockNbr != (char)(BlockNbr - 1)
             || (LongCh = WaitForBlockToEnd(18)) != 0)
            {                                         //  sequence error
                rval = LongCh;                        // if wrong seq and not
                break;                                // previous seq either,
            }                                         // then have fatal error.
            port.Tx(ACK);                             // if got repeat block,
            continue;                                 // ACK it and try again
          case CHECKSUM_ERR:
            Display(5, 48, (UseCrc) ?                 //  bad crc or checksum
             "Block Crc Error     " :
             "Block Checksum Error");
            if ((rval = WaitForBlockToEnd(4)) != 0)   // wait for about 1/4
                break;                                // second of silence,
            port.Tx(NAK);                             // then NAK block and
            break;                                    // try again
          case TIMED_OUT:
            Display(5, 48,                  //  remote not answering
             "Timed Out           ");
            port.Tx(NAK);                   // if no answer, send a NAK to
            break;                          // try to get a response
          default:
            rval = LongCh;                  //   CAN, ESC, lost carrier, ...
            break;                          // set exit code and quit
        }
    }
    if (rval == ESC_PRESSED)                // send CANs if ESC pressed
    {
        Display(5, 48, "Aborting File Transfer ...");
        WaitForBlockToEnd(18);
        SendCANs();
    }
    fclose(fh);                             // close the file, free the
    delete(buf);                            // memory, reset params and exit
    ResetXmodemMode(save_xoff, save_flushbad, save_params);
    TransferWindow(0);
    XmodemExitMsg(rval);                    // display exit message
}

/////////////////////////////////////////////////
//  SendXmodem                                 //
//://////////////////////////////////////////////
void SendXmodem(char *FileName, int BlockSize)
{
    FILE *fh;
    char *pc, *buf, *end;
    long FileLength;

    /////////////////////////////////////////////
    //  alloc memory, open file, display info  //
    /////////////////////////////////////////////
#if !defined(__TURBOC__)
    FileLength = filesize(FileName);    // Zortech's filesize
#endif
    if ((buf = new char[BlockSize + 10]) == NULL)
    {
        Prompt((char *)&buf, "Not enough memory, Press ENTER\a", 0);
        return;                         // abort if can't alloc memory
    }
    if ((fh = fopen(FileName, "rb")) == NULL)
    {
        Prompt((char *)&buf, "File not found, Press ENTER\a", 0);
        delete buf;
        return;                         // abort if can't open file
    }
#if defined(__TURBOC__)
    FileLength = filelength(fileno(fh));
#endif

    ///////////////////////////////
    //  pull up transfer window  //
    ///////////////////////////////
    TransferWindow(1);                  // display transmit transfer window
    Display(1, 48, strupr(FileName));
    vDisplay(2, 48, "%ld", FileLength); // display filename and size

    //////////////////////////////////////////////////////
    //  set xflow off, params to N81, ignore bad chars  //
    //////////////////////////////////////////////////////
    char save_xoff, save_flushbad, save_params[10];
    SetXmodemMode(save_xoff, save_flushbad, save_params);

    /////////////////////////////
    //  wait for a 'C' or NAK  //
    /////////////////////////////
    int LongCh, rval, CANsRxd, TimeOuts, UseCrc;

    CANsRxd = TimeOuts = UseCrc = 0;
    rval = 1;
    while (rval == 1)                   // wait for something to happen
    {
        switch (LongCh = RxWithTimeOut(18))
        {
          case 'C':                     // CRC xmodem
              UseCrc = 1;
              rval = 0;
              break;
          case NAK:                     // simple checksum xmodem
              rval = 0;
              break;
          case TIMED_OUT:               // didn't get anything
              if (++TimeOuts < 60)
                  continue;
              rval = LongCh;
              break;
          case CAN:                     // remote sent control-X's
              if (++CANsRxd < 2)
                  continue;
                rval = GOT_CANNED;
                break;
          case NO_CARRIER:              // lost the carrier
          case ESC_PRESSED:             // cancelled on this end
              rval = LongCh;
              break;
          default:
              CANsRxd = 0;
              break;
        }
    }

    ////////////////////////////
    //  send the file blocks  //
    ////////////////////////////
    char BlockNbr = '\0', HdrChar = (BlockSize == 1024) ? STX : SOH;
    int i, ConsecErrors, BadBlock, FirstAck, Checksum, ExtraNAK;
    long BytesTransferred = 0L;

    BadBlock = FirstAck = CANsRxd = 0;
    while (!rval)
    {
        if (!BadBlock)              // if last block was OK
        {
            ++BlockNbr;
            ConsecErrors = 0;
            i = fread(buf, 1, BlockSize, fh);   // read a block in
            if (i == 0)             // if end of file
            {
                rval = 1;           // exit value if EOT gets ACKd
                while (1)
                {
                    port.Tx(EOT);
                    if ((char)RxWithTimeOut(18) == ACK)
                        break;
                    if (i++ == 4)
                    {
                        rval = 2;   // exit value if EOT doesn't get ACKd
                        break;
                    }
                }
                break;
            }
            if (i < BlockSize)          // pad last block with CTL-Z's
                memset(&buf[i], '\x1a', BlockSize - i);
        }
        else                            // had a bad block
        {
            if (++ConsecErrors == 10)
            {                           // 10 consecutive errors, abort
                rval = ERROR_LIMIT;
                continue;
            }
            vDisplay(4, 48, "%d", ConsecErrors);
            vDisplay(5, 48, "Got NAK for block %d", BlockNbr);
            if (!FirstAck)              // if not any good blocks yet
            {
                if (BlockSize == 1024 && ConsecErrors == 2)
                {
                    BlockSize = 128;    // Xmodem-1k isn't working,
                    BadBlock = 0;       // try 128 byte blocks
                    BlockNbr = '\0';
                    HdrChar = SOH;
                    rewind(fh);
                    continue;
                }
                if (UseCrc && ConsecErrors == 8)
                    UseCrc = 0;         // last ditch effort, try checksum
            }
        }
        port.Tx(HdrChar);               // send block header
        port.Tx(BlockNbr);
        port.Tx(~BlockNbr);
        port.Tx(buf, BlockSize);        // send file data
        if (UseCrc)
        {                               // send the CRC
            Checksum = calc_crc(buf, BlockSize);
            port.Tx((char)(Checksum >> 8));
            port.Tx((char)Checksum);
        }
        else
        {                               // if not CRC, send simple checksum
            Checksum = 0;
            pc = buf, end = pc + BlockSize;
            while (pc < end)
                Checksum += (int)*pc++;
            port.Tx(Checksum);
        }
        ////////////////////
        //  wait for ACK  //
        ////////////////////
        ExtraNAK = 0;                    //---------------------------//
        while (!rval && !port.TxEmpty()) // while block is being sent //
        {                                //                           //
            if (port.RxLevel() != 0)     //  if something comes in    //
            {                            //                           //
                LongCh = port.Rx();      //  get whatever it is       //
                                         //---------------------------//
                //--------------------------------------------------//
                //   If it's an ACK ignore it.  It may be due to a  //
                // duplicate packet error but it is safer to make   //
                // the receiver timeout and send another ACK after  //
                // the packet has been sent.                        //
                //   If it's a CAN, ignore that also and make them  //
                // resend them at the end of the block.             //
                //   An out of sequence NAK may be due to a block   //
                // complement error.  Go ahead and take it but wait //
                // for a few seconds after the block has finished   //
                // transmitting to accept it as valid.              //
                //--------------------------------------------------//
                if ((char)LongCh == NAK)
                    ExtraNAK = 1;
            }
            if (KBHIT && KBREAD == X_ESC)   // monitor the keyboard
                rval = ESC_PRESSED;
            if (!port.Carrier())            // monitor the carrier
                rval = NO_CARRIER;
        }
        TimeOuts = 0;   //-------------------------------------------//
        while (!rval)   //  transmit buffer empty now, block is sent //
        {               //-------------------------------------------//
            LongCh = RxWithTimeOut(18);     // receive with 1 sec timeout
            if (LongCh >= 0)
            {
                if ((char)LongCh == ACK)    // if got an ACK, have a good
                {                           // block
                    FirstAck = 1;
                    BadBlock = 0;
                    BytesTransferred += (long)BlockSize;
                    vDisplay(3, 48, "%ld", BytesTransferred);
                    break;
                }
                if ((char)LongCh == NAK)    // if got a NAK, have a bad
                {                           // block
                    BadBlock = 1;
                    break;
                }
                if ((char)LongCh == CAN && ++CANsRxd >= 2)
                    rval = GOT_CANNED;      // remote cancelled transfer
                else
                    CANsRxd = 0;
                continue;
            }
            if (LongCh == TIMED_OUT)
            {
                if (++TimeOuts == 3 && ExtraNAK)
                {                       // If had gotten a NAK during the //
                    BadBlock = 1;       // transmission of the block and  //
                    break;              // 3 secs have passed with no     //
                }                       // chars, assume NAK was genuine  //
                else if (TimeOuts >= 45)
                    rval = LongCh;      // if nothing after 45 secs, abort
            }
            else if (LongCh < 0)        // LongCh < 0 but not a timeout is
                rval = LongCh;          // is a fatal error
        }
    }
    //////////////////////////////////////////////////////
    //  close file, free memory, reset params, & exit   //
    //////////////////////////////////////////////////////
    fclose(fh);
    delete buf;
    if (rval == ESC_PRESSED)
    {
        Display(5, 48, "Aborting file transfer ...");
        while (port.Carrier() && !port.TxEmpty())
            ;
        SendCANs();
    }
    ResetXmodemMode(save_xoff, save_flushbad, save_params);
    TransferWindow(0);
    XmodemExitMsg(rval);
}

/////////////////////////////////////////////////
//  XmodemExitMsg                              //
//://////////////////////////////////////////////
void XmodemExitMsg(int rval)
{
    switch (rval)                           // display exit message
    {
      case 1:
        Display("\nFile transferred\n");
        break;
      case 2:
        Display("\nFile transferred, EOT unconfirmed\n");
        break;
      case TIMED_OUT:
        Display("\nRemote fails to respond\n");
        break;
      case GOT_CANNED:
        Display("\nRemote aborted transfer\n");
        break;
      case ERROR_LIMIT:
        Display("\nMaximum error count exceeded\n");
        break;
      case ESC_PRESSED:
        Display("File transfer aborted\n");
        break;
      case NO_CARRIER:
        Display("\nCarrier lost\n");
        break;
      default:
        Display("\nUnknown exit code\n");
        break;
    }
}

/////////////////////////////////////////////////
//  RxWithTimeOut                              //
//://////////////////////////////////////////////
int RxWithTimeOut(int Ticks)
{
    int LongCh;

    if (!(B_RXEMPTY & (LongCh = port.Rx())))
        return (LongCh & 0xff);         // fast return if char available

    Timer t(Ticks);                     // start timeout timer
    while (1)
    {
        if (port.RxLevel())
            return (port.Rx() & 0xff);  // return char if available
        if (!port.Carrier())
            return NO_CARRIER;          // return if carrier was lost
        if (KBHIT && KBREAD == X_ESC)
            return ESC_PRESSED;         // return if ESC was pressed
        if (t.Expired())
            return TIMED_OUT;           // return if Ticks have expierd
    }
}

/////////////////////////////////////////////////
//  SetXmodemMode                              //
//://////////////////////////////////////////////
void SetXmodemMode(char& save_xoff, char& save_flushbad, char *save_params)
{
    char new_params[10], *pc;

    save_xoff = port.XFlow();           // make sure XON/XOFF is turned off
    port.XFlow('\0');
    strcpy(save_params, port.Params()); // make sure params are N,8,1
    strcpy(new_params, save_params);
    for (pc = new_params; isdigit(*pc); pc++)
        ;                               // skip over baudrate
    strcpy(pc,"N81");                   // new_params = baudrate+"N81"
    port.Params(new_params);
    DisplayParameters();
    port.TxFlush();                     // flush the transmit side only
    save_flushbad = port.FlushBadChars();
    port.FlushBadChars(1);              // flush framing errors mode on
}

/////////////////////////////////////////////////
//  ResetXmodemMode                            //
//://////////////////////////////////////////////
void ResetXmodemMode(char& save_xoff, char& save_flushbad, char *save_params)
{
    port.XFlow(save_xoff);                  // reset to original XOFF mode
    port.FlushBadChars(save_flushbad);      // reset flush bad chars switch
    port.Params(save_params);               // back to original parameters
    DisplayParameters();
}

/////////////////////////////////////////////////
//  SendCANs                                   //
//://////////////////////////////////////////////
void SendCANs()
{
    port.Tx("\x18\x18\x18\x18\x18\b\b\b\b\b");  // send a bunch of control-X's
    while (port.Carrier() && !port.TxEmpty())
        ;
    port.RxFlush();
}

/////////////////////////////////////////////////
//  WaitForBlockToEnd                          //
//://////////////////////////////////////////////
int WaitForBlockToEnd(int Ticks)
{
    int LongCh;

    while ((LongCh = RxWithTimeOut(Ticks)) >= 0)
        ;                                       // wait for timeout or error
    return (LongCh == TIMED_OUT) ? 0 : LongCh;
}

/////////////////////////////////////////////////
//  GetHdrChar                                 //
//://////////////////////////////////////////////
int GetHdrChar(int Ticks)
{
    int LongCh, TimeOuts = 0, CANsRxd = 0;

    while (1)
    {
        LongCh = RxWithTimeOut(1);
        if ((char)LongCh == SOH || (char)LongCh == STX || (char)LongCh == EOT)
            break;                  // got block header
        if (LongCh == TIMED_OUT)
        {
            if (++TimeOuts > Ticks)
                break;              // exceeded max Ticks
        }
        else if (LongCh < 0)
            break;                  // aborted, lost carrier, ...
        if ((char)LongCh == CAN && ++CANsRxd >= 2)
        {
            LongCh = GOT_CANNED;
            break;                  // remote cancelled
        }
        else
            CANsRxd = 0;            // garbage, reset CAN counter
    }
    return (LongCh);
}

/////////////////////////////////////////////////
//  TransferWindow                             //
//://////////////////////////////////////////////
void TransferWindow(int Flag)
{
    if (Flag == 0)
    {
        RemoveBox();
        return;
    }
    DrawBox(0, 26, 7, 53);
    v_color = ylw;
    if (Flag > 0)
        Display(1, 34, "Transmitting:");
    else
        Display(1, 37, "Receiving:");
    Display(2, 35, "Total Bytes:");
    Display(3, 29, "Bytes Transferred:");
    Display(4, 28, "Consecutive Errors:");
    Display(5, 36, "Last Error:");
    v_color = wht;
}

