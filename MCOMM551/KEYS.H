
/*  K E Y S . H   --  k e y   d e f i n i t i o n   h e a d e r

            Mike Dumdei, 6 Holly Lane, Texarkana TX 75503

    NOTE:  EXTENDED KEY HANDLING OPTIONS
        This header allows keys with extended codes to be defined 3
        different ways depending on what is defined when the file
        is included in the source.

        KEY_INT defined:
            The scan code of the key is returned as the high byte
            and the NUL char is returned as the low byte.  This is
            exactly as the the BIOS read key routine returns extended
            keys.   Ex:  F1 == 0x3b00

        KEY_CH defined:
            Extended keys are defined as the value of the scan code.
            This method works well when using the getch() functions --
            on the first call a NUL is returned, on the 2nd the values
            defined using this method is returned.
                Ex:  F1 == 0x3b

        KEY_NEG defined:       * * *  this is the default  * * *
            Extended keys are defined as the inverse of the scan code.
            Use this if you have a function that returns the 'scan code/char'
            for normal keys but returns negative scan values for extended
            keys.  The advantage of this method is if the value returned is
            less than 0, you know an extended key was pressed since the
            scan codes/char combinations don't have negative integer values.
                Ex:  F1 == (0 - 0x3b)
*/


#if !defined(KEY_INT)
    #if !defined(KEY_CH)
        #if !defined(KEY_NEG)
            #define KEY_NEG          /* you get this if nothing else is    */
        #endif                       /*  defined when keydef.h is included */
    #endif
#endif

/* these are the same no matter what is defined */

   /* no scan code version */
#define SPC         0x20
#define ESC         0x1b
#define BKSP        0x08
#define TAB         0x09
#define ENTER       0x0d
#define SHFT_TAB    0x0f
#define CTL_ENTER   0x0a
#define CTL_BKSP    0x7f

    /* with scan codes (X == extended code) */
#define X_SPC       0x3920
#define X_ESC       0x011b
#define X_BKSP      0x0e08
#define X_TAB       0x0f09
#define X_ENTER     0x1c0d
#define X_SHFT_TAB  0x0f00
#define X_CTL_ENTER 0x1c0a
#define X_CTL_BKSP  0x0e7f

#define GREY_AST    0x372a
#define GREY_MIN    0x4a2d
#define GREY_PLS    0x4e2b
#define C_GREY_AST  0x3710
#define S_GREY_PLS  0x4e2b
#define S_GREY_MIN  0x4a2d


/* this is the scan/char combination for extended keys */

#ifdef KEY_INT

/* No Ctl, Shft, or Alt codes */
#define F1          0x3b00
#define F2          0x3c00
#define F3          0x3d00
#define F4          0x3e00
#define F5          0x3f00
#define F6          0x4000
#define F7          0x4100
#define F8          0x4200
#define F9          0x4300
#define F10         0x4400
#define HOME        0x4700
#define UPARO       0x4800
#define PGUP        0x4900
#define LFTARO      0x4b00
#define RGTARO      0x4d00
#define END         0x4f00
#define DNARO       0x5000
#define PGDN        0x5100
#define INS         0x5200
#define DEL         0x5300

/* Shft pressed */
#define SHFT_F1     0x5400
#define SHFT_F2     0x5500
#define SHFT_F3     0x5600
#define SHFT_F4     0x5700
#define SHFT_F5     0x5800
#define SHFT_F6     0x5900
#define SHFT_F7     0x5a00
#define SHFT_F8     0x5b00
#define SHFT_F9     0x5c00
#define SHFT_F10    0x5d00

/* Ctl pressed */
#define CTL_F1      0x5e00
#define CTL_F2      0x5f00
#define CTL_F3      0x6000
#define CTL_F4      0x6100
#define CTL_F5      0x6200
#define CTL_F6      0x6300
#define CTL_F7      0x6400
#define CTL_F8      0x6500
#define CTL_F9      0x6600
#define CTL_F10     0x6700
#define CTL_HOME    0x7700
#define CTL_PGUP    0x8400
#define CTL_LFTARO  0x7300
#define CTL_RGTARO  0x7400
#define CTL_END     0x7500
#define CTL_PGDN    0x7600

