#pragma once

#include "KSignalObject.h"

class KEvent : public KSignalObject
{
public:
	KEvent(bool manualReset, bool initialState);

	void Set();
	void Reset();

	bool IsManual();

	virtual bool IsSignalled() const override;

private:
	bool m_manualReset;
	bool m_state;
};
