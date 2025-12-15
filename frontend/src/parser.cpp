#include "parser.h"

//——————————————————————————————————————————————————————————————————————————————————————————

#define _DSL_DEFINE_
#include "dsl.h"

//——————————————————————————————————————————————————————————————————————————————————————————

#define PARSER_DUMP_(node, fmt, ...) \
        BEGIN \
        GRAPH_DUMP_(lang_ctx, (node), DUMP_FULL, fmt, ##__VA_ARGS__); \
        wfcprintf(stderr, PURPLE, L"dump %-30ls | %5p | %20s | type = %-3s | l = %-15p | r = %-15p \n", \
                   (fmt), (node), #node, TYPE_CASES_TABLE[(node)->data.type].name, (node)->left, (node)->right); \
        END

static TreeNode_t* ParseProgram            (LangCtx_t* lang_ctx);

static TreeNode_t* ParseExpression         (LangCtx_t* lang_ctx);
static TreeNode_t* ParseFactor             (LangCtx_t* lang_ctx);

static TreeNode_t* ParseBracketsExpression (LangCtx_t* lang_ctx);
static TreeNode_t* ParseFunctionCall       (LangCtx_t* lang_ctx);
static TreeNode_t* ParseFunctionArguments  (LangCtx_t* lang_ctx);

static TreeNode_t* ParseUnaryOperatorCall  (LangCtx_t* lang_ctx);
static TreeNode_t* ParseUnaryOperator      (LangCtx_t* lang_ctx);

//——————————————————————————————————————————————————————————————————————————————————————————

LangErr_t ParseTokens(LangCtx_t* lang_ctx)
{
    assert(lang_ctx);

    wfcprintf(stderr, BLUE, L"Parsing tokens...\n");

    TreeNode_t* root = ParseProgram(lang_ctx);

    if (root == NULL)
        return LANG_SYNTAX_ERROR;

    lang_ctx->tree.dummy->right = root;

    TREE_CALL_DUMP(lang_ctx, "parser");

    wfcprintf(stderr, GREEN, L"Parsing success\n");

    return LANG_SUCCESS;
}

//------------------------------------------------------------------------------------------

static TreeNode_t* ParseProgram(LangCtx_t* lang_ctx)
{
    assert(lang_ctx);

    TreeNode_t* cur_token = ParseExpression(lang_ctx);

    PARSER_DUMP_(cur_token, L"program");

    if (cur_token == NULL)
        return NULL;

    if (StackSize(&lang_ctx->tokens) != lang_ctx->cur_token_index)
    {
        WPRINTERR("INVALID TOKEN");
        return NULL;
    }

    return cur_token;
}

//------------------------------------------------------------------------------------------

static TreeNode_t* ParseExpression(LangCtx_t* lang_ctx)
{
    assert(lang_ctx);
    //TODO - actual function

    TreeNode_t* cur_token = ParseFactor(lang_ctx);

    PARSER_DUMP_(cur_token, L"expression");

    return cur_token;
}

//------------------------------------------------------------------------------------------

static TreeNode_t* ParseFactor(LangCtx_t* lang_ctx)
{
    assert(lang_ctx);

    TreeNode_t* cur_token = NULL;

    cur_token = ParseBracketsExpression(lang_ctx);
    //TODO - error check

    if (cur_token != NULL) // expression with brackets parsed successfully
        return cur_token;

    cur_token = ParseUnaryOperatorCall(lang_ctx);
    //TODO - error check

    if (cur_token != NULL) // unary op call parsed successfully
        return cur_token;

    cur_token = ParseFunctionCall(lang_ctx);
    //TODO - error check

    if (cur_token != NULL) // function call parsed successfully
        return cur_token;

    cur_token = LangGetCurrentToken(lang_ctx);
    //TODO - error check

    //TODO - variable instead of identifier
    if (IS_IDENTIFIER_(cur_token) || IS_NUMBER_(cur_token)) // number or variable parsed successfully
    {
        PARSER_DUMP_(cur_token, L"number | identifier");
        lang_ctx->cur_token_index++;
        return cur_token;
    }

    WPRINTERR("UNKNOWN TOKEN");
    //TODO - error set

    return NULL;
}

//------------------------------------------------------------------------------------------

static TreeNode_t* ParseBracketsExpression(LangCtx_t* lang_ctx)
{
    assert(lang_ctx);

    TreeNode_t* open_bracket = LangGetCurrentToken(lang_ctx);

    if (open_bracket == NULL || !IS_OPERATOR_(open_bracket, OP_BRACKET_OPEN))
        return NULL;

    lang_ctx->cur_token_index++;

    PARSER_DUMP_(open_bracket, L"opening bracket");

    TreeNode_t* cur_token = ParseExpression(lang_ctx);

    if (cur_token == NULL)
    {
        //TODO - seterror
        WPRINTERR("No expression after opening bracket");
        return NULL;
    }

    TreeNode_t* close_bracket = LangGetCurrentToken(lang_ctx);

    PARSER_DUMP_(close_bracket, L"closing bracket");

    if (close_bracket == NULL || !IS_OPERATOR_(close_bracket, OP_BRACKET_CLOSE))
    {
        //TODO - seterror
        WPRINTERR("No closing bracket after opening bracket");
        return NULL;
    }

    lang_ctx->cur_token_index++;

    PARSER_DUMP_(cur_token, L"in brackets expression");

    return cur_token;
}

//------------------------------------------------------------------------------------------

static TreeNode_t* ParseFunctionCall(LangCtx_t* lang_ctx)
{
    assert(lang_ctx);

    TreeNode_t* function_call_lhs = LangGetCurrentToken(lang_ctx);

    if (function_call_lhs == NULL)
        return NULL;

    if (!IS_OPERATOR_(function_call_lhs, OP_FUNCTION_CALL_LHS))
        return NULL;

    lang_ctx->cur_token_index++;

    TreeNode_t* function_name = LangGetCurrentToken(lang_ctx);

    if (function_name == NULL)
        return NULL;

    if (!IS_IDENTIFIER_(function_name))
    {
        WPRINTERR("There should be a function identifier after function call");
        // lang_ctx->error_info.error = LANG_SYNTAX_ERROR;
        //TODO - set error
        return NULL;
    }

    lang_ctx->cur_token_index++;

    TreeNode_t* function_call_rhs = LangGetCurrentToken(lang_ctx);

    if (function_call_rhs == NULL || !IS_OPERATOR_(function_call_rhs, OP_FUNCTION_CALL_RHS))
    {
        WPRINTERR("There should be a function call ending");
        //TODO - set error
        // lang_ctx->error_info.error = LANG_SYNTAX_ERROR;
        return NULL;
    }

    lang_ctx->cur_token_index++;

    function_call_lhs->left  = function_name;
    function_call_lhs->right = ParseFunctionArguments(lang_ctx);

    PARSER_DUMP_(function_call_lhs, L"function call");

//TODO - if (LangError()) --> мб макросики какие-нибудь

    return function_call_lhs;
}

//------------------------------------------------------------------------------------------

static TreeNode_t* ParseFunctionArguments(LangCtx_t* lang_ctx)
{
    assert(lang_ctx);

    TreeNode_t* cur_token = ParseExpression(lang_ctx); // moves cur_token_index by itself

    if (cur_token == NULL)
    {
        WDPRINTF(L"No args in function\n");
        return NULL;
    }

    PARSER_DUMP_(cur_token, L"function first argument");

    TreeNode_t* params_separator = LangGetCurrentToken(lang_ctx);

    if (params_separator == NULL)
        return cur_token;

    TreeNode_t* next_param = NULL;

    while (IS_OPERATOR_(params_separator, OP_PARAMS_SEPARATOR))
    {
        lang_ctx->cur_token_index++;

        params_separator->right = cur_token;

        cur_token = params_separator;

        next_param = ParseExpression(lang_ctx); // moves cur_token_index by itself

        if (next_param == NULL)
        {
            WPRINTERR("Should be an argument after param separator");
            return NULL;
        }

        cur_token->left = next_param;

        params_separator = LangGetCurrentToken(lang_ctx);

        if (params_separator == NULL)
            break;
    }

    PARSER_DUMP_(cur_token, L"function arguments all");

    return cur_token;
}

//------------------------------------------------------------------------------------------

static TreeNode_t* ParseUnaryOperatorCall(LangCtx_t* lang_ctx)
{
    assert(lang_ctx);

    TreeNode_t* cur_token = ParseUnaryOperator(lang_ctx);

    if (cur_token == NULL)
        return NULL;

    cur_token->right = ParseExpression(lang_ctx);

    PARSER_DUMP_(cur_token, L"unary operator call");

    if (cur_token->right == NULL)
    {
        WPRINTERR("After unary op there should be an expression");
        return NULL;
    }

    return cur_token;
}

//------------------------------------------------------------------------------------------

static TreeNode_t* ParseUnaryOperator(LangCtx_t* lang_ctx)
{
    assert(lang_ctx);

    TreeNode_t* cur_token = LangGetCurrentToken(lang_ctx);

    if (cur_token == NULL)
        return NULL;

    if (cur_token->data.type != TYPE_OP)
        return NULL;

    if (cur_token->data.value.opcode == OP_OUTPUT |
        cur_token->data.value.opcode == OP_INPUT)
    {
        PARSER_DUMP_(cur_token, L"unary operator");
        lang_ctx->cur_token_index++;
        return cur_token;
    }

    return NULL;
}

//------------------------------------------------------------------------------------------



//——————————————————————————————————————————————————————————————————————————————————————————

#define _DSL_UNDEF_
#include "dsl.h"

//——————————————————————————————————————————————————————————————————————————————————————————
