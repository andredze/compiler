#include "parser.h"

//——————————————————————————————————————————————————————————————————————————————————————————

#define _DSL_DEFINE_
#include "dsl.h"

//——————————————————————————————————————————————————————————————————————————————————————————

#define PARSER_DUMP_(node, fmt, ...)                                              \
        BEGIN                                                                     \
        GRAPH_DUMP_(lang_ctx, (node), DUMP_SHORT, fmt, ##__VA_ARGS__);            \
        if (node)                                                                 \
            wfcprintf(stderr, PURPLE, L"dump %-30ls | %5p | %20s | type = %-3s |" \
                                      L"l = %-15p | r = %-15p      \n",           \
                   (fmt), (node), #node, TYPE_CASES_TABLE[(node)->data.type].name,\
                   (node)->left, (node)->right);                                  \
        END

//FIXME - ноды наоборот лево/право

static TreeNode_t* ParseProgram            (LangCtx_t* lang_ctx);
static TreeNode_t* ParseBody               (LangCtx_t* lang_ctx);

static TreeNode_t* ParseCmdSeparator       (LangCtx_t* lang_ctx);

static TreeNode_t* ParseStatement          (LangCtx_t* lang_ctx);

static TreeNode_t* ParseIfStatement        (LangCtx_t* lang_ctx);
static TreeNode_t* ParseBlockStatement     (LangCtx_t* lang_ctx);

static TreeNode_t* ParseAssignment         (LangCtx_t* lang_ctx);

static TreeNode_t* ParseExpression         (LangCtx_t* lang_ctx);
static TreeNode_t* ParseTerm               (LangCtx_t* lang_ctx);
static TreeNode_t* ParsePower              (LangCtx_t* lang_ctx);
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

    TreeNode_t* cur_token = ParseBody(lang_ctx);

    PARSER_DUMP_(cur_token, L"program");

    if (cur_token == NULL)
        return NULL;

    if (StackSize(&lang_ctx->tokens) != lang_ctx->cur_token_index)
    {
        WPRINTERR("INVALID TOKEN, size = %zu; cur_tok_ind = %zu",
                   StackSize(&lang_ctx->tokens),
                   lang_ctx->cur_token_index);
        return NULL;
    }

    return cur_token;
}

//------------------------------------------------------------------------------------------

static TreeNode_t* ParseBody(LangCtx_t* lang_ctx)
{
    assert(lang_ctx);

    TreeNode_t* statement      = ParseStatement(lang_ctx);
    TreeNode_t* next_statement = NULL;

    while (statement != NULL)
    {
        TreeNode_t* separator = ParseCmdSeparator(lang_ctx);

        if (separator == NULL)
        {
            WPRINTERR("There should be a cmd separator after statement, cur_tok_ind = %zu",
                      lang_ctx->cur_token_index);
            if (separator) PARSER_DUMP_(separator, L"expected to be a separator");
            return NULL;
        }

        next_statement = ParseStatement(lang_ctx);

        /*NOTE - to not make empty nodes with semicolons,
                 move two next lines after if
                 (left for compatibility with other people)
        */

        if (next_statement == NULL)
            break;

        separator->left = statement;
        statement = separator;

        statement->right = next_statement;
    }

    PARSER_DUMP_(statement, L"body");

    return statement;
}

//------------------------------------------------------------------------------------------

static TreeNode_t* ParseStatement(LangCtx_t* lang_ctx)
{
    assert(lang_ctx);

    TreeNode_t* cur_token = NULL;

    cur_token = ParseIfStatement(lang_ctx);

    if (cur_token != NULL)
    {
        PARSER_DUMP_(cur_token, L"statement");

        return cur_token;
    }

    cur_token = ParseAssignment(lang_ctx);

    if (cur_token != NULL)
    {
        PARSER_DUMP_(cur_token, L"statement");

        return cur_token;
    }

    cur_token = ParseExpression(lang_ctx);

    if (cur_token != NULL)
    {
        PARSER_DUMP_(cur_token, L"statement");

        return cur_token;
    }

    return NULL;
}

//------------------------------------------------------------------------------------------

