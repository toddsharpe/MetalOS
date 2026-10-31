:: Runs from src\ regardless of caller's directory (setlocal restores it on exit)
setlocal
cd /d "%~dp0..\src"

if not exist "..\build" mkdir "..\build"
del "..\build\*.obj"

call "%~dp0build_boot.bat" || exit /b 1
call "%~dp0build_kernel.bat" || exit /b 1
call "%~dp0build_user.bat" || exit /b 1
