#include "op_cases.h"
#include "lang_funcs.h"
#include "backend.h"
#include "emission.h"
#include "encoding_x86.h"

//——————————————————————————————————————————————————————————————————————————————————————————

#define _DSL_DEFINE_
#include "dsl.h"

//——————————————————————————————————————————————————————————————————————————————————————————

static BackendErr_t EmitNumber              (BackendCtx_t* backend_ctx, TreeNode_t* node);
static BackendErr_t EmitVariable            (BackendCtx_t* backend_ctx, TreeNode_t* node);
static BackendErr_t EmitVariableBody        (BackendCtx_t* backend_ctx, TreeNode_t* node);
static BackendErr_t EmitVariableDeclaration (BackendCtx_t* backend_ctx, TreeNode_t* node);
static BackendErr_t EmitFunctionDeclaration (BackendCtx_t* backend_ctx, TreeNode_t* node);
static BackendErr_t EmitNewVariable         (BackendCtx_t* backend_ctx, TreeNode_t* node);
static BackendErr_t EmitFunctionParameters  (BackendCtx_t* backend_ctx, TreeNode_t* node);
static BackendErr_t EmitFunctionCall        (BackendCtx_t* backend_ctx, TreeNode_t* node);
static BackendErr_t EmitFunctionArguments   (BackendCtx_t* backend_ctx, TreeNode_t* node);
static void         EmitArgument            (BackendCtx_t* backend_ctx);

//——————————————————————————————————————————————————————————————————————————————————————————

BackendErr_t EmitProgram(BackendCtx_t* backend_ctx)
{
    DPRINT_FUNC_ENTER_MSG();
    assert(backend_ctx);

    ASM_PRINT_NO_TAB(
        L"global main\n\n"
        L"section .text\n\n"
        L"main:\n"
    );

    size_t global_vars_size = backend_ctx->lang_ctx.global_vars_count *
                              TYPE_INT_SIZE_IN_BYTES;

    ASM_PRINT_(
        L"; stack frame for global variables"
        L"\tsub rsp, %zu\n"
        L"\tmov rbp, rsp\n", 
        global_vars_size);

    BackendErr_t error = BACKEND_SUCCESS;

    if ((error = EmitNode(backend_ctx, backend_ctx->lang_ctx.tree.dummy->right)))
    {    
        return error;
    }

    ASM_PRINT_(
        L"; end program\n\n"
        L"add rsp, %zu\n"
        L"mov rax, 0x3c\n"
        L"xor edi, edi\n"
        L"syscall\n",
        global_vars_size
    );

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
            // return EmitNumber(backend_ctx, node);

        case TYPE_ID:
            return BACKEND_INVALID_AST_INPUT;

        case TYPE_OP:
            if (OP_CASES_TABLE[node->data.value.opcode].emit_function == NULL)
            {
                WPRINTERR("Error: operator %ls doesn't support assembling",
                          OP_CASES_TABLE[node->data.value.opcode].name);

                return BACKEND_CANT_EMIT_OPERATOR;
            }
            return OP_CASES_TABLE[node->data.value.opcode].emit_function(backend_ctx, node);

        case TYPE_VAR:
            // return EmitVariable(backend_ctx, node);

        case TYPE_VAR_DECL:
            // return EmitVariableDeclaration(backend_ctx, node);

        case TYPE_FUNC_DECL:
            // return EmitFunctionDeclaration(backend_ctx, node);

        case TYPE_FUNC_CALL:
            // return EmitFunctionCall(backend_ctx, node);

        default:
            return BACKEND_UNKNOWN_TOKEN_TYPE;
    }

    return BACKEND_SUCCESS;
}

//==========================================================================================

BackendErr_t EmitMathExpressionOperation(BackendCtx_t* backend_ctx, TreeNode_t* node)
{
    assert(backend_ctx);
    assert(node);

    ASM_VERIFY_(IS_OPERATOR_(node, OP_ADD) ||
                IS_OPERATOR_(node, OP_SUB));

    ASM_VERIFY_(node->left );
    ASM_VERIFY_(node->right);

    // ASM_PRINT_(L"; math operation: %ls\n\n", 
    //            OP_CASES_TABLE[node->data.value.opcode].asm_name);

    BackendErr_t error = BACKEND_SUCCESS;

    if ((error = EmitNode(backend_ctx, node->left)))
    {
        return error;
    }
    if ((error = EmitNode(backend_ctx, node->right)))
    {    
        return error;
    }

    // POP_IN_REG(REG_RBX); // right node
    // POP_IN_REG(REG_RAX); // left node
    
    if (node->data.type == OP_ADD)
    {
        // ADD_REG_TO_REG(REG_RAX, REG_RBX);
    }
    else
    {
        // SUB_REG_FROM_REG(REG_RAX, REG_RBX);
    }

    // ASM_PRINT_(L"pop rbx");
    // ASM_PRINT_(L"pop rcx");
    // ASM_PRINT_(L"add rcx, rbx");
    // ASM_PRINT_(L"%ls\n", OP_CASES_TABLE[node->data.value.opcode].asm_name);
    // ASM_PRINT_(L"\n");

    return BACKEND_SUCCESS;
}

BackendErr_t EmitIf             (BackendCtx_t* backend_ctx, TreeNode_t* node)
{
    return BACKEND_SUCCESS;
}
BackendErr_t EmitAssignment     (BackendCtx_t* backend_ctx, TreeNode_t* node)
{
    return BACKEND_SUCCESS;
}
BackendErr_t EmitMathOperation  (BackendCtx_t* backend_ctx, TreeNode_t* node)
{
    return BACKEND_SUCCESS;
}
BackendErr_t EmitUnaryOperation (BackendCtx_t* backend_ctx, TreeNode_t* node)
{
    return BACKEND_SUCCESS;
}
BackendErr_t EmitInput          (BackendCtx_t* backend_ctx, TreeNode_t* node)
{
    return BACKEND_SUCCESS;
}
// BackendErr_t EmitElse           (BackendCtx_t* backend_ctx, TreeNode_t* node);
BackendErr_t EmitWhile          (BackendCtx_t* backend_ctx, TreeNode_t* node)
{
    return BACKEND_SUCCESS;
}
BackendErr_t EmitCmdSeparator   (BackendCtx_t* backend_ctx, TreeNode_t* node)
{
    return BACKEND_SUCCESS;
}
BackendErr_t EmitParamsSeparator(BackendCtx_t* backend_ctx, TreeNode_t* node)
{
    return BACKEND_SUCCESS;
}
BackendErr_t EmitHlt            (BackendCtx_t* backend_ctx, TreeNode_t* node)
{
    return BACKEND_SUCCESS;
}
BackendErr_t EmitReturn         (BackendCtx_t* backend_ctx, TreeNode_t* node)
{
    return BACKEND_SUCCESS;
}
BackendErr_t EmitPoint          (BackendCtx_t* backend_ctx, TreeNode_t* node)
{
    return BACKEND_SUCCESS;
}

//==========================================================================================

#define _DSL_UNDEF_
#include "dsl.h"

//——————————————————————————————————————————————————————————————————————————————————————————