static TreeNode_t* ParseIfStatement(LangCtx_t* lang_ctx)
{
    assert(lang_ctx);

    TreeNode_t* if_lhs = LangGetCurrentToken(lang_ctx);

    if (if_lhs == NULL || !IS_OPERATOR_(if_lhs, OP_IF_LHS))
        return NULL;

    lang_ctx->cur_token_index++;

    PARSER_DUMP_(if_lhs, L"if_lhs");

    TreeNode_t* condition = ParseExpression(lang_ctx);

    if (condition == NULL)
    {
        WPRINTERR("There should be an expression inside if");
        return NULL;
    }

    PARSER_DUMP_(condition, L"if_condition");

    TreeNode_t* if_rhs = LangGetCurrentToken(lang_ctx);

    if (if_rhs == NULL && !IS_OPERATOR_(if_rhs, OP_IF_RHS))
    {
        WPRINTERR("There should be an if right side");
        return NULL;
    }

    lang_ctx->cur_token_index++;

    PARSER_DUMP_(if_rhs, L"if_rhs");

    TreeNode_t* block_statement = ParseBlockStatement(lang_ctx);

    if (block_statement == NULL)
    {
        WPRINTERR("Expected block after if statement");
        return NULL;
    }

    PARSER_DUMP_(block_statement, L"if_block");

    if_lhs->left  = condition;
    if_lhs->right = block_statement;

    PARSER_DUMP_(if_lhs, L"if finale");

    return if_lhs;
}

//------------------------------------------------------------------------------------------

static TreeNode_t* ParseBlockStatement(LangCtx_t* lang_ctx)
{
    assert(lang_ctx);

    TreeNode_t* block_open = LangGetCurrentToken(lang_ctx);

    if (block_open == NULL || !IS_OPERATOR_(block_open, OP_BLOCK_BEGIN))
        return NULL;

    lang_ctx->cur_token_index++;

    PARSER_DUMP_(block_open, L"block open");

    TreeNode_t* statement      = ParseStatement(lang_ctx);
    TreeNode_t* next_statement = NULL;

    while (statement != NULL)
    {
        TreeNode_t* separator = ParseCmdSeparator(lang_ctx);

        if (separator == NULL)
        {
            WPRINTERR("There should be a cmd separator after statement");
            return NULL;
        }

        next_statement = ParseStatement(lang_ctx);

        // /*NOTE - to not make empty nodes with semicolons,
        //          move two next lines after if
        //          (left for compatibility with other people)
        // */

        if (next_statement == NULL)
            break;

        separator->left = statement;
        statement = separator;

        statement->right = next_statement;
    }

    TreeNode_t* block_close = LangGetCurrentToken(lang_ctx);

    if (block_close == NULL || !IS_OPERATOR_(block_close, OP_BLOCK_END))
    {
        WPRINTERR("There should be a block end");
        return NULL;
    }

    lang_ctx->cur_token_index++;

    WDPRINTF(L"block stmt end: lang_ctx->cur_token_index = %zu\n", lang_ctx->cur_token_index);

    PARSER_DUMP_(block_close, L"block close");
    PARSER_DUMP_(statement, L"block statement");

    return statement;
}

//------------------------------------------------------------------------------------------

static TreeNode_t* ParseCmdSeparator(LangCtx_t* lang_ctx)
{
    assert(lang_ctx);

    TreeNode_t* op_separator = LangGetCurrentToken(lang_ctx);

    if (op_separator == NULL || !IS_OPERATOR_(op_separator, OP_CMD_SEPARATOR))
    {
        return NULL;
    }

    lang_ctx->cur_token_index++;

    return op_separator;
}

//------------------------------------------------------------------------------------------

static TreeNode_t* ParseAssignment(LangCtx_t* lang_ctx)
{
    assert(lang_ctx);

    TreeNode_t* assignment_token = LangGetCurrentToken(lang_ctx);

    if (assignment_token == NULL || !IS_OPERATOR_(assignment_token, OP_ASSIGNMENT))
        return NULL;

    lang_ctx->cur_token_index++;

    TreeNode_t* cur_token = LangGetCurrentToken(lang_ctx);

    if (cur_token == NULL || !IS_IDENTIFIER_(cur_token))
    {
        WPRINTERR("There should be an identifier after assignment");
        return NULL;
    }

    lang_ctx->cur_token_index++;

    TreeNode_t* expression = ParseExpression(lang_ctx);

    if (expression == NULL)
    {
        WPRINTERR("There should be an expression after identifier in assignment");
        return NULL;
    }

    assignment_token->left  = cur_token;
    assignment_token->right = expression;

    PARSER_DUMP_(assignment_token, L"assignment");

    return assignment_token;
}

