#pragma once

#include <cstddef>
#include <cstdint>

//Sectors in and out: a disk, or a partition's window onto one
class BlockDevice
{
public:
	virtual size_t SectorSize() const = 0;
	virtual uint64_t SectorCount() const = 0;

	//False on a device error or a range past the end
	virtual bool Read(const uint64_t sector, const size_t count, void* const buffer) = 0;
	virtual bool Write(const uint64_t sector, const size_t count, const void* const buffer) = 0;

	//What was written reaches the medium
	virtual bool Flush() = 0;
};
