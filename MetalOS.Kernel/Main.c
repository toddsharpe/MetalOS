#include <efi.h>
#include "LoaderParams.h"

void main(LOADER_PARAMS* loader)
{
	loader->ConOut->OutputString(loader->ConOut, L"Kernel!\r\n");
	return;
}