#include "kernel/Drivers/VmBusDriver.h"
#include "kernel/HyperV/VmBus.h"
#include "Lib/Buffer.h"
#include "x64/intrin.h"
#include "Assert.h"

//Monitor pages handed to the host in CHANNELMSG_INITIATE_CONTACT
volatile uint8_t VmBusDriver::MonitorPage1[Arch::PageSize] = { 0 }; //Parent->child notifications
volatile uint8_t VmBusDriver::MonitorPage2[Arch::PageSize] = { 0 }; //Child->parent notifications

VmBusDriver::VmBusDriver(KDevice& device) :
	Driver(device),
	m_connectEvent(false, false),
	m_channelCallbacks(),
	m_msg_conn_id(),
	m_nextGpadlHandle(),
	m_requests()
{

}

Result VmBusDriver::Initialize()
{
	return Result::Success;
}

Result VmBusDriver::Enumerate()
{

	//Enable VMBus interrupt
	HyperV::Interrupts::EnableSintVector(HyperV::VMBUS_MESSAGE_SINT, (uint32_t)Arch::InterruptVector::HypervisorVmBus);
	KeRegisterInterrupt(Arch::InterruptVector::HypervisorVmBus, { &VmBusDriver::OnInterrupt, this });

	//Initiate connection
	HyperV::vmbus_channel_initiate_contact msg;
	memset(&msg, 0, sizeof(HyperV::vmbus_channel_initiate_contact));
	msg.header.msgtype = HyperV::vmbus_channel_message_type::CHANNELMSG_INITIATE_CONTACT;
	msg.vmbus_version_requested = HyperV::VERSION_WIN10_V5_2;
	msg.msg_sint = HyperV::VMBUS_MESSAGE_SINT;
	msg.monitor_page1 = ResolveImageVA((void*)&MonitorPage1);
	msg.monitor_page2 = ResolveImageVA((void*)&MonitorPage2);
	msg.target_vcpu = 0;

	//Wait for connection result
	HyperV::HV_CONNECTION_ID connectionId = { 0 };
	connectionId.Id = HyperV::VMBUS_MESSAGE_CONNECTION_ID_4;
	HyperV::HV_HYPERCALL_RESULT_VALUE result = HyperV::Platform::HvPostMessage(connectionId, (HyperV::HV_MESSAGE_TYPE)1, sizeof(HyperV::vmbus_channel_initiate_contact), &msg);
	Assert(KeWait(m_connectEvent) == KWaitResult::Signaled);
	connectionId.Id = m_msg_conn_id;

	//Wait for all offers to be received
	HyperV::vmbus_channel_message_header header;
	memset(&header, 0, sizeof(HyperV::vmbus_channel_message_header));
	header.msgtype = HyperV::vmbus_channel_message_type::CHANNELMSG_REQUESTOFFERS;
	result = HyperV::Platform::HvPostMessage(connectionId, VmbusMessageType, sizeof(HyperV::vmbus_channel_message_header), &header);
	Assert(KeWait(m_connectEvent) == KWaitResult::Signaled);

	return Result::Success;
}

