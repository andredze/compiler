#ifndef EMISSION_H
#define EMISSION_H

//——————————————————————————————————————————————————————————————————————————————————————————

#include "backend.h"

//——————————————————————————————————————————————————————————————————————————————————————————

BackendErr_t EmitProgram            (BackendCtx_t* backend_ctx);
    
BackendErr_t EmitNode               (BackendCtx_t* backend_ctx, TreeNode_t* node);
BackendErr_t EmitIf                 (BackendCtx_t* backend_ctx, TreeNode_t* node);
BackendErr_t EmitAssignment         (BackendCtx_t* backend_ctx, TreeNode_t* node);
BackendErr_t EmitMathExprOperation  (BackendCtx_t* backend_ctx, TreeNode_t* node);
BackendErr_t EmitUnaryOperation     (BackendCtx_t* backend_ctx, TreeNode_t* node);
BackendErr_t EmitInput              (BackendCtx_t* backend_ctx, TreeNode_t* node);
// BackendErr_t EmitElse               (BackendCtx_t* backend_ctx, TreeNode_t* node);
BackendErr_t EmitWhile              (BackendCtx_t* backend_ctx, TreeNode_t* node);
BackendErr_t EmitCmdSeparator       (BackendCtx_t* backend_ctx, TreeNode_t* node);
BackendErr_t EmitParamsSeparator    (BackendCtx_t* backend_ctx, TreeNode_t* node);
BackendErr_t EmitHlt                (BackendCtx_t* backend_ctx, TreeNode_t* node);
BackendErr_t EmitReturn             (BackendCtx_t* backend_ctx, TreeNode_t* node);
BackendErr_t EmitPoint              (BackendCtx_t* backend_ctx, TreeNode_t* node);

//——————————————————————————————————————————————————————————————————————————————————————————

typedef BackendErr_t (*EmitFunction_t) (BackendCtx_t* backend_ctx, TreeNode_t* node);

//——————————————————————————————————————————————————————————————————————————————————————————

#endif /* EMISSION_H */