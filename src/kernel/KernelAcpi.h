#include "uACPI/kernel_api.h"
#include "uACPI/uacpi.h"
#include "uACPI/event.h"
#include "uACPI/status.h"
#include "Assert.h"
#include "errno.h"

//Acpi is single threaded, just stub these out
typedef int mutex_t;
typedef int event_t;
typedef int semaphore_t;
typedef int spinlock_t;

int InitializeAcpi(void)
{
	/*
	 * Start with this as the first step of the initialization. This loads all
	 * tables, brings the event subsystem online, and enters ACPI mode. We pass
	 * in 0 as the flags as we don't want to override any default behavior for now.
	 */
	uacpi_status ret = uacpi_initialize(0);
	if (uacpi_unlikely_error(ret))
	{
		Printf("uacpi_initialize error: %s", uacpi_status_to_string(ret));
		return -ENODEV;
	}

	/*
	 * Load the AML namespace. This feeds DSDT and all SSDTs to the interpreter
	 * for execution.
	 */
	ret = uacpi_namespace_load();
	if (uacpi_unlikely_error(ret))
	{
		Printf("uacpi_namespace_load error: %s", uacpi_status_to_string(ret));
		return -ENODEV;
	}

	/*
	 * Initialize the namespace. This calls all necessary _STA/_INI AML methods,
	 * as well as _REG for registered operation region handlers.
	 */
	ret = uacpi_namespace_initialize();
	if (uacpi_unlikely_error(ret))
	{
		Printf("uacpi_namespace_initialize error: %s", uacpi_status_to_string(ret));
		return -ENODEV;
	}

	/*
	 * Tell uACPI that we have marked all GPEs we wanted for wake (even though we haven't
	 * actually marked any, as we have no power management support right now). This is
	 * needed to let uACPI enable all unmarked GPEs that have a corresponding AML handler.
	 * These handlers are used by the firmware to dynamically execute AML code at runtime
	 * to e.g. react to thermal events or device hotplug.
	 */
	ret = uacpi_finalize_gpe_initialization();
	if (uacpi_unlikely_error(ret))
	{
		Printf("uACPI GPE initialization error: %s", uacpi_status_to_string(ret));
		return -ENODEV;
	}

	/*
	 * That's it, uACPI is now fully initialized and working! You can proceed to
	 * using any public API at your discretion. The next recommended step is namespace
	 * enumeration and device discovery so you can bind drivers to ACPI objects.
	 */
	return 0;
}

uacpi_status uacpi_kernel_get_rsdp(uacpi_phys_addr *out_rsdp_address)
{
	*out_rsdp_address = (uintptr_t)GetAcpiTable();
	return uacpi_status::UACPI_STATUS_OK;
}
void *uacpi_kernel_map(uacpi_phys_addr addr, uacpi_size len)
{
	// Handle unaligned addresses
	const size_t pageOffset = addr & PageMask;
	const size_t pageCount = DivRoundUp(pageOffset + len, PageSize);

	const uintptr_t physicalBase = addr & ~PageMask;
	const void* virtualAddress = MapPages(physicalBase, pageCount, MapType::Acpi);
	return MakePointer<void*>(virtualAddress, pageOffset);
}

void uacpi_kernel_unmap(void *addr, uacpi_size len)
{
	UNUSED(addr);
	UNUSED(len);
}

void uacpi_kernel_log(uacpi_log_level, const uacpi_char* c_str)
{
	Printf(c_str);
}

uacpi_status uacpi_kernel_pci_device_open(uacpi_pci_address address, uacpi_handle *out_handle)
{
	NotImplemented();
	return uacpi_status::UACPI_STATUS_OK;
}

void uacpi_kernel_pci_device_close(uacpi_handle)
{
	NotImplemented();
}

uacpi_status uacpi_kernel_pci_read8(uacpi_handle device, uacpi_size offset, uacpi_u8 *value)
{
	NotImplemented();
	return uacpi_status::UACPI_STATUS_OK;
}

uacpi_status uacpi_kernel_pci_read16(uacpi_handle device, uacpi_size offset, uacpi_u16 *value)
{
	NotImplemented();
	return uacpi_status::UACPI_STATUS_OK;
}

uacpi_status uacpi_kernel_pci_read32(uacpi_handle device, uacpi_size offset, uacpi_u32 *value)
{
	NotImplemented();
	return uacpi_status::UACPI_STATUS_OK;
}

uacpi_status uacpi_kernel_pci_write8(uacpi_handle device, uacpi_size offset, uacpi_u8 value)
{
	NotImplemented();
	return uacpi_status::UACPI_STATUS_OK;
}

uacpi_status uacpi_kernel_pci_write16(uacpi_handle device, uacpi_size offset, uacpi_u16 value)
{
	NotImplemented();
	return uacpi_status::UACPI_STATUS_OK;
}

uacpi_status uacpi_kernel_pci_write32(uacpi_handle device, uacpi_size offset, uacpi_u32 value)
{
	NotImplemented();
	return uacpi_status::UACPI_STATUS_OK;
}

