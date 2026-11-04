#pragma once

//This file defines placement new only
inline void* operator new(size_t _Size, void* _Where)
{
	(void)_Size;
	return _Where;
}
