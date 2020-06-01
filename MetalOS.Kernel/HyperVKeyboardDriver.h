#pragma once

#include "Device.h"
#include "Driver.h"
#include "HyperVChannel.h"

//Scancodes to keyodes


class HyperVKeyboardDriver : public Driver
{
public:
	HyperVKeyboardDriver(Device& device);

	Result Initialize() override;
	Result Read(const char* buffer, size_t length) override;
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


	void OnCallback();
	void ProcessMessage(synth_kbd_msg_hdr* header, const uint32_t size);

	synth_kbd_protocol_response m_response;

	Handle m_semaphore;
	HyperVChannel m_channel;
};

