#include "crt_stdio.h"
#include <stdint.h>

//Internal protypes
int kvprintf(char const* fmt, void (*func)(int, void*), void* arg, int radix, va_list ap);
char* ksprintn(char* nbuf, uintmax_t num, int base, int* len, int upper);

//TODO: convert/rewrite these routines, or at lease source them

#define NULL 0
#define NBBY    8               /* number of bits in a byte */
/* Max number conversion buffer length: an unsigned long long int in base 2, plus NUL byte. */
#define MAXNBUF	(sizeof(intmax_t) * NBBY + 1)

char const hex2ascii_data[] = "0123456789abcdefghijklmnopqrstuvwxyz";

#define hex2ascii(hex)  (hex2ascii_data[hex])
#define toupper(c)      ((c) - 0x20 * (((c) >= 'a') && ((c) <= 'z')))

typedef size_t ssize_t;

inline int imax(int a, int b)
{
	return (a > b ? a : b);
}

int crt_sprintf(char* const _Buffer, char const* const _Format, ...)
{
	int retval;
	va_list ap;

	va_start(ap, _Format);
	retval = kvprintf(_Format, NULL, (void*)_Buffer, 10, ap);
	_Buffer[retval] = '\0';
	va_end(ap);
	return (retval);
}

//TODO: BufferCount is not used
int snprintf(char* const _Buffer, size_t const _BufferCount, char const* const _Format, ...)
{
	int retval;
	va_list ap;

	va_start(ap, _Format);
	retval = kvprintf(_Format, NULL, (void*)_Buffer, 10, ap);
	_Buffer[retval] = '\0';
	va_end(ap);
	return (retval);
}

int crt_vsprintf(char* const _Buffer, char const* const _Format, va_list _ArgList)
{
	int retval;

	retval = kvprintf(_Format, NULL, (void*)_Buffer, 10, _ArgList);
	_Buffer[retval] = '\0';
	return (retval);
}

/*
 * Put a NUL-terminated ASCII number (base <= 36) in a buffer in reverse
 * order; return an optional length and a pointer to the last character
 * written in the buffer (i.e., the first character of the string).
 * The buffer pointed to by `nbuf' must have length >= MAXNBUF.
 */
