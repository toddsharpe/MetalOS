#pragma once

#include "kernel/Core/AddressSpace.h"
#include "kernel/Types.h"
#include "user/MetalOS.Types.h"

//Fills module with the PEB's module containing address. The PEB is process memory, so each read is checked against space, an entry counts only if its image starts a reservation, and its size comes from its own headers within that reservation.
bool GetPebModule(const ProcessEnvironmentBlock* const peb, const AddressSpace& space, const uintptr_t address, KModule& module);
