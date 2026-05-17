#include "lang_funcs.h"
#include "keyword_cases.h"

//==========================================================================================

const wchar_t* GetKeywordName(Keyword_t keyword)
{
    if (!(0 <= keyword && keyword < KEYWORDS_COUNT))
        return NULL;

    return KEYWORD_CASES_TABLE[keyword].name;
}

//==========================================================================================

LangErr_t LangCtxCtor(LangCtx_t* lang_ctx)
{
    assert(lang_ctx);

    LangErr_t error = LANG_SUCCESS;

    if ((error = LangIdTableCtor(&lang_ctx->func_id_table)))
    {    
        return error;
    }

#ifdef FRONTEND

    if ((error = LangIdTableCtor(&lang_ctx->main_id_table)))
    {
        return error;
    }

    IdData_t id_data = {.name_index   = (size_t)-1, 
                        .name         = L"main",
                        .type         = ID_TYPE_FUNCTION,
                        .n_local_vars = 0,
                        .n_params     = 0};

    if ((error = LangIdTablePush(&lang_ctx->main_id_table, &id_data, NULL)))
    {
        return error;
    }

#endif /* FRONTEND */

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

#ifdef FRONTEND
    LangIdTableDtor(&lang_ctx->main_id_table);
#endif /* FRONTEND */

    LangIdTableDtor(&lang_ctx->func_id_table);

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

LangErr_t LangNamesPoolPush(NamesPool_t*   names_pool, 
                            const wchar_t* name_buf, 
                            size_t*        name_index)
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

wchar_t* LangGetIdName(NamesPool_t* names_pool, size_t name_index)
{
    assert(names_pool);

    if (!(name_index <= names_pool->size))
    {
        return NULL;
    }

    return names_pool->data[name_index];
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

    id_table->capacity         = capacity;
    id_table->size             = 0;
    id_table->current_function = 0;

    return LANG_SUCCESS;
}

//==========================================================================================

void LangIdTableDtor(IdTable_t* id_table)
{
    assert(id_table);

    for (size_t i = 0; i < id_table->size; i++)
    {
        id_table->data[i].name_index   = (size_t)-1;
        id_table->data[i].name         = NULL;
        id_table->data[i].type         = ID_TYPE_UNKNOWN;
        id_table->data[i].n_local_vars = (size_t)-1;
        id_table->data[i].n_params     = (size_t)-1;
        id_table->data[i].addr         = 0;
    }

    id_table->current_function = 0;
    id_table->size             = 0;
    id_table->capacity         = 0;

    free(id_table->data);
    id_table->data = NULL;

    WDPRINTF(L"----- IdTable destroyed -----\n");
}

//==========================================================================================

LangErr_t LangGetIdData (IdTable_t* id_table, size_t index, IdData* id_data)
{
    assert(id_table);
    assert(id_data);

    if (index >= id_table->size)
    {
        WPRINTERR(L"Index exceeds id_table->size");
        return LANG_ID_TABLE_WRONG_INDEX;
    }

    *id_data = id_table->data[index];

    return LANG_SUCCESS;
}

//——————————————————————————————————————————————————————————————————————————————————————————

static LangErr_t LangIdTableRealloc(IdTable_t* id_table);

//——————————————————————————————————————————————————————————————————————————————————————————

LangErr_t LangIdTablePush(IdTable_t* id_table, IdData_t* id_data, int* dest_index_p)
{
    DPRINT_FUNC_ENTER_MSG();
    assert(id_table != NULL);
    assert(id_data  != NULL);

    LangErr_t error = LANG_SUCCESS;

    if (id_table->size >= id_table->capacity)
    {
        if ((error = LangIdTableRealloc(id_table)))
            return error;
    }

    if (id_data->type == ID_TYPE_FUNCTION)
    {
        id_table->current_function = id_table->size;
    }

    id_table->data[id_table->size] = *id_data;
    
    if (dest_index_p)
    {
        *dest_index_p = (int) id_table->size;
    }

    id_table->size++;

    DPRINT_FUNC_LEAVE_MSG();
    return LANG_SUCCESS;
}

//==========================================================================================

void LangIdDataBuild(LangCtx_t*   lang_ctx, 
                     IdData_t*    id_data, 
                     size_t       name_index,
                     size_t       n_local_vars,
                     size_t       n_params,
                     IdType_t     id_type)
{
    assert(lang_ctx);
    assert(id_data);

    id_data->name_index   = name_index;
    id_data->name         = LangGetIdName(&lang_ctx->names_pool, name_index);
    id_data->type         = id_type;
    id_data->n_local_vars = n_local_vars;
    id_data->n_params     = n_params;
    id_data->addr         = 0;
}

//==========================================================================================

bool LangIdTableFuncIsDeclared(IdTable_t* id_table, size_t name_index)
{
    assert(id_table);

    for (size_t i = 0; i < id_table->size; i++)
    {
        if (id_table->data[i].name_index == name_index &&
            id_table->data[i].type == ID_TYPE_FUNCTION)
        {
            return true;
        }
    }

    return false;
}

//==========================================================================================

int LangIdTableGetFuncTableIndex(IdTable_t* id_table, size_t name_index)
{
    assert(id_table);

    for (size_t i = 0; i < id_table->size; i++)
    {
        if (id_table->data[i].name_index == name_index &&
            id_table->data[i].type == ID_TYPE_FUNCTION)
        {
            return (int) i;
        }
    }

    return -1;
}

//==========================================================================================

LangErr_t LangIsFuncCallArgsCorrect(LangCtx_t* lang_ctx, int func_index, int args_count)
{
    assert(lang_ctx);

    IdData_t  id_data = {};
    LangErr_t error   = LANG_SUCCESS; 

    if ((error = LangGetIdData(&lang_ctx->func_id_table, (size_t) func_index, &id_data)))
    {
        return error;
    }
    if (args_count != (int) id_data.n_params)
    {
        WPRINTERR(L"Given %d arguments for %ls, but %zu expected (func_index %d)",
                  args_count, id_data.name, id_data.n_params, func_index);
        return LANG_WRONG_ARGS_COUNT;
    }

    return LANG_SUCCESS;
}

//==========================================================================================

int LangIdTableVarGetTableIndex(IdTable_t* id_table, size_t name_index)
{
    assert(id_table);

    size_t scope_start = id_table->current_function + 1;
    size_t table_size  = id_table->size;

    for (size_t i = scope_start; i < table_size; i++)
    {
        if (id_table->data[i].name_index == name_index)
        {
            return (int) i;
        }
    }

    return -1;
}

//==========================================================================================

LangErr_t LangIdTablePushVariableIfUnique(LangCtx_t*   lang_ctx, 
                                          IdTable_t*   id_table,  
                                          size_t       name_id,
                                          IdType_t     type,
                                          int*         table_index)
{
    // type can be variable or parameter
    assert(lang_ctx);
    assert(id_table);
    assert(table_index);
    
    IdData_t id_data = {};

    LangIdDataBuild(lang_ctx, &id_data, name_id, 0, 0, type);

    if (LangIdTableFuncIsDeclared(&lang_ctx->func_id_table, name_id))
    {
        WPRINTERR(L"%ls was already declared as a function; "
                  L"can not declare a variable", id_data.name);
        return LANG_FUNC_USED_AS_VAR;
    }
    if (LangIdTableVarGetTableIndex(id_table, name_id) != -1)
    {
        WPRINTERR(L"variable %ls was already declared in the scope; "
                  L"can not declare a variable", id_data.name);
        return LANG_VAR_REDECLARATION;
    }

    return LangIdTablePush(id_table, &id_data, table_index);
}

//==========================================================================================

bool LangIdTableIdentifierIsDeclared(IdTable_t* id_table, size_t name_index)
{
    assert(id_table);

    size_t size = id_table->size;

    for (size_t i = 0; i < size; i++)
    {
        if (id_table->data[i].name_index == name_index)
        {
            return true;
        }
    }

    return false;
}

//==========================================================================================

LangErr_t LangIdTableFunctionSetParamsLocals(IdTable_t*   id_table, 
                                             size_t       id_index, 
                                             size_t       n_local_vars, 
                                             size_t       n_params)
{
    assert(id_table);

    if (id_index >= id_table->size)
    {
        WPRINTERR(L"index exceeds limits of id_table");
        return LANG_ID_TABLE_WRONG_INDEX;
    }

    id_table->data[id_index].n_local_vars = n_local_vars;
    id_table->data[id_index].n_params     = n_params;

    return LANG_SUCCESS;
} 

//==========================================================================================

LangErr_t LangIdTablePushFunctionIfUnique(LangCtx_t*   lang_ctx, 
                                          size_t       name_index,
                                          int*         table_index)
{
    assert(lang_ctx);
    
    IdData_t id_data = {};

    LangIdDataBuild(lang_ctx, &id_data, name_index, 0, 0, ID_TYPE_FUNCTION);

    if (LangIdTableIdentifierIsDeclared(&lang_ctx->func_id_table, name_index) ||
        LangIdTableIdentifierIsDeclared(&lang_ctx->main_id_table, name_index))
    {
        WPRINTERR(L"Identifier %ls was declared, can not declare a function\n",
                  id_data.name);
        return LANG_FUNC_REDECLARATION;
    }
    
    return LangIdTablePush(&lang_ctx->func_id_table, &id_data, table_index);
}

//==========================================================================================

static LangErr_t LangIdTableCountAddresses(IdTable_t* id_table)
{
    assert(id_table);

    size_t id_table_size         = id_table->size;
    size_t cur_func_params_count = 0;
    size_t cur_func_locals_count = 0;

    for (size_t i = 0; i < id_table_size; i++)
    {
        switch (id_table->data[i].type)
        {
            case ID_TYPE_FUNCTION:
                cur_func_params_count = 0;
                cur_func_locals_count = 0;
                break;

            case ID_TYPE_PARAMETER:
                id_table->data[i].addr = (int) ((cur_func_params_count + 
                                          STACK_FRAME_PROLOGUE_SIZE) * 
                                          STACK_ELEMENT_SIZE);
                cur_func_params_count++;
                break;

            case ID_TYPE_VARIABLE:
                id_table->data[i].addr = (-1) * (int) (cur_func_locals_count * STACK_ELEMENT_SIZE)
                                          - STACK_ELEMENT_SIZE; // additional minus, because rsp addresses up
                cur_func_locals_count++;
                break;

            case ID_TYPE_UNKNOWN:
            default:
                WPRINTERR(L"Poisoned type in id_table");
                return LANG_WRONG_ID_TABLE;
        }
    }

    return LANG_SUCCESS;
}

//==========================================================================================

void LangIdTableCountVarsAndParams(IdTable_t* id_table, 
                                   size_t*    n_local_vars_p, 
                                   size_t*    n_params_p)
{
    assert(id_table);
    assert(n_local_vars_p);
    assert(n_params_p);

    size_t size         = id_table->size;
    size_t n_local_vars = 0;
    size_t n_params     = 0;

    for (size_t i = 0; i < size; i++)
    {
        if (id_table->data[i].type == ID_TYPE_PARAMETER)
        {
            n_params++;
        }
        else if (id_table->data[i].type == ID_TYPE_VARIABLE)
        {
            n_local_vars++;
        }
    }

    *n_local_vars_p = n_local_vars;
    *n_params_p     = n_params;
}

//==========================================================================================

LangErr_t LangCountAddresses(LangCtx_t* lang_ctx)
{
    assert(lang_ctx);

    LangErr_t error = LANG_SUCCESS;

    if ((error = LangIdTableCountAddresses(&lang_ctx->func_id_table)))
    {
        return error;
    }

#ifdef TREE_DEBUG
    LangIdTableDump(lang_ctx, &lang_ctx->func_id_table, L"Dump after counting offsets");
#endif /* TREE_DEBUG */

    return LANG_SUCCESS;
}

//==========================================================================================

void LangIdTableGetFunctionVarsAndParams(IdTable_t*   id_table, 
                                         size_t       id_index, 
                                         size_t*      n_local_vars,
                                         size_t*      n_params)
{
    assert(id_table);
    assert(n_local_vars);
    assert(n_params);

    *n_local_vars = id_table->data[id_index].n_local_vars;
    *n_params     = id_table->data[id_index].n_params;
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
