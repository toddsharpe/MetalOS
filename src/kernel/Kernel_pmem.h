#pragma once

#include "Lib/System.h"

void KePhysicalInitialize();
paddr_t KePhysicalAlloc();
paddr_t KePhysicalAlloc(const size_t count);
bool KePhysicalTryAlloc(paddr_t& address); //False when out of memory, for allocations a process can trigger
bool KePhysicalTryAlloc(paddr_t& address, const size_t count);
void KePhysicalRef(const paddr_t address); //Another owner (an owning mapping) of an allocated page
void KePhysicalFree(const paddr_t address); //Drops one reference; frees the page with the last
void KePhysicalFree(const paddr_t address, const size_t count);
