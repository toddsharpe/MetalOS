#include <Kernel.h>
#include <Assert.h>

#include "HyperVKeyboardDriver.h"
#include "HyperVDevice.h"

HyperVKeyboardDriver::HyperVKeyboardDriver(Device& device) :
	Driver(device),
	m_response(),
	m_semaphore(),
	m_channel(KBD_VSC_SEND_RING_BUFFER_SIZE, KBD_VSC_RECV_RING_BUFFER_SIZE, { &HyperVKeyboardDriver::Callback, this })
{
	m_semaphore = kernel.KeCreateSemaphore(0, 0, "HyperVKeyboardDriver");
}

Result HyperVKeyboardDriver::Initialize()
{
	kernel.Printf("HyperVKeyboardDriver::Initialize\n");
	
	m_device.Type = DeviceType::Keyboard;

	HyperVDevice* device = (HyperVDevice*)&m_device;
	uint32_t childRelid = *(uint32_t*)device->GetResource(HyperVDevice::ResourceType::ChildRelid);
	uint32_t connectionId = *(uint32_t*)device->GetResource(HyperVDevice::ResourceType::ConnectionId);
	vmbus_channel_offer_channel* offerChannel = (vmbus_channel_offer_channel*)device->GetResource(HyperVDevice::ResourceType::OfferChannel);

	//Initialize channel
	m_channel.Initialize(offerChannel);
	
	//Initialize keyboard
	synth_kbd_protocol_request request = { 0 };
	request.header.type = SYNTH_KBD_PROTOCOL_REQUEST;
	request.version_requested.version = SYNTH_KBD_VERSION;

	this->m_channel.SendPacket(&request, sizeof(synth_kbd_protocol_request), (uint64_t)&request, VM_PKT_DATA_INBAND, VMBUS_DATA_PACKET_FLAG_COMPLETION_REQUESTED);
	kernel.KeWaitForSemaphore(*m_semaphore, INT64_MAX);
	Assert(m_response.proto_status & PROTOCOL_ACCEPTED);

	return Result::Success;
}

Result HyperVKeyboardDriver::Read(char* buffer, size_t length, size_t* bytesRead)
{
	return Result::NotImplemented;
}

Result HyperVKeyboardDriver::Write(const char* buffer, size_t length)
{
	return Result::NotImplemented;
}

Result HyperVKeyboardDriver::EnumerateChildren()
{
	return Result::NotImplemented;
}

void HyperVKeyboardDriver::OnCallback()
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
			Assert(size > sizeof(struct synth_kbd_msg_hdr));

			synth_kbd_msg_hdr* keyboardMessage = (synth_kbd_msg_hdr*)((uintptr_t)packet + (packet->offset8 << 3));
			ProcessMessage(keyboardMessage, size);
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

void HyperVKeyboardDriver::ProcessMessage(synth_kbd_msg_hdr* header, const uint32_t size)
{
	switch (header->type)
	{
	case SYNTH_KBD_PROTOCOL_RESPONSE:
	{
		Assert(size >= sizeof(synth_kbd_protocol_response));
		memcpy(&m_response, header, sizeof(synth_kbd_protocol_response));
		kernel.Printf("Keyboard Connected: %d\n", m_response.proto_status);
		kernel.KeReleaseSemaphore(*m_semaphore, 1);
	}
	break;

	case SYNTH_KBD_EVENT:
	{
		Assert(size >= sizeof(synth_kbd_keystroke));
		synth_kbd_keystroke key;
		memcpy(&key, header, sizeof(synth_kbd_keystroke));
		Assert(!(key.info & IS_UNICODE));
		VirtualKey keyCode = GetKeycode(key.make_code);

		Message* msg = new Message();
		memset(msg, 0, sizeof(Message));
		msg->Header.MessageType = MessageType::KeyEvent;
		msg->KeyEvent.Key = keyCode;
		msg->KeyEvent.Flags.Pressed = (key.info & IS_BREAK) == 0;
		kernel.KePostMessage(msg);
		break;
	}
	break;

	default:
		Assert(false);
		break;
	}
}

VirtualKey HyperVKeyboardDriver::GetKeycode(uint16_t scanCode)
{
	if (scanCode >= COUNT_OF(ScancodeSet1))
		return VK_UNMAPPED;

	return ScancodeSet1[scanCode];
}
