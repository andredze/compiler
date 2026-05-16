#ifndef RELOCATIONS_TABLE_H
#define RELOCATIONS_TABLE_H

//——————————————————————————————————————————————————————————————————————————————————————————

#include <stdlib.h>
#include "backend_err.h"
#include "common.h"

//——————————————————————————————————————————————————————————————————————————————————————————

typedef struct RelElem
{
    int      id_table_index;
    wchar_t* label;
    size_t   bin_code_pos;
}
RelElem_t;

//——————————————————————————————————————————————————————————————————————————————————————————

typedef struct RelTable
{
    size_t     capacity;
    size_t     size;

    RelElem_t* data;
}
RelTable_t;

//——————————————————————————————————————————————————————————————————————————————————————————

const size_t REL_TABLE_INIT_CAPACITY = 128;

//——————————————————————————————————————————————————————————————————————————————————————————

BackendErr_t RelTableCtor (RelTable_t* rel_table);
void         RelTableDtor (RelTable_t* rel_table);

BackendErr_t RelTablePush (RelTable_t* rel_table, 
                           wchar_t*    label, 
                           size_t      bin_code_pos,
                           int         id_table_index,
                           size_t*     rel_table_index_dst);

BackendErr_t RelTableGetLabelBinCodePos(RelTable_t* rel_table, 
                                        size_t      rel_table_index, 
                                        size_t*     bin_code_pos_dst);

BackendErr_t RelTablePopLabelBinCodePos(RelTable_t* rel_table, 
                                        size_t*     bin_code_pos_dst);

BackendErr_t RelTableGetLabelBinCodePosByIdIndex(RelTable_t* rel_table, 
                                                 size_t      id_table_index, 
                                                 size_t*     bin_code_pos_dst);

int CountLabelRelAddr(size_t label_pos, size_t pos_before_instr_using_label);

//——————————————————————————————————————————————————————————————————————————————————————————

#endif /* RELOCATIONS_TABLE_H */