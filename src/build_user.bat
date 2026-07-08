del "..\build\*.obj"

set CompileIncludes=-I.\ -I.\..\external -I.\User -I.\std

set CompileFlags=/std:c++20 /GS- /Gy /Gw /GR- /Z7 %CompileIncludes% /X
set LinkFlags=/OPT:REF /NODEFAULTLIB
set UserLibs=..\build\MetalOS-RT.lib ..\build\MetalOS-CRT.lib

:: Build MetalOS-RT.dll
ml64 /c /Fo"..\build\mrt_x64" user\MetalOS-RT\syscalls.asm || exit /b 1
cl /c user/MetalOS-RT/DllMain.cpp /Fo"..\build\MetalOS-RT" %CompileFlags% || exit /b 1
link /DLL ..\build\mrt_x64.obj ..\build\MetalOS-RT.obj %LinkFlags% /SUBSYSTEM:NATIVE /DYNAMICBASE:NO /BASE:"0x0000000180000000" /DEF:"user/MetalOS-RT/MetalOS-RT.def" /OUT:"..\build\MetalOS-RT.dll" /DEBUG /PDB:"..\build\MetalOS-RT.pdb" || exit /b 1

:: Build MetalOS-CRT.dll (Linker warning LNK4210: .CRT section exists; there may be unhandled static initializers or terminators)
cl user/MetalOS-CRT/DllMain.cpp /Fo"..\build\MetalOS-CRT" %CompileFlags% /link %LinkFlags% /SUBSYSTEM:NATIVE /DLL ..\build\MetalOS-RT.lib /DYNAMICBASE:NO /BASE:"0x00000001C0000000" /DEF:"user/MetalOS-CRT/MetalOS-CRT.def" /OUT:"..\build\MetalOS-CRT.dll" /DEBUG /PDB:"..\build\MetalOS-CRT.pdb" /ignore:4210 || exit /b 1

:: Build MetalOS-WM.dll (windowing client -> IPC to the WM process; the "user32" analog)
cl user/MetalOS-WM/DllMain.cpp /Fo"..\build\MetalOS-WM" %CompileFlags% /link %LinkFlags% /SUBSYSTEM:NATIVE /DLL %UserLibs% /DYNAMICBASE:NO /BASE:"0x0000000200000000" /DEF:"user/MetalOS-WM/MetalOS-WM.def" /OUT:"..\build\MetalOS-WM.dll" /DEBUG /PDB:"..\build\MetalOS-WM.pdb" || exit /b 1

:: Build MetalOS-NET.dll (socket API -> IPC to the netstack process; the "winsock" analog)
cl user/MetalOS-NET/DllMain.cpp /Fo"..\build\MetalOS-NET" %CompileFlags% /link %LinkFlags% /SUBSYSTEM:NATIVE /DLL %UserLibs% /DYNAMICBASE:NO /BASE:"0x0000000240000000" /DEF:"user/MetalOS-NET/MetalOS-NET.def" /OUT:"..\build\MetalOS-NET.dll" /DEBUG /PDB:"..\build\MetalOS-NET.pdb" || exit /b 1

:: GUI apps additionally link the windowing client
set GuiLibs=%UserLibs% ..\build\MetalOS-WM.lib

:: Net apps additionally link the socket client
set NetLibs=%UserLibs% ..\build\MetalOS-NET.lib

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

:: Build ps.exe
cl user/ps/Main.cpp /Fo"..\build\ps" %CompileFlags% /link %LinkFlags% /SUBSYSTEM:CONSOLE %UserLibs% /ENTRY:"main" /OUT:"..\build\ps.exe" /DEBUG /PDB:"..\build\ps.pdb" || exit /b 1

:: Build NetTest.exe (links the socket client)
cl user/NetTest/Main.cpp /Fo"..\build\nettest" %CompileFlags% /link %LinkFlags% /SUBSYSTEM:CONSOLE %NetLibs% /ENTRY:"main" /OUT:"..\build\nettest.exe" /DEBUG /PDB:"..\build\nettest.pdb" || exit /b 1

