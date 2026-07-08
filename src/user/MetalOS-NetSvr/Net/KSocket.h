#pragma once

#include "Net/Net.h"
#include "Net/PacketBuffer.h"
#include "Lib/Buffer.h"
#include "Assert.h"

//Datagram socket. Sends reuse the Net::Socket send path; receives are demultiplexed by
//Net::Socket::Receive into a per-socket queue that the netstack IPC layer drains to
//service app RecvFrom requests. netstack is single-threaded (one RX/IPC loop), so no
//lock is needed around the queue.
class KSocket
{
public:
	//Each queued record is an endpoint_t (source) followed by the payload bytes.
	static constexpr size_t MaxDatagram = Net::buffer_size;
	static constexpr size_t QueueBytes = 8192; //power of two (RingBuffer requirement)

	KSocket(const Net::proto_t proto) :
		Proto(proto),
		Local(),
		Peer(),
		Connected(false),
		Closed(false),
		Registered(false),
		m_queue(),
		m_scratch()
	{
	}

	//Push a received datagram. Drops if oversized or full.
	bool Enqueue(const Net::endpoint_t& src, const void* const data, const size_t length)
	{
		if (length > MaxDatagram)
			return false;

		memcpy(m_scratch, &src, sizeof(Net::endpoint_t));
		memcpy(m_scratch + sizeof(Net::endpoint_t), data, length);
		return m_queue.Push(m_scratch, sizeof(Net::endpoint_t) + length);
	}

	//Pop one datagram into the caller buffer (thread context). Returns false if empty.
	bool Dequeue(Net::endpoint_t& src, void* const buffer, const size_t maxLength, size_t& bytesRead)
	{
		if (m_queue.IsEmpty())
			return false;

		Buffer out(m_scratch, sizeof(m_scratch));
		size_t recordLength = 0;
		if (!m_queue.Pop(out, recordLength))
			return false;

		Assert(recordLength >= sizeof(Net::endpoint_t));
		memcpy(&src, m_scratch, sizeof(Net::endpoint_t));

		const size_t payload = recordLength - sizeof(Net::endpoint_t);
		const size_t copy = payload < maxLength ? payload : maxLength;
		memcpy(buffer, m_scratch + sizeof(Net::endpoint_t), copy);
		bytesRead = copy;
		return true;
	}

	void MarkClosed()
	{
		Closed = true;
	}

	//KPredicate hook: signalled when a datagram is available or the socket is closed.
	static bool RecvReady(void* const arg)
	{
		const KSocket* const socket = static_cast<const KSocket*>(arg);
		return socket->Closed || !socket->m_queue.IsEmpty();
	}

	Net::proto_t Proto;
	Net::endpoint_t Local;  //bound local addr+port (port demux key)
	Net::endpoint_t Peer;   //connected default peer
	bool Connected;
	bool Closed;
	bool Registered;        //present in the demux registry

private:
	Net::PacketBuffer<QueueBytes> m_queue;
	uint8_t m_scratch[sizeof(Net::endpoint_t) + MaxDatagram];
};
