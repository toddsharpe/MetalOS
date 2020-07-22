/*
 * Intel ACPI Component Architecture
 * AML/ASL+ Disassembler version 20200214 (32-bit version)
 * Copyright (c) 2000 - 2020 Intel Corporation
 * 
 * Disassembly of srat.dat, Wed Mar 11 07:48:43 2020
 *
 * ACPI Data Table [SRAT]
 *
 * Format: [HexOffset DecimalOffset ByteLength]  FieldName : FieldValue
 */

[000h 0000   4]                    Signature : "SRAT"    [System Resource Affinity Table]
[004h 0004   4]                 Table Length : 00000180
[008h 0008   1]                     Revision : 02
[009h 0009   1]                     Checksum : 41
[00Ah 0010   6]                       Oem ID : "VRTUAL"
[010h 0016   8]                 Oem Table ID : "MICROSFT"
[018h 0024   4]                 Oem Revision : 00000001
[01Ch 0028   4]              Asl Compiler ID : "MSFT"
[020h 0032   4]        Asl Compiler Revision : 00000001

[024h 0036   4]               Table Revision : 00000001
[028h 0040   8]                     Reserved : 0000000000000000

[030h 0048   1]                Subtable Type : 00 [Processor Local APIC/SAPIC Affinity]
[031h 0049   1]                       Length : 10

[032h 0050   1]      Proximity Domain Low(8) : 00
[033h 0051   1]                      Apic ID : 00
[034h 0052   4]        Flags (decoded below) : 00000001
                                     Enabled : 1
[038h 0056   1]              Local Sapic EID : 00
[039h 0057   3]    Proximity Domain High(24) : 000000
[03Ch 0060   4]                 Clock Domain : 00000000

[040h 0064   1]                Subtable Type : 01 [Memory Affinity]
[041h 0065   1]                       Length : 28

[042h 0066   4]             Proximity Domain : 00000000
[046h 0070   2]                    Reserved1 : 0000
[048h 0072   8]                 Base Address : 0000000000000000
[050h 0080   8]               Address Length : 00000000F8000000
[058h 0088   4]                    Reserved2 : 00000000
[05Ch 0092   4]        Flags (decoded below) : 00000003
                                     Enabled : 1
                               Hot Pluggable : 1
                                Non-Volatile : 0
[060h 0096   8]                    Reserved3 : 0000000000000000

[068h 0104   1]                Subtable Type : 01 [Memory Affinity]
[069h 0105   1]                       Length : 28

[06Ah 0106   4]             Proximity Domain : 00000000
[06Eh 0110   2]                    Reserved1 : 0000
[070h 0112   8]                 Base Address : 0000000100000000
[078h 0120   8]               Address Length : 0000000108000000
[080h 0128   4]                    Reserved2 : 00000000
[084h 0132   4]        Flags (decoded below) : 00000003
                                     Enabled : 1
                               Hot Pluggable : 1
                                Non-Volatile : 0
[088h 0136   8]                    Reserved3 : 0000000000000000

[090h 0144   1]                Subtable Type : 01 [Memory Affinity]
[091h 0145   1]                       Length : 28

[092h 0146   4]             Proximity Domain : 00000000
[096h 0150   2]                    Reserved1 : 0000
[098h 0152   8]                 Base Address : 0000000208000000
[0A0h 0160   8]               Address Length : 0000000DD8000000
[0A8h 0168   4]                    Reserved2 : 00000000
[0ACh 0172   4]        Flags (decoded below) : 00000003
                                     Enabled : 1
                               Hot Pluggable : 1
                                Non-Volatile : 0
[0B0h 0176   8]                    Reserved3 : 0000000000000000

[0B8h 0184   1]                Subtable Type : 01 [Memory Affinity]
[0B9h 0185   1]                       Length : 28

[0BAh 0186   4]             Proximity Domain : 00000000
[0BEh 0190   2]                    Reserved1 : 0000
[0C0h 0192   8]                 Base Address : 0000001000000000
[0C8h 0200   8]               Address Length : 000000F000000000
[0D0h 0208   4]                    Reserved2 : 00000000
[0D4h 0212   4]        Flags (decoded below) : 00000003
                                     Enabled : 1
                               Hot Pluggable : 1
                                Non-Volatile : 0
[0D8h 0216   8]                    Reserved3 : 0000000000000000

[0E0h 0224   1]                Subtable Type : 01 [Memory Affinity]
[0E1h 0225   1]                       Length : 28

[0E2h 0226   4]             Proximity Domain : 00000000
[0E6h 0230   2]                    Reserved1 : 0000
[0E8h 0232   8]                 Base Address : 0000010000000000
[0F0h 0240   8]               Address Length : 0000010000000000
[0F8h 0248   4]                    Reserved2 : 00000000
[0FCh 0252   4]        Flags (decoded below) : 00000003
                                     Enabled : 1
                               Hot Pluggable : 1
                                Non-Volatile : 0
