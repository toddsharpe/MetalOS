#pragma once

//CRT
#include "core_crt/stdint.h"
#include "core_crt/stdarg.h"
#include "core_crt/stdlib.h"
#include "core_crt/stdio.h"
#include "core_crt/wchar.h"
#include "core_crt/string.h"
#include "core_crt/ctype.h"

//MetalOS defines
#include "kernel/Arch.h"
#include "MetalOS.Loader.h"
#include "MetalOS.Space.h"

//Windows PE types
#include "windows/types.h"
#include "windows/winnt.h"

//Lib includes
#include "Lib/Bitset.h"
#include "Lib/Buffer.h"
#include "Lib/List.h"
#include "Lib/Math.h"
#include "Lib/ObjectPool.h"
#include "Lib/Path.h"
#include "Lib/Arena.h"
#include "Lib/StaticHeap.h"
#include "Lib/StaticMap.h"
#include "Lib/StaticVector.h"
#include "Lib/String.h"
#include "Lib/System.h"
#include "Lib/Time.h"
#include "WinPE.h"

//Graphics lib
#include "Graphics/Types.h"

//Kernel includes
#include "kernel/KeDisable.h"
#include "kernel/ConfigTables.h"
#include "kernel/KThread.h"
#include "kernel/UThread.h"
#include "kernel/KProcess.h"
#include "kernel/UProcess.h"
#include "kernel/Scheduler.h"
#include "kernel/LoadingScreen.h"
#include "kernel/InterruptTable.h"
#include "kernel/DeviceTree.h"
#include "kernel/Loader.h"
#include "kernel/Objects/KPipe.h"
#include "kernel/Debugger.h"

//Platform
#include "kernel/HyperV/HyperV.h"

//PDB
#include "kernel/Pdb/PdbFile.h"

//Drivers
#include "Drivers/DirectUart.h"
#include "Drivers/RamDrive.h"

//Kd64
#include "reactos/amd64\ke.h"
#include "reactos/amd64/ketypes.h"
#include "kernel/Kd64/kd64.h"

/*
 * Kernel parameters.
 */
static constexpr size_t MaxUProcess = 32;
static constexpr size_t TempArenaSize = Arch::PageSize << 10; //4MB Temp Arena
