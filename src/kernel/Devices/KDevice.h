#pragma once

#include "Lib/System.h"
#include "Lib/IntrusiveList.h"
#include "kernel/Plat.h"

struct uacpi_namespace_node;

enum class KDeviceClass
{
	Unknown,
	Keyboard,
	Mouse,
	Storage, //Its driver is a StorageDriver
	Nic,
	Display
};

enum class KDeviceType
{
	Unknown,
	Acpi,
	Platform
};

class Driver;
class KDevice : public ListLink
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
			case KDeviceType::Platform:
				PlatDisplayDevice(*this);
				break;
		}

		for (const KDevice& child : Children)
			child.Display();
	}

	char Name[64];
	char Path[128];
	char Hid[48];
	char Description[64];

	KDeviceType Type;
	KDeviceClass Class;
	IntrusiveList<KDevice> Children;
	Driver* Driver;

	union
	{
		//Acpi
		struct
		{
			uacpi_namespace_node *AcpiNode;
		};
		
		//Software and Platform
		struct
		{
			void* Context;
		};
	};
};
