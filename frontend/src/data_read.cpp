#include "data_read.h"
#include "frontend.h"
#include <wchar.h>
#include <stdlib.h>

//==========================================================================================

LangErr_t FrontendReadInputInTree(FrontendCtx* frontend_ctx, char* input_file_path)
{
    assert(frontend_ctx);
    assert(input_file_path);

    LangErr_t error = LANG_SUCCESS;

    if ((error = TreeReadData(frontend_ctx, input_file_path)))
        return error;

    strcpy(frontend_ctx->lang_ctx.src_code_file_name, GetFileName(input_file_path));

    return LANG_SUCCESS;
}

//==========================================================================================

LangErr_t TreeReadData(FrontendCtx* frontend_ctx, const char* data_file_path)
{
    assert(data_file_path != NULL);

    DPRINTF(L"Reading file %s\n", data_file_path);

    FILE* fp = fopen(data_file_path, "r");

    if (fp == NULL)
    {
        PRINTERR("Error with opening file: %s", data_file_path);
        return LANG_FILE_ERROR;
    }

    if (ReadFile(fp, &frontend_ctx->cur_symbol_ptr, data_file_path, 
                 &frontend_ctx->buffer_size))
    {
        return LANG_FILE_ERROR;
    }

    frontend_ctx->buffer = frontend_ctx->cur_symbol_ptr;
    WDPRINTF(L"frontend_ctx->buffer = %p\n", frontend_ctx->buffer);

    fclose(fp);

    return LANG_SUCCESS;
}

//==========================================================================================
