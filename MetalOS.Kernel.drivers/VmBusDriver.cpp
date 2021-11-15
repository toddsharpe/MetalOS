#include <Kernel.h>
#include <Assert.h>
#include <linux/hyperv.h>

#include "VmBusDriver.h"
#include "HyperVDevice.h"

volatile uint8_t VmBusDriver::MonitorPage1[PAGE_SIZE] = { 0 }; //Parent->child notifications
volatile uint8_t VmBusDriver::MonitorPage2[PAGE_SIZE] = { 0 }; //Child->parent notifications

VmBusDriver::VmBusDriver(Device& device) :
	Driver(device),
	m_threadSignal(),
	m_connectSemaphore(),
	m_queue(),
	m_channelCallbacks(),
	m_msg_conn_id(),
	m_nextGpadlHandle(),
	m_requests()
{

}

Result VmBusDriver::Initialize()
{
	//Initialize
	m_threadSignal = kernel.CreateSemaphore(0, 0, "HyperVThread");
	kernel.Printf("m_threadSignal 0x%016x\n", m_threadSignal);
	m_connectSemaphore = kernel.CreateSemaphore(0, 0, "HyperVConnect");
	kernel.CreateKernelThread(VmBusDriver::ThreadLoop, this);

	HyperV::SetSintVector(VMBUS_MESSAGE_SINT, (uint32_t)InterruptVector::HypervisorVmBus);
	kernel.KeRegisterInterrupt(InterruptVector::HypervisorVmBus, { &VmBusDriver::OnInterrupt, this });
	HyperV::EnableSynic();
	
	return Result::Success;
}

Result VmBusDriver::Read(char* buffer, size_t length, size_t* bytesRead)
{
	return Result::NotImplemented;
}

Result VmBusDriver::Write(const char* buffer, size_t length)
{
	return Result::NotImplemented;
}

Result VmBusDriver::EnumerateChildren()
{
	//Initiate connection
	vmbus_channel_initiate_contact msg;
	memset(&msg, 0, sizeof(vmbus_channel_initiate_contact));
	msg.header.msgtype = vmbus_channel_message_type::CHANNELMSG_INITIATE_CONTACT;
	msg.vmbus_version_requested = VERSION_WIN10_V5_2;
	msg.msg_sint = VMBUS_MESSAGE_SINT;
	msg.monitor_page1 = kernel.VirtualToPhysical((uintptr_t)& MonitorPage1);
	msg.monitor_page2 = kernel.VirtualToPhysical((uintptr_t)& MonitorPage2);
	msg.target_vcpu = 0;
	
	HV_CONNECTION_ID connectionId = { 0 };
	connectionId.Id = VMBUS_MESSAGE_CONNECTION_ID_4;
	HV_HYPERCALL_RESULT_VALUE result = HyperV::HvPostMessage(connectionId, (HV_MESSAGE_TYPE)1, sizeof(vmbus_channel_initiate_contact), &msg);
	kernel.WaitForSemaphore(m_connectSemaphore, INT64_MAX);
	connectionId.Id = m_msg_conn_id;

	//vmbus_request_offers
	vmbus_channel_message_header header;
	memset(&header, 0, sizeof(vmbus_channel_message_header));
	header.msgtype = vmbus_channel_message_type::CHANNELMSG_REQUESTOFFERS;
	result = HyperV::HvPostMessage(connectionId, (HV_MESSAGE_TYPE)1, sizeof(vmbus_channel_message_header), &header);
	kernel.WaitForSemaphore(m_connectSemaphore, INT64_MAX);

	return Result::Success;
}

void VmBusDriver::SetMonitor(uint32_t monitorId)
{
	const uint8_t group = (uint8_t)monitorId / 32;
	const uint8_t bit = (uint8_t)monitorId % 32;

	PVHV_MONITOR_PAGE page = (PVHV_MONITOR_PAGE)MonitorPage1;
	page->TriggerGroup[group].Pending |= (1 << bit);
}

HV_HYPERCALL_RESULT_VALUE VmBusDriver::PostMessage(const uint32_t size, const void* message, VmBusResponse& response)
{
	//TODO: this should probably be protected by a spinlock
	
	HV_CONNECTION_ID connectionId = { 0 };
	connectionId.Id = m_msg_conn_id;

	//Create wait event (currently a semaphore)
	Handle semaphore = kernel.CreateSemaphore(0, 0, "");

	vmbus_channel_message_header* hdr = (vmbus_channel_message_header*)message;
	switch (hdr->msgtype)
	{
		case vmbus_channel_message_type::CHANNELMSG_GPADL_HEADER:
		{
			vmbus_channel_gpadl_header* gpadlHeader = (vmbus_channel_gpadl_header*)message;
			gpadlHeader->gpadl = _InterlockedIncrement((volatile long*)&m_nextGpadlHandle);
			m_requests.push_back({ gpadlHeader->gpadl, 0, 0, semaphore, response});
		}
		break;

		case vmbus_channel_message_type::CHANNELMSG_OPENCHANNEL:
		{
			vmbus_channel_open_channel* header = (vmbus_channel_open_channel*)message;
			m_requests.push_back({ 0, header->child_relid, header->openid, semaphore, response });
		}
		break;

		default:
		{
			Assert(false);
		}
	}

	HV_HYPERCALL_RESULT_VALUE result = HyperV::HvPostMessage(connectionId, (HV_MESSAGE_TYPE)1, size, message);
	Assert(HV_SUCCESS(result.Status));

	kernel.WaitForSemaphore(semaphore, INT64_MAX);
	kernel.CloseSemaphore(semaphore);
	return result;
}

