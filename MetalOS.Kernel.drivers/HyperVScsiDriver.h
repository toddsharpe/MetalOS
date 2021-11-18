#pragma once

#include "Device.h"
#include "Driver.h"
#include "HyperVChannel.h"
#include "MetalOS.h"

/* SCSI Definitions*/
#define SCAN_WILD_CARD	~0

/*
 *      SCSI opcodes
 */

#define TEST_UNIT_READY       0x00
#define REZERO_UNIT           0x01
#define REQUEST_SENSE         0x03
#define FORMAT_UNIT           0x04
#define READ_BLOCK_LIMITS     0x05
#define REASSIGN_BLOCKS       0x07
#define READ_6                0x08
#define WRITE_6               0x0a
#define SEEK_6                0x0b
#define READ_REVERSE          0x0f
#define WRITE_FILEMARKS       0x10
#define SPACE                 0x11
#define INQUIRY               0x12
#define RECOVER_BUFFERED_DATA 0x14
#define MODE_SELECT           0x15
#define RESERVE               0x16
#define RELEASE               0x17
#define COPY                  0x18
#define ERASE                 0x19
#define MODE_SENSE            0x1a
#define START_STOP            0x1b
#define RECEIVE_DIAGNOSTIC    0x1c
#define SEND_DIAGNOSTIC       0x1d
#define ALLOW_MEDIUM_REMOVAL  0x1e

#define SET_WINDOW            0x24
#define READ_CAPACITY         0x25
#define READ_10               0x28
#define WRITE_10              0x2a
#define SEEK_10               0x2b
#define WRITE_VERIFY          0x2e
#define VERIFY                0x2f
#define SEARCH_HIGH           0x30
#define SEARCH_EQUAL          0x31
#define SEARCH_LOW            0x32
#define SET_LIMITS            0x33
#define PRE_FETCH             0x34
#define READ_POSITION         0x34
#define SYNCHRONIZE_CACHE     0x35
#define LOCK_UNLOCK_CACHE     0x36
#define READ_DEFECT_DATA      0x37
#define MEDIUM_SCAN           0x38
#define COMPARE               0x39
#define COPY_VERIFY           0x3a
#define WRITE_BUFFER          0x3b
#define READ_BUFFER           0x3c
#define UPDATE_BLOCK          0x3d
#define READ_LONG             0x3e
#define WRITE_LONG            0x3f
#define CHANGE_DEFINITION     0x40
#define WRITE_SAME            0x41
#define READ_TOC              0x43
#define LOG_SELECT            0x4c
#define LOG_SENSE             0x4d
#define MODE_SELECT_10        0x55
#define RESERVE_10            0x56
#define RELEASE_10            0x57
#define MODE_SENSE_10         0x5a
#define PERSISTENT_RESERVE_IN 0x5e
#define PERSISTENT_RESERVE_OUT 0x5f
#define MOVE_MEDIUM           0xa5
#define READ_12               0xa8
#define WRITE_12              0xaa
#define WRITE_VERIFY_12       0xae
#define SEARCH_HIGH_12        0xb0
#define SEARCH_EQUAL_12       0xb1
#define SEARCH_LOW_12         0xb2
#define READ_ELEMENT_STATUS   0xb8
#define SEND_VOLUME_TAG       0xb6
#define WRITE_LONG_2          0xea

 /*
  *  Status codes
  */

#define GOOD                 0x00
#define CHECK_CONDITION      0x01
#define CONDITION_GOOD       0x02
#define BUSY                 0x04
#define INTERMEDIATE_GOOD    0x08
#define INTERMEDIATE_C_GOOD  0x0a
#define RESERVATION_CONFLICT 0x0c
#define COMMAND_TERMINATED   0x11
#define QUEUE_FULL           0x14

#define STATUS_MASK          0x3e

  /*
   *  SENSE KEYS
   */

#define NO_SENSE            0x00
#define RECOVERED_ERROR     0x01
#define NOT_READY           0x02
#define MEDIUM_ERROR        0x03
#define HARDWARE_ERROR      0x04
#define ILLEGAL_REQUEST     0x05
#define UNIT_ATTENTION      0x06
#define DATA_PROTECT        0x07
#define BLANK_CHECK         0x08
#define COPY_ABORTED        0x0a
#define ABORTED_COMMAND     0x0b
#define VOLUME_OVERFLOW     0x0d
#define MISCOMPARE          0x0e


