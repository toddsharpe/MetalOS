#include "x64PIC.h"
#include <intrin.h>
#include "Main.h"

void x64PIC::Initialize(uint8_t masterOffset, uint8_t slaveOffset)
{
	uint8_t masterMask = 0;// __inbyte(PIC1 + REGISTER::Data);
	uint8_t slaveMask = 0;// __inbyte(PIC2 + REGISTER::Data);


	//Start initialization sequence
	__outbyte(PIC1 + REGISTER::Command, ICW1_INIT | ICW1_ICW4);
	IoWait();
	__outbyte(PIC2 + REGISTER::Command, ICW1_INIT | ICW1_ICW4);
	IoWait();

	//Write offsets (added to IRQ line to get CPU interrupt)
	__outbyte(PIC1 + REGISTER::Data, masterOffset);
	IoWait();
	__outbyte(PIC2 + REGISTER::Data, slaveOffset);
	IoWait();
	__outbyte(PIC1 + REGISTER::Data, 4);
	IoWait();
	__outbyte(PIC2 + REGISTER::Data, 2);
	IoWait();

	//Enable
	__outbyte(PIC1 + REGISTER::Data, ICW4_8086);
	IoWait();
	__outbyte(PIC2 + REGISTER::Data, ICW4_8086);
	IoWait();

	//Restore Masks
	__outbyte(PIC1 + REGISTER::Data, masterMask);
	__outbyte(PIC2 + REGISTER::Data, slaveMask);
}

void x64PIC::Display()
{
	uint8_t masterMask = __inbyte(PIC1 + REGISTER::Data);
	uint8_t slaveMask = __inbyte(PIC2 + REGISTER::Data);
	Print("Masks: 0x%02x, 0x%02x\n", masterMask, slaveMask);
}

//Placeholder in case we need to loop for hardware to catchup
void x64PIC::IoWait()
{
	volatile size_t counter = 0;
	for (; counter < 1000; ++counter);
}

