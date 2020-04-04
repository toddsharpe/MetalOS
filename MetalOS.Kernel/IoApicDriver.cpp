#include "IoApicDriver.h"
#include "Main.h"

IoApicDriver::IoApicDriver(AcpiDevice& device) : Driver(device)
{
	{
		ACPI_BUFFER result = { ACPI_ALLOCATE_BUFFER, nullptr };
		ACPI_STATUS status = AcpiEvaluateObjectTyped(m_device.GetHandle(), ACPI_STRING("_GSB"), nullptr, &result, ACPI_TYPE_INTEGER);
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
		ACPI_STATUS status = AcpiEvaluateObjectTyped(m_device.GetHandle(), ACPI_STRING("_MAT"), nullptr, &result, ACPI_TYPE_BUFFER);
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