//------------------------------------------------------------------------------------------

static TreeNode_t* ParseExpression(LangCtx_t* lang_ctx)
{
    assert(lang_ctx);

    TreeNode_t* cur_token = ParseTerm(lang_ctx); // moves cur_token_index by itself

    if (cur_token == NULL)
    {
        WDPRINTF(L"No term in expression parse\n");
        return NULL;
    }

    PARSER_DUMP_(cur_token, L"parse expression got first token");

    TreeNode_t* expr_token = LangGetCurrentToken(lang_ctx);

    if (expr_token == NULL)
        return cur_token;

    TreeNode_t* next_token = NULL;

    while (IS_OPERATOR_(expr_token, OP_ADD) ||
           IS_OPERATOR_(expr_token, OP_SUB))
    {
        lang_ctx->cur_token_index++;

        expr_token->left = cur_token;
        cur_token = expr_token;

        next_token = ParseTerm(lang_ctx); // moves cur_token_index by itself

        if (next_token == NULL)
        {
            WPRINTERR("Should be an argument after expression operation");
            return NULL;
        }

        cur_token->right = next_token;

        expr_token = LangGetCurrentToken(lang_ctx);

        if (expr_token == NULL)
            break;
    }

    PARSER_DUMP_(cur_token, L"expression all");

    return cur_token;
}

//------------------------------------------------------------------------------------------

static TreeNode_t* ParseTerm(LangCtx_t* lang_ctx)
{
    assert(lang_ctx);

    TreeNode_t* cur_token = ParsePower(lang_ctx); // moves cur_token_index by itself

    if (cur_token == NULL)
    {
        WDPRINTF(L"No power in term parse\n");
        return NULL;
    }

    PARSER_DUMP_(cur_token, L"parse term got first token");

    TreeNode_t* term_token = LangGetCurrentToken(lang_ctx);

    if (term_token == NULL)
        return cur_token;

    TreeNode_t* next_token = NULL;

    while (IS_OPERATOR_(term_token, OP_MUL) ||
           IS_OPERATOR_(term_token, OP_DIV))
    {
        lang_ctx->cur_token_index++;

        term_token->left = cur_token;

        cur_token = term_token;

        next_token = ParsePower(lang_ctx); // moves cur_token_index by itself

        if (next_token == NULL)
        {
            WPRINTERR("Should be an argument after term operation");
            return NULL;
        }

        cur_token->right = next_token;

        term_token = LangGetCurrentToken(lang_ctx);

        if (term_token == NULL)
            break;
    }

    PARSER_DUMP_(cur_token, L"term all");

    return cur_token;
}

//------------------------------------------------------------------------------------------

static TreeNode_t* ParsePower(LangCtx_t* lang_ctx)
{
    assert(lang_ctx);

    TreeNode_t* cur_token = ParseFactor(lang_ctx); // moves cur_token_index by itself

    if (cur_token == NULL)
    {
        WDPRINTF(L"No factor in parse power\n");
        return NULL;
    }

    PARSER_DUMP_(cur_token, L"parse power got first token");

    TreeNode_t* power_token = LangGetCurrentToken(lang_ctx);

    if (power_token == NULL)
        return cur_token;

    TreeNode_t* next_token = NULL;

    while (IS_OPERATOR_(power_token, OP_POW))
    {
        lang_ctx->cur_token_index++;

        power_token->left = cur_token;

        cur_token = power_token;

        next_token = ParseFactor(lang_ctx); // moves cur_token_index by itself

        if (next_token == NULL)
        {
            WPRINTERR("Should be an argument after power operation");
            return NULL;
        }

        cur_token->right = next_token;

        power_token = LangGetCurrentToken(lang_ctx);

        if (power_token == NULL)
            break;
    }

    PARSER_DUMP_(cur_token, L"power all");

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
    if (cur_token && (IS_IDENTIFIER_(cur_token) || IS_NUMBER_(cur_token))) // number or variable parsed successfully
    {
        PARSER_DUMP_(cur_token, L"number | identifier");
        lang_ctx->cur_token_index++;
        return cur_token;
    }

    // WPRINTERR("UNKNOWN TOKEN");
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

    if ((cur_token->data.value.opcode == OP_OUTPUT) |
        (cur_token->data.value.opcode == OP_INPUT))
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
