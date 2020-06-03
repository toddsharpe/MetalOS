#pragma once
#include "Device.h"
#include "Driver.h"
#include "HyperVChannel.h"
#include "MetalOS.h"

#include <queue>


namespace
{
	struct hv_input_dev_info {
		unsigned int size;
		unsigned short vendor;
		unsigned short product;
		unsigned short version;
		unsigned short reserved[11];
	};

	/* The maximum size of a synthetic input message. */
#define SYNTHHID_MAX_INPUT_REPORT_SIZE 16

/*
 * Current version
 *
 * History:
 * Beta, RC < 2008/1/22        1,0
 * RC > 2008/1/22              2,0
 */
#define SYNTHHID_INPUT_VERSION_MAJOR	2
#define SYNTHHID_INPUT_VERSION_MINOR	0
#define SYNTHHID_INPUT_VERSION		(SYNTHHID_INPUT_VERSION_MINOR | \
					 (SYNTHHID_INPUT_VERSION_MAJOR << 16))


#pragma pack(push, 1)
 /*
  * Message types in the synthetic input protocol
  */
	enum synthhid_msg_type {
		SYNTH_HID_PROTOCOL_REQUEST,
		SYNTH_HID_PROTOCOL_RESPONSE,
		SYNTH_HID_INITIAL_DEVICE_INFO,
		SYNTH_HID_INITIAL_DEVICE_INFO_ACK,
		SYNTH_HID_INPUT_REPORT,
		SYNTH_HID_MAX
	};

	/*
	 * Basic message structures.
	 */
	struct synthhid_msg_hdr {
		enum synthhid_msg_type type;
		u32 size;
	};

	struct synthhid_msg {
		struct synthhid_msg_hdr header;
		char data[1]; /* Enclosed message */
	};

	union synthhid_version {
		struct {
			u16 minor_version;
			u16 major_version;
		};
		u32 version;
	};

	/*
	 * Protocol messages
	 */
	struct synthhid_protocol_request {
		struct synthhid_msg_hdr header;
		union synthhid_version version_requested;
	};

	struct synthhid_protocol_response {
		struct synthhid_msg_hdr header;
		union synthhid_version version_requested;
		unsigned char approved;
	};

	struct hid_class_descriptor {
		uint8_t  bDescriptorType;
		uint16_t wDescriptorLength;
	};

	struct hid_descriptor {
		uint8_t  bLength;
		uint8_t  bDescriptorType;
		uint16_t bcdHID;
		uint8_t  bCountryCode;
		uint8_t  bNumDescriptors;

		struct hid_class_descriptor desc[1];
	};

	struct synthhid_device_info {
		struct synthhid_msg_hdr header;
		struct hv_input_dev_info hid_dev_info;
		struct hid_descriptor hid_descriptor;
	};

	struct synthhid_device_info_ack {
		struct synthhid_msg_hdr header;
		unsigned char reserved;
	};

	struct synthhid_input_report {
		struct synthhid_msg_hdr header;
		char buffer[1];
	};

#pragma pack(pop)

#define INPUTVSC_SEND_RING_BUFFER_SIZE		(40 * 1024)
#define INPUTVSC_RECV_RING_BUFFER_SIZE		(40 * 1024)


	enum pipe_prot_msg_type {
		PIPE_MESSAGE_INVALID,
		PIPE_MESSAGE_DATA,
		PIPE_MESSAGE_MAXIMUM
	};


	struct pipe_prt_msg {
		enum pipe_prot_msg_type type;
		u32 size;
		char data[1];
	};

	struct  mousevsc_prt_msg {
		enum pipe_prot_msg_type type;
		u32 size;
		union {
			struct synthhid_protocol_request request;
			struct synthhid_protocol_response response;
			struct synthhid_device_info_ack ack;
		};
	};
}

class HyperVMouseDriver : public Driver
{
public:
	HyperVMouseDriver(Device& device);

	Result Initialize() override;
	Result Read(const char* buffer, size_t length) override;
	Result Write(const char* buffer, size_t length) override;
	Result EnumerateChildren() override;

	static void Callback(void* context) { ((HyperVMouseDriver*)context)->OnCallback(); };

private:
	void OnCallback();
	void ProcessMessage(pipe_prt_msg* msg, const uint32_t size);

	mousevsc_prt_msg m_response;

	std::queue<KeyEvent> m_events;

	Handle m_semaphore;
	HyperVChannel m_channel;
};


