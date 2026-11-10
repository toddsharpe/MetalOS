#include "Lib/System.h"
#include "HyperVScsiDriver.h"
#include "kernel/Devices/KDevice.h"
#include "kernel/Arch.h"
#include "kernel/Thread.h"
#include "kernel/Pmem.h"
#include "kernel/Vmem.h"
#include "Lib/Math.h"
#include "Assert.h"

namespace
{
	static constexpr uint8_t Inquiry = 0x12;
	static constexpr uint8_t SynchronizeCache10 = 0x35;
	static constexpr uint8_t Read16 = 0x88;
	static constexpr uint8_t Write16 = 0x8A;
	static constexpr uint8_t ServiceActionIn16 = 0x9E;
	static constexpr uint8_t ReadCapacity16 = 0x10;

	static constexpr uint32_t InquiryLength = 36;
	static constexpr uint32_t CapacityLength = 32;

	//A direct-access block device, in the low bits of INQUIRY's first byte
	static constexpr uint8_t PeripheralDisk = 0x00;
}

HyperVScsiDriver::HyperVScsiDriver(KDevice& device) :
	StorageDriver(device),
	m_channel(RingSize, RingSize, { &HyperVScsiDriver::Callback, this }),
	m_lock(false, true),
	m_event(false, false),
	m_waiting(),
	m_pending(),
	m_nextId(1),
	m_response(),
	m_bouncePhysical(),
	m_bounce(),
	m_maxTransfer(),
	m_version(),
	m_sectorSize(),
	m_sectorCount(),
	m_ready()
{

}

bool HyperVScsiDriver::Initialize()
{
	//Open the VMBus channel (rings + GPADL happen here)
	const VmBusChild& child = VmBusChild::Of(m_device);
	m_channel.Initialize(*child.Bus, &child.Offer);

	//A failed negotiation closes the channel again rather than leaving it open with our callback registered
	if (!Negotiate())
	{
		m_channel.Close(*child.Bus);
		return false;
	}

	m_bouncePhysical = KePhysicalAlloc(BouncePages);
	m_bounce = static_cast<uint8_t*>(KeVirtualMap(m_bouncePhysical, BounceSize));
	Assert(m_bounce);

	//A controller with no disk at target 0, LUN 0 stays up with nothing to serve
	m_device.Class = KDeviceClass::Storage;
	m_ready = Probe();
	return true;
}

BlockDevice* HyperVScsiDriver::GetDisk(const size_t index)
{
	return index == 0 && m_ready ? this : nullptr;
}

size_t HyperVScsiDriver::SectorSize() const
{
	return m_sectorSize;
}

uint64_t HyperVScsiDriver::SectorCount() const
{
	return m_sectorCount;
}

bool HyperVScsiDriver::Read(const uint64_t sector, const size_t count, void* const buffer)
{
	if (!m_ready || count > m_sectorCount || sector > m_sectorCount - count)
		return false;

	KeWait(m_lock);
	const size_t chunk = m_maxTransfer / m_sectorSize;
	uint8_t* const out = static_cast<uint8_t*>(buffer);
	bool done = true;
	for (size_t at = 0; done && at < count;)
	{
		const uint32_t sectors = static_cast<uint32_t>(Min(chunk, count - at));
		done = Transfer(Read16, sector + at, sectors, Direction::In);
		if (done)
		{
			memcpy(out + at * m_sectorSize, m_bounce, sectors * m_sectorSize);
			at += sectors;
		}
	}
	m_lock.Set();
	return done;
}

bool HyperVScsiDriver::Write(const uint64_t sector, const size_t count, const void* const buffer)
{
	if (!m_ready || count > m_sectorCount || sector > m_sectorCount - count)
		return false;

	KeWait(m_lock);
	const size_t chunk = m_maxTransfer / m_sectorSize;
	const uint8_t* const in = static_cast<const uint8_t*>(buffer);
	bool done = true;
	for (size_t at = 0; done && at < count;)
	{
		const uint32_t sectors = static_cast<uint32_t>(Min(chunk, count - at));
		memcpy(m_bounce, in + at * m_sectorSize, sectors * m_sectorSize);
		done = Transfer(Write16, sector + at, sectors, Direction::Out);
		at += sectors;
	}
	m_lock.Set();
	return done;
}

