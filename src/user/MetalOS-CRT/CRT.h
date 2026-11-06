#pragma once

//CRT
#include "core_crt/stdint.h"
#include "core_crt/stdarg.h"
#include "core_crt/stdlib.h"
#include "core_crt/stdio.h"
#include "core_crt/string.h"
#include "core_crt/ctype.h"

//Include user CRT
#include "Lib/StaticHeap.h"
#include "user/stdio.h"
#include "user/stdlib.h"
#include "user/string.h"

extern Heap CrtHeap; //The CRT's malloc heap, opened by DllMain on process attach
