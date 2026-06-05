#pragma once

#include "Kernel/KDevice.h"
#include "uACPI/uacpi.h"
#include "uACPI/namespace.h"
#include "uACPI/utilities.h"
#include "uACPI/resources.h"
#include "Lib/System.h"
#include "Lib/StaticMap.h"
#include "kernel/DeviceHids.h"
#include "kernel/Drivers/VmBusDriver.h"
#include "kernel/Drivers/UartDriver.h"
#include "kernel/Drivers/RamDriveDriver.h"
#include "kernel/Drivers/HyperVKeyboardDriver.h"
#include "kernel/Drivers/HyperVMouseDriver.h"
#include "kernel/Drivers/HyperVNic.h"
#include "Lib/StaticStack.h"

class DeviceTree
{
public:
	DeviceTree() : m_root()
	{

	}

	void Enumerate(Arena& arena)
	{
		AcpiContext context(arena);
		
		//Add root node
		uacpi_namespace_node* root = uacpi_namespace_root();
		acpi_init_one_device(&context, root, 0);

		//Add system bus
		uacpi_namespace_node* sb = uacpi_namespace_get_predefined(uacpi_predefined_namespace::UACPI_PREDEFINED_NAMESPACE_SB);
		acpi_init_one_device(&context, sb, 0);
		
		uacpi_namespace_for_each_child(
			uacpi_namespace_root(),
			acpi_init_one_device,
			UACPI_NULL,
			UACPI_OBJECT_DEVICE_BIT,
			UACPI_MAX_DEPTH_ANY,
			&context);

		//Set root
		m_root = context.Nodes.Get(root);
		Assert(m_root);

		//Attach drivers
		AttachDrivers(arena);
	}

	void AddRootChild(KDevice& device, Arena& arena)
	{
		//Populate path
		char buffer[64] = {};
		sprintf(buffer, "%s%s", m_root->Path.c_str(), device.Hid.c_str());
		device.Path = arena.Copy(buffer);

		m_root->Children.Add(&device);
		AttachDriver(device, arena);
	}

	void AttachDrivers(Arena& arena)
	{
		StaticStack<KDevice*, 32> stack;
		Assert(stack.Push(m_root));

		while (!stack.IsEmpty())
		{
			KDevice* current = stack.Pop();
			AttachDriver(*current, arena);

			for (auto& child : current->Children)
				Assert(stack.Push(child));
		}
	}
	
	void Display() const
	{
		Assert(this->m_root);
		this->m_root->Display();
	}

	KDevice* GetDeviceByName(const CString& name) const
	{
		StaticStack<KDevice*, 32> stack;
		stack.Push(m_root);

		while (!stack.IsEmpty())
		{
			KDevice* current = stack.Pop();
			if (current->Name == name)
				return current;

			for (auto& child : current->Children)
				stack.Push(child);
		}

		return nullptr;
	}

	KDevice* GetDeviceByClass(const KDeviceClass kClass) const
	{
		Assert(m_root);
		
		StaticStack<KDevice*, 32> stack;
		Assert(stack.Push(m_root));

		while (!stack.IsEmpty())
		{
			KDevice* current = stack.Pop();
			if (current->Class == kClass)
				return current;

			for (auto& child : current->Children)
				Assert(stack.Push(child));
		}

		return nullptr;
	}

	//Device* GetDeviceByType(const DeviceType type) const;
	KDevice* GetDevice(const CString& path) const
	{
		Assert(m_root);
		
		StaticStack<KDevice*, 32> stack;
		Assert(stack.Push(m_root));

		while (!stack.IsEmpty())
		{
			KDevice* current = stack.Pop();
			if (current->Path == path)
				return current;

			for (auto& child : current->Children)
				Assert(stack.Push(child));
		}

		return nullptr;
	}

private:
	struct AcpiContext
	{
		AcpiContext(Arena& arena) : Arena(arena), Nodes()
		{

		}
		
		Arena& Arena;
		StaticMap<uacpi_namespace_node*, KDevice*, 32> Nodes;
	};

