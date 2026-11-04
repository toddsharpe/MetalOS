#include "core_crt/stdint.h"

#ifdef __cplusplus
extern "C"
{
#endif

	int sprintf(char *const _Buffer, char const *const _Format, ...);

	// Bounded: returns the number of characters stored (excluding the NUL)
	int snprintf(char *const _Buffer, size_t const _BufferCount, char const *const _Format, ...);

	int vsprintf(char *const _Buffer, char const *const _Format, va_list _ArgList);

	// Bounded: returns the number of characters stored (excluding the NUL)
	int vsnprintf(char *const _Buffer, size_t const _BufferCount, char const *const _Format, va_list _ArgList);

#ifdef __cplusplus
}
#endif
