#pragma once

#include <cstdint>
#include <memory>

//TODO: make sure everything gets inlined sinces its so simple
//Shouldnt this just be bitset?
class Bitvector
{
public:
	Bitvector(const size_t length);

	bool Get(const size_t index) const;
	void Set(const size_t index, const bool value);

	size_t const Length;

private:
	static constexpr size_t mapBits = std::numeric_limits<size_t>::digits;

	static constexpr size_t GetMapSize(size_t length);
	static constexpr size_t GetMapIndex(size_t index);
	static constexpr size_t GetMapShift(size_t index);

	std::unique_ptr<size_t> m_map;
};

