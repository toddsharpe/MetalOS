@echo off
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" > nul 2>&1
cd /d %~dp0
mkdir ..\build_tests 2>nul
cl Test.cpp /Fo"..\build_tests\Test" /std:c++20 /GS- /Gy /Gw /GR- /Z7 -I..\src\ /link /OPT:REF /OUT:"..\build_tests\Test.exe" /DEBUG /PDB:"..\build_tests\Test.pdb"