void VmBusDriver::OnInterrupt()
{
	std::list<uint32_t> ids;
	HyperV::ProcessInterrupts(VMBUS_MESSAGE_SINT, ids, m_queue);

	//Call callbacks
	for (uint32_t id : ids)
	{
		const auto& it = m_channelCallbacks.find(id);
		if (it == m_channelCallbacks.end())
			continue;

		(*it->second.Handler)(it->second.Context);
	}

	//Start processing thread
	if (!m_queue.empty())
		kernel.ReleaseSemaphore(this->m_threadSignal, 1);
}

void VmBusDriver::SetCallback(uint32_t id, const CallContext& context)
{
	Assert(m_channelCallbacks.find(id) == m_channelCallbacks.end());
	m_channelCallbacks.insert({ id, context });
}

uint32_t VmBusDriver::ThreadLoop()
{
	while (true)
	{
		kernel.WaitForSemaphore(m_threadSignal, INT64_MAX);
		Assert(!m_queue.empty());

		const HV_MESSAGE message = m_queue.front();
		m_queue.pop_front();

		vmbus_channel_message_header* header = (vmbus_channel_message_header*)message.Payload;
		switch (header->msgtype)
		{
		case vmbus_channel_message_type::CHANNELMSG_VERSION_RESPONSE:
		{
			vmbus_channel_version_response* response = (vmbus_channel_version_response*)message.Payload;
			Assert(response->version_supported);
			m_msg_conn_id = response->msg_conn_id;

			//Let connect proceed
			kernel.ReleaseSemaphore(m_connectSemaphore, 1);
		}
		break;
		case vmbus_channel_message_type::CHANNELMSG_OFFERCHANNEL:
		{
			vmbus_channel_offer_channel* offer = (vmbus_channel_offer_channel*)message.Payload;
			vmbus_channel_offer o = offer->offer;
			Assert(o.sub_channel_index == 0);

			HyperVDevice* child = new HyperVDevice(*offer, m_msg_conn_id);
			child->Initialize();
			
			//Update path - TODO: make Device::AddChild that does this?
			char buffer[64] = { 0 };
			sprintf(buffer, "%s\\%s", m_device.Path.c_str(), child->GetHid().c_str());
			child->Path = buffer;

			m_device.GetChildren().push_back(child);
		}
		break;
		case vmbus_channel_message_type::CHANNELMSG_ALLOFFERS_DELIVERED:
		{
			//Let connect proceed
			kernel.ReleaseSemaphore(m_connectSemaphore, 1);
		}
		break;
		case vmbus_channel_message_type::CHANNELMSG_GPADL_CREATED:
		{
			vmbus_channel_gpadl_created* created = (vmbus_channel_gpadl_created*)message.Payload;
			BusRequest* request = FindRequest(created->gpadl);
			//TODO: remove

			//Save response to caller and signal
			request->Response.gpadl_created = *created;
			kernel.ReleaseSemaphore(request->Semaphore, 1);
		}
		break;
		case vmbus_channel_message_type::CHANNELMSG_OPENCHANNEL_RESULT:
		{
			vmbus_channel_open_result* result = (vmbus_channel_open_result*)message.Payload;
			BusRequest* request = FindRequest(result->child_relid, result->openid);
			//TODO: remove

			//Save response to caller and signal
			request->Response.open_result = *result;
			kernel.ReleaseSemaphore(request->Semaphore, 1);
		}
		break;
		default:
		{
			kernel.Printf("HyperV::ThreadLoop - Type: %d Header: %d\n", message.Header.MessageType, header->msgtype);
			Assert(false);
		}
		break;
		}
	}
}

VmBusDriver::BusRequest* VmBusDriver::FindRequest(uint32_t gpadl)
{
	for (auto& request : m_requests)
	{
		if (request.Gpadl == gpadl)
			return &request;
	}

	Assert(false);
	return nullptr;
}

VmBusDriver::BusRequest* VmBusDriver::FindRequest(uint32_t child_relid, uint32_t openid)
{
	for (auto& request : m_requests)
	{
		if (request.child_relid == child_relid && request.openid == openid)
			return &request;
	}

	Assert(false);
	return nullptr;
}