char* ksprintn(char* nbuf, uintmax_t num, int base, int* lenp, int upper)
{
	char* p, c;

	p = nbuf;
	*p = '\0';
	do {
		c = hex2ascii(num % base);
		*++p = upper ? toupper(c) : c;
	} while (num /= base);
	if (lenp)
		* lenp = p - nbuf;
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
int kvprintf(char const* fmt, void (*func)(int, void*), void* arg, int radix, va_list ap)
{
#define PCHAR(c) {int cc=(c); if (func) (*func)(cc,arg); else *d++ = cc; retval++; }
	char nbuf[MAXNBUF];
	char* d;
	const char* p, * percent, * q;
	unsigned char* up;
	int ch, n;
	uintmax_t num;
	int base, lflag, qflag, tmp, width, ladjust, sharpflag, neg, sign, dot;
	int cflag, hflag, jflag, tflag, zflag;
	int bconv, dwidth, upper;
	char padc;
	int stop = 0, retval = 0;

	num = 0;
	q = NULL;
	if (!func)
		d = (char*)arg;
	else
		d = NULL;

	if (fmt == NULL)
		fmt = "(fmt null)\n";

	if (radix < 2 || radix > 36)
		radix = 10;

	for (;;) {
		padc = ' ';
		width = 0;
		while ((ch = (unsigned char)* fmt++) != '%' || stop) {
			if (ch == '\0')
				return (retval);
			PCHAR(ch);
		}
		percent = fmt - 1;
		qflag = 0; lflag = 0; ladjust = 0; sharpflag = 0; neg = 0;
		sign = 0; dot = 0; bconv = 0; dwidth = 0; upper = 0;
		cflag = 0; hflag = 0; jflag = 0; tflag = 0; zflag = 0;
	reswitch:	switch (ch = (unsigned char)* fmt++) {
	case '.':
		dot = 1;
		goto reswitch;
	case '#':
		sharpflag = 1;
		goto reswitch;
	case '+':
		sign = 1;
		goto reswitch;
	case '-':
		ladjust = 1;
		goto reswitch;
	case '%':
		PCHAR(ch);
		break;
	case '*':
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
	case '0':
		if (!dot) {
			padc = '0';
			goto reswitch;
		}
		/* FALLTHROUGH */
	case '1': case '2': case '3': case '4':
	case '5': case '6': case '7': case '8': case '9':
		for (n = 0;; ++fmt) {
			n = n * 10 + ch - '0';
			ch = *fmt;
			if (ch < '0' || ch > '9')
				break;
		}
		if (dot)
			dwidth = n;
		else
			width = n;
		goto reswitch;
	case 'b':
		ladjust = 1;
		bconv = 1;
		goto handle_nosign;
	case 'c':
		width -= 1;

		if (!ladjust && width > 0)
			while (width--)
				PCHAR(padc);
		PCHAR(va_arg(ap, int));
		if (ladjust && width > 0)
			while (width--)
				PCHAR(padc);
		break;
	case 'D':
		up = va_arg(ap, unsigned char*);
		p = va_arg(ap, char*);
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
	case 'd':
	case 'i':
		base = 10;
		sign = 1;
		goto handle_sign;
	case 'h':
		if (hflag) {
			hflag = 0;
			cflag = 1;
		}
		else
			hflag = 1;
		goto reswitch;
	case 'j':
		jflag = 1;
		goto reswitch;
	case 'l':
		if (lflag) {
			lflag = 0;
			qflag = 1;
		}
		else
			lflag = 1;
		goto reswitch;
	case 'n':
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
	case 'o':
		base = 8;
		goto handle_nosign;
	case 'p':
		base = 16;
		sharpflag = (width == 0);
		sign = 0;
		num = (uintptr_t)va_arg(ap, void*);
		goto number;
	case 'q':
		qflag = 1;
		goto reswitch;
	case 'r':
		base = radix;
		if (sign)
			goto handle_sign;
		goto handle_nosign;
	case 's':
		p = va_arg(ap, char*);
		if (p == NULL)
			p = "(null)";
		if (!dot)
			n = strlen(p);
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
	case 't':
		tflag = 1;
		goto reswitch;
	case 'u':
		base = 10;
		goto handle_nosign;
	case 'X':
		upper = 1;
		//__attribute__((fallthrough)); // For GCC to stop warning about a fallthrough here
	case 'x':
		base = 16;
		goto handle_nosign;
	case 'y':
		base = 16;
		sign = 1;
		goto handle_sign;
	case 'z':
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
			q = va_arg(ap, char*);
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
			num = (char)va_arg(ap, int);
		else
			num = va_arg(ap, int);
	number:
		if (sign && (intmax_t)num < 0) {
			neg = 1;
			num = -(intmax_t)num;
		}
		p = ksprintn(nbuf, num, base, &n, upper);
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

		if (!ladjust && padc == '0')
			dwidth = width - tmp;
		width -= tmp + imax(dwidth, n);
		dwidth -= n;
		if (!ladjust)
			while (width-- > 0)
				PCHAR(' ');
		if (neg)
			PCHAR('-');
		if (sharpflag && num != 0) {
			if (base == 8) {
				PCHAR('0');
			}
			else if (base == 16) {
				PCHAR('0');
				PCHAR('x');
			}
		}
		while (dwidth-- > 0)
			PCHAR('0');

		while (*p)
			PCHAR(*p--);

		if (bconv && num != 0) {
			/* %b conversion flag format. */
			tmp = retval;
			while (*q) {
				n = *q++;
				if (num & (1 << (n - 1))) {
					PCHAR(retval != tmp ?
						',' : '<');
					for (; (n = *q) > ' '; ++q)
						PCHAR(n);
				}
				else
					for (; *q > ' '; ++q)
						continue;
			}
			if (retval != tmp) {
				PCHAR('>');
				width -= retval - tmp;
			}
		}

		if (ladjust)
			while (width-- > 0)
				PCHAR(' ');

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


