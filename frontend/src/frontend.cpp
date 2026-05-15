#include "frontend.h"

//FIXME - stack realloc дропается дамп, скорее всего реаллок не работает

//==========================================================================================

void FrontendSetError(FrontendCtx_t*   frontend_ctx,
                      LangErrorInfo_t* error_info,
                      const wchar_t*   message,
                      ...)
{
    assert(frontend_ctx);

    va_list message_args = {};

    va_start(message_args, message);

    wchar_t buffer[MAX_BUFFER_SIZE] = {};

    vswprintf(buffer, sizeof(buffer) / sizeof(buffer[0]), message, message_args);

    frontend_ctx->error_info = *error_info;

    wcsncpy(frontend_ctx->error_info.message, buffer,
            sizeof(frontend_ctx->error_info.message) / 
            sizeof(frontend_ctx->error_info.message[0]));

    va_end(message_args);
}

//==========================================================================================

static void FrontendPrintSyntaxError(FrontendCtx_t* frontend_ctx);

//------------------------------------------------------------------------------------------

void FrontendPrintError(FrontendCtx_t* frontend_ctx)
{
    assert(frontend_ctx);

    wprintf(L"%s", RED);

    LangErrorInfo_t error_info = frontend_ctx->error_info;

    LangErr_t error = error_info.error;

    wprintf(L"---------------------------------------------"
            L"---------------------------------------------\n\n"
            L"ERROR: %d in function %s at %s:%d\n\n",
            error, error_info.func, error_info.file, error_info.line);

    if (error == LANG_PARSER_SYNTAX_ERROR ||
        error == LANG_LEXER_SYNTAX_ERROR)
    {
        FrontendPrintSyntaxError(frontend_ctx);
    }

    wprintf(L"---------------------------------------------"
            L"---------------------------------------------\n");

    wprintf(L"%s", RESET_COLOR);
}

//==========================================================================================

static void FrontendPrintErrorCodePart(FrontendCtx_t* frontend_ctx, TreeNode_t* node);

//------------------------------------------------------------------------------------------

static void FrontendPrintSyntaxError(FrontendCtx_t* frontend_ctx)
{
    assert(frontend_ctx);

    LangErrorInfo_t error_info = frontend_ctx->error_info;

    if (error_info.error == LANG_PARSER_SYNTAX_ERROR)
        wprintf(L"PARSER ERROR: \n\n");

    if (error_info.error == LANG_LEXER_SYNTAX_ERROR)
        wprintf(L"LEXER ERROR: \n\n");

    wprintf(L"expected:\n"
            L"\t%ls\n", error_info.message);

    wprintf(L"got:\n");

    FrontendPrintNode(frontend_ctx, error_info.node);

    wprintf(L"\n");
    wprintf(L"syntax error\n\n");

    FrontendPrintErrorCodePart(frontend_ctx, error_info.node);
}

//==========================================================================================

static wchar_t* FindLineStart(wchar_t* buffer_start, wchar_t* buffer_end, size_t line);

//------------------------------------------------------------------------------------------

static void FrontendPrintErrorCodePart(FrontendCtx_t* frontend_ctx, TreeNode_t* node)
{
    assert(frontend_ctx);
    assert(node);

    if (frontend_ctx->buffer == NULL)
    {
        wprintf(L"\terror: can not print: no buffer given\n");
        return;
    }

    wchar_t* buffer_start = frontend_ctx->buffer;
    wchar_t* buffer_end   = buffer_start + frontend_ctx->buffer_size;

    if (node->buf_pos == NULL || node->buf_pos > buffer_end)
    {
        wprintf(L"error: node->cur_pos was not set\n");
        return;
    }

    wchar_t* cur_pos = FindLineStart(buffer_start, buffer_end, node->line);

    if (cur_pos == NULL)
        return;

    wprintf(L"%s", RESET_COLOR);

    wprintf(L"%-5d | ");

    while (cur_pos < node->buf_pos)
    {
        putwc(*cur_pos, stdout);
        cur_pos++;
    }

    wchar_t* line_end = wcschr(node->buf_pos, L'\n');

    if (line_end != NULL)
        *line_end = L'\0';

    wprintf(L"%s", RED);

    wprintf(L"%-5d | %ls"
            L"      |\n"
            L"      |\n",
            node->line,
            node->buf_pos);
}

//==========================================================================================

static wchar_t* FindLineStart(wchar_t* buffer_start, wchar_t* buffer_end, size_t line)
{
    assert(buffer_start);
    assert(buffer_end);

    wchar_t* cur_pos = buffer_start;

    size_t enters_count = 0;

    while (enters_count + 1 < line)
    {
        cur_pos = wcschr(cur_pos, L'\n');

        if (cur_pos == NULL || cur_pos >= buffer_end)
        {
            wprintf(L"\terror: cur_line is outside of buffer\n");
            return NULL;
        }

        enters_count++;
    }

    return cur_pos;
}

//==========================================================================================

