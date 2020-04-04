#pragma once
#include <cstdint>

//https://wiki.osdev.org/8259_PIC
//https://en.wikibooks.org/wiki/X86_Assembly/Programmable_Interrupt_Controller
//https://en.wikipedia.org/wiki/Interrupt_request_(PC_architecture)

//Reasonably confident HyperV Gen2 doesn't even support PIC as the masks always return 0xFF no matter if they have been set
class x64PIC
{
public:
	static void Initialize(uint8_t masterOffset, uint8_t slaveOffset);
	static void Display();

private:
	static void IoWait();

	enum REGISTER : uint8_t
	{
		Command = 0,
		Data = 1
	};
	
	static const uint8_t PIC1 = 0x20;
	static const uint8_t PIC2 = 0xA0;

#define ICW1_ICW4	0x01		/* ICW4 (not) needed */
#define ICW1_SINGLE	0x02		/* Single (cascade) mode */
#define ICW1_INTERVAL4	0x04		/* Call address interval 4 (8) */
#define ICW1_LEVEL	0x08		/* Level triggered (edge) mode */
#define ICW1_INIT	0x10		/* Initialization - required! */

#define ICW4_8086	0x01		/* 8086/88 (MCS-80/85) mode */
#define ICW4_AUTO	0x02		/* Auto (normal) EOI */
#define ICW4_BUF_SLAVE	0x08		/* Buffered mode/slave */
#define ICW4_BUF_MASTER	0x0C		/* Buffered mode/master */
#define ICW4_SFNM	0x10		/* Special fully nested (not) */
};

