#pragma once

//Standard includes
#include "core_crt/stdarg.h"
#include "core_crt/stdint.h"
#include "core_crt/stdlib.h"
#include "core_crt/stdio.h"
#include "core_crt/wchar.h"

//Lib
#include "Assert.h"
#include "lib/Path.h"
#include "Lib/Buffer.h"
#include "Lib/String.h"
#include "Lib/System.h"

//MetalOS defines
#include "MetalOS.Loader.h"
#include "MetalOS.Space.h"

//Architecture specific
#include "x64/x64.h"
#include "x64/CpuId.h"
#include "x64/PageTables.h"

//Drivers
#include "Drivers/DirectUart.h"
#include "Drivers/RamDrive.h"

//Windows PE types
#include "windows/types.h"
#include "windows/winnt.h"

//Bootloader includes
#include "Boot/EfiDevice.h"
#include "Boot/EfiDump.h"
#include "Boot/EfiError.h"
#include "Boot/EfiMemory.h"
#include "Boot/EfiPrint.h"
