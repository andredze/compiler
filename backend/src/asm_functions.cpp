#include "op_cases.h"
#include "lang_funcs.h"

//——————————————————————————————————————————————————————————————————————————————————————————

#define _DSL_DEFINE_
#include "dsl.h"

//——————————————————————————————————————————————————————————————————————————————————————————

static LangErr_t AssembleNumber              (LangCtx_t* lang_ctx, TreeNode_t* node);
static LangErr_t AssembleVariable            (LangCtx_t* lang_ctx, TreeNode_t* node);
static LangErr_t AssembleVariableBody        (LangCtx_t* lang_ctx, TreeNode_t* node);
static LangErr_t AssembleVariableDeclaration (LangCtx_t* lang_ctx, TreeNode_t* node);
static LangErr_t AssembleFunctionDeclaration (LangCtx_t* lang_ctx, TreeNode_t* node);
static LangErr_t AssembleFunctionParameters  (LangCtx_t* lang_ctx, TreeNode_t* node);
static LangErr_t AssembleFunctionCall        (LangCtx_t* lang_ctx, TreeNode_t* node);
static LangErr_t AssembleFunctionArguments   (LangCtx_t* lang_ctx, TreeNode_t* node);

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
            return LANG_INVALID_AST_INPUT;

        case TYPE_OP:
            if (OP_CASES_TABLE[node->data.value.opcode].asm_function == NULL)
            {
                WPRINTERR("Error: operator %ls doesn't support assembling",
                          OP_CASES_TABLE[node->data.value.opcode].name);

                return LANG_UNASSEMBLE_OPERATOR;
            }
            return OP_CASES_TABLE[node->data.value.opcode].asm_function (lang_ctx, node);

        case TYPE_VAR:
            return AssembleVariable(lang_ctx, node);

        case TYPE_VAR_DECL:
            return AssembleVariableDeclaration(lang_ctx, node);

        case TYPE_FUNC_DECL:
            return AssembleFunctionDeclaration(lang_ctx, node);

        case TYPE_FUNC_CALL:
            return AssembleFunctionCall(lang_ctx, node);

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

    ASM_PRINT_(L"; number: %lg\n", node->data.value.number);

    ASM_PRINT_(L"PUSH %lg\n", node->data.value.number);

    ASM_PRINT_(L"\n");

    return LANG_SUCCESS;
}

//------------------------------------------------------------------------------------------

static LangErr_t AssembleVariable(LangCtx_t* lang_ctx, TreeNode_t* node)
{
    assert(lang_ctx);
    assert(node);

    LangErr_t error = LANG_SUCCESS;

    if ((error = AssembleVariableBody(lang_ctx, node)))
        return error;

    ASM_PRINT_(L"PUSHM [RBX] ; push [rbx + addr] \n");
    ASM_PRINT_(L"\n");

    return LANG_SUCCESS;
}

//------------------------------------------------------------------------------------------

static LangErr_t AssembleVariableBody(LangCtx_t* lang_ctx, TreeNode_t* node)
{
    assert(lang_ctx);
    assert(node);

    ASM_VERIFY_(IS_VARIABLE_(node));
    ASM_VERIFY_(node->left  == NULL);
    ASM_VERIFY_(node->right == NULL);

    IdTable_t* id_table = NULL;

    if (lang_ctx->is_in_function)
        id_table = &lang_ctx->func_id_table;
    else
        id_table = &lang_ctx->main_id_table;

    size_t addr = 0;

    if (LangCheckVariableIsNotFunction(&lang_ctx->main_id_table, node->data.value.id))
    {
        WPRINTERR(L"Syntax error: function %ls used as a variable",
                    lang_ctx->names_pool.data[node->data.value.id]);
        return LANG_FUNC_USED_AS_VAR;
    }
    if (LangIdTableGetAddress(id_table, node->data.value.id, &addr))
    {
        WPRINTERR(L"Syntax error: variable %ls was not declared",
                    lang_ctx->names_pool.data[node->data.value.id]);
        return LANG_VAR_NOT_DECLARED;
    }

    ASM_PRINT_(L"; variable %ls\n\n", lang_ctx->names_pool.data[node->data.value.id]);

    if (lang_ctx->is_in_function)
    {
        ASM_PRINT_(L"; rbp + %zu (local address)\n", addr);
        ASM_PRINT_(L"PUSHR RGX ; rbp\n");
        ASM_PRINT_(L"PUSH %zu ; local addr\n", addr);
        ASM_PRINT_(L"ADD\n");
    }
    else
    {
        ASM_PRINT_(L"PUSH %zu ; global addr\n", addr);
    }

    ASM_PRINT_(L"POPR RBX ; RBX = global addr\n");

    return LANG_SUCCESS;
}

