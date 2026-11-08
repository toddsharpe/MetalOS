#pragma once

#include "kernel/Types.h"
#include "Lib/Time.h"

class KThread;
struct KSignal;
class KEvent;
class UThread;
class UProcess;

KThread* KeCreateThread(const KThreadStart start, void* const arg, const char* const name);
void KeExitThread();
void KeExitThread(KThread& thread);
KWaitResult KeWait(KSignal& obj, const milli_t timeout = TimeoutMax);
void KeEventUnref(KEvent& event); //Drops a user event's handle or waiter reference; the event is freed at the last
void KeSleepThread(const nano_t time);
void KeYield();

UThread* KeCreateUThread(UProcess& process, const size_t stackSize, const UThreadStart entry = nullptr, void* const arg = nullptr);
