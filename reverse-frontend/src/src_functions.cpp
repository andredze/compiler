#include "operators.h"
#include "lang_ctx.h"
#include "op_cases.h"

//==========================================================================================

#define _DSL_DEFINE_
#include "dsl.h"

//==========================================================================================

static LangErr_t SrcNumber              (LangCtx_t* lang_ctx, TreeNode_t* node);
static LangErr_t SrcRepetitionPrint     (LangCtx_t* lang_ctx, Operator_t opcode);
static LangErr_t SrcEnterRepetitionPrint(LangCtx_t* lang_ctx, Operator_t opcode);
static LangErr_t SrcVariable            (LangCtx_t* lang_ctx, TreeNode_t* node);
static LangErr_t SrcVariableDeclaration (LangCtx_t* lang_ctx, TreeNode_t* node);
static LangErr_t SrcFunctionDeclaration (LangCtx_t* lang_ctx, TreeNode_t* node);
static LangErr_t SrcFunctionCall        (LangCtx_t* lang_ctx, TreeNode_t* node);

//==========================================================================================

// typedef struct FrontendCtx
// {
//     LangErrorInfo_t error_info;

//     wchar_t*      cur_symbol_ptr;
//     wchar_t*      buffer;
//     size_t        buffer_size;
//     size_t        current_line;

//     Stack_t       tokens;
//     size_t        cur_token_index; // for parser rename

//     FILE*         output_file;

//     bool          is_in_func;
//     int           in_func_vars_count;

//     LangCtx_t     lang_ctx;

// } FrontendCtx_t;

// #ifdef REVERSE
//     size_t        tabs;
// #endif /* REVERSE */

// LangErr_t LangOpenReverseFile(LangCtx_t* lang_ctx)
// {
//     assert(lang_ctx);

//     char src_file_path[MAX_FILE_NAME_LEN];

//     snprintf(src_file_path, sizeof(src_file_path), "reversed/%s.psy", lang_ctx->ast_file_name);

//     WDPRINTF(L"Reversed code file name: %s\n", lang_ctx->ast_file_name);
//     WDPRINTF(L"Opening file %s\n\n", src_file_path);

//     FILE* src_fp = fopen(src_file_path, "w");

//     if (src_fp == NULL)
//     {
//         WPRINTERR(L"Failed opening file %s", src_file_path);
//         return LANG_FILE_ERROR;
//     }

//     lang_ctx->output_file = src_fp;

//     return LANG_SUCCESS;
// }

//——————————————————————————————————————————————————————————————————————————————————————————

LangErr_t SrcNode(LangCtx_t* lang_ctx, TreeNode_t* node)
{
    assert(lang_ctx);
    assert(node);

    switch (node->data.type)
    {
        case TYPE_NUM:
            return SrcNumber(lang_ctx, node);

        case TYPE_ID:
            return LANG_INVALID_AST_INPUT;

        case TYPE_OP:
            if (OP_CASES_TABLE[node->data.value.opcode].src_function == NULL)
            {
                WPRINTERR("Error: operator %ls doesn't support reversing",
                          OP_CASES_TABLE[node->data.value.opcode].name);

                return LANG_REVERSEBLE_OPERATOR;
            }
            return OP_CASES_TABLE[node->data.value.opcode].src_function (lang_ctx, node);

        case TYPE_VAR:
            return SrcVariable(lang_ctx, node);

        case TYPE_VAR_DECL:
            return SrcVariableDeclaration(lang_ctx, node);

        case TYPE_FUNC_DECL:
            return SrcFunctionDeclaration(lang_ctx, node);

        case TYPE_FUNC_CALL:
            return SrcFunctionCall(lang_ctx, node);

        default:
            return LANG_UNKNOWN_TOKEN_TYPE;
    }

    return LANG_SUCCESS;
}

//==========================================================================================

LangErr_t SrcAssignment(LangCtx_t* lang_ctx, TreeNode_t* node)
{
    assert(lang_ctx);
    assert(node);

    SRC_PRINT_OP_(OP_ASSIGNMENT);

    SRC_PRINT_(L" ");

    if (node->left)
        SrcNode(lang_ctx, node->left);

    SRC_PRINT_(L" ");

    if (node->right)
        SrcNode(lang_ctx, node->right);

    SRC_PRINT_(L" ");

    SRC_PRINT_OP_(OP_CMD_SEPARATOR);
    SRC_PRINT_(L"\n");

    return LANG_SUCCESS;
}

//==========================================================================================

