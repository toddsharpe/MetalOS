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

	void Enumerate()
	{
		AcpiContext context;
		
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
		AttachDrivers();
	}

	void AddRootChild(KDevice& device)
	{
		//Populate path
		char buffer[64] = {};
		sprintf(buffer, "%s%s", m_root->Path.c_str(), device.Hid.c_str());
		device.Path = KeCopy(buffer, AllocType::Kernel);

		m_root->Children.Add(&device);
		AttachDriver(device);
	}

	void AttachDrivers()
	{
		StaticStack<KDevice*, 32> stack;
		Assert(stack.Push(m_root));

		while (!stack.IsEmpty())
		{
			KDevice* current = stack.Pop();
			AttachDriver(*current);

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
		AcpiContext() : Nodes() {}
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
		KDevice* device = KeAlloc<KDevice>(AllocType::Kernel);
		Assert(device);
		context->Nodes.Add(node, device);

		//Populate fields
		device->Name = KeCopy(info->name.text, AllocType::Kernel);
		device->Path = KeCopy(CString(path), AllocType::Kernel);
		if (info->flags & UACPI_NS_NODE_INFO_HAS_HID)
		{
			device->Hid = KeCopy({info->hid.value, info->hid.size - 1}, AllocType::Kernel);
			const DeviceHids::Entry* lookup = DeviceHids::Lookup(info->hid.value);
			if (lookup)
				device->Description = KeCopy(CString(lookup->Description), AllocType::Kernel);
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

	static void AttachDriver(KDevice& device)
	{
		if (device.Hid == "VMBUS" || device.Hid == "MSFT1000")
		{
			device.Driver = KeAlloc<VmBusDriver>(AllocType::Kernel, device);
			Assert(device.Driver);
		}
		else if (device.Hid == "PNP0501")
		{
			device.Driver = KeAlloc<UartDriver>(AllocType::Kernel, device);
			Assert(device.Driver);
		}
		else if (device.Hid == RamDriveHid)
		{
			device.Driver = KeAlloc<RamDriveDriver>(AllocType::Kernel, device);
			Assert(device.Driver);
		}
		else if (device.Hid == "{F912AD6D-2B17-48EA-BD65-F927A61C7684}")
		{
			device.Driver = KeAlloc<HyperVKeyboardDriver>(AllocType::Kernel, device);
			Assert(device.Driver);
		}
		else if (device.Hid == "{CFA8B69E-5B4A-4CC0-B98B-8BA1A1F3F95A}")
		{
			device.Driver = KeAlloc<HyperVMouseDriver>(AllocType::Kernel, device);
			Assert(device.Driver);
		}
		else if (device.Hid == "{F8615163-DF3E-46C5-913F-F2D2F965ED0E}")
		{
			device.Driver = KeAlloc<HyperVNic>(AllocType::Kernel, device);
			Assert(device.Driver);
		}

		//If driver was attached, attempt to initialize/enumerate
		if (device.Driver)
		{
			if (device.Driver->Initialize() != Result::Failed)
			{
				const Result enumResult = device.Driver->Enumerate();
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
