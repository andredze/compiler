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

    LangErr_t error = LANG_SUCCESS;

    if ((error = TreeReadData(lang_ctx, file_path)))
        return error;

    strcpy(lang_ctx->ast_file_name, GetFileName(file_path));

    return LANG_SUCCESS;
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
