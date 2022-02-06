
title DOS / UNIX FILE TIME CONVERSIONS
page 75,132

; (c) 1991, Mike Dumdei, 6 Holly Lane, Texarkana TX  75503

comment |======================================================================

unsigned long UnixToDos(unsigned *DosTime, unsigned *DosDate,
        unsigned long UnixTime);
 Converts Unix file time (secs since 1-1-70 GMT) to DOS file time. The
 result is stored in the variables pointed to by DosTime and DosDate and
 also returned as a long in the form DosDate:DosTime.

unsigned long DosToUnix(unsigned DosTime, unsigned DosDate);
 Converts Dos file time to Unix file time.  The return value of the
 function is the Unix time.

This function does not take into account the difference between the local
time zone and GMT.  Both MSC and TC have a 'timezone' variable that may
be used to adjust the UNIX time values.  MSC defaults to 28800 or +8 hours
(PST) and TC defaults to 18000 or +5 hours (EST).  Zortech C does not have
a timezone variable but functions in it such as 'stat' and 'fstat' that
would normally use 'timezone' have a hard coded value of 25200 or +7 hours
(MST).  There is no 'right' value -- it depends on which time zone you are
in and whether or not DST is in effect.  If an adjustment to local time is
desired, adjust by the number of minutes local time differs from GMT.

==============================================================================|
IFDEF ??version                 ;if TASM
        MASM51
        QUIRKS
ENDIF

IFNDEF model
        .MODEL  small, C
ELSE
 %      .MODEL  model, C
ENDIF
        INCLUDE casm.inc

HICONST EQU     12ceh
LOCONST EQU     0a600h          ;secs between 1-1-70 GMT & 1-1-80 GMT

        .DATA

DtoUtbl         DW      0,31,59,90,120,151,181,212,243,273,304,334
UtoDtbl         DB      0,31,28,31,30,31,30,31,31,30,31,30,31

        .CODE

UnixToDos PROC, DosTime:PTR, DosDate:PTR, UnixTime:WORD;s = 2 words
        mov     ax,UnixTime
        mov     dx,UnixTime[2]  ;DX:AX = secs since 1-1-70 GMT
        sub     ax,LOCONST
        sbb     dx,HICONST      ;DX:AX = secs since 1-1-80 GMT
        jnc     @F              ;jmp if a valid DOS date
        xor     ax,ax
        mov     dx,ax           ;use 1-1-80 localtime if bad date
@@:     shr     dx,1
        rcr     ax,1            ;DX:AX = (secs since 1-1-80) / 2
        mov     bx,43200        ;BX = secs in a day / 2
        div     bx              ;AX = days, DX = secs / 2 remaining
        push    dx              ;save (partial day secs / 2)
        xor     dx,dx
        mov     bx,1461         ;BX = days in 4 year span
        div     bx              ;AX = 4 year spans, DX = excess days
        mov     bx,OFST UtoDtbl ;BX = pointer to days in months array
        shl     ax,1
        shl     ax,1            ;AX = years in complete 4 year spans
        inc     dx              ;days 1 based rather than 0 based
        mov     cx,366
        sub     dx,cx           ;sub days in first year (leap year)
        ja      @F              ;jmp if not a leap year
        mov     BPTR [bx+2],29  ;Feb has 29 days if leap year
        jmp s   calc_month
        mov     BPTR [bx+2],28  ;Feb has 28 days in not a leap year
@@:     dec     cx
@@:     inc     ax              ;add another year to years since 1980
        sub     dx,cx           ;sub days in a year
        ja      @B              ;loop till get to current year
calc_month:
        add     dx,cx           ;subtracted once too many, add back
        xchg    ax,dx           ;AX=days this year, DX=yrs since 1980
@@:     inc     bx
        sub     al,[bx]
        sbb     ah,dh
        or      ax,ax
        jg      @B              ;sub days till get to current month
        add     al,[bx]         ;AL = current day
        sub     bx,OFST UtoDtbl ;BX = current month
        mov     dh,dl
        shl     dh,1            ;DX bits 9-15 = year
        mov     dl,al           ;DX bits 0-4 = day
        mov     cl,5
        shl     bx,cl
        or      dx,bx           ;DX bits 5-8 = month
        pop     ax              ;AX = partial day secs / 2
        push    dx              ;save completed month,day,year
        xor     dx,dx
        mov     bx,1800
        div     bx              ;AX = hours, DX = mins,secs / 2
        mov     cl,3
        shl     al,cl
        mov     bh,al           ;BH bits 11-15 = hour
        mov     ax,dx
        mov     bl,30
        div     bl              ;AL = mins, AH = secs / 2
        mov     bl,ah           ;BL bits 0-4 = secs / 2
        xor     ah,ah
        mov     cl,5
        shl     ax,cl           ;AX bits 5-10 = minute
        or      ax,bx           ;AX = DOS file time
        pLes    bx,DosTime
        mov     FP[bx],ax       ;store DOS time in return variable
        pop     dx              ;DX = DOS file date
        pLes    bx,DosDate
        mov     FP[bx],dx       ;store DOS date in return variable
        ret                     ;back to caller
UnixToDos ENDP

DosToUnix PROC, DosTime, DosDate
        mov     bx,DosDate
        mov     al,bh
        xor     ah,ah
        shr     ax,1            ;AX = DOS year
        mov     ch,bl
        and     ch,1fh
        dec     ch              ;CH = DOS day (0 based)
        and     bx,1e0h
        mov     cl,5
        shr     bx,cl
        dec     bx              ;BX = DOS month (0 based)
        mov     cl,al
        add     cl,3
        shr     cl,1
        shr     cl,1            ;CL = # leap days due to past years
        test    al,3
        jnz     @F              ;jmp if this isn't a leap year
        cmp     bl,1
        jbe     @F              ;jmp if not past leap day yet
        inc     cl              ;CL = total leap days
@@:     add     cl,ch
        xor     ch,ch           ;CX = day of month + all leap days
        shl     bx,1            ;shift month for word table lookup
        add     cx,DtoUtbl[bx]  ;CX = days this year + leap days
        mov     dx,365
        mul     dx              ;AX = days due to years
        add     ax,cx           ;AX = total days since 1-1-80
        mov     dx,43200        ;hour secs / 2 (avoid long multiply)
        mul     dx              ;DX:AX = day secs since 1-1-80 / 2
        shl     ax,1
        rcl     dx,1            ;DX:AX = secs in days since 1-1-80
        push    dx
        push    ax              ;save partial result
        mov     bx,DosTime
        mov     al,bh
        mov     cl,3
        shr     al,cl           ;AL = hours
        mov     ah,60
        mul     ah              ;AX = mins due to hours
        mov     dx,bx
        and     dx,7e0h
        mov     cl,5
        shr     dx,cl           ;DX = minutes from DosTime
        add     ax,dx           ;AX = total minutes
        mov     cx,60
        mul     cx              ;DX:AX = secs in hours and mins
        and     bx,1fh
        shl     bx,1            ;BX = secs field from DosTime
        add     ax,bx
        adc     dx,0            ;DX:AX = total secs from DosTime
        pop     bx
        pop     cx              ;CX:BX = total secs from DosDate
        add     ax,bx
        adc     dx,cx           ;DX:AX = tl secs from Dos timestamp
        add     ax,LOCONST
        adc     dx,HICONST      ;add secs from 1-1-70GMT to 1-1-80GMT
        ret                     ;back to caller
DosToUnix ENDP

        END

