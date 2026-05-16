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

    BackendErr_t error = BACKEND_SUCCESS;

    if (LangCtxCtor(&backend_ctx->lang_ctx))
    {
        return BACKEND_LANG_ERROR;
    }
    if (BinCodeCtor(&backend_ctx->bin_code, BIN_CODE_INIT_CAPACITY))
    {
        return BACKEND_BINCODE_BUFFER_ERROR;
    }
    if ((error = RelTableCtor(&backend_ctx->rel_table)))
    {
        return error;
    }

    backend_ctx->endif_labels_count            = 0;
    backend_ctx->while_labels_count            = 0;
    backend_ctx->current_stack_local_vars_size = 0;
    backend_ctx->main_node                     = NULL;

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

    backend_ctx->endif_labels_count            = 0;
    backend_ctx->while_labels_count            = 0;
    backend_ctx->current_stack_local_vars_size = 0;
    backend_ctx->main_node                     = NULL;

    if (backend_ctx->asm_file)
    {
        fclose(backend_ctx->asm_file);
        backend_ctx->asm_file = NULL;
    }

    LangCtxDtor (&backend_ctx->lang_ctx);
    BinCodeDtor (&backend_ctx->bin_code);
    RelTableDtor(&backend_ctx->rel_table);

    DPRINT_FUNC_LEAVE_MSG();
    return BACKEND_SUCCESS;
}

//==========================================================================================

BackendErr_t BackendGetFuncIdTableIdData(BackendCtx_t* backend_ctx, 
                                         size_t        index, 
                                         IdData_t**    id_data_p)
{
    assert(backend_ctx);
    assert(id_data_p);

    IdTable_t* table = &backend_ctx->lang_ctx.func_id_table;

    if (index >= table->size)
    {
        WPRINTERR(L"Index exceeds id_table size");
        return BACKEND_WRONG_ID_TABLE_INDEX;
    }

    *id_data_p = &table->data[index];

    return BACKEND_SUCCESS;
}

//==========================================================================================

BackendErr_t BackendGetVarsStackSize(BackendCtx_t* backend_ctx, size_t func_index, size_t* dst)
{
    assert(backend_ctx);

    BackendErr_t error   = BACKEND_SUCCESS;
    IdData_t*    id_data = NULL;    

    if ((error = BackendGetFuncIdTableIdData(backend_ctx, func_index, &id_data)))
    {
        return error;
    }
    if (id_data->type != ID_TYPE_FUNCTION)
    {
        WPRINTERR(L"Requested func index %d for %ls doesn't point to a func in table",
                  func_index, id_data->name);
        return BACKEND_WRONG_ID_TABLE_INDEX;
    }

    *dst = id_data->n_local_vars * STACK_ELEMENT_SIZE;

    return BACKEND_SUCCESS;
}

//==========================================================================================

BackendErr_t BackendGetArgsStackSize(BackendCtx_t* backend_ctx, size_t func_index, size_t* dst)
{
    assert(backend_ctx);

    BackendErr_t error   = BACKEND_SUCCESS;
    IdData_t*    id_data = NULL;    

    if ((error = BackendGetFuncIdTableIdData(backend_ctx, func_index, &id_data)))
    {
        return error;
    }
    if (id_data->type != ID_TYPE_FUNCTION)
    {
        WPRINTERR(L"Requested func index %d for %ls doesn't point to a func in table",
                  func_index, id_data->name);
        return BACKEND_WRONG_ID_TABLE_INDEX;
    }

    *dst = id_data->n_params * STACK_ELEMENT_SIZE;

    return BACKEND_SUCCESS;
}

//==========================================================================================

wchar_t* BackendGetIdName(BackendCtx_t* backend_ctx, TreeNode_t* node)
{
    assert(backend_ctx);
    assert(node);

    return LangGetIdName(&backend_ctx->lang_ctx.names_pool, node->data.value.id.name_index);
}

//==========================================================================================

BackendErr_t BackendGetVariableOffset(BackendCtx_t* backend_ctx, 
                                      size_t        id_index, 
                                      int*          offset)
{
    assert(backend_ctx);
    assert(offset);

    BackendErr_t error   = BACKEND_SUCCESS;
    IdData_t*    id_data = NULL;    

    if ((error = BackendGetFuncIdTableIdData(backend_ctx, id_index, &id_data)))
    {
        return error;
    }
    if (!((id_data->type == ID_TYPE_VARIABLE) || (id_data->type == ID_TYPE_PARAMETER)))
    {
        WPRINTERR(L"Requested var index %d for %ls doesn't point to a var or param in table",
                  id_index, id_data->name);
        return BACKEND_WRONG_ID_TABLE_INDEX;
    }

    *offset = id_data->addr;

    return BACKEND_SUCCESS;
}

//==========================================================================================
