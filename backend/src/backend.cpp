#include "backend.h"
#include "lang_funcs.h"
#include "common.h"

//==========================================================================================

BackendErr_t BackendOpenAsmFile(BackendCtx_t* backend_ctx)
{
    DPRINT_FUNC_ENTER_MSG();
    assert(backend_ctx);

    char asm_file_path[MAX_FILE_NAME_LEN] = {};

    snprintf(asm_file_path, sizeof(asm_file_path), "asm/%s.asm", 
             backend_ctx->ast_file_name);

    WDPRINTF(L"Asm file name: %s\n", backend_ctx->ast_file_name);
    WDPRINTF(L"Opening file %s\n\n", asm_file_path);

    FILE* asm_fp = fopen(asm_file_path, "w");

    if (asm_fp == NULL)
    {
        WPRINTERR(L"Failed opening file %s", asm_file_path);        
        return BACKEND_FILE_ERROR;
    }

    backend_ctx->asm_file = asm_fp;

    DPRINT_FUNC_LEAVE_MSG();
    return BACKEND_SUCCESS;
}

//==========================================================================================

BackendErr_t BackendCtxCtor(BackendCtx_t* backend_ctx)
{
    DPRINT_FUNC_ENTER_MSG();
    assert(backend_ctx);

    if (LangCtxCtor(&backend_ctx->lang_ctx))
    {
        return BACKEND_LANG_ERROR;
    }
    if (BinCodeCtor(&backend_ctx->bin_code, BIN_CODE_INIT_CAPACITY))
    {
        return BACKEND_BINCODE_BUFFER_ERROR;
    }

    backend_ctx->endif_labels_count = 0;
    backend_ctx->while_labels_count = 0;

    DPRINT_FUNC_LEAVE_MSG();
    return BACKEND_SUCCESS;
}

//==========================================================================================

BackendErr_t BackendCtxDtor(BackendCtx_t* backend_ctx)
{
    DPRINT_FUNC_ENTER_MSG();
    assert(backend_ctx);

    // zero the string so it can not be used
    memset(backend_ctx->ast_file_name, 0, sizeof(backend_ctx->ast_file_name));

    backend_ctx->endif_labels_count = 0;
    backend_ctx->while_labels_count = 0;

    if (backend_ctx->asm_file)
    {
        fclose(backend_ctx->asm_file);
        backend_ctx->asm_file = NULL;
    }

    LangCtxDtor(&backend_ctx->lang_ctx);
    BinCodeDtor(&backend_ctx->bin_code);

    DPRINT_FUNC_LEAVE_MSG();
    return BACKEND_SUCCESS;
}

//==========================================================================================
