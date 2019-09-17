@set VHD=%CD%\out\MetalOS.vhdx
@set VHD_SCRIPT=%CD%\out\diskpart.txt
@del %VHD_SCRIPT% >nul 2>&1

@(
echo select vdisk file=%VHD%
echo select partition 2
remove letter="X"
echo detach vdisk
echo exit
)>%VHD_SCRIPT%

diskpart /s %VHD_SCRIPT%
@del %VHD_SCRIPT% >nul 2>&1