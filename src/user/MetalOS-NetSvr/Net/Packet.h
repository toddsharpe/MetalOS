#pragma once

#include "Lib/Span.h"
#include "Net/Net.h"
#include "Assert.h"
#include <cstdint>
#include <cstddef>
#include <cstring>

namespace Net
{
	struct Packet
	{
		constexpr Packet() : backing(), src(), dst(), proto(), taken(), offset(), count()
		{

		}

		Packet(const buffer_t _backing) : backing(_backing), src(), dst(), proto(), taken(), offset(), count(_backing.Size)
		{

		}

		Packet(const buffer_t _backing, const size_t _count) : backing(_backing), src(), dst(), proto(), taken(), offset(), count(_count)
		{

		}

		uint8_t* buffer()
		{
			return &backing.Data[offset];
		}

		const uint8_t* buffer() const
		{
			return &backing.Data[offset];
		}

		size_t headroom() const
		{
			return offset;
		}

		size_t length() const
		{
			return count;
		}

		void trim(const size_t size)
		{
			count = size;
		}

		void mask(const size_t size)
		{
			AssertOp(count, >, size);
			offset += size;
			count -= size;
		}

		void unmask(const size_t size)
		{
			Assert(offset >= size);
			offset -= size;
			count += size;
		}

		buffer_t backing;

		//Layer specific data
		endpoint_t src;
		endpoint_t dst;
		proto_t proto;

		//Coming in, a socket that keeps the rx-pool backing says so here, and the driver frees what nobody took
		bool taken;

	private:
		//Packet data
		size_t offset;
		size_t count;
	};
}
