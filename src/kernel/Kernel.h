#pragma once

#include "kernel/Arch.h"
#include "kernel/Types.h"

struct LoaderParams;
struct KProcess;
class DeviceTree;

//Kernel globals (Kernel.cpp)
extern LoaderParams BootParams;
extern KProcess m_process;
extern DeviceTree m_deviceTree;

void KeRegisterInterrupt(const Arch::InterruptVector interrupt, const ActionContext& context);
