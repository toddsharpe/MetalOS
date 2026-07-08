@set VHD=%CD%\build\MetalOS.vhdx
@set VHD_SCRIPT=%CD%\build\diskpart.txt
@set BUILD_OUT=%CD%\build\
@del %VHD% >nul 2>&1
@del %VHD_SCRIPT% >nul 2>&1

@rem Build a VHD if requested

@(
echo create vdisk file=%VHD% maximum=500
echo select vdisk file=%VHD%
echo attach vdisk
echo convert gpt
echo create partition efi size=100
echo format quick fs=fat32 label="System"
echo assign letter="X"
echo exit
)>%VHD_SCRIPT%

diskpart /s %VHD_SCRIPT%

@rem Copy bootloader to VHD
xcopy %BUILD_OUT%\BOOTX64.EFI X:\EFI\BOOT\

@rem Copy kernel and kernel libraries
xcopy %BUILD_OUT%\moskrnl.exe X:\EFI\BOOT\
xcopy %BUILD_OUT%\moskrnl.pdb X:\EFI\BOOT\
xcopy %BUILD_OUT%\kdcom.dll X:\EFI\BOOT\
xcopy %BUILD_OUT%\kdcom.pdb X:\EFI\BOOT\

@rem Copy runtime libraries
xcopy %BUILD_OUT%\MetalOS-RT.dll X:\EFI\BOOT\
xcopy %BUILD_OUT%\MetalOS-RT.pdb X:\EFI\BOOT\
xcopy %BUILD_OUT%\MetalOS-CRT.dll X:\EFI\BOOT\
xcopy %BUILD_OUT%\MetalOS-CRT.pdb X:\EFI\BOOT\
xcopy %BUILD_OUT%\MetalOS-WM.dll X:\EFI\BOOT\
xcopy %BUILD_OUT%\MetalOS-WM.pdb X:\EFI\BOOT\
xcopy %BUILD_OUT%\MetalOS-NET.dll X:\EFI\BOOT\
xcopy %BUILD_OUT%\MetalOS-NET.pdb X:\EFI\BOOT\

@rem Copy metalos system apps
xcopy %BUILD_OUT%\MetalOS-WMSvr.exe X:\EFI\BOOT\
xcopy %BUILD_OUT%\MetalOS-WMSvr.pdb X:\EFI\BOOT\
xcopy %BUILD_OUT%\MetalOS-NetSvr.exe X:\EFI\BOOT\
xcopy %BUILD_OUT%\MetalOS-NetSvr.pdb X:\EFI\BOOT\

@rem Copy apps
xcopy %BUILD_OUT%\calc.exe X:\EFI\BOOT\
xcopy %BUILD_OUT%\fire.exe X:\EFI\BOOT\
xcopy %BUILD_OUT%\hello.exe X:\EFI\BOOT\
xcopy %BUILD_OUT%\init.exe X:\EFI\BOOT\
xcopy %BUILD_OUT%\init.pdb X:\EFI\BOOT\
xcopy %BUILD_OUT%\term.exe X:\EFI\BOOT\
xcopy %BUILD_OUT%\time.exe X:\EFI\BOOT\
xcopy %BUILD_OUT%\ps.exe X:\EFI\BOOT\
xcopy %BUILD_OUT%\nettest.exe X:\EFI\BOOT\
xcopy %BUILD_OUT%\dhcp.exe X:\EFI\BOOT\
xcopy %BUILD_OUT%\ping.exe X:\EFI\BOOT\
xcopy %BUILD_OUT%\crash.exe X:\EFI\BOOT\
xcopy %BUILD_OUT%\crash.pdb X:\EFI\BOOT\

@rem Copy Doom!!!
xcopy %BUILD_OUT%\doom.exe X:\EFI\BOOT\
@rem xcopy "C:\Program Files (x86)\Steam\steamapps\common\Doom 2\base\DOOM2.WAD" X:\EFI\BOOT\doom2.wad
xcopy "C:\Program Files (x86)\Steam\steamapps\common\Ultimate Doom\base\DOOM.WAD" X:\EFI\BOOT\
xcopy "C:\Program Files (x86)\Steam\steamapps\common\Ultimate Doom\base\DEFAULT.CFG" X:\EFI\BOOT\

@(
echo select vdisk file=%VHD%
echo select partition 2
echo remove letter=X
echo detach vdisk
echo exit
)>%VHD_SCRIPT%

diskpart /s %VHD_SCRIPT%
@del %VHD_SCRIPT% >nul 2>&1

@rem Fix Perms
@rem Get-VM 'MetalOS' | Select-Object VMID
icacls %VHD% /grant 586ce92d-0116-4bc6-a768-34376e124099:F
