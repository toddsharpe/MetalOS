#include <stdint.h>
#include <stdarg.h>

int wcscmp(const wchar_t* str1, const wchar_t* str2)
{
	while ((*str1) && (*str1 == *str2))
	{
		str1++;
		str2++;
	}
	return *(uint16_t*)str1 - *(uint16_t*)str2;
}

size_t wcslen(const wchar_t* str)
{
	size_t length = 0;
	while (*str != L'\0')
	{
		length++;
		str++;
	}

	return length;
}

wchar_t* wcscpy(wchar_t* dest, const wchar_t* source)
{
	size_t length = wcslen(source);
	memcpy(dest, source, length * sizeof(wchar_t));
	return dest;
}

wchar_t* wcsncpy(wchar_t* dest, const wchar_t* source, size_t num)
{
	memcpy(dest, source, num * sizeof(wchar_t));
	return dest;
}

void wcsrev(wchar_t* str)
{
	size_t length = wcslen(str);

	for (size_t i = 0; i < length / 2; i++)
	{
		wchar_t temp = str[i];
		str[i] = str[length - i - 1];
		str[length - i - 1] = temp;
	}
}

//Internal protypes
int kvwprintf(wchar_t const* fmt, void (*func)(int, void*), void* arg, int radix, va_list ap);
wchar_t* kswprintn(wchar_t* nbuf, uintmax_t num, int base, int* len, int upper);

//TODO: convert/rewrite these routines, or at lease source them

#define NBBY    8               /* number of bits in a byte */
/* Max number conversion buffer length: an unsigned long long int in base 2, plus NUL byte. */
#define MAXNBUF	(sizeof(intmax_t) * NBBY + 1)

wchar_t const hex2wchar_t_data[] = L"0123456789abcdefghijklmnopqrstuvwxyz";

#define hex2ascii(hex)  (hex2wchar_t_data[hex])
#define toupper(c)      ((c) - 0x20 * (((c) >= L'a') && ((c) <= L'z')))

typedef size_t ssize_t;

inline int imax(int a, int b)
{
	return (a > b ? a : b);
}
/*
int swprintf(wchar_t* const _Buffer, size_t const _BufferCount, wchar_t const* const _Format, ...)
{
	int retval;
	va_list ap;

	va_start(ap, _Format);
	retval = kvwprintf(_Format, NULL, (void*)_Buffer, 10, ap);
	_Buffer[retval] = L'\0';
	va_end(ap);
	return (retval);
}
*/
/*
int vwprintf(wchar_t* const _Buffer, wchar_t const* const _Format, va_list _ArgList)
{
	int retval;

	retval = kvwprintf(_Format, NULL, (void*)_Buffer, 10, _ArgList);
	_Buffer[retval] = L'\0';
	return (retval);
}
*/

/*
 * Put a NUL-terminated ASCII number (base <= 36) in a buffer in reverse
 * order; return an optional length and a pointer to the last character
 * written in the buffer (i.e., the first character of the string).
 * The buffer pointed to by `nbuf' must have length >= MAXNBUF.
 */
wchar_t* kswprintn(wchar_t* nbuf, uintmax_t num, int base, int* lenp, int upper)
{
	wchar_t* p, c;

	p = nbuf;
	*p = L'\0';
	do {
		c = hex2ascii(num % base);
		*++p = upper ? toupper(c) : c;
	} while (num /= base);
	if (lenp)
		* lenp = (int)(p - nbuf);
	return (p);
}

/*
 * Scaled down version of printf(3).
 *
 * Two additional formats:
 *
 * The format %b is supported to decode error registers.
 * Its usage is:
 *
 *	printf("reg=%b\n", regval, "<base><arg>*");
 *
 * where <base> is the output base expressed as a control character, e.g.
 * \10 gives octal; \20 gives hex.  Each arg is a sequence of characters,
 * the first of which gives the bit number to be inspected (origin 1), and
 * the next characters (up to a control character, i.e. a character <= 32),
 * give the name of the register.  Thus:
 *
 *	kvprintf("reg=%b\n", 3, "\10\2BITTWO\1BITONE");
 *
 * would produce output:
 *
 *	reg=3<BITTWO,BITONE>
 *
 * XXX:  %D  -- Hexdump, takes pointer and separator string:
 *		("%6D", ptr, ":")   -> XX:XX:XX:XX:XX:XX
 *		("%*D", len, ptr, " " -> XX XX XX XX ...
 */
