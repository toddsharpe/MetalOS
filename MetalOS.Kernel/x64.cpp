#include "x64.h"

#include <intrin.h>
#include "x64_support.h"
#include "x64_interrupts.h"

void x64::Initialize()
{
	//Load new segments
	_lgdt(&GDTR);

	//Update segment registers
	SEGMENT_SELECTOR dataSelector(GDT_KERNEL_DATA);
	SEGMENT_SELECTOR codeSelector(GDT_KERNEL_CODE);
	x64_update_segments(dataSelector.Value, codeSelector.Value);
	SEGMENT_SELECTOR tssSelector(GDT_TSS_ENTRY);
	x64_ltr(tssSelector.Value);

	//Load interrupt handlers
	__lidt(&IDTR);

	//Enable interrupts
	x64_sti();
}

// "Known good stacks" Intel SDM Vol 3A 6.14.5
volatile uint8_t x64::DOUBLEFAULT_STACK[IST_STACK_SIZE] = { 0 };
volatile uint8_t x64::NMI_Stack[IST_STACK_SIZE] = { 0 };
volatile uint8_t x64::DEBUG_STACK[IST_STACK_SIZE] = { 0 };
volatile uint8_t x64::MCE_STACK[IST_STACK_SIZE] = { 0 };

//Kernel Structures
volatile x64::TASK_STATE_SEGMENT_64 x64::TSS64 =
{
	0, //Reserved
	0, 0, 0, 0, 0, 0, //RSP[0,3) low/high
	0, 0, //Reserved
	QWordLow(DOUBLEFAULT_STACK + IST_STACK_SIZE), QWordHigh(DOUBLEFAULT_STACK + IST_STACK_SIZE), //IST1
	QWordLow(NMI_Stack + IST_STACK_SIZE), QWordHigh(NMI_Stack + IST_STACK_SIZE), //IST2
	QWordLow(DEBUG_STACK + IST_STACK_SIZE), QWordHigh(DEBUG_STACK + IST_STACK_SIZE), //IST3
	QWordLow(MCE_STACK + IST_STACK_SIZE), QWordHigh(MCE_STACK + IST_STACK_SIZE), //IST4
	0, 0, //IST5
	0, 0, //IST6
	0, 0, //IST7
	0, 0, 0, //Reserved
	0 //IO Map Base
};

//1 empty, 4 GDTs, and 1 TSS
volatile x64::KERNEL_GDTS x64::KernelGDT =
{
	{ 0 }, //First entry has to be empty
	// Seg1   Base  type  S   DPL		   P   Seg2   OS      L     DB    4GB   Base
	{ 0xFFFF, 0, 0, 0xA, true, KernelDPL, true, 0xF, false, true, false, true, 0x00 }, //64-bit code Kernel
	{ 0xFFFF, 0, 0, 0x2, true, KernelDPL, true, 0xF, false, false, true, true, 0x00 }, //64-bit data Kernel
	{ 0xFFFF, 0, 0, 0xA, true, UserDPL,	  true, 0xF, false, true, false, true, 0x00 }, //64-bit code User
	{ 0xFFFF, 0, 0, 0x2, true, UserDPL,	  true, 0xF, false, false, true, true, 0x00 }, //64-bit data User
	{
		// Seg1				Base1			Base2							type  S    DPL  P   Seg2	OS      L   DB     4GB   Base3
		sizeof(x64::TSS64) - 1, (uint16_t)&x64::TSS64, (uint8_t)((uint64_t)&x64::TSS64 >> 16), 0x9, false, 0, true, 0, false, false, false, true, (uint8_t)((uint64_t)&x64::TSS64 >> 24),
		// Base4						Zeroes
		QWordHigh(&x64::TSS64), 0, 0, 0
	}
};

