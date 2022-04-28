#pragma once
#include <cstdint>
#include <memory>

//TODO: make sure everything gets inlined sinces its so simple
//Shouldnt this just be bitset?
class Bitvector
{
public:
	Bitvector(size_t length);

	bool Get(size_t index) const;
	void Set(size_t index, bool value);

	const size_t Size() const { return m_size; };

private:
	static const uint32_t mapBits = std::numeric_limits<size_t>::digits;
	static constexpr size_t GetMapSize(size_t length);
	static constexpr size_t GetMapIndex(size_t index);
	static constexpr size_t GetMapShift(size_t index);

	size_t m_size;
	size_t* m_map;
};

