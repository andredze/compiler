#include "relocations_table.h"
#include "lang_ctx.h"

//==========================================================================================

BackendErr_t RelTableCtor(RelTable_t* rel_table)
{
    assert(rel_table);

    rel_table->size     = 0;
    rel_table->capacity = REL_TABLE_INIT_CAPACITY;

    rel_table->data = (RelElem_t*) calloc(rel_table->capacity, sizeof(RelElem_t));

    if (rel_table->data == NULL)
    {
        WPRINTERR(L"Memalloc error for rel_table->data");
        return BACKEND_MEMALLOC_ERROR;
    }

    return BACKEND_SUCCESS;
}

//==========================================================================================

static BackendErr_t RelTableRealloc(RelTable_t* rel_table)
{
    assert(rel_table);

    size_t new_cap = rel_table->capacity * 2 + 1;

    RelElem_t* new_data = (RelElem_t*) realloc(rel_table->data, sizeof(RelElem_t) * new_cap);

    if (new_data == NULL)
    {
        WPRINTERR(L"Mem realloc error for rel_table->data");
        return BACKEND_MEMALLOC_ERROR;
    }

    rel_table->capacity = new_cap;
    rel_table->data     = new_data;

    return BACKEND_SUCCESS;
}

//==========================================================================================

void RelTableDtor(RelTable_t* rel_table)
{
    assert(rel_table);

    rel_table->size     = 0;
    rel_table->capacity = 0;

    free(rel_table->data);
    rel_table->data = NULL;

    WDPRINTF(L"------- RelTable destroyed -------");
}

//==========================================================================================

BackendErr_t RelTablePushExternCall (RelTable_t*    rel_table, 
                                     const wchar_t* label, 
                                     size_t         bin_code_pos)
{
    assert(rel_table);
    assert(label);

    RelElem_t elem = {.id_table_index = (int) ID_TABLE_INDEX_EXTERN_FUNC_POISON,
                      .label          = label, 
                      .bin_code_pos   = bin_code_pos,
                      .scope          = REL_FUNC_EXTERN,
                      .type           = REL_FUNC_CALL};

    return RelTablePush(rel_table, &elem);
}

//==========================================================================================

BackendErr_t RelTablePushInnerFuncDecl (RelTable_t*    rel_table,
                                        size_t         id_table_index,
                                        const wchar_t* label,
                                        size_t         bin_code_pos,
                                        RelScopeType_t scope)
{
    assert(rel_table);

    RelElem_t elem = {.id_table_index = (int) id_table_index,
                      .label          = label, 
                      .bin_code_pos   = bin_code_pos,
                      .scope          = scope,
                      .type           = REL_FUNC_DECL};

    return RelTablePush(rel_table, &elem);
}

//==========================================================================================

BackendErr_t RelTablePushInnerFuncCall (RelTable_t*    rel_table,
                                        size_t         id_table_index,
                                        const wchar_t* label,
                                        size_t         bin_code_pos)
{
    assert(rel_table);

    RelElem_t elem = {.id_table_index = (int) id_table_index,
                      .label          = label, 
                      .bin_code_pos   = bin_code_pos,
                      .scope          = REL_FUNC_LOCAL,
                      .type           = REL_FUNC_CALL};

    return RelTablePush(rel_table, &elem);
}

//==========================================================================================

BackendErr_t RelTablePush (RelTable_t* rel_table, RelElem_t* elem)
{
    assert(rel_table);
    assert(elem);

    BackendErr_t error = BACKEND_SUCCESS;

    if (rel_table->size + 1 >= rel_table->capacity)
    {
        if ((error = RelTableRealloc(rel_table)))
        {
            return error;
        }
    }

    rel_table->data[rel_table->size++] = *elem;

    return BACKEND_SUCCESS;
}

//==========================================================================================

BackendErr_t RelTablePopLabelBinCodePos(RelTable_t* rel_table, 
                                        size_t*     bin_code_pos_dst)
{
    assert(rel_table);
    assert(bin_code_pos_dst);

    rel_table->size--;
    *bin_code_pos_dst = rel_table->data[rel_table->size].bin_code_pos;

    return BACKEND_SUCCESS;
}

//==========================================================================================

BackendErr_t RelTableGetLabelBinCodePos(RelTable_t* rel_table, 
                                        size_t      rel_table_index, 
                                        size_t*     bin_code_pos_dst)
{
    assert(rel_table);
    assert(bin_code_pos_dst);

    if (rel_table_index >= rel_table->size)
    {
        WPRINTERR(L"Failed addressing rel_table: Index %zu exceeds size %zu", 
                  rel_table_index, rel_table->size);
        return BACKEND_WRONG_REL_TABLE_INDEX;
    }

    *bin_code_pos_dst = rel_table->data[rel_table_index].bin_code_pos;

    return BACKEND_SUCCESS;
}

//==========================================================================================

BackendErr_t RelTableGetLabelBinCodePosByIdIndex(RelTable_t* rel_table, 
                                                 size_t      id_table_index, 
                                                 size_t*     bin_code_pos_dst)
{
    assert(rel_table);
    assert(bin_code_pos_dst);

    for (size_t i = 0; i < rel_table->size; i++)
    {
        if (rel_table->data[i].id_table_index == (int) id_table_index)
        {
            *bin_code_pos_dst = rel_table->data[i].bin_code_pos;
            return BACKEND_SUCCESS;
        }
    }

    WPRINTERR(L"Failed finding rel_table elem by id_index %zu", id_table_index);
    return BACKEND_REL_TABLE_LABEL_NOT_FOUND;
}

//==========================================================================================

int CountLabelRelAddr(size_t label_pos, size_t pos_before_instr_using_label)
{
    return (int) (label_pos) - (int) (pos_before_instr_using_label);
}

//==========================================================================================

LangErr_t RelTableDump(LangCtx_t*     lang_ctx, 
                       RelTable_t*    rel_table, 
                       const char*    func,
                       const char*    file,
                       int            line,
                       const wchar_t* fmt,
                       ...)
{
    assert(lang_ctx);
    assert(rel_table);

    FILE* fp = lang_ctx->tree.debug.fp;

    va_list args = {};

    va_start(args, fmt);

    fwprintf(fp, L"<h4><font color=blue>"
                 L"Dump RelTable_t %p called from %s at %s:%d"
                 L" Message: ",
                 rel_table,
                 func, 
                 file,
                 line);

    vfwprintf(fp, fmt, args);

    va_end(args);

    fwprintf(fp, L"</h4></font>\n\n");

    fwprintf(fp,
             L".size     = %zu\n"
             L".cap      = %zu\n"
             L".data     = %p\n",
             rel_table->size,
             rel_table->capacity,
             rel_table->data);

    RelElem_t* rel_data = NULL;

    fwprintf(fp, L"index  {        label,        bin_code_pos,   hex,       scope,           type      }\n");

    for (size_t i = 0; i < rel_table->size; i++)
    {
        rel_data = &rel_table->data[i];

        fwprintf(fp, L"[ %-2d]: {%20ls,      %3zu    , %#6x, %15s, %14s }\n",
                     i,
                     rel_data->label,
                     rel_data->bin_code_pos,
                     rel_data->bin_code_pos,
                     REL_FUNC_SCOPE_NAME[rel_data->scope],
                     REL_FUNC_TYPE_NAME[rel_data->type]);
    }

    fwprintf(fp, L"---------------------------------------"
                 L"---------------------------------------\n\n");
    
    fflush(fp);

    return LANG_SUCCESS;
}

//==========================================================================================
