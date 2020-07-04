#pragma once
#include <cstdint>

#include "PageTablesPool.h"


class PageTablesPool;
class PageTables
{
public:
	static PageTablesPool* Pool;

	PageTables();
	PageTables(uintptr_t physicalAddress);

	//TODO: attributes
	bool MapUserPages(uintptr_t virtualAddress, uintptr_t physicalAddress, uint32_t count);
	bool MapKernelPages(uintptr_t virtualAddress, uintptr_t physicalAddress, uint32_t count);

	uintptr_t GetCr3() const;
	
	//TODO: unmap

	void LoadKernelMappings(PageTables* copyPt);

	bool EnableWrite(uintptr_t virtualAddress);

	//Is there a use for this besides testing?
	uintptr_t ResolveAddress(uintptr_t virtualAddress);

	void Display();
	void DisplayCr3();

private:
#pragma pack(push, 1)
	typedef struct _PML4E
	{
		union
		{
			struct
			{
				uint64_t Present : 1;              // Must be 1, region invalid if 0.
				uint64_t ReadWrite : 1;            // If 0, writes not allowed.
				uint64_t UserSupervisor : 1;       // If 0, user-mode accesses not allowed.
				uint64_t PageWriteThrough : 1;     // Determines the memory type used to access PDPT.
				uint64_t PageCacheDisable : 1;     // Determines the memory type used to access PDPT.
				uint64_t Accessed : 1;             // If 0, this entry has not been used for translation.
				uint64_t Ignored1 : 1;
				uint64_t PageSize : 1;             // Must be 0 for PML4E.
				uint64_t Ignored2 : 4;
				uint64_t PageFrameNumber : 36;     // The page frame number of the PDPT of this PML4E.
				uint64_t Reserved : 4;
				uint64_t Ignored3 : 11;
				uint64_t ExecuteDisable : 1;       // If 1, instruction fetches not allowed.
			};
			uint64_t Value;
		};
	} PML4E, * PPML4E;
	static_assert(sizeof(PML4E) == sizeof(uintptr_t), "Size mismatch, only 64-bit supported.");

	typedef struct _PDPTE
	{
		union
		{
			struct
			{
				uint64_t Present : 1;              // Must be 1, region invalid if 0.
				uint64_t ReadWrite : 1;            // If 0, writes not allowed.
				uint64_t UserSupervisor : 1;       // If 0, user-mode accesses not allowed.
				uint64_t PageWriteThrough : 1;     // Determines the memory type used to access PD.
				uint64_t PageCacheDisable : 1;     // Determines the memory type used to access PD.
				uint64_t Accessed : 1;             // If 0, this entry has not been used for translation.
				uint64_t Ignored1 : 1;
				uint64_t PageSize : 1;             // If 1, this entry maps a 1GB page.
				uint64_t Ignored2 : 4;
				uint64_t PageFrameNumber : 36;     // The page frame number of the PD of this PDPTE.
				uint64_t Reserved : 4;
				uint64_t Ignored3 : 11;
				uint64_t ExecuteDisable : 1;       // If 1, instruction fetches not allowed.
			};
			uint64_t Value;
		};
	} PDPTE, * PPDPTE;
	static_assert(sizeof(PDPTE) == sizeof(uintptr_t), "Size mismatch, only 64-bit supported.");

	typedef struct _PDE
	{
		union
		{
			struct
			{
				uint64_t Present : 1;              // Must be 1, region invalid if 0.
				uint64_t ReadWrite : 1;            // If 0, writes not allowed.
				uint64_t UserSupervisor : 1;       // If 0, user-mode accesses not allowed.
				uint64_t PageWriteThrough : 1;     // Determines the memory type used to access PT.
				uint64_t PageCacheDisable : 1;     // Determines the memory type used to access PT.
				uint64_t Accessed : 1;             // If 0, this entry has not been used for translation.
				uint64_t Ignored1 : 1;
				uint64_t PageSize : 1;             // If 1, this entry maps a 2MB page.
				uint64_t Ignored2 : 4;
				uint64_t PageFrameNumber : 36;     // The page frame number of the PT of this PDE.
				uint64_t Reserved : 4;
				uint64_t Ignored3 : 11;
				uint64_t ExecuteDisable : 1;       // If 1, instruction fetches not allowed.
			};
			uint64_t Value;
		};
	} PDE, * PPDE;
	static_assert(sizeof(PDE) == sizeof(uintptr_t), "Size mismatch, only 64-bit supported.");

	typedef struct _PTE
	{
		union
		{
			struct
			{
				uint64_t Present : 1;              // Must be 1, region invalid if 0.
				uint64_t ReadWrite : 1;            // If 0, writes not allowed.
				uint64_t UserSupervisor : 1;       // If 0, user-mode accesses not allowed.
				uint64_t PageWriteThrough : 1;     // Determines the memory type used to access the memory.
				uint64_t PageCacheDisable : 1;     // Determines the memory type used to access the memory.
				uint64_t Accessed : 1;             // If 0, this entry has not been used for translation.
				uint64_t Dirty : 1;                // If 0, the memory backing this page has not been written to.
				uint64_t PageAccessType : 1;       // Determines the memory type used to access the memory.
				uint64_t Global : 1;                // If 1 and the PGE bit of CR4 is set, translations are global.
				uint64_t OSCopyOnWrite : 1;
				uint64_t OSPrototypePTE : 1;
				uint64_t OSWrite : 1;
				uint64_t PageFrameNumber : 36;     // The page frame number of the backing physical page.
				uint64_t Reserved : 4;
				uint64_t Ignored3 : 7;
				uint64_t ProtectionKey : 4;         // If the PKE bit of CR4 is set, determines the protection key.
				uint64_t ExecuteDisable : 1;       // If 1, instruction fetches not allowed.
			};
			uint64_t Value;
		};
	} PTE, * PPTE;
	static_assert(sizeof(PTE) == sizeof(uintptr_t), "Size mismatch, only 64-bit supported.");
#pragma pack(pop)

	bool MapPage(uintptr_t virtualAddress, uintptr_t physicalAddress, uint32_t count, bool global);
	bool MapPage(uintptr_t virtualAddress, uintptr_t physicalAddress, bool global);

	uintptr_t m_physicalAddress;
};