int kvwprintf(wchar_t const* fmt, void (*func)(int, void*), void* arg, int radix, va_list ap)
{
#define PCHAR(c) {int cc=(c); if (func) (*func)(cc,arg); else *d++ = cc; retval++; }
	wchar_t nbuf[MAXNBUF];
	wchar_t* d;
	const wchar_t* p, * percent, * q;
	wchar_t* up;
	int ch, n;
	uintmax_t num;
	int base, lflag, qflag, tmp, width, ladjust, sharpflag, neg, sign, dot;
	int cflag, hflag, jflag, tflag, zflag;
	int bconv, dwidth, upper;
	wchar_t padc;
	int stop = 0, retval = 0;

	num = 0;
	q = NULL;
	if (!func)
		d = (wchar_t*)arg;
	else
		d = NULL;

	if (fmt == NULL)
		fmt = L"(fmt null)\n";

	if (radix < 2 || radix > 36)
		radix = 10;

	for (;;) {
		padc = ' ';
		width = 0;
		while ((ch = (wchar_t)* fmt++) != L'%' || stop) {
			if (ch == L'\0')
				return (retval);
			PCHAR(ch);
		}
		percent = fmt - 1;
		qflag = 0; lflag = 0; ladjust = 0; sharpflag = 0; neg = 0;
		sign = 0; dot = 0; bconv = 0; dwidth = 0; upper = 0;
		cflag = 0; hflag = 0; jflag = 0; tflag = 0; zflag = 0;
	reswitch:	switch (ch = (wchar_t)* fmt++) {
	case L'.':
		dot = 1;
		goto reswitch;
	case L'#':
		sharpflag = 1;
		goto reswitch;
	case L'+':
		sign = 1;
		goto reswitch;
	case L'-':
		ladjust = 1;
		goto reswitch;
	case L'%':
		PCHAR(ch);
		break;
	case L'*':
		if (!dot) {
			width = va_arg(ap, int);
			if (width < 0) {
				ladjust = !ladjust;
				width = -width;
			}
		}
		else {
			dwidth = va_arg(ap, int);
		}
		goto reswitch;
	case L'0':
		if (!dot) {
			padc = L'0';
			goto reswitch;
		}
		/* FALLTHROUGH */
	case L'1': case L'2': case L'3': case L'4':
	case L'5': case L'6': case L'7': case L'8': case L'9':
		for (n = 0;; ++fmt) {
			n = n * 10 + ch - L'0';
			ch = *fmt;
			if (ch < L'0' || ch > L'9')
				break;
		}
		if (dot)
			dwidth = n;
		else
			width = n;
		goto reswitch;
	case L'b':
		ladjust = 1;
		bconv = 1;
		goto handle_nosign;
	case L'c':
		width -= 1;

		if (!ladjust && width > 0)
			while (width--)
				PCHAR(padc);
		PCHAR(va_arg(ap, int));
		if (ladjust && width > 0)
			while (width--)
				PCHAR(padc);
		break;
	case L'D':
		up = va_arg(ap, wchar_t*);
		p = va_arg(ap, wchar_t*);
		if (!width)
			width = 16;
		while (width--) {
			PCHAR(hex2ascii(*up >> 4));
			PCHAR(hex2ascii(*up & 0x0f));
			up++;
			if (width)
				for (q = p; *q; q++)
					PCHAR(*q);
		}
		break;
	case L'd':
	case L'i':
		base = 10;
		sign = 1;
		goto handle_sign;
	case L'h':
		if (hflag) {
			hflag = 0;
			cflag = 1;
		}
		else
			hflag = 1;
		goto reswitch;
	case L'j':
		jflag = 1;
		goto reswitch;
	case L'l':
		if (lflag) {
			lflag = 0;
			qflag = 1;
		}
		else
			lflag = 1;
		goto reswitch;
	case L'n':
		if (jflag)
			* (va_arg(ap, intmax_t*)) = retval;
		else if (qflag)
			* (va_arg(ap, long long*)) = retval;
		else if (lflag)
			* (va_arg(ap, long*)) = retval;
		else if (zflag)
			* (va_arg(ap, size_t*)) = retval;
		else if (hflag)
			* (va_arg(ap, short*)) = retval;
		else if (cflag)
			* (va_arg(ap, char*)) = retval;
		else
			*(va_arg(ap, int*)) = retval;
		break;
	case L'o':
		base = 8;
		goto handle_nosign;
	case L'p':
		base = 16;
		sharpflag = (width == 0);
		sign = 0;
		num = (uintptr_t)va_arg(ap, void*);
		goto number;
	case L'q':
		qflag = 1;
		goto reswitch;
	case L'r':
		base = radix;
		if (sign)
			goto handle_sign;
		goto handle_nosign;
	case L's':
		p = va_arg(ap, wchar_t*);
		if (p == NULL)
			p = L"(null)";
		if (!dot)
			n = (int)wcslen(p);
		else
			for (n = 0; n < dwidth && p[n]; n++)
				continue;

		width -= n;

		if (!ladjust && width > 0)
			while (width--)
				PCHAR(padc);
		while (n--)
			PCHAR(*p++);
		if (ladjust && width > 0)
			while (width--)
				PCHAR(padc);
		break;
	case L't':
		tflag = 1;
		goto reswitch;
	case L'u':
		base = 10;
		goto handle_nosign;
	case L'X':
		upper = 1;
		//__attribute__((fallthrough)); // For GCC to stop warning about a fallthrough here
	case L'x':
		base = 16;
		goto handle_nosign;
	case L'y':
		base = 16;
		sign = 1;
		goto handle_sign;
	case L'z':
		zflag = 1;
		goto reswitch;
	handle_nosign:
		sign = 0;
		if (jflag)
			num = va_arg(ap, uintmax_t);
		else if (qflag || (width == 16)) //HACK? Apparently "qx" is the old way to do 64bit? this lets us not use the q if we have %16x
			num = va_arg(ap, unsigned long long);
		else if (tflag)
			num = va_arg(ap, ptrdiff_t);
		else if (lflag)
			num = va_arg(ap, unsigned long);
		else if (zflag)
			num = va_arg(ap, size_t);
		else if (hflag)
			num = (unsigned short)va_arg(ap, int);
		else if (cflag)
			num = (unsigned char)va_arg(ap, int);
		else
			num = va_arg(ap, unsigned int);
		if (bconv) {
			q = va_arg(ap, wchar_t*);
			base = *q++;
		}
		goto number;
	handle_sign:
		if (jflag)
			num = va_arg(ap, intmax_t);
		else if (qflag)
			num = va_arg(ap, long long);
		else if (tflag)
			num = va_arg(ap, ptrdiff_t);
		else if (lflag)
			num = va_arg(ap, long);
		else if (zflag)
			num = va_arg(ap, ssize_t);
		else if (hflag)
			num = (short)va_arg(ap, int);
		else if (cflag)
			num = (wchar_t)va_arg(ap, int);
		else
			num = va_arg(ap, int);
	number:
		if (sign && (intmax_t)num < 0) {
			neg = 1;
			num = -(intmax_t)num;
		}
		p = kswprintn(nbuf, num, base, &n, upper);
		tmp = 0;

		// There's weird behavior here with #. Don't use # to get 0x with zero-padding
		// (e.g. use 0x%016qx instead, not %#016qx or %#018qx, the latter of which will pad
		// 16 characters for nonzero numbers but zeros will have 18 characters).
		// Same with octal: use a leading zero and don't rely on # if you want zero-padding.
		// # works if you don't need zero padding, though.

		if (sharpflag && num != 0) {
			if (base == 8)
				tmp++;
			else if (base == 16)
				tmp += 2;
		}
		if (neg)
			tmp++;

		if (!ladjust && padc == L'0')
			dwidth = width - tmp;
		width -= tmp + imax(dwidth, n);
		dwidth -= n;
		if (!ladjust)
			while (width-- > 0)
				PCHAR(L' ');
		if (neg)
			PCHAR(L'-');
		if (sharpflag && num != 0) {
			if (base == 8) {
				PCHAR(L'0');
			}
			else if (base == 16) {
				PCHAR(L'0');
				PCHAR(L'x');
			}
		}
		while (dwidth-- > 0)
			PCHAR(L'0');

		while (*p)
			PCHAR(*p--);

		if (bconv && num != 0) {
			/* %b conversion flag format. */
			tmp = retval;
			while (*q) {
				n = *q++;
				if (num & (1ll << ((uintmax_t)n - 1))) {
					PCHAR(retval != tmp ?
						L',' : L'<');
					for (; (n = *q) > L' '; ++q)
						PCHAR(n);
				}
				else
					for (; *q > L' '; ++q)
						continue;
			}
			if (retval != tmp) {
				PCHAR(L'>');
				width -= retval - tmp;
			}
		}

		if (ladjust)
			while (width-- > 0)
				PCHAR(L' ');

		break;
	default:
		while (percent < fmt)
			PCHAR(*percent++);
		/*
		 * Since we ignore a formatting argument it is no
		 * longer safe to obey the remaining formatting
		 * arguments as the arguments will no longer match
		 * the format specs.
		 */
		stop = 1;
		break;
	}
	}
#undef PCHAR
}

