@echo off

:wdf19

rmdir /S /Q %~dp0\objfre_win7_x86

call C:\WinDDK\7600.16385.1\bin\setenv.bat C:\WinDDK\7600.16385.1\ fre x86 WIN7
call C:\WinDDK\7600.16385.1\bin\setwdf.bat

cd /d %~dp0

build

copy /B /V /Y %~dp0\objfre_win7_x86\i386\cmntpm.sys %~dp0\win7

del %~dp0\buildfre_win7*.*

rmdir /S /Q %~dp0\objfre_win7_x86


