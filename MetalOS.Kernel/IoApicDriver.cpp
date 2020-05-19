#include "IoApicDriver.h"
#include "Main.h"

IoApicDriver::IoApicDriver(Device& device) : Driver(device)
{
	AcpiDevice* hack = (AcpiDevice*)& device;

	{
		ACPI_BUFFER result = { ACPI_ALLOCATE_BUFFER, nullptr };
		ACPI_STATUS status = AcpiEvaluateObjectTyped(hack->GetHandle(), ACPI_STRING("_GSB"), nullptr, &result, ACPI_TYPE_INTEGER);
		if (ACPI_FAILURE(status))
		{
			Print("Could not initialise AcpiEvaluateObjectTyped, %s!\n", AcpiFormatException(status));
		}
		else
		{
			ACPI_OBJECT* object = (ACPI_OBJECT*)result.Pointer;
			Assert(object);
			Print("Base: 0x%x\n", object->Integer.Value);
			delete result.Pointer;
		}
	}

	{
		ACPI_BUFFER result = { ACPI_ALLOCATE_BUFFER, nullptr };
		ACPI_STATUS status = AcpiEvaluateObjectTyped(hack->GetHandle(), ACPI_STRING("_MAT"), nullptr, &result, ACPI_TYPE_BUFFER);
		if (ACPI_FAILURE(status))
		{
			Print("Could not initialise AcpiEvaluateObjectTyped, %s!\n", AcpiFormatException(status));
		}
		else
		{
			ACPI_OBJECT* object = (ACPI_OBJECT*)result.Pointer;
			Assert(object);
			Print("Base: 0x%x\n", object->Buffer.Pointer);
			delete result.Pointer;
		}
	}
}

Result IoApicDriver::Initialize()
{
	return Result::ResultNotImplemented;
}

Result IoApicDriver::Read(const char* buffer, size_t length)
{
	return Result::ResultNotImplemented;
}

Result IoApicDriver::Write(const char* buffer, size_t length)
{
	return Result::ResultNotImplemented;
}

Result IoApicDriver::EnumerateChildren()
{
	return Result::ResultNotImplemented;
}