	//static ACPI_STATUS AddAcpiDevice(ACPI_HANDLE Object, UINT32 NestingLevel, void* Context, void** ReturnValue);
	//static void AttachDriver(Device& device);

	static uacpi_iteration_decision acpi_init_one_device(void *ctx, uacpi_namespace_node *node, uacpi_u32 node_depth)
	{
		uacpi_namespace_node_info *info;

		const char *path = uacpi_namespace_node_generate_absolute_path(node);
		uacpi_status ret = uacpi_get_namespace_node_info(node, &info);
		if (uacpi_unlikely_error(ret)) {
			Printf("unable to retrieve node %s information: %s", path, uacpi_status_to_string(ret));
			uacpi_free_absolute_path(path);
			return UACPI_ITERATION_DECISION_CONTINUE;
		}

		//Get context and create KDevice
		AcpiContext* const context = reinterpret_cast<AcpiContext*>(ctx);
		Assert(context);
		KDevice* device = context->Arena.Allocate<KDevice>();
		Assert(device);
		context->Nodes.Add(node, device);

		//Populate fields
		device->Name = context->Arena.Copy(info->name.text);
		device->Path = context->Arena.Copy(CString(path));
		if (info->flags & UACPI_NS_NODE_INFO_HAS_HID)
		{
			device->Hid = context->Arena.Copy({info->hid.value, info->hid.size - 1});
			const DeviceHids::Entry* lookup = DeviceHids::Lookup(info->hid.value);
			if (lookup)
				device->Description = context->Arena.Copy(CString(lookup->Description));
		}
		device->Type = KDeviceType::Acpi;
		device->AcpiNode = node;

		//Get parent, add to children
		uacpi_namespace_node* const parent = uacpi_namespace_node_parent(node);
		if (parent != nullptr)
		{
			const char *pp = uacpi_namespace_node_generate_absolute_path(parent);
			KDevice* parentDevice = context->Nodes.Get(parent);
			Assert(parentDevice);
			parentDevice->Children.Add(device);
		}

		uacpi_free_namespace_node_info(info);
		uacpi_free_absolute_path(path);
		return UACPI_ITERATION_DECISION_CONTINUE;
	}

	static void AttachDriver(KDevice& device, Arena& arena)
	{
		if (device.Hid == "VMBUS" || device.Hid == "MSFT1000")
		{
			device.Driver = arena.Allocate<VmBusDriver>(device);
			Assert(device.Driver);
		}
		else if (device.Hid == "PNP0501")
		{
			device.Driver = arena.Allocate<UartDriver>(device);
			Assert(device.Driver);
		}
		else if (device.Hid == RamDriveHid)
		{
			device.Driver = arena.Allocate<RamDriveDriver>(device);
			Assert(device.Driver);
		}
		else if (device.Hid == "{F912AD6D-2B17-48EA-BD65-F927A61C7684}")
		{
			device.Driver = arena.Allocate<HyperVKeyboardDriver>(device);
			Assert(device.Driver);
		}
		else if (device.Hid == "{CFA8B69E-5B4A-4CC0-B98B-8BA1A1F3F95A}")
		{
			device.Driver = arena.Allocate<HyperVMouseDriver>(device);
			Assert(device.Driver);
		}
		else if (device.Hid == "{F8615163-DF3E-46C5-913F-F2D2F965ED0E}")
		{
			device.Driver = arena.Allocate<HyperVNic>(device);
			Assert(device.Driver);
		}

		//If driver was attached, attempt to initialize/enumerate
		if (device.Driver)
		{
			if (device.Driver->Initialize(arena) != Result::Failed)
			{
				const Result enumResult = device.Driver->Enumerate(arena);
				if (enumResult == Result::Failed)
				{
					Printf("Driver failed to enumerate: %d\n", (uint32_t)enumResult);
				}
			}
			else
			{
				Printf("Initialization failed:\n");
			}
		}
	}

	KDevice* m_root;
};
