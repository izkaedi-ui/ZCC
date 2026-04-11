@echo off
cd /d D:\__DOWNLOADS\selforglinux
echo Building...
C:\msys64\ucrt64\bin\gcc.exe -O2 -std=c17 -Wall -Wextra compiler_passes.c -o passes.exe -lm 2> build_err.txt 1> build_out.txt
echo gcc exit code: %ERRORLEVEL%
echo.
type build_err.txt
type build_out.txt
echo.
if exist passes.exe (
    echo Running passes.exe...
    passes.exe
) else (
    echo passes.exe was not created.
)
