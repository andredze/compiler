#ifndef OPERATOR_H
#define OPERATOR_H

//——————————————————————————————————————————————————————————————————————————————————————————

typedef enum TokenType
{
    TYPE_OP        = 0,
    TYPE_ID        = 1,
    TYPE_NUM       = 2,

    TYPE_VAR       = 3,
    TYPE_VAR_DECL  = 4,

    TYPE_FUNC_CALL = 5,

    TYPE_FUNC_DECL = 6

} TokenType_t;

//——————————————————————————————————————————————————————————————————————————————————————————

typedef enum Operator
{
    OP_NONE                 = 0 ,
    OP_CMD_SEPARATOR        = 1 ,
    OP_BRACKET_OPEN         = 2 ,
    OP_BRACKET_CLOSE        = 3 ,
    OP_ASSIGNMENT           = 4 ,
    OP_IF_LHS               = 5 ,
    OP_IF_RHS               = 6 ,
    OP_ELSE                 = 7 ,
    OP_WHILE                = 8 ,
    OP_BLOCK_BEGIN          = 9 ,
    OP_BLOCK_END            = 10,
    OP_FUNCTION_BLOCK_BEGIN = 11,
    OP_FUNCTION_BLOCK_END   = 12,
    OP_FUNCTION_DECL_LHS    = 13,
    OP_FUNCTION_DECL_RHS    = 14,
    OP_FUNCTION_CALL_LHS    = 15,
    OP_FUNCTION_CALL_RHS    = 16,
    OP_PARAMS_SEPARATOR     = 17,
    OP_RETURN               = 18,
    OP_ADD                  = 19,
    OP_SUB                  = 20,
    OP_MUL                  = 21,
    OP_DIV                  = 22,
    OP_POW                  = 23,
    OP_OUTPUT               = 24,
    OP_INPUT                = 25,
    OP_ABORT                = 26,
    OP_VARIABLE_DECL        = 27,
    OP_EQUAL                = 28,
    OP_BIGGER               = 29,
    OP_SMALLER              = 30,
    OP_SQRT                 = 31
} Operator_t;

//——————————————————————————————————————————————————————————————————————————————————————————

#endif /* OPERATOR_H */
