#include "relocations_table.h"

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

BackendErr_t RelTablePush (RelTable_t* rel_table, 
                           wchar_t*    label, 
                           size_t      bin_code_pos,
                           int         id_table_index,
                           size_t*     rel_table_index_dst)
{
    assert(rel_table);

    BackendErr_t error = BACKEND_SUCCESS;

    if (rel_table->size + 1 >= rel_table->capacity)
    {
        if ((error = RelTableRealloc(rel_table)))
        {
            return error;
        }
    }

    if (rel_table_index_dst)
    {
        *rel_table_index_dst = rel_table->size;
    }

    rel_table->data[rel_table->size++] = {id_table_index, label, bin_code_pos};

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

int CountLabelRelAddr(size_t label_pos, size_t cur_pos)
{
    return (int) (label_pos) - (int) (cur_pos);
}

//==========================================================================================
