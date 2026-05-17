#include "elf_build.h"

//==========================================================================================

static BackendErr_t BackendOpenElfFile(BackendCtx_t* backend_ctx)
{
    DPRINT_FUNC_ENTER_MSG();
    assert(backend_ctx);

    char elf_file_path[MAX_FILE_NAME_LEN] = {};

    snprintf(elf_file_path, sizeof(elf_file_path), "elf/%s", 
             backend_ctx->ast_file_name);

    WDPRINTF(L"Opening file %s\n\n", elf_file_path);

    FILE* elf_fp = fopen(elf_file_path, "w");

    if (elf_fp == NULL)
    {
        WPRINTERR(L"Failed opening file %s", elf_file_path);        
        return BACKEND_FILE_ERROR;
    }

    backend_ctx->elf_file = elf_fp;

    DPRINT_FUNC_LEAVE_MSG();
    return BACKEND_SUCCESS;
}

//==========================================================================================

BackendErr_t BuildElf(BackendCtx_t* backend_ctx)
{
    assert(backend_ctx);

    BackendErr_t error = BACKEND_SUCCESS;

    if ((error = BackendOpenElfFile(backend_ctx)))
    {
        return error;
    }
    if ((error = StringTableCtor(&backend_ctx->str_table, STRING_TABLE_INIT_CAPACITY)))
    {
        StringTableDtor(&backend_ctx->str_table);
        return error;
    }
    if ((error = ElfBuildStringTable(backend_ctx)))
    {
        StringTableDtor(&backend_ctx->str_table);
        return error;
    }

    StringTableDtor(&backend_ctx->str_table);

    return BACKEND_SUCCESS;
}

//==========================================================================================
