#ifndef ELF_BUILD_SYMBOL_TABLE_H
#define ELF_BUILD_SYMBOL_TABLE_H

//——————————————————————————————————————————————————————————————————————————————————————————

#include <elf.h> // :538
#include <stdlib.h>
#include "backend.h"

//——————————————————————————————————————————————————————————————————————————————————————————

typedef struct SymbolTable
{
    size_t     last_local_index_plus_one;

    size_t     capacity;
    size_t     size;

    Elf64_Sym* data;
}
SymbolTable_t;

//------------------------------------------------------------------//

typedef Elf64_Sym SymbolTableElem_t;

//——————————————————————————————————————————————————————————————————————————————————————————

#ifdef BACKEND_DEBUG
    #define SYMBOL_TABLE_DUMP_(sym_table, fmt, ...)           \
            BEGIN                                             \
            WDPRINTF(fmt L"\n", ##__VA_ARGS__);               \
            if (SymbolTableDump(backend_ctx,                  \
                                sym_table,                    \
                                __func__, __FILE__, __LINE__, \
                                fmt, ##__VA_ARGS__))          \
            {                                                 \
                return BACKEND_LANG_ERROR;                    \
            }                                                 \
            END
#else
    #define SYMBOL_TABLE_DUMP_(sym_table, fmt, ...)   ;
#endif /* BACKEND_DEBUG */

//------------------------------------------------------------------//

BackendErr_t SymbolTableDump(BackendCtx_t*  backend_ctx, 
                             SymbolTable_t* sym_table,
                             const char*    func,
                             const char*    file,
                             int            line,
                             const wchar_t* fmt,
                             ...);

BackendErr_t SymbolTableCtor(SymbolTable_t* sym_table, size_t init_cap);
void         SymbolTableDtor(SymbolTable_t* sym_table);

BackendErr_t SymbolTablePush(SymbolTable_t*     sym_table, 
                             SymbolTableElem_t* elem,
                             size_t*            symtab_index_dst);

//------------------------------------------------------------------//

BackendErr_t SymbolTableBuildElem(SymbolTableElem_t* elem,
                                  size_t             str_tab_index,
                                  RelScopeType_t     scope_type,
                                  uint64_t           func_start_bin_code_addr,
                                  uint64_t           func_size);

//------------------------------------------------------------------//

BackendErr_t ElfBuildSymbolTable(BackendCtx_t* backend_ctx, SymbolTable_t* sym_table);

//——————————————————————————————————————————————————————————————————————————————————————————

#endif /* ELF_BUILD_SYMBOL_TABLE_H */