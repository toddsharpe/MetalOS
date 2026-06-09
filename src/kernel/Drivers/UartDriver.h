#pragma once

#include "kernel/KDevice.h"
#include "kernel/Drivers/Driver.h"

//PNP0501 - Generic 16550A-compatible COM port
//http://caro.su/msx/ocm_de1/16550.pdf . Page 9/18. Table 2.
//Bit field note: https://docs.microsoft.com/en-us/cpp/cpp/cpp-bit-fields?view=vs-2019
//Microsoft Specific: The ordering of data declared as bit fields is from low to high bit, as shown in the figure above.
class UartDriver : public IoDriver
{
public:
	UartDriver(KDevice& device) :
		IoDriver(device),
		m_port()
	{

	}

	Result Initialize() override
	{
		uacpi_resources *res;
		uacpi_status ret = uacpi_get_current_resources(m_device.AcpiNode, &res);
		if (uacpi_unlikely_error(ret)) {
			Printf("unable to retrieve resources: %s", uacpi_status_to_string(ret));
			return Result::Failed;
		}

		//Loop for IO device
		uacpi_status loop = uacpi_for_each_resource(res, [](void *user, uacpi_resource *resource)
		{
			if (resource->type != UACPI_RESOURCE_TYPE_IO)
				return UACPI_ITERATION_DECISION_CONTINUE;

			((UartDriver*)user)->m_port = resource->io.minimum;
			return UACPI_ITERATION_DECISION_BREAK;
		}, this);

		uacpi_free_resources(res);
		return Result::Success;
	}

	Result Read(char* buffer, size_t length, size_t* bytesRead = nullptr) override
	{
		return Result::NotImplemented;
	}

	Result Write(const char* buffer, size_t length) override
	{
		for (size_t i = 0; i < length; i++)
		{
			Write(Reg::TransmitterHolding, static_cast<uint8_t>(buffer[i]));
		}

		return Result::Success;
	}

	Result Enumerate() override
	{
		return Result::NotImplemented;
	}

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

	void Write(Reg reg, uint8_t value)
	{
		ArchWritePort(m_port + static_cast<uint8_t>(reg), value, 8);
	}

	uint16_t m_port;
};

