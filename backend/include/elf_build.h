#ifndef ELF_BUILD_H
#define ELF_BUILD_H

//——————————————————————————————————————————————————————————————————————————————————————————

#include "backend.h"
#include "elf_build_header.h"
#include "elf_build_string_table.h"
#include "elf_build_symbol_table.h"
#include "elf_build_relocation_table.h"
#include <elf.h>

//——————————————————————————————————————————————————————————————————————————————————————————

const char SH_STR_TAB[] = {'\0', '.', 't', 'e', 'x', 't', '\0', 
                           '.', 'r', 'e', 'l', 'a', '.', 't', 'e', 'x', 't', '\0',
                           '.', 's', 'y', 'm', 't', 'a', 'b', '\0',
                           '.', 's', 't', 'r', 't', 'a', 'b', '\0',
                           '.', 's', 'h', 's', 't', 'r', 't', 'a', 'b', '\0' };

const size_t SH_STRTAB_TEXT_NAME_START      = 1;
const size_t SH_STRTAB_RELA_TEXT_NAME_START = 7;
const size_t SH_STRTAB_SYMTAB_NAME_START    = 18;
const size_t SH_STRTAB_STRTAB_NAME_START    = 26;
const size_t SH_STRTAB_SH_STRTAB_NAME_START = 34;

//------------------------------------------------------------------//

const size_t SH_NULL_INDEX      = 0;
const size_t SH_TEXT_INDEX      = 1;
const size_t SH_RELA_TEXT_INDEX = 2;
const size_t SH_SYMTAB_INDEX    = 3;
const size_t SH_STRTAB_INDEX    = 4;
const size_t SH_SH_STRTAB_INDEX = 5;

//------------------------------------------------------------------//

const size_t SH_ADDR_ALIGN_NULL_INDEX      = 0;
const size_t SH_ADDR_ALIGN_TEXT_INDEX      = 1;
const size_t SH_ADDR_ALIGN_RELA_TEXT_INDEX = 8;
const size_t SH_ADDR_ALIGN_SYMTAB_INDEX    = 8;
const size_t SH_ADDR_ALIGN_STRTAB_INDEX    = 1;
const size_t SH_ADDR_ALIGN_SH_STRTAB_INDEX = 1;

//------------------------------------------------------------------//

// [0] NULL
// [1] .text
// [2] .rela.text
// [3] .symtab
// [4] .strtab
// [5] .shstrtab
const size_t SECTIONS_COUNT = 5;
const size_t SECTION_HEADERS_COUNT = SECTIONS_COUNT + 1; // + first is NULL
const size_t INDEX_OF_SECTION_HEADER_STRTAB = 4;

//------------------------------------------------------------------//

typedef Elf64_Shdr SectionHeader_t;

//------------------------------------------------------------------//

const size_t SH_VIRTUAL_ADDRESS_NULL = 0;
const size_t SHF_NOFLAGS             = 0;
const size_t SH_NO_FIXED_ENT_SIZE    = 0;
const size_t SH_NO_LINK              = 0;
const size_t SH_NO_INFO              = 0;

//------------------------------------------------------------------//

const size_t MAX_PADDING = 128;

//------------------------------------------------------------------//

typedef struct ElfSectionsOffsets
{
    size_t text;
    size_t text_size;

    size_t reloc;
    size_t reloc_size;

    size_t symtab;
    size_t symtab_size;

    size_t strtab;
    size_t strtab_size;

    size_t shstrtab;
    size_t shstrtab_size;

    size_t section_header;
}
ElfSectionsOffsets_t;

//------------------------------------------------------------------//

typedef struct ElfCtx
{
    Elf64_Ehdr           header;      
  
    RelocationTable_t    reloc_table;
  
    SymbolTable_t        sym_table;
  
    StringTable_t        str_table;

    ElfSectionsOffsets_t offsets;

    SectionHeader_t      section_header_table[SECTION_HEADERS_COUNT];
}
ElfCtx_t;

//------------------------------------------------------------------//

const char * const ELF_DIR_NAME = "elf";

//——————————————————————————————————————————————————————————————————————————————————————————

BackendErr_t BackendOpenElfFile(BackendCtx_t* backend_ctx);

BackendErr_t ElfCtxCtor(ElfCtx_t* elf_ctx, BackendCtx_t* backend_ctx);
void         ElfCtxDtor(ElfCtx_t* elf_ctx);

BackendErr_t ElfBuild(BackendCtx_t* backend_ctx, ElfCtx_t* elf_ctx);

BackendErr_t ElfWrite(FILE* elf_file, ElfCtx_t* elf_ctx, uint8_t* text);

//——————————————————————————————————————————————————————————————————————————————————————————

#endif /* ELF_BUILD_H */