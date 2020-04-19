#pragma once
#include <cstdint>
#include "msvc.h"
#include <string>
#include "Driver.h"
#include "AcpiDevice.h"
#include "StringPrinter.h"

//https://www.lammertbies.nl/comm/info/serial-uart
//https://www.freebsd.org/doc/en_US.ISO8859-1/articles/serial-uart/index.html
//PNP0501 - Generic 16550A-compatible COM port
//Bit field note: https://docs.microsoft.com/en-us/cpp/cpp/cpp-bit-fields?view=vs-2019
//Microsoft Specific: The ordering of data declared as bit fields is from low to high bit, as shown in the figure above.
class UartDriver : public Driver, public StringPrinter
{
public:
	UartDriver(AcpiDevice& device);

	void Write(const char* buffer, size_t length) override;
	void Write(const char* string);

private:
	struct _INTERRUPT_ENABLE_REGISTER //RW
	{
		uint8_t ReceivedData : 1;
		uint8_t TransmitterEmpty : 1;
		uint8_t LineStatus : 1;
		uint8_t ModemStatus : 1;
		uint8_t Reserved : 4;
	};

	enum _IIR_TYPE : uint8_t
	{
		ModemStatus = 0,
		TransmitterEmpty = 1,
		ReceivedData = 2,
		LineStatus = 3,
		CharacterTimeout = 6,
	};

	struct _INTERRUPT_IDENTIFICATION_REGISTER //RO
	{
		uint8_t InterruptPending : 1;
		_IIR_TYPE Type : 3;
		uint8_t Reserved : 2;
		uint8_t FifoEnabled : 1;
		uint8_t FifoAttempted : 1;
	};

	enum _FCR_LEVEL : uint8_t
	{
		Byte,
		DWord,
		QWord,
		FourteenBytes//ugh
	};

	struct _FIFO_CONTROL_REGISTER //WO
	{
		uint8_t Enable : 1;
		uint8_t RecieverReset : 1;
		uint8_t TransmitterReset : 1;
		uint8_t DmaMode : 1;
		uint8_t Reserved : 2;
		_FCR_LEVEL : 2;
	};

	enum _LCR_WORD_LENGTH : uint8_t
	{
		Five,
		Six,
		Seven,
		Eight
	};
	
	struct _LINE_CONTROL_REGISTER //RW
	{
		union
		{
			struct
			{
				_LCR_WORD_LENGTH LengthSelect : 2;
				uint8_t StopBits : 1;
				uint8_t ParityEnabled : 1;
				uint8_t EvenParity : 1;
				uint8_t StickParity : 1;
				uint8_t SetBreak : 1;
				uint8_t DivisorLatchAccess : 1;
			} Fields;
			uint8_t Value;
		};
	};
	static_assert(sizeof(_LINE_CONTROL_REGISTER) == sizeof(uint8_t));

	uint8_t Read(uint8_t offset);
	void Write(uint8_t offset, uint8_t value);

	uint16_t ReadDivisorLatch();

	uint16_t m_port;
};

