#pragma once

#include "Kernel/Devices/Device.h"
#include <linux/hyperv.h>
#include <map>

//static const maps don't build yet (vcrt dependencies)
struct VmBusDevice
{
	const char* Hid;
	const char* Name;
};
const VmBusDevice VMBUS_DEVICE_NAMES[] =
{
	{ "{525074DC-8985-46E2-8057-A307DC18A502}", "Hyper-V Dynamic Memory" },
	{ "{F8E65716-3CB3-4A06-9A60-1889C5CCCAB5}", "Hyper-V Remote Desktop Control Channel" },
	{ "{CFA8B69E-5B4A-4CC0-B98B-8BA1A1F3F95A}", "Hyper-V Mouse" },
	{ "{F912AD6D-2B17-48EA-BD65-F927A61C7684}", "Hyper-V Keyboard" },
	{ "{DA0A7802-E377-4AAC-8E77-0558EB1073F8}", "Hyper-V Video" },
	{ "{3375BAF4-9E15-4B30-B765-67ACB10D607B}", "Hyper-V Activation Component" },
	{ "{57164F39-9115-4E78-AB55-382F3BD5422D}", "Hyper-V Heartbeat" },
	{ "{A9A0F4E7-5A45-4D96-B827-8A841E8C03E6}", "Hyper-V Data Exchange" },
	{ "{0E0B6031-5213-4934-818B-38D90CED39DB}", "Hyper-V Guest Shutdown" },
	{ "{9527E630-D0AE-497B-ADCE-E80AB0175CAF}", "Hyper-V Time Sync" },
	{ "{35FA2E29-EA23-4236-96AE-3A6EBACBA440}", "Hyper-V Volume Shadow Copy" },
	{ "{276AACF4-AC15-426C-98DD-7521AD3F01FE}", "Hyper-V Remote Desktop Virtualization" },
	{ "{F8615163-DF3E-46C5-913F-F2D2F965ED0E}", "Hyper-V Network Adapter" },
	{ "{BA6163D9-04A1-4D29-B605-72E2FFB1DC7F}", "Hyper-V SCSI Controller" },
	{ nullptr, nullptr}
};

//HyperV Channel Offer
class HyperVDevice : public Device
{
public:
	HyperVDevice(vmbus_channel_offer_channel& channel, uint32_t conn_id);
	
	void Initialize() override;
	const void* GetResource(uint32_t type) const override;
	void DisplayDetails() const override;

	enum ResourceType
	{
		ChildRelid,
		ConnectionId,
		OfferChannel,
	};

private:
	static const bool GetDeviceName(const std::string& hid, std::string& name);

	vmbus_channel_offer_channel m_channel;
	uint32_t m_msg_conn_id;
};