:: Build dhcp.exe (links the socket client)
cl user/dhcp/Main.cpp /Fo"..\build\dhcp" %CompileFlags% /link %LinkFlags% /SUBSYSTEM:CONSOLE %NetLibs% /ENTRY:"main" /OUT:"..\build\dhcp.exe" /DEBUG /PDB:"..\build\dhcp.pdb" || exit /b 1

:: Build ping.exe (links the socket client)
cl user/ping/Main.cpp /Fo"..\build\ping" %CompileFlags% /link %LinkFlags% /SUBSYSTEM:CONSOLE %NetLibs% /ENTRY:"main" /OUT:"..\build\ping.exe" /DEBUG /PDB:"..\build\ping.pdb" || exit /b 1

::
:: Usermode system services (compositor + network stack)
::
cl user/MetalOS-WMSvr/Main.cpp /Fo"..\build\MetalOS-WMSvr" %CompileFlags% /link %LinkFlags% /SUBSYSTEM:NATIVE %UserLibs% /ENTRY:"main" /OUT:"..\build\MetalOS-WMSvr.exe" /DEBUG /PDB:"..\build\MetalOS-WMSvr.pdb" || exit /b 1

:: Build MetalOS-NetSvr.exe (usermode TCP/IP stack; claims the kernel RX ring + NIC MAC).
:: /Zc:threadSafeInit- drops the _Init_thread_* CRT deps (netstack is single-threaded).
cl user/MetalOS-NetSvr/Main.cpp /Fo"..\build\MetalOS-NetSvr" %CompileFlags% /Zc:threadSafeInit- /link %LinkFlags% /SUBSYSTEM:NATIVE %UserLibs% /ENTRY:"main" /OUT:"..\build\MetalOS-NetSvr.exe" /DEBUG /PDB:"..\build\MetalOS-NetSvr.pdb" || exit /b 1

::
:: GUI apps (link the windowing client, MetalOS-WM.lib)
::

:: Build Calc.exe
cl user/Calc/Main.cpp /Fo"..\build\calc" %CompileFlags% /link %LinkFlags% /SUBSYSTEM:NATIVE %GuiLibs% /ENTRY:"main" /OUT:"..\build\calc.exe" /DEBUG /PDB:"..\build\calc.pdb" || exit /b 1

:: Build Fire.exe
cl user/Fire/Main.cpp /Fo"..\build\fire" %CompileFlags% /link %LinkFlags% /SUBSYSTEM:NATIVE %GuiLibs% /ENTRY:"main" /OUT:"..\build\fire.exe" /DEBUG /PDB:"..\build\fire.pdb" || exit /b 1

:: Build Terminal.exe
cl user/Terminal/Main.cpp /Fo"..\build\term" %CompileFlags% /link /SUBSYSTEM:NATIVE %LinkFlags% %GuiLibs% /ENTRY:"main" /OUT:"..\build\term.exe" /DEBUG /PDB:"..\build\term.pdb" || exit /b 1

:: Build Doom.exe
cl /c /Tc user/Doom/Doom.c /Fo"..\build\doom_generic" -I.\ -IExternal\MetalOS.Doom\doomgeneric -I.\User\Doom /GS- /Gy /Gw /GR- /Z7 %CompileIncludes% /X || exit /b 1
cl user/Doom/doomgeneric_metalos.cpp %CompileFlags% -IExternal\MetalOS.Doom\doomgeneric /W3 /std:c++14 /Fo"..\build\doom" /link /SUBSYSTEM:NATIVE ..\build\doom_generic.obj %LinkFlags% %GuiLibs% /ENTRY:"main" /OUT:"..\build\doom.exe" /DEBUG /PDB:"..\build\doom.pdb" || exit /b 1

del "..\build\*.obj"