/*
 * All wire protocol details (storage protocol between the guest and the host)
 * are consolidated here.
 *
 * Begin protocol definitions.
 */

 /*
  * Version history:
  * V1 Beta: 0.1
  * V1 RC < 2008/1/31: 1.0
  * V1 RC > 2008/1/31:  2.0
  * Win7: 4.2
  * Win8: 5.1
  * Win8.1: 6.0
  * Win10: 6.2
  */

#pragma pack(push, 1)

#define VMSTOR_PROTO_VERSION(MAJOR_, MINOR_)	((((MAJOR_) & 0xff) << 8) | \
						(((MINOR_) & 0xff)))

#define VMSTOR_PROTO_VERSION_WIN6	VMSTOR_PROTO_VERSION(2, 0)
#define VMSTOR_PROTO_VERSION_WIN7	VMSTOR_PROTO_VERSION(4, 2)
#define VMSTOR_PROTO_VERSION_WIN8	VMSTOR_PROTO_VERSION(5, 1)
#define VMSTOR_PROTO_VERSION_WIN8_1	VMSTOR_PROTO_VERSION(6, 0)
#define VMSTOR_PROTO_VERSION_WIN10	VMSTOR_PROTO_VERSION(6, 2)

#define U64_MAX UINT64_MAX

  /* StorVSC-specific */
#define VMBUS_RQST_INIT (U64_MAX - 2)
#define VMBUS_RQST_RESET (U64_MAX - 3)

  /*  Packet structure describing virtual storage requests. */
enum vstor_packet_operation {
	VSTOR_OPERATION_COMPLETE_IO = 1,
	VSTOR_OPERATION_REMOVE_DEVICE = 2,
	VSTOR_OPERATION_EXECUTE_SRB = 3,
	VSTOR_OPERATION_RESET_LUN = 4,
	VSTOR_OPERATION_RESET_ADAPTER = 5,
	VSTOR_OPERATION_RESET_BUS = 6,
	VSTOR_OPERATION_BEGIN_INITIALIZATION = 7,
	VSTOR_OPERATION_END_INITIALIZATION = 8,
	VSTOR_OPERATION_QUERY_PROTOCOL_VERSION = 9,
	VSTOR_OPERATION_QUERY_PROPERTIES = 10,
	VSTOR_OPERATION_ENUMERATE_BUS = 11,
	VSTOR_OPERATION_FCHBA_DATA = 12,
	VSTOR_OPERATION_CREATE_SUB_CHANNELS = 13,
	VSTOR_OPERATION_MAXIMUM = 13
};

/*
 * WWN packet for Fibre Channel HBA
 */

struct hv_fc_wwn_packet {
	u8	primary_active;
	u8	reserved1[3];
	u8	primary_port_wwn[8];
	u8	primary_node_wwn[8];
	u8	secondary_port_wwn[8];
	u8	secondary_node_wwn[8];
};


#define STORVSC_MAX_LUNS_PER_TARGET			255
#define STORVSC_MAX_TARGETS				2
#define STORVSC_MAX_CHANNELS				8

/*
 * SRB Flag Bits
 */

#define SRB_FLAGS_QUEUE_ACTION_ENABLE		0x00000002
#define SRB_FLAGS_DISABLE_DISCONNECT		0x00000004
#define SRB_FLAGS_DISABLE_SYNCH_TRANSFER	0x00000008
#define SRB_FLAGS_BYPASS_FROZEN_QUEUE		0x00000010
#define SRB_FLAGS_DISABLE_AUTOSENSE		0x00000020
#define SRB_FLAGS_DATA_IN			0x00000040
#define SRB_FLAGS_DATA_OUT			0x00000080
#define SRB_FLAGS_NO_DATA_TRANSFER		0x00000000
#define SRB_FLAGS_UNSPECIFIED_DIRECTION	(SRB_FLAGS_DATA_IN | SRB_FLAGS_DATA_OUT)
#define SRB_FLAGS_NO_QUEUE_FREEZE		0x00000100
#define SRB_FLAGS_ADAPTER_CACHE_ENABLE		0x00000200
#define SRB_FLAGS_FREE_SENSE_BUFFER		0x00000400

 /*
  * This flag indicates the request is part of the workflow for processing a D3.
  */
