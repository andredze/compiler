#include "op_cases.h"

//——————————————————————————————————————————————————————————————————————————————————————————

#define _DSL_DEFINE_
#include "dsl.h"

//——————————————————————————————————————————————————————————————————————————————————————————

static LangErr_t AssembleIdentifier(LangCtx_t* lang_ctx, TreeNode_t* node);
static LangErr_t AssembleNumber    (LangCtx_t* lang_ctx, TreeNode_t* node);

//——————————————————————————————————————————————————————————————————————————————————————————

LangErr_t AssembleNode(LangCtx_t* lang_ctx, TreeNode_t* node)
{
    assert(lang_ctx);
    assert(node);

    switch (node->data.type)
    {
        case TYPE_NUM:
            return AssembleNumber(lang_ctx, node);

        case TYPE_ID:
            return AssembleIdentifier(lang_ctx, node);

        case TYPE_OP:
            return OP_CASES_TABLE[node->data.value.opcode].asm_function (lang_ctx, node);

        default:
            return LANG_UNKNOWN_TOKEN_TYPE;
    }

    return LANG_SUCCESS;
}

//------------------------------------------------------------------------------------------

static LangErr_t AssembleNumber(LangCtx_t* lang_ctx, TreeNode_t* node)
{
    assert(node);

    ASM_VERIFY_(IS_NUMBER_(node));
    ASM_VERIFY_(node->left  == NULL);
    ASM_VERIFY_(node->right == NULL);

    ASM_PRINT_(L"PUSH %lg\n", node->data.value.number);

    return LANG_SUCCESS;
}

//------------------------------------------------------------------------------------------

static LangErr_t AssembleIdentifier(LangCtx_t* lang_ctx, TreeNode_t* node)
{
    assert(lang_ctx);
    assert(node);

    ASM_VERIFY_(IS_IDENTIFIER_(node));
    ASM_VERIFY_(node->left  == NULL);
    ASM_VERIFY_(node->right == NULL);

    ASM_PRINT_(L"PUSH %zu\n", node->data.value.id_index);
    ASM_PRINT_(L"POPR RAX\n"  );
    ASM_PRINT_(L"PUSHM [RAX]\n");

    return LANG_SUCCESS;
}

//------------------------------------------------------------------------------------------

LangErr_t AssembleCmdSeparator(LangCtx_t* lang_ctx, TreeNode_t* node)
{
    assert(lang_ctx);
    assert(node);

    ASM_VERIFY_(IS_OPERATOR_(node, OP_CMD_SEPARATOR));
    ASM_VERIFY_(node->left);

    LangErr_t error = LANG_SUCCESS;

    if ((error = AssembleNode(lang_ctx, node->left)))
        return error;

    /*NOTE - there might be a cmd_separator with only left node
             for AST standard
    */
    if (!node->right)
        return LANG_SUCCESS;

    if ((error = AssembleNode(lang_ctx, node->right)))
        return error;

    return LANG_SUCCESS;
}

//------------------------------------------------------------------------------------------

LangErr_t AssembleIf(LangCtx_t* lang_ctx, TreeNode_t* node)
{
    assert(lang_ctx);
    assert(node);

    ASM_VERIFY_(IS_OPERATOR_(node, OP_IF_LHS));
    ASM_VERIFY_(node->left );
    ASM_VERIFY_(node->right);

    LangErr_t error = LANG_SUCCESS;

    if ((error = AssembleNode(lang_ctx, node->left)))
        return error;

    ASM_PRINT_(L"PUSH 0\n");
    ASM_PRINT_(L"JE :endif_%zu\n", lang_ctx->endif_labels_count);

    if ((error = AssembleNode(lang_ctx, node->right)))
        return error;

    ASM_PRINT_(L":endif_%zu\n", lang_ctx->endif_labels_count);

    lang_ctx->endif_labels_count++;

    ASM_PRINT_(L"\n");

    return LANG_SUCCESS;
}

//------------------------------------------------------------------------------------------

