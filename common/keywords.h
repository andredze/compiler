#ifndef KEYWORD_H
#define KEYWORD_H

//——————————————————————————————————————————————————————————————————————————————————————————

typedef enum TokenType
{
    TYPE_KEYWORD        = 0,
    TYPE_ID        = 1,
    TYPE_NUM       = 2,

    TYPE_VAR       = 3,
    TYPE_VAR_DECL  = 4,

    TYPE_FUNC_CALL = 5,

    TYPE_FUNC_DECL = 6

} TokenType_t;

//——————————————————————————————————————————————————————————————————————————————————————————

typedef enum Keyword
{
    KW_NONE                 = 0 ,
    KW_CMD_SEPARATOR        = 1 ,
    KW_BRACKET_OPEN         = 2 ,
    KW_BRACKET_CLOSE        = 3 ,
    KW_ASSIGNMENT           = 4 ,
    KW_IF_LHS               = 5 ,
    KW_IF_RHS               = 6 ,
    KW_ELSE                 = 7 ,
    KW_WHILE                = 8 ,
    KW_BLOCK_BEGIN          = 9 ,
    KW_BLOCK_END            = 10,
    KW_FUNCTION_BLOCK_BEGIN = 11,
    KW_FUNCTION_BLOCK_END   = 12,
    KW_FUNCTION_DECL_LHS    = 13,
    KW_FUNCTION_DECL_RHS    = 14,
    KW_FUNCTION_CALL_LHS    = 15,
    KW_FUNCTION_CALL_RHS    = 16,
    KW_PARAMS_SEPARATOR     = 17,
    KW_RETURN               = 18,
    KW_ADD                  = 19,
    KW_SUB                  = 20,
    KW_MUL                  = 21,
    KW_DIV                  = 22,
    KW_POW                  = 23,
    KW_OUTPUT               = 24,
    KW_INPUT                = 25,
    KW_ABORT                = 26,
    KW_VARIABLE_DECL        = 27,
    KW_EQUAL                = 28,
    KW_NOT_EQUAL            = 29,
    KW_BIGGER_EQUAL         = 30,
    KW_BIGGER               = 31,
    KW_SMALLER_EQUAL        = 32,
    KW_SMALLER              = 33,
    KW_SQRT                 = 34,
    KW_DRAW                 = 35,
    KW_POINT                = 36,
    KW_INIT_SCREEN          = 37,
    KW_DEL_SCREEN           = 38,
    KW_DRAW_SCREEN          = 39,
    KW_POINT_SCREEN         = 40
} Keyword_t;

//——————————————————————————————————————————————————————————————————————————————————————————

#endif /* KEYWORD_H */
