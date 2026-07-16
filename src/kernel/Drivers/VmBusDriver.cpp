#pragma once

#include "kernel/Drivers/VmBusDriver.h"
#include "kernel/HyperV/VmBus.h"
#include "x64/intrin.h"
#include "Assert.h"

//These aren't used
volatile uint8_t VmBusDriver::MonitorPage1[Arch::PageSize] = { 0 }; //Parent->child notifications
volatile uint8_t VmBusDriver::MonitorPage2[Arch::PageSize] = { 0 }; //Child->parent notifications

VmBusDriver::VmBusDriver(KDevice& device) :
	Driver(device),
	m_connectEvent(false, false),
	m_thread(),
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

HyperV::HV_HYPERCALL_RESULT_VALUE VmBusDriver::PostMessage(const uint32_t size, const void* message, HyperV::VmBusResponse& response)
{
	//TODO: this should probably be protected by a spinlock
	
	HyperV::HV_CONNECTION_ID connectionId = { 0 };
	connectionId.Id = m_msg_conn_id;

	//Create wait event
	KEvent event(false, false);

	HyperV::vmbus_channel_message_header* hdr = (HyperV::vmbus_channel_message_header*)message;
	switch (hdr->msgtype)
	{
		case HyperV::vmbus_channel_message_type::CHANNELMSG_GPADL_HEADER:
		{
			HyperV::vmbus_channel_gpadl_header* gpadlHeader = (HyperV::vmbus_channel_gpadl_header*)message;
			gpadlHeader->gpadl = _InterlockedIncrement64((volatile __int64*)&m_nextGpadlHandle);
			m_requests.Add({ gpadlHeader->gpadl, 0, 0, &event, &response});
		}
		break;

		case HyperV::vmbus_channel_message_type::CHANNELMSG_OPENCHANNEL:
		{
			HyperV::vmbus_channel_open_channel* header = (HyperV::vmbus_channel_open_channel*)message;
			m_requests.Add({ 0, header->child_relid, header->openid, &event, &response });
		}
		break;

		default:
		{
			Assert(false);
		}
	}

	HyperV::HV_HYPERCALL_RESULT_VALUE result = HyperV::Platform::HvPostMessage(connectionId, (HyperV::HV_MESSAGE_TYPE)1, size, message);
	Assert(HV_SUCCESS(result.Status));

	KeWait(event);
	return result;
}

uint32_t VmBusDriver::BeginGpadl(const uint32_t size, void* message, HyperV::VmBusResponse& response, KEvent& event)
{
	HyperV::vmbus_channel_gpadl_header* gpadlHeader = (HyperV::vmbus_channel_gpadl_header*)message;
	gpadlHeader->gpadl = _InterlockedIncrement64((volatile __int64*)&m_nextGpadlHandle);
	m_requests.Add({ gpadlHeader->gpadl, 0, 0, &event, &response });

	HyperV::HV_CONNECTION_ID connectionId = { 0 };
	connectionId.Id = m_msg_conn_id;
	HyperV::HV_HYPERCALL_RESULT_VALUE result = HyperV::Platform::HvPostMessage(connectionId, (HyperV::HV_MESSAGE_TYPE)1, size, message);
	Assert(HV_SUCCESS(result.Status));
	return gpadlHeader->gpadl;
}

void VmBusDriver::PostGpadlBody(const uint32_t size, const void* message)
{
	HyperV::HV_CONNECTION_ID connectionId = { 0 };
	connectionId.Id = m_msg_conn_id;
	HyperV::HV_HYPERCALL_RESULT_VALUE result;
	do {
		result = HyperV::Platform::HvPostMessage(connectionId, (HyperV::HV_MESSAGE_TYPE)1, size, message);
	} while (result.Status == HyperV::HV_STATUS_INSUFFICIENT_BUFFERS);
	Assert(HV_SUCCESS(result.Status));
}

void VmBusDriver::SetCallback(uint32_t id, const ActionContext& context)
{
	//Assert(m_channelCallbacks.find(id) == m_channelCallbacks.end());
	m_channelCallbacks.Add(id, context);
}

VmBusDriver::BusRequest* VmBusDriver::FindRequest(uint32_t gpadl)
{
	for (BusRequest& request : m_requests)
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
				BusRequest* request = FindRequest(created->gpadl);
				//TODO: remove

				//Save response to caller and signal
				request->Response->gpadl_created = *created;
				request->Event->Set();
			}
			break;

			case HyperV::vmbus_channel_message_type::CHANNELMSG_OPENCHANNEL_RESULT:
			{
				HyperV::vmbus_channel_open_result* result = (HyperV::vmbus_channel_open_result*)message.Payload;
				BusRequest* request = FindRequest(result->child_relid, result->openid);
				//TODO: remove

				//Save response to caller and signal
				request->Response->open_result = *result;
				request->Event->Set();
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
				Printf("HyperV::ThreadLoop - Type: %d Header: %d\n", message.Header.MessageType, header->msgtype);
				//NotImplemented();
			}
			break;
		}
	}
	
	HyperV::Interrupts::EOI();
}

/*
Connection order:
Guest: CHANNELMSG_INITIATE_CONTACT. Host: CHANNELMSG_VERSION_RESPONSE
Guest: CHANNELMSG_REQUESTOFFERS. Host: multiple CHANNELMSG_OFFERCHANNEL, ended with CHANNELMSG_ALLOFFERS_DELIVERED
*/
