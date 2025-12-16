#include "AST_write.h"

//——————————————————————————————————————————————————————————————————————————————————————————

static void ASTWriteNode(LangCtx_t* lang_ctx, const TreeNode_t* node, FILE* fp, int rank);

//------------------------------------------------------------------------------------------

LangErr_t ASTWriteData(LangCtx_t* lang_ctx)
{
    assert(lang_ctx);

    char data_file_path[MAX_FILE_NAME_LEN] = "";

    WDPRINTF(L"ast filename = %s;", lang_ctx->ast_file_name);

    snprintf(data_file_path, sizeof(data_file_path), "AST/%s.txt", lang_ctx->ast_file_name);

    FILE* fp = fopen(data_file_path, "w");

    if (fp == NULL)
    {
        WPRINTERR("Error with opening file: %s", data_file_path);
        return LANG_FILE_ERROR;
    }

    ASTWriteNode(lang_ctx, lang_ctx->tree.dummy->right, fp, 0);

    fclose(fp);

    WDPRINTF(L"База данных записана в файл: %s\n", data_file_path);

    return LANG_SUCCESS;
}

//------------------------------------------------------------------------------------------

static void ASTWriteNodeData(LangCtx_t* lang_ctx, const TokenData_t* data, FILE* fp);

//——————————————————————————————————————————————————————————————————————————————————————————

static void ASTWriteNode(LangCtx_t* lang_ctx, const TreeNode_t* node, FILE* fp, int rank)
{
    assert(fp != NULL);

    if (node == NULL)
    {
        fwprintf(fp, L" nil");
        return;
    }

    for (int i = 0; i < rank; i++)
        fwprintf(fp, L"\t");

    fwprintf(fp, L"( ");
    ASTWriteNodeData(lang_ctx, &node->data, fp);

    if (node->left != NULL)
        fwprintf(fp, L"\n");

    ASTWriteNode(lang_ctx, node->left , fp, rank + 1);
    ASTWriteNode(lang_ctx, node->right, fp, rank + 1);

    if (node->left != NULL)
    {
        for (int i = 0; i < rank; i++)
            fwprintf(fp, L"\t");
    }
    else
        fwprintf(fp, L" ");

    fwprintf(fp, L")\n");
}

//------------------------------------------------------------------------------------------

static void ASTWriteNodeData(LangCtx_t* lang_ctx, const TokenData_t* data, FILE* fp)
{
    assert(data);
    assert(fp);

    switch (data->type)
    {
        case TYPE_OP:
            fwprintf(fp, L"\"%ls\"", OP_CASES_TABLE[data->value.opcode].ast_format);
            break;

        case TYPE_ID:
            fwprintf(fp, L"\"%ls\"", lang_ctx->id_table.data[data->value.id_index]);
            break;

        case TYPE_NUM:
            fwprintf(fp, L"\"%lg\"", data->value.number);
            break;

        default:
            return;
    }
}

//------------------------------------------------------------------------------------------
