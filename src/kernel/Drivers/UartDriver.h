#pragma once
#include <cstdint>
#include <string>
#include "Driver.h"
#include "Devices/Device.h"
#include "KRingBuffer.h"
#include "StringPrinter.h"

//PNP0501 - Generic 16550A-compatible COM port
//http://caro.su/msx/ocm_de1/16550.pdf . Page 9/18. Table 2.
//Bit field note: https://docs.microsoft.com/en-us/cpp/cpp/cpp-bit-fields?view=vs-2019
//Microsoft Specific: The ordering of data declared as bit fields is from low to high bit, as shown in the figure above.
class UartDriver : public Driver, public StringPrinter
{
public:
	UartDriver(Device& device);

	Result Initialize() override;
	Result Read(char* buffer, size_t length, size_t* bytesRead = nullptr) override;
	Result Write(const char* buffer, size_t length) override;
	Result EnumerateChildren() override;

	static void OnInterrupt(void* arg) { ((UartDriver*)arg)->OnInterrupt(); };

	virtual void Write(const std::string& string) override;

private:
	enum class Reg
	{
		//DLAB = 0
		ReceiverHolding = 0b000, //R (RHR)
		TransmitterHolding = 0b000, //W (THR)

		//DLAB = 1
		DivisorLatchLeastSig = 0b000, //RW (DLL)
		DivisorLatchMostSig = 0b001, //RW (DLM)
		PrescalerDivision = 0b101, //W (PSD)

		InterruptEnable = 0b001, //RW (IER)
		InterruptStatus = 0b010, //R (ISR)
		FifoControl = 0b010, //W (FCR)
		LineControl = 0b011, //RW (LCR)
		ModemControl = 0b100, //RW (MCR)
		LineStatus = 0b101, //R (LSR)
		ModemStatus = 0b110, //R (MSR)
		ScratchPad = 0b111 //RW (SPR)
	};

	struct InterruptEnableReg
	{
		union
		{
			struct
			{
				uint8_t DataReady : 1;
				uint8_t ThrEmpty : 1;
				uint8_t ReceiverLineStatus : 1;
				uint8_t ModemStatus : 1;
				uint8_t Reserved : 2;
				uint8_t DmaRxEnd : 1;
				uint8_t DmaTxEnd : 1;
			};
			uint8_t AsUint8;
		};
	};
	static_assert(sizeof(InterruptEnableReg) == sizeof(uint8_t));

	enum class InterruptType : uint8_t
	{
		ReceiverLineStatus = 0b011,
		ReceivedDataReady = 0b010,
		ReceptionTimeout = 0b110,
		ThrEmpty = 0b001,
		ModemStatus = 0b000,
		DmaReceptionEndOfTranfer = 0b111,
		DmaTransmissionEndOfTranfer = 0b101,
	};

	struct InterruptStatusReg
	{
		union
		{
			struct
			{
				uint8_t Status : 1;
				InterruptType Type : 3;
				uint8_t DmaRxEnd : 1;
				uint8_t DmaTxEnd : 1;
				uint8_t FifoEnabled : 2;
			};
			uint8_t AsUint8;
		};
	};
	static_assert(sizeof(InterruptStatusReg) == sizeof(uint8_t));

	enum class FifoTriggerLevel : uint8_t
	{
		Byte = 0b00,
		DWord = 0b01,
		QWord = 0b10,
		FourteenBytes = 0b11
	};

	struct FifoControlReg
	{
		union
		{
			struct
			{
				uint8_t Enable : 1;
				uint8_t RxReset : 1;
				uint8_t TxReset : 1;
				uint8_t DmaMode : 1;
				uint8_t EnableDmaEnd : 1;
				uint8_t Reserved : 1;
				FifoTriggerLevel TriggerLevel : 2;
			};
			uint8_t AsUint8;
		};
	};
	static_assert(sizeof(FifoControlReg) == sizeof(uint8_t));

	enum LcrWordLength : uint8_t
	{
		Five = 0b00,
		Six = 0b01,
		Seven = 0b10,
		Eight = 0b00
	};
	
	struct LineControlReg
	{
		union
		{
			struct
			{
				LcrWordLength WordLength : 2;
				uint8_t StopBits : 1;
				uint8_t ParityEnabled : 1;
				uint8_t EvenParity : 1;
				uint8_t ForceParity : 1;
				uint8_t SetBreak : 1;
				uint8_t DivisorLatchAccess : 1; //DLAB
			};
			uint8_t AsUint8;
		};
	};
	static_assert(sizeof(LineControlReg) == sizeof(uint8_t));

	struct ModemControlReg
	{
		union
		{
			struct
			{
				uint8_t DTR : 1;
				uint8_t RTS : 1;
				uint8_t Out1 : 1;
				uint8_t Out2IntEnable : 1;
				uint8_t LoopBack : 1;
				uint8_t Reserved : 3;
			};
			uint8_t AsUint8;
		};
	};
	static_assert(sizeof(ModemControlReg) == sizeof(uint8_t));

	struct LineStatusReg
	{
		union
		{
			struct
			{
				uint8_t DataReady : 1;
				uint8_t OverrunError : 1;
				uint8_t ParityError : 1;
				uint8_t FramingError : 1;
				uint8_t BreakInterrupt : 1;
				uint8_t ThrEmpty : 1;
				uint8_t TransmitterEmpty : 1;
				uint8_t FifoDataError : 1;
			};
			uint8_t AsUint8;
		};
	};
	static_assert(sizeof(LineStatusReg) == sizeof(uint8_t));

	struct ModemStatusReg
	{
		union
		{
			struct
			{
				uint8_t DeltaCTS : 1;
				uint8_t DeltaDSR : 1;
				uint8_t TrailingEdgeRI : 1;
				uint8_t DeltaCD : 1;
				uint8_t CTS : 1;
				uint8_t DSR : 1;
				uint8_t RI : 1;
				uint8_t CD : 1;
			};
			uint8_t AsUint8;
		};
	};
	static_assert(sizeof(ModemStatusReg) == sizeof(uint8_t));

	void OnInterrupt();
	uint8_t Read(Reg reg);
	void Write(Reg reg, uint8_t value);

	uint16_t m_port;
	size_t m_index;
	KRingBuffer<uint8_t, 0x100> m_rxBuffer;
};