//Sends a GPADL_HEADER with the first PFNs, then GPADL_BODY messages for the rest, and waits for GPADL_CREATED
uint32_t VmBusDriver::CreateGpadl(const uint32_t childRelid, const paddr_t address, const size_t pageCount)
{
	static constexpr size_t MaxHeaderPfns = 26;
	static constexpr size_t MaxBodyPfns = 28;

	HyperV::HV_CONNECTION_ID connectionId = { 0 };
	connectionId.Id = m_msg_conn_id;

	const size_t headerPfns = pageCount < MaxHeaderPfns ? pageCount : MaxHeaderPfns;
	const size_t headerMsgSize = sizeof(HyperV::vmbus_channel_gpadl_header)
		+ sizeof(HyperV::gpa_range)
		+ (headerPfns - ANYSIZE_ARRAY) * sizeof(uint64_t);

	StaticBuffer<256> headerBuf;
	HyperV::vmbus_channel_gpadl_header* header =
		(HyperV::vmbus_channel_gpadl_header*)headerBuf.Data;
	memset(header, 0, headerMsgSize);
	header->header.msgtype = HyperV::vmbus_channel_message_type::CHANNELMSG_GPADL_HEADER;
	header->child_relid = childRelid;
	header->rangecount = 1;
	header->range_buflen = (uint16_t)(sizeof(HyperV::gpa_range) + (pageCount - ANYSIZE_ARRAY) * sizeof(uint64_t));
	header->range[0].byte_count = (uint32_t)(pageCount << Arch::PageShift);
	header->range[0].byte_offset = 0;
	for (size_t i = 0; i < headerPfns; i++)
		header->range[0].pfn_array[i] = (address >> Arch::PageShift) + i;

	KEvent event(false, false);
	HyperV::VmBusResponse response;
	header->gpadl = _InterlockedIncrement64((volatile __int64*)&m_nextGpadlHandle);
	AddRequest({ header->gpadl, 0, 0, &event, &response });

	HyperV::HV_HYPERCALL_RESULT_VALUE result = HyperV::Platform::HvPostMessage(connectionId, (HyperV::HV_MESSAGE_TYPE)1, (uint32_t)headerMsgSize, header);
	Assert(HV_SUCCESS(result.Status));
	const uint32_t gpadl = header->gpadl;

	size_t pfnsSent = headerPfns;
	uint32_t bodyNum = 0;
	while (pfnsSent < pageCount)
	{
		const size_t batchSize = (pageCount - pfnsSent) < MaxBodyPfns ? (pageCount - pfnsSent) : MaxBodyPfns;
		const size_t bodyMsgSize = sizeof(HyperV::vmbus_channel_gpadl_body)
			+ (batchSize - ANYSIZE_ARRAY) * sizeof(uint64_t);

		StaticBuffer<256> bodyBuf;
		HyperV::vmbus_channel_gpadl_body* body =
			(HyperV::vmbus_channel_gpadl_body*)bodyBuf.Data;
		memset(body, 0, bodyMsgSize);
		body->header.msgtype = HyperV::vmbus_channel_message_type::CHANNELMSG_GPADL_BODY;
		body->msgnumber = bodyNum++;
		body->gpadl = gpadl;
		for (size_t i = 0; i < batchSize; i++)
			body->pfn[i] = (address >> Arch::PageShift) + pfnsSent + i;

		do {
			result = HyperV::Platform::HvPostMessage(connectionId, (HyperV::HV_MESSAGE_TYPE)1, (uint32_t)bodyMsgSize, body);
		} while (result.Status == HyperV::HV_STATUS_INSUFFICIENT_BUFFERS);
		Assert(HV_SUCCESS(result.Status));
		pfnsSent += batchSize;
	}

	KeWait(event);
	Assert(response.gpadl_created.creation_status == 0);
	return response.gpadl_created.gpadl;
}

HyperV::HV_HYPERCALL_RESULT_VALUE VmBusDriver::OpenChannel(const HyperV::vmbus_channel_open_channel& message, HyperV::VmBusResponse& response)
{
	HyperV::HV_CONNECTION_ID connectionId = { 0 };
	connectionId.Id = m_msg_conn_id;

	//Create wait event
	KEvent event(false, false);
	AddRequest({ 0, message.child_relid, message.openid, &event, &response });

	HyperV::HV_HYPERCALL_RESULT_VALUE result = HyperV::Platform::HvPostMessage(connectionId, (HyperV::HV_MESSAGE_TYPE)1, sizeof(HyperV::vmbus_channel_open_channel), &message);
	Assert(HV_SUCCESS(result.Status));

	KeWait(event);
	return result;
}

void VmBusDriver::SetCallback(uint32_t id, const ActionContext& context)
{
	//Assert(m_channelCallbacks.find(id) == m_channelCallbacks.end());
	m_channelCallbacks.Add(id, context);
}

//The ISR removes answered requests, so add with interrupts off
void VmBusDriver::AddRequest(const BusRequest& request)
{
	const cpu_flags_t flags = ArchDisableInterrupts();
	const bool added = m_requests.Add(request);
	ArchRestoreFlags(flags);
	Assert(added);
}

//Called from the ISR: removes the answered request so its stack-owned event and response aren't kept
VmBusDriver::BusRequest VmBusDriver::TakeRequest(const uint32_t gpadl, const uint32_t child_relid, const uint32_t openid)
{
	for (size_t i = 0; i < m_requests.Count(); i++)
	{
		const BusRequest request = m_requests[i];
		if (request.Gpadl == gpadl && request.child_relid == child_relid && request.openid == openid)
		{
			m_requests.RemoveAt(i);
			return request;
		}
	}

	Assert(false);
	return {};
}

