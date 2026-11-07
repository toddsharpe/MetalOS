#pragma once

#include "kernel/Drivers/Driver.h"
#include "kernel/Plat_HyperV/Drivers/HyperVChannel.h"
#include "kernel/Objects/KEvent.h"
#include "kernel/Kernel_display.h"

//Hyper-V synthetic video (VSC) {DA0A7802-E377-4AAC-8E77-0558EB1073F8}: negotiates synthvid, points the host at the VRAM and sets the mode
class HyperVVideoDriver : public DisplayDriver
{
public:
	HyperVVideoDriver(KDevice& device);

	bool Initialize() override;

	//DisplayDriver: the shared guest-physical VRAM (the GOP MMIO region) at the 32bpp mode negotiated with the host
	KFramebufferInfo GetFramebuffer() const override;

	static void Callback(void* context) { ((HyperVVideoDriver*)context)->OnCallback(); }

	//Tell the host the whole screen changed.
	void Flush() override;

private:
	//The synthvid channel runs over the VMBus "pipe" protocol: every message is prefixed with a
	//pipe_msg_hdr { type=PIPE_MSG_DATA, size=<synthvid message bytes> }.
	static constexpr uint32_t PIPE_MSG_DATA = 1;

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
		ResolutionRequest = 13,
		ResolutionResponse = 14,
	};

	static constexpr uint32_t SYNTHVID_VERSION_WIN10 = (5u << 16) | 3u;
	static constexpr size_t MAX_RESOLUTIONS = 64;

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

	struct synthvid_situation_update_ack
	{
		uint64_t user_ctx; //the update's, echoed back
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

	struct hvd_screen_info
	{
		uint16_t width;
		uint16_t height;
	};

	struct synthvid_resolution_request
	{
		uint8_t maximum_resolution_count;
	};

	struct synthvid_resolution_response
	{
		uint8_t edid_block[128];
		uint8_t resolution_count;
		uint8_t default_resolution_index;
		uint8_t is_standard;
		hvd_screen_info supported_resolution[MAX_RESOLUTIONS];
	};

	//Full wire messages (pipe header + synthvid header + payload), packed so sizeof() is the exact
	//byte count on the wire.
	struct VersionRequestMsg { pipe_msg_hdr pipe; synthvid_msg_hdr hdr; synthvid_version_request body; };
	struct VramLocationMsg { pipe_msg_hdr pipe; synthvid_msg_hdr hdr; synthvid_vram_location body; };
	struct SituationUpdateMsg { pipe_msg_hdr pipe; synthvid_msg_hdr hdr; synthvid_situation_update body; };
	struct DirtMsg { pipe_msg_hdr pipe; synthvid_msg_hdr hdr; synthvid_dirt body; };
	struct ResolutionRequestMsg { pipe_msg_hdr pipe; synthvid_msg_hdr hdr; synthvid_resolution_request body; };
#pragma pack(pop)

	void OnCallback();
	void ProcessMessage(synthvid_msg_hdr* header, const uint32_t size);
	bool Negotiate();
	bool Request(const void* const message, const size_t size, const synthvid_msg_type answer);
	bool QueryResolutions();
	bool SetLargestMode();
	bool SetMode(const uint32_t width, const uint32_t height);
	bool SendSituationUpdate(const uint32_t width, const uint32_t height, const uint32_t pitchBytes);

	uint32_t m_width;
	uint32_t m_height;
	uint32_t m_pitchBytes;
	size_t m_vramSize; //bytes the host maps at m_framebufferGpa (upper bound on a mode)
	paddr_t m_framebufferGpa;

	uint32_t m_version;
	synthvid_version_response m_versionResponse;
	synthvid_resolution_response m_resolutions;
	KEvent m_event;
	volatile bool m_waiting; //a request is waiting for an m_answer message; only that sets m_event
	volatile synthvid_msg_type m_answer;
	volatile uint64_t m_situationContext; //user_ctx of the latest SITUATION_UPDATE (1, 2, ...)
	HyperVChannel m_channel;
};
