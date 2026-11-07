#pragma once

#include "Lib/System.h"

//A name to opaque value (e.g. a grant token) rendezvous, so unrelated processes can find a server
static constexpr size_t MaxEndpointName = 32; //Including the NUL
static constexpr uint32_t KernelEndpointOwner = 0; //Process ids start at 1
bool KeEndpointRegister(const char* const name, const uint64_t id, const uint32_t owner);
bool KeEndpointLookup(const char* const name, uint64_t& id);

//Connection handoff: a client posts an id, the server polls to accept it
bool KeEndpointPost(const char* const name, const uint64_t value);
bool KeEndpointPoll(const char* const name, const uint32_t owner, uint64_t& value);
void KeEndpointRelease(const uint32_t owner);
