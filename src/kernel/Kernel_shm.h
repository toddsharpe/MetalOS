#pragma once

#include "Lib/System.h"

struct KProcess;
class KSharedMemory;

//Regions are reached only through the handles and grants that reference them; there is no global id lookup
KSharedMemory* KeShmCreate(const size_t size);
void* KeShmMap(KProcess& process, KSharedMemory& shm);
void KeShmUnref(KSharedMemory& shm);

//A kernel-held region (e.g. a ring), initialized and then published as a grant under an endpoint for a server to claim
void* KePublishRegion(const char* const endpoint, const size_t size, void (*const initialize)(void* region));
