#if defined(CRT_EXPORT)
#define LIBRARY __declspec(dllexport)
#elif defined(CRT_STATIC)
#define LIBRARY 
#else
#define LIBRARY __declspec(dllimport)
#endif

#define SEEK_CUR    1
#define SEEK_END    2
#define SEEK_SET    0

#define stdin 0
#define stdout 1
#define stderr 2

#define BUFSIZ  512

#define EOF    (-1)

#define _FILE_DEFINED
typedef void FILE;

#ifdef __cplusplus
extern "C"
{
#endif

LIBRARY FILE* fopen(char const* _FileName, char const* _Mode);
LIBRARY size_t fread(void* _Buffer, size_t _ElementSize, size_t _ElementCount, FILE* _Stream);
LIBRARY size_t fwrite(void const* _Buffer, size_t _ElementSize, size_t _ElementCount, FILE* _Stream);
LIBRARY int fseek(FILE* _Stream, long _Offset, int _Origin);
LIBRARY int ftell(FILE* _Stream);
LIBRARY int fclose(FILE* _Stream);

LIBRARY int fprintf(FILE* const _Stream, char const* const _Format, ...);

LIBRARY int rename(char const* _OldFileName, char const* _NewFileName);
LIBRARY int remove(char const* _FileName);

LIBRARY int printf(char const* const _Format, ...);

#ifdef __cplusplus
}
#endif
