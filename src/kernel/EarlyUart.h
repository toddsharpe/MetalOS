#pragma once

#include "StringPrinter.h"

enum class ComPort : uint16_t
{
	Com1 = 0x3F8,
	Com2 = 0x2F8,
};

class EarlyUart : public StringPrinter
{
public:
	EarlyUart(ComPort port);

	virtual void Write(const std::string& string) override;

private:
	static constexpr size_t MaxBuffer = 256;

	const ComPort m_port;
};

