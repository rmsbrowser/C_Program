echo off
cls
echo This batch file converts all the COMM_? composite libraries to Turbo C
echo format.  The COMM_? libraries and the TC_X? libraries must be in the
echo current directory or passed as an argument to the batch file.  TLIB.EXE,
echo Turbo's LIB program, must be in the path.
echo   Ex: tclibs c:\turboc\lib\
echo In the above example both the COMM and TC libs will be expected to be in
echo the c:\turboc\lib directory.  The trailing backslash is required.
echo ÿ
echo Press control-C to abort the batch file or,
pause
echo on
if not exist %1tc_xs.lib goto medium
if exist %1comm_s.lib tlib %1comm_s /C -pushpop -fpushpop -int14cts -watchdog -allocrb +%1tc_xs.lib,nul
if exist %1comm_s.bak del %1comm_s.bak
:medium
if not exist %1tc_xm.lib goto compact
if exist %1comm_m.lib tlib %1comm_m /C -pushpop -fpushpop -int14cts -watchdog -allocrb +%1tc_xm.lib,nul
if exist %1comm_m.bak del %1comm_m.bak
:compact
if not exist %1tc_xc.lib goto large
if exist %1comm_c.lib tlib %1comm_c /C -pushpop -fpushpop -int14cts -watchdog -allocrb +%1tc_xc.lib,nul
if exist %1comm_c.bak del %1comm_c.bak
:large
if not exist %1tc_xl.lib goto finished
if exist %1comm_l.lib tlib %1comm_l /C -pushpop -fpushpop -int14cts -watchdog -allocrb +%1tc_xl.lib,nul
if exist %1comm_l.bak del %1comm_l.bak
:huge
if not exist %1tc_xl.lib goto finished
if not exist %1comm_l.lib goto finished
copy %1comm_l.lib %1comm_h.lib
tlib %1comm_h /C @mktccomh.rsp
if exist %1comm_h.bak del %1comm_h.bak
:finished
