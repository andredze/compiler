#include "AST_read.h"

//------------------------------------------------------------------------------------------

static void SkipSpaces(wchar_t* buffer, ssize_t* pos);
static int  SkipLetter(wchar_t* buffer, ssize_t* pos, wchar_t letter);

//——————————————————————————————————————————————————————————————————————————————————————————

LangErr_t ASTReadData(LangCtx_t* lang_ctx, char* ast_file_path)
{
    assert(lang_ctx);

    Tree_t* tree = &lang_ctx->tree;

    FILE* fp = fopen(ast_file_path, "r");

    if (fp == NULL)
    {
        WPRINTERR("Error with opening file: %s", ast_file_path);
        return LANG_FILE_ERROR;
    }

    wchar_t* buffer = NULL;

    if (ReadFile(fp, &buffer, ast_file_path))
    {
        WPRINTERR("Error with reading file");
        free(buffer);
        return LANG_FILE_ERROR;
    }

    WDPRINTF(L"READ BUFFER: %ls\n\n"
             L"---------------------------------------------------",
             buffer);

    strcpy(lang_ctx->ast_file_name, GetFileName(ast_file_path));

    ssize_t i = 0;

    if (ReadNode(lang_ctx, &tree->dummy->right, buffer, &i))
    {
        free(buffer);
        return LANG_TREE_ERROR;
    }

    free(buffer);

    TREE_CALL_DUMP(lang_ctx, "DUMP AFTER TREE READ DATA %s", ast_file_path);

    return LANG_SUCCESS;
}

//------------------------------------------------------------------------------------------

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

        TREE_CALL_DUMP(lang_ctx, "DUMP AFTER NODE CTOR in READ AST");

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

//------------------------------------------------------------------------------------------

static LangErr_t GetNodeData(LangCtx_t* lang_ctx, TokenData_t* node_data, wchar_t* word)
{
    assert(node_data);
    assert(lang_ctx);
    assert(word);

    for (size_t opcode = 0; opcode < OPERATORS_COUNT; opcode++)
    {
        const wchar_t* opname = OP_CASES_TABLE[opcode].ast_format;

        if (wcscmp(word, opname) == 0)
        {
            node_data->type         = TYPE_OP;
            node_data->value.opcode = (Operator_t) opcode;

            return LANG_SUCCESS;
        }
    }

    if (iswdigit(word[0]))
    {
        double value = wcstod(word, NULL);

        node_data->type         = TYPE_NUM;
        node_data->value.number = value;

        return LANG_SUCCESS;
    }

    size_t id_index = 0;

    LangErr_t error = LANG_SUCCESS;

    if ((error = LangIdTablePush(lang_ctx, word, &id_index)))
        return error;

    node_data->type           = TYPE_ID;
    node_data->value.id_index = id_index;

    return LANG_SUCCESS;
}

//------------------------------------------------------------------------------------------

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

//------------------------------------------------------------------------------------------

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

//------------------------------------------------------------------------------------------
