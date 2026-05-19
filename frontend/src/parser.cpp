#include "parser.h"

//——————————————————————————————————————————————————————————————————————————————————————————

#define _DSL_DEFINE_
#include "dsl.h"

//——————————————————————————————————————————————————————————————————————————————————————————

#ifdef DEBUG
#define PARSER_DUMP_(node, fmt, ...)                                                    \
        BEGIN                                                                           \
        GRAPH_DUMP_(&frontend_ctx->lang_ctx, (node), DUMP_SHORT, fmt, ##__VA_ARGS__);   \
        if (node)                                                                       \
            wfcprintf(stderr, PURPLE, L"dump %-30ls | %5p | %20s | type = %-3s |"       \
                                      L"l = %-15p | r = %-15p      \n",                 \
                   (fmt), (node), #node, TYPE_CASES_TABLE[(node)->data.type].name,      \
                   (node)->left, (node)->right);                                        \
        END
#else
#define PARSER_DUMP_(node, fmt, ...) ;
#endif /* DEBUG */

//——————————————————————————————————————————————————————————————————————————————————————————

static TreeNode_t* ParseProgram             (FrontendCtx_t* frontend_ctx);
static TreeNode_t* ParseBody                (FrontendCtx_t* frontend_ctx);

static TreeNode_t* ParseCmdSeparator        (FrontendCtx_t* frontend_ctx);

static TreeNode_t* ParseFunctionDeclaration (FrontendCtx_t* frontend_ctx);
static TreeNode_t* ParseFunctionParameters  (FrontendCtx_t* frontend_ctx, 
                                             int*           params_count_p);

static TreeNode_t* ParseStatement           (FrontendCtx_t* frontend_ctx);
static TreeNode_t* ParseFunctionStatement   (FrontendCtx_t* frontend_ctx);
static TreeNode_t* ParseFunctionBlock       (FrontendCtx_t* frontend_ctx);

static TreeNode_t* ParseReturn              (FrontendCtx_t* frontend_ctx);

static TreeNode_t* ParseIfStatement         (FrontendCtx_t* frontend_ctx);
static TreeNode_t* ParseWhileStatement      (FrontendCtx_t* frontend_ctx);
static TreeNode_t* ParseBlockStatement      (FrontendCtx_t* frontend_ctx);

static TreeNode_t* ParseVariableDeclaration (FrontendCtx_t* frontend_ctx);

static TreeNode_t* ParseAssignment          (FrontendCtx_t* frontend_ctx);

static TreeNode_t* ParseExpression          (FrontendCtx_t* frontend_ctx);
static TreeNode_t* ParseTerm                (FrontendCtx_t* frontend_ctx);
static TreeNode_t* ParsePower               (FrontendCtx_t* frontend_ctx);
static TreeNode_t* ParseFactor              (FrontendCtx_t* frontend_ctx);

static TreeNode_t* ParseVariable            (FrontendCtx_t* frontend_ctx);

static TreeNode_t* ParseBracketsExpression  (FrontendCtx_t* frontend_ctx);
static TreeNode_t* ParseFunctionCall        (FrontendCtx_t* frontend_ctx);
static TreeNode_t* ParseFunctionArguments   (FrontendCtx_t* frontend_ctx, 
                                             int*           args_count_p);

static TreeNode_t* ParseLibFuncCall         (FrontendCtx_t* frontend_ctx);
static TreeNode_t* ParseLibFuncKeyword      (FrontendCtx_t* frontend_ctx);

static void        SetIdentifierTokenType   (LangCtx_t*     lang_ctx,
                                             TreeNode_t*    cur_token,
                                             TokenType_t    new_type);

//——————————————————————————————————————————————————————————————————————————————————————————

LangErr_t ParseTokens(FrontendCtx_t* frontend_ctx)
{
    assert(frontend_ctx);

    TreeNode_t* root = ParseProgram(frontend_ctx);

    if (root == NULL)
        return LANG_SYNTAX_ERROR;

    frontend_ctx->lang_ctx.tree.dummy->right = root;

    TREE_CALL_DUMP(&frontend_ctx->lang_ctx, frontend_ctx->buffer, "parser");

    return LANG_SUCCESS;
}

//==========================================================================================

static TreeNode_t* ParseProgram(FrontendCtx_t* frontend_ctx)
{
    assert(frontend_ctx);

    TreeNode_t* cur_token = ParseBody(frontend_ctx);

    if (cur_token == NULL)
        return NULL;

    PARSER_DUMP_(cur_token, L"program");

    if (StackSize(&frontend_ctx->tokens) != frontend_ctx->cur_token_index)
    {
        SYNTAX_ERROR_(L"INVALID TOKEN, size = %zu; cur_tok_ind = %zu",
                   StackSize(&frontend_ctx->tokens),
                   frontend_ctx->cur_token_index);
        return NULL;
    }

    return cur_token;
}

//==========================================================================================

static TreeNode_t* ParseBody(FrontendCtx_t* frontend_ctx)
{
    assert(frontend_ctx);

    TreeNode_t  dummy_root     = {};
    TreeNode_t* last_separator = &dummy_root;

    while (true)
    {
        TreeNode_t* statement = ParseStatement(frontend_ctx);

        if (statement == NULL)
            break;

        TreeNode_t* separator = ParseCmdSeparator(frontend_ctx);

        if (separator == NULL)
        {
            SET_PARSER_ERROR_(statement, L"Missing \"%ls\"", 
                                         GetKeywordName(KW_CMD_SEPARATOR));

            TreeNode_t* cur_tok = FrontendGetCurrentToken(frontend_ctx);

            SYNTAX_ERROR_(L"Missing \"%ls\"", GetKeywordName(KW_CMD_SEPARATOR));
            // PARSER_DUMP_(lang_ctx->tokens.data[frontend_ctx->cur_token_index - 1],
            //                 L"expected to have a separator after");
            return NULL;
        }

        separator->left       = statement;
        last_separator->right = separator;
        last_separator        = separator;

        PARSER_DUMP_(separator, L"body: separator");
    }

    if (dummy_root.right == NULL)
        return NULL;

    PARSER_DUMP_(dummy_root.right, L"body la finale");

    return dummy_root.right;
}

//==========================================================================================

static TreeNode_t* ParseFunctionDeclaration(FrontendCtx_t* frontend_ctx)
{
    assert(frontend_ctx);

    TreeNode_t* func_decl_lhs = FrontendGetCurrentToken(frontend_ctx);

    if (func_decl_lhs == NULL || !IS_KEYWORD_(func_decl_lhs, KW_FUNCTION_DECL_LHS))
        return NULL;

    frontend_ctx->cur_token_index++;
    frontend_ctx->is_in_func = 1;

    PARSER_DUMP_(func_decl_lhs, L"function declaration lhs");

    TreeNode_t* function_name = FrontendGetCurrentToken(frontend_ctx);

    if (function_name == NULL || !IS_IDENTIFIER_(function_name))
    {
        SET_PARSER_ERROR_(func_decl_lhs, L"Missing function name after \"%ls\"",
                                         GetKeywordName(KW_FUNCTION_DECL_LHS));

        SYNTAX_ERROR_(L"Expected function name after function declaration lhs");

        return NULL;
    }

    SetIdentifierTokenType(&frontend_ctx->lang_ctx, function_name, TYPE_FUNC_DECL);

    frontend_ctx->cur_token_index++;

    PARSER_DUMP_(function_name, L"function declaration: name %ls",
                 &frontend_ctx->lang_ctx.names_pool.data[function_name->data.value.id.name_index]);

    TreeNode_t* func_decl_rhs = FrontendGetCurrentToken(frontend_ctx);

    if (func_decl_rhs == NULL || !IS_KEYWORD_(func_decl_rhs, KW_FUNCTION_DECL_RHS))
        return NULL;

    frontend_ctx->cur_token_index++;

    PARSER_DUMP_(func_decl_rhs, L"function declaration rhs");

    int table_index = 0;

    if (LangIdTablePushFunctionIfUnique(&frontend_ctx->lang_ctx,
                                        function_name->data.value.id.name_index,
                                        &table_index))
    {
        return NULL;
    }

    function_name->data.value.id.id_index = (size_t) table_index;

    int params_count = 0;

    TreeNode_t* function_parameters = ParseFunctionParameters(frontend_ctx, &params_count);
    //TODO - add check error

    if (LangIdTableFunctionSetParamsCount(&frontend_ctx->lang_ctx.func_id_table,
                                          function_name->data.value.id.id_index,
                                          params_count))
    {
        return NULL;
    }

    //FIXME: can omit counting vars, and count them afterwards for all the table + addresses
    frontend_ctx->in_func_vars_count = 0;

    TreeNode_t* function_block = ParseFunctionBlock(frontend_ctx);

    if (function_block == NULL)
    {
        SET_PARSER_ERROR_(func_decl_rhs, L"Missing function \"%ls\" body",
                          LangGetIdName(&frontend_ctx->lang_ctx.names_pool, 
                                        function_name->data.value.id.name_index));

        SYNTAX_ERROR_(L"Expected function block after function declaration");
        return NULL;
    }

    PARSER_DUMP_(function_block, L"function declaration: got function block");

    function_name->left  = function_parameters;
    function_name->right = function_block;

    PARSER_DUMP_(function_name, L"function declaration la finale");

    WDPRINTF(L"params_count for func:  %ls = %d\n", 
             frontend_ctx->lang_ctx.names_pool.data[function_name->data.value.id.name_index], 
             params_count);

    WDPRINTF(L"in_func_vars_count for func: %ls = %d\n", 
             frontend_ctx->lang_ctx.names_pool.data[function_name->data.value.id.name_index], 
             frontend_ctx->in_func_vars_count);

    if (LangIdTableFunctionSetLocalsCount(&frontend_ctx->lang_ctx.func_id_table,
                                          function_name->data.value.id.id_index,
                                          frontend_ctx->in_func_vars_count))
    {
        return NULL;
    }

    frontend_ctx->in_func_vars_count = 0;
    frontend_ctx->is_in_func = 0;

    return function_name;
}

//==========================================================================================

static TreeNode_t* ParseFunctionParameters(FrontendCtx_t* frontend_ctx, 
                                           int*           params_count_p)
{
    assert(frontend_ctx);

    //FIXME: can omit counting params, and count them afterwards for all the table + addresses

    TreeNode_t* cur_token = FrontendGetCurrentToken(frontend_ctx);

    if (cur_token == NULL || !IS_IDENTIFIER_(cur_token))
        return NULL;

    SetIdentifierTokenType(&frontend_ctx->lang_ctx, cur_token, TYPE_VAR);

    (*params_count_p)++;

    frontend_ctx->cur_token_index++;

    PARSER_DUMP_(cur_token, L"function first parameter");

    int table_index = 0;

    if (LangIdTablePushVariableIfUnique(&frontend_ctx->lang_ctx,
                                        &frontend_ctx->lang_ctx.func_id_table, 
                                        cur_token->data.value.id.name_index,
                                        ID_TYPE_PARAMETER,
                                        &table_index))
    {
        return NULL;
    }

    cur_token->data.value.id.id_index = (size_t) table_index;

    TreeNode_t* params_separator = FrontendGetCurrentToken(frontend_ctx);

    if (params_separator == NULL)
        return cur_token;

    TreeNode_t* next_param = NULL;

    while (IS_KEYWORD_(params_separator, KW_PARAMS_SEPARATOR))
    {
        (*params_count_p)++;

        frontend_ctx->cur_token_index++;

        params_separator->left = cur_token;

        cur_token = params_separator;

        next_param = FrontendGetCurrentToken(frontend_ctx);

        if (next_param == NULL || !IS_IDENTIFIER_(next_param))
        {
            SYNTAX_ERROR_("Should be an parameter after param separator");
            return NULL;
        }

        SetIdentifierTokenType(&frontend_ctx->lang_ctx, next_param, TYPE_VAR);

        frontend_ctx->cur_token_index++;

        if (LangIdTablePushVariableIfUnique(&frontend_ctx->lang_ctx,
                                            &frontend_ctx->lang_ctx.func_id_table, 
                                            next_param->data.value.id.name_index,
                                            ID_TYPE_PARAMETER,
                                            &table_index))
        {
            return NULL;
        }

        next_param->data.value.id.id_index = (size_t) table_index;

        cur_token->right = next_param;

        params_separator = FrontendGetCurrentToken(frontend_ctx);

        if (params_separator == NULL)
            break;
    }

    PARSER_DUMP_(cur_token, L"function parameters all");

    return cur_token;
}

//==========================================================================================

static TreeNode_t* ParseFunctionBlock(FrontendCtx_t* frontend_ctx)
{
    assert(frontend_ctx);

    TreeNode_t* block_begin = FrontendGetCurrentToken(frontend_ctx);

    if (block_begin == NULL || !IS_KEYWORD_(block_begin, KW_FUNCTION_BLOCK_BEGIN))
        return NULL;

    frontend_ctx->cur_token_index++;

    PARSER_DUMP_(block_begin, L"function block: begin");

    TreeNode_t  dummy_root     = {};
    TreeNode_t* last_separator = &dummy_root;

    while (true)
    {
        TreeNode_t* statement = ParseFunctionStatement(frontend_ctx);

        if (statement == NULL)
            break;

        TreeNode_t* separator = ParseCmdSeparator(frontend_ctx);

        if (separator == NULL)
        {
            SYNTAX_ERROR_(L"There should be a cmd separator after function statement, cur_tok_ind = %zu",
                           frontend_ctx->cur_token_index);
            PARSER_DUMP_(frontend_ctx->tokens.data[frontend_ctx->cur_token_index - 1],
                            L"expected to have a separator after");
            return NULL;
        }

        separator->left       = statement;
        last_separator->right = separator;
        last_separator        = separator;

        PARSER_DUMP_(separator, L"function block: separator");
    }

    if (dummy_root.right == NULL)
        return NULL;

    PARSER_DUMP_(dummy_root.right, L"function block body");

    TreeNode_t* block_end = FrontendGetCurrentToken(frontend_ctx);

    if (block_end == NULL || !IS_KEYWORD_(block_end, KW_FUNCTION_BLOCK_END))
    {

        SYNTAX_ERROR_(L"Expected function block end");
        return NULL;
    }

    frontend_ctx->cur_token_index++;

    PARSER_DUMP_(block_end,        L"function block: end");
    PARSER_DUMP_(dummy_root.right, L"function block statement");

    return dummy_root.right;
}

//==========================================================================================

#define TRY_PARSING_(ParseFunc_, message)                                    \
        BEGIN                                                                \
            cur_token = ParseFunc_(frontend_ctx);                            \
                                                                             \
            if (cur_token != NULL)                                           \
            {                                                                \
                PARSER_DUMP_((cur_token), L"statement: got %ls", (message)); \
                return cur_token;                                            \
            }                                                                \
        END

//------------------------------------------------------------------//

static TreeNode_t* ParseStatement(FrontendCtx_t* frontend_ctx)
{
    assert(frontend_ctx);

    TreeNode_t* cur_token = NULL;

    //FIXME: set error and check after call; drops when syntax error
    TRY_PARSING_(ParseIfStatement        , L"statement");
    TRY_PARSING_(ParseWhileStatement     , L"statement");
    TRY_PARSING_(ParseVariableDeclaration, L"declaration");
    TRY_PARSING_(ParseFunctionDeclaration, L"declaration");
    TRY_PARSING_(ParseAssignment         , L"assignment");
    TRY_PARSING_(ParseExpression         , L"expression");

    return NULL;
}

#undef TRY_PARSING_

//==========================================================================================

static TreeNode_t* ParseFunctionStatement(FrontendCtx_t* frontend_ctx)
{
    assert(frontend_ctx);

    TreeNode_t* cur_token = NULL;

    cur_token = ParseReturn(frontend_ctx);

    if (cur_token != NULL)
    {
        PARSER_DUMP_(cur_token, L"function statement: got return statement");
        return cur_token;
    }

    cur_token = ParseStatement(frontend_ctx);

    if (cur_token != NULL)
    {
        PARSER_DUMP_(cur_token, L"function statement: got statement");
        return cur_token;
    }

    return NULL;
}

//==========================================================================================

static TreeNode_t* ParseReturn(FrontendCtx_t* frontend_ctx)
{
    assert(frontend_ctx);

    TreeNode_t* cur_token = FrontendGetCurrentToken(frontend_ctx);

    if (cur_token == NULL || !IS_KEYWORD_(cur_token, KW_RETURN))
        return NULL;

    PARSER_DUMP_(cur_token, L"return keyword");

    frontend_ctx->cur_token_index++;

    cur_token->right = ParseExpression(frontend_ctx);

    PARSER_DUMP_(cur_token, L"return");

    if (cur_token->right == NULL)
    {
        SYNTAX_ERROR_("After return there should be an expression");
        return NULL;
    }

    return cur_token;
}

//==========================================================================================

static TreeNode_t* ParseCondition(FrontendCtx_t* frontend_ctx);

//——————————————————————————————————————————————————————————————————————————————————————————

static TreeNode_t* ParseIfStatement(FrontendCtx_t* frontend_ctx)
{
    assert(frontend_ctx);

    TreeNode_t* if_lhs = FrontendGetCurrentToken(frontend_ctx);

    if (if_lhs == NULL || !IS_KEYWORD_(if_lhs, KW_IF_LHS))
        return NULL;

    frontend_ctx->cur_token_index++;

    PARSER_DUMP_(if_lhs, L"if_lhs");

    TreeNode_t* condition = ParseCondition(frontend_ctx);

    if (condition == NULL)
    {
        SYNTAX_ERROR_("There should be a condition inside if");
        return NULL;
    }

    PARSER_DUMP_(condition, L"if_condition");

    TreeNode_t* if_rhs = FrontendGetCurrentToken(frontend_ctx);

    if (if_rhs == NULL && !IS_KEYWORD_(if_rhs, KW_IF_RHS))
    {
        SYNTAX_ERROR_("There should be an if right side");
        return NULL;
    }

    frontend_ctx->cur_token_index++;

    PARSER_DUMP_(if_rhs, L"if_rhs");

    TreeNode_t* block_statement = ParseBlockStatement(frontend_ctx);

    if (block_statement == NULL)
    {
        SYNTAX_ERROR_("Expected block after if statement");
        return NULL;
    }

    PARSER_DUMP_(block_statement, L"if_block");

    if_lhs->left  = condition;
    if_lhs->right = block_statement;

    PARSER_DUMP_(if_lhs, L"if finale");

    return if_lhs;
}

//==========================================================================================

static TreeNode_t* ParseWhileStatement(FrontendCtx_t* frontend_ctx)
{
    assert(frontend_ctx);

    TreeNode_t* while_token = FrontendGetCurrentToken(frontend_ctx);

    if (while_token == NULL || !IS_KEYWORD_(while_token, KW_WHILE))
        return NULL;

    frontend_ctx->cur_token_index++;

    PARSER_DUMP_(while_token, L"while");

    TreeNode_t* condition = ParseCondition(frontend_ctx);

    if (condition == NULL)
    {
        SYNTAX_ERROR_("Expected condition inside while");
        return NULL;
    }

    PARSER_DUMP_(condition, L"while_condition");

    TreeNode_t* block_statement = ParseBlockStatement(frontend_ctx);

    if (block_statement == NULL)
    {
        SYNTAX_ERROR_("Expected block after while condition");
        return NULL;
    }

    PARSER_DUMP_(block_statement, L"while block");

    while_token->left  = condition;
    while_token->right = block_statement;

    PARSER_DUMP_(while_token, L"while finale");

    return while_token;
}

//==========================================================================================

static TreeNode_t* ParseCondition(FrontendCtx_t* frontend_ctx)
{
    assert(frontend_ctx);

    TreeNode_t* lhs = ParseExpression(frontend_ctx);

    if (lhs == NULL)
        return NULL;

    PARSER_DUMP_(lhs, L"condition: lhs");

    TreeNode_t* comp = FrontendGetCurrentToken(frontend_ctx);

    if (comp == NULL || !IS_TYPE_(comp, TYPE_KEYWORD) ||
        !(HAS_KEYWORD_(comp, KW_EQUAL        ) ||
          HAS_KEYWORD_(comp, KW_BIGGER_EQUAL ) ||
          HAS_KEYWORD_(comp, KW_BIGGER       ) ||
          HAS_KEYWORD_(comp, KW_SMALLER_EQUAL) ||
          HAS_KEYWORD_(comp, KW_SMALLER      )))
    {
        SYNTAX_ERROR_(L"Expected comparison sign in condition");
        return NULL;
    }

    PARSER_DUMP_(comp, L"condition: comp sign");

    frontend_ctx->cur_token_index++;

    TreeNode_t* rhs = ParseExpression(frontend_ctx);

    if (rhs == NULL)
    {
        SYNTAX_ERROR_(L"Expected second expression after sign in condition");
        return NULL;
    }

    PARSER_DUMP_(rhs, L"condition: rhs");

    comp->left  = lhs;
    comp->right = rhs;

    PARSER_DUMP_(comp, L"condition finale");

    return comp;
}

//==========================================================================================

static TreeNode_t* ParseBlockStatement(FrontendCtx_t* frontend_ctx)
{
    assert(frontend_ctx);

    TreeNode_t* block_open = FrontendGetCurrentToken(frontend_ctx);

    if (block_open == NULL || !IS_KEYWORD_(block_open, KW_BLOCK_BEGIN))
    {
        if (block_open)
        {
            WDPRINTF(L"Got type %ls\n", TYPE_CASES_TABLE[block_open->data.type].name);
        }
        return NULL;
    }

    frontend_ctx->cur_token_index++;

    PARSER_DUMP_(block_open, L"block open");

    TreeNode_t* statement = ParseBody(frontend_ctx);

    if (statement == NULL)
    {
        SYNTAX_ERROR_("Expected block body");
        return NULL;
    }

    TreeNode_t* block_close = FrontendGetCurrentToken(frontend_ctx);

    if (block_close == NULL || !IS_KEYWORD_(block_close, KW_BLOCK_END))
    {
        SYNTAX_ERROR_("There should be a block end");
        return NULL;
    }

    frontend_ctx->cur_token_index++;

    WDPRINTF(L"block stmt end: frontend_ctx->cur_token_index = %zu\n", frontend_ctx->cur_token_index);

    PARSER_DUMP_(block_close, L"block close");
    PARSER_DUMP_(statement, L"block statement");

    return statement;
}

//==========================================================================================

static TreeNode_t* ParseCmdSeparator(FrontendCtx_t* frontend_ctx)
{
    assert(frontend_ctx);

    TreeNode_t* op_separator = FrontendGetCurrentToken(frontend_ctx);

    if (op_separator == NULL || !IS_KEYWORD_(op_separator, KW_CMD_SEPARATOR))
    {
        return NULL;
    }

    frontend_ctx->cur_token_index++;

    return op_separator;
}

//==========================================================================================

static TreeNode_t* ParseVariableDeclaration(FrontendCtx_t* frontend_ctx)
{
    assert(frontend_ctx);

    TreeNode_t* var_decl = FrontendGetCurrentToken(frontend_ctx);

    if (var_decl == NULL || !IS_KEYWORD_(var_decl, KW_VARIABLE_DECL))
        return NULL;

    frontend_ctx->cur_token_index++;

    PARSER_DUMP_(var_decl, L"var declaration keyword");

    TreeNode_t* cur_token = FrontendGetCurrentToken(frontend_ctx);

    if (cur_token == NULL || !IS_IDENTIFIER_(cur_token))
    {
        SYNTAX_ERROR_(L"Expected identifier in variable declaration");
        return NULL;
    }

    SetIdentifierTokenType(&frontend_ctx->lang_ctx, cur_token, TYPE_VAR_DECL);

    frontend_ctx->cur_token_index++;

    PARSER_DUMP_(cur_token, L"var_declaration (finale): identifier");

    IdTable_t* id_table = NULL;

    if (frontend_ctx->is_in_func)
    {
        frontend_ctx->in_func_vars_count++;
        id_table = &frontend_ctx->lang_ctx.func_id_table;
    }
    else
    {
        id_table = &frontend_ctx->lang_ctx.main_id_table;
    }

    int table_index = 0;

    if (LangIdTablePushVariableIfUnique(&frontend_ctx->lang_ctx, 
                                        id_table, 
                                        cur_token->data.value.id.name_index,
                                        ID_TYPE_VARIABLE, 
                                        &table_index))
    {
        return NULL;
    }

    cur_token->data.value.id.id_index = (size_t) table_index;

    return cur_token;
}

//==========================================================================================

static TreeNode_t* ParseAssignment(FrontendCtx_t* frontend_ctx)
{
    assert(frontend_ctx);

    TreeNode_t* assignment_token = FrontendGetCurrentToken(frontend_ctx);

    if (assignment_token == NULL || !IS_KEYWORD_(assignment_token, KW_ASSIGNMENT))
        return NULL;

    frontend_ctx->cur_token_index++;

    TreeNode_t* cur_token = ParseVariable(frontend_ctx);

    if (cur_token == NULL)
    {
        SYNTAX_ERROR_("There should be an identifier after assignment");
        return NULL;
    }

    PARSER_DUMP_(cur_token, L"assignment: got variable");

    TreeNode_t* expression = ParseExpression(frontend_ctx);

    if (expression == NULL)
    {
        SYNTAX_ERROR_("There should be an expression after identifier in assignment");
        return NULL;
    }

    assignment_token->left  = cur_token;
    assignment_token->right = expression;

    PARSER_DUMP_(assignment_token, L"assignment");

    return assignment_token;
}

//==========================================================================================

static TreeNode_t* ParseExpression(FrontendCtx_t* frontend_ctx)
{
    assert(frontend_ctx);

    TreeNode_t* cur_token = ParseTerm(frontend_ctx); // moves cur_token_index by itself

    if (cur_token == NULL)
    {
        WDPRINTF(L"No term in expression parse\n");
        return NULL;
    }

    PARSER_DUMP_(cur_token, L"parse expression got first token");

    TreeNode_t* expr_token = FrontendGetCurrentToken(frontend_ctx);

    if (expr_token == NULL)
        return cur_token;

    TreeNode_t* next_token = NULL;

    while (IS_KEYWORD_(expr_token, KW_ADD) ||
           IS_KEYWORD_(expr_token, KW_SUB))
    {
        frontend_ctx->cur_token_index++;

        expr_token->left = cur_token;
        cur_token = expr_token;

        next_token = ParseTerm(frontend_ctx); // moves cur_token_index by itself

        if (next_token == NULL)
        {
            SYNTAX_ERROR_("Should be an argument after expression operation");
            return NULL;
        }

        cur_token->right = next_token;

        expr_token = FrontendGetCurrentToken(frontend_ctx);

        if (expr_token == NULL)
            break;
    }

    PARSER_DUMP_(cur_token, L"expression all");

    return cur_token;
}

//==========================================================================================

static TreeNode_t* ParseTerm(FrontendCtx_t* frontend_ctx)
{
    assert(frontend_ctx);

    TreeNode_t* cur_token = ParsePower(frontend_ctx); // moves cur_token_index by itself

    if (cur_token == NULL)
    {
        WDPRINTF(L"No power in term parse\n");
        return NULL;
    }

    PARSER_DUMP_(cur_token, L"parse term got first token");

    TreeNode_t* term_token = FrontendGetCurrentToken(frontend_ctx);

    if (term_token == NULL)
        return cur_token;

    TreeNode_t* next_token = NULL;

    while (IS_KEYWORD_(term_token, KW_MUL) ||
           IS_KEYWORD_(term_token, KW_DIV))
    {
        frontend_ctx->cur_token_index++;

        term_token->left = cur_token;

        cur_token = term_token;

        next_token = ParsePower(frontend_ctx); // moves cur_token_index by itself

        if (next_token == NULL)
        {
            SYNTAX_ERROR_("Should be an argument after term operation");
            return NULL;
        }

        cur_token->right = next_token;

        term_token = FrontendGetCurrentToken(frontend_ctx);

        if (term_token == NULL)
            break;
    }

    PARSER_DUMP_(cur_token, L"term all");

    return cur_token;
}

//==========================================================================================

static TreeNode_t* ParsePower(FrontendCtx_t* frontend_ctx)
{
    assert(frontend_ctx);

    TreeNode_t* cur_token = ParseFactor(frontend_ctx); // moves cur_token_index by itself

    if (cur_token == NULL)
    {
        WDPRINTF(L"No factor in parse power\n");
        return NULL;
    }

    PARSER_DUMP_(cur_token, L"parse power got first token");

    TreeNode_t* power_token = FrontendGetCurrentToken(frontend_ctx);

    if (power_token == NULL)
        return cur_token;

    TreeNode_t* next_token = NULL;

    while (IS_KEYWORD_(power_token, KW_POW))
    {
        frontend_ctx->cur_token_index++;

        power_token->left = cur_token;

        cur_token = power_token;

        next_token = ParseFactor(frontend_ctx); // moves cur_token_index by itself

        if (next_token == NULL)
        {
            SYNTAX_ERROR_("Should be an argument after power operation");
            return NULL;
        }

        cur_token->right = next_token;

        power_token = FrontendGetCurrentToken(frontend_ctx);

        if (power_token == NULL)
            break;
    }

    PARSER_DUMP_(cur_token, L"power all");

    return cur_token;
}

//==========================================================================================

static TreeNode_t* ParseFactor(FrontendCtx_t* frontend_ctx)
{
    assert(frontend_ctx);

    TreeNode_t* cur_token = NULL;

    cur_token = ParseBracketsExpression(frontend_ctx);
    //TODO - error check

    if (cur_token != NULL) // expression with brackets parsed successfully
        return cur_token;

    cur_token = ParseLibFuncCall(frontend_ctx);
    //TODO - error check

    if (cur_token != NULL) // unary op call parsed successfully
        return cur_token;

    cur_token = ParseFunctionCall(frontend_ctx);
    //TODO - error check

    if (cur_token != NULL) // function call parsed successfully
        return cur_token;

    cur_token = ParseVariable(frontend_ctx);

    if (cur_token != NULL) // variable parsed successfully
        return cur_token;

    cur_token = FrontendGetCurrentToken(frontend_ctx);
    //TODO - error check

    if (cur_token && IS_NUMBER_(cur_token)) // number or variable parsed successfully
    {
        PARSER_DUMP_(cur_token, L"number");

        frontend_ctx->cur_token_index++;

        return cur_token;
    }

    // WPRINTERR("UNKNOWN TOKEN");
    //TODO - error set

    return NULL;
}

//==========================================================================================

static TreeNode_t* ParseVariable(FrontendCtx_t* frontend_ctx)
{
    assert(frontend_ctx);

    TreeNode_t* cur_token = FrontendGetCurrentToken(frontend_ctx);

    if (cur_token == NULL || !IS_IDENTIFIER_(cur_token))
        return NULL;

    SetIdentifierTokenType(&frontend_ctx->lang_ctx, cur_token, TYPE_VAR);

    frontend_ctx->cur_token_index++;

    IdTable_t* id_table = NULL;

    if (frontend_ctx->is_in_func)
    {
        id_table = &frontend_ctx->lang_ctx.func_id_table;
    }
    else
    {
        id_table = &frontend_ctx->lang_ctx.main_id_table;
    }

    int table_index = LangIdTableVarGetTableIndex(id_table, cur_token->data.value.id.name_index);

    if (table_index == -1)
    {
        SYNTAX_ERROR_(L"Variable %ls was not declared\n",
                      frontend_ctx->lang_ctx.names_pool.data[cur_token->data.value.id.name_index]);
        return NULL;
    }

    cur_token->data.value.id.id_index = (size_t) table_index;

    PARSER_DUMP_(cur_token, L"variable");

    return cur_token;
}

//==========================================================================================

static TreeNode_t* ParseBracketsExpression(FrontendCtx_t* frontend_ctx)
{
    assert(frontend_ctx);

    TreeNode_t* open_bracket = FrontendGetCurrentToken(frontend_ctx);

    if (open_bracket == NULL || !IS_KEYWORD_(open_bracket, KW_BRACKET_OPEN))
        return NULL;

    frontend_ctx->cur_token_index++;

    PARSER_DUMP_(open_bracket, L"opening bracket");

    TreeNode_t* cur_token = ParseExpression(frontend_ctx);

    if (cur_token == NULL)
    {
        //TODO - seterror
        SYNTAX_ERROR_("No expression after opening bracket");
        return NULL;
    }

    TreeNode_t* close_bracket = FrontendGetCurrentToken(frontend_ctx);

    PARSER_DUMP_(close_bracket, L"closing bracket");

    if (close_bracket == NULL || !IS_KEYWORD_(close_bracket, KW_BRACKET_CLOSE))
    {
        //TODO - seterror
        SYNTAX_ERROR_("No closing bracket after opening bracket");
        return NULL;
    }

    frontend_ctx->cur_token_index++;

    PARSER_DUMP_(cur_token, L"in brackets expression");

    return cur_token;
}

//==========================================================================================

static TreeNode_t* ParseFunctionCall(FrontendCtx_t* frontend_ctx)
{
    assert(frontend_ctx);

    TreeNode_t* function_call_lhs = FrontendGetCurrentToken(frontend_ctx);

    if (function_call_lhs == NULL)
        return NULL;

    if (!IS_KEYWORD_(function_call_lhs, KW_FUNCTION_CALL_LHS))
        return NULL;

    frontend_ctx->cur_token_index++;

    TreeNode_t* function_name = FrontendGetCurrentToken(frontend_ctx);

    if (function_name == NULL || !IS_IDENTIFIER_(function_name))
    {
        SYNTAX_ERROR_("There should be a function identifier after function call");
        // lang_ctx->error_info.error = LANG_SYNTAX_ERROR;
        //TODO - set error
        return NULL;
    }

    SetIdentifierTokenType(&frontend_ctx->lang_ctx, function_name, TYPE_FUNC_CALL);

    frontend_ctx->cur_token_index++;

    int func_id_index = LangIdTableGetFuncTableIndex(&frontend_ctx->lang_ctx.func_id_table, 
                                                     function_name->data.value.id.name_index);

    if (func_id_index == -1)
    {
        SYNTAX_ERROR_(L"Function %ls was not declared\n",
                      LangGetIdName(&frontend_ctx->lang_ctx.names_pool, 
                                function_name->data.value.id.name_index));
        return NULL;
    }

    function_name->data.value.id.id_index = (size_t) func_id_index;

    TreeNode_t* function_call_rhs = FrontendGetCurrentToken(frontend_ctx);

    if (function_call_rhs == NULL || !IS_KEYWORD_(function_call_rhs, KW_FUNCTION_CALL_RHS))
    {
        SYNTAX_ERROR_("There should be a function call ending");
        //TODO - set error
        // lang_ctx->error_info.error = LANG_SYNTAX_ERROR;
        return NULL;
    }

    frontend_ctx->cur_token_index++;

    int args_count = 0;

    function_name->left = ParseFunctionArguments(frontend_ctx, &args_count);

    WDPRINTF(L"args_count for func %ls = %d\n", 
             LangGetIdName(&frontend_ctx->lang_ctx.names_pool, 
                           function_name->data.value.id.name_index), 
             args_count);

    if (LangIsFuncCallArgsCorrect(&frontend_ctx->lang_ctx, func_id_index, args_count))
    {
        return NULL;
    }

    PARSER_DUMP_(function_name, L"function call");

//TODO - if (LangError()) --> мб макросики какие-нибудь

    return function_name;
}

//==========================================================================================

static TreeNode_t* ParseFunctionArguments(FrontendCtx_t* frontend_ctx, int* args_count_p)
{
    assert(frontend_ctx);

    *args_count_p = 0;

    TreeNode_t* cur_token = ParseExpression(frontend_ctx); // moves cur_token_index by itself

    if (cur_token == NULL)
    {
        WDPRINTF(L"No args in function\n");
        return NULL;
    }

    PARSER_DUMP_(cur_token, L"function first argument");

    (*args_count_p)++;

    TreeNode_t* params_separator = FrontendGetCurrentToken(frontend_ctx);

    if (params_separator == NULL)
        return cur_token;

    TreeNode_t* next_param = NULL;

    while (IS_KEYWORD_(params_separator, KW_PARAMS_SEPARATOR))
    {
        (*args_count_p)++;

        frontend_ctx->cur_token_index++;

        params_separator->left = cur_token;

        cur_token = params_separator;

        next_param = ParseExpression(frontend_ctx); // moves cur_token_index by itself

        if (next_param == NULL)
        {
            SYNTAX_ERROR_("Should be an argument after param separator");
            return NULL;
        }

        cur_token->right = next_param;

        params_separator = FrontendGetCurrentToken(frontend_ctx);

        if (params_separator == NULL)
            break;
    }

    PARSER_DUMP_(cur_token, L"function arguments all");

    return cur_token;
}

//==========================================================================================

static TreeNode_t* ParseLibFuncCall(FrontendCtx_t* frontend_ctx)
{
    assert(frontend_ctx);

    TreeNode_t* cur_token = ParseLibFuncKeyword(frontend_ctx);

    if (cur_token == NULL)
        return NULL;

    int args_count = 0;

    cur_token->right = ParseFunctionArguments(frontend_ctx, &args_count);

    PARSER_DUMP_(cur_token, L"unary operator call");

    int expected_args_count = 0;

    switch (cur_token->data.value.keyword)
    {
        case KW_OUTPUT:         expected_args_count = 1; break;
        case KW_SQRT:           expected_args_count = 1; break;
        case KW_DRAW:           expected_args_count = 0; break;
        case KW_POINT:          expected_args_count = 3; break;
        case KW_RETURN:         expected_args_count = 1; break;
        case KW_INIT_SCREEN:    expected_args_count = 2; break;
        case KW_DEL_SCREEN:     expected_args_count = 0; break;
        case KW_DRAW_SCREEN:    expected_args_count = 0; break;
        case KW_POINT_SCREEN:   expected_args_count = 6; break;
        case KW_INPUT:
            expected_args_count = 1; 
            if (!(IS_VARIABLE_(cur_token->right)))
            {
                SYNTAX_ERROR_(L"Input lib func call argument is not a variable");
                return NULL;
            }
            break;

        default:
            SYNTAX_ERROR_(L"Current token is not a lib func call");
            return NULL;
    }

    if (expected_args_count != args_count)
    {
        SYNTAX_ERROR_(L"Lib func call wrong args count %d (expected %d)", 
                      args_count, expected_args_count);
        return NULL;
    }

    return cur_token;
}

//==========================================================================================

static TreeNode_t* ParseLibFuncKeyword(FrontendCtx_t* frontend_ctx)
{
    assert(frontend_ctx);

    TreeNode_t* cur_token = FrontendGetCurrentToken(frontend_ctx);

    if (cur_token == NULL || cur_token->data.type != TYPE_KEYWORD)
        return NULL;

    if ((cur_token->data.value.keyword == KW_OUTPUT       ) |
        (cur_token->data.value.keyword == KW_INPUT        ) |
        (cur_token->data.value.keyword == KW_SQRT         ) |
        (cur_token->data.value.keyword == KW_DRAW         ) |
        (cur_token->data.value.keyword == KW_RETURN       ) |
        (cur_token->data.value.keyword == KW_POINT        ) |
        (cur_token->data.value.keyword == KW_INIT_SCREEN  ) |
        (cur_token->data.value.keyword == KW_DEL_SCREEN   ) |
        (cur_token->data.value.keyword == KW_DRAW_SCREEN  ) |
        (cur_token->data.value.keyword == KW_POINT_SCREEN ))
    {
        PARSER_DUMP_(cur_token, L"lib func call keyword");
        frontend_ctx->cur_token_index++;
        return cur_token;
    }

    return NULL;
}

//==========================================================================================

static void SetIdentifierTokenType(LangCtx_t* lang_ctx, TreeNode_t* cur_token,
                                   TokenType_t new_type)
{
    assert(cur_token);
    assert(lang_ctx);

    assert(cur_token->data.type == TYPE_ID);

    cur_token->data.type = new_type;
}

//==========================================================================================

//——————————————————————————————————————————————————————————————————————————————————————————

#define _DSL_UNDEF_
#include "dsl.h"

//——————————————————————————————————————————————————————————————————————————————————————————
