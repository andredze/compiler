#include "lang_funcs.h"

//------------------------------------------------------------------------------------------

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

    if ((error = LangIdTableCtor(&lang_ctx->id_table)))
        return error;

    if (TreeCtor(&lang_ctx->tree))
    {
        WPRINTERR(L"Language tree construct failed");
        return LANG_TREE_ERROR;
    }

    if (TreeOpenLogFile(lang_ctx))
        return LANG_TREE_ERROR;

    return LANG_SUCCESS;
}

//------------------------------------------------------------------------------------------

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

    if (lang_ctx->output_file != NULL)
        fclose(lang_ctx->output_file);

#endif /* BACKEND */

    LangIdTableDtor(&lang_ctx->id_table);

    free(lang_ctx->buffer);

    lang_ctx->cur_symbol_ptr = NULL;
    lang_ctx->buffer         = NULL;

    TreeCloseLogFile(lang_ctx);
}

//------------------------------------------------------------------------------------------

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

    for (size_t i = 0; i < lang_ctx->id_table.size; i++)
    {
        if (wcscmp(lang_ctx->id_table.data[i], id_name_buf) == 0)
        {
            *id_index = i;
            return LANG_SUCCESS;
        }
    }

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
