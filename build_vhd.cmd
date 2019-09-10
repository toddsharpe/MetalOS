@set VHD=%CD%\zerosharp.vhdx
@set VHD_SCRIPT=%CD%\diskpart.txt
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

xcopy BOOTX64.EFI X:\EFI\BOOT\

@(
echo select vdisk file=%VHD%
echo select partition 2
echo remove letter=X
echo detach vdisk
echo exit
)>%VHD_SCRIPT%

diskpart /s %VHD_SCRIPT%
