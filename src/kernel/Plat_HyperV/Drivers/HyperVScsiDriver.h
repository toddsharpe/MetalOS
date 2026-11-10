#pragma once

#include "kernel/Devices/Driver.h"
#include "kernel/Plat_HyperV/Drivers/HyperVChannel.h"
#include "kernel/Objects/KEvent.h"

//Hyper-V synthetic SCSI controller (storvsc) {BA6163D9-04A1-4D29-B605-72E2FFB1DC7F}: the boot disk at target 0, LUN 0 as a block device
class HyperVScsiDriver : public StorageDriver, public BlockDevice
{
public:
	HyperVScsiDriver(KDevice& device);

	bool Initialize() override;

	//StorageDriver: the one disk this driver serves
	BlockDevice* GetDisk(const size_t index) override;

	size_t SectorSize() const override;
	uint64_t SectorCount() const override;
	bool Read(const uint64_t sector, const size_t count, void* const buffer) override;
	bool Write(const uint64_t sector, const size_t count, const void* const buffer) override;
	bool Flush() override;

	static void Callback(void* context) { static_cast<HyperVScsiDriver*>(context)->OnCallback(); }

private:
	static constexpr size_t RingSize = 32 * 1024;

	//Transfers go through one physically contiguous buffer, the most the host takes in one request
	static constexpr size_t BouncePages = 16;
	static constexpr size_t BounceSize = BouncePages * Arch::PageSize;
	static_assert(BouncePages <= HyperVChannel::GpaDirectMaxPages);

	static constexpr milli_t Timeout = 5000;

	//Protocol versions are major << 8 | minor; Win8's 5.1 is the first with the SRB's Win8 extension
	static constexpr uint16_t VersionWin10 = 0x0602;
	static constexpr uint16_t VersionWin8_1 = 0x0600;
	static constexpr uint16_t VersionWin8 = 0x0501;

	static constexpr size_t MaxCdb = 16;
	static constexpr size_t SenseSize = 20;

	enum class vstor_operation : uint32_t
	{
		CompleteIo = 1,
		RemoveDevice = 2,
		ExecuteSrb = 3,
		ResetLun = 4,
		ResetAdapter = 5,
		ResetBus = 6,
		BeginInitialization = 7,
		EndInitialization = 8,
		QueryProtocolVersion = 9,
		QueryProperties = 10,
		EnumerateBus = 11,
	};

	static constexpr uint32_t REQUEST_COMPLETION_FLAG = 0x1;

	static constexpr uint32_t SRB_FLAGS_DISABLE_SYNCH_TRANSFER = 0x00000008;
	static constexpr uint32_t SRB_FLAGS_DATA_IN = 0x00000040;
	static constexpr uint32_t SRB_FLAGS_DATA_OUT = 0x00000080;

	static constexpr uint8_t SRB_STATUS_SUCCESS = 0x01;
	static constexpr uint8_t SRB_STATUS_AUTOSENSE_VALID = 0x80;
	static constexpr uint8_t SRB_STATUS_MASK = 0x3F;

	//The SAM status byte and the fixed-format sense key that says the device changed under us
	static constexpr uint8_t SCSI_STATUS_GOOD = 0x00;
	static constexpr uint8_t SCSI_STATUS_CHECK_CONDITION = 0x02;
	static constexpr uint8_t SENSE_KEY_UNIT_ATTENTION = 0x06;

	enum class Direction : uint8_t
	{
		Out = 0,
		In = 1,
		None = 2,
	};

#pragma pack(push, 1)
	struct vmscsi_win8_extension
	{
		uint16_t reserve;
		uint8_t queue_tag;
		uint8_t queue_action;
		uint32_t srb_flags;
		uint32_t time_out_value;
		uint32_t queue_sort_key;
	};

	struct vmscsi_request
	{
		uint16_t length;
		uint8_t srb_status;
		uint8_t scsi_status;
		uint8_t port_number;
		uint8_t path_id;
		uint8_t target_id;
		uint8_t lun;
		uint8_t cdb_length;
		uint8_t sense_info_length;
		uint8_t data_in;
		uint8_t reserved;
		uint32_t data_transfer_length;
		union
		{
			uint8_t cdb[MaxCdb];
			uint8_t sense_data[SenseSize];
		};
		vmscsi_win8_extension win8_extension;
	};
	static_assert(sizeof(vmscsi_request) == 52);

	struct vmstorage_channel_properties
	{
		uint32_t reserved;
		uint16_t max_channel_cnt;
		uint16_t reserved1;
		uint32_t flags;
		uint32_t max_transfer_bytes;
		uint64_t reserved2;
	};

	struct vmstorage_protocol_version
	{
		uint16_t major_minor;
		uint16_t revision;
	};

	struct vstor_packet
	{
		vstor_operation operation;
		uint32_t flags;
		uint32_t status;
		union
		{
			vmscsi_request srb;
			vmstorage_channel_properties properties;
			vmstorage_protocol_version version;
		};
	};
	static_assert(sizeof(vstor_packet) == 64);
#pragma pack(pop)

	bool Negotiate();
	bool Probe();

	//Sends packet and waits for its completion in m_response, with dmaBytes of the bounce buffer as its data; false on no answer
	bool Execute(const vstor_packet& packet, const uint32_t dmaBytes);
	bool Operation(const vstor_operation operation);

	//One SCSI command to the disk, its data in the bounce buffer; false unless it succeeds
	bool Command(const uint8_t* const cdb, const uint8_t cdbLength, const Direction direction, const uint32_t bytes);
	bool Transfer(const uint8_t opcode, const uint64_t sector, const uint32_t count, const Direction direction);

	void OnCallback();

	HyperVChannel m_channel;

	//Serializes the disk: an auto-reset event that starts signaled is a lock a waiting thread sleeps on
	KEvent m_lock;
	KEvent m_event;
	bool m_waiting;
	uint64_t m_pending;
	uint64_t m_nextId;
	vstor_packet m_response;

	paddr_t m_bouncePhysical;
	uint8_t* m_bounce;
	uint32_t m_maxTransfer;

	uint16_t m_version;
	uint32_t m_sectorSize;
	uint64_t m_sectorCount;
	bool m_ready;
};