/* Alt pressed */
#define ALT_F1      0x6800
#define ALT_F2      0x6900
#define ALT_F3      0x6a00
#define ALT_F4      0x6b00
#define ALT_F5      0x6c00
#define ALT_F6      0x6d00
#define ALT_F7      0x6e00
#define ALT_F8      0x6f00
#define ALT_F9      0x7000
#define ALT_F10     0x7100
#define ALT_1       0x7800
#define ALT_2       0x7900
#define ALT_3       0x7a00
#define ALT_4       0x7b00
#define ALT_5       0x7c00
#define ALT_6       0x7d00
#define ALT_7       0x7e00
#define ALT_8       0x7f00
#define ALT_9       0x8000
#define ALT_0       0x8100
#define ALT_MIN     0x8200
#define ALT_EQU     0x8300
#define ALT_A       0x1e00
#define ALT_B       0x3000
#define ALT_C       0x2e00
#define ALT_D       0x2000
#define ALT_E       0x1200
#define ALT_F       0x2100
#define ALT_G       0x2200
#define ALT_H       0x2300
#define ALT_I       0x1700
#define ALT_J       0x2400
#define ALT_K       0x2500
#define ALT_L       0x2600
#define ALT_M       0x3200
#define ALT_N       0x3100
#define ALT_O       0x1800
#define ALT_P       0x1900
#define ALT_Q       0x1000
#define ALT_R       0x1300
#define ALT_S       0x1f00
#define ALT_T       0x1400
#define ALT_U       0x1600
#define ALT_V       0x2f00
#define ALT_W       0x1100
#define ALT_X       0x2d00
#define ALT_Y       0x1500
#define ALT_Z       0x2c00

#endif

/* this is the scan code only version of key definitions */

#ifdef  KEY_CH

/* No Ctl, Shft, or Alt codes */
#define F1          0x3b
#define F2          0x3c
#define F3          0x3d
#define F4          0x3e
#define F5          0x3f
#define F6          0x40
#define F7          0x41
#define F8          0x42
#define F9          0x43
#define F10         0x44
#define HOME        0x47
#define UPARO       0x48
#define PGUP        0x49
#define LFTARO      0x4b
#define RGTARO      0x4d
#define END         0x4f
#define DNARO       0x50
#define PGDN        0x51
#define INS         0x52
#define DEL         0x53

/* Shft pressed */
#define SHFT_F1     0x54
#define SHFT_F2     0x55
#define SHFT_F3     0x56
#define SHFT_F4     0x57
#define SHFT_F5     0x58
#define SHFT_F6     0x59
#define SHFT_F7     0x5a
#define SHFT_F8     0x5b
#define SHFT_F9     0x5c
#define SHFT_F10    0x5d

/* Ctl pressed */
#define CTL_F1      0x5e
#define CTL_F2      0x5f
#define CTL_F3      0x60
#define CTL_F4      0x61
#define CTL_F5      0x62
#define CTL_F6      0x63
#define CTL_F7      0x64
#define CTL_F8      0x65
#define CTL_F9      0x66
#define CTL_F10     0x67
#define CTL_HOME    0x77
#define CTL_PGUP    0x84
#define CTL_LFTARO  0x73
#define CTL_RGTARO  0x74
#define CTL_END     0x75
#define CTL_PGDN    0x76

/* Alt pressed */
#define ALT_F1      0x68
#define ALT_F2      0x69
#define ALT_F3      0x6a
#define ALT_F4      0x6b
#define ALT_F5      0x6c
#define ALT_F6      0x6d
#define ALT_F7      0x6e
#define ALT_F8      0x6f
#define ALT_F9      0x70
#define ALT_F10     0x71
#define ALT_1       0x78
#define ALT_2       0x79
#define ALT_3       0x7a
#define ALT_4       0x7b
#define ALT_5       0x7c
#define ALT_6       0x7d
#define ALT_7       0x7e
#define ALT_8       0x7f
#define ALT_9       0x80
#define ALT_0       0x81
#define ALT_MIN     0x82
#define ALT_EQU     0x83
#define ALT_A       0x1e
#define ALT_B       0x30
#define ALT_C       0x2e
#define ALT_D       0x20
#define ALT_E       0x12
#define ALT_F       0x21
#define ALT_G       0x22
#define ALT_H       0x23
#define ALT_I       0x17
#define ALT_J       0x24
#define ALT_K       0x25
#define ALT_L       0x26
#define ALT_M       0x32
#define ALT_N       0x31
#define ALT_O       0x18
#define ALT_P       0x19
#define ALT_Q       0x10
#define ALT_R       0x13
#define ALT_S       0x1f
#define ALT_T       0x14
#define ALT_U       0x16
#define ALT_V       0x2f
#define ALT_W       0x11
#define ALT_X       0x2d
#define ALT_Y       0x15
#define ALT_Z       0x2c

#endif

/* this is the inverse scan code definitions */

