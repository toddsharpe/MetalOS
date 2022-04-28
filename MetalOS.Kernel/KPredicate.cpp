#include "KPredicate.h"

KPredicate::KPredicate(SignalPredicate predicate, void* arg) :
	KSignalObject(KObjectType::Predicate),
	m_predicate(predicate),
	m_arg(arg)
{

}

bool KPredicate::IsSignalled() const
{
	return (*m_predicate)(m_arg);
}
