#pragma once

#include "MetalOS.System.h"
#include <stdint.h>

//Intel SDM Vol3A Chapter 4.5
//M=48 (MAXPHYADDR)

//Paging Structures - https://gist.github.com/mvankuipers/
//https://queazan.wordpress.com/2013/12/21/paging-under-amd64/
//https://software.intel.com/sites/default/files/managed/39/c5/325462-sdm-vol-1-2abcd-3abcd.pdf
//Class is inherently x64, so use uint64_t instead of uintptr_t
class PageTablesPool;
class PageTables
{
public:
	//Pool physical pages came from
	static PageTablesPool* Pool;
	static bool Debug;

	PageTables();
	PageTables(const paddr_t root);

	//NOTE(tsharpe): Could be static methods, but don't want to own the allocation
	void OpenCurrent();
	void CreateNew();
	paddr_t GetRoot() const;
	bool IsActive() const;

	//TODO(tsharpe): page attributes
	bool MapPages(const uintptr_t virtualBase, const paddr_t physicalBase, const size_t count, const bool global) const;
	//TODO: unmap
	paddr_t ResolveAddress(const uintptr_t virtualAddress) const;

	//Table manipulation
	void ClearKernelEntries() const;
	void LoadKernelMappings() const;

	void Display();
	void DisplayRoot() const;

private:
#pragma pack(push, 1)
	//Intel SDM Vol3A Table 4-14
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
				uint64_t PageSize : 1;             // Reserved, must be 0 for PML4E.
				uint64_t Ignored2 : 4;
				uint64_t PageFrameNumber : 36;     // The page frame number of the PDPT of this PML4E.
				uint64_t Reserved : 4;
				uint64_t Ignored3 : 11;
				uint64_t ExecuteDisable : 1;       // If 1, instruction fetches not allowed.
			};
			uint64_t Value;
		};
	} PML4E, * PPML4E;
	static_assert(sizeof(_PML4E) == sizeof(uintptr_t), "Size mismatch, only 64-bit supported.");

	//Intel SDM Vol3A Table 4-15
	typedef struct _PDPTE_PAGE //Maps 1GB Page
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
				uint64_t Dirty : 1;                // Ignored if PageSize=0, Dirty if 1GB page
				uint64_t PageSize : 1;             // If 1, this entry maps a 1GB page.
				uint64_t Global : 1;               // Ignored if PageSize=1, Global if 1GB page
				uint64_t Ignored1 : 3;             //
				uint64_t PageAccessType : 1;       //
				uint64_t Reserved1 : 17;           // Reserved (must be 0).
				uint64_t PageFrameNumber : 18;     //
				uint64_t Reserved2 : 4;
				uint64_t Ignored2 : 7;
				uint64_t Ignored3 : 4;
				uint64_t ExecuteDisable : 1;       // If 1, instruction fetches not allowed.
			};
			uint64_t Value;
		};
	} PDPTE_PAGE, * PPDPTE_PAGE;
	static_assert(sizeof(_PDPTE_PAGE) == sizeof(uintptr_t), "Incorrect.");

	//Intel SDM Vol3A Table 4-16
	typedef struct _PDPTE_DIR
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
	} PDPTE_DIR, * PPDPTE_DIR;
	static_assert(sizeof(_PDPTE_DIR) == sizeof(uintptr_t), "Incorrect.");

	//Intel SDM Vol3A Table 4-17
	typedef struct _PDE_PAGE //Maps 2MB page
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
				uint64_t Dirty : 1;
				uint64_t PageSize : 1;             // If 1, this entry maps a 2MB page.
				uint64_t Global : 1;
				uint64_t Ignored1 : 3;
				uint64_t PageAccessType : 1;       //
				uint64_t Reserved1 : 8;
				uint64_t PageFrameNumber : 27;     // The page frame number of the PT of this PDE.
				uint64_t Reserved2 : 4;
				uint64_t Ignored2 : 7;
				uint64_t Ignored3 : 4;
				uint64_t ExecuteDisable : 1;       // If 1, instruction fetches not allowed.
			};
			uint64_t Value;
		};
	} PDE_PAGE, * PPDE_PAGE;
	static_assert(sizeof(_PDE_PAGE) == sizeof(uintptr_t), "Incorrect.");

	//Intel SDM Vol3A Table 4-18
	typedef struct _PDE_DIR
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
	} PDE_DIR, * PPDE_DIR;
	static_assert(sizeof(_PDE_DIR) == sizeof(uintptr_t), "Size mismatch, only 64-bit supported.");

	//Intel SDM Vol3A Table 4-19
	typedef struct _PTE //Maps a 4KB page
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
	static_assert(sizeof(_PTE) == sizeof(uintptr_t), "Incorrect.");

	struct VirtualAddress
	{
		union
		{
			struct
			{
				uint64_t offset : 12;
				uint64_t index1 : 9;
				uint64_t index2 : 9;
				uint64_t index3 : 9;
				uint64_t index4 : 9;
				uint64_t upper : 16;
			};
			uint64_t AsUint64;
		};

	};
	static_assert(sizeof(VirtualAddress) == sizeof(uint64_t), "Size mismatch");
#pragma pack(pop)

	bool MapPage(const uintptr_t virtualAddress, const paddr_t physicalAddress, const bool global) const;
	uintptr_t BuildAddress(const size_t i4, const size_t i3, const size_t i2, const size_t i1, const size_t offset) const;

	paddr_t m_root;
};
