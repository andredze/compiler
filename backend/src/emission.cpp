#include "keyword_cases.h"
#include "lang_funcs.h"
#include "backend.h"
#include "emission.h"
#include "encoding_x86.h"
#include "assembly_nasm.h"

//——————————————————————————————————————————————————————————————————————————————————————————

#define _DSL_DEFINE_
#include "dsl.h"

//——————————————————————————————————————————————————————————————————————————————————————————

static BackendErr_t EmitMain                (BackendCtx_t* backend_ctx, TreeNode_t* node);
static BackendErr_t EmitFunctions           (BackendCtx_t* backend_ctx, TreeNode_t* node);
static BackendErr_t EmitNumber              (BackendCtx_t* backend_ctx, TreeNode_t* node);
static BackendErr_t EmitVariable            (BackendCtx_t* backend_ctx, TreeNode_t* node);
static BackendErr_t EmitFunctionDeclaration (BackendCtx_t* backend_ctx, TreeNode_t* node);
static BackendErr_t EmitFunctionCall        (BackendCtx_t* backend_ctx, TreeNode_t* node);
static BackendErr_t EmitFunctionArguments   (BackendCtx_t* backend_ctx, TreeNode_t* node);

static BackendErr_t EmitLabel(BackendCtx_t* backend_ctx,
                              wchar_t*      label,
                              int           id_table_index,
                              size_t*       rel_table_index_dst);

//——————————————————————————————————————————————————————————————————————————————————————————
// на этом же этапе составлять таблицу rel_table для относительных сдвигов для колла
// и метки для насма + для джампов там же

BackendErr_t EmitProgram(BackendCtx_t* backend_ctx)
{
    DPRINT_FUNC_ENTER_MSG();
    assert(backend_ctx);

    BackendErr_t error = BACKEND_SUCCESS;

    ASM_PRINT_(
        L"global %ls\n\n"
        L"default rel\n\n"
        L"section .text\n\n",
        NASM_ENTRY_LABEL
    );

    if ((error = EmitFunctions(backend_ctx, backend_ctx->lang_ctx.tree.dummy->right)))
    {
        return error;
    }

    EMIT_VERIFY_(backend_ctx->main_node);

    if ((error = EmitMain(backend_ctx, backend_ctx->main_node)))
    {
        return error;
    }

    DPRINT_FUNC_LEAVE_MSG();
    return BACKEND_SUCCESS;
}

//==========================================================================================

static BackendErr_t EmitLabel(BackendCtx_t* backend_ctx, 
                              wchar_t*      label, 
                              int           id_table_index,
                              size_t*       rel_table_index_dst)
{
    assert(backend_ctx);
    assert(label);

    //TODO: translit
    ASM_PRINT_(L"%ls:\n", label);

    BackendErr_t error = BACKEND_SUCCESS;
    
    if ((error = RelTablePush(&backend_ctx->rel_table,
                              label,
                              backend_ctx->bin_code.size,
                              id_table_index,
                              rel_table_index_dst)))
    {
        return error;
    }

    REL_TABLE_DUMP_(L"after emitting label %ls at %zu (hex %#x)",
                    label,
                    backend_ctx->bin_code.size,
                    backend_ctx->bin_code.size);

    WDPRINTF(L"after emitting label %ls at %zu (hex %#x)\n", 
             label, 
             backend_ctx->bin_code.size,
             backend_ctx->bin_code.size);

    return BACKEND_SUCCESS;
}

//==========================================================================================

