#include "AST_read.h"
#include "tree_dump.h"
#include "lang_funcs.h"

//==========================================================================================

static void SkipSpaces(wchar_t* buffer, ssize_t* pos);
static int  SkipLetter(wchar_t* buffer, ssize_t* pos, wchar_t letter);
static void ReadIdTable(LangCtx_t* lang_ctx, wchar_t* buffer, ssize_t* pos);
static LangErr_t ReadGlobalsCount(LangCtx_t* lang_ctx, wchar_t* buffer, ssize_t* pos);

//——————————————————————————————————————————————————————————————————————————————————————————

TreeErr_t TreeReadBufferDump(LangCtx_t* lang_ctx, 
                             const char* cur_symbol_ptr,
                             const char* buffer,
                             const char* fmt, ...)
{
    assert(fmt != NULL);

    int pos = (int) (cur_symbol_ptr - buffer);

    va_list args = {};
    va_start(args, fmt);

    FILE* fp = lang_ctx->tree.debug.fp;

    fwprintf(fp, L"<pre><h4><font color=green>");

    // vfprintf(fp, fmt, args);

    fwprintf(fp, L"</h4></font>\n"
                L"<font color=gray>");

    fwprintf(fp, L"\"");

    for (int i = 0; i < pos; i++)
    {
        fwprintf(fp, L"%lc", buffer[i]);
    }

    fwprintf(fp, L"</font><font color=red>%c</font>", buffer[pos]);

    if (*(buffer + pos) != '\0')
    {
        fwprintf(fp, L"<font color=blue>%ls</font>\n\n", buffer + pos + 1);
    }

    fwprintf(fp, L"\"");

    va_end(args);

    fflush(fp);

    return TREE_SUCCESS;
}

//==========================================================================================

LangErr_t ASTReadData(LangCtx_t* lang_ctx, 
                      char*      ast_file_path, 
                      char       src_file_name[MAX_FILENAME_LEN])
{
    DPRINT_FUNC_ENTER_MSG();
    assert(lang_ctx);

    Tree_t* tree = &lang_ctx->tree;

    FILE* fp = fopen(ast_file_path, "r");

    if (fp == NULL)
    {
        WPRINTERR(L"Error with opening file: %s", ast_file_path);
        return LANG_FILE_ERROR;
    }

    wchar_t* buffer   = NULL;
    size_t   buf_size = 0;

    if (ReadFile(fp, &buffer, ast_file_path, &buf_size))
    {
        WPRINTERR("Error with reading file");
        free(buffer);
        return LANG_FILE_ERROR;
    }

    WDPRINTF(L"READ BUFFER: %ls\n\n"
             L"---------------------------------------------------",
             buffer);

    strncpy(src_file_name, GetFileName(ast_file_path), MAX_FILENAME_LEN);

    ssize_t i = 0;

    if (ReadNode(lang_ctx, &tree->dummy->right, buffer, &i))
    {
        free(buffer);
        return LANG_TREE_ERROR;
    }

    ReadIdTable(lang_ctx, buffer, &i);

    free(buffer);

    #ifdef TREE_DEBUG
        LangIdTableDump(lang_ctx, &lang_ctx->func_id_table, L"read id_table dump");
    #endif /* TREE_DEBUG */

    TREE_CALL_DUMP(lang_ctx, NULL, "DUMP AFTER TREE READ DATA %s", ast_file_path);

    DPRINT_FUNC_LEAVE_MSG();
    return LANG_SUCCESS;
}

//——————————————————————————————————————————————————————————————————————————————————————————

static bool ReadIdData(LangCtx_t* lang_ctx, wchar_t* buffer, ssize_t* pos);

//——————————————————————————————————————————————————————————————————————————————————————————

static void ReadIdTable(LangCtx_t* lang_ctx, wchar_t* buffer, ssize_t* pos)
{
    DPRINT_FUNC_ENTER_MSG();
    assert(lang_ctx);
    assert(buffer);
    assert(pos);

    WDPRINTF(L"Reading Id Table\n");

    bool got_data = false;

    do
    {
        got_data = ReadIdData(lang_ctx, buffer, pos);
    }
    while (got_data);

    DPRINT_FUNC_LEAVE_MSG();
}

//==========================================================================================