volatile x64::IDT_GATE x64::IDT[IDT_COUNT] =
{
	IDT_GATE((uint64_t)& ISR_HANDLER(0)),
	IDT_GATE((uint64_t)& ISR_HANDLER(1)),
	IDT_GATE((uint64_t)& ISR_HANDLER(2), IST_NMI_IDX, IDT_GATE_TYPE::InterruptGate32),
	IDT_GATE((uint64_t)& ISR_HANDLER(3), IST_DEBUG_IDX, IDT_GATE_TYPE::InterruptGate32),
	IDT_GATE((uint64_t)& ISR_HANDLER(4)),
	IDT_GATE((uint64_t)& ISR_HANDLER(5)),
	IDT_GATE((uint64_t)& ISR_HANDLER(6)),
	IDT_GATE((uint64_t)& ISR_HANDLER(7)),
	IDT_GATE((uint64_t)& ISR_HANDLER(8), IST_DOUBLEFAULT_IDX, IDT_GATE_TYPE::InterruptGate32),
	IDT_GATE((uint64_t)& ISR_HANDLER(9)),
	IDT_GATE((uint64_t)& ISR_HANDLER(10)),
	IDT_GATE((uint64_t)& ISR_HANDLER(11)),
	IDT_GATE((uint64_t)& ISR_HANDLER(12)),
	IDT_GATE((uint64_t)& ISR_HANDLER(13)),
	IDT_GATE((uint64_t)& ISR_HANDLER(14)),
	IDT_GATE((uint64_t)& ISR_HANDLER(15)),
	IDT_GATE((uint64_t)& ISR_HANDLER(16)),
	IDT_GATE((uint64_t)& ISR_HANDLER(17)),
	IDT_GATE((uint64_t)& ISR_HANDLER(18), IST_MCE_IDX, IDT_GATE_TYPE::InterruptGate32),
	IDT_GATE((uint64_t)& ISR_HANDLER(19)),
	IDT_GATE((uint64_t)& ISR_HANDLER(20)),
	IDT_GATE((uint64_t)& ISR_HANDLER(21)),
	IDT_GATE((uint64_t)& ISR_HANDLER(22)),
	IDT_GATE((uint64_t)& ISR_HANDLER(23)),
	IDT_GATE((uint64_t)& ISR_HANDLER(24)),
	IDT_GATE((uint64_t)& ISR_HANDLER(25)),
	IDT_GATE((uint64_t)& ISR_HANDLER(26)),
	IDT_GATE((uint64_t)& ISR_HANDLER(27)),
	IDT_GATE((uint64_t)& ISR_HANDLER(28)),
	IDT_GATE((uint64_t)& ISR_HANDLER(29)),
	IDT_GATE((uint64_t)& ISR_HANDLER(30)),
	IDT_GATE((uint64_t)& ISR_HANDLER(31)),
	IDT_GATE((uint64_t)& ISR_HANDLER(32)),
	IDT_GATE((uint64_t)& ISR_HANDLER(33)),
	IDT_GATE((uint64_t)& ISR_HANDLER(34)),
	IDT_GATE((uint64_t)& ISR_HANDLER(35)),
	IDT_GATE((uint64_t)& ISR_HANDLER(36)),
	IDT_GATE((uint64_t)& ISR_HANDLER(37)),
	IDT_GATE((uint64_t)& ISR_HANDLER(38)),
	IDT_GATE((uint64_t)& ISR_HANDLER(39)),
	IDT_GATE((uint64_t)& ISR_HANDLER(40)),
	IDT_GATE((uint64_t)& ISR_HANDLER(41)),
	IDT_GATE((uint64_t)& ISR_HANDLER(42)),
	IDT_GATE((uint64_t)& ISR_HANDLER(43)),
	IDT_GATE((uint64_t)& ISR_HANDLER(44)),
	IDT_GATE((uint64_t)& ISR_HANDLER(45)),
	IDT_GATE((uint64_t)& ISR_HANDLER(46)),
	IDT_GATE((uint64_t)& ISR_HANDLER(47)),
	IDT_GATE((uint64_t)& ISR_HANDLER(48)),
	IDT_GATE((uint64_t)& ISR_HANDLER(49)),
	IDT_GATE((uint64_t)& ISR_HANDLER(50)),
	IDT_GATE((uint64_t)& ISR_HANDLER(51)),
	IDT_GATE((uint64_t)& ISR_HANDLER(52)),
	IDT_GATE((uint64_t)& ISR_HANDLER(53)),
	IDT_GATE((uint64_t)& ISR_HANDLER(54)),
	IDT_GATE((uint64_t)& ISR_HANDLER(55)),
	IDT_GATE((uint64_t)& ISR_HANDLER(56)),
	IDT_GATE((uint64_t)& ISR_HANDLER(57)),
	IDT_GATE((uint64_t)& ISR_HANDLER(58)),
	IDT_GATE((uint64_t)& ISR_HANDLER(59)),
	IDT_GATE((uint64_t)& ISR_HANDLER(60)),
	IDT_GATE((uint64_t)& ISR_HANDLER(61)),
	IDT_GATE((uint64_t)& ISR_HANDLER(62)),
	IDT_GATE((uint64_t)& ISR_HANDLER(63)),
	IDT_GATE((uint64_t)& ISR_HANDLER(64)),
	IDT_GATE((uint64_t)& ISR_HANDLER(65)),
	IDT_GATE((uint64_t)& ISR_HANDLER(66)),
	IDT_GATE((uint64_t)& ISR_HANDLER(67)),
	IDT_GATE((uint64_t)& ISR_HANDLER(68)),
	IDT_GATE((uint64_t)& ISR_HANDLER(69)),
	IDT_GATE((uint64_t)& ISR_HANDLER(70)),
	IDT_GATE((uint64_t)& ISR_HANDLER(71)),
	IDT_GATE((uint64_t)& ISR_HANDLER(72)),
	IDT_GATE((uint64_t)& ISR_HANDLER(73)),
	IDT_GATE((uint64_t)& ISR_HANDLER(74)),
	IDT_GATE((uint64_t)& ISR_HANDLER(75)),
	IDT_GATE((uint64_t)& ISR_HANDLER(76)),
	IDT_GATE((uint64_t)& ISR_HANDLER(77)),
	IDT_GATE((uint64_t)& ISR_HANDLER(78)),
	IDT_GATE((uint64_t)& ISR_HANDLER(79)),
	IDT_GATE((uint64_t)& ISR_HANDLER(80)),
	IDT_GATE((uint64_t)& ISR_HANDLER(81)),
	IDT_GATE((uint64_t)& ISR_HANDLER(82)),
	IDT_GATE((uint64_t)& ISR_HANDLER(83)),
	IDT_GATE((uint64_t)& ISR_HANDLER(84)),
	IDT_GATE((uint64_t)& ISR_HANDLER(85)),
	IDT_GATE((uint64_t)& ISR_HANDLER(86)),
	IDT_GATE((uint64_t)& ISR_HANDLER(87)),
	IDT_GATE((uint64_t)& ISR_HANDLER(88)),
	IDT_GATE((uint64_t)& ISR_HANDLER(89)),
	IDT_GATE((uint64_t)& ISR_HANDLER(90)),
	IDT_GATE((uint64_t)& ISR_HANDLER(91)),
	IDT_GATE((uint64_t)& ISR_HANDLER(92)),
	IDT_GATE((uint64_t)& ISR_HANDLER(93)),
	IDT_GATE((uint64_t)& ISR_HANDLER(94)),
	IDT_GATE((uint64_t)& ISR_HANDLER(95)),
	IDT_GATE((uint64_t)& ISR_HANDLER(96)),
	IDT_GATE((uint64_t)& ISR_HANDLER(97)),
	IDT_GATE((uint64_t)& ISR_HANDLER(98)),
	IDT_GATE((uint64_t)& ISR_HANDLER(99)),
	IDT_GATE((uint64_t)& ISR_HANDLER(100)),
	IDT_GATE((uint64_t)& ISR_HANDLER(101)),
	IDT_GATE((uint64_t)& ISR_HANDLER(102)),
	IDT_GATE((uint64_t)& ISR_HANDLER(103)),
	IDT_GATE((uint64_t)& ISR_HANDLER(104)),
	IDT_GATE((uint64_t)& ISR_HANDLER(105)),
	IDT_GATE((uint64_t)& ISR_HANDLER(106)),
	IDT_GATE((uint64_t)& ISR_HANDLER(107)),
	IDT_GATE((uint64_t)& ISR_HANDLER(108)),
	IDT_GATE((uint64_t)& ISR_HANDLER(109)),
	IDT_GATE((uint64_t)& ISR_HANDLER(110)),
	IDT_GATE((uint64_t)& ISR_HANDLER(111)),
	IDT_GATE((uint64_t)& ISR_HANDLER(112)),
	IDT_GATE((uint64_t)& ISR_HANDLER(113)),
	IDT_GATE((uint64_t)& ISR_HANDLER(114)),
	IDT_GATE((uint64_t)& ISR_HANDLER(115)),
	IDT_GATE((uint64_t)& ISR_HANDLER(116)),
	IDT_GATE((uint64_t)& ISR_HANDLER(117)),
	IDT_GATE((uint64_t)& ISR_HANDLER(118)),
	IDT_GATE((uint64_t)& ISR_HANDLER(119)),
	IDT_GATE((uint64_t)& ISR_HANDLER(120)),
	IDT_GATE((uint64_t)& ISR_HANDLER(121)),
	IDT_GATE((uint64_t)& ISR_HANDLER(122)),
	IDT_GATE((uint64_t)& ISR_HANDLER(123)),
	IDT_GATE((uint64_t)& ISR_HANDLER(124)),
	IDT_GATE((uint64_t)& ISR_HANDLER(125)),
	IDT_GATE((uint64_t)& ISR_HANDLER(126)),
	IDT_GATE((uint64_t)& ISR_HANDLER(127)),
	IDT_GATE((uint64_t)& ISR_HANDLER(128)),
	IDT_GATE((uint64_t)& ISR_HANDLER(129)),
	IDT_GATE((uint64_t)& ISR_HANDLER(130)),
	IDT_GATE((uint64_t)& ISR_HANDLER(131)),
	IDT_GATE((uint64_t)& ISR_HANDLER(132)),
	IDT_GATE((uint64_t)& ISR_HANDLER(133)),
	IDT_GATE((uint64_t)& ISR_HANDLER(134)),
	IDT_GATE((uint64_t)& ISR_HANDLER(135)),
	IDT_GATE((uint64_t)& ISR_HANDLER(136)),
	IDT_GATE((uint64_t)& ISR_HANDLER(137)),
	IDT_GATE((uint64_t)& ISR_HANDLER(138)),
	IDT_GATE((uint64_t)& ISR_HANDLER(139)),
	IDT_GATE((uint64_t)& ISR_HANDLER(140)),
	IDT_GATE((uint64_t)& ISR_HANDLER(141)),
	IDT_GATE((uint64_t)& ISR_HANDLER(142)),
	IDT_GATE((uint64_t)& ISR_HANDLER(143)),
	IDT_GATE((uint64_t)& ISR_HANDLER(144)),
	IDT_GATE((uint64_t)& ISR_HANDLER(145)),
	IDT_GATE((uint64_t)& ISR_HANDLER(146)),
	IDT_GATE((uint64_t)& ISR_HANDLER(147)),
	IDT_GATE((uint64_t)& ISR_HANDLER(148)),
	IDT_GATE((uint64_t)& ISR_HANDLER(149)),
	IDT_GATE((uint64_t)& ISR_HANDLER(150)),
	IDT_GATE((uint64_t)& ISR_HANDLER(151)),
	IDT_GATE((uint64_t)& ISR_HANDLER(152)),
	IDT_GATE((uint64_t)& ISR_HANDLER(153)),
	IDT_GATE((uint64_t)& ISR_HANDLER(154)),
	IDT_GATE((uint64_t)& ISR_HANDLER(155)),
	IDT_GATE((uint64_t)& ISR_HANDLER(156)),
	IDT_GATE((uint64_t)& ISR_HANDLER(157)),
	IDT_GATE((uint64_t)& ISR_HANDLER(158)),
	IDT_GATE((uint64_t)& ISR_HANDLER(159)),
	IDT_GATE((uint64_t)& ISR_HANDLER(160)),
	IDT_GATE((uint64_t)& ISR_HANDLER(161)),
	IDT_GATE((uint64_t)& ISR_HANDLER(162)),
	IDT_GATE((uint64_t)& ISR_HANDLER(163)),
	IDT_GATE((uint64_t)& ISR_HANDLER(164)),
	IDT_GATE((uint64_t)& ISR_HANDLER(165)),
	IDT_GATE((uint64_t)& ISR_HANDLER(166)),
	IDT_GATE((uint64_t)& ISR_HANDLER(167)),
	IDT_GATE((uint64_t)& ISR_HANDLER(168)),
	IDT_GATE((uint64_t)& ISR_HANDLER(169)),
	IDT_GATE((uint64_t)& ISR_HANDLER(170)),
	IDT_GATE((uint64_t)& ISR_HANDLER(171)),
	IDT_GATE((uint64_t)& ISR_HANDLER(172)),
	IDT_GATE((uint64_t)& ISR_HANDLER(173)),
	IDT_GATE((uint64_t)& ISR_HANDLER(174)),
	IDT_GATE((uint64_t)& ISR_HANDLER(175)),
	IDT_GATE((uint64_t)& ISR_HANDLER(176)),
	IDT_GATE((uint64_t)& ISR_HANDLER(177)),
	IDT_GATE((uint64_t)& ISR_HANDLER(178)),
	IDT_GATE((uint64_t)& ISR_HANDLER(179)),
	IDT_GATE((uint64_t)& ISR_HANDLER(180)),
	IDT_GATE((uint64_t)& ISR_HANDLER(181)),
	IDT_GATE((uint64_t)& ISR_HANDLER(182)),
	IDT_GATE((uint64_t)& ISR_HANDLER(183)),
	IDT_GATE((uint64_t)& ISR_HANDLER(184)),
	IDT_GATE((uint64_t)& ISR_HANDLER(185)),
	IDT_GATE((uint64_t)& ISR_HANDLER(186)),
	IDT_GATE((uint64_t)& ISR_HANDLER(187)),
	IDT_GATE((uint64_t)& ISR_HANDLER(188)),
	IDT_GATE((uint64_t)& ISR_HANDLER(189)),
	IDT_GATE((uint64_t)& ISR_HANDLER(190)),
	IDT_GATE((uint64_t)& ISR_HANDLER(191)),
	IDT_GATE((uint64_t)& ISR_HANDLER(192)),
	IDT_GATE((uint64_t)& ISR_HANDLER(193)),
	IDT_GATE((uint64_t)& ISR_HANDLER(194)),
	IDT_GATE((uint64_t)& ISR_HANDLER(195)),
	IDT_GATE((uint64_t)& ISR_HANDLER(196)),
	IDT_GATE((uint64_t)& ISR_HANDLER(197)),
	IDT_GATE((uint64_t)& ISR_HANDLER(198)),
	IDT_GATE((uint64_t)& ISR_HANDLER(199)),
	IDT_GATE((uint64_t)& ISR_HANDLER(200)),
	IDT_GATE((uint64_t)& ISR_HANDLER(201)),
	IDT_GATE((uint64_t)& ISR_HANDLER(202)),
	IDT_GATE((uint64_t)& ISR_HANDLER(203)),
	IDT_GATE((uint64_t)& ISR_HANDLER(204)),
	IDT_GATE((uint64_t)& ISR_HANDLER(205)),
	IDT_GATE((uint64_t)& ISR_HANDLER(206)),
	IDT_GATE((uint64_t)& ISR_HANDLER(207)),
	IDT_GATE((uint64_t)& ISR_HANDLER(208)),
	IDT_GATE((uint64_t)& ISR_HANDLER(209)),
	IDT_GATE((uint64_t)& ISR_HANDLER(210)),
	IDT_GATE((uint64_t)& ISR_HANDLER(211)),
	IDT_GATE((uint64_t)& ISR_HANDLER(212)),
	IDT_GATE((uint64_t)& ISR_HANDLER(213)),
	IDT_GATE((uint64_t)& ISR_HANDLER(214)),
	IDT_GATE((uint64_t)& ISR_HANDLER(215)),
	IDT_GATE((uint64_t)& ISR_HANDLER(216)),
	IDT_GATE((uint64_t)& ISR_HANDLER(217)),
	IDT_GATE((uint64_t)& ISR_HANDLER(218)),
	IDT_GATE((uint64_t)& ISR_HANDLER(219)),
	IDT_GATE((uint64_t)& ISR_HANDLER(220)),
	IDT_GATE((uint64_t)& ISR_HANDLER(221)),
	IDT_GATE((uint64_t)& ISR_HANDLER(222)),
	IDT_GATE((uint64_t)& ISR_HANDLER(223)),
	IDT_GATE((uint64_t)& ISR_HANDLER(224)),
	IDT_GATE((uint64_t)& ISR_HANDLER(225)),
	IDT_GATE((uint64_t)& ISR_HANDLER(226)),
	IDT_GATE((uint64_t)& ISR_HANDLER(227)),
	IDT_GATE((uint64_t)& ISR_HANDLER(228)),
	IDT_GATE((uint64_t)& ISR_HANDLER(229)),
	IDT_GATE((uint64_t)& ISR_HANDLER(230)),
	IDT_GATE((uint64_t)& ISR_HANDLER(231)),
	IDT_GATE((uint64_t)& ISR_HANDLER(232)),
	IDT_GATE((uint64_t)& ISR_HANDLER(233)),
	IDT_GATE((uint64_t)& ISR_HANDLER(234)),
	IDT_GATE((uint64_t)& ISR_HANDLER(235)),
	IDT_GATE((uint64_t)& ISR_HANDLER(236)),
	IDT_GATE((uint64_t)& ISR_HANDLER(237)),
	IDT_GATE((uint64_t)& ISR_HANDLER(238)),
	IDT_GATE((uint64_t)& ISR_HANDLER(239)),
	IDT_GATE((uint64_t)& ISR_HANDLER(240)),
	IDT_GATE((uint64_t)& ISR_HANDLER(241)),
	IDT_GATE((uint64_t)& ISR_HANDLER(242)),
	IDT_GATE((uint64_t)& ISR_HANDLER(243)),
	IDT_GATE((uint64_t)& ISR_HANDLER(244)),
	IDT_GATE((uint64_t)& ISR_HANDLER(245)),
	IDT_GATE((uint64_t)& ISR_HANDLER(246)),
	IDT_GATE((uint64_t)& ISR_HANDLER(247)),
	IDT_GATE((uint64_t)& ISR_HANDLER(248)),
	IDT_GATE((uint64_t)& ISR_HANDLER(249)),
	IDT_GATE((uint64_t)& ISR_HANDLER(250)),
	IDT_GATE((uint64_t)& ISR_HANDLER(251)),
	IDT_GATE((uint64_t)& ISR_HANDLER(252)),
	IDT_GATE((uint64_t)& ISR_HANDLER(253)),
	IDT_GATE((uint64_t)& ISR_HANDLER(254)),
	IDT_GATE((uint64_t)& ISR_HANDLER(255))
};

x64::DESCRIPTOR_TABLE x64::GDTR =
{
	sizeof(KernelGDT) - 1,
	(uint64_t)& KernelGDT
};

x64::DESCRIPTOR_TABLE x64::IDTR =
{
	sizeof(IDT) - 1,
	(uint64_t)IDT
};

uint32_t x64_read_port(uint32_t port, uint8_t width)
{
	switch (width)
	{
		case 8:
			return __inbyte(port);
		case 16:
			return __inword(port);
		case 32:
			return __indword(port);
		default:
			return 0;
	}
}

void x64_write_port(uint32_t port, uint32_t value, uint8_t width)
{
	switch (width)
	{
		case 8:
			return __outbyte(port, value);
		case 16:
			return __outword(port, value);
		case 32:
			return __outdword(port, value);
	}
}
