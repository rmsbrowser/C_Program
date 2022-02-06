
/*/////////////////////////////////////////////////////////////////////
//                                                                   //
//                                                                   //
//  ZCMPLR.C -- compiler specific functions                          //
//                                                                   //
//   (c) 1991, Mike Dumdei, 6 Holly Lane, Texarkana TX, 75503        //
//                                                                   //
//////////////////////////////////////////////////////////////////// */
#ifndef __TURBOC__          /* defined by Turbo C compiler */
  #ifndef __ZTC__           /* defined by Zortech C compiler */
    #define __MSC__         /* assume Microsoft C if not TC or ZTC */
  #endif
#endif

/*/////////////////////////////////////////////////////////////////
//                                                               //
//                                                               //
//      Microsoft C  /  Quick C                                  //
//                                                               //
//                                                               //
//////////////////////////////////////////////////////////////// */
#ifdef __MSC__

#include <dos.h>
/*/////////////////////////////////////////////////////////
//  DosFindFirst -- find file (1st instance)             //
//////////////////////////////////////////////////:disk:/*/
int DosFindFirst(char *pathname, int atrib, struct find_t *fstruc)
{
    return (_dos_findfirst(pathname, atrib, fstruc));
}

/*/////////////////////////////////////////////////////////
//  DosFindNext -- find file (except 1st instance)       //
//////////////////////////////////////////////////:disk:/*/
int DosFindNext(struct find_t *fstruc)
{
    return (_dos_findnext(fstruc));
}

/*/////////////////////////////////////////////////////////
//  DosGetDiskFree -- gets free space left on disk       //
//////////////////////////////////////////////////:disk:/*/
long DosGetDiskFree(int drive)
{
    struct diskfree_t spc;
    _dos_getdiskfree(drive, &spc);
    return ((long)spc.avail_clusters * (long)spc.sectors_per_cluster *
     (long)spc.bytes_per_sector);
}

/*/////////////////////////////////////////////////////////
//  DosSetFileTime -- set file date/time                 //
//////////////////////////////////////////////////:disk:/*/
int DosSetFileTime(int handle, unsigned date, unsigned time)
{
    return (_dos_setftime(handle, date, time));
}
#endif


/*/////////////////////////////////////////////////////////////////
//                                                               //
//                                                               //
//      Turbo C  /  Turbo C++                                    //
//                                                               //
//                                                               //
//////////////////////////////////////////////////////////////// */
#ifdef __TURBOC__

#include <dos.h>
#include <dir.h>
#include <io.h>
/*/////////////////////////////////////////////////////////
//  DosFindFirst -- find file (1st instance)             //
//////////////////////////////////////////////////:disk:/*/
int DosFindFirst(char *pathname, int atrib, struct ffblk *fstruc)
{
    return (findfirst(pathname, fstruc, atrib));
}

/*/////////////////////////////////////////////////////////
//  DosFindNext -- find file (except 1st instance)       //
//////////////////////////////////////////////////:disk:/*/
int DosFindNext(struct ffblk *fstruc)
{
    return (findnext(fstruc));
}

/*/////////////////////////////////////////////////////////
//  DosGetDiskFree -- gets free space left on disk       //
//////////////////////////////////////////////////:disk:/*/
long DosGetDiskFree(int drive)
{
    struct dfree spc;
    getdfree(drive, &spc);
    return ((long)spc.df_avail * (long)spc.df_sclus * (long)spc.df_bsec);
}

/*/////////////////////////////////////////////////////////
//  DosSetFileTime -- set file date/time                 //
//////////////////////////////////////////////////:disk:/*/
int DosSetFileTime(int handle, unsigned date, unsigned time)
{
    unsigned ftim = (date << 8) | time;
    return (setftime(handle, (struct ftime *)&ftim));
}

/*/////////////////////////////////////////////////////////
//  _bios_keybrd --  link to Turbo C/C++ bios key        //
/////////////////////////////////////////////////////////*/
int _bios_keybrd(int flag)
{
    return bioskey(flag);
}
#endif


/*/////////////////////////////////////////////////////////////////
//                                                               //
//                                                               //
//      Zortech C  /  Zortech C++                                //
//                                                               //
//                                                               //
//////////////////////////////////////////////////////////////// */
#ifdef __ZTC__

#define MSDOS 1
#include <dos.h>
long timezone = 0L;

/*/////////////////////////////////////////////////////////
//  DosFindFirst -- find file (1st instance)             //
//////////////////////////////////////////////////:disk: */
int DosFindFirst(char *pathname, int atrib, struct find_t *fstruc)
{
    return (_dos_findfirst(pathname, atrib, fstruc));
}

/*/////////////////////////////////////////////////////////
//  DosFindNext -- find file (except 1st instance)       //
//////////////////////////////////////////////////:disk: */
int DosFindNext(struct find_t *fstruc)
{
    return (_dos_findnext(fstruc));
}

/*/////////////////////////////////////////////////////////
//  DosGetDiskFree -- gets free space left on disk       //
//////////////////////////////////////////////////:disk: */
long DosGetDiskFree(int drive)
{
    return (dos_getdiskfreespace(drive));
}

/*/////////////////////////////////////////////////////////
//  DosSetFileTime -- set file date/time                 //
//////////////////////////////////////////////////:disk: */
int DosSetFileTime(int handle, unsigned date, unsigned time)
{
    return (dos_setftime(handle, date, time));
}
#endif

