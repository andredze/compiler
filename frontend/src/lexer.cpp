#include "lexer.h"
#include "lang_ctx.h"
#include "lang_funcs.h"
#include <ctype.h>
#include <wchar.h>
#include <wctype.h>
// #include <stddef.h>
#include <assert.h>

//——————————————————————————————————————————————————————————————————————————————————————————

#define _DSL_DEFINE_
#include "dsl.h"

//——————————————————————————————————————————————————————————————————————————————————————————

static inline int IsAcceptableFirstSymbol(wchar_t ch);
static inline int IsAcceptableSymbol     (wchar_t ch);

static void PrintSyntaxError(LangCtx_t* lang_ctx, const char* file, const char* func,
                             const int  line,     const char* fmt, ...);

//------------------------------------------------------------------------------------------

#define SYNTAX_ERROR(lang_ctx, fmt, ...)                                                                \
        {   TreeDumpInfo_t dump_info = {TREE_SUCCESS, __PRETTY_FUNCTION__, __FILE__, __LINE__};         \
            PrintSyntaxError(lang_ctx, __FILE__, __PRETTY_FUNCTION__, __LINE__, (fmt), ##__VA_ARGS__);  \
            TreeDump((lang_ctx), &dump_info, DUMP_SHORT, (fmt), ##__VA_ARGS__);                         \
            return LANG_SYNTAX_ERROR; }

//——————————————————————————————————————————————————————————————————————————————————————————

static LangErr_t ParseToken(LangCtx_t* lang_ctx);

//——————————————————————————————————————————————————————————————————————————————————————————

    // TODO: SyntaxError(); with lines and |
                                        // |
                                        // |
    // TODO: mystrncmp позволяющий любой регистр

static void PrintSyntaxError(LangCtx_t* lang_ctx, const char* file, const char* func,
                             const int  line,     const char* fmt, ...)
{
    char message[MAX_SYNTAX_ERR_MESSAGE_LEN] = {};

    va_list args = {};
    va_start(args, fmt);

    vsnprintf(message, sizeof(message), fmt, args);

    va_end(args);

    fwprintf(stderr, L"ERROR from %s at %s:%d\n\tSyntax error: %s\n"
                     L"  %d   | ",
                     func, file, line, message, lang_ctx->current_line);

    wchar_t* cur_pos = lang_ctx->code;

    wprintf(L"code = %ls\n", lang_ctx->code);

    while (*cur_pos != '\n' && *cur_pos != '\0')
    {
        fwprintf(stderr, L"%lc", *cur_pos);
        cur_pos++;
    }

    fwprintf(stderr, L"\n"
                     L"      |\n"
                     L"      |\n");

    fwprintf(stderr, L"code:\n");

    for (int i = 0; i < lang_ctx->code - lang_ctx->buffer; i++)
    {
        fwprintf(stderr, L"%lc", lang_ctx->buffer[i]);
        wfcprintf(stderr, GRAY, L"%lc", lang_ctx->buffer[i]);
    }

    wfcprintf(stderr, RED, L"%lc", *lang_ctx->code);

    wfcprintf(stderr, BLUE, L"%ls\n", lang_ctx->code + 1);
}

//------------------------------------------------------------------------------------------

LangErr_t LexicallyAnalyze(LangCtx_t* lang_ctx)
{
    assert(lang_ctx       != NULL);
    assert(lang_ctx->code != NULL);

    LangErr_t status = LANG_SUCCESS;

    WDPRINTF(L"LEXER: code = %ls\n", lang_ctx->code);

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

    SYNTAX_ERROR(lang_ctx, "Unknown symbol");
}

//——————————————————————————————————————————————————————————————————————————————————————————

#define PUSH_TOKEN_(lang_ctx, __token)                                        \
        TreeNode_t* _token = __token;                                         \
        TreeGraphDumpSubtree(lang_ctx, (_token), DUMP_FULL);                  \
        WDPRINTF(L"Pushing token %p\n", (_token));                            \
        if (StackPush(&lang_ctx->tokens, (_token)))                           \
        {                                                                     \
            WPRINTERR("Failed stack push token");                             \
            StackDump(&lang_ctx->tokens, STACK_SUCCESS, "Push token fail");  \
            return LANG_STACK_ERROR;                                          \
        }                                                                     \
        StackDump(&lang_ctx->tokens, STACK_SUCCESS, "Pushed token");

//——————————————————————————————————————————————————————————————————————————————————————————

static LangErr_t ProcessOperatorTokenRepetitions(LangCtx_t* lang_ctx, size_t op_code);

//——————————————————————————————————————————————————————————————————————————————————————————

static LangErr_t ProcessOperatorTokenCase(LangCtx_t* lang_ctx, bool* do_continue)
{
    assert(do_continue);
    assert(lang_ctx);

    LangErr_t status = LANG_SUCCESS;

    for (size_t op_code = 1; op_code < OPERATORS_COUNT; op_code++)
    {
        const wchar_t* opname     = OP_CASES_TABLE[op_code].name;
        size_t         opname_len = OP_CASES_TABLE[op_code].name_len;

        // WDPRINTF(L"op_name = %ls;\n"
        //          L"op_len = %zu;\n"
        //          L"------------------------------\n",
        //          opname,
        //          opname_len);

        if (wcsncmp(lang_ctx->code, opname, opname_len) == 0 &&
            !IsAcceptableSymbol(lang_ctx->code[opname_len]))
        {
            // WDPRINTF(L"op_name = %ls;\n"
            //          L"op_len = %zu;\n"
            //          L"------------------------------\n",
            //          opname,
            //          opname_len);

            lang_ctx->code += opname_len;

            if ((status = ProcessOperatorTokenRepetitions(lang_ctx, op_code)))
                return status;

            *do_continue = true;

            PUSH_TOKEN_(lang_ctx, OPERATOR_((Operator_t) op_code));

            return LANG_SUCCESS;
        }
    }

    return LANG_SUCCESS;
}

//------------------------------------------------------------------------------------------

static LangErr_t ProcessOperatorTokenRepetitions(LangCtx_t* lang_ctx, size_t op_code)
{
    assert(lang_ctx);

    const wchar_t* opname          = OP_CASES_TABLE[op_code].name;
    size_t         opname_len      = OP_CASES_TABLE[op_code].name_len;
    int            op_repeat_times = OP_CASES_TABLE[op_code].repeat_times;

    for (int i = 1; i < op_repeat_times; i++)
    {
        ProcessSpacesCase(lang_ctx, NULL);

        if (!(wcsncmp(lang_ctx->code, opname, opname_len) == 0 &&
              !IsAcceptableSymbol(lang_ctx->code[opname_len])))
        {
            SYNTAX_ERROR(lang_ctx, "operator should repeat %d times", op_repeat_times);
        }

        lang_ctx->code += opname_len;
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
    wchar_t*  num_code_end = NULL;

    value = wcstod(lang_ctx->code, &num_code_end);

    lang_ctx->code = num_code_end;

    PUSH_TOKEN_(lang_ctx, NUMBER_(value));

    return LANG_SUCCESS;
}

//------------------------------------------------------------------------------------------

static LangErr_t ProcessIdentifierTokenCase(LangCtx_t* lang_ctx, bool* do_continue)
{
    assert(do_continue);
    assert(lang_ctx);

    if (!IsAcceptableFirstSymbol(*lang_ctx->code))
        return LANG_SUCCESS;

    *do_continue = true;

    // TODO: сделать указатель на буффер code вместо strdup
    wchar_t buf[MAX_OPERATOR_NAME_LEN] = L"";

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

static inline int IsAcceptableSymbol(wchar_t ch)
{
    return iswalnum(ch) || ch == '_';
}

//------------------------------------------------------------------------------------------

static inline int IsAcceptableFirstSymbol(wchar_t ch)
{
    return iswalpha(ch) || ch == '_';
}

//------------------------------------------------------------------------------------------

static void ProcessSpacesCase(LangCtx_t* lang_ctx, bool* do_continue)
{
    assert(lang_ctx);

    if (!iswspace(*lang_ctx->code))
        return;

    if (do_continue != NULL)
        *do_continue = true;

    do
    {
        if (*lang_ctx->code == '\n')
        {
            lang_ctx->current_line++;
        }
        lang_ctx->code++;
    }
    while (iswspace(*lang_ctx->code));
}

//——————————————————————————————————————————————————————————————————————————————————————————

#define _DSL_UNDEF_
#include "dsl.h"

//——————————————————————————————————————————————————————————————————————————————————————————
