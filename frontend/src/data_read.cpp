#include "data_read.h"
#include <wchar.h>
#include <stdlib.h>

//------------------------------------------------------------------------------------------

LangErr_t TreeReadInputData(LangCtx* lang_ctx)
{
    char file_path[MAX_FILENAME_LEN] = {};

    printf("Enter database file path: ");

    if (scanf("%s", file_path) != 1)
    {
        PRINTERR("scanf failed");
        return LANG_INVALID_INPUT;
    }

    getchar();

    return TreeReadData(lang_ctx, file_path);
}

//------------------------------------------------------------------------------------------

LangErr_t TreeReadData(LangCtx* lang_ctx, const char* data_file_path)
{
    // DEBUG_TREE_CHECK(lang_ctx, "ERROR BEFORE TREE READ DATA");

    assert(data_file_path != NULL);

    DPRINTF(L"Reading file %s\n", data_file_path);

    FILE* fp = fopen(data_file_path, "r");

    if (fp == NULL)
    {
        PRINTERR("Error with opening file: %s", data_file_path);
        return LANG_FILE_ERROR;
    }

    if (ReadFile(fp, &lang_ctx->cur_symbol_ptr, data_file_path))
        return LANG_FILE_ERROR;

    lang_ctx->buffer = lang_ctx->cur_symbol_ptr;

    fclose(fp);

    return LANG_SUCCESS;
}

//------------------------------------------------------------------------------------------

int ReadFile(FILE* fp, wchar_t** buffer_ptr, const char* file_path)
{
    assert(buffer_ptr != NULL);
    assert(file_path  != NULL);
    assert(fp         != NULL);

    size_t size = 0;

    if (CountSize(file_path, &size))
    {
        return 1;
    }

    char* buffer = (char*) calloc(size, 1);

    if (buffer == NULL)
    {
        PRINTERR("Memory allocation failed");
        return 1;
    }

    if (fread(buffer, size - 1, 1, fp) != 1)
    {
        PRINTERR("Reading file error");
        return 1;
    }

    buffer[size - 1] = '\0'; /* set null-term */
    WDPRINTF(L"buffer = %s;\n", buffer);

    *buffer_ptr = (wchar_t*) calloc(size, sizeof(wchar_t));

    if (*buffer_ptr == NULL)
    {
        PRINTERR("Memory allocation failed");
        return 1;
    }

    WDPRINTF(L"Converting mb to wc\n");

    if (mbstowcs(*buffer_ptr, buffer, size) == -1)
    {
        PRINTERR("mbtowc conversion failed");
        return 1;
    }
    WDPRINTF(L"Converted mb to wc\n");

    WDPRINTF(L"buffer_ptr = %p\n", *buffer_ptr);
    (*buffer_ptr)[size - 1] = '\0';

    WDPRINTF(L"code = %ls;\n", *buffer_ptr);

    free(buffer);

    return 0;
}

//------------------------------------------------------------------------------------------

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

//------------------------------------------------------------------------------------------
