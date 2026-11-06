:: Rebuilds and runs the host unit tests: run.bat [Debug|Release], Debug by default; run from a VS x64 Native Tools prompt
setlocal
set Config=%~1
if "%Config%"=="" set Config=Debug
msbuild "%~dp0Tests.vcxproj" /t:Rebuild /m /nologo /v:m /p:Configuration=%Config% /p:Platform=x64 || exit /b 1
"%~dp0..\build_tests\Test.exe" || exit /b 1
