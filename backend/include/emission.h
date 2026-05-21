#ifndef EMISSION_H
#define EMISSION_H

//——————————————————————————————————————————————————————————————————————————————————————————

#include "backend.h"
#include "instruction.h"

//——————————————————————————————————————————————————————————————————————————————————————————

BackendErr_t EmitProgram            (BackendCtx_t* backend_ctx);
BackendErr_t EmitIf                 (BackendCtx_t* backend_ctx, TreeNode_t* node);
BackendErr_t EmitAssignment         (BackendCtx_t* backend_ctx, TreeNode_t* node);
BackendErr_t EmitMathExprOperation  (BackendCtx_t* backend_ctx, TreeNode_t* node);
BackendErr_t EmitLibFuncCall        (BackendCtx_t* backend_ctx, TreeNode_t* node);
BackendErr_t EmitInput              (BackendCtx_t* backend_ctx, TreeNode_t* node);
// BackendErr_t EmitElse               (BackendCtx_t* backend_ctx, TreeNode_t* node);
BackendErr_t EmitWhile              (BackendCtx_t* backend_ctx, TreeNode_t* node);
BackendErr_t EmitCmdSeparator       (BackendCtx_t* backend_ctx, TreeNode_t* node);
BackendErr_t EmitExit               (BackendCtx_t* backend_ctx, TreeNode_t* node);
BackendErr_t EmitReturn             (BackendCtx_t* backend_ctx, TreeNode_t* node);
BackendErr_t EmitPoint              (BackendCtx_t* backend_ctx, TreeNode_t* node);

//——————————————————————————————————————————————————————————————————————————————————————————

typedef BackendErr_t (*EmitFunction_t) (BackendCtx_t* backend_ctx, TreeNode_t* node);

//——————————————————————————————————————————————————————————————————————————————————————————

const wchar_t * const KW_INPUT_FUNC_NAME        = L"СкажиМнеКтоТы";
const wchar_t * const KW_OUTPUT_FUNC_NAME       = L"ЗаставьИхУслышать";
const wchar_t * const KW_SQRT_FUNC_NAME         = L"Исток";
const wchar_t * const KW_DRAW_FUNC_NAME         = L"Нарисуй";
const wchar_t * const KW_POINT_FUNC_NAME        = L"Поставь";
const wchar_t * const KW_INIT_SCREEN_FUNC_NAME  = L"ИнициализируйЭкран";
const wchar_t * const KW_DEL_SCREEN_FUNC_NAME   = L"УдалиЭкран";
const wchar_t * const KW_DRAW_SCREEN_FUNC_NAME  = L"НарисуйЭкран";
const wchar_t * const KW_POINT_SCREEN_FUNC_NAME = L"ПоставьНаЭкран";

//——————————————————————————————————————————————————————————————————————————————————————————

typedef struct ExternFunctionCase
{
    Keyword_t       keyword;
    const wchar_t*  name;
}
ExternFunctionCase_t;

//------------------------------------------------------------------//

const ExternFunctionCase_t EXTERN_FUNCS_TABLE[] = {
    {KW_INPUT       , L"СкажиМнеКтоТы"      },
    {KW_OUTPUT      , L"ЗаставьИхУслышать"  },
    {KW_SQRT        , L"Исток"              },
    {KW_DRAW        , L"Нарисуй"            },
    {KW_POINT       , L"Поставь"            },
    {KW_INIT_SCREEN , L"ИнициализируйЭкран" },
    {KW_DEL_SCREEN  , L"УдалиЭкран"         },
    {KW_DRAW_SCREEN , L"НарисуйЭкран"       },
    {KW_POINT_SCREEN, L"ПоставьНаЭкран"     }
};

//------------------------------------------------------------------//

const size_t EXTERN_FUNCS_COUNT = sizeof(EXTERN_FUNCS_TABLE) / sizeof(EXTERN_FUNCS_TABLE[0]);

//——————————————————————————————————————————————————————————————————————————————————————————

const Register_t CALL_REGS_TABLE[] = {
    REG_RDI, REG_RSI, REG_RDX, 
    REG_RCX, REG_R8,  REG_R9
};

const size_t CALL_ARGS_REGS_COUNT = sizeof(CALL_REGS_TABLE) / sizeof(CALL_REGS_TABLE[0]);

//——————————————————————————————————————————————————————————————————————————————————————————

#endif /* EMISSION_H */