//------------------------------------------------------------------------------------------

static LangErr_t AssembleVariableDeclaration(LangCtx_t* lang_ctx, TreeNode_t* node)
{
    assert(lang_ctx);
    assert(node);

    ASM_VERIFY_(IS_VAR_DECL_(node));
    ASM_VERIFY_(node->left  == NULL);
    ASM_VERIFY_(node->right == NULL);

    ASM_PRINT_(L"; variable declaration: %ls\n\n",
               lang_ctx->names_pool.data[node->data.value.id]);

    IdTable_t* id_table = NULL;

    if (lang_ctx->is_in_function)
        id_table = &lang_ctx->func_id_table;
    else
        id_table = &lang_ctx->main_id_table;

    if (LangIdInTable(id_table, node->data.value.id))
    {
        WPRINTERR(L"Syntax error: redeclaration of variable %ls",
                    lang_ctx->names_pool.data[node->data.value.id]);
        return LANG_VAR_REDECLARATION;
    }

    LangErr_t error = LANG_SUCCESS;

    if ((error = LangIdTablePush(lang_ctx, id_table, node->data.value.id, ID_TYPE_VARIABLE, 0)))
        return error;

    ASM_PRINT_(L"; pushing stack of variables (rsp++)\n");

    ASM_PRINT_(L"PUSHR RHX ; rsp\n");
    ASM_PRINT_(L"PUSH 1\n");
    ASM_PRINT_(L"ADD\n");
    ASM_PRINT_(L"POPR RHX ; rsp = rsp + 1\n");
    ASM_PRINT_(L"\n");

    return LANG_SUCCESS;
}

//------------------------------------------------------------------------------------------

static LangErr_t AssembleFunctionDeclaration(LangCtx_t* lang_ctx, TreeNode_t* node)
{
    assert(lang_ctx);
    assert(node);

    size_t last_cur_addr = lang_ctx->cur_addr;

    ASM_VERIFY_(IS_FUNC_DECL_(node));
    ASM_VERIFY_(node->right != NULL);

    if (lang_ctx->is_in_function)
    {
        WPRINTERR("Syntax error: function declaration inside of function is not supported");
        return LANG_FUNC_DECL_IN_FUNC;
    }
    if (LangIdInTable(&lang_ctx->main_id_table, node->data.value.id))
    {
        WPRINTERR(L"Syntax error: redeclaration of function %ls",
                    lang_ctx->names_pool.data[node->data.value.id]);
        return LANG_FUNC_REDECLARATION;
    }

    LangErr_t error = LANG_SUCCESS;

    if ((error = LangIdTableCtor(&lang_ctx->func_id_table)))
        return error;

    lang_ctx->is_in_function = true;

    ASM_PRINT_(L"; function declaration: %ls\n\n",
               lang_ctx->names_pool.data[node->data.value.id]);

// transliterate
    ASM_PRINT_(L"JMP :%ls_end\n", lang_ctx->names_pool.data[node->data.value.id]);
    ASM_PRINT_(L":%ls\n", lang_ctx->names_pool.data[node->data.value.id]);

    ASM_PRINT_(L"PUSHR RGX ; save rbp\n\n");
    ASM_PRINT_(L"; copy rsp to rbp\n");
    ASM_PRINT_(L"PUSHR RHX \n");
    ASM_PRINT_(L"POPR RGX \n");

    if (node->left)
    {
        if ((error = AssembleFunctionParameters(lang_ctx, node)))
        {
            LangIdTableDtor(&lang_ctx->func_id_table);
            return error;
        }
    }
    if ((error = LangIdTablePush(lang_ctx, &lang_ctx->main_id_table, node->data.value.id,
                                 ID_TYPE_FUNCTION, lang_ctx->params_count)))
    {
        LangIdTableDtor(&lang_ctx->func_id_table);
        return error;
    }
    if ((error = AssembleNode(lang_ctx, node->right)))
    {
        LangIdTableDtor(&lang_ctx->func_id_table);
        return error;
    }

    ASM_PRINT_(L"; set rsp to the start\n\n");

    ASM_PRINT_(L"PUSHR RGX \n");
    ASM_PRINT_(L"POPR RHX \n\n");
    ASM_PRINT_(L"POPR RGX ; get previous rbp\n\n");

    ASM_PRINT_(L":%ls_end\n", lang_ctx->names_pool.data[node->data.value.id]);

    lang_ctx->is_in_function = 0;
    lang_ctx->cur_addr = last_cur_addr;

    LangIdTableDtor(&lang_ctx->func_id_table);

    return LANG_SUCCESS;
}

