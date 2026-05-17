#include "elf_build.h"

//==========================================================================================

BackendErr_t BackendOpenElfFile(BackendCtx_t* backend_ctx)
{
    DPRINT_FUNC_ENTER_MSG();
    assert(backend_ctx);

    char elf_file_path[MAX_FILE_NAME_LEN] = {};

    snprintf(elf_file_path, sizeof(elf_file_path), "elf/%s", 
             backend_ctx->ast_file_name);

    WDPRINTF(L"Opening file %s\n\n", elf_file_path);

    FILE* elf_fp = fopen(elf_file_path, "wb");

    if (elf_fp == NULL)
    {
        WPRINTERR(L"Failed opening file %s", elf_file_path);        
        return BACKEND_FILE_ERROR;
    }

    backend_ctx->elf_file = elf_fp;

    DPRINT_FUNC_LEAVE_MSG();
    return BACKEND_SUCCESS;
}

//==========================================================================================

BackendErr_t ElfCtxCtor(ElfCtx_t* elf_ctx, BackendCtx_t* backend_ctx)
{
    assert(backend_ctx);
    assert(elf_ctx);

    BackendErr_t error = BACKEND_SUCCESS;

    if ((error = StringTableCtor(&elf_ctx->str_table, STRING_TABLE_INIT_CAPACITY)))
    {
        return error;
    }
    if ((error = SymbolTableCtor(&elf_ctx->sym_table, backend_ctx->rel_table.size + 2)))
    {
        return error;
    }
    if ((error = RelocationTableCtor(&elf_ctx->reloc_table, backend_ctx->rel_table.size + 1)))
    {
        return error;
    }

    return BACKEND_SUCCESS;
}

//==========================================================================================

void ElfCtxDtor(ElfCtx_t* elf_ctx)
{
    assert(elf_ctx);

    StringTableDtor    (&elf_ctx->str_table);
    SymbolTableDtor    (&elf_ctx->sym_table);
    RelocationTableDtor(&elf_ctx->reloc_table);
}

//==========================================================================================

static size_t GetAlignedAddress(size_t address, size_t alignment)
{
    size_t mod = address % alignment;
    
    return (mod == 0) ? address : (address + (alignment - mod));
}

//==========================================================================================

static void ElfCountSectionsOffsets(ElfCtx_t* elf_ctx)
{
    assert(elf_ctx);

    ElfSectionsOffsets_t* offsets = &elf_ctx->offsets;

    offsets->text = sizeof(Elf64_Ehdr);
    
    // must be 8-bytes aligned
    offsets->reloc = GetAlignedAddress(offsets->text + offsets->text_size, SH_ADDR_ALIGN_RELA_TEXT_INDEX); 
    
    offsets->reloc_size = elf_ctx->reloc_table.size * sizeof(elf_ctx->reloc_table.data[0]);

    // must be 8-bytes aligned
    offsets->symtab = GetAlignedAddress(offsets->reloc + offsets->reloc_size, SH_ADDR_ALIGN_SYMTAB_INDEX);
    
    offsets->symtab_size = elf_ctx->sym_table.size * sizeof(elf_ctx->sym_table.data[0]);

    offsets->strtab = offsets->symtab + offsets->symtab_size;

    offsets->strtab_size = elf_ctx->str_table.size * sizeof(elf_ctx->str_table.data[0]);

    offsets->shstrtab = offsets->strtab + offsets->strtab_size;

    offsets->shstrtab_size = sizeof(SH_STR_TAB);

    offsets->section_header = offsets->shstrtab + offsets->shstrtab_size;
}

//==========================================================================================

inline
void ElfSectionHeaderSet(SectionHeader_t* section_header,
                         Elf64_Word	      sh_name,
                         Elf64_Word	      sh_type,
                         Elf64_Word	      sh_flags,
                         Elf64_Word	      sh_offset,
                         Elf64_Word	      sh_size,
                         Elf64_Word	      sh_link,
                         Elf64_Word	      sh_info,
                         Elf64_Word	      sh_addralign,
                         Elf64_Word	      sh_entsize)
{
    assert(section_header);

    section_header->sh_name      = sh_name;
    section_header->sh_type      = sh_type;
    section_header->sh_flags     = sh_flags;
    section_header->sh_addr      = SH_VIRTUAL_ADDRESS_NULL;
    section_header->sh_offset    = sh_offset;
    section_header->sh_size      = sh_size;
    section_header->sh_link      = sh_link;
    section_header->sh_info      = sh_info;
    section_header->sh_addralign = sh_addralign;
    section_header->sh_entsize   = sh_entsize;
}

