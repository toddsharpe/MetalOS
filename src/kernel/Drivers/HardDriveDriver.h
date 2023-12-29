#pragma once

#include "Kernel/Objects/KFile.h"
#include <string>
#include <memory>

class HardDriveDriver
{
public:
	virtual Result OpenFile(KFile& file, const std::string& path, const GenericAccess access) const = 0;
	virtual size_t ReadFile(const KFile& handle, void* const buffer, const size_t bytesToRead) const = 0;
};
