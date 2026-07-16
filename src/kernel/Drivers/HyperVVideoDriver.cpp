#include "Lib/System.h"
#include "HyperVVideoDriver.h"
#include "kernel/KDevice.h"
#include "kernel/Api.h"
#include "MetalOS.Space.h"
#include "Assert.h"

HyperVVideoDriver::HyperVVideoDriver(KDevice& device) :
	DisplayDriver(device),
	m_width(),
	m_height(),
	m_pitchBytes(),
	m_barSize(),
	m_framebufferGpa(),
	m_framebuffer(),
	m_versionResponse(),
	m_event(false, false),
	m_channel(VSC_SEND_RING_BUFFER_SIZE, VSC_RECV_RING_BUFFER_SIZE, { &HyperVVideoDriver::Callback, this })
{

}

Result HyperVVideoDriver::Initialize()
{
	m_device.Class = KDeviceClass::Display;

	//Open the VMBus channel (rings + GPADL happen here).
	m_channel.Initialize(&m_device.m_channel);

	//1) Negotiate the synthvid protocol version, newest first (matches modern hyperv_fb):
	//3.5 (Win10/Server2016), 3.2 (Win8.1/Server2012R2), 3.0 (Win7/Server2008R2).
	static constexpr uint32_t kVersions[] = { (5u << 16) | 3u, (2u << 16) | 3u, (0u << 16) | 3u };
	bool negotiated = false;
	for (size_t i = 0; i < sizeof(kVersions) / sizeof(kVersions[0]); i++)
	{
		VersionRequestMsg version = {};
		version.pipe.type = PIPE_MSG_DATA;
		version.pipe.size = sizeof(version) - sizeof(pipe_msg_hdr);
		version.hdr.type = synthvid_msg_type::VersionRequest;
		version.hdr.size = sizeof(version) - sizeof(pipe_msg_hdr);
		version.body.version = kVersions[i];
		m_channel.SendPacket(&version, sizeof(version), (uint64_t)&version, HyperV::VM_PKT_DATA_INBAND, VMBUS_DATA_PACKET_FLAG_COMPLETION_REQUESTED);
		if (KeWait(m_event, 2000) != KWaitResult::Signaled)
		{
			Printf("HyperVVideo: no version response for 0x%x (child_relid=%u)\n", kVersions[i], m_device.child_relid);
			return Result::Failed;
		}
		if (m_versionResponse.is_accepted)
		{
			negotiated = true;
			break;
		}
	}
	if (!negotiated)
	{
		Printf("HyperVVideo: host rejected all synthvid versions\n");
		return Result::Failed;
	}

	//2) Use the UEFI GOP framebuffer as the VRAM. On a Gen2 VM the GOP framebuffer IS the Hyper-V
	//synthetic-video MMIO region, and the host only accepts a VRAM GPA inside that region — arbitrary
	//guest RAM is rejected (no VRAM_LOCATION_ACK). This also makes the host display exactly what
	//MetalOS already renders into the GOP framebuffer, so there is no separate/blank buffer.
	const KFramebufferInfo fb = KeGetFramebuffer();
	m_width = fb.Width;
	m_height = fb.Height;
	m_pitchBytes = fb.Pitch * sizeof(uint32_t); //Pitch is in pixels
	m_framebufferGpa = fb.Base;
	m_barSize = fb.Size; //size of the framebuffer BAR; a mode can't need more VRAM than this
	m_framebuffer = reinterpret_cast<void*>(KernelPhysicalStart + m_framebufferGpa);

	//3) Point the host at the VRAM (a guest-physical address inside the MMIO framebuffer region).
	VramLocationMsg vram = {};
	vram.pipe.type = PIPE_MSG_DATA;
	vram.pipe.size = sizeof(vram) - sizeof(pipe_msg_hdr);
	vram.hdr.type = synthvid_msg_type::VramLocation;
	vram.hdr.size = sizeof(vram) - sizeof(pipe_msg_hdr);
	vram.body.user_ctx = m_framebufferGpa;
	vram.body.is_vram_gpa_specified = 1;
	vram.body.vram_gpa = m_framebufferGpa;
	m_channel.SendPacket(&vram, sizeof(vram), (uint64_t)&vram, HyperV::VM_PKT_DATA_INBAND, VMBUS_DATA_PACKET_FLAG_COMPLETION_REQUESTED);
	if (KeWait(m_event, 2000) != KWaitResult::Signaled)
	{
		Printf("HyperVVideo: no VRAM_LOCATION ack\n");
		return Result::Failed;
	}

	//4) Set the display mode, negotiated with the host over synthvid (no UEFI/GOP mode change). Try
	//1920x1080; if the host rejects it, fall back to the boot GOP mode (already in m_width/m_height).
	if (SetMode(1920, 1080, 32) != Result::Success)
		SendSituationUpdate(m_width, m_height, m_pitchBytes);

	Printf("HyperVVideo: %dx%d, VRAM at 0x%016x (gpa 0x%016x)\n", m_width, m_height, m_framebuffer, m_framebufferGpa);
	return Result::Success;
}

Result HyperVVideoDriver::SetMode(const uint32_t width, const uint32_t height, const uint32_t bpp)
{
	if (bpp != 32)
		return Result::NotImplemented;

	const uint32_t pitchBytes = width * sizeof(uint32_t);

	//The host scans out of the shared BAR at m_framebufferGpa; a mode that needs more VRAM than the
	//BAR would make the host read past it (garbled/torn lower rows). Refuse it.
	if ((size_t)height * pitchBytes > m_barSize)
	{
		Printf("HyperVVideo: mode %ux%u needs 0x%x > BAR 0x%x; refusing\n",
			width, height, (uint32_t)((size_t)height * pitchBytes), (uint32_t)m_barSize);
		return Result::Failed;
	}

	if (!SendSituationUpdate(width, height, pitchBytes))
		return Result::Failed;

	//Only the negotiated mode is recorded here. Callers that draw (e.g. MapFramebuffer -> the WM)
	//map the VRAM at m_framebufferGpa for this size themselves.
	m_width = width;
	m_height = height;
	m_pitchBytes = pitchBytes;
	return Result::Success;
}

