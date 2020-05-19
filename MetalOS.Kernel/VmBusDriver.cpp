#include "VmBusDriver.h"
#include "Main.h"
#include "x64_support.h"
#include "HyperVDevice.h"
#include <crt_stdio.h>

volatile uint8_t VmBusDriver::MonitorPage1[PAGE_SIZE] = { 0 };
volatile uint8_t VmBusDriver::MonitorPage2[PAGE_SIZE] = { 0 };

VmBusDriver::VmBusDriver(Device& device) :
	Driver(device),
	m_threadSignal(),
	m_connectSemaphore(),
	m_queue(),
	m_msg_conn_id()
{

}

Result VmBusDriver::Initialize()
{
	//Initialize
	m_threadSignal = kernel.CreateSemaphore(0, 0, "HyperVThread");
	m_connectSemaphore = kernel.CreateSemaphore(0, 0, "HyperVConnect");
	kernel.CreateThread(VmBusDriver::ThreadLoop, this);
	
	HyperV::SetSintVector(VMBUS_MESSAGE_SINT, InterruptVector::HypervisorVmBus);
	kernel.RegisterInterrupt(InterruptVector::HypervisorVmBus, { &VmBusDriver::OnInterrupt, this });
	HyperV::EnableSynic();
	
	return Result::ResultSuccess;
}

Result VmBusDriver::Read(const char* buffer, size_t length)
{
	return Result::ResultNotImplemented;
}

Result VmBusDriver::Write(const char* buffer, size_t length)
{
	return Result::ResultNotImplemented;
}

Result VmBusDriver::EnumerateChildren()
{
	//Initiate connection
	vmbus_channel_initiate_contact msg;
	memset(&msg, 0, sizeof(vmbus_channel_initiate_contact));
	msg.header.msgtype = CHANNELMSG_INITIATE_CONTACT;
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
	header.msgtype = CHANNELMSG_REQUESTOFFERS;
	result = HyperV::HvPostMessage(connectionId, (HV_MESSAGE_TYPE)1, sizeof(vmbus_channel_message_header), &header);
	kernel.WaitForSemaphore(m_connectSemaphore, INT64_MAX);

	return Result::ResultNotImplemented;
}

void VmBusDriver::OnInterrupt()
{
	HyperV::ProcessInterrupts(VMBUS_MESSAGE_SINT, m_queue);

	//Start processing thread
	kernel.ReleaseSemaphore(this->m_threadSignal, 1);
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
		case CHANNELMSG_VERSION_RESPONSE:
		{
			vmbus_channel_version_response* response = (vmbus_channel_version_response*)message.Payload;
			Assert(response->version_supported);
			m_msg_conn_id = response->msg_conn_id;

			//Let connect proceed
			kernel.ReleaseSemaphore(m_connectSemaphore, 1);
		}
		break;
		case CHANNELMSG_OFFERCHANNEL:
		{
			vmbus_channel_offer_channel* offer = (vmbus_channel_offer_channel*)message.Payload;
			vmbus_channel_offer o = offer->offer;
			Assert(o.sub_channel_index == 0);

			HyperVDevice* child = new HyperVDevice(*offer);
			child->Initialize();
			
			//Update path
			char buffer[64] = { 0 };
			crt_sprintf(buffer, "%s\\%s", m_device.Path.c_str(), child->GetHid().c_str());
			child->Path = buffer;

			m_device.GetChildren().push_back(child);
		}
		break;
		case CHANNELMSG_ALLOFFERS_DELIVERED:
		{
			//Let connect proceed
			kernel.ReleaseSemaphore(m_connectSemaphore, 1);
		}
		break;
		default:
		{
			Print("HyperV::ThreadLoop - Type: %d Header: %d\n", message.Header.MessageType, header->msgtype);
		}
		break;
		}
	}
}
