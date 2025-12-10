#include "lang_ctx.h"

//------------------------------------------------------------------------------------------

LangErr_t LangCtxCtor(LangCtx_t* lang_ctx)
{
    assert(lang_ctx != NULL);

    if (StackCtor(&lang_ctx->tokens))
    {
        PRINTERR("Tokens stack construct failed");
        return LANG_STACK_ERROR;
    }

    lang_ctx->code = NULL;

    if (TreeCtor(lang_ctx->tree))
    {
        PRINTERR("Language tree construct failed");
        return LANG_TREE_ERROR;
    }

    LangErr_t error = LANG_SUCCESS;

    if ((error = LangIdTableCtor(&lang_ctx->id_table)))
        return error;

    return LANG_SUCCESS;
}

//------------------------------------------------------------------------------------------

void LangCtxDtor(LangCtx_t* lang_ctx)
{
    assert(lang_ctx);

    StackDtor      (&lang_ctx->tokens  );
    TreeDtor       (&lang_ctx->tree    );
    LangIdTableDtor(&lang_ctx->id_table);

    free(lang_ctx->code);

    lang_ctx->code = NULL;
}

//==========================================================================================

LangErr_t LangIdTableCtor(IdTable_t* id_table)
{
    assert(id_table);

    size_t capacity = DEFAULT_ID_TABLE_CAPACITY;

    id_table->data = (char**) calloc(capacity, sizeof(char*));

    if (id_table->data == NULL)
    {
        PRINTERR("Memory allocation failed");
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

    DPRINTF("> IdTable destroyed");
}

//——————————————————————————————————————————————————————————————————————————————————————————

static LangErr_t LangIdTableRealloc(IdTable_t* id_table);

//——————————————————————————————————————————————————————————————————————————————————————————

LangErr_t LangIdTablePush(LangCtx_t* lang_ctx, const char* id_name_buf, size_t* id_index)
{
    assert(id_name_buf != NULL);
    assert(lang_ctx    != NULL);
    assert(id_index    != NULL);

    IdTable_t* id_table = lang_ctx->id_table;

    char* id_name = strdup(id_name_buf);

    if (id_name == NULL)
    {
        PRINTERR("Memory allocation failed");
        return LANG_MEMALLOC_ERROR;
    }

    if (id_table->size >= id_table->capacity)
    {
        if ((error = LangIdTableRealloc(id_table)))
            return error;
    }

    id_table->data[id_table->size++] = id_name;

    return LANG_SUCCESS;
}

//------------------------------------------------------------------------------------------

static LangErr_t LangIdTableRealloc(IdTable_t* id_table)
{
    assert(id_table);

    size_t new_capacity = id_table->capacity * 2 + 1;

    char* new_data = realloc(id_table->data, new_capacity * sizeof(*id_table->data));

    if (new_data == NULL)
    {
        PRINTERR("Memory reallocation failed");
        return LANG_MEMALLOC_ERROR;
    }

    id_table->data     = new_data;
    id_table->capacity = new_capacity;

    return LANG_SUCCESS;
}

//------------------------------------------------------------------------------------------
