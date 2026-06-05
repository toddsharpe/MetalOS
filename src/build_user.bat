del "..\build\*.obj"

set CompileIncludes=-I.\ -I.\..\external -I.\User

set CompileFlags=/std:c++20 /GS- /Gy /Gw /GR- /Z7 %CompileIncludes% /X
set LinkFlags=/OPT:REF /NODEFAULTLIB
set UserLibs=..\build\mosrt.lib ..\build\crt.lib

:: Build MRT.dll
ml64 /c /Fo"..\build\mrt_x64" user\MRT\syscalls.asm || exit /b 1
cl /c user/MRT/DllMain.cpp /Fo"..\build\mosrt" %CompileFlags% || exit /b 1
link /DLL ..\build\mrt_x64.obj ..\build\mosrt.obj %LinkFlags% /SUBSYSTEM:NATIVE /DYNAMICBASE:NO /BASE:"0x0000000180000000" /DEF:"user/MRT/MRT.def" /OUT:"..\build\mosrt.dll" /DEBUG /PDB:"..\build\mosrt.pdb" || exit /b 1

:: Build CRT.dll (Linker warning LNK4210: .CRT section exists; there may be unhandled static initializers or terminators)
cl user/crt/DllMain.cpp /Fo"..\build\crt" %CompileFlags% /link %LinkFlags% /SUBSYSTEM:NATIVE /DLL ..\build\mosrt.lib /DYNAMICBASE:NO /BASE:"0x00000001C0000000" /DEF:"user/crt/CRT.def" /OUT:"..\build\crt.dll" /DEBUG /PDB:"..\build\crt.pdb" /ignore:4210 || exit /b 1

::
:: Init process
::
:: Build Init.exe
cl user/Init/Main.cpp /Fo"..\build\init" %CompileFlags% /link %LinkFlags% /SUBSYSTEM:NATIVE %UserLibs% /ENTRY:"main" /OUT:"..\build\init.exe" /DEBUG /PDB:"..\build\init.pdb" || exit /b 1

::
:: Console Apps
::
:: Build Crash.exe
cl user/Crash/Main.cpp /Fo"..\build\crash" %CompileFlags% /link %LinkFlags% /SUBSYSTEM:CONSOLE %UserLibs% /ENTRY:"main" /OUT:"..\build\crash.exe" /DEBUG /PDB:"..\build\crash.pdb" || exit /b 1

:: Build Hello.exe
cl user/Hello/Main.cpp /Fo"..\build\hello" %CompileFlags% /link %LinkFlags% /SUBSYSTEM:CONSOLE %UserLibs% /ENTRY:"main"  /OUT:"..\build\hello.exe" /DEBUG /PDB:"..\build\hello.pdb" || exit /b 1

:: Build Time.exe
cl user/Time/Main.cpp /Fo"..\build\time" %CompileFlags% /link %LinkFlags% /SUBSYSTEM:CONSOLE %UserLibs% /ENTRY:"main" /OUT:"..\build\time.exe" /DEBUG /PDB:"..\build\time.pdb" || exit /b 1

::
:: GUI apps
::

:: Build Calc.exe
cl user/Calc/Main.cpp /Fo"..\build\calc" %CompileFlags% /link %LinkFlags% /SUBSYSTEM:NATIVE %UserLibs% /ENTRY:"main" /OUT:"..\build\calc.exe" /DEBUG /PDB:"..\build\calc.pdb" || exit /b 1

:: Build Fire.exe
cl user/Fire/Main.cpp /Fo"..\build\fire" %CompileFlags% /link %LinkFlags% /SUBSYSTEM:NATIVE %UserLibs% /ENTRY:"main" /OUT:"..\build\fire.exe" /DEBUG /PDB:"..\build\fire.pdb" || exit /b 1

:: Build Terminal.exe
cl user/Terminal/Main.cpp /Fo"..\build\term" %CompileFlags% /link /SUBSYSTEM:NATIVE %LinkFlags% %UserLibs% /ENTRY:"main" /OUT:"..\build\term.exe" /DEBUG /PDB:"..\build\term.pdb" || exit /b 1

:: Build Doom.exe
cl /c /Tc user/Doom/Doom.c /Fo"..\build\doom_generic" -I.\ -IExternal\MetalOS.Doom\doomgeneric -I.\User\Doom /GS- /Gy /Gw /GR- /Z7 %CompileIncludes% /X || exit /b 1
cl user/Doom/doomgeneric_metalos.cpp %CompileFlags% -IExternal\MetalOS.Doom\doomgeneric /W3 /std:c++14 /Fo"..\build\doom" /link /SUBSYSTEM:NATIVE ..\build\doom_generic.obj %LinkFlags% %UserLibs% /ENTRY:"main" /OUT:"..\build\doom.exe" /DEBUG /PDB:"..\build\doom.pdb" || exit /b 1

del "..\build\*.obj"