static BackendErr_t EmitMain(BackendCtx_t* backend_ctx, TreeNode_t* node)
{
    DPRINT_FUNC_ENTER_MSG();
    assert(backend_ctx);
    assert(node);

    EMIT_VERIFY_(IS_KEYWORD_(node, KW_CMD_SEPARATOR));
    EMIT_VERIFY_(node->left);

    BackendErr_t error = BACKEND_SUCCESS;
    
    ASM_PRINT_(L"\n%ls:\n", NASM_ENTRY_LABEL);

    // 0 table element is main
    size_t main_vars_size = 0;

    if ((error = BackendGetVarsStackSize(backend_ctx, 0, &main_vars_size)))
    {
        return error;
    }

    ASM_COMMENT_(L"stack frame for main variables");

    MOV_REG_REG_(REG_RBP, REG_RSP);
    MOV_REG_IMM_(REG_RDI, (int) main_vars_size);
    SUB_REG_REG_(REG_RSP, REG_RDI);

    if ((error = EmitNode(backend_ctx, node)))
    {    
        return error;
    }

    ASM_COMMENT_(L"end program");

    MOV_REG_IMM_(REG_RDI, (int) main_vars_size);
    ADD_REG_REG_(REG_RSP, REG_RDI);
    MOV_REG_IMM_(REG_RAX, SYSCALL_CODE_EXIT);
    MOV_REG_IMM_(REG_RDI, EXIT_SUCCESS);
    SYSCALL_();

    DPRINT_FUNC_LEAVE_MSG();
    return BACKEND_SUCCESS;
}

//==========================================================================================

static BackendErr_t EmitFunctions(BackendCtx_t* backend_ctx, TreeNode_t* node)
{
    DPRINT_FUNC_ENTER_MSG();
    assert(backend_ctx);
    assert(node);

    EMIT_VERIFY_(IS_KEYWORD_(node, KW_CMD_SEPARATOR));
    EMIT_VERIFY_(node->left);

    BackendErr_t error = BACKEND_SUCCESS;

    if (IS_FUNC_DECL_(node->left))
    {
        if ((error = EmitFunctionDeclaration(backend_ctx, node->left)))
        {
            return error;
        }
        if ((error = EmitFunctions(backend_ctx, node->right)))
        {
            return error;
        }

        DPRINT_FUNC_LEAVE_MSG();
        return BACKEND_SUCCESS;
    }

    // the first node without FUNC_DECL left child is main
    if (backend_ctx->main_node == NULL)
    {
        backend_ctx->main_node = node;
    }
    // but firstly emit all the other functions
    if (node->right)
    {
        if ((error = EmitFunctions(backend_ctx, node->right)))
        {
            return error;
        }
    }

    DPRINT_FUNC_LEAVE_MSG();
    return BACKEND_SUCCESS;
}

//==========================================================================================

BackendErr_t EmitNode(BackendCtx_t* backend_ctx, TreeNode_t* node)
{
    assert(backend_ctx);
    assert(node);

    switch (node->data.type)
    {
        case TYPE_NUM:
            return EmitNumber(backend_ctx, node);

        case TYPE_ID:
            return BACKEND_INVALID_AST_INPUT;

        case TYPE_KEYWORD:
        {
            KeywordCase_t  kw_case   = KEYWORD_CASES_TABLE[node->data.value.keyword];
            EmitFunction_t emit_func = kw_case.emit_function;

            if (emit_func == NULL)
            {
                WPRINTERR(L"Error: keyword %ls doesn't support assembling",
                          kw_case.name);

                return BACKEND_CANT_EMIT_KEYWORD;
            }
            return emit_func(backend_ctx, node);
        }

        case TYPE_VAR:
            return EmitVariable(backend_ctx, node);

        case TYPE_VAR_DECL:
            return BACKEND_SUCCESS;

        case TYPE_FUNC_DECL:
            // function declarations were emitted by EmitFunctions
            return BACKEND_SUCCESS;

        case TYPE_FUNC_CALL:
            return EmitFunctionCall(backend_ctx, node);

        default:
            return BACKEND_UNKNOWN_TOKEN_TYPE;
    }

    return BACKEND_SUCCESS;
}

//==========================================================================================