#define SRB_FLAGS_D3_PROCESSING			0x00000800
#define SRB_FLAGS_IS_ACTIVE			0x00010000
#define SRB_FLAGS_ALLOCATED_FROM_ZONE		0x00020000
#define SRB_FLAGS_SGLIST_FROM_POOL		0x00040000
#define SRB_FLAGS_BYPASS_LOCKED_QUEUE		0x00080000
#define SRB_FLAGS_NO_KEEP_AWAKE			0x00100000
#define SRB_FLAGS_PORT_DRIVER_ALLOCSENSE	0x00200000
#define SRB_FLAGS_PORT_DRIVER_SENSEHASPORT	0x00400000
#define SRB_FLAGS_DONT_START_NEXT_PACKET	0x00800000
#define SRB_FLAGS_PORT_DRIVER_RESERVED		0x0F000000
#define SRB_FLAGS_CLASS_DRIVER_RESERVED		0xF0000000

#define SP_UNTAGGED			((unsigned char) ~0)
#define SRB_SIMPLE_TAG_REQUEST		0x20

  /*
   * Platform neutral description of a scsi request -
   * this remains the same across the write regardless of 32/64 bit
   * note: it's patterned off the SCSI_PASS_THROUGH structure
   */
#define STORVSC_MAX_CMD_LEN			0x10

#define POST_WIN7_STORVSC_SENSE_BUFFER_SIZE	0x14
#define PRE_WIN8_STORVSC_SENSE_BUFFER_SIZE	0x12

#define STORVSC_SENSE_BUFFER_SIZE		0x14
#define STORVSC_MAX_BUF_LEN_WITH_PADDING	0x14

   /*
	* Sense buffer size changed in win8; have a run-time
	* variable to track the size we should use.  This value will
	* likely change during protocol negotiation but it is valid
	* to start by assuming pre-Win8.
	*/
static int sense_buffer_size = PRE_WIN8_STORVSC_SENSE_BUFFER_SIZE;

/*
 * The storage protocol version is determined during the
 * initial exchange with the host.  It will indicate which
 * storage functionality is available in the host.
*/
static int vmstor_proto_version;

#define STORVSC_LOGGING_NONE	0
#define STORVSC_LOGGING_ERROR	1
#define STORVSC_LOGGING_WARN	2

