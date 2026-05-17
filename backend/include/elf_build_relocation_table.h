#ifndef ELF_BUILD_RELOCATION_TABLE_H
#define ELF_BUILD_RELOCATION_TABLE_H

//——————————————————————————————————————————————————————————————————————————————————————————

#include <elf.h> // :666
#include <stdlib.h>
#include "backend.h"

//——————————————————————————————————————————————————————————————————————————————————————————

/*
    R_ADDEND:
    LabelAddress = (DispAddress + Addend) + Displacement
*/
const int R_ADDEND = -4;

const size_t CALL_INSTR_SIZE = 1;

//——————————————————————————————————————————————————————————————————————————————————————————

typedef Elf64_Rela RelocationTableElem_t;

//------------------------------------------------------------------//

typedef struct RelocationTable
{
    size_t capacity;
    size_t size;

    RelocationTableElem_t* data;
}
RelocationTable_t;

//——————————————————————————————————————————————————————————————————————————————————————————

#ifdef BACKEND_DEBUG
    #define RELOCATION_TABLE_DUMP_(reloc_table, fmt, ...)         \
            BEGIN                                                 \
            WDPRINTF(fmt L"\n", ##__VA_ARGS__);                   \
            if (RelocationTableDump(backend_ctx,                  \
                                    reloc_table,                  \
                                    __func__, __FILE__, __LINE__, \
                                    fmt, ##__VA_ARGS__))          \
            {                                                     \
                return BACKEND_LANG_ERROR;                        \
            }                                                     \
            END
#else
    #define RELOCATION_TABLE_DUMP_(reloc_table, fmt, ...)   ;
#endif /* BACKEND_DEBUG */

BackendErr_t RelocationTableDump(BackendCtx_t*      backend_ctx, 
                                 RelocationTable_t* reloc_table,
                                 const char*        func,
                                 const char*        file,
                                 int                line,
                                 const wchar_t*     fmt,
                                 ...);

//------------------------------------------------------------------//

BackendErr_t RelocationTableCtor(RelocationTable_t* reloc_table, size_t init_cap);

BackendErr_t RelocationTablePush(RelocationTable_t* reloc_table, 
                                 RelocationTableElem_t* elem);

void         RelocationTableDtor(RelocationTable_t* reloc_table);

BackendErr_t RelocationTableBuildElem(RelocationTableElem_t* elem,
                                      size_t                 disp_address,
                                      size_t                 symtab_index);

BackendErr_t ElfBuildRelocationTable(BackendCtx_t*      backend_ctx, 
                                     RelocationTable_t* reloc_table);

//——————————————————————————————————————————————————————————————————————————————————————————

#endif /* ELF_BUILD_RELOCATION_TABLE_H */