#ifdef KEY_NEG

/* No Ctl, Shft, or Alt codes */
#define F1          (0 - 0x3b)
#define F2          (0 - 0x3c)
#define F3          (0 - 0x3d)
#define F4          (0 - 0x3e)
#define F5          (0 - 0x3f)
#define F6          (0 - 0x40)
#define F7          (0 - 0x41)
#define F8          (0 - 0x42)
#define F9          (0 - 0x43)
#define F10         (0 - 0x44)
#define HOME        (0 - 0x47)
#define UPARO       (0 - 0x48)
#define PGUP        (0 - 0x49)
#define LFTARO      (0 - 0x4b)
#define RGTARO      (0 - 0x4d)
#define END         (0 - 0x4f)
#define DNARO       (0 - 0x50)
#define PGDN        (0 - 0x51)
#define INS         (0 - 0x52)
#define DEL         (0 - 0x53)

/* Shft pressed */
#define SHFT_F1     (0 - 0x54)
#define SHFT_F2     (0 - 0x55)
#define SHFT_F3     (0 - 0x56)
#define SHFT_F4     (0 - 0x57)
#define SHFT_F5     (0 - 0x58)
#define SHFT_F6     (0 - 0x59)
#define SHFT_F7     (0 - 0x5a)
#define SHFT_F8     (0 - 0x5b)
#define SHFT_F9     (0 - 0x5c)
#define SHFT_F10    (0 - 0x5d)

/* Ctl pressed */
#define CTL_F1      (0 - 0x5e)
#define CTL_F2      (0 - 0x5f)
#define CTL_F3      (0 - 0x60)
#define CTL_F4      (0 - 0x61)
#define CTL_F5      (0 - 0x62)
#define CTL_F6      (0 - 0x63)
#define CTL_F7      (0 - 0x64)
#define CTL_F8      (0 - 0x65)
#define CTL_F9      (0 - 0x66)
#define CTL_F10     (0 - 0x67)
#define CTL_HOME    (0 - 0x77)
#define CTL_PGUP    (0 - 0x84)
#define CTL_LFTARO  (0 - 0x73)
#define CTL_RGTARO  (0 - 0x74)
#define CTL_END     (0 - 0x75)
#define CTL_PGDN    (0 - 0x76)

/* Alt pressed */
#define ALT_F1      (0 - 0x68)
#define ALT_F2      (0 - 0x69)
#define ALT_F3      (0 - 0x6a)
#define ALT_F4      (0 - 0x6b)
#define ALT_F5      (0 - 0x6c)
#define ALT_F6      (0 - 0x6d)
#define ALT_F7      (0 - 0x6e)
#define ALT_F8      (0 - 0x6f)
#define ALT_F9      (0 - 0x70)
#define ALT_F10     (0 - 0x71)
#define ALT_1       (0 - 0x78)
#define ALT_2       (0 - 0x79)
#define ALT_3       (0 - 0x7a)
#define ALT_4       (0 - 0x7b)
#define ALT_5       (0 - 0x7c)
#define ALT_6       (0 - 0x7d)
#define ALT_7       (0 - 0x7e)
#define ALT_8       (0 - 0x7f)
#define ALT_9       (0 - 0x80)
#define ALT_0       (0 - 0x81)
#define ALT_MIN     (0 - 0x82)
#define ALT_EQU     (0 - 0x83)
#define ALT_A       (0 - 0x1e)
#define ALT_B       (0 - 0x30)
#define ALT_C       (0 - 0x2e)
#define ALT_D       (0 - 0x20)
#define ALT_E       (0 - 0x12)
#define ALT_F       (0 - 0x21)
#define ALT_G       (0 - 0x22)
#define ALT_H       (0 - 0x23)
#define ALT_I       (0 - 0x17)
#define ALT_J       (0 - 0x24)
#define ALT_K       (0 - 0x25)
#define ALT_L       (0 - 0x26)
#define ALT_M       (0 - 0x32)
#define ALT_N       (0 - 0x31)
#define ALT_O       (0 - 0x18)
#define ALT_P       (0 - 0x19)
#define ALT_Q       (0 - 0x10)
#define ALT_R       (0 - 0x13)
#define ALT_S       (0 - 0x1f)
#define ALT_T       (0 - 0x14)
#define ALT_U       (0 - 0x16)
#define ALT_V       (0 - 0x2f)
#define ALT_W       (0 - 0x11)
#define ALT_X       (0 - 0x2d)
#define ALT_Y       (0 - 0x15)
#define ALT_Z       (0 - 0x2c)

#endif
 
