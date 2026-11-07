#pragma once

struct LoaderParams;
struct KProcess;
class DeviceTree;

//Kernel globals (Kernel.cpp)
extern LoaderParams BootParams;
extern KProcess m_process;
extern DeviceTree m_deviceTree;
