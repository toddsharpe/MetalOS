#pragma once

#include "core_crt/stdint.h"

typedef void* (*ReadBlock)(void* const context, const uint32_t blockNumber);