LangErr_t SrcCompareOperator(LangCtx_t* lang_ctx, TreeNode_t* node)
{
    assert(lang_ctx);
    assert(node);

    if (node->left)
        SrcNode(lang_ctx, node->left);

    SRC_PRINT_(L" ");

    SRC_PRINT_OP_(node->data.value.opcode);

    SRC_PRINT_(L" ");

    if (node->right)
        SrcNode(lang_ctx, node->right);

    SRC_PRINT_(L"\n");

    return LANG_SUCCESS;
}

//==========================================================================================

LangErr_t SrcUnaryOperator(LangCtx_t* lang_ctx, TreeNode_t* node)
{
    assert(lang_ctx);
    assert(node);

    SRC_PRINT_OP_(node->data.value.opcode);

    SRC_PRINT_(L" ");

    if (node->right)
        SrcNode(lang_ctx, node->right);

    SRC_PRINT_(L" ");

    SRC_PRINT_OP_(OP_CMD_SEPARATOR);
    SRC_PRINT_(L"\n");

    return LANG_SUCCESS;
}

//==========================================================================================

LangErr_t SrcMathOperation(LangCtx_t* lang_ctx, TreeNode_t* node)
{
    assert(lang_ctx);
    assert(node);

    if (node->left)
        SrcNode(lang_ctx, node->left);

    SRC_PRINT_(L" ");

    SRC_PRINT_OP_(node->data.value.opcode);
    SRC_PRINT_(L" ");

    if (node->right)
        SrcNode(lang_ctx, node->right);

    SRC_PRINT_(L" ");

    return LANG_SUCCESS;
}

//==========================================================================================

LangErr_t SrcAbort(LangCtx_t* lang_ctx, TreeNode_t* node)
{
    assert(lang_ctx);
    assert(node);

    SRC_PRINT_OP_(OP_ABORT);
    SRC_PRINT_(L"\n");

    return LANG_SUCCESS;
}

//==========================================================================================

LangErr_t SrcParamsSeparator(LangCtx_t* lang_ctx, TreeNode_t* node)
{
    assert(lang_ctx);
    assert(node);

    if (node->left)
        SrcNode(lang_ctx, node->left);

    if (node->right)
    {
        SRC_PRINT_OP_(OP_PARAMS_SEPARATOR);
        SRC_PRINT_(L" ");
        SrcNode(lang_ctx, node->right);
    }

    return LANG_SUCCESS;
}

//==========================================================================================

LangErr_t SrcWhile(LangCtx_t* lang_ctx, TreeNode_t* node)
{
    assert(lang_ctx);
    assert(node);

    SrcEnterRepetitionPrint(lang_ctx, OP_WHILE);
    SRC_PRINT_(L"\n\n");

    if (node->left)
        SrcNode(lang_ctx, node->left);

    SRC_PRINT_(L"\n");

    SRC_PRINT_TABS_();
    SrcRepetitionPrint(lang_ctx, OP_BLOCK_BEGIN);
    SRC_PRINT_(L"\n\n");

    lang_ctx->tabs++;

    if (node->right)
        SrcNode(lang_ctx, node->right);

    SRC_PRINT_(L"\n");

    lang_ctx->tabs--;

    SRC_PRINT_TABS_();
    SrcRepetitionPrint(lang_ctx, OP_BLOCK_END);
    SRC_PRINT_(L"\n\n");

    SRC_PRINT_TABS_();
    SRC_PRINT_OP_(OP_CMD_SEPARATOR);
    SRC_PRINT_(L"\n");

    return LANG_SUCCESS;
}

//==========================================================================================

LangErr_t SrcIf(LangCtx_t* lang_ctx, TreeNode_t* node)
{
    assert(lang_ctx);
    assert(node);

    SRC_PRINT_OP_(OP_IF_LHS);
    SRC_PRINT_(L"\n\n");

    if (node->left)
        SrcNode(lang_ctx, node->left);
    SRC_PRINT_(L"\n\n");

    SRC_PRINT_OP_(OP_IF_RHS);
    SRC_PRINT_(L"\n\n");

    SRC_PRINT_TABS_();
    SrcRepetitionPrint(lang_ctx, OP_BLOCK_BEGIN);
    SRC_PRINT_(L"\n");

    lang_ctx->tabs++;

    if (node->right)
        SrcNode(lang_ctx, node->right);

    SRC_PRINT_(L"\n");

    lang_ctx->tabs--;

    SRC_PRINT_TABS_();
    SrcRepetitionPrint(lang_ctx, OP_BLOCK_END);
    SRC_PRINT_(L"\n\n");

    SRC_PRINT_TABS_();
    SRC_PRINT_OP_(OP_CMD_SEPARATOR);
    SRC_PRINT_(L"\n");

    return LANG_SUCCESS;
}

