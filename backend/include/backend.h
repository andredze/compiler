#ifndef BACKEND_H
#define BACKEND_H

//——————————————————————————————————————————————————————————————————————————————————————————

#include "common.h"
#include "common.h"
#include "tree_types.h"
#include "lang_ctx.h"
#include "id_types.h"
#include "bin_code_buffer.h"

//——————————————————————————————————————————————————————————————————————————————————————————

const size_t TYPE_INT_SIZE_IN_BYTES = 4;

//——————————————————————————————————————————————————————————————————————————————————————————

typedef struct BackendCtx
{
    char        ast_file_name[MAX_FILENAME_LEN];

    FILE*       asm_file;
    FILE*       elf_file;

    LangCtx_t   lang_ctx;

    size_t      endif_labels_count;
    size_t      while_labels_count;

    BinCode_t   bin_code;
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
    BACKEND_LANG_ERROR,

    BACKEND_INVALID_OPCODE,
    BACKEND_INVALID_OPERAND_TYPE,

    BACKEND_INVALID_INSTRUCTION,
    BACKEND_BINCODE_BUFFER_ERROR,

    BACKEND_CREATE_INSTRUCTION_ERROR
}
BackendErr_t;

//——————————————————————————————————————————————————————————————————————————————————————————

BackendErr_t BackendCtxCtor    (BackendCtx_t* backend_ctx);
BackendErr_t BackendCtxDtor    (BackendCtx_t* backend_ctx);
BackendErr_t BackendOpenAsmFile(BackendCtx_t* backend_ctx);

//——————————————————————————————————————————————————————————————————————————————————————————

#endif /* BACKEND_H */