bool HyperVScsiDriver::Flush()
{
	if (!m_ready)
		return false;

	KeWait(m_lock);
	const uint8_t cdb[10] = { SynchronizeCache10 };
	const bool done = Command(cdb, sizeof(cdb), Direction::None, 0);
	m_lock.Set();
	return done;
}

//Begin, a protocol version both sides speak, the channel's properties, end
bool HyperVScsiDriver::Negotiate()
{
	if (!Operation(vstor_operation::BeginInitialization))
	{
		Printf("HyperVScsi: no answer to BEGIN_INITIALIZATION\n");
		return false;
	}

	//Newest first; a version the host rejects completes with a nonzero status
	static constexpr uint16_t versions[] = { VersionWin10, VersionWin8_1, VersionWin8 };
	for (const uint16_t version : versions)
	{
		vstor_packet packet = {};
		packet.operation = vstor_operation::QueryProtocolVersion;
		packet.flags = REQUEST_COMPLETION_FLAG;
		packet.version.major_minor = version;
		if (!Execute(packet, 0))
		{
			Printf("HyperVScsi: no answer to QUERY_PROTOCOL_VERSION 0x%x\n", version);
			return false;
		}

		if (m_response.operation == vstor_operation::CompleteIo && m_response.status == 0)
		{
			m_version = version;
			break;
		}
	}
	if (m_version == 0)
	{
		Printf("HyperVScsi: host rejected protocol versions 6.2 to 5.1\n");
		return false;
	}

	vstor_packet properties = {};
	properties.operation = vstor_operation::QueryProperties;
	properties.flags = REQUEST_COMPLETION_FLAG;
	if (!Execute(properties, 0) || m_response.operation != vstor_operation::CompleteIo || m_response.status != 0)
	{
		Printf("HyperVScsi: no answer to QUERY_PROPERTIES\n");
		return false;
	}

	//The host's limit on one request, or the bounce buffer's where that is smaller or the host names none
	const uint32_t hostMax = m_response.properties.max_transfer_bytes;
	m_maxTransfer = (hostMax == 0 || hostMax > BounceSize) ? static_cast<uint32_t>(BounceSize) : hostMax;

	if (!Operation(vstor_operation::EndInitialization))
	{
		Printf("HyperVScsi: no answer to END_INITIALIZATION\n");
		return false;
	}
	return true;
}

//The disk at target 0, LUN 0: a direct-access device, its capacity, and a look at its partition table
bool HyperVScsiDriver::Probe()
{
	const uint8_t inquiry[6] = { Inquiry, 0, 0, 0, static_cast<uint8_t>(InquiryLength), 0 };
	if (!Command(inquiry, sizeof(inquiry), Direction::In, InquiryLength) || (m_bounce[0] & 0x1F) != PeripheralDisk)
	{
		Printf("HyperVScsi: no disk at target 0, LUN 0\n");
		return false;
	}

	//The last sector and the sector size, big-endian
	uint8_t capacity[16] = { ServiceActionIn16, ReadCapacity16 };
	capacity[13] = static_cast<uint8_t>(CapacityLength);
	if (!Command(capacity, sizeof(capacity), Direction::In, CapacityLength))
	{
		Printf("HyperVScsi: READ CAPACITY failed\n");
		return false;
	}

	uint64_t last = 0;
	for (size_t i = 0; i < 8; i++)
		last = (last << 8) | m_bounce[i];
	m_sectorCount = last + 1;
	m_sectorSize = (static_cast<uint32_t>(m_bounce[8]) << 24) | (static_cast<uint32_t>(m_bounce[9]) << 16) | (static_cast<uint32_t>(m_bounce[10]) << 8) | m_bounce[11];
	if (m_sectorSize == 0 || BounceSize % m_sectorSize != 0 || m_maxTransfer < m_sectorSize)
	{
		Printf("HyperVScsi: sector size %u doesn't fit transfers of %u bytes\n", m_sectorSize, m_maxTransfer);
		return false;
	}

	//The boot disk is GPT, whose header in sector 1 starts with its signature
	const bool gpt = Transfer(Read16, 1, 1, Direction::In) && memcmp(m_bounce, "EFI PART", 8) == 0;
	Printf("HyperVScsi: protocol %u.%u, disk 0: %u MB in %u-byte sectors%s\n",
		m_version >> 8, m_version & 0xFF, static_cast<uint32_t>((m_sectorCount * m_sectorSize) >> 20), m_sectorSize, gpt ? ", GPT" : "");
	return true;
}

