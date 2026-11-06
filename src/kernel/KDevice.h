#pragma once

#include "Lib/System.h"
#include "Lib/LinkedList.h"
#include "uACPI/namespace.h"
#include "kernel/HyperV/VmBus.h"

enum class KDeviceClass
{
	Unknown,
	Keyboard,
	Mouse,
	Storage,
	Nic,
	Display
};

enum class KDeviceType
{
	Unknown,
	Acpi,
	Software,
	HyperV
};

class Driver;
class VmBusDriver;
class KDevice
{
public:
	constexpr KDevice() :
		Name(),
		Path(),
		Hid(),
		Description(),
		Type(),
		Class(),
		Children(),
		Driver(),
		AcpiNode()
	{

	}

	void Display() const
	{
		Printf("%s (0x%016x)\n", Path, this);
		if (Name[0])
			Printf("    Name : %s\n", Name);
		if (Hid[0])
			Printf("    HID  : %s\n", Hid);
		if (Description[0])
			Printf("    Desc : %s\n", Description);
		Printf("    Type : %d\n", Type);
		Printf("    Class: %d\n", Class);
		Printf("    Driver: 0x%016X\n", Driver);

		switch (Type)
		{
			case KDeviceType::HyperV:
				Printf("     - RelId: %d\n", child_relid);
				Printf("     - MsgConnId: %d\n", m_msg_conn_id);
				break;
		}

		for (const KDevice* const child : Children)
			child->Display();
	}

	char Name[64];
	char Path[128];
	char Hid[48];
	char Description[64];

	KDeviceType Type;
	KDeviceClass Class;
	LinkedList<KDevice*> Children;
	Driver* Driver;

	union
	{
		//Acpi
		struct
		{
			uacpi_namespace_node *AcpiNode;
		};
		
		//Software
		struct
		{
			void* Context;
		};

		//HyperV
		struct
		{
			//TODO(tsharpe): Determine which properties to keep of channel
			uint32_t child_relid;
			uint32_t m_msg_conn_id;
			HyperV::vmbus_channel_offer_channel m_channel;
			VmBusDriver* VmBus; //The bus that offered the channel, which creates its GPADLs and opens it
		};
	};
};
