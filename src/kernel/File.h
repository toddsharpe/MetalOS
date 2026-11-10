#pragma once

#include "kernel/Types.h"

class FileSystem;

//Where paths resolve: FAT32 on the boot disk
void KeMountRoot(FileSystem& fileSystem);

//The first disk's partitions from its GPT, and its EFI system partition as the FAT32 root
void KeMountStorage();

bool KeCreateFile(KFile& file, const char* const path, const KFileAccess access);
bool KeReadFile(KFile& file, void* buffer, const size_t bufferSize, size_t* bytesRead);
bool KeSetFilePosition(KFile& file, const size_t position);
void* KeLoadFile(const char* const path); //Null if the file is missing or empty, or there's no memory for it
