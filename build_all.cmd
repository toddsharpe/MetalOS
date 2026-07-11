@echo off
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1
cd /d "c:\Users\todds\GitHub\toddsharpe\MetalOS\src"
call "c:\Users\todds\GitHub\toddsharpe\MetalOS\src\build_kernel.bat"
echo KERNEL_EXIT=%errorlevel%
