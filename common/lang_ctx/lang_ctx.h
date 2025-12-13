#ifndef LANG_STRUCT_H
#define LANG_STRUCT_H

//——————————————————————————————————————————————————————————————————————————————————————————

#include "tree_types.h"
#include "stack.h"
#include <stdio.h>
#include <wchar.h>

//——————————————————————————————————————————————————————————————————————————————————————————

//FIXME - структура id
typedef wchar_t* Identifier_t;

typedef struct IdTable
{
    Identifier_t* data;

    size_t        size;
    size_t        capacity;

} IdTable_t;

//——————————————————————————————————————————————————————————————————————————————————————————

typedef struct LangCtx
{
    wchar_t*      code;
    wchar_t*      buffer;
    size_t        current_line;

    Stack_t       tokens;
    Tree_t        tree;

    IdTable_t     id_table;

    TreeDebugData debug;

} LangCtx_t;

//——————————————————————————————————————————————————————————————————————————————————————————

typedef enum LangErr
{
    LANG_SUCCESS = 0,

    LANG_SYNTAX_ERROR,
    LANG_MEMALLOC_ERROR,
    LANG_FILE_ERROR,
    LANG_STACK_ERROR,
    LANG_TREE_ERROR,
    LANG_INVALID_INPUT

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

#endif /* LANG_STRUCT_H */
