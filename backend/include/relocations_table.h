    #ifndef RELOCATIONS_TABLE_H
#define RELOCATIONS_TABLE_H

//——————————————————————————————————————————————————————————————————————————————————————————

#include <stdlib.h>
#include "backend_err.h"
#include "lang_ctx.h"
#include "common.h"

//——————————————————————————————————————————————————————————————————————————————————————————

const wchar_t * const MAIN_ENTRY_LABEL = L"main"; 

//——————————————————————————————————————————————————————————————————————————————————————————

typedef enum RelFuncType
{
    REL_FUNC_DECL,
    REL_FUNC_CALL
}
RelFuncType_t;

//------------------------------------------------------------------//

const char * const REL_FUNC_TYPE_NAME[] = {
    [REL_FUNC_DECL] = "REL_FUNC_DECL",
    [REL_FUNC_CALL] = "REL_FUNC_CALL"
};

//——————————————————————————————————————————————————————————————————————————————————————————

typedef enum RelScopeType
{
    REL_FUNC_GLOBAL,
    REL_FUNC_LOCAL,
    REL_FUNC_EXTERN
}
RelScopeType_t;

//------------------------------------------------------------------//

const char * const REL_FUNC_SCOPE_NAME[] = {
    [REL_FUNC_GLOBAL] = "REL_FUNC_GLOBAL",
    [REL_FUNC_LOCAL ] = "REL_FUNC_LOCAL",
    [REL_FUNC_EXTERN] = "REL_FUNC_EXTERN"
};

//——————————————————————————————————————————————————————————————————————————————————————————

typedef struct RelElem
{
    int             id_table_index;
    const wchar_t*  label;
    
    size_t          bin_code_pos;

    RelScopeType_t  scope;
    RelFuncType_t   type;

    size_t          strtab_index;
    size_t          symtab_index;

    size_t          func_size;
}
RelElem_t;

//——————————————————————————————————————————————————————————————————————————————————————————

typedef struct RelTable
{
    size_t     capacity;
    size_t     size;

    RelElem_t* data;
}
RelTable_t;

//——————————————————————————————————————————————————————————————————————————————————————————

const size_t REL_TABLE_INIT_CAPACITY           = 128;
const size_t ID_TABLE_INDEX_EXTERN_FUNC_POISON = 67676767;

//——————————————————————————————————————————————————————————————————————————————————————————

#ifdef BACKEND_DEBUG
    #define REL_TABLE_DUMP_(fmt, ...)                       \
            BEGIN                                           \
            WDPRINTF(fmt, ##__VA_ARGS__);                   \
            if (RelTableDump(&backend_ctx->lang_ctx,        \
                             &backend_ctx->rel_table,       \
                             __func__, __FILE__, __LINE__,  \
                            fmt, ##__VA_ARGS__))            \
            {                                               \
                return BACKEND_LANG_ERROR;                  \
            }                                               \
            END
#else
    #define REL_TABLE_DUMP_(fmt, ...)   ;
#endif /* BACKEND_DEBUG */

//——————————————————————————————————————————————————————————————————————————————————————————

LangErr_t RelTableDump(LangCtx_t*     lang_ctx, 
                       RelTable_t*    rel_table, 
                       const char*    func,
                       const char*    file,
                       int            line,
                       const wchar_t* fmt,
                       ...);

//------------------------------------------------------------------//

BackendErr_t RelTableCtor (RelTable_t* rel_table);
void         RelTableDtor (RelTable_t* rel_table);

BackendErr_t RelTablePush (RelTable_t* rel_table, RelElem_t* elem);

BackendErr_t RelTablePushExternCall (RelTable_t*    rel_table, 
                                     const wchar_t* label, 
                                     size_t         bin_code_pos);

BackendErr_t RelTablePushInnerFuncDecl (RelTable_t*    rel_table,
                                        size_t         id_table_index,
                                        const wchar_t* label,
                                        size_t         bin_code_pos,
                                        RelScopeType_t scope);

BackendErr_t RelTablePushInnerFuncCall (RelTable_t*    rel_table,
                                        size_t         id_table_index,
                                        const wchar_t* label,
                                        size_t         bin_code_pos);

BackendErr_t RelTableGetLabelBinCodePos(RelTable_t* rel_table, 
                                        size_t      rel_table_index, 
                                        size_t*     bin_code_pos_dst);

BackendErr_t RelTablePopLabelBinCodePos(RelTable_t* rel_table, 
                                        size_t*     bin_code_pos_dst);

BackendErr_t RelTableGetLabelBinCodePosByIdIndex(RelTable_t* rel_table, 
                                                 size_t      id_table_index, 
                                                 size_t*     bin_code_pos_dst);

BackendErr_t RelTableCountFuncSizes(RelTable_t* rel_table, size_t section_text_size);

int CountLabelRelAddr(size_t label_pos, size_t pos_before_instr_using_label);

//——————————————————————————————————————————————————————————————————————————————————————————

#endif /* RELOCATIONS_TABLE_H */