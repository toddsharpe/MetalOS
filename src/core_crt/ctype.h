#include "core_crt/stdint.h"

#define _ACPI_XA 0x00 /* extra alphabetic - not supported */
#define _ACPI_XS 0x40 /* extra space */
#define _ACPI_BB 0x00 /* BEL, BS, etc. - not supported */
#define _ACPI_CN 0x20 /* CR, FF, HT, NL, VT */
#define _ACPI_DI 0x04 /* '0'-'9' */
#define _ACPI_LO 0x02 /* 'a'-'z' */
#define _ACPI_PU 0x10 /* punctuation */
#define _ACPI_SP 0x08 /* space, tab, CR, LF, VT, FF */
#define _ACPI_UP 0x01 /* 'A'-'Z' */
#define _ACPI_XD 0x80 /* '0'-'9', 'A'-'F', 'a'-'f' */

#ifdef __cplusplus
extern "C"
{
#endif

	int isdigit(int _C);

	int isspace(int _C);

	int isxdigit(int _C);

	int isprint(int _C);

	int toupper(int _C);

	int tolower(int _C);

#ifdef __cplusplus
}
#endif
