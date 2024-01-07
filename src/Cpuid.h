#pragma once
#include <string>
#include <bitset>

class Cpuid
{
public:
	Cpuid();

	bool IsIntel() const
	{
		return m_isIntel;
	}

	bool IsAMD() const
	{
		return m_isAmd;
	}

	const std::string& GetVendor() const
	{
		return m_vendor;
	}

	bool IsHypervisor()
	{
		return m_func1Ecx[31];
	}

	static void GetVendor(std::string& vendor);

private:
	enum Regs
	{
		EAX,
		EBX,
		ECX,
		EDX
	};

	typedef std::bitset<std::numeric_limits<uint32_t>::digits> bitset_32;

	std::string m_vendor;
	bool m_isIntel;
	bool m_isAmd;

	bitset_32 m_func1Ecx;
};