void VmBusDriver::OnInterrupt()
{
	//Get SINT events and message if available
	StaticVector<uint32_t, HyperV::Interrupts::MaxChannelIds> ids;
	HyperV::HV_MESSAGE message = {};
	HyperV::Interrupts::ProcessInterrupts(HyperV::VMBUS_MESSAGE_SINT, ids, message);

	for (uint32_t id : ids)
	{
		ActionContext ctx = {};
		if (m_channelCallbacks.Get(id, ctx))
		{
			ctx.Invoke();
		}
		else
		{
			Printf("VmBus: unhandled channel event id=%u\n", id);
		}
	}

	//Process message
	if (message.Header.MessageType == VmbusMessageType)
	{
		HyperV::vmbus_channel_message_header* header = (HyperV::vmbus_channel_message_header*)message.Payload;
		switch (header->msgtype)
		{
			case HyperV::vmbus_channel_message_type::CHANNELMSG_VERSION_RESPONSE:
			{
				HyperV::vmbus_channel_version_response* response = (HyperV::vmbus_channel_version_response*)message.Payload;
				Assert(response->version_supported);
				m_msg_conn_id = response->msg_conn_id;

				//Let enumeration proceed
				m_connectEvent.Set();
			}
			break;

			case HyperV::vmbus_channel_message_type::CHANNELMSG_OFFERCHANNEL:
			{
				HyperV::vmbus_channel_offer_channel* offer = (HyperV::vmbus_channel_offer_channel*)message.Payload;
				Assert(offer->offer.sub_channel_index == 0);

				//Create child device
				KDevice* device = KeAlloc<KDevice>(AllocType::Kernel);
				m_device.Children.Add(device);
				device->Type = KDeviceType::HyperV;

				//Populate HID
				{
					const HyperV::guid_t& if_type = offer->offer.if_type;
					char buffer[64] = {};
					const size_t length = sprintf(buffer, "{%08lX-%04hX-%04hX-%02hhX%02hhX-%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX}",
						if_type.Data1,
						if_type.Data2,
						if_type.Data3,
						if_type.Data4[0], if_type.Data4[1], if_type.Data4[2], if_type.Data4[3],
						if_type.Data4[4], if_type.Data4[5], if_type.Data4[6], if_type.Data4[7]);
					device->Hid = KeCopy({buffer, length}, AllocType::Kernel);
				}

				//Populate name and description
				const DeviceHids::Entry* lookup = DeviceHids::Lookup(device->Hid.c_str());
				if (lookup)
				{
					device->Description = KeCopy(CString(lookup->Description), AllocType::Kernel);
					device->Name = device->Description;
				}

				//Populate path
				{
					char buffer[128] = {};
					sprintf(buffer, "%s\\%s", m_device.Path.c_str(), device->Hid.c_str());
					device->Path = KeCopy(buffer, AllocType::Kernel);
				}

				//Save properties
				device->child_relid = offer->child_relid;
				device->m_msg_conn_id = m_msg_conn_id;
				device->m_channel = *offer;
				device->VmBus = this;
			}
			break;

			case HyperV::vmbus_channel_message_type::CHANNELMSG_ALLOFFERS_DELIVERED:
			{
				//Let enumeration proceed
				m_connectEvent.Set();
			}
			break;

			case HyperV::vmbus_channel_message_type::CHANNELMSG_GPADL_CREATED:
			{
				HyperV::vmbus_channel_gpadl_created* created = (HyperV::vmbus_channel_gpadl_created*)message.Payload;
				const BusRequest request = TakeRequest(created->gpadl, 0, 0);

				//Save response to caller and signal
				request.Response->gpadl_created = *created;
				request.Event->Set();
			}
			break;

			case HyperV::vmbus_channel_message_type::CHANNELMSG_OPENCHANNEL_RESULT:
			{
				HyperV::vmbus_channel_open_result* result = (HyperV::vmbus_channel_open_result*)message.Payload;
				const BusRequest request = TakeRequest(0, result->child_relid, result->openid);

				//Save response to caller and signal
				request.Response->open_result = *result;
				request.Event->Set();
			}
			break;

			case HyperV::vmbus_channel_message_type::CHANNELMSG_RESCIND_CHANNELOFFER:
			{
				const uint32_t relid = *(uint32_t*)((uintptr_t)message.Payload + sizeof(HyperV::vmbus_channel_message_header));
				Printf("VmBus: RESCIND child_relid=%u\n", relid);
			}
			break;

			default:
			{
				Printf("VmBus: unhandled message Type: %d Header: %d\n", message.Header.MessageType, header->msgtype);
			}
			break;
		}
	}
	
	HyperV::Interrupts::EOI();
}
