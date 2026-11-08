#include "kernel/Syscalls/Syscalls.h"
#include "kernel/Core/Scheduler.h"
#include "kernel/Devices/Driver.h"
#include "kernel/Kernel.h"
#include "kernel/Devices/DeviceTree.h"
#include "kernel/Vmem.h"
#include "kernel/Display.h"

//Map the linear framebuffer into the calling process (used by the usermode WM).
SyscallResult MapFramebuffer(GraphicsDevice* device)
{
	UProcess& proc = Scheduler::GetUProcess();
	if (!device || !proc.Space.IsValid(device))
		return SyscallResult::InvalidPointer;

	//The display driver has the current (possibly host-negotiated) mode; without one (its init failed) use the boot GOP descriptor
	KDevice* const display = m_deviceTree.GetDeviceByClass(KDeviceClass::Display);
	DisplayDriver* const driver = display ? static_cast<DisplayDriver*>(display->Driver) : nullptr;
	const KFramebufferInfo fb = driver ? driver->GetFramebuffer() : KeGetFramebuffer();

	void* const va = KeVirtualMap(proc, fb.Base, fb.Size);
	if (!va)
		return SyscallResult::Failed;

	device->FrameBase = va;
	device->Width = fb.Width;
	device->Height = fb.Height;
	device->PixelsPerScanLine = fb.Pitch;
	return SyscallResult::Success;
}

//Present: ask the display driver to push the current framebuffer to the display (synthvid DIRT).
SyscallResult FlushFramebuffer()
{
	KDevice* const display = m_deviceTree.GetDeviceByClass(KDeviceClass::Display);
	if (!display || !display->Driver)
		return SyscallResult::Success; //no display driver (boot GOP is scanned out directly)

	static_cast<DisplayDriver*>(display->Driver)->Flush();
	return SyscallResult::Success;
}
