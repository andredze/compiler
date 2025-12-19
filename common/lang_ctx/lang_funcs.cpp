#include "lang_funcs.h"

//==========================================================================================

//FIXME - stack realloc дропается дамп, скорее всего реаллок не работает

//——————————————————————————————————————————————————————————————————————————————————————————

LangErr_t LangCtxCtor(LangCtx_t* lang_ctx)
{
    assert(lang_ctx != NULL);

    LangErr_t error = LANG_SUCCESS;

#ifdef FRONTEND
    if (StackCtor(&lang_ctx->tokens, 256))
    {
        WPRINTERR(L"Tokens stack construct failed");
        return LANG_STACK_ERROR;
    }

    lang_ctx->cur_symbol_ptr = NULL;
    lang_ctx->current_line   = 1;

#endif /* FRONTEND */

    if ((error = LangNamesPoolCtor(&lang_ctx->names_pool)))
        return error;

#ifdef BACKEND
    lang_ctx->first_point = true;

    if ((error = LangIdTableCtor(&lang_ctx->main_id_table)))
        return error;

    lang_ctx->getting_function_params = false;

#endif /* BACKEND */

    if (TreeCtor(&lang_ctx->tree))
    {
        WPRINTERR(L"Language tree construct failed");
        return LANG_TREE_ERROR;
    }

    if (TreeOpenLogFile(lang_ctx))
        return LANG_TREE_ERROR;

    return LANG_SUCCESS;
}

//==========================================================================================

void LangCtxDtor(LangCtx_t* lang_ctx)
{
    assert(lang_ctx);

#ifdef FRONTEND

    TreeSingleNodeDtor(lang_ctx->tree.dummy, &lang_ctx->tree);

    for (size_t i = 0; i < lang_ctx->tokens.size; i++)
    {
        WDPRINTF(L"freed token = %p\n", lang_ctx->tokens.data[i]);
        free(lang_ctx->tokens.data[i]);
    }
    StackDtor(&lang_ctx->tokens);

#endif /* FRONTEND */

#ifdef BACKEND

    TreeDtor(&lang_ctx->tree);

    LangIdTableDtor  (&lang_ctx->main_id_table);
    // LangIdTableDtor  (&lang_ctx->func_id_table);

#endif /* BACKEND */

#ifdef REVERSE
    TreeDtor(&lang_ctx->tree);
#endif /* REVERSE */

    if (lang_ctx->output_file != NULL)
        fclose(lang_ctx->output_file);

    LangNamesPoolDtor(&lang_ctx->names_pool);

    free(lang_ctx->buffer);

    lang_ctx->cur_symbol_ptr = NULL;
    lang_ctx->buffer         = NULL;

    TreeCloseLogFile(lang_ctx);
}

//==========================================================================================

LangErr_t LangOpenAsmFile(LangCtx_t* lang_ctx)
{
    assert(lang_ctx);

    char asm_file_path[MAX_FILE_NAME_LEN];

    snprintf(asm_file_path, sizeof(asm_file_path), "asm/%s.asm", lang_ctx->ast_file_name);

    WDPRINTF(L"Asm file name: %s\n", lang_ctx->ast_file_name);
    WDPRINTF(L"Opening file %s\n\n",   asm_file_path);

    FILE* asm_fp = fopen(asm_file_path, "w");

    if (asm_fp == NULL)
    {
        WPRINTERR(L"Failed opening file %s", asm_file_path);
        return LANG_FILE_ERROR;
    }

#ifdef BACKEND
    lang_ctx->output_file = asm_fp;
#endif /* BACKEND */

    return LANG_SUCCESS;
}

//==========================================================================================

