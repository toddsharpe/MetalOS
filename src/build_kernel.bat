del "..\build\kernel_acpi.obj"
del "..\build\kernel_64.obj"
del "..\build\moskrnl.obj"

set CompileIncludes=-I.\ -I.\..\external

set CompileFlags=/std:c++17 /GS- /Gy /Gw /GR- /Z7 %CompileIncludes% /X
set LinkFlags=/OPT:REF /NODEFAULTLIB /SUBSYSTEM:NATIVE

:: Compile ACPI
cl /c /Tc Acpi/Acpi.c /Fo"..\build\moskrnl_acpi" -I.\ -IAcpi -IExternal\uACPI\source -IExternal\uACPI\include /GS- /Gy /Gw /GR- /Z7 || exit /b 1

:: Assemble arch support
ml64 /c /Fo"..\build\kernel_64" kernel\Arch_x64\Arch.asm || exit /b 1

:: Compile kernel
cl /c Kernel\Kernel.cpp /Fo"..\build\moskrnl" %CompileFlags% -IAcpi -I..\external\gnu-efi -I..\external\gnu-efi\x86_64 -IExternal\uACPI\include /D"UACPI_OVERRIDE_TYPES" || exit /b 1

:: Linker warning LNK4210: .CRT section exists; there may be unhandled static initializers or terminators
link ..\build\moskrnl_acpi.obj ..\build\moskrnl.obj ..\build\kernel_64.obj /DYNAMICBASE:NO %LinkFlags% /BASE:"0xFFFF800001000000" /MACHINE:X64 /ENTRY:"ArchMain" /DEF:"Kernel\Kernel.def" /OUT:"..\build\moskrnl.exe" /DEBUG /PDB:"..\build\moskrnl.pdb" /ignore:4210 || exit /b 1

:: Kdcom
cl kdcom/build.cpp /Fo"..\build\kdcom" %CompileFlags% -I..\external\reactos /link /dll /DYNAMICBASE:NO %LinkFlags% /BASE:"0xFFFF800002000000" /MACHINE:X64 /DEF:"kdcom/kdcom.def" /OUT:"..\build\kdcom.dll" /DEBUG /PDB:"..\build\kdcom.pdb" || exit /b 1