bool HyperVScsiDriver::Execute(const vstor_packet& packet, const uint32_t dmaBytes)
{
	cpu_flags_t flags = ArchDisableInterrupts();
	m_event.Reset();
	m_pending = m_nextId++;
	m_response = {};
	m_waiting = true;
	ArchRestoreFlags(flags);

	const bool sent = dmaBytes == 0 ?
		m_channel.SendPacket(&packet, sizeof(packet), m_pending, HyperV::VM_PKT_DATA_INBAND, VMBUS_DATA_PACKET_FLAG_COMPLETION_REQUESTED) :
		m_channel.SendPacketGpaDirect(&packet, sizeof(packet), m_pending, m_bouncePhysical, dmaBytes);
	const bool answered = sent && KeWait(m_event, Timeout) == KWaitResult::Signaled;

	flags = ArchDisableInterrupts();
	m_waiting = false;
	ArchRestoreFlags(flags);
	return answered;
}

bool HyperVScsiDriver::Operation(const vstor_operation operation)
{
	vstor_packet packet = {};
	packet.operation = operation;
	packet.flags = REQUEST_COMPLETION_FLAG;
	return Execute(packet, 0) && m_response.operation == vstor_operation::CompleteIo && m_response.status == 0;
}

bool HyperVScsiDriver::Command(const uint8_t* const cdb, const uint8_t cdbLength, const Direction direction, const uint32_t bytes)
{
	Assert(cdbLength <= MaxCdb && bytes <= BounceSize);

	//A disk reports a unit attention once after it changes or first comes up, so the command goes again
	for (size_t attempt = 0; attempt < 3; attempt++)
	{
		vstor_packet packet = {};
		packet.operation = vstor_operation::ExecuteSrb;
		packet.flags = REQUEST_COMPLETION_FLAG;

		vmscsi_request& srb = packet.srb;
		srb.length = sizeof(vmscsi_request);
		srb.cdb_length = cdbLength;
		srb.sense_info_length = static_cast<uint8_t>(SenseSize);
		srb.data_in = static_cast<uint8_t>(direction);
		srb.data_transfer_length = bytes;
		memcpy(srb.cdb, cdb, cdbLength);
		srb.win8_extension.srb_flags = SRB_FLAGS_DISABLE_SYNCH_TRANSFER |
			(direction == Direction::In ? SRB_FLAGS_DATA_IN : direction == Direction::Out ? SRB_FLAGS_DATA_OUT : 0);
		srb.win8_extension.time_out_value = 60;

		if (!Execute(packet, bytes))
			return false;

		const vmscsi_request& done = m_response.srb;
		if (m_response.status == 0 && (done.srb_status & SRB_STATUS_MASK) == SRB_STATUS_SUCCESS && done.scsi_status == SCSI_STATUS_GOOD)
			return true;

		const bool unitAttention = done.scsi_status == SCSI_STATUS_CHECK_CONDITION && (done.srb_status & SRB_STATUS_AUTOSENSE_VALID) != 0 &&
			(done.sense_data[2] & 0x0F) == SENSE_KEY_UNIT_ATTENTION;
		if (!unitAttention)
			return false;
	}
	return false;
}

//READ(16) or WRITE(16): count sectors at sector, big-endian, through the bounce buffer
bool HyperVScsiDriver::Transfer(const uint8_t opcode, const uint64_t sector, const uint32_t count, const Direction direction)
{
	uint8_t cdb[16] = { opcode };
	for (size_t i = 0; i < 8; i++)
		cdb[2 + i] = static_cast<uint8_t>(sector >> (56 - 8 * i));
	for (size_t i = 0; i < 4; i++)
		cdb[10 + i] = static_cast<uint8_t>(count >> (24 - 8 * i));

	const uint32_t bytes = count * m_sectorSize;
	return Command(cdb, sizeof(cdb), direction, bytes) && m_response.srb.data_transfer_length == bytes;
}

void HyperVScsiDriver::OnCallback()
{
	m_channel.Drain([this](const HyperV::vmpacket_descriptor& packet, uint8_t* const data, const uint32_t size)
	{
		//Only the completion a request is waiting for counts; the host's bus notifications and answers that came too late are dropped
		if (packet.type != HyperV::VM_PKT_COMP || !m_waiting || packet.trans_id != m_pending)
			return;

		memcpy(&m_response, data, Min<size_t>(size, sizeof(m_response)));
		m_waiting = false;
		m_event.Set();
	});
}
