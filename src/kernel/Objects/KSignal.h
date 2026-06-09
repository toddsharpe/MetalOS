#pragma once

#include "Assert.h"
#include "kernel/Api.h"

typedef bool(*SignalPredicate)(void* const arg);

enum class KSignalType
{
	Event,
	Semaphore,
	Predicate,
};

struct KSignal
{
	constexpr KSignal(const KSignalType type = KSignalType::Event) :
		Type(type),
		Event{ false, false }
	{
		
	}
	
	bool IsSignalled() const
	{
		switch (Type)
		{
			case KSignalType::Event:
				return Event.State;
			case KSignalType::Semaphore:
				return Semaphore.Value > 0;
			case KSignalType::Predicate:
				return Predicate.Fn(Predicate.Arg);
			default:
				return false;
		}
	}

	void Observed()
	{
		switch (Type)
		{
			case KSignalType::Event:
				if (!Event.ManualReset)
					Event.State = false;
				break;
			case KSignalType::Semaphore:
				Semaphore.Value--;
				break;
			case KSignalType::Predicate:
				break;
		}
	}

	void Display() const
	{
		switch (Type)
		{
			case KSignalType::Event:
				Printf("KSignal::Event\n");
				Printf("    ManualReset: %d\n", Event.ManualReset);
				Printf("    State: %d\n", Event.State);
				break;
			case KSignalType::Semaphore:
				Printf("KSignal::Semaphore\n");
				Printf("    Value: %d\n", Semaphore.Value);
				Printf("    Limit: %d\n", Semaphore.Limit);
				break;
			case KSignalType::Predicate:
				Printf("KSignal::Predicate\n");
				break;
		}
	}

protected:
	KSignalType Type;

	union
	{
		struct
		{
			bool ManualReset;
			bool State;
		} Event;
		struct
		{
			int Value;
			int Limit;
		} Semaphore;
		struct
		{
			SignalPredicate Fn;
			void* Arg;
		} Predicate;
	};
};
