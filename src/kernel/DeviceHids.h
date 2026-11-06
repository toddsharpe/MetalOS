#pragma once

namespace DeviceHids
{
	struct Entry
	{
		const char *Name;
		const char *Description;
	};

	const Entry* Lookup(const char *HardwareId);
}
