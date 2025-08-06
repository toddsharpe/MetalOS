#pragma once

#include "kernel/Objects/KPipe.h"

class UPipe
{
public:
	UPipe(KPipe& kPipe, const KPipeOp op) :
		KPipe(kPipe),
		Op(op)
	{

	}

	KPipe& KPipe;
	const KPipeOp Op;
};

