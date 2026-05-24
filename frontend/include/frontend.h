#ifndef FRONTEND_H
#define FRONTEND_H

//——————————————————————————————————————————————————————————————————————————————————————————

#include "lang_funcs.h"
#include "lang_ctx.h"

//——————————————————————————————————————————————————————————————————————————————————————————

const size_t TOKENS_STACK_INIT_CAPACITY = 256;

//——————————————————————————————————————————————————————————————————————————————————————————

typedef struct FrontendCtx
{
    LangErrorInfo_t error_info;

    wchar_t*        cur_symbol_ptr;
    wchar_t*        buffer;

    size_t          buffer_size;
    size_t          current_line;

    Stack_t         tokens;
    size_t          cur_token_index;

    FILE*           output_file;

    bool            is_in_func;
    int             in_func_vars_count;

    LangCtx_t       lang_ctx;

} FrontendCtx_t;

//——————————————————————————————————————————————————————————————————————————————————————————

void FrontendSetError(FrontendCtx_t*   frontend_ctx,
                      LangErrorInfo_t* error_info,
                      const wchar_t*   message,
                      ...);

LangErr_t   FrontendCtxCtor         (FrontendCtx_t* frontend_ctx);
void        FrontendCtxDtor         (FrontendCtx_t* frontend_ctx);
void        FrontendPrintError      (FrontendCtx_t* frontend_ctx);
void        FrontendPrintNode       (FrontendCtx_t* frontend_ctx, TreeNode_t* node);
TreeNode_t* FrontendGetCurrentToken (FrontendCtx_t* frontend_ctx);

LangErr_t FrontendRecountIdentifiersIndexes(LangCtx_t* lang_ctx);

//——————————————————————————————————————————————————————————————————————————————————————————
    
#endif /* FRONTEND_H */