#pragma once

#include "kernel/Types.h"
#include "Lib/Span.h"

class UThread;
class UProcess;
struct ProcessListEntry;

UProcess* KeCreateProcess(const char* const commandLine);
void KeTerminateProcess(UProcess& process, const uint32_t exitCode);
void KeExitUThread(UThread& thread, const uint32_t exitCode); //The process's last live thread ending ends the process; doesn't return for the calling thread
void KeReleaseUThread(UThread& thread);
void KeReleaseUProcess(UProcess& process);
void KeSignalReaper();
uint32_t KeReaperThread(void* const arg);
bool KeIsProcessAlive(const uint32_t id);
size_t KeGetProcessList(ProcessListEntry* const buffer, const size_t maxCount);

const KModule* KeAddModule(const cstr_t name, void* const image, const size_t imageSize); //Null once the kernel's module table is full
const KModule* KeLoadLibrary(const char* const name);
bool KeGetModule(const uintptr_t address, KModule& module); //Fills module with the kernel module containing address