static BackendErr_t EmitFunctionDeclaration(BackendCtx_t* backend_ctx, TreeNode_t* node)
{
    DPRINT_FUNC_ENTER_MSG();
    assert(backend_ctx);
    assert(node);

    EMIT_VERIFY_(IS_FUNC_DECL_(node));
    EMIT_VERIFY_(node->right);

    ASM_PRINT_(L"\n");
    ASM_COMMENT_(L"function declaration %ls", BackendGetIdName(backend_ctx, node));
    
    BackendErr_t error = BACKEND_SUCCESS;

    if ((error = EmitLabel(backend_ctx,
                           BackendGetIdName(backend_ctx, node),
                           (int) node->data.value.id.id_index,
                           NULL)))
    {
        return error;
    }
    
    size_t stack_local_vars_size = 0;

    if ((error = BackendGetVarsStackSize(backend_ctx, 
                                         node->data.value.id.id_index, 
                                         &stack_local_vars_size)))
    {
        return error;
    }

    PUSH_REG_(REG_RBP);

    backend_ctx->current_stack_local_vars_size = stack_local_vars_size;
    
    MOV_REG_IMM_(REG_RDX, (int) stack_local_vars_size);
    SUB_REG_REG_(REG_RSP, REG_RDX);

    if ((error = EmitNode(backend_ctx, node->right)))
    {
        return error;
    }

    DPRINT_FUNC_LEAVE_MSG();
    return BACKEND_SUCCESS;
}

//==========================================================================================

static BackendErr_t EmitFunctionCall(BackendCtx_t* backend_ctx, TreeNode_t* node)
{
    assert(backend_ctx);
    assert(node);

    EMIT_VERIFY_(IS_FUNC_CALL_(node));
    EMIT_VERIFY_(node->right == NULL);

    ASM_COMMENT_(L"function call %ls", BackendGetIdName(backend_ctx, node));

    BackendErr_t error = BACKEND_SUCCESS;
    
    size_t stack_frame_size = 0;

    if ((error = BackendGetArgsStackSize(backend_ctx, 
                                         node->data.value.id.id_index, 
                                         &stack_frame_size)))
    {
        return error;
    }

    if (node->left)
    {
        if ((error = EmitFunctionArguments(backend_ctx, node->left)))
        {
            return error;
        }
    }

    size_t func_label_bin_code_pos = 0;

    if ((error = RelTableGetLabelBinCodePosByIdIndex(&backend_ctx->rel_table,
                                                     node->data.value.id.id_index,
                                                     &func_label_bin_code_pos)))
    {
        return error;
    }

    int rel_addr = CountLabelRelAddr(func_label_bin_code_pos, 
                                     backend_ctx->bin_code.size);

    CALL_REL_(rel_addr, BackendGetIdName(backend_ctx, node));

    // return stack to its state
    MOV_REG_IMM_(REG_RDX, (int) stack_frame_size);
    ADD_REG_REG_(REG_RSP, REG_RDX);

    // push return value
    PUSH_REG_(REG_RAX);

    return BACKEND_SUCCESS;
}

//==========================================================================================

static BackendErr_t EmitFunctionSingleArgument(BackendCtx_t* backend_ctx, TreeNode_t* node)
{
    assert(backend_ctx);
    assert(node);

    EMIT_VERIFY_(IS_VARIABLE_(node));
    EMIT_VERIFY_(node->left  == NULL);
    EMIT_VERIFY_(node->right == NULL);

    BackendErr_t error = BACKEND_SUCCESS;

    if ((error = EmitNode(backend_ctx, node)))
    {
        return error;
    }
    // argument was pushed in stack by EmitNode
    // so we can do nothing

    return BACKEND_SUCCESS;
}

//==========================================================================================

static BackendErr_t EmitFunctionArguments(BackendCtx_t* backend_ctx, TreeNode_t* node)
{
    assert(backend_ctx);
    assert(node);

    if (IS_KEYWORD_(node, KW_PARAMS_SEPARATOR))
    {
        EMIT_VERIFY_((node->left  != NULL) &&
                     (node->right != NULL));
    }
    
    BackendErr_t error = BACKEND_SUCCESS;

    if (IS_KEYWORD_(node, KW_PARAMS_SEPARATOR))
    {
        // right node is always a single argument
        if ((error = EmitFunctionSingleArgument(backend_ctx, node->right)))
        {
            return error;
        }
        // keep doing recursive calls for left subtree
        if ((error = EmitFunctionArguments(backend_ctx, node->left)))
        {
            return error;
        }
    }
    else
    {
        // stop recursion
        if ((error = EmitFunctionSingleArgument(backend_ctx, node)))
        {
            return error;
        }
    }

    return BACKEND_SUCCESS;
}

