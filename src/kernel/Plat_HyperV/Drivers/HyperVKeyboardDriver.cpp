#include "Lib/System.h"
#include "HyperVKeyboardDriver.h"
#include "kernel/KDevice.h"
#include <Assert.h>
#include "user/MetalOS.Types.h"
#include "user/MetalOS.UI.h"
#include "kernel/Api.h"

//http://www.scs.stanford.edu/10wi-cs140/pintos/specs/kbd/scancodes-1.html
static constexpr VirtualKey ScancodeSet1[] =
{
	/* 0x00 */ VK_UNMAPPED, VK_ESCAPE, '1', '2', '3', '4', '5', '6',
	/* 0x08 */ '7', '8', '9', '0', VK_OEM_MINUS, VK_OEM_1, VK_BACK, VK_TAB,
	/* 0x10 */ 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I',
	/* 0x18 */ 'O', 'P', VK_OEM_4, VK_OEM_6, VK_RETURN, VK_CONTROL, 'A', 'S',
	/* 0x20 */ 'D', 'F', 'G', 'H', 'J', 'K', 'L', VK_OEM_1,
	/* 0x28 */ VK_OEM_7, VK_OEM_3, VK_LSHIFT, VK_OEM_5, 'Z', 'X', 'C', 'V',
	/* 0x30 */ 'B', 'N', 'M', ',', '.', VK_OEM_2, VK_RSHIFT, VK_MULTIPLY,
	/* 0x38 */ VK_MENU, VK_SPACE, VK_CAPITAL, VK_F1, VK_F2, VK_F3, VK_F4, VK_F5,
	/* 0x40 */ VK_F6, VK_F7, VK_F8, VK_F9, VK_F10, VK_NUMLOCK, VK_SCROLL, VK_HOME,
	/* 0x48 */ VK_UP, VK_PRIOR, VK_SUBTRACT, VK_LEFT, VK_NUMPAD5, VK_RIGHT, VK_ADD, VK_NUMPAD1,
	/* 0x50 */ VK_DOWN, VK_NUMPAD3, VK_NUMPAD0, VK_DECIMAL, VK_UNMAPPED, VK_UNMAPPED, VK_UNMAPPED, VK_F11,
	/* 0x58 */ VK_F12
};

namespace
{
	VirtualKey GetKeycode(uint16_t scanCode)
	{
		if (scanCode >= ArraySize(ScancodeSet1))
			return VK_UNMAPPED;

		return ScancodeSet1[scanCode];
	}

	//The host sends typed text (e.g. VM Connect's "Type clipboard text") as UTF-16 code units; map them to the keys the scan code path produces
	VirtualKey GetUnicodeKeycode(const uint16_t c)
	{
		if (c >= 'a' && c <= 'z')
			return (VirtualKey)(c - 'a' + 'A');
		if ((c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == ',' || c == '.')
			return (VirtualKey)c;

		switch (c)
		{
			case ' ':
				return VK_SPACE;

			case '\r':
			case '\n':
				return VK_RETURN;

			case '\b':
				return VK_BACK;

			case '\t':
				return VK_TAB;

			case '-':
				return VK_OEM_MINUS;

			case '/':
				return VK_OEM_2;

			default:
				return VK_UNMAPPED;
		}
	}
}

HyperVKeyboardDriver::HyperVKeyboardDriver(KDevice& device) :
	Driver(device),
	m_response(),
	m_event(false, false),
	m_channel(KBD_VSC_SEND_RING_BUFFER_SIZE, KBD_VSC_RECV_RING_BUFFER_SIZE, { &HyperVKeyboardDriver::Callback, this })
{

}

bool HyperVKeyboardDriver::Initialize()
{
	m_device.Class = KDeviceClass::Keyboard;

	//Initialize channel with the device's offer, not a stack copy: the channel keeps the pointer for SetEvent
	m_channel.Initialize(*VmBusChild::Of(m_device).Bus, &VmBusChild::Of(m_device).Offer);
	
	//Initialize keyboard
	synth_kbd_protocol_request request = { };
	request.header.type = synth_kbd_msg_type::SYNTH_KBD_PROTOCOL_REQUEST;
	request.version_requested.version = SYNTH_KBD_VERSION;

	m_channel.SendPacket(&request, sizeof(synth_kbd_protocol_request), (uint64_t)&request, HyperV::VM_PKT_DATA_INBAND, VMBUS_DATA_PACKET_FLAG_COMPLETION_REQUESTED);
	KeWait(m_event);
	Assert(m_response.proto_status & PROTOCOL_ACCEPTED);

	return true;
}

void HyperVKeyboardDriver::OnCallback()
{
	m_channel.Drain([this](const HyperV::vmpacket_descriptor& packet, uint8_t* const payload, const uint32_t size)
	{
		switch (packet.type)
		{
		case HyperV::VM_PKT_COMP:
			break;

		case HyperV::VM_PKT_DATA_INBAND:
		{
			Assert(size > sizeof(struct synth_kbd_msg_hdr));

			synth_kbd_msg_hdr* keyboardMessage = (synth_kbd_msg_hdr*)payload;
			ProcessMessage(keyboardMessage, size);
		}
		break;

		default:
			Assert(false);
			break;
		}
	});
}

void HyperVKeyboardDriver::ProcessMessage(synth_kbd_msg_hdr* header, const uint32_t size)
{
	switch (header->type)
	{
		case synth_kbd_msg_type::SYNTH_KBD_PROTOCOL_RESPONSE:
		{
			Assert(size >= sizeof(synth_kbd_protocol_response));
			memcpy(&m_response, header, sizeof(synth_kbd_protocol_response));
			m_event.Set();
		}
		break;

		case synth_kbd_msg_type::SYNTH_KBD_EVENT:
		{
			Assert(size >= sizeof(synth_kbd_keystroke));
			synth_kbd_keystroke key;
			memcpy(&key, header, sizeof(synth_kbd_keystroke));
			const VirtualKey keyCode = (key.info & IS_UNICODE) ? GetUnicodeKeycode(key.make_code) : GetKeycode(key.make_code);

			Message message = {};
			message.Header.MessageType = MessageType::KeyEvent;
			message.KeyEvent.Key = keyCode;
			message.KeyEvent.Flags.Pressed = (key.info & IS_BREAK) == 0;
			KeInputPost(message); //feed the kernel->WM input ring
			break;
		}
		break;

		default:
			Assert(false);
			break;
	}
}