//==========================================================================================

static BackendErr_t ElfBuildSectionHeaderTable(ElfCtx_t* elf_ctx)
{
    assert(elf_ctx);

    // NULL
    elf_ctx->section_header_table[0] = {};

    // .text
    ElfSectionHeaderSet(&elf_ctx->section_header_table[SH_TEXT_INDEX],
                        SH_STRTAB_TEXT_NAME_START,
                        SHT_PROGBITS,
                        SHF_EXECINSTR | SHF_ALLOC,
                        elf_ctx->offsets.text,
                        elf_ctx->offsets.text_size,
                        SH_NO_LINK,
                        SH_NO_INFO,
                        SH_ADDR_ALIGN_TEXT_INDEX,
                        SH_NO_FIXED_ENT_SIZE);
    // .rela.text
    ElfSectionHeaderSet(&elf_ctx->section_header_table[SH_RELA_TEXT_INDEX],
                        SH_STRTAB_RELA_TEXT_NAME_START,
                        SHT_RELA,
                        SHF_NOFLAGS,
                        elf_ctx->offsets.reloc,
                        elf_ctx->offsets.reloc_size,
                        SH_SYMTAB_INDEX,
                        SH_TEXT_INDEX,
                        SH_ADDR_ALIGN_RELA_TEXT_INDEX,
                        sizeof(RelocationTableElem_t));
    // .symtab
    ElfSectionHeaderSet(&elf_ctx->section_header_table[SH_SYMTAB_INDEX],
                        SH_STRTAB_SYMTAB_NAME_START,
                        SHT_SYMTAB,
                        SHF_NOFLAGS,
                        elf_ctx->offsets.symtab,
                        elf_ctx->offsets.symtab_size,
                        SH_STRTAB_INDEX,
                        elf_ctx->sym_table.last_local_index_plus_one,
                        SH_ADDR_ALIGN_SYMTAB_INDEX,
                        sizeof(SymbolTableElem_t));
    // .strtab
    ElfSectionHeaderSet(&elf_ctx->section_header_table[SH_STRTAB_INDEX],
                        SH_STRTAB_STRTAB_NAME_START,
                        SHT_STRTAB,
                        SHF_NOFLAGS,
                        elf_ctx->offsets.strtab,
                        elf_ctx->offsets.strtab_size,
                        SH_NO_LINK,
                        SH_NO_INFO,
                        SH_ADDR_ALIGN_STRTAB_INDEX,
                        SH_NO_FIXED_ENT_SIZE);
    // .shstrtab
    ElfSectionHeaderSet(&elf_ctx->section_header_table[SH_SH_STRTAB_INDEX],
                        SH_STRTAB_SH_STRTAB_NAME_START,
                        SHT_STRTAB,
                        SHF_NOFLAGS,
                        elf_ctx->offsets.shstrtab,
                        elf_ctx->offsets.shstrtab_size,
                        SH_NO_LINK,
                        SH_NO_INFO,
                        SH_ADDR_ALIGN_SH_STRTAB_INDEX,
                        SH_NO_FIXED_ENT_SIZE);

    return BACKEND_SUCCESS;
}

//==========================================================================================

BackendErr_t ElfBuild(BackendCtx_t* backend_ctx, ElfCtx_t* elf_ctx)
{
    assert(backend_ctx);
    assert(elf_ctx);

    BackendErr_t error = BACKEND_SUCCESS;

    if ((error = ElfBuildStringTable(backend_ctx, &elf_ctx->str_table)))
    {
        return error;
    }
    if ((error = ElfBuildSymbolTable(backend_ctx, &elf_ctx->sym_table)))
    {
        return error;
    }
    if ((error = ElfBuildRelocationTable(backend_ctx, &elf_ctx->reloc_table)))
    {
        return error;
    }

    elf_ctx->offsets.text_size = BinCodeGetCurrentPos(&backend_ctx->bin_code);

    ElfCountSectionsOffsets(elf_ctx);
    
    if ((error = ElfBuildHeader(&elf_ctx->header, elf_ctx->offsets.section_header)))
    {
        return error;
    }
    if ((error = ElfBuildSectionHeaderTable(elf_ctx)))
    {
        return error;
    }

    return BACKEND_SUCCESS;
}

//==========================================================================================

static size_t fwrite_padding(FILE* file, size_t padding_bytes_count)
{
    assert(file);

    if (padding_bytes_count == 0)
    {
        return 1;
    }

    if (padding_bytes_count >= MAX_PADDING)
    {
        return (size_t)-1;
    }

    void* padding = (void*) calloc(padding_bytes_count, 1);
    
    if (padding == NULL)
    {
        return (size_t)-1;
    }

    size_t returned = fwrite(padding, padding_bytes_count, 1, file);

    free(padding);

    return returned;
}

