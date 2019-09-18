#include "Path.h"
#include "String.h"

//TODO: These string manipulation functions need length checks
//Buffer overflows for days

//Includes trailing slash
void GetDirectory(CHAR16* source, CHAR16* destination)
{
	UINT32 current = 0;
	UINT32 marker = 0;
	while (source[current] != L'\0')
	{
		if (source[current] == L'\\')
			marker = current;
		current++;
	}

	//Include trailing slash
	marker++;

	//Copy to destination
	efi_strcpy_n(source, destination, marker);

	//Add null
	destination[marker + 1] = L'\0';
}

//Kinda written shitty
void PathCombine(CHAR16* source, CHAR16* destination, CHAR16* file)
{
	//Copy over directory
	GetDirectory(source, destination);

	//Copy in file
	efi_strcpy(file, destination + efi_strlen(destination));
}