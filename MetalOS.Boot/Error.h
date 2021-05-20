#pragma once

#include <efi.h>

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

#define WIDE_HELPER(x) L##x
#define WSTR(x) WIDE_HELPER(x)

#define WFILE WSTR(__FILE__)
#define LLINE WSTR(STR(__LINE__))

#define ReturnIfNotSuccess(x) status = x; if (EFI_ERROR(status)) { Error::DisplayError(L#x, WFILE, LLINE, status); return status; }
#define ReturnIfNotSuccessNoDisplay(x) status = x; if (EFI_ERROR(status)) { return status; }

struct
{
	EFI_STATUS      Code;
	const CHAR16* Desc;
} ErrorCodeTable[] = {
	{  EFI_SUCCESS,                L"Success"},
	{  EFI_LOAD_ERROR,             L"Load Error"},
	{  EFI_INVALID_PARAMETER,      L"Invalid Parameter"},
	{  EFI_UNSUPPORTED,            L"Unsupported"},
	{  EFI_BAD_BUFFER_SIZE,        L"Bad Buffer Size"},
	{  EFI_BUFFER_TOO_SMALL,       L"Buffer Too Small"},
	{  EFI_NOT_READY,              L"Not Ready"},
	{  EFI_DEVICE_ERROR,           L"Device Error"},
	{  EFI_WRITE_PROTECTED,        L"Write Protected"},
	{  EFI_OUT_OF_RESOURCES,       L"Out of Resources"},
	{  EFI_VOLUME_CORRUPTED,       L"Volume Corrupt"},
	{  EFI_VOLUME_FULL,            L"Volume Full"},
	{  EFI_NO_MEDIA,               L"No Media"},
	{  EFI_MEDIA_CHANGED,          L"Media changed"},
	{  EFI_NOT_FOUND,              L"Not Found"},
	{  EFI_ACCESS_DENIED,          L"Access Denied"},
	{  EFI_NO_RESPONSE,            L"No Response"},
	{  EFI_NO_MAPPING,             L"No mapping"},
	{  EFI_TIMEOUT,                L"Time out"},
	{  EFI_NOT_STARTED,            L"Not started"},
	{  EFI_ALREADY_STARTED,        L"Already started"},
	{  EFI_ABORTED,                L"Aborted"},
	{  EFI_ICMP_ERROR,             L"ICMP Error"},
	{  EFI_TFTP_ERROR,             L"TFTP Error"},
	{  EFI_PROTOCOL_ERROR,         L"Protocol Error"},
	{  EFI_INCOMPATIBLE_VERSION,   L"Incompatible Version"},
	{  EFI_SECURITY_VIOLATION,     L"Security Policy Violation"},
	{  EFI_CRC_ERROR,              L"CRC Error"},
	{  EFI_END_OF_MEDIA,           L"End of Media"},
	{  EFI_END_OF_FILE,            L"End of File"},
	{  EFI_INVALID_LANGUAGE,       L"Invalid Languages"},
	{  EFI_COMPROMISED_DATA,       L"Compromised Data"},

	// warnings
	{  EFI_WARN_UNKOWN_GLYPH,      L"Warning Unknown Glyph"},
	{  EFI_WARN_DELETE_FAILURE,    L"Warning Delete Failure"},
	{  EFI_WARN_WRITE_FAILURE,     L"Warning Write Failure"},
	{  EFI_WARN_BUFFER_TOO_SMALL,  L"Warning Buffer Too Small"},
	{  0, nullptr}
};

class Error
{
public:
	static EFI_STATUS DisplayError(const CHAR16* function, const CHAR16* file, const CHAR16* line, EFI_STATUS status);
	
private:
	static void StatusToString(OUT CHAR16* buffer, IN EFI_STATUS status);
};


