#include "parser.h"

//——————————————————————————————————————————————————————————————————————————————————————————

#define _DSL_DEFINE_
#include "dsl.h"

//——————————————————————————————————————————————————————————————————————————————————————————

static TreeNode_t* ParseProgram          (LangCtx_t* lang_ctx);

static TreeNode_t* ParseFunctionCall     (LangCtx_t* lang_ctx);
static TreeNode_t* ParseUnaryOperatorCall(LangCtx_t* lang_ctx);
static TreeNode_t* ParseUnaryOperator    (LangCtx_t* lang_ctx);

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

    TreeNode_t* cur_token = ParseUnaryOperator(lang_ctx);

    return cur_token;
}

//------------------------------------------------------------------------------------------

static TreeNode_t* ParseFunctionCall(LangCtx_t* lang_ctx)
{
    assert(lang_ctx);

    TreeNode_t* cur_token = ParseUnaryOperator(lang_ctx);

    if (cur_token == NULL)
        return NULL;

    // cur_token->right = ParseExpression(lang_ctx);

    return cur_token;
}

//------------------------------------------------------------------------------------------

static TreeNode_t* ParseUnaryOperatorCall(LangCtx_t* lang_ctx)
{
    assert(lang_ctx);

    TreeNode_t* cur_token = ParseUnaryOperator(lang_ctx);

    if (cur_token == NULL)
        return NULL;

    // cur_token->right = ParseExpression(lang_ctx);

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
