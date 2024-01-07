#pragma once

#include <cstdint>
#include <memory>

class Bitvector
{
public:
	Bitvector(const size_t length);

	void Initialize();
	bool Get(const size_t index) const;
	void Set(const size_t index, const bool value);

	size_t const Length;

private:
	size_t* m_map;
};