[100h 0256   8]                    Reserved3 : 0000000000000000

[108h 0264   1]                Subtable Type : 01 [Memory Affinity]
[109h 0265   1]                       Length : 28

[10Ah 0266   4]             Proximity Domain : 00000000
[10Eh 0270   2]                    Reserved1 : 0000
[110h 0272   8]                 Base Address : 0000020000000000
[118h 0280   8]               Address Length : 0000020000000000
[120h 0288   4]                    Reserved2 : 00000000
[124h 0292   4]        Flags (decoded below) : 00000003
                                     Enabled : 1
                               Hot Pluggable : 1
                                Non-Volatile : 0
[128h 0296   8]                    Reserved3 : 0000000000000000

[130h 0304   1]                Subtable Type : 01 [Memory Affinity]
[131h 0305   1]                       Length : 28

[132h 0306   4]             Proximity Domain : 00000000
[136h 0310   2]                    Reserved1 : 0000
[138h 0312   8]                 Base Address : 0000040000000000
[140h 0320   8]               Address Length : 0000040000000000
[148h 0328   4]                    Reserved2 : 00000000
[14Ch 0332   4]        Flags (decoded below) : 00000003
                                     Enabled : 1
                               Hot Pluggable : 1
                                Non-Volatile : 0
[150h 0336   8]                    Reserved3 : 0000000000000000

[158h 0344   1]                Subtable Type : 01 [Memory Affinity]
[159h 0345   1]                       Length : 28

[15Ah 0346   4]             Proximity Domain : 00000000
[15Eh 0350   2]                    Reserved1 : 0000
[160h 0352   8]                 Base Address : 0000080000000000
[168h 0360   8]               Address Length : 0000080000000000
[170h 0368   4]                    Reserved2 : 00000000
[174h 0372   4]        Flags (decoded below) : 00000003
                                     Enabled : 1
                               Hot Pluggable : 1
                                Non-Volatile : 0
[178h 0376   8]                    Reserved3 : 0000000000000000

Raw Table Data: Length 384 (0x180)

    0000: 53 52 41 54 80 01 00 00 02 41 56 52 54 55 41 4C  // SRAT.....AVRTUAL
    0010: 4D 49 43 52 4F 53 46 54 01 00 00 00 4D 53 46 54  // MICROSFT....MSFT
    0020: 01 00 00 00 01 00 00 00 00 00 00 00 00 00 00 00  // ................
    0030: 00 10 00 00 01 00 00 00 00 00 00 00 00 00 00 00  // ................
    0040: 01 28 00 00 00 00 00 00 00 00 00 00 00 00 00 00  // .(..............
    0050: 00 00 00 F8 00 00 00 00 00 00 00 00 03 00 00 00  // ................
    0060: 00 00 00 00 00 00 00 00 01 28 00 00 00 00 00 00  // .........(......
    0070: 00 00 00 00 01 00 00 00 00 00 00 08 01 00 00 00  // ................
    0080: 00 00 00 00 03 00 00 00 00 00 00 00 00 00 00 00  // ................
    0090: 01 28 00 00 00 00 00 00 00 00 00 08 02 00 00 00  // .(..............
    00A0: 00 00 00 D8 0D 00 00 00 00 00 00 00 03 00 00 00  // ................
    00B0: 00 00 00 00 00 00 00 00 01 28 00 00 00 00 00 00  // .........(......
    00C0: 00 00 00 00 10 00 00 00 00 00 00 00 F0 00 00 00  // ................
    00D0: 00 00 00 00 03 00 00 00 00 00 00 00 00 00 00 00  // ................
    00E0: 01 28 00 00 00 00 00 00 00 00 00 00 00 01 00 00  // .(..............
    00F0: 00 00 00 00 00 01 00 00 00 00 00 00 03 00 00 00  // ................
    0100: 00 00 00 00 00 00 00 00 01 28 00 00 00 00 00 00  // .........(......
    0110: 00 00 00 00 00 02 00 00 00 00 00 00 00 02 00 00  // ................
    0120: 00 00 00 00 03 00 00 00 00 00 00 00 00 00 00 00  // ................
    0130: 01 28 00 00 00 00 00 00 00 00 00 00 00 04 00 00  // .(..............
    0140: 00 00 00 00 00 04 00 00 00 00 00 00 03 00 00 00  // ................
    0150: 00 00 00 00 00 00 00 00 01 28 00 00 00 00 00 00  // .........(......
    0160: 00 00 00 00 00 08 00 00 00 00 00 00 00 08 00 00  // ................
    0170: 00 00 00 00 03 00 00 00 00 00 00 00 00 00 00 00  // ................
