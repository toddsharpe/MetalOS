#pragma once

class HyperVChannel;
class HyperVRingBuffer
{
public:
	HyperVRingBuffer(const uint32_t size, HyperVChannel& channel);

	void Initialize(const paddr_t address);

	void Write(const CBuffer* buffers, const size_t count);
	void Increment(const uint32_t length);
	void* Read(const uint32_t length);
	bool CommitRead();

	void Display();

	const size_t Size;
	const size_t DataSize;

private:
#define VMBUS_PKT_TRAILER	8
	uint32_t Copy(const uint32_t location, const void* buffer, const uint32_t length);

	struct hv_ring_buffer
	{
		/* Offset in bytes from the start of ring data below */
		uint32_t write_index;

		/* Offset in bytes from the start of ring data below */
		uint32_t read_index;

		uint32_t interrupt_mask;

		/*
		* WS2012/Win8 and later versions of Hyper-V implement interrupt
		* driven flow management. The feature bit feat_pending_send_sz
		* is set by the host on the host->guest ring buffer, and by the
		* guest on the guest->host ring buffer.
		*
		* The meaning of the feature bit is a bit complex in that it has
		* semantics that apply to both ring buffers.  If the guest sets
		* the feature bit in the guest->host ring buffer, the guest is
		* telling the host that:
		* 1) It will set the pending_send_sz field in the guest->host ring
		*    buffer when it is waiting for space to become available, and
		* 2) It will read the pending_send_sz field in the host->guest
		*    ring buffer and interrupt the host when it frees enough space
		*
		* Similarly, if the host sets the feature bit in the host->guest
		* ring buffer, the host is telling the guest that:
		* 1) It will set the pending_send_sz field in the host->guest ring
		*    buffer when it is waiting for space to become available, and
		* 2) It will read the pending_send_sz field in the guest->host
		*    ring buffer and interrupt the guest when it frees enough space
		*
		* If either the guest or host does not set the feature bit that it
		* owns, that guest or host must do polling if it encounters a full
		* ring buffer, and not signal the other end with an interrupt.
		*/
		uint32_t pending_send_sz;
		uint32_t reserved1[12];
		union
		{
			struct
			{
				uint32_t feat_pending_send_sz : 1;
			};
			uint32_t value;
		} feature_bits;

		/* Pad it to PAGE_SIZE so that data starts on page boundary */
		uint8_t	reserved2[4028];

		/*
		* Ring data starts here + RingDataStartOffset
		* !!! DO NOT place any fields below this !!!
		*/
		uint8_t buffer[0];
	};
	static_assert(sizeof(hv_ring_buffer) == Arch::PageSize, "Invalid hv_ring_buffer");

	//Total size
	
	HyperVChannel& m_channel;

	//Header
	volatile hv_ring_buffer* m_header;

	//Dont write the header read index until completely done reading (since HV may then start overwriting it) so this is temp
	uint32_t m_iterator;
};