void FrontendPrintNode(FrontendCtx_t* frontend_ctx, TreeNode_t* node)
{
    assert(frontend_ctx);

    if (node == NULL)
    {
        wprintf(L"node = NULL\n");
    }

    wprintf(
LR"(    node [%p]:
        left  [%p]
        right [%p]
        type = %ls;
        value = )",
        node,
        node->left,
        node->right,
        TYPE_CASES_TABLE[node->data.type].name);

    switch (node->data.type)
    {
        case TYPE_NUM:
            wprintf(LANG_NUM_SPEC, node->data.value.number);
            break;

        case TYPE_KEYWORD:
            wprintf(L"%ls", KEYWORD_CASES_TABLE[node->data.value.keyword].name);
            break;

        case TYPE_ID:
        case TYPE_VAR:
        case TYPE_VAR_DECL:
        case TYPE_FUNC_DECL:
        case TYPE_FUNC_CALL:
            wprintf(L"%ls", 
                    frontend_ctx->lang_ctx.names_pool.data[node->data.value.id.name_index]);
            break;

        default:
            wprintf(L"UNKNOWN");
    }

    wprintf(L";\n");
}

//==========================================================================================

LangErr_t FrontendCtxCtor(FrontendCtx_t* frontend_ctx)
{
    assert(frontend_ctx);

    LangErr_t error = LANG_SUCCESS;

    if (StackCtor(&frontend_ctx->tokens, TOKENS_STACK_INIT_CAPACITY))
    {
        WPRINTERR(L"Tokens stack construct failed");
        return LANG_STACK_ERROR;
    }

    frontend_ctx->cur_symbol_ptr = NULL;
    frontend_ctx->current_line   = 1;

    if ((error = LangCtxCtor(&frontend_ctx->lang_ctx)))
    {
        return error;
    }

    return LANG_SUCCESS;
}

//==========================================================================================

void FrontendCtxDtor(FrontendCtx_t* frontend_ctx)
{
    assert(frontend_ctx);

    //------------------------------------------------------------------//
    /* Tree could have been not fully created, because of that
        we will free all the tokens in stack, and the tree dummy 
    */
    //------------------------------------------------------------------//

    TreeSingleNodeDtor(frontend_ctx->lang_ctx.tree.dummy, 
                       &frontend_ctx->lang_ctx.tree);

    for (size_t i = 0; i < frontend_ctx->tokens.size; i++)
    {
        WDPRINTF(L"freed token = %p\n", frontend_ctx->tokens.data[i]);
        
        free(frontend_ctx->tokens.data[i]);
    }

    StackDtor(&frontend_ctx->tokens);

    //------------------------------------------------------------------//

    LangCtxDtor(&frontend_ctx->lang_ctx);

    if (frontend_ctx->output_file != NULL)
    {
        fclose(frontend_ctx->output_file);
    }

    WDPRINTF(L"Freeing buffer %p\n", frontend_ctx->buffer);
    free(frontend_ctx->buffer);

    frontend_ctx->cur_symbol_ptr = NULL;
    frontend_ctx->buffer         = NULL;
}

//==========================================================================================

TreeNode_t* FrontendGetCurrentToken(FrontendCtx_t* frontend_ctx)
{
    assert(frontend_ctx);

    TreeNode_t* token = NULL;

    if (StackGetElement(&frontend_ctx->tokens, frontend_ctx->cur_token_index, &token))
    {
        PRINTERR("Error with getting token from stack");
        return NULL;
    }

    return token;
}

//==========================================================================================

static void FrontendShiftNodeIdIndexesForInFuncIdentifiers(LangCtx_t*  lang_ctx,
                                                           TreeNode_t* node, 
                                                           size_t      indexes_shift,
                                                           int         in_func)
{
    assert(node);

    if (node->data.type == TYPE_FUNC_DECL)
    {
        in_func = 1;
    }

    switch (node->data.type)
    {
        case TYPE_VAR:
        case TYPE_VAR_DECL:
            if (!in_func)
            {
                break;
            }
        // else fallthrough
        case TYPE_FUNC_CALL:
        case TYPE_FUNC_DECL:
            WDPRINTF(L"changed id %ls from %zu to %zu\n", 
                     LangGetIdName(&lang_ctx->names_pool, node->data.value.id.name_index),
                     node->data.value.id.id_index,
                     node->data.value.id.id_index + indexes_shift);
            node->data.value.id.id_index += indexes_shift;
            break;

        case TYPE_ID:
        case TYPE_KEYWORD:
        case TYPE_NUM:
        default:
            break;
    }
    
    if (node->left)
    {
        FrontendShiftNodeIdIndexesForInFuncIdentifiers(lang_ctx, node->left , indexes_shift, in_func);
    }
    if (node->right)
    {
        FrontendShiftNodeIdIndexesForInFuncIdentifiers(lang_ctx, node->right, indexes_shift, in_func);   
    }
}

//==========================================================================================

LangErr_t FrontendRecountIdentifiersIndexes(LangCtx_t* lang_ctx)
{
    assert(lang_ctx);

    LangErr_t error = LANG_SUCCESS;

    size_t n_main_params = 0;
    size_t n_main_vars   = 0;

    LangIdTableCountVarsAndParams(&lang_ctx->main_id_table, &n_main_vars, &n_main_params);

    if (n_main_params != 0)
    {
        WPRINTERR(L"IdTable error! Main can not have params");
        return LANG_WRONG_ID_TABLE;
    }

    lang_ctx->main_id_table.data[0].n_local_vars = n_main_vars;

    WDPRINTF(L"n_main_vars = %zu\n\n", n_main_vars);

    FrontendShiftNodeIdIndexesForInFuncIdentifiers(lang_ctx,
                                                   lang_ctx->tree.dummy->right,
                                                   n_main_vars + 1,
                                                   0);

    GRAPH_DUMP_(lang_ctx, lang_ctx->tree.dummy->right, DUMP_SHORT, L"after recounting ids");

    return LANG_SUCCESS;
}

//==========================================================================================
