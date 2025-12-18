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

LangErr_t   LangOpenReverseFile   (LangCtx_t* lang_ctx);

//------------------------------------------------------------------------------------------

LangErr_t LangNamesPoolCtor       (NamesPool_t* names_pool);
void      LangNamesPoolDtor       (NamesPool_t* names_pool);
LangErr_t LangNamesPoolPush       (NamesPool_t* names_pool, const wchar_t* name_buf, size_t* name_index);

//------------------------------------------------------------------------------------------

LangErr_t   LangIdTableCtor       (IdTable_t* id_table);
void        LangIdTableDtor       (IdTable_t* id_table);
LangErr_t   LangIdTablePush       (LangCtx_t* lang_ctx, IdTable_t* id_table,
                                   Identifier_t id, IdType_t type, size_t n_params);

LangErr_t   LangCheckVariableIsNotFunction(IdTable_t* id_table, Identifier_t id);
LangErr_t   LangIdTableGetAddress         (IdTable_t* id_table, Identifier_t id, int* addr);
bool        LangGetIdInTable              (IdTable_t* id_table, Identifier_t id, size_t* id_index);
bool        LangIdInTable                 (IdTable_t* id_table, Identifier_t id);

//------------------------------------------------------------------------------------------

TreeNode_t* LangGetCurrentToken   (LangCtx_t* lang_ctx);

//——————————————————————————————————————————————————————————————————————————————————————————

const size_t DEFAULT_ID_TABLE_CAPACITY   = 64;
const size_t DEFAULT_NAMES_POOL_CAPACITY = 64;

//——————————————————————————————————————————————————————————————————————————————————————————

#endif /* LANG_FUNCS_H */
