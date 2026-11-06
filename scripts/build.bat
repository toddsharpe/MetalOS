:: Full rebuild of MetalOS.sln (boot loader, kernel, user mode, host tests) into build\: build.bat [Debug|Release], Debug by default; run from a VS x64 Native Tools prompt
setlocal
set Config=%~1
if "%Config%"=="" set Config=Debug
msbuild "%~dp0..\MetalOS.sln" /t:Rebuild /m /nologo /v:m /p:Configuration=%Config% /p:Platform=x64 || exit /b 1