static bool ReadIdData(LangCtx_t* lang_ctx, wchar_t* buffer, ssize_t* pos)
{
    DPRINT_FUNC_ENTER_MSG();
    assert(lang_ctx);
    assert(buffer);
    assert(pos);

    IdData_t id_data = {};
    int symbols_read = 0;

    wchar_t id_buff[MAX_IDENTIFIER_LEN] = {};
    int     ret_value = 0;
    int     type      = 0;

    // name_index, name, id_type, n_local_vars, n_params, addr
    if ((ret_value = swscanf(&buffer[*pos], L"[%zu, \"%l[^\"]\", %d, %zu, %zu, %d]\n%n",
                             &id_data.name_index,
                             id_buff,
                             &type,
                             &id_data.n_local_vars,
                             &id_data.n_params,
                             &id_data.addr,
                             &symbols_read)) != 6)
    {
        WDPRINTF(L"ret_value = %d\n", ret_value);
        WDPRINTF(L"!!! RETURNING FALSE !!!\n", ret_value);
        return false;
    }

    id_data.type = (IdType_t) type;

    WDPRINTF(L"id_buff = %ls\n", id_buff);

    if (id_data.name_index != (size_t)-1)
    {
        id_data.name = lang_ctx->names_pool.data[id_data.name_index];
    }
    else
    {
        id_data.name = L"main";
    }

    LangIdTablePush(&lang_ctx->func_id_table, &id_data, NULL);

    *pos = *pos + symbols_read;

    if (buffer[*pos] == '\0')
    {
        return false;
    }

    DPRINT_FUNC_LEAVE_MSG();
    return true;
}

//==========================================================================================

LangErr_t ReadNode(LangCtx_t* lang_ctx, TreeNode_t** node, wchar_t* buffer, ssize_t* pos)
{
    assert(lang_ctx != NULL);
    assert(buffer   != NULL);
    assert(node     != NULL);

    LangErr_t error = LANG_SUCCESS;
    wchar_t first_char = buffer[*pos];

    if (first_char == L'(')
    {
        (*pos)++;
        SkipSpaces (buffer, pos);
        // BUFFER_DUMP(buffer, *pos, "BUFFER DUMP SKIPPING OPENING BRACKET");

        TokenData_t data = {};

        if ((error = ReadNodeData(lang_ctx, buffer, pos, &data)))
            return error;

        *node = TreeNodeCtor(&lang_ctx->tree, data, NULL, NULL, NULL);

        TREE_CALL_DUMP(lang_ctx, NULL, "DUMP AFTER NODE CTOR in READ AST");

        if ((error = ReadNode(lang_ctx, &(*node)->left,  buffer, pos)))
            return error;

        if ((error = ReadNode(lang_ctx, &(*node)->right, buffer, pos)))
            return error;

        if (SkipLetter(buffer, pos, L')'))
            return LANG_INVALID_AST_INPUT;

        SkipSpaces(buffer, pos);
    }
    else if (wcsncmp(&buffer[*pos], L"nil", 3) == 0)
    {
        (*pos) += 3;
        SkipSpaces (buffer, pos);
    }
    else
    {
        WPRINTERR(L"Syntax error in tree data (unknown symbol = \"%lc\" )\n", first_char);
        return LANG_INVALID_AST_INPUT;
    }

    return LANG_SUCCESS;
}

//——————————————————————————————————————————————————————————————————————————————————————————

static LangErr_t GetNodeData(LangCtx_t* lang_ctx, TokenData_t* node_data, wchar_t* word);

//——————————————————————————————————————————————————————————————————————————————————————————

LangErr_t ReadNodeData(LangCtx_t* lang_ctx, wchar_t* buffer, ssize_t* pos, TokenData_t* node_data)
{
    assert(node_data != NULL);
    assert(buffer    != NULL);
    assert(pos       != NULL);

    int data_len = 0;

    if (swscanf(&buffer[*pos], L"\"%*[^\"]\"%n", &data_len) != 0)
    {
        PRINTERR("Error with reading data");
        return LANG_INVALID_AST_INPUT;
    }

    /* node_data points to the start of the word in buffer */
    wchar_t* word = buffer + *pos + 1;

    /* moving pos to the next word */
    (*pos) += data_len;

    /* setting null-terminator for node_data (instead of closing quote) */
    buffer[*pos - 1] = '\0';

    GetNodeData(lang_ctx, node_data, word);

    SkipSpaces(buffer, pos);

    return LANG_SUCCESS;
}

//==========================================================================================

static LangErr_t GetNodeDataKeyword (TokenData_t* node_data, wchar_t* string_data);
static LangErr_t GetNodeDataNum     (TokenData_t* node_data, wchar_t* string_data);
static LangErr_t GetNodeDataId      (LangCtx_t* lang_ctx, TokenData_t* node_data, wchar_t* string_data);

//——————————————————————————————————————————————————————————————————————————————————————————

