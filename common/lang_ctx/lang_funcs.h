#ifndef LANG_FUNCS_H
#define LANG_FUNCS_H

//——————————————————————————————————————————————————————————————————————————————————————————

#include "lang_ctx.h"
#include "tree_commands.h"
#include "stack.h"
#include <wchar.h>

//——————————————————————————————————————————————————————————————————————————————————————————

const wchar_t* GetKeywordName(Keyword_t keyword);

//------------------------------------------------------------------------------------------

void LangPrintNode       (LangCtx_t* lang_ctx, TreeNode_t* node);
void LangPrintError      (LangCtx_t* lang_ctx);
void LangPrintSyntaxError(LangCtx_t* lang_ctx);

//------------------------------------------------------------------------------------------

LangErr_t   LangCtxCtor           (LangCtx_t* lang_ctx);
void        LangCtxDtor           (LangCtx_t* lang_ctx);
LangErr_t   LangOpenReverseFile   (LangCtx_t* lang_ctx);

//==========================================================================================

LangErr_t LangNamesPoolCtor       (NamesPool_t* names_pool);
void      LangNamesPoolDtor       (NamesPool_t* names_pool);
LangErr_t LangNamesPoolPush       (NamesPool_t* names_pool, const wchar_t* name_buf, size_t* name_index);
wchar_t*  LangGetIdName           (NamesPool_t* names_pool, size_t name_index);

//==========================================================================================

LangErr_t   LangIdTableCtor       (IdTable_t* id_table);
void        LangIdTableDtor       (IdTable_t* id_table);
LangErr_t   LangIdTablePush       (IdTable_t* id_table, IdData_t* id_data, int* dest_index_p);

void LangIdDataBuild(LangCtx_t*   lang_ctx, 
                     IdData_t*    id_data, 
                     size_t       name_index,
                     size_t       n_local_vars,
                     size_t       n_params,
                     IdType_t     id_type);

bool LangIdTableFuncIsDeclared          (IdTable_t* id_table, size_t name_index);
int  LangIdTableGetFuncTableIndex       (IdTable_t* id_table, size_t name_index);
int  LangIdTableVarGetTableIndex        (IdTable_t* id_table, size_t name_index);
bool LangIdTableIdentifierIsDeclared    (IdTable_t* id_table, size_t name_index);

LangErr_t LangIsFuncCallArgsCorrect      (LangCtx_t* lang_ctx, 
                                          int        func_index, 
                                          int        args_count);

LangErr_t LangIdTablePushVariableIfUnique(LangCtx_t*   lang_ctx, 
                                          IdTable_t*   id_table,  
                                          size_t       name_id,
                                          IdType_t     type,
                                          int*         table_index);

LangErr_t LangIdTablePushFunctionIfUnique(LangCtx_t*   lang_ctx, 
                                          size_t       name_index,
                                          int*         table_index);

void LangIdTableCountVarsAndParams(IdTable_t* id_table, 
                                   size_t*    n_local_vars_p, 
                                   size_t*    n_params_p);

void LangIdTableGetFunctionVarsAndParams(IdTable_t*   id_table, 
                                         size_t       id_index, 
                                         size_t*      n_local_vars,
                                         size_t*      n_params);

LangErr_t LangGetIdData (IdTable_t* id_table, size_t index, IdData* id_data);

LangErr_t LangIdTableFunctionSetParamsLocals(IdTable_t*   id_table, 
                                             size_t       id_index, 
                                             size_t       n_local_vars, 
                                             size_t       n_params);

//------------------------------------------------------------------//

LangErr_t LangCountAddresses(LangCtx_t* lang_ctx);

//——————————————————————————————————————————————————————————————————————————————————————————

const size_t STACK_ELEMENT_SIZE          = 8; // 64-bit arch
const size_t STACK_FRAME_PROLOGUE_SIZE   = 2; // for rbp + return address
const size_t DEFAULT_ID_TABLE_CAPACITY   = 64;
const size_t DEFAULT_NAMES_POOL_CAPACITY = 64;
const size_t MAX_BUFFER_SIZE             = 256;

//——————————————————————————————————————————————————————————————————————————————————————————

#endif /* LANG_FUNCS_H */
