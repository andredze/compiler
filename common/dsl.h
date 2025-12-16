#ifdef _DSL_DEFINE_

//——————————————————————————————————————————————————————————————————————————————————————————

#include "tree_types.h"
#include "tree_commands.h"

/* =============== Domain Specific Language for my programming language =============== */

#define OPERATOR_(op_code)    LangOperatorNodeCtor      (lang_ctx, (op_code ), NULL, NULL)
#define IDENTIFIER_(id_index) LangIdentifierNodeCtor    (lang_ctx, (id_index))
#define NUMBER_(number)       LangNumberNodeCtor        (lang_ctx, (number  ))

//------------------------------------------------------------------------------------------

#define IS_OPERATOR_(node, op_code) ((node)->data.type == TYPE_OP && (node)->data.value.opcode == (op_code))
#define IS_IDENTIFIER_(node)        ((node)->data.type == TYPE_ID )
#define IS_NUMBER_(node)            ((node)->data.type == TYPE_NUM)

//------------------------------------------------------------------------------------------

/* if operation has 1 argument, it should be placed in right node */

#define ISVALUE_(node, number) (node->data.type == TYPE_NUM && \
                                CompareDoubles(node->data.value.num, (number)) == 0)

#define ADD_(l, r)          LangOperatorNodeCtor(lang_ctx, OP_ADD, (l),  (r))
#define SUB_(l, r)          LangOperatorNodeCtor(lang_ctx, OP_SUB, (l),  (r))
#define MUL_(l, r)          LangOperatorNodeCtor(lang_ctx, OP_MUL, (l),  (r))
#define DIV_(l, r)          LangOperatorNodeCtor(lang_ctx, OP_DIV, (l),  (r))
#define POW_(l, r)          LangOperatorNodeCtor(lang_ctx, OP_POW, (l),  (r))
#define UNARY_(oper, r)     LangOperatorNodeCtor(lang_ctx, (oper), NULL, (r))

//------------------------------------------------------------------------------------------

#define ASM_PRINT_(...)                                 \
        BEGIN                                           \
        fwprintf(lang_ctx->output_file, ##__VA_ARGS__); \
        END

#define ASM_VERIFY_(cond)                                 \
        BEGIN                                             \
        if (!(cond))                                      \
        {                                                 \
            WPRINTERR(L"ASM_VERIFY_(%s) dropped", #cond); \
            return LANG_BACKEND_AST_SYNTAX_ERROR;         \
        }                                                 \
        END

/* ==================================================================================== */

#undef    _DSL_DEFINE_
#endif /* _DSL_DEFINE_ */

//——————————————————————————————————————————————————————————————————————————————————————————

#ifdef _DSL_UNDEF_

//------------------------------------------------------------------------------------------

#undef OPERATOR_
#undef IDENTIFIER_
#undef NUMBER_

#undef IS_OPERATOR_
#undef IS_IDENTIFIER_
#undef IS_NUMBER_

#undef ISVALUE_

#undef NUM_
#undef ADD_
#undef SUB_
#undef MUL_
#undef DIV_
#undef POW_
#undef UNARY_

#undef ASM_PRINT_

//------------------------------------------------------------------------------------------

#undef    _DSL_UNDEF_
#endif /* _DSL_UNDEF_*/
