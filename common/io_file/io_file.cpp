#include "io_file.h"
#include <string.h>
#include <locale.h>

//==========================================================================================

int ReadFile(FILE* fp, wchar_t** buffer_ptr, const char* file_path, size_t* buf_size)
{
    assert(buffer_ptr != NULL);
    assert(file_path  != NULL);
    assert(fp         != NULL);

    setlocale(LC_ALL, "en_US.UTF-8");

    size_t size = 0;

    if (CountSize(file_path, &size))
    {
        WPRINTERR(L"CountSize() error");
        return 1;
    }

    char* buffer = (char*) calloc(size, 1);

    if (buffer == NULL)
    {
        WPRINTERR(L"Memory allocation failed");
        return 1;
    }

    size_t bytes_read = fread(buffer, 1, size - 1, fp);

    if (bytes_read != size - 1)
    {
        WPRINTERR(L"Reading file error");
        free(buffer);
        return 1;
    }

    buffer[size - 1] = '\0';

    fprintf(stderr, "buffer (UTF-8):\n\"%s\"\n\n"
                    "------------------------------------\n",
                    buffer);

    size_t wchar_count = mbstowcs(NULL, buffer, 0);

    if (wchar_count == (size_t)-1)
    {
        WPRINTERR(L"Invalid UTF-8 sequence");
        free(buffer);
        return 1;
    }

    WDPRINTF(L"filesize    = %zu bytes;\n"  , size);
    WDPRINTF(L"buflen      = %zu chars;\n"  , strlen(buffer));
    WDPRINTF(L"wchar_count = %zu wchar_t;\n", wchar_count);

    *buffer_ptr = (wchar_t*) calloc(wchar_count + 1, sizeof(wchar_t));
    *buf_size   = wchar_count;

    if (*buffer_ptr == NULL)
    {
        free(buffer);
        WPRINTERR(L"Memory allocation failed");
        return 1;
    }

    WDPRINTF(L"Converting UTF-8 to wchar_t\n");

    if (mbstowcs(*buffer_ptr, buffer, wchar_count + 1) == (size_t)-1)
    {
        free(buffer);
        free(*buffer_ptr);
        *buffer_ptr = NULL;
        WPRINTERR(L"mbtowc conversion failed");
        return 1;
    }

    WDPRINTF(L"Successfully converted UTF-8 to wchar_t\n");
    WDPRINTF(L"buffer_ptr = %p\n", *buffer_ptr);

    fwprintf(stderr, L"Converted wchar_t string:\n\"%ls\"\n\n"
                     L"------------------------------------\n",
                     *buffer_ptr);

    free(buffer);

    return 0;
}

//==========================================================================================

int CountSize(const char* file_path, size_t* size)
{
    struct stat fileinfo = {};

    if (stat(file_path, &fileinfo) == -1)
    {
        PRINTERR("Error with stat()");
        return 1;
    }

    *size = (size_t) fileinfo.st_size + 1;

    return 0;
}

//==========================================================================================

char* GetFileName(char* file_path)
{
    assert(file_path);

    char* format = strchr(file_path, '.');

    if (format)
        *format = '\0';

    char* slash = file_path;
    char* start = NULL;

    while ((slash = strchr(slash, '/')) != NULL)
    {
        slash++;
        start = slash;
    }
    WDPRINTF(L"Start: %s\n", start);

    return start;
}

//==========================================================================================