//==========================================================================================

BackendErr_t EmitReturn(BackendCtx_t* backend_ctx, TreeNode_t* node)
{
    assert(backend_ctx);
    assert(node);

    EMIT_VERIFY_(IS_KEYWORD_(node, KW_RETURN));
    EMIT_VERIFY_(node->left  == NULL);
    EMIT_VERIFY_(node->right);

    BackendErr_t error = BACKEND_SUCCESS;

    // Get return value
    if ((error = EmitNode(backend_ctx, node->right)))
    {
        return error;
    }

    POP_REG_(REG_RAX);

    MOV_REG_IMM_(REG_RDX, (int) backend_ctx->current_stack_local_vars_size);
    ADD_REG_REG_(REG_RSP, REG_RDX);

    POP_REG_(REG_RBP);

    RET_();

    return BACKEND_SUCCESS;
}

//==========================================================================================

static BackendErr_t EmitVariable(BackendCtx_t* backend_ctx, TreeNode_t* node)
{
    assert(backend_ctx);
    assert(node);

    EMIT_VERIFY_(IS_VARIABLE_(node));
    EMIT_VERIFY_(node->left  == NULL);
    EMIT_VERIFY_(node->right == NULL);
    
    BackendErr_t error = BACKEND_SUCCESS;

    int var_offset = 0;

    if ((error = BackendGetVariableOffset(backend_ctx, 
                                          node->data.value.id.id_index, 
                                          &var_offset)))
    {
        return error;
    }

    ASM_COMMENT_(L"variable %ls with offset %d", 
                 BackendGetIdName(backend_ctx, node),
                 var_offset);

    MOV_REG_MEM_DISP_(REG_RDX, REG_RBP, var_offset);

    PUSH_REG_(REG_RDX);

    return BACKEND_SUCCESS;
}

//==========================================================================================

static BackendErr_t EmitNumber(BackendCtx_t* backend_ctx, TreeNode_t* node)
{
    assert(backend_ctx);
    assert(node);

    EMIT_VERIFY_(IS_NUMBER_(node));
    EMIT_VERIFY_(node->left  == NULL);
    EMIT_VERIFY_(node->right == NULL);

    ASM_COMMENT_(L"number");

    MOV_REG_IMM_(REG_RDX, node->data.value.number);
    
    PUSH_REG_(REG_RDX);

    return BACKEND_SUCCESS;
}

//==========================================================================================

BackendErr_t EmitMathExprOperation(BackendCtx_t* backend_ctx, TreeNode_t* node)
{
    assert(backend_ctx);
    assert(node);

    EMIT_VERIFY_(IS_KEYWORD_(node, KW_ADD) ||
                 IS_KEYWORD_(node, KW_SUB) ||
                 IS_KEYWORD_(node, KW_MUL) ||
                 IS_KEYWORD_(node, KW_DIV));

    EMIT_VERIFY_(node->left );
    EMIT_VERIFY_(node->right);

    ASM_COMMENT_(L"math expression %ls", 
                 KEYWORD_CASES_TABLE[node->data.value.keyword].name);

    BackendErr_t error = BACKEND_SUCCESS;

    if ((error = EmitNode(backend_ctx, node->left)))
    {
        return error;
    }
    if ((error = EmitNode(backend_ctx, node->right)))
    {
        return error;
    }

    POP_REG_(REG_RBX); // right node
    POP_REG_(REG_RAX); // left  node
    
    if (IS_KEYWORD_(node, KW_ADD))
    {
        ADD_REG_REG_(REG_RAX, REG_RBX);
    }
    else if (IS_KEYWORD_(node, KW_SUB))
    {
        SUB_REG_REG_(REG_RAX, REG_RBX);
    }
    else if (IS_KEYWORD_(node, KW_MUL))
    {
        MOV_REG_IMM_(REG_RDX, 0);
        IMUL_REG_(REG_RBX);
    }
    else
    {
        MOV_REG_IMM_(REG_RDX, 0);
        IDIV_REG_(REG_RBX);
    }

    PUSH_REG_(REG_RAX);

    return BACKEND_SUCCESS;
}

