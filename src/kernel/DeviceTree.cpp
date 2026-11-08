#include "kernel/DeviceTree.h"
#include "kernel/Heap.h"
#include "kernel/DeviceHids.h"
#include "kernel/Plat.h"
#include "kernel/Drivers/RamDriveDriver.h"
#include "x64/intrin.h"
#include "uACPI/uacpi.h"
#include "uACPI/namespace.h"
#include "uACPI/utilities.h"
#include "uACPI/resources.h"
#include "core_crt/stdio.h"
#include "Lib/StaticMap.h"
#include "Lib/StaticVector.h"
#include "Lib/String.h"
#include "Assert.h"

namespace
{
	struct AcpiContext
	{
		AcpiContext() : Nodes() {}
		StaticMap<uacpi_namespace_node*, KDevice*, 32> Nodes;
	};

	uacpi_iteration_decision acpi_init_one_device(void *ctx, uacpi_namespace_node *node, uacpi_u32 node_depth)
	{
		UNUSED(node_depth);
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
		StrCopy(device->Name, Str(info->name.text));
		StrCopy(device->Path, CStr(path));
		if (info->flags & UACPI_NS_NODE_INFO_HAS_HID)
		{
			StrCopy(device->Hid, CStr(info->hid.value, info->hid.size - 1));
			const DeviceHids::Entry* lookup = DeviceHids::Lookup(info->hid.value);
			if (lookup)
				StrCopy(device->Description, CStr(lookup->Description));
		}
		device->Type = KDeviceType::Acpi;
		device->AcpiNode = node;

		//Get parent, add to children
		uacpi_namespace_node* const parent = uacpi_namespace_node_parent(node);
		if (parent != nullptr)
		{
			KDevice* parentDevice = context->Nodes.Get(parent);
			Assert(parentDevice);
			parentDevice->Children.InsertTail(*device);
		}

		uacpi_free_namespace_node_info(info);
		uacpi_free_absolute_path(path);
		return UACPI_ITERATION_DECISION_CONTINUE;
	}
}

//Depth-first from the root, visiting each device before its children (so children a driver enumerates are visited too); stops when visit returns true
template <typename Visit>
KDevice* DeviceTree::Walk(Visit visit) const
{
	Assert(m_root);

	StaticVector<KDevice*, 32> stack;
	Assert(stack.Add(m_root));
	while (stack.Count() != 0)
	{
		KDevice* const current = stack.Pop();
		if (visit(*current))
			return current;

		for (KDevice& child : current->Children)
			Assert(stack.Add(&child));
	}
	return nullptr;
}

void DeviceTree::Enumerate()
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

void DeviceTree::AddRootChild(KDevice& device)
{
	//Populate path
	snprintf(device.Path, sizeof(device.Path), "%s%s", m_root->Path, device.Hid);

	m_root->Children.InsertTail(device);
	AttachDriver(device);
}

void DeviceTree::AttachDrivers()
{
	Walk([this](KDevice& device) { AttachDriver(device); return false; });
}

void DeviceTree::Display() const
{
	Assert(this->m_root);
	this->m_root->Display();
}

KDevice* DeviceTree::GetDeviceByClass(const KDeviceClass kClass) const
{
	return Walk([kClass](const KDevice& device) { return device.Class == kClass; });
}

void DeviceTree::AttachDriver(KDevice& device)
{
	if (Str(device.Hid) == Str(RamDriveHid))
	{
		device.Driver = KeAlloc<RamDriveDriver>(AllocType::Kernel, device);
		Assert(device.Driver);
	}
	else
	{
		device.Driver = PlatCreateDriver(device);
	}

	//If driver was attached, attempt to initialize (a bus adds its child devices here, which Walk then visits)
	if (device.Driver && !device.Driver->Initialize())
		Printf("Initialization failed:\n");
}
