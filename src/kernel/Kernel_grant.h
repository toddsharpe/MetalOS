#pragma once

#include "core_crt/stdint.h"

enum class UObjectType;

//Capability passing: a single-use token carries a handle's type and kernel object to the process that claims it
static constexpr uint32_t KernelGrantOwner = 0; //Process ids start at 1
uint64_t KeGrantShare(const UObjectType type, void* const object, const uint32_t granter);
bool KeGrantClaim(const uint64_t token, UObjectType& type, void*& object);
void KeGrantRevoke(const uint32_t granter);
