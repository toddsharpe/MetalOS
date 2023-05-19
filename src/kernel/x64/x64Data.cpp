#include "x64.h"

#include "Interrupts.h"

// "Known good stacks" Intel SDM Vol 3A 6.14.5
volatile uint8_t x64::DOUBLEFAULT_STACK[IstStackSize] = { 0 };
volatile uint8_t x64::NMI_Stack[IstStackSize] = { 0 };
volatile uint8_t x64::DEBUG_STACK[IstStackSize] = { 0 };
volatile uint8_t x64::MCE_STACK[IstStackSize] = { 0 };

//Kernel Structures
volatile x64::TASK_STATE_SEGMENT_64 x64::TSS64 =
{
	0, //Reserved
	0, 0, //RSP 0 low/high
	0, 0, //RSP 1 low/high
	0, 0, //RSP 2 low/high
	0, 0, //Reserved
	QWordLow(DOUBLEFAULT_STACK + IstStackSize), QWordHigh(DOUBLEFAULT_STACK + IstStackSize), //IST1
	QWordLow(NMI_Stack + IstStackSize), QWordHigh(NMI_Stack + IstStackSize), //IST2
	QWordLow(DEBUG_STACK + IstStackSize), QWordHigh(DEBUG_STACK + IstStackSize), //IST3
	QWordLow(MCE_STACK + IstStackSize), QWordHigh(MCE_STACK + IstStackSize), //IST4
	0, 0, //IST5
	0, 0, //IST6
	0, 0, //IST7
	0, 0, 0, //Reserved
	0 //IO Map Base
};

#define GDT_TYPE_RW 0x2
#define GDT_TYPE_EX 0x8

//1 empty, 4 GDTs, and 1 TSS
volatile x64::KERNEL_GDTS x64::KernelGDT =
{
	{ 0 }, //First entry has to be empty
	// Seg1   Base  type						  S    DPL		   P    Seg2   OS      L     DB   4GB   Base
	{ 0xFFFF, 0, 0, GDT_TYPE_RW | GDT_TYPE_EX,    true, KernelDPL, true, 0xF, false, true, false, true, 0x00 }, //64-bit code Kernel
	{ 0xFFFF, 0, 0, GDT_TYPE_RW,                  true, KernelDPL, true, 0xF, false, false, true, true, 0x00 }, //64-bit data Kernel
	{ 0xFFFF, 0, 0, GDT_TYPE_RW | GDT_TYPE_EX,    true, UserDPL,   true, 0xF, false, false, true, true, 0x00 }, //32-bit code User
	{ 0xFFFF, 0, 0, GDT_TYPE_RW,                  true, UserDPL,   true, 0xF, false, false, true, true, 0x00 }, //64-bit data User
	{ 0xFFFF, 0, 0, GDT_TYPE_RW | GDT_TYPE_EX,    true, UserDPL,   true, 0xF, false, true, false, true, 0x00 }, //64-bit code User
	{
		// Seg1				Base1			Base2							type  S    DPL  P   Seg2	OS      L   DB     4GB   Base3
		sizeof(x64::TSS64) - 1, (uint16_t)&x64::TSS64, (uint8_t)((uint64_t)&x64::TSS64 >> 16), 0x9, false, 0, true, 0, false, false, false, true, (uint8_t)((uint64_t)&x64::TSS64 >> 24),
		// Base4						Zeroes
		QWordHigh(&x64::TSS64), 0, 0, 0
	}
};

//cs = starhigh + 2
//ss = starhigh + 1
//

