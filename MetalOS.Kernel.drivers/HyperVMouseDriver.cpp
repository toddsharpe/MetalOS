#include <Kernel.h>
#include <Assert.h>

#include "HyperVMouseDriver.h"
#include "HyperVDevice.h"

HyperVMouseDriver::HyperVMouseDriver(Device& device) :
	Driver(device),
	m_semaphore(),
	m_events(),
	m_channel(INPUTVSC_SEND_RING_BUFFER_SIZE, INPUTVSC_RECV_RING_BUFFER_SIZE, { &HyperVMouseDriver::Callback, this })
{
	m_semaphore = kernel.CreateSemaphore(0, 0, "HyperVMouseDriver");
}

Result HyperVMouseDriver::Initialize()
{
	kernel.Printf("HyperVKeyboardDriver::Initialize\n");

	m_device.Type = DeviceType::Mouse;

	HyperVDevice* device = (HyperVDevice*)&m_device;
	uint32_t childRelid = *(uint32_t*)device->GetResource(HyperVDevice::ResourceType::ChildRelid);
	uint32_t connectionId = *(uint32_t*)device->GetResource(HyperVDevice::ResourceType::ConnectionId);
	vmbus_channel_offer_channel* offerChannel = (vmbus_channel_offer_channel*)device->GetResource(HyperVDevice::ResourceType::OfferChannel);

	//Initialize channel
	m_channel.Initialize(offerChannel);

	//Initialize keyboard
	mousevsc_prt_msg request;
	memset(&request, 0, sizeof(mousevsc_prt_msg));
	request.type = PIPE_MESSAGE_DATA;
	request.size = sizeof(struct synthhid_protocol_request);
	request.request.header.type = SYNTH_HID_PROTOCOL_REQUEST;
	request.request.header.size = sizeof(unsigned int);
	request.request.version_requested.version = SYNTHHID_INPUT_VERSION;

	this->m_channel.SendPacket(&request, sizeof(struct pipe_prt_msg) - sizeof(unsigned char) + sizeof(struct synthhid_protocol_request),
		(unsigned long)&request, VM_PKT_DATA_INBAND, VMBUS_DATA_PACKET_FLAG_COMPLETION_REQUESTED);

	//Wait for SYNTH_HID_PROTOCOL_RESPONSE and SYNTH_HID_INITIAL_DEVICE_INFO
	kernel.WaitForSemaphore(m_semaphore, INT64_MAX);

	//Send info ack
	mousevsc_prt_msg ack;
	memset(&ack, 0, sizeof(struct mousevsc_prt_msg));
	ack.type = PIPE_MESSAGE_DATA;
	ack.size = sizeof(struct synthhid_device_info_ack);
	ack.ack.header.type = SYNTH_HID_INITIAL_DEVICE_INFO_ACK;
	ack.ack.header.size = 1;
	ack.ack.reserved = 0;

	this->m_channel.SendPacket(&ack, sizeof(struct pipe_prt_msg) - sizeof(unsigned char) + sizeof(struct synthhid_device_info_ack),
		(unsigned long)&ack, VM_PKT_DATA_INBAND, VMBUS_DATA_PACKET_FLAG_COMPLETION_REQUESTED);

	return Result::Success;
}

Result HyperVMouseDriver::Read(char* buffer, size_t length, size_t* bytesRead)
{
	return Result::NotImplemented;
}

Result HyperVMouseDriver::Write(const char* buffer, size_t length)
{
	return Result::NotImplemented;
}

Result HyperVMouseDriver::EnumerateChildren()
{
	return Result::NotImplemented;
}

void HyperVMouseDriver::OnCallback()
{
	vmpacket_descriptor* packet;
	uint32_t length = sizeof(vmpacket_descriptor);
	//The nuance here is that this structure vmpacket_descriptor is what is sent, which has an extra field (transactionid)
	//compared to what is received. the code uses the offset field from desc pointer to compensate for this, but it should really be refactored
	while ((packet = (vmpacket_descriptor*)m_channel.ReadPacket(length)) != nullptr)
	{
		switch (packet->type)
		{
		case VM_PKT_COMP:
			break;

		case VM_PKT_DATA_INBAND:
		{
			const uint32_t size = (packet->len8 << 3) - (packet->offset8 << 3);
			Assert(size >= sizeof(struct pipe_prt_msg));

			pipe_prt_msg* mouseMessage = (pipe_prt_msg*)((uintptr_t)packet + (packet->offset8 << 3));
			ProcessMessage(mouseMessage, size);
		}
		break;

		default:
			Assert(false);
			break;
		}

		m_channel.NextPacket(packet->len8 << 3);
	}
	m_channel.StopRead();
}

//Messages consist of |pipe_prt_msg|synthhid_msg|
//mousevsc_prt_msg contains pipe_prt_msg so the math is pretty fucked up. rewrite
void HyperVMouseDriver::ProcessMessage(pipe_prt_msg* msg, const uint32_t size)
{
	if (msg->type != PIPE_MESSAGE_DATA)
		return;

	synthhid_msg* message = (synthhid_msg*)msg->data;
	kernel.Printf("Size: %d\n", msg->size);
	kernel.Printf("ProcessMessage: %d Size: %d\n", message->header.type, message->header.size);
	switch (message->header.type)
	{
	case SYNTH_HID_PROTOCOL_RESPONSE:
	{
		const uint32_t innerSize = msg->size + sizeof(struct pipe_prt_msg) - sizeof(unsigned char);
		kernel.Printf("innerSize %d struct %d\n", innerSize, sizeof(mousevsc_prt_msg));
		Assert(innerSize == sizeof(mousevsc_prt_msg));
		memcpy(&m_response, msg, innerSize);
		Assert(m_response.response.approved);
	}
	break;

	case SYNTH_HID_INITIAL_DEVICE_INFO:
	{
		Assert(msg->size >= sizeof(struct hv_input_dev_info));

		kernel.ReleaseSemaphore(m_semaphore, 1);
	}
	break;

	case SYNTH_HID_INPUT_REPORT:
	{
		synthhid_input_report* report = (synthhid_input_report*)msg->data;

		kernel.Printf("Report: %d\n", report->header.size);

		const uint32_t innerSize = msg->size + sizeof(struct pipe_prt_msg) - sizeof(unsigned char);
		kernel.Printf("innerSize %d struct %d\n", innerSize, sizeof(synthhid_input_report));
		Assert(innerSize == sizeof(synthhid_input_report));

		////TODO: wait for init?

		//Assert(size >= sizeof(synth_kbd_keystroke));
		//synth_kbd_keystroke key;
		//memcpy(&key, header, sizeof(synth_kbd_keystroke));
		////Print("Key: %d (%x) = %c Unicode: %d down: %d\n", key.make_code, key.make_code,(char)(key.make_code), key.info & IS_UNICODE, key.info & IS_BREAK);
		//VirtualKey keyCode = GetKeycode(key.make_code);

		//KeyEvent event = { 0 };
		//event.Key = keyCode;
		//event.Flags.Pressed = (key.make_code & IS_BREAK) != 0;
		//m_events.push(event);
		break;
	}
	break;

	default:
		Assert(false);
		break;
	}
}
