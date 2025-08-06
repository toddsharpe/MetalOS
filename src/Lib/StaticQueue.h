#pragma once

#include "Lib\CircularBuffer.h"

template <typename T, size_t N>
using StaticQueue = CircularBuffer<T, N>;
