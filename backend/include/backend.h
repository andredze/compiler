#ifndef BACKEND_H
#define BACKEND_H

//——————————————————————————————————————————————————————————————————————————————————————————

#include "common.h"
#include "common.h"
#include "tree_types.h"
#include "lang_ctx.h"
#include "id_types.h"
#include "bin_code_buffer.h"
#include "backend_err.h"
#include "relocations_table.h"

//——————————————————————————————————————————————————————————————————————————————————————————

const int    TEXT_SECTION_NUMBER    = 1;
const size_t TYPE_INT_SIZE_IN_BYTES = 4;

//——————————————————————————————————————————————————————————————————————————————————————————

typedef struct BackendCtx
{
    char          ast_file_name[MAX_FILENAME_LEN];

    FILE*         asm_file;
    FILE*         elf_file;

    LangCtx_t     lang_ctx;

    size_t        endif_labels_count;
    size_t        while_labels_count;

    BinCode_t     bin_code;

    size_t        current_stack_local_vars_size;

    TreeNode_t*   main_node;

    RelTable_t    rel_table;
}
BackendCtx_t;

//——————————————————————————————————————————————————————————————————————————————————————————

BackendErr_t BackendCtxCtor    (BackendCtx_t* backend_ctx);
BackendErr_t BackendCtxDtor    (BackendCtx_t* backend_ctx);
BackendErr_t BackendOpenAsmFile(BackendCtx_t* backend_ctx);

BackendErr_t BackendGetFuncIdTableIdData(BackendCtx_t* backend_ctx, 
                                         size_t        index, 
                                         IdData_t**    id_data_p);

BackendErr_t BackendGetVarsStackSize(BackendCtx_t* backend_ctx, size_t func_index, size_t* dst);
BackendErr_t BackendGetArgsStackSize(BackendCtx_t* backend_ctx, size_t func_index, size_t* dst);

BackendErr_t BackendGetVariableOffset(BackendCtx_t* backend_ctx, 
                                      size_t        id_index, 
                                      int*          offset);

wchar_t* BackendGetIdName(BackendCtx_t* backend_ctx, TreeNode_t* node);

//——————————————————————————————————————————————————————————————————————————————————————————

#endif /* BACKEND_H */