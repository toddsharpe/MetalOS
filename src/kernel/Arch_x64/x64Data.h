#pragma once

#include "core_crt/stdint.h"
#include "x64/x64.h"
#include "x64/Types.h"
#include "x64/IdtGate.h"
#include "Interrupts.h"

namespace x64
{
	static constexpr size_t IdtCount = 256;
	static constexpr size_t IstStackSize = (1 << 12);//4k Stack

	static constexpr size_t IST_DOUBLEFAULT_IDX = 1;
	static constexpr size_t IST_NMI_IDX = 2;
	static constexpr size_t IST_DEBUG_IDX = 3;
	static constexpr size_t IST_MCE_IDX = 4;

	// "Known good stacks" Intel SDM Vol 3A 6.14.5
	volatile uint8_t DOUBLEFAULT_STACK[IstStackSize] = { 0 };
	volatile uint8_t NMI_Stack[IstStackSize] = { 0 };
	volatile uint8_t DEBUG_STACK[IstStackSize] = { 0 };
	volatile uint8_t MCE_STACK[IstStackSize] = { 0 };

	//Kernel Structures
	volatile TASK_STATE_SEGMENT_64 TSS64 =
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

	//1 empty, 4 GDTs, and 1 TSS
	volatile KERNEL_GDTS KernelGDT =
	{
		{ 0 }, //First entry has to be empty
		// Seg1   Base  type		   S    DPL		   P    Seg2   OS      L     DB   4GB   Base
		{ 0xFFFF, 0, 0, GDT_TYPE_RWX, true, KernelDpl, true, 0xF, false, true, false, true, 0x00 }, //64-bit code Kernel
		{ 0xFFFF, 0, 0, GDT_TYPE_RW,  true, KernelDpl, true, 0xF, false, false, true, true, 0x00 }, //64-bit data Kernel
		{ 0xFFFF, 0, 0, GDT_TYPE_RWX, true, UserDpl,   true, 0xF, false, false, true, true, 0x00 }, //32-bit code User
		{ 0xFFFF, 0, 0, GDT_TYPE_RW,  true, UserDpl,   true, 0xF, false, false, true, true, 0x00 }, //64-bit data User
		{ 0xFFFF, 0, 0, GDT_TYPE_RWX, true, UserDpl,   true, 0xF, false, true, false, true, 0x00 }, //64-bit code User
		{
			// Seg1				Base1			Base2							type  S    DPL  P   Seg2	OS      L   DB     4GB   Base3
			sizeof(TSS64) - 1, (uint16_t)&TSS64, (uint8_t)((uint64_t)&TSS64 >> 16), 0x9, false, 0, true, 0, false, false, false, true, (uint8_t)((uint64_t)&TSS64 >> 24),
			// Base4						Zeroes
			QWordHigh(&TSS64), 0, 0, 0
		}
	};

	//cs = starhigh + 2
	//ss = starhigh + 1

