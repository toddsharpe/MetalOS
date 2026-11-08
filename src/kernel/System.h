#pragma once

#include "core_crt/stdint.h"
#include "Lib/Time.h"

struct KSystemTime
{
	uint16_t Year;
	uint16_t Month;
	uint16_t Day;
	uint16_t Hour;
	uint16_t Minute;
	uint16_t Second;

	//Fractional second in both milli and nano
	uint16_t Milliseconds;
	uint32_t Nanoseconds;
};

void KePauseSystem();
void KeResumeSystem();
milli_t KeGetTicks();
nano_t KeGetNanoseconds();
void KeGetSystemTime(KSystemTime& time);
