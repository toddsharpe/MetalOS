del "..\build\*.obj"
mkdir ..\build_tests

set CompileIncludes=-I..\src\

set CompileFlags=/std:c++17 /GS- /Gy /Gw /GR- /Z7 %CompileIncludes%
set LinkFlags=/OPT:REF

cl Test.cpp /Fo"..\build_tests\Test" %CompileFlags% /link %LinkFlags% /OUT:"..\build_tests\Test.exe" /DEBUG /PDB:"..\build_tests\Test.pdb" || exit /b 1
..\build_tests\Test.exe  || exit /b 1
