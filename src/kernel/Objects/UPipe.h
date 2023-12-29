#pragma once

#include "Kernel/Objects/UObject.h"
#include "Kernel/Objects/UserPipe.h"

#include <memory>

class UPipe : public UObject
{
public:
	UPipe(std::shared_ptr<UserPipe>& pipe, const PipeOp op) :
		UObject(UObjectType::Pipe),
		Pipe(pipe),
		Op(op)
	{

	}

	virtual bool IsWriteable() const override
	{
		return Op == PipeOp::Write;
	}

	virtual bool IsReadable() const override
	{
		return Op == PipeOp::Read;
	}

	virtual void Display() const override
	{
		UObject::Display();
		Printf("  UReadPipe\n");
		Printf("    Readers: %d\n", Pipe->Readers);
		Printf("    Writers: %d\n", Pipe->Writers);
	}

	std::shared_ptr<UserPipe> Pipe;
	const PipeOp Op;
};
