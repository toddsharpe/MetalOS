#pragma once

#include "kernel/Drivers/Driver.h"
#include "kernel/Drivers/HyperVChannel.h"
#include "user/MetalOS.Keys.h"

class HyperVKeyboardDriver : public Driver
{
public:
	HyperVKeyboardDriver(KDevice& device);

	Result Initialize() override;
	Result Enumerate() override;

	static void Callback(void* context) { ((HyperVKeyboardDriver*)context)->OnCallback(); };

private:
	//Current version 1.0
	static constexpr uint32_t SYNTH_KBD_VERSION_MAJOR = 1;
	static constexpr uint32_t SYNTH_KBD_VERSION_MINOR = 0;
	static constexpr uint32_t SYNTH_KBD_VERSION = (SYNTH_KBD_VERSION_MINOR | (SYNTH_KBD_VERSION_MAJOR << 16));
	
	//Ring buffer sizes
	static constexpr size_t KBD_VSC_SEND_RING_BUFFER_SIZE = (40 * 1024);
	static constexpr size_t KBD_VSC_RECV_RING_BUFFER_SIZE = (40 * 1024);

	//Proto
	static constexpr uint32_t PROTOCOL_ACCEPTED = 1;

	//Event flags
	static constexpr uint32_t IS_UNICODE = 0b0001;
	static constexpr uint32_t IS_BREAK = 0b0010;
	static constexpr uint32_t IS_E0 = 0b0100;
	static constexpr uint32_t IS_E1 = 0b1000;

	/*
	 * Message types in the synthetic input protocol
	 */
	enum class synth_kbd_msg_type
	{
		NONE,
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
		synth_kbd_msg_type type;
	};

	struct synth_kbd_msg
	{
		synth_kbd_msg_hdr header;
		char data[]; /* Enclosed message */
	};

	union synth_kbd_version
	{
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

	struct synth_kbd_protocol_response
	{
		synth_kbd_msg_hdr header;
		uint32_t proto_status;
	};

	struct synth_kbd_keystroke {
		struct synth_kbd_msg_hdr header;
		uint16_t make_code;
		uint16_t reserved0;
		uint32_t info; /* Additional information */
	};

	void OnCallback();
	void ProcessMessage(synth_kbd_msg_hdr* header, const uint32_t size);

	synth_kbd_protocol_response m_response;
	KEvent m_event;
	HyperVChannel m_channel;
};

