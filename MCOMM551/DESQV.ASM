
title DESQVIEW INTERFACE FOR VIDEO & TIME SLICING
page 75,132

;extracted from DVTECH.NOT provided by Quarterdeck
;modified for use with MCOMM Async Lib

comment |==================================================================

Functions used to test for presence of DesqView, get DVs video seg
address, and release time to DV.  Does not reset v_seg variable in MCOMM
video code.  You must do that on return from 'TestDesqView' if you want the
MCOMM video code to be DesqView aware.  You will also need to set v_snow
to 0 if DesqView is detected and v_seg != DV_VideoSeg.  Must call
'TestDesqView' before other functions will work.
==========================================================================|
IFDEF ??version                 ;if TASM
        MASM51
        QUIRKS
ENDIF

IFNDEF  model
        .MODEL  small, c
ELSE
  %     .MODEL model, c
ENDIF

        .CODE

DVPresent       DB      0

;*/////////////////////////////////////////////////////////
;/  TestDesqView - tests if running under DesqView       //
;/   returns: DV version number if under DV, else 0      //
;/////////////////////////////////////////////////////// */
TestDesqView PROC
        mov     ax,2b01h        ; set date & check DV version
        mov     cx,'DE'         ; set CX DX to DESQ
        mov     dx,'SQ'         ; for the date (invalid)
        int     21h             ; call DOS
        cmp     al,0ffh         ; check for invalid
        mov     ax,0
        jz      @F              ; DESQview is not running
        mov     cs:DVPresent,1  ; set 'DesqView present' flag
        mov     ax,bx
 @@:    ret                     ; return version number
TestDesqView ENDP

;*/////////////////////////////////////////////////////////
;/  DV_VideoSeg - gets DesqView alternate video buffer   //
;/   address                                             //
;/////////////////////////////////////////////////////// */
DV_VideoSeg PROC USES di, VidBufAdr
        mov     ax,VidBufAdr
        cmp     cs:DVPresent,0
        je      @F              ;exit if DV not running
        mov     es,ax
        sub     di,di           ;ES:DI = actual hardware video buffer
        mov     ah,0feh
        int     10h             ;get DesqView's video buffer
        mov     ax,es
 @@:    ret                     ;back to caller
DV_VideoSeg ENDP

;*/////////////////////////////////////////////////////////
;/  DV_TimeSlice - releases time to DesqView application //
;/    Must call 'TestDesqView' first.  Does nothing if   //
;/    DesqView was not detected.                         //
;/////////////////////////////////////////////////////// */
DV_TimeSlice PROC
        cmp     cs:DVPresent,0
        je      @F              ; exit if DesqView not detected
        mov     ax,101ah
        int     15h             ; switch to DV stack
        mov     ax,1000h        ; DV Pause API function call
        int     15h
        mov     ax,1025h
        int     15h             ;switch back to user stack
 @@:    ret                     ;back to caller
DV_TimeSlice ENDP

        END

