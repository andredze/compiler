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
    char          ast_file_name[MAX_FILENAME_LEN];

//TODO: код ошибки вся инфа об ошибке передавать до main
    wchar_t*      cur_symbol_ptr; // cur
    wchar_t*      buffer;
    size_t        current_line;

    Stack_t       tokens;
    Tree_t        tree;

    IdTable_t     id_table;

    TreeDebugData debug;

    size_t        cur_token_index; // for parser rename

#ifdef BACKEND
    FILE*         output_file;
    size_t        endif_labels_count;
#endif /* BACKEND */

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
    LANG_INVALID_INPUT,
    LANG_INVALID_AST_INPUT,
    LANG_BACKEND_AST_SYNTAX_ERROR,
    LANG_UNKNOWN_TOKEN_TYPE

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
