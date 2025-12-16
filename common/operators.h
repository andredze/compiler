#ifndef OPERATOR_H
#define OPERATOR_H

//——————————————————————————————————————————————————————————————————————————————————————————

typedef enum TokenType
{
    TYPE_OP   = 0,
    TYPE_ID   = 1,
    TYPE_NUM  = 2
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
    OP_FUNCTION_DECL_LEFT   = 13,
    OP_FUNCTION_DECL_RIGHT  = 14,
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
    OP_ABORT                = 26
} Operator_t;

//——————————————————————————————————————————————————————————————————————————————————————————

typedef struct OperatorCase
{
    Operator_t     code;

    const char*    code_str;

    int            repeat_times;

    const wchar_t* name;
    size_t         name_len;

    const wchar_t* ast_format;

} OperatorCase_t;

//——————————————————————————————————————————————————————————————————————————————————————————

#define SET_OP_CASE_(code,          repeat_times,   name,                                     ast_format) \
        [(code)] = {(code), #code, (repeat_times), (name), sizeof(name) / sizeof(*name) - 1, (ast_format)}

//------------------------------------------------------------------------------------------

const OperatorCase_t OP_CASES_TABLE[] =
{
//                      code,         repeat_times,         name                  ast_format
    SET_OP_CASE_(OP_NONE                ,   1,   L""                            , L""               ),
    SET_OP_CASE_(OP_CMD_SEPARATOR       ,   1,   L"ЗАМОЛЧИ"                     , L"_END_STATEMENT_"),
    SET_OP_CASE_(OP_BRACKET_OPEN        ,   1,   L"("                           , L""               ),
    SET_OP_CASE_(OP_BRACKET_CLOSE       ,   1,   L")"                           , L""               ),
    SET_OP_CASE_(OP_ASSIGNMENT          ,   1,   L"пересади в"                  , L"_ASSIGNMENT_"   ),
    SET_OP_CASE_(OP_IF_LHS              ,   1,   L"ты думаешь"                  , L"_IF_"           ), // алгоритмы маркова
    SET_OP_CASE_(OP_IF_RHS              ,   1,   L"сможет что-то изменить?"     , L""               ), // некоторые токены включают проверку
    SET_OP_CASE_(OP_ELSE                ,   1,   L"не верь им"                  , L"_ELSE_"         ), // никогда не / запрещаю / нисколько -> не
    SET_OP_CASE_(OP_WHILE               ,   5,   L"снова и снова"               , L"_WHILE_"        ),
    SET_OP_CASE_(OP_BLOCK_BEGIN         ,   5,   L"ЗАТКНИСЬ"                    , L""               ),
    SET_OP_CASE_(OP_BLOCK_END           ,   4,   L"ОНИ СМОТРЯТ"                 , L""               ), // сжимать все пробелы до 1 либо совмещать 2 токена в 1 на этапе лексики
    SET_OP_CASE_(OP_FUNCTION_BLOCK_BEGIN,   3,   L"за что?"                     , L""               ),
    SET_OP_CASE_(OP_FUNCTION_BLOCK_END  ,   1,   L"нет нет нет нет все кончено?", L""               ),
    SET_OP_CASE_(OP_FUNCTION_DECL_LEFT  ,   1,   L"за"                          , L""               ),
    SET_OP_CASE_(OP_FUNCTION_DECL_RIGHT ,   1,   L"отомсти"                     , L""               ),
    SET_OP_CASE_(OP_FUNCTION_CALL_LHS   ,   1,   L"ритуал: во имя"              , L""               ),
    SET_OP_CASE_(OP_FUNCTION_CALL_RHS   ,   1,   L"принеси в жертву"            , L""               ),
    SET_OP_CASE_(OP_PARAMS_SEPARATOR    ,   1,   L"и"                           , L""               ),
    SET_OP_CASE_(OP_RETURN              ,   1,   L"оставь"                      , L""               ),
    SET_OP_CASE_(OP_ADD                 ,   1,   L"нарастить на"                , L"_MATH_ADD_"     ),
    SET_OP_CASE_(OP_SUB                 ,   1,   L"избавить от"                 , L"_MATH_SUB_"     ),
    SET_OP_CASE_(OP_MUL                 ,   1,   L"усилить в"                   , L"_MATH_MUL_"     ),
    SET_OP_CASE_(OP_DIV                 ,   1,   L"расщепить на"                , L"_MATH_DIV_"     ),
    SET_OP_CASE_(OP_POW                 ,   1,   L"расплодить в"                , L"_MATH_POW_"     ),
    SET_OP_CASE_(OP_OUTPUT              ,   1,   L"заставь их услышать"         , L""               ),
    SET_OP_CASE_(OP_INPUT               ,   1,   L"скажи мне кто ты,"           , L""               ),
    SET_OP_CASE_(OP_ABORT               ,   1,   L"аборт"                       , L""               )  // hlt
};

//------------------------------------------------------------------------------------------

#undef SET_OP_CASE_

//——————————————————————————————————————————————————————————————————————————————————————————

const size_t OPERATORS_COUNT       = sizeof(OP_CASES_TABLE) / sizeof(OP_CASES_TABLE[0]);
const size_t MAX_OPERATOR_NAME_LEN = 128;

//——————————————————————————————————————————————————————————————————————————————————————————

#endif /* OPERATOR_H */
