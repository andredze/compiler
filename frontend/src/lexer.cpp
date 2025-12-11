#include "lexer.h"
#include "lang_ctx.h"
#include "lang_funcs.h"
#include <ctype.h>
#include <assert.h>

//——————————————————————————————————————————————————————————————————————————————————————————

#define _DSL_DEFINE_
#include "dsl.h"

//——————————————————————————————————————————————————————————————————————————————————————————

static void PrintSyntaxError(LangCtx_t* lang_ctx, const char* file, const char* func,
                             const int  line,     const char* fmt, ...);

//------------------------------------------------------------------------------------------

#define SYNTAX_ERROR(lang_ctx, fmt, ...)                                                                \
        {   TreeDumpInfo_t dump_info = {TREE_SUCCESS, __PRETTY_FUNCTION__, __FILE__, __LINE__};         \
            TreeDump((lang_ctx), &dump_info, DUMP_SHORT, (fmt), ##__VA_ARGS__);                         \
            PrintSyntaxError(lang_ctx, __FILE__, __PRETTY_FUNCTION__, __LINE__, (fmt), ##__VA_ARGS__);  \
            return LANG_SYNTAX_ERROR; }

//——————————————————————————————————————————————————————————————————————————————————————————

static LangErr_t ParseToken(LangCtx_t* lang_ctx);

//——————————————————————————————————————————————————————————————————————————————————————————

static void PrintSyntaxError(LangCtx_t* lang_ctx, const char* file, const char* func,
                             const int  line,     const char* fmt, ...)
{
    char message[MAX_SYNTAX_ERR_MESSAGE_LEN] = {};

    va_list args = {};
    va_start(args, fmt);

    vsnprintf(message, sizeof(message), fmt, args);

    va_end(args);

    fcprintf(stderr, RED, "ERROR from %s at %s:%d\n\tSyntax error: %s\n"
                          "  %d   | ",
                           func, file, line, message, lang_ctx->current_line);

    char* cur_pos = lang_ctx->code;

    while (*cur_pos != '\n' && *cur_pos != '\0')
    {
        fcprintf(stderr, RED, "%c", *cur_pos);
        cur_pos++;
    }

    fcprintf(stderr, RED, "\n"
                          "      |\n"
                          "      |\n");

    fcprintf(stderr, BLUE, "code:\n");

    for (int i = 0; i < lang_ctx->code - lang_ctx->buffer; i += 2)
        fcprintf(stderr, GRAY, "%c", lang_ctx->buffer[i]);

    fcprintf(stderr, RED, "%c", *lang_ctx->code);

    fcprintf(stderr, BLUE, "%s\n", lang_ctx->code + 1);
}

//------------------------------------------------------------------------------------------

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

static LangErr_t ProcessOperatorTokenCase   (LangCtx_t* lang_ctx, bool* do_continue);
static LangErr_t ProcessNumberTokenCase     (LangCtx_t* lang_ctx, bool* do_continue);
static LangErr_t ProcessIdentifierTokenCase (LangCtx_t* lang_ctx, bool* do_continue);
static void      ProcessSpacesCase          (LangCtx_t* lang_ctx, bool* do_continue);

//——————————————————————————————————————————————————————————————————————————————————————————

static LangErr_t ParseToken(LangCtx_t* lang_ctx)
{
    assert(lang_ctx);

    LangErr_t status = LANG_SUCCESS;
    bool do_continue = false;

    ProcessSpacesCase(lang_ctx, &do_continue);

    if (do_continue)
        return status;

    status = ProcessOperatorTokenCase(lang_ctx, &do_continue);

    if (status != LANG_SUCCESS || do_continue)
        return status;

    status = ProcessNumberTokenCase(lang_ctx, &do_continue);

    if (status != LANG_SUCCESS || do_continue)
        return status;

    status = ProcessIdentifierTokenCase(lang_ctx, &do_continue);

    if (status != LANG_SUCCESS || do_continue)
        return status;

    // TODO: SyntaxError(); with lines and |
                                        // |
                                        // |

    SYNTAX_ERROR(lang_ctx, "Unknown symbol");

    return LANG_SYNTAX_ERROR;
}

//——————————————————————————————————————————————————————————————————————————————————————————

#define PUSH_TOKEN_(lang_ctx, __token)                                        \
        TreeNode_t* _token = __token;                                         \
        DPRINTF("Dumping token %p\n", (_token));                              \
        TreeGraphDumpSubtree(lang_ctx, (_token), DUMP_FULL);                  \
        DPRINTF("Pushing token %p\n", (_token));                              \
        if (StackPush(&lang_ctx->tokens, (_token)))                           \
        {                                                                     \
            PRINTERR("Failed stack push token");                              \
            StackDump(&lang_ctx->tokens, STACK_SUCCESS,  "Push token fail");  \
            return LANG_STACK_ERROR;                                          \
        }                                                                     \
        StackDump(&lang_ctx->tokens, STACK_SUCCESS,  "Pushed token");

//------------------------------------------------------------------------------------------

static LangErr_t ProcessOperatorTokenCase(LangCtx_t* lang_ctx, bool* do_continue)
{
    assert(do_continue);
    assert(lang_ctx);

    for (size_t op_code = 1; op_code < OPERATORS_COUNT; op_code++)
    {
//         DPRINTF("code = %s;\n"
//                 "op_name = %s;\n"
//                 "op_len = %zu;\n"
//                 "------------------------------\n",
//                 lang_ctx->code,
//                 OP_CASES_TABLE[op_code].name,
//                 OP_CASES_TABLE[op_code].name_len);

        if (strncmp(lang_ctx->code,
                    OP_CASES_TABLE[op_code].name,
                    OP_CASES_TABLE[op_code].name_len) == 0)
        {
            *do_continue = true;

            PUSH_TOKEN_(lang_ctx, OPERATOR_((Operator_t) op_code));

            lang_ctx->code += OP_CASES_TABLE[op_code].name_len;

            return LANG_SUCCESS;
        }
    }

    return LANG_SUCCESS;
}

//------------------------------------------------------------------------------------------

static LangErr_t ProcessNumberTokenCase(LangCtx_t* lang_ctx, bool* do_continue)
{
    assert(do_continue);
    assert(lang_ctx);

    if (!isdigit(*lang_ctx->code))
        return LANG_SUCCESS;

    *do_continue = true;

    double value        = 0.0;
    char*  num_code_end = NULL;

    value = strtod(lang_ctx->code, &num_code_end);

    lang_ctx->code = num_code_end;

    PUSH_TOKEN_(lang_ctx, NUMBER_(value));

    return LANG_SUCCESS;
}

//——————————————————————————————————————————————————————————————————————————————————————————

static inline int IsAcceptableFirstSymbol(char ch);
static inline int IsAcceptableSymbol     (char ch);

//——————————————————————————————————————————————————————————————————————————————————————————

static LangErr_t ProcessIdentifierTokenCase(LangCtx_t* lang_ctx, bool* do_continue)
{
    assert(do_continue);
    assert(lang_ctx);

    if (!IsAcceptableFirstSymbol(*lang_ctx->code))
        return LANG_SUCCESS;

    *do_continue = true;

    char buf[MAX_OPERATOR_NAME_LEN] = "";

    size_t i = 0;

    do
    {
        buf[i++] = *(lang_ctx->code++);
    }
    while (IsAcceptableSymbol(*lang_ctx->code));

    size_t id_index = 0;

    LangErr_t status = LANG_SUCCESS;

    if ((status = LangIdTablePush(lang_ctx, buf, &id_index)))
        return status;

    PUSH_TOKEN_(lang_ctx, IDENTIFIER_(id_index));

    return LANG_SUCCESS;
}

//------------------------------------------------------------------------------------------

#undef PUSH_TOKEN_

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

//------------------------------------------------------------------------------------------

static void ProcessSpacesCase(LangCtx_t* lang_ctx, bool* do_continue)
{
    assert(do_continue);
    assert(lang_ctx);

    if (!isspace(*lang_ctx->code))
        return;

    *do_continue = true;

    do
    {
        if (*lang_ctx->code == '\n')
        {
            lang_ctx->current_line++;
        }
        lang_ctx->code++;
    }
    while (isspace(*lang_ctx->code));
}

//——————————————————————————————————————————————————————————————————————————————————————————

#define _DSL_UNDEF_
#include "dsl.h"

//——————————————————————————————————————————————————————————————————————————————————————————
