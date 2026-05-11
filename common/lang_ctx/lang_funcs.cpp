#include "lang_funcs.h"
#include "op_cases.h"

//==========================================================================================

const wchar_t* GetOpName(Operator_t opcode)
{
    if (!(0 <= opcode && opcode < OPERATORS_COUNT))
        return NULL;

    return OP_CASES_TABLE[opcode].name;
}

//==========================================================================================

LangErr_t LangCtxCtor(LangCtx_t* lang_ctx)
{
    assert(lang_ctx);

    LangErr_t error = LANG_SUCCESS;

    if ((error = LangIdTableCtor(&lang_ctx->main_id_table)))
    {    
        return error;
    }
    if ((error = LangNamesPoolCtor(&lang_ctx->names_pool)))
    {    
        return error;
    }

    if (TreeCtor(&lang_ctx->tree))
    {
        WPRINTERR(L"Language tree construct failed");
        return LANG_TREE_ERROR;
    }

    WDPRINTF(L"Tree logfile opening\n");

    if (TreeOpenLogFile(lang_ctx))
    {
        return LANG_TREE_ERROR;
    }

    WDPRINTF(L"tree_ptr = %p\n", lang_ctx->tree);

    return LANG_SUCCESS;
}

//==========================================================================================

void LangCtxDtor(LangCtx_t* lang_ctx)
{
    assert(lang_ctx);

    LangIdTableDtor(&lang_ctx->main_id_table);

    LangNamesPoolDtor(&lang_ctx->names_pool);

#ifndef FRONTEND
    // In frontend tree nodes are freed from tokens stack

    TreeDtor(&lang_ctx->tree);

#endif /* FRONTEND */

    TreeCloseLogFile(lang_ctx);
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

LangErr_t LangNamesPoolPush(NamesPool_t*    names_pool, 
                            const wchar_t*  name_buf, 
                            size_t*         name_index)
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

wchar_t* LangGetIdName(NamesPool_t* names_pool, Identifier_t index)
{
    assert(names_pool);

    if (!(index <= names_pool->size))
    {
        return NULL;
    }

    return names_pool->data[index];
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

void LangIdTableDump(IdTable_t* id_table)
{
    DPRINT_FUNC_ENTER_MSG();
    assert(id_table);

    wprintf(L"\n---------dumping id_table %p-----------\n", id_table);
    wprintf(L".size = %zu\n", id_table->size);
    wprintf(L".cap  = %zu\n", id_table->capacity);
    wprintf(L".data = %p\n",  id_table->data);

    IdData_t* id_data = NULL;

    for (size_t i = 0; i < id_table->size; i++)
    {
        id_data = &id_table->data[i];

        if (id_data->name == NULL)
        {
            continue;
        }

        wprintf(L"{%zu, %ls, %d, %zu, %zu, addr = %d}\n",
                id_data->name_index,
                id_data->name,
                id_data->type,
                id_data->memory_needed,
                id_data->n_params,
                id_data->addr);
    }

    wprintf(L"---------------------------------------\n");
    DPRINT_FUNC_LEAVE_MSG();
}

//==========================================================================================

void LangIdTableDtor(IdTable_t* id_table)
{
    assert(id_table);

    for (size_t i = 0; i < id_table->size; i++)
    {
        id_table->data[i].name_index = (size_t)-1;
        id_table->data[i].type       = ID_TYPE_UNKNOWN;
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

LangErr_t LangIdTablePush(IdTable_t* id_table, IdData_t* id_data)
{
    assert(id_table != NULL);
    assert(id_data  != NULL);

    LangErr_t error = LANG_SUCCESS;

    if (id_table->size >= id_table->capacity)
    {
        if ((error = LangIdTableRealloc(id_table)))
            return error;
    }

    id_table->data[id_table->size] = *id_data;
    id_table->size++;

    return LANG_SUCCESS;
}

//==========================================================================================

LangErr_t LangSafePushIdTable(LangCtx_t* lang_ctx, IdTable_t* id_table, IdData_t* id_data)
{
    if (LangIdInTable(id_table, id_data->name_index))
    {
        WPRINTERR(L"Variable %ls was already declared\n",
                    lang_ctx->names_pool.data[id_data->name_index]);
        return LANG_VAR_REDECLARATION;
    }

    LangErr_t error = LANG_SUCCESS;

    if ((error = LangIdTablePush(id_table, id_data)))
    {
        return error;
    }

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

LangErr_t LangGetFuncIndex(LangCtx_t* lang_ctx, Identifier_t id, size_t* func_id_index)
{
    assert(lang_ctx);

    size_t id_index = 0;

    if (LangGetIdInTable(&lang_ctx->main_id_table, id, &id_index) == true)
    {
        if (lang_ctx->main_id_table.data[id_index].type == ID_TYPE_FUNCTION)
        {
            *func_id_index = id_index;
            return LANG_SUCCESS;
        }
    }

    return LANG_FUNC_NOT_DECLARED;
}

//==========================================================================================

LangErr_t LangGetIdData(IdTable_t* id_table, size_t index, IdData_t* id_data)
{
    assert(id_table != NULL);

    if (id_table->size <= index)
    {
        return LAND_ID_TABLE_WRONG_INDEX;
    }

    *id_data = id_table->data[index];

    return LANG_SUCCESS;
}

//==========================================================================================

size_t LangIdTableCountVars(IdTable_t* id_table)
{
    assert(id_table);

    size_t global_vars_count = 0;

    for (size_t i = 0; i < id_table->size; i++)
    {
        if (id_table->data[i].type == ID_TYPE_VARIABLE)
        {
            global_vars_count++;
        }
    }

    return global_vars_count;
}

//==========================================================================================

LangErr_t LangIsFuncCallArgsCorrect(LangCtx_t* lang_ctx, 
                                    size_t     func_id_index, 
                                    int        args_count)
{
    assert(lang_ctx);

    IdData_t id_data = {};
    LangErr_t error = LANG_SUCCESS;

    if ((error = LangGetIdData(&lang_ctx->main_id_table, func_id_index, &id_data)))
    {
        return error;
    }

    if (args_count != id_data.n_params)
    {
        WPRINTERR(L"WRONG AMOUNT OF ARGS for function %ls\n",
                  lang_ctx->names_pool.data[id_data.name_index]);
        return LANG_WRONG_ARGS_COUNT;
    }

    return LANG_SUCCESS;
}

//==========================================================================================

bool LangFuncWasDeclared(LangCtx_t* lang_ctx, Identifier_t id)
{
    assert(lang_ctx);

    size_t id_index = 0;

    if (LangGetIdInTable(&lang_ctx->main_id_table, id, &id_index) == true)
    {
        if (lang_ctx->main_id_table.data[id_index].type == ID_TYPE_FUNCTION)
        {
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

//==========================================================================================
