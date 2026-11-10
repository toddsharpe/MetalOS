@rem Builds the boot disk without diskpart, so it needs no admin (see BuildVhd.cs)
@setlocal
@for %%I in ("%~dp0..\build") do @set "BUILD_OUT=%%~fI"
@set VHD=%BUILD_OUT%\MetalOS.vhdx
@set DOOM_DATA=C:\Program Files (x86)\Steam\steamapps\common\Ultimate Doom\base

@rem Game data is optional - it isn't available in CI
@set DOOM_FILES=
@if exist "%DOOM_DATA%\DOOM.WAD" set DOOM_FILES="%DOOM_DATA%\DOOM.WAD"
@if not exist "%DOOM_DATA%\DOOM.WAD" echo DOOM.WAD not found, skipping game data
@if exist "%DOOM_DATA%\DEFAULT.CFG" set DOOM_FILES=%DOOM_FILES% "%DOOM_DATA%\DEFAULT.CFG"

@pushd "%BUILD_OUT%"
@rem The EFI system partition holds what the loader loads; the MetalOS partition holds the rest
dotnet run --file "%~dp0BuildVhd.cs" -- "%VHD%" ^
	BOOTX64.EFI moskrnl.exe moskrnl.pdb ^
	--os ^
	kdcom.dll kdcom.pdb ^
	MetalOS-RT.dll MetalOS-RT.pdb MetalOS-CRT.dll MetalOS-CRT.pdb MetalOS-WM.dll MetalOS-WM.pdb MetalOS-NET.dll MetalOS-NET.pdb ^
	MetalOS-WMSvr.exe MetalOS-WMSvr.pdb MetalOS-NetSvr.exe MetalOS-NetSvr.pdb ^
	calc.exe fire.exe hello.exe init.exe init.pdb term.exe time.exe ps.exe nettest.exe dhcp.exe ping.exe crash.exe crash.pdb ^
	doom.exe %DOOM_FILES%
@set FAILED=%ERRORLEVEL%
@popd
@if not %FAILED%==0 echo ERROR: failed to build %VHD%& exit /b %FAILED%

@rem Let the local Hyper-V VM open the disk (its id is from Get-VM 'MetalOS' | Select-Object VMID)
@if not defined CI icacls "%VHD%" /grant 586ce92d-0116-4bc6-a768-34376e124099:F
