@setlocal
@for %%I in ("%~dp0..\build") do @set "BUILD_OUT=%%~fI"
@set VHD=%BUILD_OUT%\MetalOS.vhdx
@set VHD_SCRIPT=%BUILD_OUT%\diskpart.txt
@set EFI_BOOT=X:\EFI\BOOT\
@set DOOM_DATA=C:\Program Files (x86)\Steam\steamapps\common\Ultimate Doom\base
@set FAILED=0
@set ASSIGNED=
@del "%VHD%" >nul 2>&1
@del "%VHD_SCRIPT%" >nul 2>&1

@rem Build a VHD if requested
@rem Expandable and sized to fit the 100MB EFI partition (at this size convert gpt adds no MSR partition)

@(
echo create vdisk file="%VHD%" maximum=128 type=expandable
echo select vdisk file="%VHD%"
echo attach vdisk
echo convert gpt
echo create partition efi size=100
echo format quick fs=fat32 label="System"
echo assign letter="X"
echo exit
)>"%VHD_SCRIPT%"

diskpart /s "%VHD_SCRIPT%" || (set "FAILED=1" & goto :Detach)
@set ASSIGNED=1

@rem Copy bootloader to VHD
@call :Copy BOOTX64.EFI

@rem Copy kernel and kernel libraries
@call :Copy moskrnl.exe
@call :Copy moskrnl.pdb
@call :Copy kdcom.dll
@call :Copy kdcom.pdb

@rem Copy runtime libraries
@call :Copy MetalOS-RT.dll
@call :Copy MetalOS-RT.pdb
@call :Copy MetalOS-CRT.dll
@call :Copy MetalOS-CRT.pdb
@call :Copy MetalOS-WM.dll
@call :Copy MetalOS-WM.pdb
@call :Copy MetalOS-NET.dll
@call :Copy MetalOS-NET.pdb

@rem Copy metalos system apps
@call :Copy MetalOS-WMSvr.exe
@call :Copy MetalOS-WMSvr.pdb
@call :Copy MetalOS-NetSvr.exe
@call :Copy MetalOS-NetSvr.pdb

@rem Copy apps
@call :Copy calc.exe
@call :Copy fire.exe
@call :Copy hello.exe
@call :Copy init.exe
@call :Copy init.pdb
@call :Copy term.exe
@call :Copy time.exe
@call :Copy ps.exe
@call :Copy nettest.exe
@call :Copy dhcp.exe
@call :Copy ping.exe
@call :Copy crash.exe
@call :Copy crash.pdb

@rem Copy Doom!!! (game data is optional - it isn't available in CI)
@call :Copy doom.exe
@rem xcopy "C:\Program Files (x86)\Steam\steamapps\common\Doom 2\base\DOOM2.WAD" X:\EFI\BOOT\doom2.wad
@if not exist "%DOOM_DATA%\DOOM.WAD" echo DOOM.WAD not found, skipping game data
@if exist "%DOOM_DATA%\DOOM.WAD" xcopy "%DOOM_DATA%\DOOM.WAD" %EFI_BOOT%
@if exist "%DOOM_DATA%\DEFAULT.CFG" xcopy "%DOOM_DATA%\DEFAULT.CFG" %EFI_BOOT%

:Detach
@rem Only release X: if this run assigned it - otherwise it may belong to another volume
@(
if defined ASSIGNED echo select volume=X
if defined ASSIGNED echo remove letter=X
echo select vdisk file="%VHD%"
echo detach vdisk
echo exit
)>"%VHD_SCRIPT%"

diskpart /s "%VHD_SCRIPT%" || set "FAILED=1"
@del "%VHD_SCRIPT%" >nul 2>&1

@rem Fix Perms (local Hyper-V VM only)
@rem Get-VM 'MetalOS' | Select-Object VMID
@if not defined CI icacls "%VHD%" /grant 586ce92d-0116-4bc6-a768-34376e124099:F

@if %FAILED%==1 echo ERROR: failed to build %VHD%
@exit /b %FAILED%

:Copy
@xcopy "%BUILD_OUT%\%~1" %EFI_BOOT% || set "FAILED=1"
@exit /b
