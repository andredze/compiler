#ifndef BACKEND_H
#define BACKEND_H

//——————————————————————————————————————————————————————————————————————————————————————————

#include "common.h"
#include "common.h"
#include "tree_types.h"
#include "lang_ctx.h"
#include "id_types.h"

//——————————————————————————————————————————————————————————————————————————————————————————

const size_t TYPE_INT_SIZE_IN_BYTES = 4;

//——————————————————————————————————————————————————————————————————————————————————————————

typedef struct Backend
{
    char        ast_file_name[MAX_FILENAME_LEN];
    FILE*       asm_file;

    LangCtx_t   lang_ctx;

    size_t      endif_labels_count;
    size_t      while_labels_count;
}
BackendCtx_t;

//——————————————————————————————————————————————————————————————————————————————————————————

typedef enum BackendErr
{
    BACKEND_SUCCESS,
    BACKEND_INVALID_AST_INPUT,
    BACKEND_CANT_EMIT_OPERATOR,
    BACKEND_UNKNOWN_TOKEN_TYPE,
    BACKEND_FILE_ERROR,
    BACKEND_LANG_ERROR
}
BackendErr_t;

//——————————————————————————————————————————————————————————————————————————————————————————

BackendErr_t BackendCtxCtor    (BackendCtx_t* backend_ctx);
BackendErr_t BackendCtxDtor    (BackendCtx_t* backend_ctx);
BackendErr_t BackendOpenAsmFile(BackendCtx_t* backend_ctx);

//——————————————————————————————————————————————————————————————————————————————————————————

#endif /* BACKEND_H */