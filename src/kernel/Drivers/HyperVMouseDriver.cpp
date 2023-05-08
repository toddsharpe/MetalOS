#include <Kernel.h>
#include <Assert.h>

#include "HyperVMouseDriver.h"
#include "Devices/HyperVDevice.h"

//TODO: could be refactored into HID + HyperV portions
//For now it does both
HyperVMouseDriver::HyperVMouseDriver(Device& device) :
	Driver(device),
	m_response(),
	m_event(false, false),
	m_channel(INPUTVSC_SEND_RING_BUFFER_SIZE, INPUTVSC_RECV_RING_BUFFER_SIZE, { &HyperVMouseDriver::Callback, this })
{

}

Result HyperVMouseDriver::Initialize()
{
	kernel.Printf("HyperVMouseDriver::Initialize\n");

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
		(uint64_t)&request, VM_PKT_DATA_INBAND, VMBUS_DATA_PACKET_FLAG_COMPLETION_REQUESTED);

	//Wait for SYNTH_HID_PROTOCOL_RESPONSE and SYNTH_HID_INITIAL_DEVICE_INFO
	kernel.KeWait(m_event);

	//Send info ack
	mousevsc_prt_msg ack;
	memset(&ack, 0, sizeof(struct mousevsc_prt_msg));
	ack.type = PIPE_MESSAGE_DATA;
	ack.size = sizeof(struct synthhid_device_info_ack);
	ack.ack.header.type = SYNTH_HID_INITIAL_DEVICE_INFO_ACK;
	ack.ack.header.size = 1;
	ack.ack.reserved = 0;

	this->m_channel.SendPacket(&ack, sizeof(struct pipe_prt_msg) - sizeof(unsigned char) + sizeof(struct synthhid_device_info_ack),
		(uint64_t)&ack, VM_PKT_DATA_INBAND, VMBUS_DATA_PACKET_FLAG_COMPLETION_REQUESTED);

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

	//See: MouseHidReport.txt
	case SYNTH_HID_INITIAL_DEVICE_INFO:
	{
		Assert(msg->size >= sizeof(struct hv_input_dev_info));
		kernel.Printf("SYNTH_HID_INITIAL_DEVICE_INFO\n");

		synthhid_device_info* info = (synthhid_device_info*)msg->data;
		kernel.Printf("Vendor 0x%x Product: 0x%x Version 0x%x\n", info->hid_dev_info.vendor, info->hid_dev_info.product, info->hid_dev_info.version);

		//HID 1.1 E.8 HID Descriptor (Mouse)
		Assert(info->hid_descriptor.bLength = 0x9);
		Assert(info->hid_descriptor.bDescriptorType == HID_DT_HID);
		Assert(info->hid_descriptor.bcdHID == 0x101);
		Assert(info->hid_descriptor.bNumDescriptors == 1); //Report descriptor

		const uint32_t innerSize = msg->size + sizeof(struct pipe_prt_msg) - sizeof(unsigned char);
		kernel.Printf("MsgSize 0x%x, Inner? 0x%x\n", msg->size, innerSize);
		
		//msg->size = sizeof(synthhid_device_info) + wDescriptorLength

		kernel.Printf("Info 0x%x\n", sizeof(synthhid_device_info));

		kernel.Printf("T: 0x%x, L: 0x%x\n", info->hid_descriptor.desc[0].bDescriptorType, info->hid_descriptor.desc[0].wDescriptorLength);

		//TODO: actually parse the report
		//char* start = (char*)&info->hid_descriptor + info->hid_descriptor.bLength;
		//info->hid_descriptor.desc[0].wDescriptorLength
		//kernel.PrintBytes(start, info->hid_descriptor.desc[0].wDescriptorLength);

		//Ack device info
		m_event.Set();
	}
	break;

	case SYNTH_HID_INPUT_REPORT:
	{
		//TODO: wait for init?

		synthhid_input_report* report = (synthhid_input_report*)msg->data;
		
		//From inspection
		//off, len
		//  0, 1 - Buttons
		//  1, 2 - X position (0, 32767) - int16 pos
		//  3, 2 - y position (0, 32767) - int16 pos
		//  5, 2 - wheel (-32767, 32767) - int16 full range
		Assert(report->header.size == 0x7);

		//Parse
		bool leftClick = report->buffer[0] & 1;
		bool rightClick = report->buffer[0] & 2;
		uint16_t x = *(uint16_t*)&report->buffer[1];
		uint16_t y = *(uint16_t*)&report->buffer[3];
		uint16_t wheel = *(uint16_t*)&report->buffer[5];
		//kernel.Printf("X: %d, Y: %d, Wheel: %d, L: %d R: %d\n", x, y, wheel, leftClick, rightClick);

		Message message = {};
		message.Header.MessageType = MessageType::MouseEvent;
		message.MouseEvent.Buttons.LeftPressed = leftClick;
		message.MouseEvent.Buttons.RightPressed = rightClick;
		message.MouseEvent.XPosition = x;
		message.MouseEvent.YPosition = y;
		kernel.KePostMessage(message);
		break;
	}
	break;

	default:
		Assert(false);
		break;
	}
}
