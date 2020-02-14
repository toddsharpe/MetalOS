#include "Kernel.h"

#include <intrin.h>
#include <cstdarg>

const Color Red = { 0x00, 0x00, 0xFF, 0x00 };
const Color Black = { 0x00, 0x00, 0x00, 0x00 };

Kernel::Kernel() :
	m_address(0),
	m_imageSize(0),
	m_pLoading(nullptr),
	m_pDisplay(nullptr),
	m_lastProcessId(0),
	m_processes(nullptr),
	m_lastSemaphoreId(0),
	m_semaphores(nullptr)
{

}

void Kernel::Initialize(const PLOADER_PARAMS params)
{
	//Save Loader Params
	m_address = params->KernelAddress;
	m_imageSize = params->KernelImageSize;

	//Initialize Heap

	
	m_pDisplay = new Display(params->Display);
	m_pDisplay->ColorScreen(Black);
	m_pLoading = new LoadingScreen(*m_pDisplay);

	m_pLoading->WriteText("Kernel");
}

void Kernel::HandleInterrupt(size_t vector, PINTERRUPT_FRAME pFrame)
{
	m_pLoading->WriteLineFormat("ISR: %d, Code: %d, RBP: 0x%16x, RIP: 0x%16x", vector, pFrame->ErrorCode, pFrame->RBP, pFrame->RIP);
	switch (vector)
	{
	case 14:
		m_pLoading->WriteLineFormat("CR2: 0x%16x", __readcr2());
		break;
	}
}

void Kernel::Bugcheck(const char* file, const char* line, const char* assert)
{
	//display.ColorScreen(Red);
	//TODO: color background function (pass lambda bool to colorscreen)

	//loading->ResetX();
	//loading->ResetY();
	m_pLoading->WriteLine(file);
	m_pLoading->WriteLine(line);
	m_pLoading->WriteLine(assert);

	__halt();
}

void Kernel::Print(const char* format, ...)
{
	va_list args;

	va_start(args, format);
	m_pLoading->WriteLineFormat(format, args);
	va_end(args);
}

void Kernel::Print(const char* format, va_list args)
{
	m_pLoading->WriteLineFormat(format, args);
}
