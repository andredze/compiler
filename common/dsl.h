#ifdef _DSL_DEFINE_

//——————————————————————————————————————————————————————————————————————————————————————————

#include "tree_types.h"
#include "tree_commands.h"

/* =============== Domain Specific Language for my programming language =============== */

#define SET_LEXER_ERROR_(error, node, expected, ...)                                       \
        BEGIN                                                                              \
        LANG_SET_ERROR_(lang_ctx, LANG_LEXER_SYNTAX_ERROR, NULL, expected, ##__VA_ARGS__); \
        END

#define SET_PARSER_ERROR_(node, expected, ...)                                              \
        BEGIN                                                                               \
        LANG_SET_ERROR_(lang_ctx, LANG_PARSER_SYNTAX_ERROR, node, expected, ##__VA_ARGS__); \
        END

#ifdef FRONTEND
#define IDENTIFIER_(id_index) LangIdentifierNodeCtor    (lang_ctx, (name_index))
#endif /* FRONTEND */

#define OPERATOR_(op_code)    LangOperatorNodeCtor      (lang_ctx, (op_code ), NULL, NULL)
#define NUMBER_(number)       LangNumberNodeCtor        (lang_ctx, (number  ))

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

#define ADD_(l, r)          LangOperatorNodeCtor(lang_ctx, OP_ADD, (l),  (r))
#define SUB_(l, r)          LangOperatorNodeCtor(lang_ctx, OP_SUB, (l),  (r))
#define MUL_(l, r)          LangOperatorNodeCtor(lang_ctx, OP_MUL, (l),  (r))
#define DIV_(l, r)          LangOperatorNodeCtor(lang_ctx, OP_DIV, (l),  (r))
#define POW_(l, r)          LangOperatorNodeCtor(lang_ctx, OP_POW, (l),  (r))
#define UNARY_(oper, r)     LangOperatorNodeCtor(lang_ctx, (oper), NULL, (r))

//==========================================================================================

#define SRC_PRINT_TABS_()                                                \
        BEGIN                                                            \
        fwprintf(lang_ctx->output_file, L"%*s", 4*lang_ctx->tabs, L" "); \
        END

#define SRC_PRINT_(...)                                                  \
        BEGIN                                                            \
        fwprintf(lang_ctx->output_file, ##__VA_ARGS__);                  \
        END

#define SRC_PRINT_ID_(node__)                                                   \
        BEGIN                                                                   \
        SRC_PRINT_(L"%ls", lang_ctx->names_pool.data[(node__)->data.value.id]); \
        END

#define SRC_PRINT_OP_(opcode_)                              \
        BEGIN                                               \
        SRC_PRINT_(L"%ls", OP_CASES_TABLE[(opcode_)].name); \
        END

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

//==========================================================================================

#undef    _DSL_UNDEF_
#endif /* _DSL_UNDEF_*/
