#ifndef LANG_FUNCS_H
#define LANG_FUNCS_H

//——————————————————————————————————————————————————————————————————————————————————————————

#include "lang_ctx.h"
#include "tree_commands.h"
#include "stack.h"
#include <wchar.h>

//——————————————————————————————————————————————————————————————————————————————————————————

LangErr_t   LangCtxCtor           (LangCtx_t* lang_ctx);
void        LangCtxDtor           (LangCtx_t* lang_ctx);

LangErr_t   LangOpenAsmFile       (LangCtx_t* lang_ctx);

LangErr_t   LangIdTableCtor       (IdTable_t* id_table);
void        LangIdTableDtor       (IdTable_t* id_table);
LangErr_t   LangIdTablePush       (LangCtx_t* lang_ctx, const wchar_t* id_name_buf, size_t* id_index);

TreeNode_t* LangGetCurrentToken   (LangCtx_t* lang_ctx);

//——————————————————————————————————————————————————————————————————————————————————————————

#endif /* LANG_FUNCS_H */
