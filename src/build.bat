del "..\build\*.obj"

mkdir "..\build"
call build_boot.bat || exit /b 1
call build_kernel.bat || exit /b 1
call build_user.bat || exit /b 1
