@set VHD=%CD%\out\MetalOS.vhdx
@set VHD_SCRIPT=%CD%\out\diskpart.txt
@set BUILD_OUT=%CD%\x64\Debug
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

@rem Copy files to VHD
xcopy %BUILD_OUT%\BOOTX64.EFI X:\EFI\BOOT\
xcopy %BUILD_OUT%\moskrnl.exe X:\EFI\BOOT\
xcopy %BUILD_OUT%\moskrnl.pdb X:\EFI\BOOT\
xcopy %BUILD_OUT%\shell.exe X:\EFI\BOOT\
xcopy %BUILD_OUT%\fire.exe X:\EFI\BOOT\
xcopy %BUILD_OUT%\mosrt.dll X:\EFI\BOOT\

@rem Copy Doom!!!
xcopy %BUILD_OUT%\doom.exe X:\EFI\BOOT\
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
icacls %VHD% /grant ee572dc7-71d5-4787-a3e2-91738eb43faa:F
