#pragma once

class KSignalObject
{
public:
	virtual bool IsSignalled() const = 0;
	virtual void Observed() {}
	virtual void Display() const {}
};