LangErr_t LangOpenReverseFile(LangCtx_t* lang_ctx)
{
    assert(lang_ctx);

    char src_file_path[MAX_FILE_NAME_LEN];

    snprintf(src_file_path, sizeof(src_file_path), "reversed/%s.psy", lang_ctx->ast_file_name);

    WDPRINTF(L"Reversed code file name: %s\n", lang_ctx->ast_file_name);
    WDPRINTF(L"Opening file %s\n\n", src_file_path);

    FILE* src_fp = fopen(src_file_path, "w");

    if (src_fp == NULL)
    {
        WPRINTERR(L"Failed opening file %s", src_file_path);
        return LANG_FILE_ERROR;
    }

    lang_ctx->output_file = src_fp;

    return LANG_SUCCESS;
}

//==========================================================================================

LangErr_t LangNamesPoolCtor(NamesPool_t* names_pool)
{
    assert(names_pool);

    size_t capacity = DEFAULT_NAMES_POOL_CAPACITY;

    names_pool->data = (wchar_t**) calloc(capacity, sizeof(wchar_t*));

    if (names_pool->data == NULL)
    {
        WPRINTERR(L"Memory allocation failed");
        return LANG_MEMALLOC_ERROR;
    }

    names_pool->capacity = capacity;
    names_pool->size     = 0;

    return LANG_SUCCESS;
}

//==========================================================================================

void LangNamesPoolDtor(NamesPool_t* names_pool)
{
    assert(names_pool);

    for (size_t i = 0; i < names_pool->size; i++)
    {
        free(names_pool->data[i]);
        names_pool->data[i] = NULL;
    }

    names_pool->size     = 0;
    names_pool->capacity = 0;

    free(names_pool->data);
    names_pool->data = NULL;

    WDPRINTF(L"------- NamesPool destroyed -------\n");
}

//==========================================================================================

static LangErr_t LangNamesPoolRealloc(NamesPool_t* names_pool);

//——————————————————————————————————————————————————————————————————————————————————————————

LangErr_t LangNamesPoolPush(NamesPool_t* names_pool, const wchar_t* name_buf, size_t* name_index)
{
    assert(name_index != NULL);
    assert(names_pool != NULL);
    assert(name_buf   != NULL);

    for (size_t i = 0; i < names_pool->size; i++)
    {
        if (wcscmp(names_pool->data[i], name_buf) == 0)
        {
            *name_index = i;
            return LANG_SUCCESS;
        }
    }

    wchar_t* name = wcsdup(name_buf);

    if (name == NULL)
    {
        PRINTERR("Memory allocation failed");
        return LANG_MEMALLOC_ERROR;
    }

    LangErr_t error = LANG_SUCCESS;

    if (names_pool->size >= names_pool->capacity)
    {
        if ((error = LangNamesPoolRealloc(names_pool)))
            return error;
    }

    *name_index = names_pool->size;
    names_pool->data[names_pool->size++] = name;

    return LANG_SUCCESS;
}

//==========================================================================================

static LangErr_t LangNamesPoolRealloc(NamesPool_t* names_pool)
{
    assert(names_pool);

    size_t new_capacity = names_pool->capacity * 2 + 1;

    wchar_t** new_data = (wchar_t**) realloc(names_pool->data, new_capacity * sizeof(*names_pool->data));

    if (new_data == NULL)
    {
        WPRINTERR(L"Memory reallocation failed");
        return LANG_MEMALLOC_ERROR;
    }

    names_pool->data     = new_data;
    names_pool->capacity = new_capacity;

    return LANG_SUCCESS;
}

//==========================================================================================

#ifdef BACKEND

LangErr_t LangIdTableCtor(IdTable_t* id_table)
{
    assert(id_table);

    size_t capacity = DEFAULT_ID_TABLE_CAPACITY;

    id_table->data = (IdData_t*) calloc(capacity, sizeof(IdData_t));

    if (id_table->data == NULL)
    {
        WPRINTERR(L"Memory allocation failed");
        return LANG_MEMALLOC_ERROR;
    }

    id_table->capacity = capacity;
    id_table->size     = 0;

    return LANG_SUCCESS;
}

//==========================================================================================