//------------------------------------------------------------------------------------------

static LangErr_t AssembleFunctionCall(LangCtx_t* lang_ctx, TreeNode_t* node)
{
    assert(lang_ctx);
    assert(node);

    ASM_VERIFY_(IS_FUNC_CALL_(node));

    size_t id_index = 0;

    if (!LangGetIdInTable(&lang_ctx->main_id_table, node->data.value.id, &id_index))
    {
        WPRINTERR(L"Syntax error: function %ls was not declared",
                    lang_ctx->names_pool.data[node->data.value.id]);
        return LANG_FUNC_NOT_DECLARED;
    }

    ASM_PRINT_(L"; function call %ls\n\n", lang_ctx->names_pool.data[node->data.value.id]);

    LangErr_t error = LANG_SUCCESS;

    size_t n_params = lang_ctx->main_id_table.data[id_index].n_params;

    if (node->left)
    {
        if ((error = AssembleFunctionArguments(lang_ctx, node)))
            return error;
    }

    if (n_params != lang_ctx->params_count)
    {
        WDPRINTF(L"n_params = %zu | lang_ctx->params_count = %zu\n",
                 n_params, lang_ctx->params_count);
        WPRINTERR(L"Syntax error: wrong args count for %ls",
                  lang_ctx->names_pool.data[node->data.value.id]);
        return LANG_WRONG_ARGS_COUNT;
    }

    ASM_PRINT_(L"CALL :%ls\n", lang_ctx->names_pool.data[node->data.value.id]);
    ASM_PRINT_(L"PUSHR RAX ; get return value\n");

    ASM_PRINT_(L"\n");

    return LANG_SUCCESS;
}

//------------------------------------------------------------------------------------------

LangErr_t AssembleFunctionParameters(LangCtx_t* lang_ctx, TreeNode_t* node)
{
    assert(lang_ctx);
    assert(node);

    ASM_PRINT_(L"; function parameters\n\n");

    lang_ctx->params_count = 0;

    LangErr_t error = LANG_SUCCESS;

    if (node->left)
    {
        if ((error = AssembleNode(lang_ctx, node->left)))
            return error;
    }

    if (!IS_OPERATOR_(node->left, OP_PARAMS_SEPARATOR))
    {
        lang_ctx->params_count++;
    }

    return LANG_SUCCESS;
}

//------------------------------------------------------------------------------------------

LangErr_t AssembleFunctionArguments(LangCtx_t* lang_ctx, TreeNode_t* node)
{
    assert(lang_ctx);
    assert(node);

    ASM_PRINT_(L"; function arguments\n\n");

    lang_ctx->assembling_args = true;
    lang_ctx->params_count = 0;

    if (!node->left)
        return LANG_SUCCESS;

    LangErr_t error = LANG_SUCCESS;

    if ((error = AssembleNode(lang_ctx, node->left)))
        return error;

    if (!IS_OPERATOR_(node->left, OP_PARAMS_SEPARATOR))
    {
        ASM_PRINT_(L"POPM [RHX]\n\n");
        lang_ctx->params_count++;
        lang_ctx->cur_addr++;
    }

    ASM_PRINT_(L"\n");

    lang_ctx->assembling_args = false;

    return LANG_SUCCESS;
}

//------------------------------------------------------------------------------------------

