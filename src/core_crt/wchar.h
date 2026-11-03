#include "core_crt/stdint.h"

#ifdef __cplusplus
extern "C"
{
#endif

	int wcscmp(const wchar_t *str1, const wchar_t *str2);

	size_t wcslen(const wchar_t *str);

	wchar_t *wcscpy(wchar_t *dest, const wchar_t *source);

	wchar_t *wcsncpy(wchar_t *dest, const wchar_t *source, size_t num);

#ifdef __cplusplus
}
#endif
