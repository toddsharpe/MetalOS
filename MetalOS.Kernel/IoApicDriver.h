#pragma once
#include "msvc.h"
#include <cstdint>
#include <string>
#include "Driver.h"
#include "Device.h"

//https://wiki.osdev.org/APIC
//ioapic.pdf
class IoApicDriver : public Driver
{
public:
	IoApicDriver(Device& device);

	Result Initialize() override;
	Result Read(char* buffer, size_t length, size_t* bytesRead = nullptr) override;
	Result Write(const char* buffer, size_t length) override;
	Result EnumerateChildren() override;

	Result MapInterrupt(InterruptVector vector, uint16_t Irq);
	Result UnmaskInterrupt(uint16_t Irq);

private:
	struct IoApic
	{
		volatile uint32_t Index;
		uint32_t Reserved[3];
		volatile uint32_t Data;
	};
	
	enum IoApicReg
	{
		ID = 0x00, //RW
		Version = 0x01, //RO
		ArbitrationID = 0x02, //RO
		FirstEntry = 0x10
	};

	struct IdReg
	{
		union
		{
			struct
			{
				uint32_t Reserved1 : 24;
				uint32_t Id : 4;
				uint32_t Reserved2 : 4;
			};
			uint32_t AsUint32;
		};
	};
	static_assert(sizeof(IdReg) == sizeof(uint32_t));

	struct VersionReg
	{
		union
		{
			struct
			{
				uint32_t Version : 8;
				uint32_t Reserved1 : 8;
				uint32_t MaxEntry : 8;
				uint32_t Reserved2 : 8;
			};
			uint32_t AsUint32;
		};
	};
	static_assert(sizeof(VersionReg) == sizeof(uint32_t));

	enum class DeliveryMode : uint64_t
	{
		Fixed = 0b000,
		LowestPriority = 0b001,
		SMI = 0b010,
		Reserved1 = 0b011,
		NMI = 0b100,
		INIT = 0b101,
		Reserved2 = 0b110,
		ExtINT = 0b111
	};
	
	struct RedirectionEntry
	{
		union
		{
			struct
			{
				uint64_t InterruptVector : 8; //INTVEC - R/W
				DeliveryMode DeliveryMode : 3; //DELMOD - R/W
				uint64_t DestinationMode : 1; //DESTMOD - R/W
				uint64_t DeliveryStatus : 1; //DELIVS - RO
				uint64_t InterruptPolarity : 1; //INTPOL - R/W
				uint64_t RemoteIRR : 1; //RO
				uint64_t TriggerMode : 1; //R/W
				uint64_t InterruptMasked : 1; //R/W
				uint64_t Reserved : 39;
				uint64_t Destination : 8; //R/W
			};

			struct
			{
				uint32_t Low;
				uint32_t High;
			};

			uint64_t AsUint64;
		};
	};
	static_assert(sizeof(RedirectionEntry) == sizeof(uint64_t));

	RedirectionEntry ReadEntry(int pin);
	void WriteEntry(int pin, RedirectionEntry entry);

	uint32_t ReadReg(IoApicReg reg);
	void WriteReg(IoApicReg reg, const uint32_t value);

	IoApic* m_apic;
	uint32_t m_maxEntry;
};