uacpi_status uacpi_kernel_io_map(uacpi_io_addr base, uacpi_size len, uacpi_handle *out_handle)
{
	*out_handle = reinterpret_cast<uacpi_handle>(base);
	return uacpi_status::UACPI_STATUS_OK;
}

void uacpi_kernel_io_unmap(uacpi_handle handle)
{
	NotImplemented();
}

uacpi_status uacpi_kernel_io_read8(uacpi_handle, uacpi_size offset, uacpi_u8 *out_value)
{
	*out_value = ArchReadPort(offset, 8);
	return uacpi_status::UACPI_STATUS_OK;
}

uacpi_status uacpi_kernel_io_read16(uacpi_handle, uacpi_size offset, uacpi_u16 *out_value)
{
	*out_value = ArchReadPort(offset, 16);
	return uacpi_status::UACPI_STATUS_OK;
}

uacpi_status uacpi_kernel_io_read32(uacpi_handle, uacpi_size offset, uacpi_u32 *out_value)
{
	*out_value = ArchReadPort(offset, 32);
	return uacpi_status::UACPI_STATUS_OK;
}

uacpi_status uacpi_kernel_io_write8(uacpi_handle, uacpi_size offset, uacpi_u8 in_value)
{
	ArchWritePort(offset, in_value, 8);
	return uacpi_status::UACPI_STATUS_OK;
}
uacpi_status uacpi_kernel_io_write16(uacpi_handle, uacpi_size offset, uacpi_u16 in_value)
{
	ArchWritePort(offset, in_value, 16);
	return uacpi_status::UACPI_STATUS_OK;
}

uacpi_status uacpi_kernel_io_write32(uacpi_handle, uacpi_size offset, uacpi_u32 in_value)
{
	ArchWritePort(offset, in_value, 32);
	return uacpi_status::UACPI_STATUS_OK;
}

void *uacpi_kernel_alloc(uacpi_size size)
{
	return KeHeapAlloc(size, HeapAllocType::Acpi);
}

void uacpi_kernel_free(void *mem)
{
	KeHeapFree(mem, HeapAllocType::Acpi);
}

uacpi_u64 uacpi_kernel_get_nanoseconds_since_boot(void)
{
	return KeGetNanoseconds();
}

void uacpi_kernel_stall(uacpi_u8 usec)
{
	NotImplemented();
}

void uacpi_kernel_sleep(uacpi_u64 msec)
{
	NotImplemented();
}

uacpi_handle uacpi_kernel_create_mutex(void)
{
	return KeHeapAlloc(sizeof(mutex_t), HeapAllocType::Acpi);
}

void uacpi_kernel_free_mutex(uacpi_handle ptr)
{
	KeHeapFree(ptr, HeapAllocType::Acpi);
}

uacpi_handle uacpi_kernel_create_event(void)
{
	return KeHeapAlloc(sizeof(event_t), HeapAllocType::Acpi);
}

void uacpi_kernel_free_event(uacpi_handle ptr)
{
	KeHeapFree(ptr, HeapAllocType::Acpi);
}

static int id = 1;
uacpi_thread_id uacpi_kernel_get_thread_id(void)
{
	//ACPI is single-threaded, just return 1
	return &id;
}

uacpi_status uacpi_kernel_acquire_mutex(uacpi_handle, uacpi_u16)
{
	return uacpi_status::UACPI_STATUS_OK;
}

void uacpi_kernel_release_mutex(uacpi_handle)
{
}

uacpi_bool uacpi_kernel_wait_for_event(uacpi_handle, uacpi_u16)
{
	return UACPI_TRUE;
}

void uacpi_kernel_signal_event(uacpi_handle)
{
}

void uacpi_kernel_reset_event(uacpi_handle)
{
}

uacpi_status uacpi_kernel_handle_firmware_request(uacpi_firmware_request*)
{
	return uacpi_status::UACPI_STATUS_OK;
}

uacpi_status uacpi_kernel_install_interrupt_handler(uacpi_u32 irq, uacpi_interrupt_handler, uacpi_handle ctx, uacpi_handle *out_irq_handle)
{
	return uacpi_status::UACPI_STATUS_OK;
}

uacpi_status uacpi_kernel_uninstall_interrupt_handler(uacpi_interrupt_handler, uacpi_handle irq_handle)
{
	return uacpi_status::UACPI_STATUS_OK;
}

uacpi_handle uacpi_kernel_create_spinlock(void)
{
	return KeHeapAlloc(sizeof(spinlock_t), HeapAllocType::Acpi);
}

void uacpi_kernel_free_spinlock(uacpi_handle ptr)
{
	KeHeapFree(ptr, HeapAllocType::Acpi);
}

uacpi_cpu_flags uacpi_kernel_lock_spinlock(uacpi_handle)
{
	return 0;
}

void uacpi_kernel_unlock_spinlock(uacpi_handle, uacpi_cpu_flags)
{
}

uacpi_status uacpi_kernel_schedule_work(uacpi_work_type, uacpi_work_handler, uacpi_handle ctx)
{
	return uacpi_status::UACPI_STATUS_OK;
}

uacpi_status uacpi_kernel_wait_for_work_completion(void)
{
	return uacpi_status::UACPI_STATUS_OK;
}