	volatile IdtGate IDT[IdtCount] =
	{
		IdtGate((uint64_t)&x64_interrupt_handler_0),
		IdtGate((uint64_t)&x64_interrupt_handler_1),
		IdtGate((uint64_t)&x64_interrupt_handler_2, IST_NMI_IDX),
		IdtGate((uint64_t)&x64_interrupt_handler_3, IST_DEBUG_IDX, UserDpl),
		IdtGate((uint64_t)&x64_interrupt_handler_4),
		IdtGate((uint64_t)&x64_interrupt_handler_5),
		IdtGate((uint64_t)&x64_interrupt_handler_6),
		IdtGate((uint64_t)&x64_interrupt_handler_7),
		IdtGate((uint64_t)&x64_interrupt_handler_8, IST_DOUBLEFAULT_IDX),
		IdtGate((uint64_t)&x64_interrupt_handler_9),
		IdtGate((uint64_t)&x64_interrupt_handler_10),
		IdtGate((uint64_t)&x64_interrupt_handler_11),
		IdtGate((uint64_t)&x64_interrupt_handler_12),
		IdtGate((uint64_t)&x64_interrupt_handler_13),
		IdtGate((uint64_t)&x64_interrupt_handler_14),
		IdtGate((uint64_t)&x64_interrupt_handler_15),
		IdtGate((uint64_t)&x64_interrupt_handler_16),
		IdtGate((uint64_t)&x64_interrupt_handler_17),
		IdtGate((uint64_t)&x64_interrupt_handler_18, IST_MCE_IDX),
		IdtGate((uint64_t)&x64_interrupt_handler_19),
		IdtGate((uint64_t)&x64_interrupt_handler_20),
		IdtGate((uint64_t)&x64_interrupt_handler_21),
		IdtGate((uint64_t)&x64_interrupt_handler_22),
		IdtGate((uint64_t)&x64_interrupt_handler_23),
		IdtGate((uint64_t)&x64_interrupt_handler_24),
		IdtGate((uint64_t)&x64_interrupt_handler_25),
		IdtGate((uint64_t)&x64_interrupt_handler_26),
		IdtGate((uint64_t)&x64_interrupt_handler_27),
		IdtGate((uint64_t)&x64_interrupt_handler_28),
		IdtGate((uint64_t)&x64_interrupt_handler_29),
		IdtGate((uint64_t)&x64_interrupt_handler_30),
		IdtGate((uint64_t)&x64_interrupt_handler_31),
		IdtGate((uint64_t)&x64_interrupt_handler_32),
		IdtGate((uint64_t)&x64_interrupt_handler_33),
		IdtGate((uint64_t)&x64_interrupt_handler_34),
		IdtGate((uint64_t)&x64_interrupt_handler_35),
		IdtGate((uint64_t)&x64_interrupt_handler_36),
		IdtGate((uint64_t)&x64_interrupt_handler_37),
		IdtGate((uint64_t)&x64_interrupt_handler_38),
		IdtGate((uint64_t)&x64_interrupt_handler_39),
		IdtGate((uint64_t)&x64_interrupt_handler_40),
		IdtGate((uint64_t)&x64_interrupt_handler_41),
		IdtGate((uint64_t)&x64_interrupt_handler_42),
		IdtGate((uint64_t)&x64_interrupt_handler_43),
		IdtGate((uint64_t)&x64_interrupt_handler_44),
		IdtGate((uint64_t)&x64_interrupt_handler_45),
		IdtGate((uint64_t)&x64_interrupt_handler_46),
		IdtGate((uint64_t)&x64_interrupt_handler_47),
		IdtGate((uint64_t)&x64_interrupt_handler_48),
		IdtGate((uint64_t)&x64_interrupt_handler_49),
		IdtGate((uint64_t)&x64_interrupt_handler_50),
		IdtGate((uint64_t)&x64_interrupt_handler_51),
		IdtGate((uint64_t)&x64_interrupt_handler_52),
		IdtGate((uint64_t)&x64_interrupt_handler_53),
		IdtGate((uint64_t)&x64_interrupt_handler_54),
		IdtGate((uint64_t)&x64_interrupt_handler_55),
		IdtGate((uint64_t)&x64_interrupt_handler_56),
		IdtGate((uint64_t)&x64_interrupt_handler_57),
		IdtGate((uint64_t)&x64_interrupt_handler_58),
		IdtGate((uint64_t)&x64_interrupt_handler_59),
		IdtGate((uint64_t)&x64_interrupt_handler_60),
		IdtGate((uint64_t)&x64_interrupt_handler_61),
		IdtGate((uint64_t)&x64_interrupt_handler_62),
		IdtGate((uint64_t)&x64_interrupt_handler_63),
		IdtGate((uint64_t)&x64_interrupt_handler_64),
		IdtGate((uint64_t)&x64_interrupt_handler_65),
		IdtGate((uint64_t)&x64_interrupt_handler_66),
		IdtGate((uint64_t)&x64_interrupt_handler_67),
		IdtGate((uint64_t)&x64_interrupt_handler_68),
		IdtGate((uint64_t)&x64_interrupt_handler_69),
		IdtGate((uint64_t)&x64_interrupt_handler_70),
		IdtGate((uint64_t)&x64_interrupt_handler_71),
		IdtGate((uint64_t)&x64_interrupt_handler_72),
		IdtGate((uint64_t)&x64_interrupt_handler_73),
		IdtGate((uint64_t)&x64_interrupt_handler_74),
		IdtGate((uint64_t)&x64_interrupt_handler_75),
		IdtGate((uint64_t)&x64_interrupt_handler_76),
		IdtGate((uint64_t)&x64_interrupt_handler_77),
		IdtGate((uint64_t)&x64_interrupt_handler_78),
		IdtGate((uint64_t)&x64_interrupt_handler_79),
		IdtGate((uint64_t)&x64_interrupt_handler_80),
		IdtGate((uint64_t)&x64_interrupt_handler_81),
		IdtGate((uint64_t)&x64_interrupt_handler_82),
		IdtGate((uint64_t)&x64_interrupt_handler_83),
		IdtGate((uint64_t)&x64_interrupt_handler_84),
		IdtGate((uint64_t)&x64_interrupt_handler_85),
		IdtGate((uint64_t)&x64_interrupt_handler_86),
		IdtGate((uint64_t)&x64_interrupt_handler_87),
		IdtGate((uint64_t)&x64_interrupt_handler_88),
		IdtGate((uint64_t)&x64_interrupt_handler_89),
		IdtGate((uint64_t)&x64_interrupt_handler_90),
		IdtGate((uint64_t)&x64_interrupt_handler_91),
		IdtGate((uint64_t)&x64_interrupt_handler_92),
		IdtGate((uint64_t)&x64_interrupt_handler_93),
		IdtGate((uint64_t)&x64_interrupt_handler_94),
		IdtGate((uint64_t)&x64_interrupt_handler_95),
		IdtGate((uint64_t)&x64_interrupt_handler_96),
		IdtGate((uint64_t)&x64_interrupt_handler_97),
		IdtGate((uint64_t)&x64_interrupt_handler_98),
		IdtGate((uint64_t)&x64_interrupt_handler_99),
		IdtGate((uint64_t)&x64_interrupt_handler_100),
		IdtGate((uint64_t)&x64_interrupt_handler_101),
		IdtGate((uint64_t)&x64_interrupt_handler_102),
		IdtGate((uint64_t)&x64_interrupt_handler_103),
		IdtGate((uint64_t)&x64_interrupt_handler_104),
		IdtGate((uint64_t)&x64_interrupt_handler_105),
		IdtGate((uint64_t)&x64_interrupt_handler_106),
		IdtGate((uint64_t)&x64_interrupt_handler_107),
		IdtGate((uint64_t)&x64_interrupt_handler_108),
		IdtGate((uint64_t)&x64_interrupt_handler_109),
		IdtGate((uint64_t)&x64_interrupt_handler_110),
		IdtGate((uint64_t)&x64_interrupt_handler_111),
		IdtGate((uint64_t)&x64_interrupt_handler_112),
		IdtGate((uint64_t)&x64_interrupt_handler_113),
		IdtGate((uint64_t)&x64_interrupt_handler_114),
		IdtGate((uint64_t)&x64_interrupt_handler_115),
		IdtGate((uint64_t)&x64_interrupt_handler_116),
		IdtGate((uint64_t)&x64_interrupt_handler_117),
		IdtGate((uint64_t)&x64_interrupt_handler_118),
		IdtGate((uint64_t)&x64_interrupt_handler_119),
		IdtGate((uint64_t)&x64_interrupt_handler_120),
		IdtGate((uint64_t)&x64_interrupt_handler_121),
		IdtGate((uint64_t)&x64_interrupt_handler_122),
		IdtGate((uint64_t)&x64_interrupt_handler_123),
		IdtGate((uint64_t)&x64_interrupt_handler_124),
		IdtGate((uint64_t)&x64_interrupt_handler_125),
		IdtGate((uint64_t)&x64_interrupt_handler_126),
		IdtGate((uint64_t)&x64_interrupt_handler_127),
		IdtGate((uint64_t)&x64_interrupt_handler_128),
		IdtGate((uint64_t)&x64_interrupt_handler_129),
		IdtGate((uint64_t)&x64_interrupt_handler_130),
		IdtGate((uint64_t)&x64_interrupt_handler_131),
		IdtGate((uint64_t)&x64_interrupt_handler_132),
		IdtGate((uint64_t)&x64_interrupt_handler_133),
		IdtGate((uint64_t)&x64_interrupt_handler_134),
		IdtGate((uint64_t)&x64_interrupt_handler_135),
		IdtGate((uint64_t)&x64_interrupt_handler_136),
		IdtGate((uint64_t)&x64_interrupt_handler_137),
		IdtGate((uint64_t)&x64_interrupt_handler_138),
		IdtGate((uint64_t)&x64_interrupt_handler_139),
		IdtGate((uint64_t)&x64_interrupt_handler_140),
		IdtGate((uint64_t)&x64_interrupt_handler_141),
		IdtGate((uint64_t)&x64_interrupt_handler_142),
		IdtGate((uint64_t)&x64_interrupt_handler_143),
		IdtGate((uint64_t)&x64_interrupt_handler_144),
		IdtGate((uint64_t)&x64_interrupt_handler_145),
		IdtGate((uint64_t)&x64_interrupt_handler_146),
		IdtGate((uint64_t)&x64_interrupt_handler_147),
		IdtGate((uint64_t)&x64_interrupt_handler_148),
		IdtGate((uint64_t)&x64_interrupt_handler_149),
		IdtGate((uint64_t)&x64_interrupt_handler_150),
		IdtGate((uint64_t)&x64_interrupt_handler_151),
		IdtGate((uint64_t)&x64_interrupt_handler_152),
		IdtGate((uint64_t)&x64_interrupt_handler_153),
		IdtGate((uint64_t)&x64_interrupt_handler_154),
		IdtGate((uint64_t)&x64_interrupt_handler_155),
		IdtGate((uint64_t)&x64_interrupt_handler_156),
		IdtGate((uint64_t)&x64_interrupt_handler_157),
		IdtGate((uint64_t)&x64_interrupt_handler_158),
		IdtGate((uint64_t)&x64_interrupt_handler_159),
		IdtGate((uint64_t)&x64_interrupt_handler_160),
		IdtGate((uint64_t)&x64_interrupt_handler_161),
		IdtGate((uint64_t)&x64_interrupt_handler_162),
		IdtGate((uint64_t)&x64_interrupt_handler_163),
		IdtGate((uint64_t)&x64_interrupt_handler_164),
		IdtGate((uint64_t)&x64_interrupt_handler_165),
		IdtGate((uint64_t)&x64_interrupt_handler_166),
		IdtGate((uint64_t)&x64_interrupt_handler_167),
		IdtGate((uint64_t)&x64_interrupt_handler_168),
		IdtGate((uint64_t)&x64_interrupt_handler_169),
		IdtGate((uint64_t)&x64_interrupt_handler_170),
		IdtGate((uint64_t)&x64_interrupt_handler_171),
		IdtGate((uint64_t)&x64_interrupt_handler_172),
		IdtGate((uint64_t)&x64_interrupt_handler_173),
		IdtGate((uint64_t)&x64_interrupt_handler_174),
		IdtGate((uint64_t)&x64_interrupt_handler_175),
		IdtGate((uint64_t)&x64_interrupt_handler_176),
		IdtGate((uint64_t)&x64_interrupt_handler_177),
		IdtGate((uint64_t)&x64_interrupt_handler_178),
		IdtGate((uint64_t)&x64_interrupt_handler_179),
		IdtGate((uint64_t)&x64_interrupt_handler_180),
		IdtGate((uint64_t)&x64_interrupt_handler_181),
		IdtGate((uint64_t)&x64_interrupt_handler_182),
		IdtGate((uint64_t)&x64_interrupt_handler_183),
		IdtGate((uint64_t)&x64_interrupt_handler_184),
		IdtGate((uint64_t)&x64_interrupt_handler_185),
		IdtGate((uint64_t)&x64_interrupt_handler_186),
		IdtGate((uint64_t)&x64_interrupt_handler_187),
		IdtGate((uint64_t)&x64_interrupt_handler_188),
		IdtGate((uint64_t)&x64_interrupt_handler_189),
		IdtGate((uint64_t)&x64_interrupt_handler_190),
		IdtGate((uint64_t)&x64_interrupt_handler_191),
		IdtGate((uint64_t)&x64_interrupt_handler_192),
		IdtGate((uint64_t)&x64_interrupt_handler_193),
		IdtGate((uint64_t)&x64_interrupt_handler_194),
		IdtGate((uint64_t)&x64_interrupt_handler_195),
		IdtGate((uint64_t)&x64_interrupt_handler_196),
		IdtGate((uint64_t)&x64_interrupt_handler_197),
		IdtGate((uint64_t)&x64_interrupt_handler_198),
		IdtGate((uint64_t)&x64_interrupt_handler_199),
		IdtGate((uint64_t)&x64_interrupt_handler_200),
		IdtGate((uint64_t)&x64_interrupt_handler_201),
		IdtGate((uint64_t)&x64_interrupt_handler_202),
		IdtGate((uint64_t)&x64_interrupt_handler_203),
		IdtGate((uint64_t)&x64_interrupt_handler_204),
		IdtGate((uint64_t)&x64_interrupt_handler_205),
		IdtGate((uint64_t)&x64_interrupt_handler_206),
		IdtGate((uint64_t)&x64_interrupt_handler_207),
		IdtGate((uint64_t)&x64_interrupt_handler_208),
		IdtGate((uint64_t)&x64_interrupt_handler_209),
		IdtGate((uint64_t)&x64_interrupt_handler_210),
		IdtGate((uint64_t)&x64_interrupt_handler_211),
		IdtGate((uint64_t)&x64_interrupt_handler_212),
		IdtGate((uint64_t)&x64_interrupt_handler_213),
		IdtGate((uint64_t)&x64_interrupt_handler_214),
		IdtGate((uint64_t)&x64_interrupt_handler_215),
		IdtGate((uint64_t)&x64_interrupt_handler_216),
		IdtGate((uint64_t)&x64_interrupt_handler_217),
		IdtGate((uint64_t)&x64_interrupt_handler_218),
		IdtGate((uint64_t)&x64_interrupt_handler_219),
		IdtGate((uint64_t)&x64_interrupt_handler_220),
		IdtGate((uint64_t)&x64_interrupt_handler_221),
		IdtGate((uint64_t)&x64_interrupt_handler_222),
		IdtGate((uint64_t)&x64_interrupt_handler_223),
		IdtGate((uint64_t)&x64_interrupt_handler_224),
		IdtGate((uint64_t)&x64_interrupt_handler_225),
		IdtGate((uint64_t)&x64_interrupt_handler_226),
		IdtGate((uint64_t)&x64_interrupt_handler_227),
		IdtGate((uint64_t)&x64_interrupt_handler_228),
		IdtGate((uint64_t)&x64_interrupt_handler_229),
		IdtGate((uint64_t)&x64_interrupt_handler_230),
		IdtGate((uint64_t)&x64_interrupt_handler_231),
		IdtGate((uint64_t)&x64_interrupt_handler_232),
		IdtGate((uint64_t)&x64_interrupt_handler_233),
		IdtGate((uint64_t)&x64_interrupt_handler_234),
		IdtGate((uint64_t)&x64_interrupt_handler_235),
		IdtGate((uint64_t)&x64_interrupt_handler_236),
		IdtGate((uint64_t)&x64_interrupt_handler_237),
		IdtGate((uint64_t)&x64_interrupt_handler_238),
		IdtGate((uint64_t)&x64_interrupt_handler_239),
		IdtGate((uint64_t)&x64_interrupt_handler_240),
		IdtGate((uint64_t)&x64_interrupt_handler_241),
		IdtGate((uint64_t)&x64_interrupt_handler_242),
		IdtGate((uint64_t)&x64_interrupt_handler_243),
		IdtGate((uint64_t)&x64_interrupt_handler_244),
		IdtGate((uint64_t)&x64_interrupt_handler_245),
		IdtGate((uint64_t)&x64_interrupt_handler_246),
		IdtGate((uint64_t)&x64_interrupt_handler_247),
		IdtGate((uint64_t)&x64_interrupt_handler_248),
		IdtGate((uint64_t)&x64_interrupt_handler_249),
		IdtGate((uint64_t)&x64_interrupt_handler_250),
		IdtGate((uint64_t)&x64_interrupt_handler_251),
		IdtGate((uint64_t)&x64_interrupt_handler_252),
		IdtGate((uint64_t)&x64_interrupt_handler_253),
		IdtGate((uint64_t)&x64_interrupt_handler_254),
		IdtGate((uint64_t)&x64_interrupt_handler_255)
	};

	DescriptorTable GDTR =
	{
		sizeof(KernelGDT) - 1,
		(uint64_t)&KernelGDT
	};

	DescriptorTable IDTR =
	{
		sizeof(IDT) - 1,
		(uint64_t)IDT
	};
}
