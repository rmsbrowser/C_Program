
/** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*                                                                           *
*       P U S H  /  P O P   S C R E E N S   T O   N E A R   M E M O R Y     *
*                Mike Dumdei, 6 Holly Lane, Texarkana TX  75503             *
*                    Requires ASM modules PO_SCRN & PU_SCRN                 *
*                                                                           *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
#if defined(__TURBOC__)
  #include <alloc.h>
#elif defined(__ZTC__)
  #include <stdlib.h>
#else
  #include <malloc.h>
#endif

int pu_scrnd(int, int, int, int, char *);
int po_scrnd(char *);

static char *ScrnPtrAry[10];            /* max of 10 screens may be pushed */
static char **ScrnPtr = ScrnPtrAry;
static char CurntScrn = -1;

/*
        p u s h s c r n
*/
int pushscrn(int row, int col, int nrows, int ncols)
{
    if (CurntScrn == 9)
        return (-1);                 /* can't push it if 10 already pushed */
    if ((*ScrnPtr = malloc(2 * nrows * ncols + 16)) == (char *)0)
        return (-2);                    /* can't push it if no memory left */
    pu_scrnd(row, col, nrows, ncols, *ScrnPtr);
    CurntScrn++, ScrnPtr++;
    return (0);         /* push scrn, update variables, and return success */
}

/*
        p o p s c r n
*/
int popscrn(void)
{
    if (CurntScrn < 0)
        return (-1);                         /* can't pop it if not pushed */
    --CurntScrn, --ScrnPtr;
    po_scrnd(*ScrnPtr);
    free(*ScrnPtr);
    return (0);         /* adj vars, pop the screen, free the mem, & retrn */
}


