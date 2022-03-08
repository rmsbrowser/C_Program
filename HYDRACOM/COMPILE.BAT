bcc -c -ms -f- -G -O -Z -d -w+ -H=HYDRACOM.SYM -D__PROTO__ *.C 
pause
tasm /mx /v /z /zi /w2 /DMODL=small *.ASM
pause
rem  tlink /x/c/d 
