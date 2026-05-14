#ifndef KEYWORD_CASES_H
#define KEYWORD_CASES_H

//——————————————————————————————————————————————————————————————————————————————————————————

#include "keywords.h"
#include "lang_ctx.h"

#ifdef BACKEND
    #include "emission.h"
#endif /* BACKEND */

//——————————————————————————————————————————————————————————————————————————————————————————

typedef struct KeywordCase
{
    Keyword_t      code;

    const char*    code_str;

    int            repeat_times;

    const wchar_t* name;
    size_t         name_len;

    const wchar_t* ast_format;

#ifdef BACKEND
    BackendErr_t (*emit_function) (BackendCtx_t*, TreeNode_t*);
#endif /* BACKEND */

#ifdef REVERSE
    LangErr_t (*src_function) (LangCtx_t*, TreeNode_t*);
#endif /* REVERSE */

} KeywordCase_t;

//——————————————————————————————————————————————————————————————————————————————————————————

#ifdef REVERSE

LangErr_t SrcNode(LangCtx_t* lang_ctx, TreeNode_t* node);

LangErr_t SrcCmdSeparator   (LangCtx_t* lang_ctx, TreeNode_t* node);
LangErr_t SrcAssignment     (LangCtx_t* lang_ctx, TreeNode_t* node);
LangErr_t SrcWhile          (LangCtx_t* lang_ctx, TreeNode_t* node);
LangErr_t SrcIf             (LangCtx_t* lang_ctx, TreeNode_t* node);
LangErr_t SrcAbort          (LangCtx_t* lang_ctx, TreeNode_t* node);
LangErr_t SrcMathOperation  (LangCtx_t* lang_ctx, TreeNode_t* node);
LangErr_t SrcUnaryOperator  (LangCtx_t* lang_ctx, TreeNode_t* node);
LangErr_t SrcCompareOperator(LangCtx_t* lang_ctx, TreeNode_t* node);
LangErr_t SrcParamsSeparator(LangCtx_t* lang_ctx, TreeNode_t* node);

#endif /* REVERSE */

//——————————————————————————————————————————————————————————————————————————————————————————

#ifdef BACKEND

