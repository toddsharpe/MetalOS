#pragma once
#include "Driver.h"

//TODO: remove apic specific code out, setup interface, and implement x2apic incase?
class ProcessorDriver : public Driver
{
public:
	ProcessorDriver(AcpiDevice& device);
	void Write(const char* buffer, size_t length)
	{

	}

	void Display();

private:
	//Each register is 32 bits, aligned on 128bit boundaries
	//LVT - Local Vector Table
	enum _LAPIC_REGISTERS
	{
		LocalApicId = 0x20, //RO
		LocalApicVersion = 0x30, //RO
		TaskPriority = 0x80, //RW
		ArbitrationPriority = 0x90, //RO
		ProcessorPriority = 0xA0, //RO
		EOI = 0xB0, //WO
		RemoteRead = 0xC0, //RO
		LocalDestination = 0xD0, //RW
		DestinationFormat = 0xE0, //RW
		SpuriousInterruptVector = 0xF0, //RW
		InService0 = 0x100, //RO
		InService1 = 0x110, //RO
		InService2 = 0x120, //RO
		InService3 = 0x130, //RO
		InService4 = 0x140, //RO
		InService5 = 0x150, //RO
		InService6 = 0x160, //RO
		InService7 = 0x170, //RO
		TriggerMode0 = 0x180, //RO
		TriggerMode1 = 0x190, //RO
		TriggerMode2 = 0x1A0, //RO
		TriggerMode3 = 0x1B0, //RO
		TriggerMode4 = 0x1C0, //RO
		TriggerMode5 = 0x1D0, //RO
		TriggerMode6 = 0x1E0, //RO
		TriggerMode7 = 0x1F0, //RO
		InterruptRequest0 = 0x200, //RO
		InterruptRequest1 = 0x210, //RO
		InterruptRequest2 = 0x220, //RO
		InterruptRequest3 = 0x230, //RO
		InterruptRequest4 = 0x240, //RO
		InterruptRequest5 = 0x250, //RO
		InterruptRequest6 = 0x260, //RO
		InterruptRequest7 = 0x270, //RO
		ErrorStatus = 0x280, //RO
		LVTCMCI = 0x2F0, //RW
		InterruptCommand0 = 0x300, //RW
		InterruptCommand1 = 0x310, //RW
		LVTTimer = 0x320, //RW
		LVTThermalSensor = 0x330, //RW
		LVTPerfCounters = 0x340, //RW
		LVTLINT0 = 0x350, //RW
		LVTLINT1 = 0x360, //RW
		LVTError = 0x370, //RW
		TimerInitialCount = 0x380, //RW
		TimerCurrentCount = 0x390, //RW
		TimerDivideConfig = 0x3E0, //RW
	};
	
	struct _IA32_APIC_BASE_MSR
	{
		union
		{
			struct
			{
				uint64_t Reserved0 : 8;
				uint64_t BootStrapProcessor : 1;
				uint64_t Reserved1 : 2;
				uint64_t ApicEnabled : 1;
				uint64_t ApicBase : 36; //Physical frame number
				uint64_t Reserved2 : 16;
			};
			uint64_t Value;
		};
	};

	struct _APIC_ID_REGISTER
	{
		union
		{
			struct
			{
				uint32_t Reserved : 24;
				uint32_t ApicId : 8;
			};
			uint32_t Value;
		};
	};

	struct _APIC_VERSION_REGISTER
	{
		union
		{
			struct
			{
				uint32_t Version : 8;
				uint32_t Reserved0 : 8;
				uint32_t MaxLVTEntry : 8;
				uint32_t EOIBroadcastSuppressionSupport : 1;
				uint32_t Reserved1 : 7;
			};
			uint32_t Value;
		};
	};

	enum _LVT_TIMER_MODE : uint32_t
	{
		OneShot = 0b00,
		Periodic = 0b01,
		TSC_Deadline = 0b10
	};

	//Other combinations reserved
	enum _LVT_DELIVERY_MODE : uint32_t
	{
		Fixed = 0b000,
		SMI = 0b010,
		NMI = 0b100,
		ExtINT = 0b111,
	};

	enum _LVT_TRIGGER_MODE : uint32_t
	{
		Edge = 0,
		Level = 1
	};

	//Intel SDM Vol. 3A Figure 10-8
	//
	struct _LVT_REGISTER
	{
		union
		{
			struct
			{
				uint32_t Vector : 8;
				_LVT_DELIVERY_MODE : 3;
				uint32_t Reserved0 : 1;
				uint32_t DeliveryStatus : 1;
				uint32_t InputPinPolarity : 1;
				uint32_t RemoteIRR : 1;
				_LVT_TRIGGER_MODE TriggerMode : 1;
				uint32_t Mask : 1;
				_LVT_TIMER_MODE Mode : 2;
				uint32_t Reserved2 : 13;
			};
			uint32_t Value;
		};
	};
	static const uint32_t LVT_REGISTER_DEFAULT = 0x10000;

	//"Before attempt to read from the ESR, software should first write to it."
	struct _ERROR_STATUS_REGISTER
	{
		union
		{
			struct
			{
				uint32_t SendChecksumError : 1;
				uint32_t ReceiveChecksumError : 1;
				uint32_t SendAcceptError : 1;
				uint32_t ReceiveAcceptError : 1;
				uint32_t RedirectableIPI : 1;
				uint32_t SendIllegalVector : 1;
				uint32_t ReceivedIllegalVector : 1;
				uint32_t IllegalRegisterAddress : 1;
				uint32_t Reserved : 24;
			};
			uint32_t Value;
		};
	};

	//Note: bit 2 is reserved (so always 0)
	enum _TIMER_DIVIDE : uint32_t
	{
		DivideBy2 = 0b0000,
		DivideBy4 = 0b0001,
		DivideBy8 = 0b0010,
		DivideBy16 = 0b0011,
		DivideBy32 = 0b1000,
		DivideBy64 = 0b1001,
		DivideBy128 = 0b1010,
		DivideBy1 = 0b1011,
	};

	struct _TIMER_DIVIDE_CONFIG_REGISTER
	{
		union
		{
			struct
			{
				_TIMER_DIVIDE DivideValue : 4;
				uint32_t Reserved : 28;
			};
			uint32_t Value;
		};
	};

	static const int IA32_APIC_BASE = 0x1B;

	static void WriteRegister(_LAPIC_REGISTERS reg, uint32_t value);
	static uint32_t ReadRegister(_LAPIC_REGISTERS reg);

	void* m_apic;
};

