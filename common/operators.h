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
    OP_NONE                 = 0,
    OP_CMD_SEPARATOR        = 1,
    OP_BRACKET_OPEN         = 2,
    OP_BRACKET_CLOSE        = 3,
    OP_ASSIGNMENT           = 4,
    OP_IF_LEFT              = 5,
    OP_IF_RIGHT             = 6,
    OP_ELSE                 = 7,
    OP_WHILE                = 8,
    OP_BLOCK_BEGIN          = 9,
    OP_BLOCK_END            = 10,
    OP_FUNCTION_BLOCK_BEGIN = 11,
    OP_FUNCTION_BLOCK_END   = 12,
    OP_FUNCTION_DECL_LEFT   = 13,
    OP_FUNCTION_DECL_RIGHT  = 14,
    OP_FUNCTION_CALL_LEFT   = 15,
    OP_FUNCTION_CALL_RIGHT  = 16,
    OP_PARAMS_SEPARATOR     = 17,
    OP_RETURN               = 18,
    OP_ADD                  = 19,
    OP_SUB                  = 20,
    OP_MUL                  = 21,
    OP_DIV                  = 22,
    OP_POW_LEFT             = 23,
    OP_POW_RIGHT            = 24,
    OP_OUTPUT               = 25,
    OP_INPUT                = 26,
    OP_ABORT                = 27
} Operator_t;

//——————————————————————————————————————————————————————————————————————————————————————————

typedef struct OperatorCase
{
    Operator_t     code;

    const char*    code_str;

    int            repeat_times;

    const wchar_t* name;
    size_t         name_len;

} OperatorCase_t;

//——————————————————————————————————————————————————————————————————————————————————————————

#define SET_OP_CASE_(code,          repeat_times,   name                                      ) \
        [(code)] = {(code), #code, (repeat_times), (name), sizeof(name) / sizeof(wchar_t) - 1 }

//------------------------------------------------------------------------------------------

const OperatorCase_t OP_CASES_TABLE[] =
{
//                      code,         repeat_times,         name
    SET_OP_CASE_(OP_NONE                ,   1,   NULL                            ),
    SET_OP_CASE_(OP_CMD_SEPARATOR       ,   1,   L"ЗАМОЛЧИ"                      ),
    SET_OP_CASE_(OP_BRACKET_OPEN        ,   1,   L"("                            ),
    SET_OP_CASE_(OP_BRACKET_CLOSE       ,   1,   L")"                            ),
    SET_OP_CASE_(OP_ASSIGNMENT          ,   1,   L"пересади в"                   ),
    SET_OP_CASE_(OP_IF_LEFT             ,   1,   L"ты думаешь"                   ),
    SET_OP_CASE_(OP_IF_RIGHT            ,   1,   L"сможет что-то изменить?"      ),
    SET_OP_CASE_(OP_ELSE                ,   1,   L"не верь им"                   ),
    SET_OP_CASE_(OP_WHILE               ,   5,   L"снова и снова"                ),
    SET_OP_CASE_(OP_BLOCK_BEGIN         ,   5,   L"ЗАТКНИСЬ"                     ),
    SET_OP_CASE_(OP_BLOCK_END           ,   4,   L"ОНИ СМОТРЯТ"                  ),
    SET_OP_CASE_(OP_FUNCTION_BLOCK_BEGIN,   3,   L"за что?"                      ),
    SET_OP_CASE_(OP_FUNCTION_BLOCK_END  ,   1,   L"нет нет нет нет все кончено?" ),
    SET_OP_CASE_(OP_FUNCTION_DECL_LEFT  ,   1,   L"за"                           ),
    SET_OP_CASE_(OP_FUNCTION_DECL_RIGHT ,   1,   L"отомсти"                      ),
    SET_OP_CASE_(OP_FUNCTION_CALL_LEFT  ,   1,   L"ритуал: во имя"               ),
    SET_OP_CASE_(OP_FUNCTION_CALL_RIGHT ,   1,   L"принеси в жертву"             ),
    SET_OP_CASE_(OP_PARAMS_SEPARATOR    ,   1,   L"и"                            ),
    SET_OP_CASE_(OP_RETURN              ,   1,   L"оставь"                       ),
    SET_OP_CASE_(OP_ADD                 ,   1,   L"нарастить на"                 ),
    SET_OP_CASE_(OP_SUB                 ,   1,   L"избавить от"                  ),
    SET_OP_CASE_(OP_MUL                 ,   1,   L"усилить в"                    ),
    SET_OP_CASE_(OP_DIV                 ,   1,   L"расщепить на"                 ),
    SET_OP_CASE_(OP_POW_LEFT            ,   1,   L"расплодить в"                 ),
    SET_OP_CASE_(OP_POW_RIGHT           ,   1,   L"раз"                          ),
    SET_OP_CASE_(OP_OUTPUT              ,   1,   L"заставь их услышать"          ),
    SET_OP_CASE_(OP_INPUT               ,   1,   L"скажи мне кто ты,"            ),
    SET_OP_CASE_(OP_ABORT               ,   1,   L"аборт"                        )  // hlt
};

//------------------------------------------------------------------------------------------

#undef SET_OP_CASE_

//——————————————————————————————————————————————————————————————————————————————————————————

const size_t OPERATORS_COUNT       = sizeof(OP_CASES_TABLE) / sizeof(OP_CASES_TABLE[0]);
const size_t MAX_OPERATOR_NAME_LEN = 128;

//——————————————————————————————————————————————————————————————————————————————————————————

#endif /* OPERATOR_H */
