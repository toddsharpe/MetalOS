#pragma once
#include <cstdint>
#include <memory>

//TODO: make sure everything gets inlined sinces its so simple
class Bitvector
{
public:
	Bitvector(size_t length);
	void Set(size_t index);
	bool Get(size_t index) const;
	void Clear(size_t index);

	size_t Size() { return m_size; };

private:
	static const uint32_t mapBits = std::numeric_limits<size_t>::digits;
	static constexpr size_t GetMapSize(size_t length);
	static constexpr size_t GetMapIndex(size_t index);
	static constexpr size_t GetMapShift(size_t index);

	size_t m_size;
	std::unique_ptr<size_t[]> m_map;
};

