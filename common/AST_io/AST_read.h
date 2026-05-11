#ifndef AST_READ_H
#define AST_READ_H

//——————————————————————————————————————————————————————————————————————————————————————————

#include <wchar.h>
#include <wctype.h>
#include "tree_commands.h"
#include "tree_dump.h"
#include "lang_ctx.h"
#include "lang_funcs.h"
#include "io_file.h"

//——————————————————————————————————————————————————————————————————————————————————————————

TreeErr_t TreeReadBufferDump(LangCtx_t* lang_ctx, 
                             const char* cur_symbol_ptr,
                             const char* buffer,
                             const char* fmt, ...);

LangErr_t ASTReadData   (LangCtx_t* lang_ctx, 
                         char*      ast_file_path, 
                         char       src_file_name[MAX_FILENAME_LEN]);

LangErr_t ReadNode      (LangCtx_t*     lang_ctx, 
                         TreeNode_t**   node, 
                         wchar_t*       buffer, 
                         ssize_t*       pos);

LangErr_t ReadNodeData  (LangCtx_t*     lang_ctx, 
                         wchar_t*       buffer, 
                         ssize_t*       pos, 
                         TokenData_t*   node_data);

//——————————————————————————————————————————————————————————————————————————————————————————

const size_t MAX_BUFFER_LEN     = 128;
const size_t MAX_IDENTIFIER_LEN = 128;

//——————————————————————————————————————————————————————————————————————————————————————————

#endif /* AST_READ_H */
