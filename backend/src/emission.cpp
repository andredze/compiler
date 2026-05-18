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

static BackendErr_t EmitNode                (BackendCtx_t* backend_ctx, TreeNode_t* node);
static BackendErr_t EmitMain                (BackendCtx_t* backend_ctx, TreeNode_t* node);
static BackendErr_t EmitFunctions           (BackendCtx_t* backend_ctx, TreeNode_t* node);
static BackendErr_t EmitNumber              (BackendCtx_t* backend_ctx, TreeNode_t* node);
static BackendErr_t EmitVariable            (BackendCtx_t* backend_ctx, TreeNode_t* node);
static BackendErr_t EmitFunctionDeclaration (BackendCtx_t* backend_ctx, TreeNode_t* node);
static BackendErr_t EmitFunctionCall        (BackendCtx_t* backend_ctx, TreeNode_t* node);
static BackendErr_t EmitFunctionArguments   (BackendCtx_t* backend_ctx, TreeNode_t* node,
                                             int is_extern_function);

static BackendErr_t EmitAndPushLabel(BackendCtx_t* backend_ctx,
                                     wchar_t*      label,
                                     int           id_table_index);

//——————————————————————————————————————————————————————————————————————————————————————————

BackendErr_t EmitProgram(BackendCtx_t* backend_ctx)
{
    DPRINT_FUNC_ENTER_MSG();
    assert(backend_ctx);

    BackendErr_t error = BACKEND_SUCCESS;

    ASM_PRINT_(
        L"global %ls\n\n"
        L"default rel\n\n"
        L"section .text\n\n"
        L"extern %ls\n"
        L"extern %ls\n"
        L"extern %ls\n"
        L"extern %ls\n"
        L"extern %ls\n\n",
        MAIN_ENTRY_LABEL,
        KW_INPUT_FUNC_NAME,
        KW_OUTPUT_FUNC_NAME,
        KW_SQRT_FUNC_NAME,
        KW_DRAW_FUNC_NAME,
        KW_POINT_FUNC_NAME
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

    if ((error = RelTableCountFuncSizes(&backend_ctx->rel_table, 
                                        BinCodeGetCurrentPos(&backend_ctx->bin_code))))
    {
        return error;
    }

    REL_TABLE_DUMP_(L"counted funcs size");

    DPRINT_FUNC_LEAVE_MSG();
    return BACKEND_SUCCESS;
}

//==========================================================================================

static BackendErr_t AlignStackIfNeeded(BackendCtx_t* backend_ctx)
{
    assert(backend_ctx);

    if ((backend_ctx->current_func_stack_alignment & 0b1) == 0b1)
    {
        ASM_COMMENT_(L"align the stack (curr_size = %d)", 
                     backend_ctx->current_func_stack_alignment * 8);

        SUB_REG_IMM_(REG_RSP, 8);
        backend_ctx->did_an_alignment = 1;
    }

    return BACKEND_SUCCESS;
}

//==========================================================================================

static BackendErr_t ReturnStackAlignmentIfNeeded(BackendCtx_t* backend_ctx)
{
    assert(backend_ctx);

    if (backend_ctx->did_an_alignment)
    {
        ASM_COMMENT_(L"return the stack alignment (curr_size = %d)", 
                     backend_ctx->current_func_stack_alignment * 8);

        ADD_REG_IMM_(REG_RSP, 8);
        backend_ctx->did_an_alignment = 0;
    }

    return BACKEND_SUCCESS;
}

//==========================================================================================

BackendErr_t EmitExit(BackendCtx_t* backend_ctx, TreeNode_t* node)
{
    assert(backend_ctx);

    POP_REG_(REG_RBP);
    MOV_REG_IMM_(REG_RAX, SYSCALL_CODE_EXIT);
    MOV_REG_IMM_(REG_RDI, EXIT_SUCCESS);
    SYSCALL_();

    return BACKEND_SUCCESS;
}

//==========================================================================================

static BackendErr_t EmitAndPushLabel(BackendCtx_t* backend_ctx, 
                                     wchar_t*      label, 
                                     int           id_table_index)
{
    assert(backend_ctx);
    assert(label);

    ASM_PRINT_(L"%ls:\n", label);

    BackendErr_t error = BACKEND_SUCCESS;
    
    if ((error = RelTablePushInnerFuncDecl(&backend_ctx->rel_table,
                                           (size_t) id_table_index,
                                           label,
                                           BinCodeGetCurrentPos(&backend_ctx->bin_code),
                                           REL_FUNC_LOCAL)))
    {
        return error;
    }

    REL_TABLE_DUMP_(L"after emitting label %ls at %zu (hex %#x)",
                    label,
                    BinCodeGetCurrentPos(&backend_ctx->bin_code),
                    BinCodeGetCurrentPos(&backend_ctx->bin_code));

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
    
    ASM_PRINT_(L"\n%ls:\n", MAIN_ENTRY_LABEL);

    size_t main_start_bin_code_pos = BinCodeGetCurrentPos(&backend_ctx->bin_code);

    // 0 table element is main
    size_t main_vars_size = 0;

    if ((error = BackendGetVarsStackSize(backend_ctx, 0, &main_vars_size)))
    {
        return error;
    }

    backend_ctx->current_func_stack_alignment = 1;

    ASM_COMMENT_(L"stack frame for main variables");

    backend_ctx->current_func_stack_alignment = 1;

    PUSH_REG_(REG_RBP);
    MOV_REG_REG_(REG_RBP, REG_RSP);
    SUB_REG_IMM_(REG_RSP, (int) main_vars_size);

    if ((error = EmitNode(backend_ctx, node)))
    {    
        return error;
    }

    ASM_COMMENT_(L"end program");

    ADD_REG_IMM_(REG_RSP, (int) main_vars_size);

    if ((error = EmitExit(backend_ctx, NULL)))
    {
        return error;
    }

    if ((error = RelTablePushInnerFuncDecl(&backend_ctx->rel_table,
                                           0,
                                           MAIN_ENTRY_LABEL,
                                           main_start_bin_code_pos,
                                           REL_FUNC_GLOBAL)))
    {
        return error;
    }

    REL_TABLE_DUMP_(L"GLOBAL: %ls", MAIN_ENTRY_LABEL);

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

static BackendErr_t EmitNode(BackendCtx_t* backend_ctx, TreeNode_t* node)
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
                WPRINTERR(L"Error: keyword %ls doesn't support emission",
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

    if ((error = EmitAndPushLabel(backend_ctx,
                                  BackendGetIdName(backend_ctx, node),
                                  (int) node->data.value.id.id_index)))
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

    backend_ctx->current_func_stack_alignment = 1;

    PUSH_REG_(REG_RBP);

    MOV_REG_REG_(REG_RBP, REG_RSP);

    backend_ctx->current_stack_local_vars_size = stack_local_vars_size;
    
    SUB_REG_IMM_(REG_RSP, (int) stack_local_vars_size);

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

    if ((error = AlignStackIfNeeded(backend_ctx)))
    {
        return error;
    }

    if (node->left)
    {
        if ((error = EmitFunctionArguments(backend_ctx, node->left, 0)))
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

    if ((error = RelTablePushInnerFuncCall(&backend_ctx->rel_table,
                                           node->data.value.id.id_index,
                                           BackendGetIdName(backend_ctx, node),
                                           BinCodeGetCurrentPos(&backend_ctx->bin_code))))
    {
        return error;
    }

    REL_TABLE_DUMP_(L"CALL: added func %ls", BackendGetIdName(backend_ctx, node));

    int rel_addr = CountLabelRelAddr(func_label_bin_code_pos, 
                                     BinCodeGetCurrentPos(&backend_ctx->bin_code));

    CALL_REL_(rel_addr, BackendGetIdName(backend_ctx, node));

    // return stack to its state
    if ((error = ReturnStackAlignmentIfNeeded(backend_ctx)))
    {
        return error;
    }

    ADD_REG_IMM_(REG_RSP, (int) stack_frame_size);

    return BACKEND_SUCCESS;
}

//==========================================================================================

static BackendErr_t EmitFunctionSingleArgument(BackendCtx_t* backend_ctx, TreeNode_t* node,
                                               int is_extern_function)
{
    assert(backend_ctx);
    assert(node);

    BackendErr_t error = BACKEND_SUCCESS;

    if ((error = EmitNode(backend_ctx, node)))
    {
        return error;
    }

    size_t reg_index = backend_ctx->current_args_count;

    if (is_extern_function && reg_index <= CALL_ARGS_REGS_COUNT)
    {   
        MOV_REG_REG_(CALL_REGS_TABLE[reg_index], REG_RAX);
    }
    else
    {
        PUSH_REG_(REG_RAX);
    }

    backend_ctx->current_args_count++;

    return BACKEND_SUCCESS;
}

//==========================================================================================

static BackendErr_t EmitFunctionArguments(BackendCtx_t* backend_ctx, TreeNode_t* node,
                                          int is_extern_function)
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
        // right order
        if (is_extern_function)
        {
            // keep doing recursive calls for left subtree
            if ((error = EmitFunctionArguments(backend_ctx, node->left,
                                               is_extern_function)))
                return error;
            // right node is always a single argument
            if ((error = EmitFunctionSingleArgument(backend_ctx, node->right,
                                                    is_extern_function)))
                return error;
        }
        else
        {
            // right node is always a single argument
            if ((error = EmitFunctionSingleArgument(backend_ctx, node->right,
                                                    is_extern_function)))
                return error;
            // keep doing recursive calls for left subtree
            if ((error = EmitFunctionArguments(backend_ctx, node->left,
                                               is_extern_function)))
                return error;
        }
    }
    else
    {
        // stop recursion
        if ((error = EmitFunctionSingleArgument(backend_ctx, node,
                                                is_extern_function)))
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

    // Get return value to rax
    if ((error = EmitNode(backend_ctx, node->right)))
    {
        return error;
    }

    ADD_REG_IMM_(REG_RSP, (int) backend_ctx->current_stack_local_vars_size);

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

    MOV_REG_MEM_DISP_(REG_RAX, REG_RBP, var_offset);

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

    MOV_REG_IMM_(REG_RAX, node->data.value.number);

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

    if ((error = EmitNode(backend_ctx, node->right)))
    {
        return error;
    }

    // right subtree result (second operand) is in RAX, 
    // save on stack for calling left subtree emission
    PUSH_REG_(REG_RAX); 

    if ((error = EmitNode(backend_ctx, node->left)))
    {
        return error;
    }

    // first operand is in rax
    // get second operand to rbx
    POP_REG_(REG_RCX);
    
    if (IS_KEYWORD_(node, KW_ADD))
    {
        ADD_REG_REG_(REG_RAX, REG_RCX);
    }
    else if (IS_KEYWORD_(node, KW_SUB))
    {
        SUB_REG_REG_(REG_RAX, REG_RCX);
    }
    else if (IS_KEYWORD_(node, KW_MUL))
    {
        IMUL_REG_(REG_RCX);
    }
    else
    {
        MOV_REG_IMM_(REG_RDX, 0);
        CQO_();
        IDIV_REG_(REG_RCX);
    }

    return BACKEND_SUCCESS;
}

//==========================================================================================

static BackendErr_t EmitAssignRegRaxToVariable(BackendCtx_t* backend_ctx, TreeNode_t* var)
{
    assert(backend_ctx);
    assert(var);

    EMIT_VERIFY_(IS_VARIABLE_(var));
    EMIT_VERIFY_(var->left  == NULL);
    EMIT_VERIFY_(var->right == NULL);

    BackendErr_t error = BACKEND_SUCCESS;

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

    MOV_MEM_DISP_REG_(REG_RBP, var_offset, REG_RAX);

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
    // result is in rax

    if ((error = EmitAssignRegRaxToVariable(backend_ctx, var)))
    {
        return error;
    }

    return BACKEND_SUCCESS;
}

//==========================================================================================

static OpcodeType_t GetJccOpcodeFromKeyword(Keyword_t jcc_kw)
{
    switch (jcc_kw)
    {
        case KW_EQUAL:          return OPCODE_JE_REL;
        case KW_NOT_EQUAL:      return OPCODE_JNE_REL;
        case KW_BIGGER:         return OPCODE_JG_REL;
        case KW_BIGGER_EQUAL:   return OPCODE_JGE_REL;
        case KW_SMALLER:        return OPCODE_JL_REL;
        case KW_SMALLER_EQUAL:  return OPCODE_JLE_REL;
        default:                break;
    }

    WPRINTERR(L"Not a comparison keyword given to function for getting jcc opcodes"
              L" given %d", jcc_kw);

    return OPCODE_UNKNOWN;
}

//==========================================================================================

static OpcodeType_t GetJccOppositeOpcodeFromKeyword(Keyword_t jcc_kw)
{
    switch (jcc_kw)
    {
        case KW_EQUAL:          return OPCODE_JNE_REL;
        case KW_NOT_EQUAL:      return OPCODE_JE_REL;
        case KW_BIGGER:         return OPCODE_JLE_REL;
        case KW_BIGGER_EQUAL:   return OPCODE_JL_REL;
        case KW_SMALLER:        return OPCODE_JGE_REL;
        case KW_SMALLER_EQUAL:  return OPCODE_JG_REL;
        default:                break;
    }

    WPRINTERR(L"Not a comparison keyword given to function for getting jcc opcodes"
              L" given %d", jcc_kw);

    return OPCODE_UNKNOWN;
}

//==========================================================================================

static BackendErr_t EmitConditionComparison(BackendCtx_t* backend_ctx, TreeNode_t* node)
{
    assert(backend_ctx);
    assert(node);

    EMIT_VERIFY_(IS_KEYWORD_(node, KW_EQUAL        ) ||
                 IS_KEYWORD_(node, KW_NOT_EQUAL    ) ||
                 IS_KEYWORD_(node, KW_BIGGER       ) ||
                 IS_KEYWORD_(node, KW_BIGGER_EQUAL ) ||
                 IS_KEYWORD_(node, KW_SMALLER      ) ||
                 IS_KEYWORD_(node, KW_SMALLER_EQUAL));

    EMIT_VERIFY_(node->left );
    EMIT_VERIFY_(node->right);

    BackendErr_t error = BACKEND_SUCCESS;

    if ((error = EmitNode(backend_ctx, node->right)))
    {
        return error;
    }

    PUSH_REG_(REG_RAX);

    if ((error = EmitNode(backend_ctx, node->left)))
    {
        return error;
    }

    // first operand (left subtree) result is in rax
    // second operand (right subtree) result
    POP_REG_(REG_RCX);

    CMP_REG_REG_(REG_RAX, REG_RCX);

    return BACKEND_SUCCESS;
}

//==========================================================================================

static BackendErr_t EmitJccForFixup(BackendCtx_t* backend_ctx, 
                                    OpcodeType_t  jcc_opcode, 
                                    size_t*       jump_addr,
                                    size_t*       jump_disp_addr,
                                    wchar_t*      jump_label)
{
    assert(jump_label);
    assert(jump_disp_addr);
    assert(jump_addr);
    assert(backend_ctx);

    *jump_addr = BinCodeGetCurrentPos(&backend_ctx->bin_code);
    WDPRINTF(L"Got jump_addr %zu (%#x)\n", *jump_addr, *jump_addr);

    // rel = 0 for fixing later; when 0, disp will be equal to -instr_size
    JCC_REL_(jcc_opcode, 0, jump_label);

    *jump_disp_addr = BinCodeGetCurrentPos(&backend_ctx->bin_code) - DISP_SIZE;
    WDPRINTF(L"Got jump_disp_addr %zu (%#x)\n", *jump_disp_addr, *jump_disp_addr);

    return BACKEND_SUCCESS;
}

//==========================================================================================

static BackendErr_t EmitJccNodeForFixup(BackendCtx_t* backend_ctx, 
                                        TreeNode_t*   node, 
                                        size_t*       jump_addr,
                                        size_t*       jump_disp_addr,
                                        wchar_t*      jump_label)
{
    assert(jump_label);
    assert(jump_disp_addr);
    assert(jump_addr);
    assert(backend_ctx);
    assert(node);

    OpcodeType_t jcc_opcode = GetJccOppositeOpcodeFromKeyword(node->data.value.keyword);

    if (jcc_opcode == OPCODE_UNKNOWN)
    {
        return BACKEND_INVALID_OPCODE;
    }

    return EmitJccForFixup(backend_ctx, jcc_opcode, 
                           jump_addr, jump_disp_addr, jump_label);
}

//==========================================================================================

static BackendErr_t EmitConditionForFixup(BackendCtx_t* backend_ctx, 
                                          TreeNode_t*   node, 
                                          size_t*       jump_addr,
                                          size_t*       jump_disp_addr,
                                          wchar_t*      wrong_condition_label)
{
    assert(wrong_condition_label);
    assert(jump_disp_addr);
    assert(jump_addr);
    assert(backend_ctx);
    assert(node);

    BackendErr_t error = BACKEND_SUCCESS;

    if ((error = EmitConditionComparison(backend_ctx, node)))
    {
        return error;
    }
    if ((error = EmitJccNodeForFixup(backend_ctx, node,
                                     jump_addr, jump_disp_addr,
                                     wrong_condition_label)))
    {
        return error;
    }

    return BACKEND_SUCCESS;
}

//==========================================================================================

static BackendErr_t EmitAndFixupLabel(BackendCtx_t* backend_ctx, 
                                      wchar_t*      label,
                                      size_t        jump_addr,
                                      size_t        jump_disp_addr)
{
    assert(backend_ctx);
    assert(label);

    ASM_PRINT_(L"%ls:\n", label);

    int rel_addr = CountLabelRelAddr(BinCodeGetCurrentPos(&backend_ctx->bin_code),
                                     jump_addr);

    BinAddToDisplacement(&backend_ctx->bin_code, jump_disp_addr, rel_addr);

    WDPRINTF(L"Dump after inserting an addition of %#x rel_addr"
             L" to %zu (%#x) jump_disp_addr", rel_addr, jump_disp_addr, jump_disp_addr);
    BIN_CODE_DUMP(&backend_ctx->bin_code);

    return BACKEND_SUCCESS;
}

//==========================================================================================

BackendErr_t EmitIf(BackendCtx_t* backend_ctx, TreeNode_t* node)
{
    assert(backend_ctx);
    assert(node);

    EMIT_VERIFY_(IS_KEYWORD_(node, KW_IF_LHS));
    EMIT_VERIFY_(node->left );
    EMIT_VERIFY_(node->right);

    BackendErr_t error = BACKEND_SUCCESS;

    wchar_t endif_label[MAX_BUFFER_SIZE] = {};

    swprintf(endif_label, sizeof(endif_label) / sizeof(endif_label[0]),
             L".endif_%zu", backend_ctx->endif_labels_count++);

    size_t jump_addr      = 0;
    size_t jump_disp_addr = 0;

    if ((error = EmitConditionForFixup(backend_ctx, node->left, 
                                       &jump_addr, 
                                       &jump_disp_addr, 
                                       endif_label)))
    {
        return error;
    }
    // if-statement body
    if ((error = EmitNode(backend_ctx, node->right)))
    {
        return error;
    }
    if ((error = EmitAndFixupLabel(backend_ctx, endif_label, jump_addr, jump_disp_addr)))
    {
        return error;
    }

    return BACKEND_SUCCESS;
}

//==========================================================================================

static BackendErr_t EmitConditionNoFixup(BackendCtx_t* backend_ctx, 
                                         TreeNode_t*   node, 
                                         OpcodeType_t  jcc_opcode,
                                         wchar_t*      condition_label,
                                         size_t        condition_label_addr)
{
    assert(condition_label);
    assert(backend_ctx);
    assert(node);

    EMIT_VERIFY_(OPCODE_JMP_REL <= jcc_opcode && jcc_opcode <= OPCODE_JLE_REL);

    BackendErr_t error = BACKEND_SUCCESS;

    if ((error = EmitConditionComparison(backend_ctx, node)))
    {
        return error;
    }

    int rel_addr = CountLabelRelAddr(condition_label_addr, 
                                     BinCodeGetCurrentPos(&backend_ctx->bin_code));

    JCC_REL_(jcc_opcode, rel_addr, condition_label);

    return BACKEND_SUCCESS;
}

//==========================================================================================

BackendErr_t EmitWhile(BackendCtx_t* backend_ctx, TreeNode_t* node)
{
    assert(backend_ctx);
    assert(node);

    EMIT_VERIFY_(IS_KEYWORD_(node, KW_WHILE));
    EMIT_VERIFY_(node->left );
    EMIT_VERIFY_(node->right);

    BackendErr_t error = BACKEND_SUCCESS;

    // jmp .check_condition
    //------------------------------------------------------------------//
    wchar_t check_condition_label[MAX_BUFFER_SIZE] = {};
    wchar_t next_label           [MAX_BUFFER_SIZE] = {};

    swprintf(check_condition_label,
             sizeof(check_condition_label) / sizeof(check_condition_label[0]),
             L".check_condition_%zu", backend_ctx->while_labels_count);

    swprintf(next_label, sizeof(next_label) / sizeof(next_label[0]),
             L".next_%zu", backend_ctx->while_labels_count);
    
    backend_ctx->while_labels_count++;

    size_t jmp_check_cond_addr      = 0;
    size_t jmp_check_cond_disp_addr = 0;

    if ((error = EmitJccForFixup(backend_ctx, OPCODE_JMP_REL,
                                 &jmp_check_cond_addr,
                                 &jmp_check_cond_disp_addr,
                                 check_condition_label)))
    {
        return error;
    }
    //------------------------------------------------------------------//
    // .next:

    ASM_PRINT_(L"%ls:\n", next_label);
    size_t next_label_addr = BinCodeGetCurrentPos(&backend_ctx->bin_code);
    //------------------------------------------------------------------//
    // cycle body
    if ((error = EmitNode(backend_ctx, node->right)))
    {
        return error;
    }
    //------------------------------------------------------------------//
    // .check_condition:
    if ((error = EmitAndFixupLabel(backend_ctx, check_condition_label, 
                                   jmp_check_cond_addr, 
                                   jmp_check_cond_disp_addr)))
    {
        return error;
    }
    //------------------------------------------------------------------//
    // jcc .next

    OpcodeType_t jcc_opcode = GetJccOpcodeFromKeyword(node->left->data.value.keyword);

    if (jcc_opcode == OPCODE_UNKNOWN)
    {
        return BACKEND_INVALID_OPCODE;
    }

    if ((error = EmitConditionNoFixup(backend_ctx, node->left, jcc_opcode, 
                                      next_label,  next_label_addr)))
    {
        return error;
    }
    //------------------------------------------------------------------//

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

BackendErr_t EmitLibFuncCall(BackendCtx_t* backend_ctx, TreeNode_t* node)
{
    assert(backend_ctx);
    assert(node);

    EMIT_VERIFY_(IS_KEYWORD_(node, KW_OUTPUT) ||
                 IS_KEYWORD_(node, KW_SQRT  ) ||
                 IS_KEYWORD_(node, KW_DRAW  ) ||
                 IS_KEYWORD_(node, KW_POINT ));
    
    EMIT_VERIFY_(node->left == NULL);

    BackendErr_t error = BACKEND_SUCCESS;
    
    backend_ctx->current_args_count = 0;

    if (node->right)
    {
        if ((error = EmitFunctionArguments(backend_ctx, node->right, 1)))
        {
            return error;
        }
    }

    backend_ctx->current_args_count = 0;

    const wchar_t* func_name = NULL;

    switch (node->data.value.keyword)
    {
        case KW_OUTPUT: func_name = KW_OUTPUT_FUNC_NAME; break;
        case KW_SQRT:   func_name = KW_SQRT_FUNC_NAME;   break;
        case KW_DRAW:   func_name = KW_DRAW_FUNC_NAME;   break;
        case KW_POINT:  func_name = KW_POINT_FUNC_NAME;  break;
        default:
            WPRINTERR(L"Keyword %d for unary operation doesn't have a function",
                      node->data.value.keyword);
            return BACKEND_INVALID_AST_INPUT;
    }

    if ((error = RelTablePushExternCall(&backend_ctx->rel_table, 
                                        func_name, 
                                        BinCodeGetCurrentPos(&backend_ctx->bin_code))))
    {
        return error;
    }

    CALL_REL_(UNDEFINED_FUNC_ADDR, func_name);
    
    REL_TABLE_DUMP_(L"EXTERN: %s added func %ls", 
                    KEYWORD_CASES_TABLE[node->data.value.keyword].code_str,
                    func_name);

    return BACKEND_SUCCESS;
}

//==========================================================================================

BackendErr_t EmitInput(BackendCtx_t* backend_ctx, TreeNode_t* node)
{
    assert(backend_ctx);
    assert(node);

    EMIT_VERIFY_(IS_KEYWORD_(node, KW_INPUT));
    EMIT_VERIFY_(node->right);
    EMIT_VERIFY_(IS_VARIABLE_(node->right));
    EMIT_VERIFY_(node->left == NULL);

    const wchar_t* func_name = KW_INPUT_FUNC_NAME;

    BackendErr_t error = BACKEND_SUCCESS;

    if ((error = RelTablePushExternCall(&backend_ctx->rel_table, 
                                        func_name, 
                                        BinCodeGetCurrentPos(&backend_ctx->bin_code))))
    {
        return error;
    }

    CALL_REL_(UNDEFINED_FUNC_ADDR, func_name);

    if ((error = EmitAssignRegRaxToVariable(backend_ctx, node->right)))
    {
        return error;
    }

    REL_TABLE_DUMP_(L"EXTERN: %s added func %ls", 
                    KEYWORD_CASES_TABLE[node->data.value.keyword].code_str,
                    func_name);

    return BACKEND_SUCCESS;
}

//==========================================================================================

#define _DSL_UNDEF_
#include "dsl.h"

//——————————————————————————————————————————————————————————————————————————————————————————
