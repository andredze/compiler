#ifndef OP_CASES_H
#define OP_CASES_H

//——————————————————————————————————————————————————————————————————————————————————————————

#include "operators.h"
#include "lang_ctx.h"

//——————————————————————————————————————————————————————————————————————————————————————————

typedef struct OperatorCase
{
    Operator_t     code;

    const char*    code_str;

    int            repeat_times;

    const wchar_t* name;
    size_t         name_len;

    const wchar_t* ast_format;

#ifdef BACKEND
    enum LangErr (*asm_function) (struct LangCtx*);
#endif /* BACKEND */

} OperatorCase_t;

//——————————————————————————————————————————————————————————————————————————————————————————

#ifdef BACKEND

#define SET_OP_CASE_(code,          repeat_times,   name,                                     ast_format,                                      asm_function) \
        [(code)] = {(code), #code, (repeat_times), (name), sizeof(name) / sizeof(*name) - 1, (ast_format), (enum LangErr (*)(struct LangCtx*))(asm_function)}

#else

#define SET_OP_CASE_(code,          repeat_times,   name,                                     ast_format,   asm_function) \
        [(code)] = {(code), #code, (repeat_times), (name), sizeof(name) / sizeof(*name) - 1, (ast_format)               }

#endif /* BACKEND */

//------------------------------------------------------------------------------------------

const OperatorCase_t OP_CASES_TABLE[] =
{
//                      code,         repeat_times,         name                  ast_format       asm_function
    SET_OP_CASE_(OP_NONE                ,   1,   L""                            , L""               , NULL),
    SET_OP_CASE_(OP_CMD_SEPARATOR       ,   1,   L"ЗАМОЛЧИ"                     , L"_END_STATEMENT_", NULL),
    SET_OP_CASE_(OP_BRACKET_OPEN        ,   1,   L"("                           , L""               , NULL),
    SET_OP_CASE_(OP_BRACKET_CLOSE       ,   1,   L")"                           , L""               , NULL),
    SET_OP_CASE_(OP_ASSIGNMENT          ,   1,   L"пересади в"                  , L"_ASSIGNMENT_"   , NULL),
    SET_OP_CASE_(OP_IF_LHS              ,   1,   L"ты думаешь"                  , L"_IF_"           , NULL), // алгоритмы маркова
    SET_OP_CASE_(OP_IF_RHS              ,   1,   L"сможет что-то изменить?"     , L""               , NULL), // некоторые токены включают проверку
    SET_OP_CASE_(OP_ELSE                ,   1,   L"не верь им"                  , L"_ELSE_"         , NULL), // никогда не / запрещаю / нисколько -> не
    SET_OP_CASE_(OP_WHILE               ,   5,   L"снова и снова"               , L"_WHILE_"        , NULL),
    SET_OP_CASE_(OP_BLOCK_BEGIN         ,   5,   L"ЗАТКНИСЬ"                    , L""               , NULL),
    SET_OP_CASE_(OP_BLOCK_END           ,   4,   L"ОНИ СМОТРЯТ"                 , L""               , NULL), // сжимать все пробелы до 1 либо совмещать 2 токена в 1 на этапе лексики
    SET_OP_CASE_(OP_FUNCTION_BLOCK_BEGIN,   3,   L"за что?"                     , L""               , NULL),
    SET_OP_CASE_(OP_FUNCTION_BLOCK_END  ,   1,   L"нет нет нет нет все кончено?", L""               , NULL),
    SET_OP_CASE_(OP_FUNCTION_DECL_LEFT  ,   1,   L"за"                          , L""               , NULL),
    SET_OP_CASE_(OP_FUNCTION_DECL_RIGHT ,   1,   L"отомсти"                     , L""               , NULL),
    SET_OP_CASE_(OP_FUNCTION_CALL_LHS   ,   1,   L"ритуал: во имя"              , L""               , NULL),
    SET_OP_CASE_(OP_FUNCTION_CALL_RHS   ,   1,   L"принеси в жертву"            , L""               , NULL),
    SET_OP_CASE_(OP_PARAMS_SEPARATOR    ,   1,   L"и"                           , L""               , NULL),
    SET_OP_CASE_(OP_RETURN              ,   1,   L"оставь"                      , L""               , NULL),
    SET_OP_CASE_(OP_ADD                 ,   1,   L"нарастить на"                , L"_MATH_ADD_"     , NULL),
    SET_OP_CASE_(OP_SUB                 ,   1,   L"избавить от"                 , L"_MATH_SUB_"     , NULL),
    SET_OP_CASE_(OP_MUL                 ,   1,   L"усилить в"                   , L"_MATH_MUL_"     , NULL),
    SET_OP_CASE_(OP_DIV                 ,   1,   L"расщепить на"                , L"_MATH_DIV_"     , NULL),
    SET_OP_CASE_(OP_POW                 ,   1,   L"расплодить в"                , L"_MATH_POW_"     , NULL),
    SET_OP_CASE_(OP_OUTPUT              ,   1,   L"заставь их услышать"         , L""               , NULL),
    SET_OP_CASE_(OP_INPUT               ,   1,   L"скажи мне кто ты,"           , L""               , NULL),
    SET_OP_CASE_(OP_ABORT               ,   1,   L"аборт"                       , L""               , NULL)  // hlt
};

//------------------------------------------------------------------------------------------

#undef SET_OP_CASE_

//——————————————————————————————————————————————————————————————————————————————————————————

const size_t OPERATORS_COUNT       = sizeof(OP_CASES_TABLE) / sizeof(OP_CASES_TABLE[0]);
const size_t MAX_OPERATOR_NAME_LEN = 128;

//——————————————————————————————————————————————————————————————————————————————————————————

#endif /* OP_CASES_H */
