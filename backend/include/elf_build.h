#ifndef ELF_BUILD_H
#define ELF_BUILD_H

//——————————————————————————————————————————————————————————————————————————————————————————

#include "backend.h"
#include "elf_build_string_table.h"
#include "elf_build_symbol_table.h"
#include <elf.h>

//——————————————————————————————————————————————————————————————————————————————————————————

typedef struct ElfCtx
{
    StringTable_t str_table;
    SymbolTable_t sym_table;
}
ElfCtx_t;

//------------------------------------------------------------------//

const char * const ELF_DIR_NAME = "elf";

//——————————————————————————————————————————————————————————————————————————————————————————

BackendErr_t ElfCtxCtor(ElfCtx_t* elf_ctx, BackendCtx_t* backend_ctx);
void         ElfCtxDtor(ElfCtx_t* elf_ctx);

BackendErr_t BuildElf(BackendCtx_t* backend_ctx);

//——————————————————————————————————————————————————————————————————————————————————————————

#endif /* ELF_BUILD_H */