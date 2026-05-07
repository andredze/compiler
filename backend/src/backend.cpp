#include "backend.h"
#include "common.h"

//==========================================================================================

BackendErr_t BackendCtor(BackendCtx_t* backend_ctx)
{
    assert(backend_ctx);

    if (LangCtxCtor())
    {
        
    }

    return BACKEND_SUCCESS;
}

//==========================================================================================

BackendErr_t BackendOpenAsmFile(BackendCtx_t* backend_ctx)
{
    assert(backend_ctx);

    

    return BACKEND_SUCCESS;
}

//==========================================================================================

BackendErr_t BackendOpenAsmFile(BackendCtx_t* backend_ctx)
{
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

    return BACKEND_SUCCESS;
}

//==========================================================================================
