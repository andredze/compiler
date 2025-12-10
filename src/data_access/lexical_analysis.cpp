#include "lexical_analysis.h"
#include "lang_ctx.h"
#include <ctype.h>
#include <assert.h>

//——————————————————————————————————————————————————————————————————————————————————————————

#define _DSL_DEFINE_
#include "dsl.h"

//——————————————————————————————————————————————————————————————————————————————————————————

static LangErr_t ParseToken(LangCtx_t* lang_ctx);

//——————————————————————————————————————————————————————————————————————————————————————————

LangErr_t LexicallyAnalyze(LangCtx_t* lang_ctx)
{
    assert(lang_ctx != NULL);

    LangErr_t status = LANG_SUCCESS;

    while (*lang_ctx->code != '\0')
    {
        if ((status = ParseToken(lang_ctx)))
            return status;
    }

    return LANG_SUCCESS;
}

//——————————————————————————————————————————————————————————————————————————————————————————

static LangErr_t ProcessOperatorTokenCase   (LangCtx_t* lang_ctx, bool* got_token);
static LangErr_t ProcessNumberTokenCase     (LangCtx_t* lang_ctx, bool* got_token);
static LangErr_t ProcessIdentifierTokenCase (LangCtx_t* lang_ctx, bool* got_token);

//——————————————————————————————————————————————————————————————————————————————————————————

static LangErr_t ParseToken(LangCtx_t* lang_ctx)
{
    assert(lang_ctx);

    LangErr_t status    = LANG_SUCCESS;
    bool      got_token = false;

    status = ProcessOperatorTokenCase(lang_ctx, &got_token);

    if (status != LANG_SUCCESS || got_token)
        return status;

    status = ProcessNumberTokenCase(lang_ctx, &got_token);

    if (status != LANG_SUCCESS || got_token)
        return status;

    status = ProcessIdentifierTokenCase(lang_ctx, &got_token);

    if (status != LANG_SUCCESS || got_token)
        return status;

    SyntaxError();

    PRINTERR("Unknown symbol");

    return LANG_SYNTAX_ERROR;
}

//------------------------------------------------------------------------------------------

static LangErr_t ProcessOperatorTokenCase(LangCtx_t* lang_ctx, bool* got_token)
{
    assert(got_token);
    assert(lang_ctx);

    for (size_t op_code = 0; op_code < OP_COUNT; op_code++)
    {
        if (strncmp(*lang_ctx->code,
                    OP_CASES_TABLE[op_code].name,
                    OP_CASES_TABLE[op_code].name_len) == 0)
        {
            TreeNode_t* token = OPERATOR_(op_code);

            if (StackPush(lang_ctx->tokens, token))
            {
                PRINTERR("Failed stack push token");
                return LANG_STACK_ERROR;
            }

            lang_ctx->code += OP_CASES_TABLE[op_code].name_len;

            return LANG_SUCCESS;
        }
    }

    return LANG_SUCCESS;
}

//------------------------------------------------------------------------------------------

static LangErr_t ProcessNumberTokenCase(LangCtx_t* lang_ctx, bool* got_token)
{
    assert(got_token);
    assert(lang_ctx);

    if (!isdigit(*lang_ctx->code))
        return LANG_SUCCESS;

    double value        = 0.0;
    char*  num_code_end = NULL;

    value = strtod(*lang_ctx->code, &num_code_end);

    lang_ctx->code = num_code_end;

    TreeNode_t* token = NUMBER_(id_index);

    if (StackPush(lang_ctx->tokens, token))
    {
        PRINTERR("Failed stack push token");
        return LANG_STACK_ERROR;
    }

    return LANG_SUCCESS;
}

//——————————————————————————————————————————————————————————————————————————————————————————

static inline int IsAcceptableFirstSymbol(char ch);
static inline int IsAcceptableSymbol     (char ch);

//——————————————————————————————————————————————————————————————————————————————————————————

static LangErr_t ProcessIdentifierTokenCase(LangCtx_t* lang_ctx, bool* got_token)
{
    assert(got_token);
    assert(lang_ctx);

    if (!IsAcceptableFirstSymbol(*lang_ctx->code))
        return LANG_SUCCESS;

    char buf[MAX_OPERATOR_NAME_LEN] = "";

    for (size_t i = 0; IsAcceptableSymbol(*lang_ctx->code); i++)
    {
        buf[i] = *(lang_ctx->code++);
    }

    size_t id_index = 0;

    if ((status = LangIdTablePush(lang_ctx, buf, &id_index)))
        return status;

    TreeNode_t* token = IDENTIFIER_(id_index);

    if (StackPush(lang_ctx->tokens, token))
    {
        PRINTERR("Failed stack push token");
        return LANG_STACK_ERROR;
    }

    return LANG_SUCCESS;
}

//------------------------------------------------------------------------------------------

static inline int IsAcceptableSymbol(char ch)
{
    return isalnum(ch) || ch == '_';
}

//------------------------------------------------------------------------------------------

static inline int IsAcceptableFirstSymbol(char ch)
{
    return isalpha(ch) || ch == '_';
}

//——————————————————————————————————————————————————————————————————————————————————————————

#define _DSL_UNDEF_
#include "dsl.h"

//——————————————————————————————————————————————————————————————————————————————————————————
