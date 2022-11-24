#pragma once

#include "Device.h"
#include "Driver.h"
#include "HyperVChannel.h"
#include <shared\MetalOS.Keys.h>
#include <kernel\MetalOS.Internal.h>

#include <queue>


//http://www.scs.stanford.edu/10wi-cs140/pintos/specs/kbd/scancodes-1.html
static const VirtualKey ScancodeSet1[] =
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

class HyperVKeyboardDriver : public Driver
{
public:
	HyperVKeyboardDriver(Device& device);

	Result Initialize() override;
	Result Read(char* buffer, size_t length, size_t* bytesRead = nullptr) override;
	Result Write(const char* buffer, size_t length) override;
	Result EnumerateChildren() override;

	static void Callback(void* context) { ((HyperVKeyboardDriver*)context)->OnCallback(); };

private:
	/*
	 * Current version 1.0
	 *
	 */
#define SYNTH_KBD_VERSION_MAJOR 1
#define SYNTH_KBD_VERSION_MINOR	0
#define SYNTH_KBD_VERSION		(SYNTH_KBD_VERSION_MINOR | \
					 (SYNTH_KBD_VERSION_MAJOR << 16))


	 /*
	  * Message types in the synthetic input protocol
	  */
	enum synth_kbd_msg_type {
		SYNTH_KBD_PROTOCOL_REQUEST = 1,
		SYNTH_KBD_PROTOCOL_RESPONSE = 2,
		SYNTH_KBD_EVENT = 3,
		SYNTH_KBD_LED_INDICATORS = 4,
	};

	/*
	 * Basic message structures.
	 */
	struct synth_kbd_msg_hdr
	{
		uint32_t type;
	};

	struct synth_kbd_msg
	{
		synth_kbd_msg_hdr header;
		char data[]; /* Enclosed message */
	};

	union synth_kbd_version {
		uint32_t version;
	};

	/*
	 * Protocol messages
	 */
	struct synth_kbd_protocol_request
	{
		synth_kbd_msg_hdr header;
		synth_kbd_version version_requested;
	};

#define BIT(nr) (1UL << (nr))
#define PROTOCOL_ACCEPTED	BIT(0)
	struct synth_kbd_protocol_response
	{
		synth_kbd_msg_hdr header;
		uint32_t proto_status;
	};

#define IS_UNICODE	BIT(0)
#define IS_BREAK	BIT(1)
#define IS_E0		BIT(2)
#define IS_E1		BIT(3)
	struct synth_kbd_keystroke {
		struct synth_kbd_msg_hdr header;
		uint16_t make_code;
		uint16_t reserved0;
		uint32_t info; /* Additional information */
	};


#define HK_MAXIMUM_MESSAGE_SIZE 256

#define KBD_VSC_SEND_RING_BUFFER_SIZE		(40 * 1024)
#define KBD_VSC_RECV_RING_BUFFER_SIZE		(40 * 1024)

#define XTKBD_EMUL0     0xe0
#define XTKBD_EMUL1     0xe1
#define XTKBD_RELEASE   0x80

	VirtualKey GetKeycode(uint16_t scanCode);

	void OnCallback();
	void ProcessMessage(synth_kbd_msg_hdr* header, const uint32_t size);

	synth_kbd_protocol_response m_response;

	KEvent m_event;
	HyperVChannel m_channel;
};