//==========================================================================================

BackendErr_t EmitAssignment(BackendCtx_t* backend_ctx, TreeNode_t* node)
{
    assert(backend_ctx);
    assert(node);

    TreeNode_t* var = node->left;

    if (!IS_VARIABLE_(var))
    {
        return BACKEND_SUCCESS;
    }

    EMIT_VERIFY_(var->left  == NULL);
    EMIT_VERIFY_(var->right == NULL);
    EMIT_VERIFY_(node->right);

    BackendErr_t error = BACKEND_SUCCESS;

    if ((error = EmitNode(backend_ctx, node->right)))
    {
        return error;
    }

    int var_offset = 0;

    if ((error = BackendGetVariableOffset(backend_ctx, 
                                          var->data.value.id.id_index,
                                          &var_offset)))
    {
        return error;
    }

    ASM_COMMENT_(L"assignment to variable %ls with offset %d", 
                 BackendGetIdName(backend_ctx, var),
                 var_offset);

    POP_REG_(REG_RDX);

    MOV_MEM_DISP_REG_(REG_RBP, var_offset, REG_RDX);

    return BACKEND_SUCCESS;
}

//==========================================================================================

BackendErr_t EmitIf(BackendCtx_t* backend_ctx, TreeNode_t* node)
{
    assert(backend_ctx);
    assert(node);

    


    return BACKEND_SUCCESS;
}

//==========================================================================================

BackendErr_t EmitUnaryOperation (BackendCtx_t* backend_ctx, TreeNode_t* node)
{
    assert(backend_ctx);
    assert(node);

    return BACKEND_SUCCESS;
}

//==========================================================================================

BackendErr_t EmitInput          (BackendCtx_t* backend_ctx, TreeNode_t* node)
{
    assert(backend_ctx);
    assert(node);

    return BACKEND_SUCCESS;
}

//==========================================================================================

BackendErr_t EmitWhile          (BackendCtx_t* backend_ctx, TreeNode_t* node)
{
    assert(backend_ctx);
    assert(node);

    return BACKEND_SUCCESS;
}

//==========================================================================================

BackendErr_t EmitCmdSeparator(BackendCtx_t* backend_ctx, TreeNode_t* node)
{
    assert(backend_ctx);
    assert(node);

    EMIT_VERIFY_(IS_KEYWORD_(node, KW_CMD_SEPARATOR));
    EMIT_VERIFY_(node->left);

    BackendErr_t error = BACKEND_SUCCESS;

    if ((error = EmitNode(backend_ctx, node->left)))
    {
        return error;
    }
    /*NOTE - there might be a cmd_separator with only left node
             for AST standard
    */
    if (node->right == NULL)
    {
        return BACKEND_SUCCESS;
    }
    if ((error = EmitNode(backend_ctx, node->right)))
    {
        return error;
    }

    return BACKEND_SUCCESS;
}

//==========================================================================================

BackendErr_t EmitHlt            (BackendCtx_t* backend_ctx, TreeNode_t* node)
{
    assert(backend_ctx);
    assert(node);

    return BACKEND_SUCCESS;
}

//==========================================================================================

BackendErr_t EmitPoint          (BackendCtx_t* backend_ctx, TreeNode_t* node)
{
    assert(backend_ctx);
    assert(node);

    return BACKEND_SUCCESS;
}

//==========================================================================================

#define _DSL_UNDEF_
#include "dsl.h"

//——————————————————————————————————————————————————————————————————————————————————————————