/*
typedef void* _locale_t;
int __stdio_common_vswprintf(
	unsigned __int64 const options,
	wchar_t* const buffer,
	size_t           const buffer_count,
	wchar_t const* const format,
	_locale_t        const locale,
	va_list          const arglist
)
{
	return vwprintf(buffer, format, arglist);
}
*/

/*
int vswprintf(wchar_t* const _Buffer, size_t const _BufferCount, wchar_t const* const _Format, va_list _ArgList)
{
	int retval;

	retval = kvwprintf(_Format, NULL, (void*)_Buffer, 10, _ArgList);
	_Buffer[retval] = L'\0';
	return (retval);
}
*/

int swprintf(wchar_t* const _Buffer, size_t const _BufferCount, wchar_t const* const _Format, ...)
{
    int _Result;
    va_list _ArgList;
	va_start(_ArgList, _Format);
    _Result = vswprintf(_Buffer, _BufferCount, _Format, _ArgList);
    va_end(_ArgList);
    return _Result;
}

//TODO: use _BufferCount
int vswprintf(wchar_t* const _Buffer, size_t const _BufferCount, wchar_t const* const _Format, va_list _ArgList)
{
	int retval;

	retval = kvwprintf(_Format, NULL, (void*)_Buffer, 10, _ArgList);
	_Buffer[retval] = L'\0';
	return (retval);
}

