@ECHO OFF
SET GCCPATH="%PROGRAMFILES%\mingw-w64\x86_64-8.1.0-win32-seh-rt_v6-rev0\mingw64\bin"
SET SRC=.\src
SET OUT=.\build

IF "%2"=="/c" cls

REM ECHO ============ Starting Compilation ============
%GCCPATH%\gcc.exe %SRC%\main.c -o %OUT%\out.exe

IF "%1"=="/r" %OUT%\out.exe -i .\test\bool.n --debug
