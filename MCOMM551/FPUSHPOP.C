
/** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*                                                                           *
*       P U S H  /  P O P   S C R E E N S   T O   F A R   M E M O R Y       *
*                Mike Dumdei, 6 Holly Lane, Texarkana TX  75503             *
*                  Requires ASM modules FPO_SCRN & FPU_SCRN                 *
*                                                                           *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
#if defined(__TURBOC__)
  #define _fmalloc farmalloc
  #define _ffree   farfree
  #include <alloc.h>
#elif defined(__ZTC__)
  #define _fmalloc farmalloc
  #define _ffree   farfree
  #include <dos.h>
#else
  #include <malloc.h>
#endif

int     fpu_scrnd(int, int, int, int, char far *);
int     fpo_scrnd(char far *);

static char far *ScrnPtrAry[10];        /* max of 10 screens may be pushed */
static char far **ScrnPtr = ScrnPtrAry;
static char CurntScrn = -1;

/*
        f p u s h s c r n
*/
int fpushscrn(int row, int col, int nrows, int ncols)
{
    if (CurntScrn == 9)
        return (-1);                 /* can't push it if 10 already pushed */
    if ((*ScrnPtr = _fmalloc(2 * nrows * ncols + 16)) == (char far *)0)
        return (-2);                    /* can't push it if no memory left */
    fpu_scrnd(row, col, nrows, ncols, *ScrnPtr);
    CurntScrn++, ScrnPtr++;
    return (0);         /* push scrn, update variables, and return success */
}

/*
        f p o p s c r n
*/
int fpopscrn(void)
{
    if (CurntScrn < 0)
        return (-1);                         /* can't pop it if not pushed */
    --CurntScrn, --ScrnPtr;
    fpo_scrnd(*ScrnPtr);
    _ffree(*ScrnPtr);
    return (0);         /* adj vars, pop the screen, free the mem, & retrn */
}



