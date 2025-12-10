#ifndef LANG_STRUCT_H
#define LANG_STRUCT_H

//——————————————————————————————————————————————————————————————————————————————————————————

#include "tree_types.h"
#include "stack.h"

//——————————————————————————————————————————————————————————————————————————————————————————

typedef struct IdTable
{
    char** data;

    size_t size;
    size_t capacity;

} IdTable_t;

//——————————————————————————————————————————————————————————————————————————————————————————

typedef struct LangCtx
{
    char*       code;
    Stack_t     tokens;

    Tree_t      tree;

    IdTable_t   id_table;

} LangCtx_t;

//——————————————————————————————————————————————————————————————————————————————————————————

typedef enum LangErr
{
    LANG_SUCCESS = 0,

    LANG_SYNTAX_ERROR,
    LANG_MEMALLOC_ERROR,
    LANG_FILE_ERROR,
    LANG_STACK_ERROR,
    LANG_TREE_ERROR

} LangErr_t;

//——————————————————————————————————————————————————————————————————————————————————————————

// const char* const LANG_STR_ERRORS[] =
// {
//     [LANG_SUCCESS]          = "Lang ctx is ok",
//     [LANG_SYNTAX_ERROR]     = "",
//     [LANG_MEMALLOC_ERROR]   = "",
//     [LANG_FILE_ERROR]       = ""
// };

//——————————————————————————————————————————————————————————————————————————————————————————

const size_t DEFAULT_ID_TABLE_CAPACITY = 64;

//——————————————————————————————————————————————————————————————————————————————————————————

LangErr_t LangCtxCtor       (LangCtx_t* lang_ctx);
void      LangCtxDtor       (LangCtx_t* lang_ctx);

LangErr_t LangIdTableCtor   (IdTable_t* id_table);
void      LangIdTableDtor   (IdTable_t* id_table);
LangErr_t LangIdTablePush   (LangCtx_t* lang_ctx, const char* id_name_buf, size_t* id_index);

//——————————————————————————————————————————————————————————————————————————————————————————

#endif /* LANG_STRUCT_H */
