/*
 * Intel ACPI Component Architecture
 * AML/ASL+ Disassembler version 20200214 (32-bit version)
 * Copyright (c) 2000 - 2020 Intel Corporation
 * 
 * Disassembly of xsdt.dat, Wed Mar 11 07:50:41 2020
 *
 * ACPI Data Table [XSDT]
 *
 * Format: [HexOffset DecimalOffset ByteLength]  FieldName : FieldValue
 */

[000h 0000   4]                    Signature : "XSDT"    [Extended System Description Table]
[004h 0004   4]                 Table Length : 0000005C
[008h 0008   1]                     Revision : 01
[009h 0009   1]                     Checksum : D8
[00Ah 0010   6]                       Oem ID : "VRTUAL"
[010h 0016   8]                 Oem Table ID : "MICROSFT"
[018h 0024   4]                 Oem Revision : 00000001
[01Ch 0028   4]              Asl Compiler ID : "MSFT"
[020h 0032   4]        Asl Compiler Revision : 00000001

[024h 0036   8]       ACPI Table Address   0 : 00000000F7FF8000
[02Ch 0044   8]       ACPI Table Address   1 : 00000000F7FF7000
[034h 0052   8]       ACPI Table Address   2 : 00000000F7FF6000
[03Ch 0060   8]       ACPI Table Address   3 : 00000000F7FF5000
[044h 0068   8]       ACPI Table Address   4 : 00000000F7FF4000
[04Ch 0076   8]       ACPI Table Address   5 : 00000000F7FF3000
[054h 0084   8]       ACPI Table Address   6 : 00000000F6DD6000

Raw Table Data: Length 92 (0x5C)

    0000: 58 53 44 54 5C 00 00 00 01 D8 56 52 54 55 41 4C  // XSDT\.....VRTUAL
    0010: 4D 49 43 52 4F 53 46 54 01 00 00 00 4D 53 46 54  // MICROSFT....MSFT
    0020: 01 00 00 00 00 80 FF F7 00 00 00 00 00 70 FF F7  // .............p..
    0030: 00 00 00 00 00 60 FF F7 00 00 00 00 00 50 FF F7  // .....`.......P..
    0040: 00 00 00 00 00 40 FF F7 00 00 00 00 00 30 FF F7  // .....@.......0..
    0050: 00 00 00 00 00 60 DD F6 00 00 00 00              // .....`......