#define SET_KW_CASE_(code,          repeat_times,   name,                                     ast_format,                                                 emit_function,  src_function) \
        [(code)] = {(code), #code, (repeat_times), (name), sizeof(name) / sizeof(*name) - 1, (ast_format), (BackendErr_t (*)(BackendCtx_t*, TreeNode_t*))(emit_function),              }

#elif defined(REVERSE)

#define SET_KW_CASE_(code,          repeat_times,   name,                                     ast_format,   emit_function,                          src_function) \
        [(code)] = {(code), #code, (repeat_times), (name), sizeof(name) / sizeof(*name) - 1, (ast_format), (LangErr_t (*)(LangCtx_t*, TreeNode_t*))(src_function)}

#elif defined(FRONTEND)

#define SET_KW_CASE_(code,          repeat_times,   name,                                     ast_format, asm_function, src_function) \
        [(code)] = {(code), #code, (repeat_times), (name), sizeof(name) / sizeof(*name) - 1, (ast_format) }

#else

#define SET_KW_CASE_(code, repeat_times, name, ast_format, asm_function, src_function) ;

#endif /* FRONTEND */

//==========================================================================================

const KeywordCase_t KEYWORD_CASES_TABLE[] =
{
//                      code,         repeat_times,         name                  ast_format               emit_function           src_function
    SET_KW_CASE_(KW_NONE                ,   1,   L""                            , L""                , NULL                   , NULL              ),
    SET_KW_CASE_(KW_CMD_SEPARATOR       ,   1,   L"ЗАМОЛЧИ"                     , L"_END_STATEMENT_" , EmitCmdSeparator       , SrcCmdSeparator   ),
    SET_KW_CASE_(KW_BRACKET_OPEN        ,   1,   L"("                           , L""                , NULL                   , NULL              ),
    SET_KW_CASE_(KW_BRACKET_CLOSE       ,   1,   L")"                           , L""                , NULL                   , NULL              ),
    SET_KW_CASE_(KW_ASSIGNMENT          ,   1,   L"пересади в"                  , L"_ASSIGNMENT_"    , EmitAssignment         , SrcAssignment     ),
    SET_KW_CASE_(KW_IF_LHS              ,   1,   L"ты думаешь"                  , L"_IF_"            , EmitIf                 , SrcIf             ), // алгоритмы маркова
    SET_KW_CASE_(KW_IF_RHS              ,   1,   L"сможет что-то изменить?"     , L""                , NULL                   , NULL              ), // некоторые токены включают проверку
    SET_KW_CASE_(KW_ELSE                ,   1,   L"не верь им"                  , L"_ELSE_"          , NULL                   , NULL              ), // никогда не / запрещаю / нисколько -> не
    SET_KW_CASE_(KW_WHILE               ,   5,   L"снова и снова"               , L"_WHILE_"         , EmitWhile              , SrcWhile          ),
    SET_KW_CASE_(KW_BLOCK_BEGIN         ,   5,   L"ЗАТКНИСЬ"                    , L""                , NULL                   , NULL              ),
    SET_KW_CASE_(KW_BLOCK_END           ,   4,   L"ОНИ СМОТРЯТ"                 , L""                , NULL                   , NULL              ), // сжимать все пробелы до 1 либо совмещать 2 токена в 1 на этапе лексики
    SET_KW_CASE_(KW_FUNCTION_BLOCK_BEGIN,   3,   L"за что?"                     , L""                , NULL                   , NULL              ),
    SET_KW_CASE_(KW_FUNCTION_BLOCK_END  ,   1,   L"нет нет нет нет все кончено?", L""                , NULL                   , NULL              ),
    SET_KW_CASE_(KW_FUNCTION_DECL_LHS   ,   1,   L"за"                          , L""                , NULL                   , NULL              ),
    SET_KW_CASE_(KW_FUNCTION_DECL_RHS   ,   1,   L"отомсти"                     , L""                , NULL                   , NULL              ),
    SET_KW_CASE_(KW_FUNCTION_CALL_LHS   ,   1,   L"ритуал: во имя"              , L""                , NULL                   , NULL              ),
    SET_KW_CASE_(KW_FUNCTION_CALL_RHS   ,   1,   L"принеси в жертву"            , L""                , NULL                   , NULL              ),
    SET_KW_CASE_(KW_PARAMS_SEPARATOR    ,   1,   L"и"                           , L"_COMMA_"         , EmitParamsSeparator    , SrcParamsSeparator),
    SET_KW_CASE_(KW_RETURN              ,   1,   L"оставь"                      , L"_RETURN_"        , EmitReturn             , SrcUnaryOperator  ),
    SET_KW_CASE_(KW_ADD                 ,   1,   L"нарастить на"                , L"_MATH_ADD_"      , EmitMathOperation      , SrcMathOperation  ),
    SET_KW_CASE_(KW_SUB                 ,   1,   L"избавить от"                 , L"_MATH_SUB_"      , EmitMathOperation      , SrcMathOperation  ),
    SET_KW_CASE_(KW_MUL                 ,   1,   L"усилить в"                   , L"_MATH_MUL_"      , EmitMathOperation      , SrcMathOperation  ),
    SET_KW_CASE_(KW_DIV                 ,   1,   L"расщепить на"                , L"_MATH_DIV_"      , EmitMathOperation      , SrcMathOperation  ),
    SET_KW_CASE_(KW_POW                 ,   1,   L"расплодить в"                , L"_MATH_POW_"      , EmitMathOperation      , SrcMathOperation  ),
    SET_KW_CASE_(KW_OUTPUT              ,   1,   L"заставь их услышать"         , L"_OUTPUT_"        , EmitUnaryOperation     , SrcUnaryOperator  ),
    SET_KW_CASE_(KW_INPUT               ,   1,   L"скажи мне кто ты,"           , L"_INPUT_"         , EmitInput              , SrcUnaryOperator  ),
    SET_KW_CASE_(KW_ABORT               ,   1,   L"аборт"                       , L"_ABORT_"         , EmitHlt                , SrcAbort          ),
    SET_KW_CASE_(KW_VARIABLE_DECL       ,   1,   L"голос в голове"              , L""                , NULL                   , NULL              ),
    SET_KW_CASE_(KW_EQUAL               ,   1,   L"такой же как"                , L"_LOG_EQUAL_"     , NULL                   , SrcCompareOperator),
    SET_KW_CASE_(KW_NOT_EQUAL           ,   1,   L"противоположен"              , L"_LOG_NOT_EQUAL_" , NULL                   , SrcCompareOperator),
    SET_KW_CASE_(KW_BIGGER_EQUAL        ,   1,   L"больше-равен"                , L"_LOG_MORE_EQUAL_", NULL                   , SrcCompareOperator),
    SET_KW_CASE_(KW_BIGGER              ,   1,   L"больше"                      , L"_LOG_MORE_"      , NULL                   , SrcCompareOperator),
    SET_KW_CASE_(KW_SMALLER_EQUAL       ,   1,   L"меньше-равен"                , L"_LOG_LESS_EQUAL_", NULL                   , SrcCompareOperator),
    SET_KW_CASE_(KW_SMALLER             ,   1,   L"меньше"                      , L"_LOG_LESS_"      , NULL                   , SrcCompareOperator),
    SET_KW_CASE_(KW_SQRT                ,   1,   L"исток"                       , L"_MATH_SQRT_"     , EmitUnaryOperation     , SrcUnaryOperator  ),
    SET_KW_CASE_(KW_DRAW                ,   1,   L"рисуй"                       , L"_DRAW_"          , EmitUnaryOperation     , SrcUnaryOperator  ),
    SET_KW_CASE_(KW_POINT               ,   1,   L"поставь точку"               , L"_POINT_"         , EmitPoint              , SrcUnaryOperator  )
};

//==========================================================================================

#undef SET_KW_CASE_

//——————————————————————————————————————————————————————————————————————————————————————————

const size_t KEYWORDS_COUNT       = sizeof(KEYWORD_CASES_TABLE) / sizeof(KEYWORD_CASES_TABLE[0]);
const size_t MAX_KEYWORD_NAME_LEN = 128;

//——————————————————————————————————————————————————————————————————————————————————————————

#endif /* KEYWORD_CASES_H */