void LangIdTableDtor(IdTable_t* id_table)
{
    assert(id_table);

    for (size_t i = 0; i < id_table->size; i++)
    {
        id_table->data[i].name_index = (size_t)-1;
        id_table->data[i].type       = ID_TYPE_UNKNOWN;
        id_table->data[i].addr       = (size_t)-1;
    }

    id_table->size     = 0;
    id_table->capacity = 0;

    free(id_table->data);
    id_table->data = NULL;

    WDPRINTF(L"----- IdTable destroyed -----\n");
}

//——————————————————————————————————————————————————————————————————————————————————————————

static LangErr_t LangIdTableRealloc(IdTable_t* id_table);

//——————————————————————————————————————————————————————————————————————————————————————————

LangErr_t LangIdTablePush(LangCtx_t* lang_ctx, IdTable_t* id_table, Identifier_t id,
                          IdType_t type, size_t n_params)
{
    assert(lang_ctx != NULL);
    assert(id_table != NULL);

    LangErr_t error = LANG_SUCCESS;

    if (id_table->size >= id_table->capacity)
    {
        if ((error = LangIdTableRealloc(id_table)))
            return error;
    }

    id_table->data[id_table->size].name_index = id;
    id_table->data[id_table->size].type       = type;

    if (type == ID_TYPE_VARIABLE)
    {
        id_table->data[id_table->size].addr = lang_ctx->cur_addr;
        lang_ctx->cur_addr++;
    }
    else
    {
        id_table->data[id_table->size].n_params = n_params;
    }

    id_table->size++;

    return LANG_SUCCESS;
}

//==========================================================================================

bool LangGetIdInTable(IdTable_t* id_table, Identifier_t id, size_t* id_index)
{
    assert(id_table);

    for (size_t i = 0; i < id_table->size; i++)
    {
        if (id_table->data[i].name_index == id)
        {
            *id_index = i;
            return true;
        }
    }

    return false;
}

//==========================================================================================

bool LangIdInTable(IdTable_t* id_table, Identifier_t id)
{
    assert(id_table);

    for (size_t i = 0; i < id_table->size; i++)
    {
        if (id_table->data[i].name_index == id)
            return true;
    }

    return false;
}

//==========================================================================================

LangErr_t LangCheckVariableIsNotFunction(IdTable_t* id_table, Identifier_t id)
{
    assert(id_table);

    for (size_t i = 0; i < id_table->size; i++)
    {
        if (id_table->data[i].name_index == id && id_table->data[i].type != ID_TYPE_VARIABLE)
        {
            return LANG_FUNC_USED_AS_VAR;
        }
    }

    return LANG_SUCCESS;
}

//==========================================================================================

LangErr_t LangIdTableGetAddress(IdTable_t* id_table, Identifier_t id, int* addr)
{
    assert(id_table);
    assert(addr);

    for (size_t i = 0; i < id_table->size; i++)
    {
        if (id_table->data[i].name_index == id)
        {
            *addr = id_table->data[i].addr;
            return LANG_SUCCESS;
        }
    }

    return LANG_VAR_NOT_DECLARED;
}

//==========================================================================================

static LangErr_t LangIdTableRealloc(IdTable_t* id_table)
{
    assert(id_table);

    size_t new_capacity = id_table->capacity * 2 + 1;

    IdData_t* new_data = (IdData_t*) realloc(id_table->data, new_capacity * sizeof(*id_table->data));

    if (new_data == NULL)
    {
        WPRINTERR(L"Memory reallocation failed");
        return LANG_MEMALLOC_ERROR;
    }

    id_table->data     = new_data;
    id_table->capacity = new_capacity;

    return LANG_SUCCESS;
}

#endif /* BACKEND */

//==========================================================================================

TreeNode_t* LangGetCurrentToken(LangCtx_t* lang_ctx)
{
    assert(lang_ctx);

    TreeNode_t* token = NULL;

    if (StackGetElement(&lang_ctx->tokens, lang_ctx->cur_token_index, &token))
    {
        PRINTERR("Error with getting token from stack");
        return NULL;
    }

    return token;
}

//==========================================================================================