#define storvsc_log(dev, level, fmt, ...)			\
do {								\
	if (do_logging(level))					\
		dev_warn(&(dev)->device, fmt, ##__VA_ARGS__);	\
} while (0)

struct vmscsi_win8_extension {
	/*
	 * The following were added in Windows 8
	 */
	u16 reserve;
	u8  queue_tag;
	u8  queue_action;
	u32 srb_flags;
	u32 time_out_value;
	u32 queue_sort_ey;
} __packed;

struct vmscsi_request {
	u16 length;
	u8 srb_status;
	u8 scsi_status;

	u8  port_number;
	u8  path_id;
	u8  target_id;
	u8  lun;

	u8  cdb_length;
	u8  sense_info_length;
	u8  data_in;
	u8  reserved;

	u32 data_transfer_length;

	union {
		u8 cdb[STORVSC_MAX_CMD_LEN];
		u8 sense_data[STORVSC_SENSE_BUFFER_SIZE];
		u8 reserved_array[STORVSC_MAX_BUF_LEN_WITH_PADDING];
	};
	/*
	 * The following was added in win8.
	 */
	struct vmscsi_win8_extension win8_extension;
};


/*
 * The size of the vmscsi_request has changed in win8. The
 * additional size is because of new elements added to the
 * structure. These elements are valid only when we are talking
 * to a win8 host.
 * Track the correction to size we need to apply. This value
 * will likely change during protocol negotiation but it is
 * valid to start by assuming pre-Win8.
 */
static int vmscsi_size_delta = sizeof(struct vmscsi_win8_extension);

/*
 * The list of storage protocols in order of preference.
 */
struct vmstor_protocol {
	int protocol_version;
	int sense_buffer_size;
	int vmscsi_size_delta;
};


static const struct vmstor_protocol vmstor_protocols[] = {
	{
		VMSTOR_PROTO_VERSION_WIN10,
		POST_WIN7_STORVSC_SENSE_BUFFER_SIZE,
		0
	},
	{
		VMSTOR_PROTO_VERSION_WIN8_1,
		POST_WIN7_STORVSC_SENSE_BUFFER_SIZE,
		0
	},
	{
		VMSTOR_PROTO_VERSION_WIN8,
		POST_WIN7_STORVSC_SENSE_BUFFER_SIZE,
		0
	},
	{
		VMSTOR_PROTO_VERSION_WIN7,
		PRE_WIN8_STORVSC_SENSE_BUFFER_SIZE,
		sizeof(struct vmscsi_win8_extension),
	},
	{
		VMSTOR_PROTO_VERSION_WIN6,
		PRE_WIN8_STORVSC_SENSE_BUFFER_SIZE,
		sizeof(struct vmscsi_win8_extension),
	}
};


/*
 * This structure is sent during the initialization phase to get the different
 * properties of the channel.
 */

#define STORAGE_CHANNEL_SUPPORTS_MULTI_CHANNEL		0x1

struct vmstorage_channel_properties {
	u32 reserved;
	u16 max_channel_cnt;
	u16 reserved1;

	u32 flags;
	u32   max_transfer_bytes;

	u64  reserved2;
} __packed;

/*  This structure is sent during the storage protocol negotiations. */
struct vmstorage_protocol_version {
	/* Major (MSW) and minor (LSW) version numbers. */
	u16 major_minor;

	/*
	 * Revision number is auto-incremented whenever this file is changed
	 * (See FILL_VMSTOR_REVISION macro above).  Mismatch does not
	 * definitely indicate incompatibility--but it does indicate mismatched
	 * builds.
	 * This is only used on the windows side. Just set it to 0.
	 */
	u16 revision;
} __packed;

/* Channel Property Flags */
#define STORAGE_CHANNEL_REMOVABLE_FLAG		0x1
#define STORAGE_CHANNEL_EMULATED_IDE_FLAG	0x2

struct vstor_packet {
	/* Requested operation type */
	enum vstor_packet_operation operation;

	/*  Flags - see below for values */
	u32 flags;

	/* Status of the request returned from the server side. */
	u32 status;

	/* Data payload area */
	union {
		/*
		 * Structure used to forward SCSI commands from the
		 * client to the server.
		 */
		struct vmscsi_request vm_srb; //SCSI_REQUEST_BLOCK

		/* Structure used to query channel properties. */
		struct vmstorage_channel_properties storage_channel_properties;

		/* Used during version negotiations. */
		struct vmstorage_protocol_version version;

		/* Fibre channel address packet */
		struct hv_fc_wwn_packet wwn_packet;

		/* Number of sub-channels to create */
		u16 sub_channel_count;

		/* This will be the maximum of the union members */
		u8  buffer[0x34];
	};
} __packed;

/*
 * Packet Flags:
 *
 * This flag indicates that the server should send back a completion for this
 * packet.
 */

#define REQUEST_COMPLETION_FLAG	0x1

 /* Matches Windows-end */
enum storvsc_request_type {
	WRITE_TYPE = 0,
	READ_TYPE,
	UNKNOWN_TYPE,
};

/*
 * SRB status codes and masks; a subset of the codes used here.
 */

#define SRB_STATUS_AUTOSENSE_VALID	0x80
#define SRB_STATUS_QUEUE_FROZEN		0x40
#define SRB_STATUS_INVALID_LUN	0x20
#define SRB_STATUS_SUCCESS	0x01
#define SRB_STATUS_ABORTED	0x02
#define SRB_STATUS_ERROR	0x04
#define SRB_STATUS_DATA_OVERRUN	0x12

#define SRB_STATUS(status) \
	(status & ~(SRB_STATUS_AUTOSENSE_VALID | SRB_STATUS_QUEUE_FROZEN))
 /*
  * This is the end of Protocol specific defines.
  */


#define SCSI_VSC_SEND_RING_BUFFER_SIZE		(40 * 1024)
#define SCSI_VSC_RECV_RING_BUFFER_SIZE		(40 * 1024)

#pragma pack(pop)

struct Transaction
{
	vstor_packet Request;
	vstor_packet Response;
	Handle Semaphore;
};

class HyperVScsiDriver : public Driver
{
public:
	HyperVScsiDriver(Device& device);

	Result Initialize() override;
	Result Read(char* buffer, size_t length, size_t* bytesRead = nullptr) override;
	Result Write(const char* buffer, size_t length) override;
	Result EnumerateChildren() override;

	static void Callback(void* context) { ((HyperVScsiDriver*)context)->OnCallback(); };

private:
	uint32_t m_sizeDelta;

	void OnCallback();
	void Execute(Transaction* transaction, bool status_check);

	Handle m_semaphore;
	HyperVChannel m_channel;
};