static LangErr_t GetNodeData(LangCtx_t* lang_ctx, TokenData_t* node_data, wchar_t* word)
{
    assert(node_data);
    assert(lang_ctx);
    assert(word);

    wchar_t type_name  [MAX_BUFFER_LEN] = {};
    wchar_t string_data[MAX_BUFFER_LEN] = {};

    if (swscanf(word, L"%ls %ls", type_name, string_data) != 2)
    {
        WPRINTERR("swscanf in AST read failed");
        return LANG_INVALID_AST_INPUT;
    }

    bool got_type = false;

    for (size_t i = 0; i < TYPES_COUNT; i++)
    {
        if (wcscmp(type_name, TYPE_CASES_TABLE[i].ast_format) == 0)
        {
            node_data->type = TYPE_CASES_TABLE[i].type;
            got_type = true;
            break;
        }
    }

    if (!got_type)
    {
        WPRINTERR(L"unknown token type in AST read");
        return LANG_INVALID_AST_INPUT;
    }

    switch (node_data->type)
    {
        case TYPE_KEYWORD:
            return GetNodeDataKeyword(node_data, string_data);

        case TYPE_NUM:
            return GetNodeDataNum(node_data, string_data);

        case TYPE_ID:
            WPRINTERR("Type identifier should not be in back-end");
            return LANG_INVALID_AST_INPUT;

        case TYPE_VAR:
        case TYPE_VAR_DECL:
        case TYPE_FUNC_DECL:
        case TYPE_FUNC_CALL:
            return GetNodeDataId(lang_ctx, node_data, string_data);

        default:
            return LANG_INVALID_AST_INPUT;
    }

    return LANG_INVALID_AST_INPUT;
}

//==========================================================================================

static LangErr_t GetNodeDataKeyword(TokenData_t* node_data, wchar_t* string_data)
{
    assert(string_data != NULL);
    assert(node_data   != NULL);

    for (size_t keyword = 0; keyword < KEYWORDS_COUNT; keyword++)
    {
        const wchar_t* opname = KEYWORD_CASES_TABLE[keyword].ast_format;

        if (wcscmp(string_data, opname) == 0)
        {
            node_data->type          = TYPE_KEYWORD;
            node_data->value.keyword = (Keyword_t) keyword;

            return LANG_SUCCESS;
        }
    }

    WPRINTERR("Unknown keyword in AST");

    return LANG_INVALID_AST_INPUT;
}

//==========================================================================================

static LangErr_t GetNodeDataNum(TokenData_t* node_data, wchar_t* string_data)
{
    assert(string_data != NULL);
    assert(node_data   != NULL);

    if (string_data[0] != L'-' && !iswdigit(string_data[0]))
    {
        WPRINTERR("Not a number in AST with type number");
        return LANG_INVALID_AST_INPUT;
    }

    double value = wcstod(string_data, NULL);

    node_data->type         = TYPE_NUM;
    node_data->value.number = value;

    return LANG_SUCCESS;
}

//==========================================================================================

static LangErr_t GetNodeDataId(LangCtx_t* lang_ctx, TokenData_t* node_data, wchar_t* string_data)
{
    assert(string_data != NULL);
    assert(node_data   != NULL);

    size_t name_index = 0;
    size_t id_index   = 0;

    wchar_t name[MAX_BUFFER_LEN] = {};

    if (swscanf(string_data, L"%zu_%ls", &id_index, string_data) != 2)
    {
        WPRINTERR("swscanf in AST read NodeDataId failed");
        return LANG_INVALID_AST_INPUT;
    }

    LangErr_t error = LANG_SUCCESS;

    if ((error = LangNamesPoolPush(&lang_ctx->names_pool, string_data, &name_index)))
        return error;

    node_data->value.id.name_index = name_index;
    node_data->value.id.id_index   = id_index;

    return LANG_SUCCESS;
}

//==========================================================================================

static void SkipSpaces(wchar_t* buffer, ssize_t* pos)
{
    assert(buffer != NULL);
    assert(pos    != NULL);

    wchar_t ch = '\0';

    while ((ch = buffer[*pos]) != '\0' && iswspace(ch))
    {
        (*pos)++;
    }
}

//==========================================================================================

static int SkipLetter(wchar_t* buffer, ssize_t* pos, wchar_t letter)
{
    assert(buffer != NULL);
    assert(pos    != NULL);

    wchar_t current_char = buffer[*pos];

    if (current_char != letter)
    {
        WPRINTERR(L"Syntax error: expected %lc, got %lc (%d)", letter, current_char, current_char);
        return 1;
    }
    (*pos)++;

    return 0;
}

//==========================================================================================
