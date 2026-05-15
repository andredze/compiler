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

    ASM_PRINT_(
        L"global _start\n\n"
        L"default rel\n\n"
        L"section .text\n\n"
        L"_start:\n"
    );

    size_t main_vars_size = backend_ctx->lang_ctx.global_vars_count *
                            TYPE_INT_SIZE_IN_BYTES;

    ASM_COMMENT_(L"stack frame for main variables");

    MOV_REG_IMM_(REG_RDI, (int) main_vars_size);
    SUB_REG_REG_(REG_RSP, REG_RDI);
    MOV_REG_REG_(REG_RBP, REG_RSP);

    BackendErr_t error = BACKEND_SUCCESS;

    if ((error = EmitNode(backend_ctx, backend_ctx->lang_ctx.tree.dummy->right)))
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
            KeywordCase_t  kw_case   = KEYWORD_CASES_TABLE[node->data.value.keyword];
            EmitFunction_t emit_func = kw_case.emit_function;

            if (emit_func == NULL)
            {
                WPRINTERR(L"Error: keyword %ls doesn't support assembling",
                          kw_case.name);

                return BACKEND_CANT_EMIT_KEYWORD;
            }
            return emit_func(backend_ctx, node);

        case TYPE_VAR:
            return EmitVariable(backend_ctx, node);

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

static BackendErr_t EmitVariable(BackendCtx_t* backend_ctx, TreeNode_t* node)
{
    assert(backend_ctx);
    assert(node);

    ASM_VERIFY_(IS_VARIABLE_(node));
    ASM_VERIFY_(node->left  == NULL);
    ASM_VERIFY_(node->right == NULL);
    
    int var_offset = 0;

    if (LangIdTableGetAddress(&backend_ctx->lang_ctx, 
                              node->data.value.id, 
                              &var_offset))
    {
        return BACKEND_LANG_ERROR;
    }

    MOV_REG_MEM_DISP_(REG_RDX, REG_RBP, var_offset);

    PUSH_REG_(REG_RDX);

    return BACKEND_SUCCESS;
}

//==========================================================================================

static BackendErr_t EmitNumber(BackendCtx_t* backend_ctx, TreeNode_t* node)
{
    assert(backend_ctx);
    assert(node);

    ASM_VERIFY_(IS_NUMBER_(node));
    ASM_VERIFY_(node->left  == NULL);
    ASM_VERIFY_(node->right == NULL);

    ASM_COMMENT_(L"number");

    MOV_REG_IMM_(REG_RDX, node->data.value.number);
    
    PUSH_REG_(REG_RDX);

    return BACKEND_SUCCESS;
}

//==========================================================================================

BackendErr_t EmitMathExpressionOperation(BackendCtx_t* backend_ctx, TreeNode_t* node)
{
    assert(backend_ctx);
    assert(node);

    ASM_VERIFY_(IS_KEYWORD_(node, KW_ADD) ||
                IS_KEYWORD_(node, KW_SUB));

    ASM_VERIFY_(node->left );
    ASM_VERIFY_(node->right);

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
    
    if (node->data.type == KW_ADD)
    {
        ADD_REG_REG_(REG_RAX, REG_RBX);
    }
    else
    {
        SUB_REG_REG_(REG_RAX, REG_RBX);
    }

    return BACKEND_SUCCESS;
}

//==========================================================================================

BackendErr_t EmitIf(BackendCtx_t* backend_ctx, TreeNode_t* node)
{
    return BACKEND_SUCCESS;
}

//==========================================================================================

BackendErr_t EmitAssignment     (BackendCtx_t* backend_ctx, TreeNode_t* node)
{
    return BACKEND_SUCCESS;
}

//==========================================================================================

BackendErr_t EmitMathOperation  (BackendCtx_t* backend_ctx, TreeNode_t* node)
{
    return BACKEND_SUCCESS;
}

//==========================================================================================

BackendErr_t EmitUnaryOperation (BackendCtx_t* backend_ctx, TreeNode_t* node)
{
    return BACKEND_SUCCESS;
}

//==========================================================================================

BackendErr_t EmitInput          (BackendCtx_t* backend_ctx, TreeNode_t* node)
{
    return BACKEND_SUCCESS;
}
// BackendErr_t EmitElse           (BackendCtx_t* backend_ctx, TreeNode_t* node);

//==========================================================================================

BackendErr_t EmitWhile          (BackendCtx_t* backend_ctx, TreeNode_t* node)
{
    return BACKEND_SUCCESS;
}

//==========================================================================================

BackendErr_t EmitCmdSeparator   (BackendCtx_t* backend_ctx, TreeNode_t* node)
{
    return BACKEND_SUCCESS;
}

//==========================================================================================

BackendErr_t EmitParamsSeparator(BackendCtx_t* backend_ctx, TreeNode_t* node)
{
    return BACKEND_SUCCESS;
}

//==========================================================================================

BackendErr_t EmitHlt            (BackendCtx_t* backend_ctx, TreeNode_t* node)
{
    return BACKEND_SUCCESS;
}

//==========================================================================================

BackendErr_t EmitReturn         (BackendCtx_t* backend_ctx, TreeNode_t* node)
{
    return BACKEND_SUCCESS;
}

//==========================================================================================

BackendErr_t EmitPoint          (BackendCtx_t* backend_ctx, TreeNode_t* node)
{
    return BACKEND_SUCCESS;
}

//==========================================================================================

#define _DSL_UNDEF_
#include "dsl.h"

//——————————————————————————————————————————————————————————————————————————————————————————
