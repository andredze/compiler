#ifndef LANG_STRUCT_H
#define LANG_STRUCT_H

//——————————————————————————————————————————————————————————————————————————————————————————

#include "id_types.h"
#include "tree_types.h"
#include "stack.h"
#include <stdio.h>
#include <wchar.h>

//——————————————————————————————————————————————————————————————————————————————————————————

const char * const AST_DIR_NAME = "ast";

//——————————————————————————————————————————————————————————————————————————————————————————

typedef enum LangErr
{
    LANG_SUCCESS = 0,

    LANG_SYNTAX_ERROR,
    LANG_SYSTEM_ERROR,
    LANG_MEMALLOC_ERROR,
    LANG_FILE_ERROR,
    LANG_STACK_ERROR,
    LANG_TREE_ERROR,

    LANG_INVALID_INPUT,
    LANG_INVALID_AST_INPUT,
    LANG_BACKEND_AST_SYNTAX_ERROR,
    LANG_UNKNOWN_TOKEN_TYPE,
    LANG_UNASSEMBLE_OPERATOR,
    LANG_REVERSEBLE_OPERATOR,

    LANG_PARSER_SYNTAX_ERROR,
    LANG_LEXER_SYNTAX_ERROR,

    LAND_ID_TABLE_WRONG_INDEX,

    LANG_VAR_REDECLARATION,
    LANG_VAR_NOT_DECLARED,
    LANG_FUNC_DECL_IN_FUNC,
    LANG_FUNC_REDECLARATION,
    LANG_FUNC_NOT_DECLARED,
    LANG_FUNC_USED_AS_VAR,
    LANG_WRONG_ARGS_COUNT,
    LANG_WRONG_AST_FORMAT

} LangErr_t;

//——————————————————————————————————————————————————————————————————————————————————————————

const size_t MAX_MESSAGE_LEN = 128;

//------------------------------------------------------------------------------------------

typedef struct
{
    enum LangErr   error;

    TreeNode_t*    node;

    const char*    func;
    const char*    file;
    int            line;

    wchar_t        message[MAX_MESSAGE_LEN];

} LangErrorInfo_t;

//——————————————————————————————————————————————————————————————————————————————————————————

typedef struct LangCtx
{
    char        src_code_file_name[MAX_FILENAME_LEN];

    Tree_t      tree;

    NamesPool_t names_pool;

    IdTable_t   main_id_table;
    IdTable_t   func_id_table;

    size_t      global_vars_count;
}
LangCtx_t;

//——————————————————————————————————————————————————————————————————————————————————————————

#endif /* LANG_STRUCT_H */
