#ifndef TREE_TYPES_H
#define TREE_TYPES_H

//——————————————————————————————————————————————————————————————————————————————————————————

#include "common.h"
#include "operators.h"

//——————————————————————————————————————————————————————————————————————————————————————————

#ifdef BACKEND

typedef struct Identifier
{
    size_t name_index;
    size_t id_index;

    bool   is_global;

} Identifier_t;

#endif /* BACKEND */

#ifdef FRONTEND

typedef size_t Identifier_t;

#endif /* FRONTEND */

//——————————————————————————————————————————————————————————————————————————————————————————

typedef union TokenValue
{
    Operator_t   opcode;
    double       number;
    Identifier_t id;

} TokenValue_t;

//——————————————————————————————————————————————————————————————————————————————————————————

typedef struct TokenData
{
    TokenType_t  type;
    TokenValue_t value;

} TokenData_t;

//——————————————————————————————————————————————————————————————————————————————————————————

typedef TokenData_t TreeElem_t;

const TreeElem_t TREE_POISON = {TYPE_NUM, { .number = 0 }};

//——————————————————————————————————————————————————————————————————————————————————————————

typedef struct TreeNode
{
    TreeElem_t data;

    TreeNode*  left;
    TreeNode*  right;

    TreeNode*  parent;

} TreeNode_t;

//——————————————————————————————————————————————————————————————————————————————————————————

const int MAX_FILE_PATH_LEN = 256;
const int MAX_DIR_PATH_LEN  = 100;
const int MAX_FILE_NAME_LEN = 120;
const int MAX_STR_TIME_LEN  = 64;

//——————————————————————————————————————————————————————————————————————————————————————————

typedef struct TreeDebugData
{
    FILE* fp;

    wchar_t log_file_path   [MAX_FILE_PATH_LEN];
    wchar_t graph_file_name [MAX_FILE_NAME_LEN];
    wchar_t img_file_path   [MAX_FILE_PATH_LEN];
    wchar_t dot_file_path   [MAX_FILE_PATH_LEN];

    int graphs_count;

    wchar_t str_time [MAX_STR_TIME_LEN];
    wchar_t log_dir  [MAX_DIR_PATH_LEN];
    wchar_t img_dir  [MAX_DIR_PATH_LEN];
    wchar_t dot_dir  [MAX_DIR_PATH_LEN];

} TreeDebugData_t;

//——————————————————————————————————————————————————————————————————————————————————————————

typedef struct Tree
{
    TreeNode_t* dummy; // null (fake) element
    size_t      size;

    char* buffer;

} Tree_t;

//——————————————————————————————————————————————————————————————————————————————————————————

typedef enum TreeErr
{
    TREE_SUCCESS           = 0,
    TREE_NULL              = 1,
    TREE_SIZE_EXCEEDS_MAX  = 2,
    TREE_LOOP              = 3,

    TREE_CALLOC_ERROR      = 4,
    TREE_DUMP_ERROR        = 5,
    TREE_SYSTEM_FUNC_ERROR = 6,
    TREE_FILE_ERROR        = 7,
    TREE_STACK_ERROR       = 8,

    TREE_INVALID_INPUT     = 9,
    TREE_DTOR_ERROR        = 10,
    TREE_LOST_CONNECTION   = 11

} TreeErr_t;

//——————————————————————————————————————————————————————————————————————————————————————————

const char* const TREE_STR_ERRORS[] =
{
    [TREE_SUCCESS]            = "Tree is ok",
    [TREE_NULL]               = "Nullptr given",
    [TREE_SIZE_EXCEEDS_MAX]   = "Tree size exceeded maximum possible value",
    [TREE_LOOP]               = "Tree is looped",
    [TREE_CALLOC_ERROR]       = "Memory allocation for tree failed",
    [TREE_DUMP_ERROR]         = "Error with dumping tree",
    [TREE_SYSTEM_FUNC_ERROR]  = "System function failed",
    [TREE_FILE_ERROR]         = "Error with opening/writing to file",
    [TREE_STACK_ERROR]        = "Error with stack commands",
    [TREE_INVALID_INPUT]      = "User input is invalid",
    [TREE_DTOR_ERROR]         = "Error with destroying tree",
    [TREE_LOST_CONNECTION]    = "Tree node->parent doesn't match actual parent"
};

//——————————————————————————————————————————————————————————————————————————————————————————

typedef struct TreeDumpInfo
{
    TreeErr_t   error;
    const char* func;
    const char* file;
    int         line;
} TreeDumpInfo_t;

//——————————————————————————————————————————————————————————————————————————————————————————

typedef struct TypeCase
{
    TokenType_t    type;
    const char*    name;

    const wchar_t* shape;

    const wchar_t* color;
    const wchar_t* fillcolor;
    const wchar_t* fontcolor;

    const wchar_t* ast_format;

} TypeCase_t;

//——————————————————————————————————————————————————————————————————————————————————————————

#define SET_TYPE_CASE_(code,   name,   shape,   color,   fillcolor,   fontcolor,   ast_format) \
        [(code)]   = {(code), (name), (shape), (color), (fillcolor), (fontcolor), (ast_format)}

//------------------------------------------------------------------------------------------

const TypeCase_t TYPE_CASES_TABLE[] =
{
//                      code        dump_name   dump_shape  boundaries_color fill_color      font_color    ast_format
    SET_TYPE_CASE_(TYPE_OP       , "OP"       , L"Mrecord", L"#000064"  , L"#C0C0FF"  , L"#000064"  , L"OPER"     ),
    SET_TYPE_CASE_(TYPE_ID       , "ID"       , L"Mrecord", L"#006400"  , L"#C0FFC0"  , L"#006400"  , L""         ),
    SET_TYPE_CASE_(TYPE_NUM      , "NUM"      , L"Mrecord", L"#990000"  , L"#FFC0C0"  , L"#990000"  , L"NUM"      ),
    SET_TYPE_CASE_(TYPE_VAR      , "VAR"      , L"Mrecord", L"#153e61ff", L"#aadce2ff", L"#153e61ff", L"VAR"      ),
    SET_TYPE_CASE_(TYPE_VAR_DECL , "VAR_DECL" , L"Mrecord", L"#43236eff", L"#af92d6ff", L"#43236eff", L"VAR_INIT" ),
    SET_TYPE_CASE_(TYPE_FUNC_CALL, "FUNC_CALL", L"Mrecord", L"#2e4517ff", L"#bcd2a5ff", L"#2e4517ff", L"FUNC"     ),
    SET_TYPE_CASE_(TYPE_FUNC_DECL, "FUNC_DECL", L"Mrecord", L"#451126ff", L"#df7a93ff", L"#451126ff", L"FUNC_INIT")
};

//------------------------------------------------------------------------------------------

#undef SET_TYPE_CASE_

const size_t TYPES_COUNT = sizeof(TYPE_CASES_TABLE) / sizeof(TYPE_CASES_TABLE[0]);

//——————————————————————————————————————————————————————————————————————————————————————————

const size_t TREE_MAX_SIZE = 1024 * 1024 * 1024;

//——————————————————————————————————————————————————————————————————————————————————————————

#endif /* TREE_TYPES_H */
