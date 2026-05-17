#ifndef EMISSION_H
#define EMISSION_H

//——————————————————————————————————————————————————————————————————————————————————————————

#include "backend.h"

//——————————————————————————————————————————————————————————————————————————————————————————

BackendErr_t EmitProgram            (BackendCtx_t* backend_ctx);
BackendErr_t EmitIf                 (BackendCtx_t* backend_ctx, TreeNode_t* node);
BackendErr_t EmitAssignment         (BackendCtx_t* backend_ctx, TreeNode_t* node);
BackendErr_t EmitMathExprOperation  (BackendCtx_t* backend_ctx, TreeNode_t* node);
BackendErr_t EmitUnaryOperation     (BackendCtx_t* backend_ctx, TreeNode_t* node);
BackendErr_t EmitInput              (BackendCtx_t* backend_ctx, TreeNode_t* node);
// BackendErr_t EmitElse               (BackendCtx_t* backend_ctx, TreeNode_t* node);
BackendErr_t EmitWhile              (BackendCtx_t* backend_ctx, TreeNode_t* node);
BackendErr_t EmitCmdSeparator       (BackendCtx_t* backend_ctx, TreeNode_t* node);
BackendErr_t EmitExit               (BackendCtx_t* backend_ctx, TreeNode_t* node);
BackendErr_t EmitReturn             (BackendCtx_t* backend_ctx, TreeNode_t* node);

//——————————————————————————————————————————————————————————————————————————————————————————

typedef BackendErr_t (*EmitFunction_t) (BackendCtx_t* backend_ctx, TreeNode_t* node);

//——————————————————————————————————————————————————————————————————————————————————————————

const int             UNDEFINED_FUNC_ADDR = 0;

const wchar_t * const KW_INPUT_FUNC_NAME  = L"скажи_мне_кто_ты";
const wchar_t * const KW_OUTPUT_FUNC_NAME = L"my_print";
const wchar_t * const KW_SQRT_FUNC_NAME   = L"исток";
const wchar_t * const KW_DRAW_FUNC_NAME   = L"нарисуй";
const wchar_t * const KW_POINT_FUNC_NAME  = L"поставь";

//——————————————————————————————————————————————————————————————————————————————————————————

#endif /* EMISSION_H */