//==========================================================================================

#define FWRITE(name, src_ptr, src_size)                                   \
        BEGIN                                                             \
        if ((src_size) != 0)                                              \
        {                                                                 \
            if (fwrite((void*) (src_ptr), (src_size), 1, elf_file) != 1)  \
            {                                                             \
                WPRINTERR(L"Failed writing " name);                       \
                return BACKEND_FAILED_FWRITE_TO_ELF;                      \
            }                                                             \
        }                                                                 \
        END

//------------------------------------------------------------------//

BackendErr_t ElfWrite(FILE* elf_file, ElfCtx_t* elf_ctx, uint8_t* text)
{
    assert(elf_file);
    assert(elf_ctx);
    assert(text);

    FWRITE(L"elf header", &elf_ctx->header, sizeof(elf_ctx->header));
    FWRITE(L"elf text", text, elf_ctx->offsets.text_size);

    size_t padding = elf_ctx->offsets.reloc - (elf_ctx->offsets.text + 
                                               elf_ctx->offsets.text_size);

    if (fwrite_padding(elf_file, padding) != 1)
    {
        WPRINTERR(L"Failed writing text padding %d", padding);
        return BACKEND_FAILED_FWRITE_TO_ELF;
    }
    
    FWRITE(L"elf rela text", &elf_ctx->reloc_table.data, elf_ctx->offsets.reloc_size);

    padding = elf_ctx->offsets.symtab - (elf_ctx->offsets.reloc + elf_ctx->offsets.reloc_size);

    if (fwrite_padding(elf_file, padding) != 1)
    {
        WPRINTERR(L"Failed writing rela text padding %d", padding);
        return BACKEND_FAILED_FWRITE_TO_ELF;
    }

    FWRITE(L"elf symtab",   elf_ctx->sym_table.data,       elf_ctx->offsets.symtab_size);

    // if (fwrite((void*) &elf_ctx->sym_table.data, elf_ctx->offsets.symtab_size, 
    //             1, elf_file) != 1)
    // {
    //     WPRINTERR(L"Failed writing symtab");
    //     return BACKEND_FAILED_FWRITE_TO_ELF;
    // }
    // if (fwrite((void*) elf_ctx->str_table.data, elf_ctx->offsets.strtab_size, 
    //             1, stderr) != 1)
    // {
    //     WPRINTERR(L"Failed writing strtab");
    //     return BACKEND_FAILED_FWRITE_TO_ELF;
    // }

    FWRITE(L"elf strtab",   elf_ctx->str_table.data,       elf_ctx->offsets.strtab_size);
    FWRITE(L"elf shstrtab", SH_STR_TAB,                    elf_ctx->offsets.shstrtab_size);
    FWRITE(L"elf sh table", elf_ctx->section_header_table, sizeof(elf_ctx->section_header_table));

    return BACKEND_SUCCESS;
}

//------------------------------------------------------------------//

#undef FWRITE

    // if (fwrite((void*) &elf_ctx->header, sizeof(elf_ctx->header), 1, elf_file) != 1)
    // {
    //     WPRINTERR(L"Failed writing elf->header");
    //     return BACKEND_FAILED_FWRITE_TO_ELF;
    // }
    // if (fwrite((void*) text, elf_ctx->offsets.text_size, 1, elf_file) != 1)
    // {
    //     WPRINTERR(L"Failed writing text");
    //     return BACKEND_FAILED_FWRITE_TO_ELF;
    // }


    // if (fwrite((void*) &elf_ctx->reloc_table.data, elf_ctx->offsets.reloc_size, 
    //             1, elf_file) != 1)
    // {
    //     WPRINTERR(L"Failed writing rela text");
    //     return BACKEND_FAILED_FWRITE_TO_ELF;
    // }


    // if (fwrite((void*) SH_STR_TAB, elf_ctx->offsets.shstrtab_size, 1, elf_file) != 1)
    // {
    //     WPRINTERR(L"Failed writing shstrtab");
    //     return BACKEND_FAILED_FWRITE_TO_ELF;
    // }
    // if (fwrite((void*) &elf_ctx->section_header_table, sizeof(elf_ctx->section_header_table), 
    //             1, elf_file) != 1)
    // {
    //     WPRINTERR(L"Failed writing section header table");
    //     return BACKEND_FAILED_FWRITE_TO_ELF;
    // }

//==========================================================================================