volatile x64::IDT_GATE x64::IDT[IdtCount] =
{
	IDT_GATE((uint64_t)&InterruptHandler(0)),
	IDT_GATE((uint64_t)&InterruptHandler(1)),
	IDT_GATE((uint64_t)&InterruptHandler(2), IST_NMI_IDX, IDT_GATE_TYPE::InterruptGate32),
	IDT_GATE((uint64_t)&InterruptHandler(3), IST_DEBUG_IDX, IDT_GATE_TYPE::InterruptGate32, UserDPL),
	IDT_GATE((uint64_t)&InterruptHandler(4)),
	IDT_GATE((uint64_t)&InterruptHandler(5)),
	IDT_GATE((uint64_t)&InterruptHandler(6)),
	IDT_GATE((uint64_t)&InterruptHandler(7)),
	IDT_GATE((uint64_t)&InterruptHandler(8), IST_DOUBLEFAULT_IDX, IDT_GATE_TYPE::InterruptGate32),
	IDT_GATE((uint64_t)&InterruptHandler(9)),
	IDT_GATE((uint64_t)&InterruptHandler(10)),
	IDT_GATE((uint64_t)&InterruptHandler(11)),
	IDT_GATE((uint64_t)&InterruptHandler(12)),
	IDT_GATE((uint64_t)&InterruptHandler(13)),
	IDT_GATE((uint64_t)&InterruptHandler(14)),
	IDT_GATE((uint64_t)&InterruptHandler(15)),
	IDT_GATE((uint64_t)&InterruptHandler(16)),
	IDT_GATE((uint64_t)&InterruptHandler(17)),
	IDT_GATE((uint64_t)&InterruptHandler(18), IST_MCE_IDX, IDT_GATE_TYPE::InterruptGate32),
	IDT_GATE((uint64_t)&InterruptHandler(19)),
	IDT_GATE((uint64_t)&InterruptHandler(20)),
	IDT_GATE((uint64_t)&InterruptHandler(21)),
	IDT_GATE((uint64_t)&InterruptHandler(22)),
	IDT_GATE((uint64_t)&InterruptHandler(23)),
	IDT_GATE((uint64_t)&InterruptHandler(24)),
	IDT_GATE((uint64_t)&InterruptHandler(25)),
	IDT_GATE((uint64_t)&InterruptHandler(26)),
	IDT_GATE((uint64_t)&InterruptHandler(27)),
	IDT_GATE((uint64_t)&InterruptHandler(28)),
	IDT_GATE((uint64_t)&InterruptHandler(29)),
	IDT_GATE((uint64_t)&InterruptHandler(30)),
	IDT_GATE((uint64_t)&InterruptHandler(31)),
	IDT_GATE((uint64_t)&InterruptHandler(32)),
	IDT_GATE((uint64_t)&InterruptHandler(33)),
	IDT_GATE((uint64_t)&InterruptHandler(34)),
	IDT_GATE((uint64_t)&InterruptHandler(35)),
	IDT_GATE((uint64_t)&InterruptHandler(36)),
	IDT_GATE((uint64_t)&InterruptHandler(37)),
	IDT_GATE((uint64_t)&InterruptHandler(38)),
	IDT_GATE((uint64_t)&InterruptHandler(39)),
	IDT_GATE((uint64_t)&InterruptHandler(40)),
	IDT_GATE((uint64_t)&InterruptHandler(41)),
	IDT_GATE((uint64_t)&InterruptHandler(42)),
	IDT_GATE((uint64_t)&InterruptHandler(43)),
	IDT_GATE((uint64_t)&InterruptHandler(44)),
	IDT_GATE((uint64_t)&InterruptHandler(45)),
	IDT_GATE((uint64_t)&InterruptHandler(46)),
	IDT_GATE((uint64_t)&InterruptHandler(47)),
	IDT_GATE((uint64_t)&InterruptHandler(48)),
	IDT_GATE((uint64_t)&InterruptHandler(49)),
	IDT_GATE((uint64_t)&InterruptHandler(50)),
	IDT_GATE((uint64_t)&InterruptHandler(51)),
	IDT_GATE((uint64_t)&InterruptHandler(52)),
	IDT_GATE((uint64_t)&InterruptHandler(53)),
	IDT_GATE((uint64_t)&InterruptHandler(54)),
	IDT_GATE((uint64_t)&InterruptHandler(55)),
	IDT_GATE((uint64_t)&InterruptHandler(56)),
	IDT_GATE((uint64_t)&InterruptHandler(57)),
	IDT_GATE((uint64_t)&InterruptHandler(58)),
	IDT_GATE((uint64_t)&InterruptHandler(59)),
	IDT_GATE((uint64_t)&InterruptHandler(60)),
	IDT_GATE((uint64_t)&InterruptHandler(61)),
	IDT_GATE((uint64_t)&InterruptHandler(62)),
	IDT_GATE((uint64_t)&InterruptHandler(63)),
	IDT_GATE((uint64_t)&InterruptHandler(64)),
	IDT_GATE((uint64_t)&InterruptHandler(65)),
	IDT_GATE((uint64_t)&InterruptHandler(66)),
	IDT_GATE((uint64_t)&InterruptHandler(67)),
	IDT_GATE((uint64_t)&InterruptHandler(68)),
	IDT_GATE((uint64_t)&InterruptHandler(69)),
	IDT_GATE((uint64_t)&InterruptHandler(70)),
	IDT_GATE((uint64_t)&InterruptHandler(71)),
	IDT_GATE((uint64_t)&InterruptHandler(72)),
	IDT_GATE((uint64_t)&InterruptHandler(73)),
	IDT_GATE((uint64_t)&InterruptHandler(74)),
	IDT_GATE((uint64_t)&InterruptHandler(75)),
	IDT_GATE((uint64_t)&InterruptHandler(76)),
	IDT_GATE((uint64_t)&InterruptHandler(77)),
	IDT_GATE((uint64_t)&InterruptHandler(78)),
	IDT_GATE((uint64_t)&InterruptHandler(79)),
	IDT_GATE((uint64_t)&InterruptHandler(80)),
	IDT_GATE((uint64_t)&InterruptHandler(81)),
	IDT_GATE((uint64_t)&InterruptHandler(82)),
	IDT_GATE((uint64_t)&InterruptHandler(83)),
	IDT_GATE((uint64_t)&InterruptHandler(84)),
	IDT_GATE((uint64_t)&InterruptHandler(85)),
	IDT_GATE((uint64_t)&InterruptHandler(86)),
	IDT_GATE((uint64_t)&InterruptHandler(87)),
	IDT_GATE((uint64_t)&InterruptHandler(88)),
	IDT_GATE((uint64_t)&InterruptHandler(89)),
	IDT_GATE((uint64_t)&InterruptHandler(90)),
	IDT_GATE((uint64_t)&InterruptHandler(91)),
	IDT_GATE((uint64_t)&InterruptHandler(92)),
	IDT_GATE((uint64_t)&InterruptHandler(93)),
	IDT_GATE((uint64_t)&InterruptHandler(94)),
	IDT_GATE((uint64_t)&InterruptHandler(95)),
	IDT_GATE((uint64_t)&InterruptHandler(96)),
	IDT_GATE((uint64_t)&InterruptHandler(97)),
	IDT_GATE((uint64_t)&InterruptHandler(98)),
	IDT_GATE((uint64_t)&InterruptHandler(99)),
	IDT_GATE((uint64_t)&InterruptHandler(100)),
	IDT_GATE((uint64_t)&InterruptHandler(101)),
	IDT_GATE((uint64_t)&InterruptHandler(102)),
	IDT_GATE((uint64_t)&InterruptHandler(103)),
	IDT_GATE((uint64_t)&InterruptHandler(104)),
	IDT_GATE((uint64_t)&InterruptHandler(105)),
	IDT_GATE((uint64_t)&InterruptHandler(106)),
	IDT_GATE((uint64_t)&InterruptHandler(107)),
	IDT_GATE((uint64_t)&InterruptHandler(108)),
	IDT_GATE((uint64_t)&InterruptHandler(109)),
	IDT_GATE((uint64_t)&InterruptHandler(110)),
	IDT_GATE((uint64_t)&InterruptHandler(111)),
	IDT_GATE((uint64_t)&InterruptHandler(112)),
	IDT_GATE((uint64_t)&InterruptHandler(113)),
	IDT_GATE((uint64_t)&InterruptHandler(114)),
	IDT_GATE((uint64_t)&InterruptHandler(115)),
	IDT_GATE((uint64_t)&InterruptHandler(116)),
	IDT_GATE((uint64_t)&InterruptHandler(117)),
	IDT_GATE((uint64_t)&InterruptHandler(118)),
	IDT_GATE((uint64_t)&InterruptHandler(119)),
	IDT_GATE((uint64_t)&InterruptHandler(120)),
	IDT_GATE((uint64_t)&InterruptHandler(121)),
	IDT_GATE((uint64_t)&InterruptHandler(122)),
	IDT_GATE((uint64_t)&InterruptHandler(123)),
	IDT_GATE((uint64_t)&InterruptHandler(124)),
	IDT_GATE((uint64_t)&InterruptHandler(125)),
	IDT_GATE((uint64_t)&InterruptHandler(126)),
	IDT_GATE((uint64_t)&InterruptHandler(127)),
	IDT_GATE((uint64_t)&InterruptHandler(128)),
	IDT_GATE((uint64_t)&InterruptHandler(129)),
	IDT_GATE((uint64_t)&InterruptHandler(130)),
	IDT_GATE((uint64_t)&InterruptHandler(131)),
	IDT_GATE((uint64_t)&InterruptHandler(132)),
	IDT_GATE((uint64_t)&InterruptHandler(133)),
	IDT_GATE((uint64_t)&InterruptHandler(134)),
	IDT_GATE((uint64_t)&InterruptHandler(135)),
	IDT_GATE((uint64_t)&InterruptHandler(136)),
	IDT_GATE((uint64_t)&InterruptHandler(137)),
	IDT_GATE((uint64_t)&InterruptHandler(138)),
	IDT_GATE((uint64_t)&InterruptHandler(139)),
	IDT_GATE((uint64_t)&InterruptHandler(140)),
	IDT_GATE((uint64_t)&InterruptHandler(141)),
	IDT_GATE((uint64_t)&InterruptHandler(142)),
	IDT_GATE((uint64_t)&InterruptHandler(143)),
	IDT_GATE((uint64_t)&InterruptHandler(144)),
	IDT_GATE((uint64_t)&InterruptHandler(145)),
	IDT_GATE((uint64_t)&InterruptHandler(146)),
	IDT_GATE((uint64_t)&InterruptHandler(147)),
	IDT_GATE((uint64_t)&InterruptHandler(148)),
	IDT_GATE((uint64_t)&InterruptHandler(149)),
	IDT_GATE((uint64_t)&InterruptHandler(150)),
	IDT_GATE((uint64_t)&InterruptHandler(151)),
	IDT_GATE((uint64_t)&InterruptHandler(152)),
	IDT_GATE((uint64_t)&InterruptHandler(153)),
	IDT_GATE((uint64_t)&InterruptHandler(154)),
	IDT_GATE((uint64_t)&InterruptHandler(155)),
	IDT_GATE((uint64_t)&InterruptHandler(156)),
	IDT_GATE((uint64_t)&InterruptHandler(157)),
	IDT_GATE((uint64_t)&InterruptHandler(158)),
	IDT_GATE((uint64_t)&InterruptHandler(159)),
	IDT_GATE((uint64_t)&InterruptHandler(160)),
	IDT_GATE((uint64_t)&InterruptHandler(161)),
	IDT_GATE((uint64_t)&InterruptHandler(162)),
	IDT_GATE((uint64_t)&InterruptHandler(163)),
	IDT_GATE((uint64_t)&InterruptHandler(164)),
	IDT_GATE((uint64_t)&InterruptHandler(165)),
	IDT_GATE((uint64_t)&InterruptHandler(166)),
	IDT_GATE((uint64_t)&InterruptHandler(167)),
	IDT_GATE((uint64_t)&InterruptHandler(168)),
	IDT_GATE((uint64_t)&InterruptHandler(169)),
	IDT_GATE((uint64_t)&InterruptHandler(170)),
	IDT_GATE((uint64_t)&InterruptHandler(171)),
	IDT_GATE((uint64_t)&InterruptHandler(172)),
	IDT_GATE((uint64_t)&InterruptHandler(173)),
	IDT_GATE((uint64_t)&InterruptHandler(174)),
	IDT_GATE((uint64_t)&InterruptHandler(175)),
	IDT_GATE((uint64_t)&InterruptHandler(176)),
	IDT_GATE((uint64_t)&InterruptHandler(177)),
	IDT_GATE((uint64_t)&InterruptHandler(178)),
	IDT_GATE((uint64_t)&InterruptHandler(179)),
	IDT_GATE((uint64_t)&InterruptHandler(180)),
	IDT_GATE((uint64_t)&InterruptHandler(181)),
	IDT_GATE((uint64_t)&InterruptHandler(182)),
	IDT_GATE((uint64_t)&InterruptHandler(183)),
	IDT_GATE((uint64_t)&InterruptHandler(184)),
	IDT_GATE((uint64_t)&InterruptHandler(185)),
	IDT_GATE((uint64_t)&InterruptHandler(186)),
	IDT_GATE((uint64_t)&InterruptHandler(187)),
	IDT_GATE((uint64_t)&InterruptHandler(188)),
	IDT_GATE((uint64_t)&InterruptHandler(189)),
	IDT_GATE((uint64_t)&InterruptHandler(190)),
	IDT_GATE((uint64_t)&InterruptHandler(191)),
	IDT_GATE((uint64_t)&InterruptHandler(192)),
	IDT_GATE((uint64_t)&InterruptHandler(193)),
	IDT_GATE((uint64_t)&InterruptHandler(194)),
	IDT_GATE((uint64_t)&InterruptHandler(195)),
	IDT_GATE((uint64_t)&InterruptHandler(196)),
	IDT_GATE((uint64_t)&InterruptHandler(197)),
	IDT_GATE((uint64_t)&InterruptHandler(198)),
	IDT_GATE((uint64_t)&InterruptHandler(199)),
	IDT_GATE((uint64_t)&InterruptHandler(200)),
	IDT_GATE((uint64_t)&InterruptHandler(201)),
	IDT_GATE((uint64_t)&InterruptHandler(202)),
	IDT_GATE((uint64_t)&InterruptHandler(203)),
	IDT_GATE((uint64_t)&InterruptHandler(204)),
	IDT_GATE((uint64_t)&InterruptHandler(205)),
	IDT_GATE((uint64_t)&InterruptHandler(206)),
	IDT_GATE((uint64_t)&InterruptHandler(207)),
	IDT_GATE((uint64_t)&InterruptHandler(208)),
	IDT_GATE((uint64_t)&InterruptHandler(209)),
	IDT_GATE((uint64_t)&InterruptHandler(210)),
	IDT_GATE((uint64_t)&InterruptHandler(211)),
	IDT_GATE((uint64_t)&InterruptHandler(212)),
	IDT_GATE((uint64_t)&InterruptHandler(213)),
	IDT_GATE((uint64_t)&InterruptHandler(214)),
	IDT_GATE((uint64_t)&InterruptHandler(215)),
	IDT_GATE((uint64_t)&InterruptHandler(216)),
	IDT_GATE((uint64_t)&InterruptHandler(217)),
	IDT_GATE((uint64_t)&InterruptHandler(218)),
	IDT_GATE((uint64_t)&InterruptHandler(219)),
	IDT_GATE((uint64_t)&InterruptHandler(220)),
	IDT_GATE((uint64_t)&InterruptHandler(221)),
	IDT_GATE((uint64_t)&InterruptHandler(222)),
	IDT_GATE((uint64_t)&InterruptHandler(223)),
	IDT_GATE((uint64_t)&InterruptHandler(224)),
	IDT_GATE((uint64_t)&InterruptHandler(225)),
	IDT_GATE((uint64_t)&InterruptHandler(226)),
	IDT_GATE((uint64_t)&InterruptHandler(227)),
	IDT_GATE((uint64_t)&InterruptHandler(228)),
	IDT_GATE((uint64_t)&InterruptHandler(229)),
	IDT_GATE((uint64_t)&InterruptHandler(230)),
	IDT_GATE((uint64_t)&InterruptHandler(231)),
	IDT_GATE((uint64_t)&InterruptHandler(232)),
	IDT_GATE((uint64_t)&InterruptHandler(233)),
	IDT_GATE((uint64_t)&InterruptHandler(234)),
	IDT_GATE((uint64_t)&InterruptHandler(235)),
	IDT_GATE((uint64_t)&InterruptHandler(236)),
	IDT_GATE((uint64_t)&InterruptHandler(237)),
	IDT_GATE((uint64_t)&InterruptHandler(238)),
	IDT_GATE((uint64_t)&InterruptHandler(239)),
	IDT_GATE((uint64_t)&InterruptHandler(240)),
	IDT_GATE((uint64_t)&InterruptHandler(241)),
	IDT_GATE((uint64_t)&InterruptHandler(242)),
	IDT_GATE((uint64_t)&InterruptHandler(243)),
	IDT_GATE((uint64_t)&InterruptHandler(244)),
	IDT_GATE((uint64_t)&InterruptHandler(245)),
	IDT_GATE((uint64_t)&InterruptHandler(246)),
	IDT_GATE((uint64_t)&InterruptHandler(247)),
	IDT_GATE((uint64_t)&InterruptHandler(248)),
	IDT_GATE((uint64_t)&InterruptHandler(249)),
	IDT_GATE((uint64_t)&InterruptHandler(250)),
	IDT_GATE((uint64_t)&InterruptHandler(251)),
	IDT_GATE((uint64_t)&InterruptHandler(252)),
	IDT_GATE((uint64_t)&InterruptHandler(253)),
	IDT_GATE((uint64_t)&InterruptHandler(254)),
	IDT_GATE((uint64_t)&InterruptHandler(255))
};

x64::DESCRIPTOR_TABLE x64::GDTR =
{
	sizeof(KernelGDT) - 1,
	(uint64_t)&KernelGDT
};

x64::DESCRIPTOR_TABLE x64::IDTR =
{
	sizeof(IDT) - 1,
	(uint64_t)IDT
};
