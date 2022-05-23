/*
 * Intel ACPI Component Architecture
 * AML/ASL+ Disassembler version 20200214 (32-bit version)
 * Copyright (c) 2000 - 2020 Intel Corporation
 * 
 * Disassembly of waet.dat, Wed Mar 11 07:50:37 2020
 *
 * ACPI Data Table [WAET]
 *
 * Format: [HexOffset DecimalOffset ByteLength]  FieldName : FieldValue
 */

[000h 0000   4]                    Signature : "WAET"    [Windows ACPI Emulated Devices Table]
[004h 0004   4]                 Table Length : 00000028
[008h 0008   1]                     Revision : 01
[009h 0009   1]                     Checksum : 22
[00Ah 0010   6]                       Oem ID : "VRTUAL"
[010h 0016   8]                 Oem Table ID : "MICROSFT"
[018h 0024   4]                 Oem Revision : 00000001
[01Ch 0028   4]              Asl Compiler ID : "MSFT"
[020h 0032   4]        Asl Compiler Revision : 00000001

[024h 0036   4]        Flags (decoded below) : 00000003
                        RTC needs no INT ack : 1
                     PM timer, one read only : 1

Raw Table Data: Length 40 (0x28)

    0000: 57 41 45 54 28 00 00 00 01 22 56 52 54 55 41 4C  // WAET(...."VRTUAL
    0010: 4D 49 43 52 4F 53 46 54 01 00 00 00 4D 53 46 54  // MICROSFT....MSFT
    0020: 01 00 00 00 03 00 00 00                          // ........
