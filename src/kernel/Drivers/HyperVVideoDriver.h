#pragma once

#include "kernel/Drivers/Driver.h"
#include "kernel/Drivers/HyperVChannel.h"
#include "kernel/Objects/KEvent.h"

//Hyper-V synthetic video (VSC), device class {DA0A7802-E377-4AAC-8E77-0558EB1073F8}. Negotiates
//the synthvid protocol with the host, points it at a guest-physical VRAM buffer, and reports a
//display situation (resolution / pitch). Draw 32bpp pixels into Framebuffer(), then call Flush()
//to push the changed region to the host.
class HyperVVideoDriver : public DisplayDriver
{
public:
	HyperVVideoDriver(KDevice& device);

	Result Initialize() override;
	Result Enumerate() override;

	//DisplayDriver: mode is negotiated with the host over synthvid; the framebuffer is the shared
	//guest-physical VRAM (the GOP MMIO region). Only 32bpp is supported.
	Result SetMode(const uint32_t width, const uint32_t height, const uint32_t bpp) override;
	Result GetMode(uint32_t& width, uint32_t& height, uint32_t& bpp) const override;
	Result GetFrameBuffer(void*& outAddress, size_t& outSize) const override;

	static void Callback(void* context) { ((HyperVVideoDriver*)context)->OnCallback(); }

	//Guest VRAM (linear, 32bpp), or nullptr until Initialize() succeeds.
	void* Framebuffer() const { return m_framebuffer; }

	//Tell the host the whole screen changed.
	void Flush() override;

private:
	//synthvid protocol version 3.2 (Windows 8.1 / Server 2012 R2 and later, Gen2 VMs).
	static constexpr uint32_t SYNTHVID_VERSION_MAJOR = 3;
	static constexpr uint32_t SYNTHVID_VERSION_MINOR = 2;
	static constexpr uint32_t SYNTHVID_VERSION = (SYNTHVID_VERSION_MINOR << 16) | SYNTHVID_VERSION_MAJOR;

	//The synthvid channel runs over the VMBus "pipe" protocol: every message is prefixed with a
	//pipe_msg_hdr { type=PIPE_MSG_DATA, size=<synthvid message bytes> }.
	static constexpr uint32_t PIPE_MSG_DATA = 1;

	//Kept small so both rings' page frames fit in a single GPADL header (HyperVChannel doesn't
	//split into a GPADL body). synthvid messages are tiny, so this is plenty.
	static constexpr size_t VSC_SEND_RING_BUFFER_SIZE = (40 * 1024);
	static constexpr size_t VSC_RECV_RING_BUFFER_SIZE = (40 * 1024);

	enum class synthvid_msg_type : uint32_t
	{
		Error = 0,
		VersionRequest = 1,
		VersionResponse = 2,
		VramLocation = 3,
		VramLocationAck = 4,
		SituationUpdate = 5,
		SituationUpdateAck = 6,
		PointerPosition = 7,
		PointerShape = 8,
		FeatureChange = 9,
		Dirt = 10,
	};

#pragma pack(push, 1)
	struct pipe_msg_hdr
	{
		uint32_t type; //PIPE_MSG_DATA
		uint32_t size; //bytes of synthvid message that follow (synthvid_msg_hdr + payload)
	};

	struct synthvid_msg_hdr
	{
		synthvid_msg_type type;
		uint32_t size; //size of this header plus the payload that follows
	};

	struct synthvid_version_request
	{
		uint32_t version;
	};

	struct synthvid_version_response
	{
		uint32_t version;
		uint8_t is_accepted;
		uint8_t max_video_outputs;
	};

	struct synthvid_vram_location
	{
		uint64_t user_ctx;
		uint8_t is_vram_gpa_specified;
		uint64_t vram_gpa;
	};

	struct video_output_situation
	{
		uint8_t active;
		uint32_t vram_offset;
		uint8_t depth_bits;
		uint32_t width_pixels;
		uint32_t height_pixels;
		uint32_t pitch_bytes;
	};

	struct synthvid_situation_update
	{
		uint64_t user_ctx;
		uint8_t video_output_count;
		video_output_situation video_output[1];
	};

	struct synthvid_rect
	{
		int32_t x1, y1, x2, y2;
	};

	struct synthvid_dirt
	{
		uint8_t video_output;
		uint8_t dirt_count;
		synthvid_rect dirt[1];
	};

	//Full wire messages (pipe header + synthvid header + payload), packed so sizeof() is the exact
	//byte count on the wire.
	struct VersionRequestMsg { pipe_msg_hdr pipe; synthvid_msg_hdr hdr; synthvid_version_request body; };
	struct VramLocationMsg { pipe_msg_hdr pipe; synthvid_msg_hdr hdr; synthvid_vram_location body; };
	struct SituationUpdateMsg { pipe_msg_hdr pipe; synthvid_msg_hdr hdr; synthvid_situation_update body; };
	struct DirtMsg { pipe_msg_hdr pipe; synthvid_msg_hdr hdr; synthvid_dirt body; };
#pragma pack(pop)

	void OnCallback();
	void ProcessMessage(synthvid_msg_hdr* header, const uint32_t size);
	bool SendSituationUpdate(const uint32_t width, const uint32_t height, const uint32_t pitchBytes);

	uint32_t m_width;
	uint32_t m_height;
	uint32_t m_pitchBytes;
	size_t m_barSize; //bytes of the framebuffer BAR (upper bound on a mode's VRAM)
	paddr_t m_framebufferGpa;
	void* m_framebuffer; //kernel VA of the VRAM (KernelPhysicalStart + gpa)

	synthvid_version_response m_versionResponse;
	KEvent m_event;
	HyperVChannel m_channel;
};
