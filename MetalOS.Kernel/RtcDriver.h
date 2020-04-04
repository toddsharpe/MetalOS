#pragma once
#include "Driver.h"

//http://www.walshcomptech.com/ohlandl/config/cmos_registers.html
//https://wiki.osdev.org/CMOS
//http://stanislavs.org/helppc/cmos_ram.html

//RTC is IRQ 8
class RtcDriver : public Driver
{
public:
	RtcDriver(AcpiDevice& device);

	void Enable();

	//TODO: the interface to driver should have status codes and this should return not implemented
	void Write(const char* buffer, size_t length)
	{

	}

	void Display() const;

private:
	enum Register : uint8_t
	{
		RtcSeconds,
		RtcSecondsAlarm,
		RtcMinutes,
		RtcMinutesAlarm,
		RtcHours,
		RtsHoursAlarm,
		RtcDayOfTheWeek,
		RtcDateDay,
		RtcDateMonth,
		RtcYear,
		StatusRegisterA,
		StatusRegisterB,
		StatusRegisterC //RO
	};

	//frequency =  32768 >> (rate-1);
	//if rate = 6, 32768 >> 5 == 1kHz
	//if rate = 15, 32768 >> 14 == 2Hz
	struct _STATUS_REGISTER_A
	{
		union
		{
			struct
			{
				uint8_t RateSelectionFreq : 4; //Default: 0110 (selects 1kHz)
				uint8_t TimeFrequencyDivider : 3; //Default: 010 (selects 32kHz)
				uint8_t UpdateInProgress : 1;
			};
			uint8_t Value;
		};
	};

	struct _STATUS_REGISTER_B
	{
		union
		{
			struct
			{
				uint8_t EnableDaylightSavings : 1;
				uint8_t ArmyTime : 1;
				uint8_t DateMode : 1;
				uint8_t EnableSquareWave : 1;
				uint8_t UpdateInterruptEnable : 1;
				uint8_t AlarmInterruptEnable : 1;
				uint8_t PeriodInterruptEnable : 1;
				uint8_t SetClock : 1;
			};
			uint8_t Value;
		};
	};

	struct _STATUS_REGISTER_C //RO
	{
		union
		{
			struct
			{
				uint8_t Reserved : 4;
				uint8_t UpdateEndedEvent : 1;
				uint8_t AlarmEvent : 1;
				uint8_t PeriodicEvent : 1;
				uint8_t EventPending : 1;
			};
			uint8_t Value;
		};
	};

	static const uint8_t DisableNMI = (1 << 7);
	uint8_t ReadRegister(Register reg) const;
	void WriteRegister(Register reg, uint8_t value);

	uint16_t m_controlPort;
	uint16_t m_ioPort;
};