LangErr_t AssembleReturn(LangCtx_t* lang_ctx, TreeNode_t* node)
{
    assert(lang_ctx);
    assert(node);

    ASM_VERIFY_(IS_OPERATOR_(node, OP_RETURN));
    ASM_VERIFY_(node->right);

    ASM_PRINT_(L"; return\n\n");

    LangErr_t error = LANG_SUCCESS;

    if ((error = AssembleNode(lang_ctx, node->right)))
        return error;

    ASM_PRINT_(L"; set rsp (RHX) to current rbp (RGX)\n");
    ASM_PRINT_(L"PUSHR RGX\n");
    ASM_PRINT_(L"POPR RHX\n\n");
    ASM_PRINT_(L"POPR RAX ; put return value in RAX\n");
    ASM_PRINT_(L"; get previous rbp (RGX) from stack\n");
    ASM_PRINT_(L"POPR RGX\n\n");

    ASM_PRINT_(L"RET\n");
    ASM_PRINT_(L"\n");

    return LANG_SUCCESS;
}

//------------------------------------------------------------------------------------------

static void AsmIncrementRsp(LangCtx_t* lang_ctx)
{
    ASM_PRINT_(L"; rsp++\n");
    ASM_PRINT_(L"PUSHR RHX\n");
    ASM_PRINT_(L"PUSH 1\n");
    ASM_PRINT_(L"ADD\n");
    ASM_PRINT_(L"POPR RHX\n");
    lang_ctx->cur_addr++;
}

//------------------------------------------------------------------------------------------

LangErr_t AssembleParamsSeparator(LangCtx_t* lang_ctx, TreeNode_t* node)
{
    assert(lang_ctx);
    assert(node);

    ASM_VERIFY_(IS_OPERATOR_(node, OP_PARAMS_SEPARATOR));
    ASM_VERIFY_(node->left);

    lang_ctx->params_count++;

    LangErr_t error = LANG_SUCCESS;

    if ((error = AssembleNode(lang_ctx, node->left)))
        return error;

    if (lang_ctx->assembling_args)
    {
        AsmIncrementRsp(lang_ctx);
        ASM_PRINT_(L"POPM [RHX]\n\n");
    }

    if (node->right)
    {
        if ((error = AssembleNode(lang_ctx, node->right)))
            return error;

        if (lang_ctx->assembling_args && !IS_OPERATOR_(node->right, OP_PARAMS_SEPARATOR))
        {
            AsmIncrementRsp(lang_ctx);

            ASM_PRINT_(L"POPM [RHX]\n\n");
            lang_ctx->params_count++;
        }
        else if (IS_VAR_DECL_(node->right))
            lang_ctx->params_count++;
    }

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

    if (node->right == NULL)
        return LANG_SUCCESS;

    if ((error = AssembleNode(lang_ctx, node->right)))
        return error;

    return LANG_SUCCESS;
}

//------------------------------------------------------------------------------------------

static LangErr_t AssembleCondition(LangCtx_t* lang_ctx, TreeNode_t* node);

//——————————————————————————————————————————————————————————————————————————————————————————

LangErr_t AssembleIf(LangCtx_t* lang_ctx, TreeNode_t* node)
{
    assert(lang_ctx);
    assert(node);

    ASM_VERIFY_(IS_OPERATOR_(node, OP_IF_LHS));
    ASM_VERIFY_(node->left );
    ASM_VERIFY_(node->right);

    ASM_PRINT_(L"; if\n");
    ASM_PRINT_(L"; ----------------condition----------------\n\n");

    LangErr_t error = LANG_SUCCESS;

    if ((error = AssembleCondition(lang_ctx, node->left)))
        return error;

    ASM_PRINT_(L" :endif_%zu\n", lang_ctx->endif_labels_count);

    ASM_PRINT_(L"; ----------------statement----------------\n\n");

    if ((error = AssembleNode(lang_ctx, node->right)))
        return error;

    ASM_PRINT_(L":endif_%zu\n", lang_ctx->endif_labels_count);

    ASM_PRINT_(L"; ------------------endif------------------\n\n");

    lang_ctx->endif_labels_count++;

    ASM_PRINT_(L"\n");

    return LANG_SUCCESS;
}

//------------------------------------------------------------------------------------------

