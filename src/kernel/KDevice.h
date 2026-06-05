#pragma once

#include "Lib/System.h"
#include "Lib/StaticVector.h"
#include "uACPI/namespace.h"
#include "kernel/HyperV/VmBus.h"

enum class KDeviceClass
{
	Unknown,
	Keyboard,
	Mouse,
	Storage,
	Serial,
	Nic
};

enum class KDeviceType
{
	Unknown,
	Acpi,
	Software,
	HyperV
};

class Driver;
class KDevice
{
public:
	KDevice() : Name(), Path(), Hid(), Description(), Type(), Class(), Children(), Driver(), AcpiNode()
	{

	}

	void Initialize(Arena& arena)
	{

	}

	void Display() const
	{
		Printf("%s (0x%016x)\n", Path.c_str(), this);
		if (Name.Length)
			Printf("    Name : %s\n", Name.c_str());
		if (Hid.Length)
			Printf("    HID  : %s\n", Hid.c_str());
		if (Description.Length)
			Printf("    Desc : %s\n", Description.c_str());
		Printf("    Type : %d\n", Type);
		Printf("    Class: %d\n", Class);
		Printf("    Driver: 0x%016X\n", Driver);

		for (const KDevice* const child : Children)
			child->Display();
	}

	CString Name;
	CString Path;
	CString Hid;
	CString Description;

	KDeviceType Type;
	KDeviceClass Class;
	StaticVector<KDevice*, 16> Children;
	Driver* Driver;

	//Acpi
	uacpi_namespace_node *AcpiNode;

	//Software
	void* Context;

	//HyperV
	//TODO(tsharpe): Determine which properties to keep of channel
	uint32_t child_relid;
	uint32_t m_msg_conn_id;
	HyperV::vmbus_channel_offer_channel m_channel;
};
