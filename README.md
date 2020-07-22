# MetalOS

## Introduction

### Goals
* 64bit
* 48-bit canonical address space (Shallow usermode, deep kernel mode, not identity paging)
* Ring 0 (Kernel) and Ring 3 (User)

### Non-Goals
* Run on hardware/off HyperV Gen2
* Support for bios


## Architecture

### Features
* Kernel Call Stack (virtual unwind + PDB support)

    IP: 0xffff8000010122c6 Function: ?Bugcheck@Kernel@@QEAAXPEBD00@Z Line: 395  
    IP: 0xffff800001017b3c Function: ?SetScreenBuffer@Kernel@@QEAA?AW4SystemCallResult@@PEAX@Z Line: 173  
    IP: 0xffff800001015383 Function: ?Syscall@Kernel@@QEAA_KPEAUSystemcallFrame@@@Z Line: 899  
    IP: 0xffff80000102e87f Function: SYSTEMCALL_HANDLER Line: 56  
    IP: 0xffff800001039c9c Function: x64_SYSTEMCALL Line: 50  

### 3rd Part code
* ACPCIA
* Virtual Unwinder from coreclr
* kvprintf

## Screenshots

## Doom
![Doom](Screenshots/Doom2.png)

![Doom](Screenshots/Doom3.png)