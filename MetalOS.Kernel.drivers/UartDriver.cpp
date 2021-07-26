#include "UartDriver.h"

#include "Kernel.h"
#include "Assert.h"

UartDriver::UartDriver(Device& device) : Driver(device), m_port(), m_index(), m_rxBuffer()
{
	const ACPI_RESOURCE* resource = (ACPI_RESOURCE*)device.GetResource(ACPI_RESOURCE_TYPE_IO);
	Assert(resource->Data.Io.AddressLength == 8);
	m_port = resource->Data.Io.Minimum;
}

Result UartDriver::Initialize()
{
	m_device.Type = DeviceType::Serial;

	//Disable interrupts
	InterruptEnableReg ier = { 0 };
	Write(Reg::InterruptEnable, ier.AsUint8);

	//Enable FIFO, clear RX/TX queues, set interrupt watermark at 14 bytes
	FifoControlReg fcr = { 0 };
	fcr.TriggerLevel = FifoTriggerLevel::FourteenBytes;
	fcr.Enable = 1;
	fcr.RxReset = 1;
	fcr.TxReset = 1;
	Write(Reg::FifoControl, fcr.AsUint8);

	//Mark terminal ready, enable interrupts
	ModemControlReg mcr = { 0 };
	mcr.Out2IntEnable = 1;
	mcr.RTS = 1;
	mcr.DTR = 1;
	Write(Reg::ModemControl, mcr.AsUint8);

	//Enable interrupts
	ier.DataReady = 1;
	Write(Reg::InterruptEnable, ier.AsUint8);

	return Result::NotImplemented;
}

Result UartDriver::Read(char* buffer, size_t length, size_t* bytesRead)
{
	if (length > m_rxBuffer.Count())
		return Result::Failed;
	
	Assert(buffer != nullptr);
	uint8_t* pBuffer = reinterpret_cast<uint8_t*>(buffer);

	size_t count = 0;
	while (count < length)
	{
		if (!m_rxBuffer.Read(*pBuffer))
			break;
		pBuffer++;
		count++;
	}

	if (bytesRead != nullptr)
		*bytesRead = count;

	return Result::Success;
}

Result UartDriver::Write(const char* buffer, size_t length)
{
	for (size_t i = 0; i < length; i++)
	{
		Write(Reg::TransmitterHolding, static_cast<uint8_t>(buffer[i]));
	}

	return Result::Success;
}

Result UartDriver::EnumerateChildren()
{
	return Result::NotImplemented;
}

void UartDriver::Write(const char* string)
{
	this->Write(string, strlen(string));
}

void UartDriver::OnInterrupt()
{
	InterruptStatusReg isr = { 0 };
	isr.AsUint8 = Read(Reg::InterruptStatus);
	Assert(isr.Status == false);//Interrupt is pending
	//Assert(isr.Type == InterruptType::ReceivedDataReady); ReceptionTimeout?

	kernel.Printf("ISR: 0x%x\n", isr.AsUint8);
	
	//Read all characters into a buffer
	LineStatusReg status = { 0 };
	status.AsUint8 = Read(Reg::LineStatus);
	while (status.DataReady)
	{
		Assert(!m_rxBuffer.IsFull());

		m_rxBuffer.Write(Read(Reg::ReceiverHolding));
		status.AsUint8 = Read(Reg::LineStatus);
	}

	//Ensure interrupts are handled
	isr.AsUint8 = Read(Reg::InterruptStatus);
	Assert(isr.Status == true);
	
	kernel.Printf("RxBuffer: 0x%x of 0x%x\n", m_rxBuffer.Count(), m_rxBuffer.Size());
	kernel.PrintBytes((char*)&m_rxBuffer, sizeof(m_rxBuffer));
}

uint8_t UartDriver::Read(Reg reg)
{
	return ArchReadPort(m_port + static_cast<uint8_t>(reg), std::numeric_limits<uint8_t>::digits);
}

void UartDriver::Write(Reg reg, uint8_t value)
{
	ArchWritePort(m_port + static_cast<uint8_t>(reg), value, std::numeric_limits<uint8_t>::digits);
}