LangErr_t AssembleAssignment(LangCtx_t* lang_ctx, TreeNode_t* node)
{
    assert(lang_ctx);
    assert(node);

    ASM_VERIFY_(IS_OPERATOR_(node, OP_ASSIGNMENT));
    ASM_VERIFY_(node->left && IS_IDENTIFIER_(node->left));
    ASM_VERIFY_(node->right);

    LangErr_t error = LANG_SUCCESS;

    if ((error = AssembleNode(lang_ctx, node->right)))
        return error;

    ASM_PRINT_(L"PUSH %zu\n", node->data.value.id_index);
    ASM_PRINT_(L"POPR RAX\n"  );
    ASM_PRINT_(L"POPM [RAX]\n");

    ASM_PRINT_(L"\n");

    return LANG_SUCCESS;
}

//------------------------------------------------------------------------------------------

LangErr_t AssembleMathOperation(LangCtx_t* lang_ctx, TreeNode_t* node)
{
    assert(lang_ctx);
    assert(node);

    ASM_VERIFY_(IS_OPERATOR_(node, OP_ADD) ||
                IS_OPERATOR_(node, OP_SUB) ||
                IS_OPERATOR_(node, OP_MUL) ||
                IS_OPERATOR_(node, OP_DIV) ||
                IS_OPERATOR_(node, OP_POW));

    ASM_VERIFY_(node->left );
    ASM_VERIFY_(node->right);

    LangErr_t error = LANG_SUCCESS;

    if ((error = AssembleNode(lang_ctx, node->left)))
        return error;

    if ((error = AssembleNode(lang_ctx, node->right)))
        return error;

    ASM_PRINT_(L"%ls\n", OP_CASES_TABLE[node->data.value.opcode].asm_name);

    ASM_PRINT_(L"\n");

    return LANG_SUCCESS;
}

//------------------------------------------------------------------------------------------

LangErr_t AssembleUnaryOperation(LangCtx_t* lang_ctx, TreeNode_t* node)
{
    assert(lang_ctx);
    assert(node);

    ASM_VERIFY_(IS_OPERATOR_(node, OP_INPUT ) ||
                IS_OPERATOR_(node, OP_OUTPUT));

    ASM_VERIFY_(node->left == NULL);
    ASM_VERIFY_(node->right);

    LangErr_t error = LANG_SUCCESS;

    if ((error = AssembleNode(lang_ctx, node->right)))
        return error;

    ASM_PRINT_(L"%ls\n", OP_CASES_TABLE[node->data.value.opcode].asm_name);

    ASM_PRINT_(L"\n");

    return LANG_SUCCESS;
}

//------------------------------------------------------------------------------------------

LangErr_t AssembleInput(LangCtx_t* lang_ctx, TreeNode_t* node)
{
    assert(lang_ctx);
    assert(node);

    ASM_VERIFY_(IS_OPERATOR_(node, OP_INPUT));

    ASM_VERIFY_(node->left == NULL);
    ASM_VERIFY_(node->right && IS_IDENTIFIER_(node->right));

    LangErr_t error = LANG_SUCCESS;

    ASM_PRINT_(L"%ls\n", OP_CASES_TABLE[node->data.value.opcode].asm_name);

    /* get value from stack to a variable */
    ASM_PRINT_(L"PUSH %zu\n", node->data.value.id_index);
    ASM_PRINT_(L"POPR RAX\n"  );
    ASM_PRINT_(L"POPM [RAX]\n");

    ASM_PRINT_(L"\n");

    return LANG_SUCCESS;
}

//------------------------------------------------------------------------------------------

LangErr_t AssembleHlt(LangCtx_t* lang_ctx, TreeNode_t* node)
{
    assert(lang_ctx);
    assert(node);

    ASM_VERIFY_(IS_OPERATOR_(node, OP_ABORT));
    ASM_VERIFY_(node->left  == NULL);
    ASM_VERIFY_(node->right == NULL);

    ASM_PRINT_(L"%ls\n", OP_CASES_TABLE[node->data.value.opcode].asm_name);
    ASM_PRINT_(L"\n");

    return LANG_SUCCESS;
}

//——————————————————————————————————————————————————————————————————————————————————————————

#define _DSL_UNDEF_
#include "dsl.h"

//——————————————————————————————————————————————————————————————————————————————————————————