//==========================================================================================

LangErr_t SrcCmdSeparator(LangCtx_t* lang_ctx, TreeNode_t* node)
{
    assert(lang_ctx);
    assert(node);

    if (node->left)
        SrcNode(lang_ctx, node->left);

    if (node->right)
        SrcNode(lang_ctx, node->right);

    return LANG_SUCCESS;
}

//==========================================================================================

static LangErr_t SrcRepetitionPrint(LangCtx_t* lang_ctx, Operator_t opcode)
{
    assert(lang_ctx);

    SRC_PRINT_OP_(opcode);

    for (int i = 1; i < OP_CASES_TABLE[opcode].repeat_times; i++)
    {
        SRC_PRINT_(L" ");
        SRC_PRINT_OP_(opcode);
    }

    return LANG_SUCCESS;
}

//==========================================================================================

static LangErr_t SrcEnterRepetitionPrint(LangCtx_t* lang_ctx, Operator_t opcode)
{
    assert(lang_ctx);

    SRC_PRINT_OP_(opcode);

    for (int i = 1; i < OP_CASES_TABLE[opcode].repeat_times; i++)
    {
        SRC_PRINT_TABS_();
        SRC_PRINT_(L"\n");
        SRC_PRINT_OP_(opcode);
    }

    return LANG_SUCCESS;
}

//==========================================================================================

static LangErr_t SrcNumber(LangCtx_t* lang_ctx, TreeNode_t* node)
{
    assert(lang_ctx);
    assert(node);

    SRC_PRINT_(L"%lg", node->data.value.number);

    return LANG_SUCCESS;
}

//==========================================================================================

static LangErr_t SrcVariable(LangCtx_t* lang_ctx, TreeNode_t* node)
{
    assert(lang_ctx);
    assert(node);

    SRC_PRINT_ID_(node);

    return LANG_SUCCESS;
}

//==========================================================================================

static LangErr_t SrcVariableDeclaration(LangCtx_t* lang_ctx, TreeNode_t* node)
{
    assert(lang_ctx);
    assert(node);

    SRC_PRINT_OP_(OP_VARIABLE_DECL);
    SRC_PRINT_(L" ");

    SRC_PRINT_ID_(node);
    SRC_PRINT_(L" ");

    SRC_PRINT_OP_(OP_CMD_SEPARATOR);
    SRC_PRINT_(L"\n");

    return LANG_SUCCESS;
}

//==========================================================================================

static LangErr_t SrcFunctionDeclaration(LangCtx_t* lang_ctx, TreeNode_t* node)
{
    assert(lang_ctx);
    assert(node);

    SRC_PRINT_OP_(OP_FUNCTION_DECL_LHS);
    SRC_PRINT_(L" ");

    SRC_PRINT_ID_(node);
    SRC_PRINT_(L" ");

    SRC_PRINT_OP_(OP_FUNCTION_DECL_RHS);
    SRC_PRINT_(L" ");

    SrcNode(lang_ctx, node->left);
    SRC_PRINT_(L"\n");

    lang_ctx->tabs++;

    SRC_PRINT_TABS_();
    SrcRepetitionPrint(lang_ctx, OP_FUNCTION_BLOCK_BEGIN);
    SRC_PRINT_(L"\n\n");

    SrcNode(lang_ctx, node->right);
    SRC_PRINT_(L"\n\n");

    SRC_PRINT_TABS_();
    SrcRepetitionPrint(lang_ctx, OP_FUNCTION_BLOCK_END);
    SRC_PRINT_(L"\n\n");

    SRC_PRINT_TABS_();
    SRC_PRINT_OP_(OP_CMD_SEPARATOR);
    SRC_PRINT_(L"\n");

    lang_ctx->tabs--;

    return LANG_SUCCESS;
}

//==========================================================================================

static LangErr_t SrcFunctionCall(LangCtx_t* lang_ctx, TreeNode_t* node)
{
    assert(lang_ctx);
    assert(node);

    SRC_PRINT_OP_(OP_FUNCTION_CALL_LHS);
    SRC_PRINT_(L" ");

    SRC_PRINT_ID_(node);
    SRC_PRINT_(L" ");

    SRC_PRINT_OP_(OP_FUNCTION_CALL_RHS);
    SRC_PRINT_(L" ");

    if (node->left)
        SrcNode(lang_ctx, node->left);
    SRC_PRINT_(L" ");

    return LANG_SUCCESS;
}

//==========================================================================================

#define _DSL_UNDEF_
#include "dsl.h"

//==========================================================================================
