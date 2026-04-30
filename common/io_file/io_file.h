#ifndef IO_FILE_H
#define IO_FILE_H

//——————————————————————————————————————————————————————————————————————————————————————————

#include "common.h"
#include <wchar.h>
#include <stdio.h>
#include <sys/stat.h>

//——————————————————————————————————————————————————————————————————————————————————————————

int   ReadFile   (FILE* fp, wchar_t** buffer_ptr, const char* file_path, size_t* buf_size);
int   CountSize  (const char* file_path, size_t* size);
char* GetFileName(char* file_path);

//——————————————————————————————————————————————————————————————————————————————————————————

#endif /* IO_FILE_H */
