#include "lang_funcs.h"

//------------------------------------------------------------------------------------------

//FIXME - stack realloc дропается дамп, скорее всего реаллок не работает

LangErr_t LangCtxCtor(LangCtx_t* lang_ctx)
{
    assert(lang_ctx != NULL);

    if (StackCtor(&lang_ctx->tokens, 256))
    {
        WPRINTERR(L"Tokens stack construct failed");
        return LANG_STACK_ERROR;
    }

    lang_ctx->cur_symbol_ptr         = NULL;
    lang_ctx->current_line = 1;

    if (TreeCtor(&lang_ctx->tree))
    {
        WPRINTERR(L"Language tree construct failed");
        return LANG_TREE_ERROR;
    }

    if (TreeOpenLogFile(lang_ctx))
        return LANG_TREE_ERROR;

    LangErr_t error = LANG_SUCCESS;

    if ((error = LangIdTableCtor(&lang_ctx->id_table)))
        return error;

    return LANG_SUCCESS;
}

//------------------------------------------------------------------------------------------

void LangCtxDtor(LangCtx_t* lang_ctx)
{
    assert(lang_ctx);

    TreeSingleNodeDtor(lang_ctx->tree.dummy, &lang_ctx->tree);

    LangIdTableDtor(&lang_ctx->id_table);

    for (size_t i = 0; i < lang_ctx->tokens.size; i++)
    {
        WDPRINTF(L"freed token = %p\n", lang_ctx->tokens.data[i]);
        free(lang_ctx->tokens.data[i]);
    }

    StackDtor(&lang_ctx->tokens);

    free(lang_ctx->buffer);

    lang_ctx->cur_symbol_ptr   = NULL;
    lang_ctx->buffer = NULL;

    TreeCloseLogFile(lang_ctx);
}

//==========================================================================================

LangErr_t LangIdTableCtor(IdTable_t* id_table)
{
    assert(id_table);

    size_t capacity = DEFAULT_ID_TABLE_CAPACITY;

    id_table->data = (Identifier_t*) calloc(capacity, sizeof(Identifier_t));

    if (id_table->data == NULL)
    {
        WPRINTERR(L"Memory allocation failed");
        return LANG_MEMALLOC_ERROR;
    }

    id_table->capacity = capacity;
    id_table->size     = 0;

    return LANG_SUCCESS;
}

//------------------------------------------------------------------------------------------

void LangIdTableDtor(IdTable_t* id_table)
{
    assert(id_table);

    for (size_t i = 0; i < id_table->size; i++)
    {
        free(id_table->data[i]);
        id_table->data[i] = NULL;
    }

    id_table->size     = 0;
    id_table->capacity = 0;

    free(id_table->data);
    id_table->data = NULL;

    WDPRINTF(L"> IdTable destroyed\n");
}

//——————————————————————————————————————————————————————————————————————————————————————————

static LangErr_t LangIdTableRealloc(IdTable_t* id_table);

//——————————————————————————————————————————————————————————————————————————————————————————

LangErr_t LangIdTablePush(LangCtx_t* lang_ctx, const wchar_t* id_name_buf, size_t* id_index)
{
    assert(id_name_buf != NULL);
    assert(lang_ctx    != NULL);
    assert(id_index    != NULL);

    IdTable_t* id_table = &lang_ctx->id_table;

    //FIXME - без strdup - а
    Identifier_t id_name = wcsdup(id_name_buf);

    if (id_name == NULL)
    {
        PRINTERR("Memory allocation failed");
        return LANG_MEMALLOC_ERROR;
    }

    LangErr_t error = LANG_SUCCESS;

    if (id_table->size >= id_table->capacity)
    {
        if ((error = LangIdTableRealloc(id_table)))
            return error;
    }

    *id_index = id_table->size;
    id_table->data[id_table->size++] = id_name;

    return LANG_SUCCESS;
}

//------------------------------------------------------------------------------------------

static LangErr_t LangIdTableRealloc(IdTable_t* id_table)
{
    assert(id_table);

    size_t new_capacity = id_table->capacity * 2 + 1;

    Identifier_t* new_data = (Identifier_t*) realloc(id_table->data,
                                                     new_capacity * sizeof(*id_table->data));

    if (new_data == NULL)
    {
        WPRINTERR(L"Memory reallocation failed");
        return LANG_MEMALLOC_ERROR;
    }

    id_table->data     = new_data;
    id_table->capacity = new_capacity;

    return LANG_SUCCESS;
}

//------------------------------------------------------------------------------------------

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

//------------------------------------------------------------------------------------------
