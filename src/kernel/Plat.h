#pragma once

#include "Lib/Time.h"

class Driver;
class KDevice;

//The platform the kernel runs on (Plat_HyperV); generic kernel code reaches hypervisor services only through these
void PlatInitialize();
void PlatStartTimer(const nano_t period, const uint8_t vector);
void PlatEndOfInterrupt();
nano100_t PlatReadTime();
Driver* PlatCreateDriver(KDevice& device); //Null if the platform has no driver for the device's HID
void PlatDisplayDevice(const KDevice& device);
