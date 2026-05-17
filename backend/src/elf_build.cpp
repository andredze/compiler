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

BackendErr_t ElfCtxCtor(ElfCtx_t* elf_ctx, BackendCtx_t* backend_ctx)
{
    assert(elf_ctx);

    BackendErr_t error = BACKEND_SUCCESS;

    if ((error = StringTableCtor(&elf_ctx->str_table, STRING_TABLE_INIT_CAPACITY)))
    {
        return error;
    }
    if ((error = SymbolTableCtor(&elf_ctx->sym_table, backend_ctx->rel_table.size + 1)))
    {
        return error;
    }

    return BACKEND_SUCCESS;
}

//==========================================================================================

void ElfCtxDtor(ElfCtx_t* elf_ctx)
{
    assert(elf_ctx);

    StringTableDtor(&elf_ctx->str_table);
    SymbolTableDtor(&elf_ctx->sym_table);
}

//==========================================================================================

BackendErr_t BuildElf(BackendCtx_t* backend_ctx)
{
    assert(backend_ctx);

    ElfCtx_t elf_ctx = {};

    BackendErr_t error = BACKEND_SUCCESS;

    if ((error = ElfCtxCtor(&elf_ctx, backend_ctx)))
    {
        ElfCtxDtor(&elf_ctx);
        return error;
    }
    if ((error = BackendOpenElfFile(backend_ctx)))
    {
        ElfCtxDtor(&elf_ctx);
        return error;
    }
    if ((error = ElfBuildStringTable(backend_ctx, &elf_ctx.str_table)))
    {
        ElfCtxDtor(&elf_ctx);
        return error;
    }
    if ((error = ElfBuildSymbolTable(backend_ctx, &elf_ctx.sym_table)))
    {
        ElfCtxDtor(&elf_ctx);
        return error;
    }

    ElfCtxDtor(&elf_ctx);

    return BACKEND_SUCCESS;
}

//==========================================================================================
