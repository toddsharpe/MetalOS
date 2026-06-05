del "..\build\*.obj"

set CompileIncludes=-I..\external\gnu-efi -I..\external\gnu-efi\x86_64 -I.\ -I..\external

set CompileFlags=/Zc:wchar_t- /std:c++20 /GS- /Gy /Gw /Z7 %CompileIncludes%
set LinkFlags=/OPT:REF /NODEFAULTLIB

cl Boot\EfiMain.cpp /Fo"..\build\EfiMain" %CompileFlags% /link %LinkFlags% /map /DYNAMICBASE:NO /SUBSYSTEM:EFI_APPLICATION /ENTRY:"EfiMain" /OUT:"..\build\BOOTX64.efi" || exit /b 1

del "..\build\*.obj"