Result HyperVVideoDriver::GetMode(uint32_t& width, uint32_t& height, uint32_t& bpp) const
{
	width = m_width;
	height = m_height;
	bpp = 32;
	return Result::Success;
}

Result HyperVVideoDriver::GetFrameBuffer(void*& outAddress, size_t& outSize) const
{
	//Guest-physical base of the VRAM (the caller maps it); size covers the current mode.
	outAddress = reinterpret_cast<void*>(m_framebufferGpa);
	outSize = (size_t)m_height * m_pitchBytes;
	return Result::Success;
}

//Sends a SITUATION_UPDATE for a single active output and waits for the host's ack. Returns true if
//the host accepted the mode within the timeout.
bool HyperVVideoDriver::SendSituationUpdate(const uint32_t width, const uint32_t height, const uint32_t pitchBytes)
{
	SituationUpdateMsg situation = {};
	situation.pipe.type = PIPE_MSG_DATA;
	situation.pipe.size = sizeof(situation) - sizeof(pipe_msg_hdr);
	situation.hdr.type = synthvid_msg_type::SituationUpdate;
	situation.hdr.size = sizeof(situation) - sizeof(pipe_msg_hdr);
	situation.body.user_ctx = 0;
	situation.body.video_output_count = 1;
	situation.body.video_output[0].active = 1;
	situation.body.video_output[0].vram_offset = 0;
	situation.body.video_output[0].depth_bits = 32;
	situation.body.video_output[0].width_pixels = width;
	situation.body.video_output[0].height_pixels = height;
	situation.body.video_output[0].pitch_bytes = pitchBytes;
	m_channel.SendPacket(&situation, sizeof(situation), (uint64_t)&situation, HyperV::VM_PKT_DATA_INBAND, VMBUS_DATA_PACKET_FLAG_COMPLETION_REQUESTED);
	return KeWait(m_event, 2000) == KWaitResult::Signaled;
}

Result HyperVVideoDriver::Enumerate()
{
	return Result::NotImplemented;
}

void HyperVVideoDriver::Flush()
{
	//Mark the entire framebuffer dirty so the host re-reads it.
	DirtMsg dirt = {};
	dirt.pipe.type = PIPE_MSG_DATA;
	dirt.pipe.size = sizeof(dirt) - sizeof(pipe_msg_hdr);
	dirt.hdr.type = synthvid_msg_type::Dirt;
	dirt.hdr.size = sizeof(dirt) - sizeof(pipe_msg_hdr);
	dirt.body.video_output = 0;
	dirt.body.dirt_count = 1;
	dirt.body.dirt[0].x1 = 0;
	dirt.body.dirt[0].y1 = 0;
	dirt.body.dirt[0].x2 = (int32_t)m_width;
	dirt.body.dirt[0].y2 = (int32_t)m_height;
	m_channel.SendPacket(&dirt, sizeof(dirt), (uint64_t)&dirt, HyperV::VM_PKT_DATA_INBAND, 0);
}

void HyperVVideoDriver::OnCallback()
{
	HyperV::vmpacket_descriptor* packet;
	uint32_t length = sizeof(HyperV::vmpacket_descriptor);
	do
	{
		while ((packet = (HyperV::vmpacket_descriptor*)m_channel.ReadPacket(length)) != nullptr)
		{
			switch (packet->type)
			{
				case HyperV::VM_PKT_COMP:
					break;

				case HyperV::VM_PKT_DATA_INBAND:
				{
					//Incoming messages are pipe-wrapped: [pipe_msg_hdr][synthvid_msg_hdr][payload].
					const uint32_t size = (packet->len8 << 3) - (packet->offset8 << 3);
					uint8_t* const data = (uint8_t*)packet + (packet->offset8 << 3);
					pipe_msg_hdr* const pipe = (pipe_msg_hdr*)data;
					if (size >= sizeof(pipe_msg_hdr) + sizeof(synthvid_msg_hdr) && pipe->type == PIPE_MSG_DATA)
					{
						synthvid_msg_hdr* header = (synthvid_msg_hdr*)(data + sizeof(pipe_msg_hdr));
						ProcessMessage(header, size - sizeof(pipe_msg_hdr));
					}
				}
				break;

				default:
					break;
			}

			m_channel.NextPacket(packet->len8 << 3);
		}
	} while (m_channel.StopRead());
}

void HyperVVideoDriver::ProcessMessage(synthvid_msg_hdr* header, const uint32_t size)
{
	switch (header->type)
	{
		case synthvid_msg_type::VersionResponse:
		{
			Assert(size >= sizeof(synthvid_msg_hdr) + sizeof(synthvid_version_response));
			memcpy(&m_versionResponse, (uint8_t*)header + sizeof(synthvid_msg_hdr), sizeof(synthvid_version_response));
			m_event.Set();
		}
		break;

		case synthvid_msg_type::VramLocationAck:
		case synthvid_msg_type::SituationUpdateAck:
			m_event.Set();
			break;

		//Notifications we don't currently act on.
		case synthvid_msg_type::FeatureChange:
		case synthvid_msg_type::PointerPosition:
		case synthvid_msg_type::PointerShape:
			break;

		default:
			break;
	}
}
