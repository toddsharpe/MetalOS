#pragma once

#include "Lib/System.h"

class NicDriver;

//The kernel owns the NICs and raw ethernet frames; the TCP/IP stack is the usermode netstack
struct KMacAddress { uint8_t bytes[6]; };

void KeInitNetDevice();
void KeNetRxFrame(NicDriver* const nic, const void* const frame, const size_t length);
bool KeNetSend(const uint32_t ifIdx, const void* const frame, const size_t length);
size_t KeNetInterfaceCount();
NicDriver* KeNetGetInterface(const size_t index);
