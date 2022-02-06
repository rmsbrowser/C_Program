echo off
cls
echo This batch file converts all the COMM_? composite libraries to Microsoft
echo C format.  The COMM_? libraries and the MSC_X? libraries must be in the
echo current directory or passed as an argument to the batch file.  LIB.EXE
echo must be in the path.
echo   Ex: msclibs c:\lib\
echo In the above example both the COMM_? and MSC_X? libs will be expected to
echo be in the c:\lib directory.  The trailing backslash is required.
echo ÿ
echo Press control-C to abort the batch file or,
pause
echo on
if not exist %1msc_xs.lib goto medium
if exist %1comm_s.lib lib %1comm_s  -pushpop -fpushpop -int14cts -watchdog -allocrb +%1msc_xs.lib;
if exist %1comm_s.bak del %1comm_s.bak
:medium
if not exist %1msc_xm.lib goto compact
if exist %1comm_m.lib lib %1comm_m  -pushpop -fpushpop -int14cts -watchdog -allocrb +%1msc_xm.lib;
if exist %1comm_m.bak del %1comm_m.bak
:compact
if not exist %1msc_xc.lib goto large
if exist %1comm_c.lib lib %1comm_c  -pushpop -fpushpop -int14cts -watchdog -allocrb +%1msc_xc.lib;
if exist %1comm_c.bak del %1comm_c.bak
:large
if not exist %1msc_xl.lib goto huge
if exist %1comm_l.lib lib %1comm_l  -pushpop -fpushpop -int14cts -watchdog -allocrb +%1msc_xl.lib;
if exist %1comm_l.bak del %1comm_l.bak
:huge
if exist %1comm_h.lib del comm_h.lib