LangErr_t AssembleWhile(LangCtx_t* lang_ctx, TreeNode_t* node)
{
    assert(lang_ctx);
    assert(node);

    ASM_VERIFY_(IS_OPERATOR_(node, OP_WHILE));
    ASM_VERIFY_(node->left );
    ASM_VERIFY_(node->right);

    ASM_PRINT_(L"; while\n");

    ASM_PRINT_(L":while_start_%zu\n\n", lang_ctx->while_labels_count);
    ASM_PRINT_(L"; ----------------condition----------------\n\n");

    LangErr_t error = LANG_SUCCESS;

    if ((error = AssembleCondition(lang_ctx, node->left)))
        return error;

    ASM_PRINT_(L" :while_end_%zu\n", lang_ctx->while_labels_count);

    ASM_PRINT_(L"; ----------------statement----------------\n\n");

    if ((error = AssembleNode(lang_ctx, node->right)))
        return error;

    ASM_PRINT_(L"JMP :while_start_%zu\n", lang_ctx->while_labels_count);
    ASM_PRINT_(L":while_end_%zu\n", lang_ctx->while_labels_count);

    ASM_PRINT_(L"; ----------------while_end----------------\n\n");

    lang_ctx->while_labels_count++;

    ASM_PRINT_(L"\n");

    return LANG_SUCCESS;
}

//------------------------------------------------------------------------------------------

static LangErr_t AssembleCondition(LangCtx_t* lang_ctx, TreeNode_t* node)
{
    assert(lang_ctx);
    assert(node);

    ASM_VERIFY_(IS_TYPE_(node, TYPE_OP));
    ASM_VERIFY_(node->left );
    ASM_VERIFY_(node->right);

    LangErr_t error = LANG_SUCCESS;

    if ((error = AssembleNode(lang_ctx, node->left)))
        return error;

    if ((error = AssembleNode(lang_ctx, node->right)))
        return error;

    ASM_PRINT_(L"%ls", OP_CASES_TABLE[node->data.value.opcode].asm_name);

    return LANG_SUCCESS;
}

//------------------------------------------------------------------------------------------

LangErr_t AssembleAssignment(LangCtx_t* lang_ctx, TreeNode_t* node)
{
    assert(lang_ctx);
    assert(node);

    ASM_VERIFY_(IS_OPERATOR_(node, OP_ASSIGNMENT));
    ASM_VERIFY_(node->left && IS_VARIABLE_(node->left));
    ASM_VERIFY_(node->right);

    ASM_PRINT_(L"; assignment:\n\n");

    LangErr_t error = LANG_SUCCESS;

    if ((error = AssembleNode(lang_ctx, node->right)))
        return error;

    if ((error = AssembleVariableBody(lang_ctx, node->left)))
        return error;

    ASM_PRINT_(L"POPM [RBX] ; push [rbx + addr] \n");
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

    ASM_PRINT_(L"; math operation: %ls\n\n", OP_CASES_TABLE[node->data.value.opcode].asm_name);

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
                IS_OPERATOR_(node, OP_OUTPUT) ||
                IS_OPERATOR_(node, OP_SQRT));

    ASM_VERIFY_(node->left == NULL);
    ASM_VERIFY_(node->right);

    ASM_PRINT_(L"; output\n\n");

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
    ASM_VERIFY_(node->right && IS_VARIABLE_(node->right));

    ASM_PRINT_(L"; input\n\n", lang_ctx->names_pool.data[node->right->data.value.id]);
    ASM_PRINT_(L"%ls\n", OP_CASES_TABLE[node->data.value.opcode].asm_name);

    LangErr_t error = LANG_SUCCESS;

    if ((error = AssembleVariableBody(lang_ctx, node->right)))
        return error;

    ASM_PRINT_(L"POPM [RBX] ; push [rbx + addr] \n");
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

    ASM_PRINT_(L"; halt\n\n");

    ASM_PRINT_(L"%ls\n", OP_CASES_TABLE[node->data.value.opcode].asm_name);
    ASM_PRINT_(L"\n");

    return LANG_SUCCESS;
}

//——————————————————————————————————————————————————————————————————————————————————————————

#define _DSL_UNDEF_
#include "dsl.h"

//——————————————————————————————————————————————————————————————————————————————————————————
