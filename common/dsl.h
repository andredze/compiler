#ifdef _DSL_DEFINE_

//——————————————————————————————————————————————————————————————————————————————————————————

#include "tree_types.h"

#ifdef FRONTEND
        #include "frontend.h"
#endif /* FRONTEND */

#include "tree_commands.h"

/* =============== Domain Specific Language for my programming language =============== */

#define FRONTEND_SET_ERROR_(frontend_ctx, error, node, message, ...)              \
        BEGIN                                                                     \
            LangErrorInfo_t info_ = {error, node, __func__, __FILE__, __LINE__};  \
            FrontendSetError(frontend_ctx, &info_, message, ##__VA_ARGS__);       \
        END

#define SET_LEXER_ERROR_(error, node, expected, ...)                                               \
        BEGIN                                                                                      \
        FRONTEND_SET_ERROR_(frontend_ctx, LANG_LEXER_SYNTAX_ERROR, NULL, expected, ##__VA_ARGS__); \
        END

#define SET_PARSER_ERROR_(node, expected, ...)                                                      \
        BEGIN                                                                                       \
        FRONTEND_SET_ERROR_(frontend_ctx, LANG_PARSER_SYNTAX_ERROR, node, expected, ##__VA_ARGS__); \
        END

#define IDENTIFIER_(id_index) LangIdentifierNodeCtor    (&frontend_ctx->lang_ctx, (name_index))

#define OPERATOR_(op_code)    LangOperatorNodeCtor      (&frontend_ctx->lang_ctx, (op_code ), NULL, NULL)
#define NUMBER_(number)       LangNumberNodeCtor        (&frontend_ctx->lang_ctx, (number  ))

//==========================================================================================

#define IS_TYPE_(node, _type)       ((node)->data.type == (_type))
#define IS_OPERATOR_(node, op_code) ((node)->data.type == TYPE_OP && (node)->data.value.opcode == (op_code))
#define IS_VARIABLE_(node)          ((node)->data.type == TYPE_VAR       )
#define IS_VAR_DECL_(node)          ((node)->data.type == TYPE_VAR_DECL  )
#define IS_FUNC_DECL_(node)         ((node)->data.type == TYPE_FUNC_DECL )
#define IS_FUNC_CALL_(node)         ((node)->data.type == TYPE_FUNC_CALL )
#define IS_IDENTIFIER_(node)        ((node)->data.type == TYPE_ID )
#define IS_NUMBER_(node)            ((node)->data.type == TYPE_NUM)
#define HAS_OPCODE_(node, op_code)  ((node)->data.value.opcode == (op_code))

//==========================================================================================

/* if operation has 1 argument, it should be placed in right node */

#define ISVALUE_(node, number) (node->data.type == TYPE_NUM && \
                                CompareDoubles(node->data.value.num, (number)) == 0)

#define ADD_(l, r)          LangOperatorNodeCtor(&frontend_ctx->lang_ctx, OP_ADD, (l),  (r))
#define SUB_(l, r)          LangOperatorNodeCtor(&frontend_ctx->lang_ctx, OP_SUB, (l),  (r))
#define MUL_(l, r)          LangOperatorNodeCtor(&frontend_ctx->lang_ctx, OP_MUL, (l),  (r))
#define DIV_(l, r)          LangOperatorNodeCtor(&frontend_ctx->lang_ctx, OP_DIV, (l),  (r))
#define POW_(l, r)          LangOperatorNodeCtor(&frontend_ctx->lang_ctx, OP_POW, (l),  (r))
#define UNARY_(oper, r)     LangOperatorNodeCtor(&frontend_ctx->lang_ctx, (oper), NULL, (r))

//==========================================================================================

#define SRC_PRINT_TABS_()                                                \
        BEGIN                                                            \
        fwprintf(frontend_ctx->output_file, L"%*s", 4*lang_ctx->tabs, L" "); \
        END

#define SRC_PRINT_(...)                                                  \
        BEGIN                                                            \
        fwprintf(frontend_ctx->output_file, ##__VA_ARGS__);                  \
        END

#define SRC_PRINT_ID_(node__)                                                   \
        BEGIN                                                                   \
        SRC_PRINT_(L"%ls", frontend_ctx->names_pool.data[(node__)->data.value.id]); \
        END

#define SRC_PRINT_OP_(opcode_)                              \
        BEGIN                                               \
        SRC_PRINT_(L"%ls", OP_CASES_TABLE[(opcode_)].name); \
        END

//------------------------------------------------------------------------------------------

#define ASM_PRINT_NO_TAB(...)                           \
        BEGIN                                           \
        fwprintf(backend_ctx->asm_file, ##__VA_ARGS__); \
        END

#define ASM_PRINT_(...)                                     \
        BEGIN                                               \
        fwprintf(backend_ctx->asm_file, L"\t" __VA_ARGS__); \
        END

//------------------------------------------------------------------//
/*                            Encoding                              */

#define ENCODE_VERIFY_(cond)                                    \
        BEGIN                                                   \
        if (!(cond))                                            \
        {                                                       \
            WPRINTERR(L"ENCODE_VERIFY_(%s) dropped", #cond);    \
            return BACKEND_INVALID_INSTRUCTION;                 \
        }                                                       \
        END

//------------------------------------------------------------------//

#define GENERATE_CODE_(instr_create_func, opcode, ...)                            \
        BEGIN                                                                     \
        Instruction_t* instr = instr_create_func(opcode, ##__VA_ARGS__);          \
                                                                                  \
        if (instr == NULL)                                                        \
        {                                                                         \
            return BACKEND_CREATE_INSTRUCTION_ERROR;                              \
        }                                                                         \
                                                                                  \
        INSTRUCTION_DUMP(instr);                                                  \
                                                                                  \
        BackendErr_t error = BACKEND_SUCCESS;                                     \
                                                                                  \
        if ((error = GenerateCodeFromInstruction(&backend_ctx->bin_code, instr))) \
        {                                                                         \
            return error;                                                         \
        }                                                                         \
                                                                                  \
        BIN_CODE_DUMP(&backend_ctx->bin_code);                                    \
        END

//------------------------------------------------------------------//

#define MOV_REG_REG_(reg1, reg2) \
        GENERATE_CODE_(InstructionCreateRegReg, OPCODE_MOV_REG_REG, reg1, reg2)

// can not be used with rsp and r12 as memory base register
#define MOV_REG_MEM_(reg, base) \
        GENERATE_CODE_(InstructionCreateRegMem, OPCODE_MOV_REG_MEM, reg, base, 0)

#define MOV_REG_MEM_DISP_(reg, base, disp) \
        GENERATE_CODE_(InstructionCreateRegMem, OPCODE_MOV_REG_MEM, reg, base, disp)

#define MOV_MEM_REG_(base, reg) \
        GENERATE_CODE_(InstructionCreateMemReg, OPCODE_MOV_MEM_REG, base, 0, reg)

#define MOV_MEM_DISP_REG_(base, disp, reg) \
        GENERATE_CODE_(InstructionCreateMemReg, OPCODE_MOV_MEM_REG, base, disp, reg)

#define MOV_REG_IMM_(reg, imm) \
        GENERATE_CODE_(InstructionCreateRegImm, OPCODE_MOV_REG_IMM, reg, imm)

#define ADD_REG_REG_(reg1, reg2) \
        GENERATE_CODE_(InstructionCreateRegReg, OPCODE_ADD_REG_REG, reg1, reg2)

#define SUB_REG_REG_(reg1, reg2) \
        GENERATE_CODE_(InstructionCreateRegReg, OPCODE_SUB_REG_REG, reg1, reg2)

#define IMUL_REG_(reg) \
        GENERATE_CODE_(InstructionCreateRegNone, OPCODE_IMUL_REG, reg)

#define IDIV_REG_(reg) \
        GENERATE_CODE_(InstructionCreateRegNone, OPCODE_IDIV_REG, reg)

#define PUSH_REG_(reg) \
        GENERATE_CODE_(InstructionCreateRegNone, OPCODE_PUSH_REG, reg)

#define POP_REG_(reg) \
        GENERATE_CODE_(InstructionCreateRegNone, OPCODE_POP_REG, reg)

#define CALL_REL_(rel) \
        GENERATE_CODE_(InstructionCreateRelNone, OPCODE_CALL_REL, rel)

#define JMP_REL_(rel) \
        GENERATE_CODE_(InstructionCreateRelNone, OPCODE_JMP_REL, rel)

#define JE_REL_(rel) \
        GENERATE_CODE_(InstructionCreateRelNone, OPCODE_JE_REL, rel)

#define JNE_REL_(rel) \
        GENERATE_CODE_(InstructionCreateRelNone, OPCODE_JNE_REL, rel)

#define JA_REL_(rel) \
        GENERATE_CODE_(InstructionCreateRelNone, OPCODE_JA_REL, rel)

#define JAE_REL_(rel) \
        GENERATE_CODE_(InstructionCreateRelNone, OPCODE_JAE_REL, rel)

#define JB_REL_(rel) \
        GENERATE_CODE_(InstructionCreateRelNone, OPCODE_JB_REL, rel)

#define JBE_REL_(rel) \
        GENERATE_CODE_(InstructionCreateRelNone, OPCODE_JBE_REL, rel)

#define RET_() \
        GENERATE_CODE_(InstructionCreateNoneNone, OPCODE_RET)

#define SYSCALL_() \
        GENERATE_CODE_(InstructionCreateNoneNone, OPCODE_SYSCALL)

#define CMP_REG_REG_(reg1, reg2) \
        GENERATE_CODE_(InstructionCreateRegReg, OPCODE_CMP_REG_REG, reg1, reg2)

#define CMP_REG_IMM_(reg, imm) \
        GENERATE_CODE_(InstructionCreateRegImm, OPCODE_CMP_REG_IMM, reg, imm)

//------------------------------------------------------------------//

#define ASM_VERIFY_(cond)                                 \
        BEGIN                                             \
        if (!(cond))                                      \
        {                                                 \
            WPRINTERR(L"ASM_VERIFY_(%s) dropped", #cond); \
            return BACKEND_INVALID_AST_INPUT;             \
        }                                                 \
        END

/* ==================================================================================== */

#undef    _DSL_DEFINE_
#endif /* _DSL_DEFINE_ */

//——————————————————————————————————————————————————————————————————————————————————————————

#ifdef _DSL_UNDEF_

//==========================================================================================

#undef SET_PARSER_ERROR_
#undef SET_LEXER_ERROR_

#undef OPERATOR_
#undef IDENTIFIER_
#undef NUMBER_

#undef IS_TYPE_
#undef IS_OPERATOR_
#undef IS_IDENTIFIER_
#undef IS_NUMBER_
#undef HAS_OPCODE_

#undef ISVALUE_

#undef NUM_
#undef ADD_
#undef SUB_
#undef MUL_
#undef DIV_
#undef POW_
#undef UNARY_

#undef SRC_PRINT_
#undef SRC_PRINT_ID_
#undef SRC_PRINT_OP_

#undef ASM_PRINT_
#undef ASM_VERIFY_

#undef ENCODE_VERIFY_

//------------------------------------------------------------------//

#undef GENERATE_CODE_

#undef MOV_REG_REG_
#undef MOV_REG_MEM_
#undef MOV_REG_MEM_DISP_
#undef MOV_MEM_DISP_REG_
#undef MOV_MEM_REG_
#undef MOV_REG_IMM_
#undef ADD_REG_REG_
#undef SUB_REG_REG_
#undef IMUL_REG_
#undef IDIV_REG_
#undef PUSH_REG_
#undef POP_REG_
#undef RET_
#undef SYSCALL_

//==========================================================================================

#undef    _DSL_UNDEF_
#endif /* _DSL_UNDEF_*/
