#include "elf_build.h"

//==========================================================================================

BackendErr_t BackendOpenElfFile(BackendCtx_t* backend_ctx)
{
    DPRINT_FUNC_ENTER_MSG();
    assert(backend_ctx);

    char elf_file_path[MAX_FILE_NAME_LEN] = {};

    snprintf(elf_file_path, sizeof(elf_file_path), "elf/%s.o", 
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

    if (elf_ctx->buffer)
    {
        free(elf_ctx->buffer);
        elf_ctx->buffer = NULL;
    }
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
    
    WDPRINTF(L".text offset %#x\n", offsets->text);

    // must be 8-bytes aligned
    
    offsets->reloc = GetAlignedAddress(offsets->text + offsets->text_size, SH_ADDR_ALIGN_RELA_TEXT_INDEX); 
    
    offsets->before_reloc_padding = offsets->reloc - (offsets->text + offsets->text_size);
    
    offsets->reloc_size = elf_ctx->reloc_table.size * sizeof(elf_ctx->reloc_table.data[0]);

    WDPRINTF(L"text_size %#x\n",            offsets->text_size);
    WDPRINTF(L"before reloc padding %#x\n", offsets->before_reloc_padding);
    WDPRINTF(L".reloc offset %#x\n",        offsets->reloc);

    // must be 8-bytes aligned
    offsets->symtab = GetAlignedAddress(offsets->reloc + offsets->reloc_size, SH_ADDR_ALIGN_SYMTAB_INDEX);
    
    offsets->before_symtab_padding = offsets->symtab - (offsets->reloc + offsets->reloc_size);
    
    offsets->symtab_size = elf_ctx->sym_table.size * sizeof(elf_ctx->sym_table.data[0]);
    
    WDPRINTF(L"reloc_size %#x\n",            offsets->reloc_size);
    WDPRINTF(L"before symtab padding %#x\n", offsets->before_symtab_padding);
    WDPRINTF(L".symtab offset %#x\n",        offsets->symtab);

    WDPRINTF(L"symtab_size %#x\n",           offsets->symtab_size);

    offsets->strtab = offsets->symtab + offsets->symtab_size;

    WDPRINTF(L".strtab offset %#x\n",        offsets->strtab);

    offsets->strtab_size = elf_ctx->str_table.size * sizeof(elf_ctx->str_table.data[0]);

    WDPRINTF(L"strtab_size %#x\n",           offsets->strtab_size);

    offsets->shstrtab = offsets->strtab + offsets->strtab_size;

    WDPRINTF(L".shstrtab offset %#x\n",      offsets->shstrtab);

    offsets->shstrtab_size = sizeof(SH_STR_TAB);

    WDPRINTF(L"shstrtab_size %#x\n",         offsets->shstrtab_size);

    offsets->section_header_table = offsets->shstrtab + offsets->shstrtab_size;

    WDPRINTF(L"section_header_table offset %#x\n", offsets->section_header_table);

    offsets->section_header_table_size = sizeof(elf_ctx->section_header_table);

    WDPRINTF(L"section_header_table_size %#x\n", offsets->section_header_table_size);

    offsets->total_size = offsets->section_header_table + offsets->section_header_table_size;

    WDPRINTF(L"total_size %#x\n", offsets->total_size);
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
                        (Elf64_Word) elf_ctx->offsets.text,
                        (Elf64_Word) elf_ctx->offsets.text_size,
                        SH_NO_LINK,
                        SH_NO_INFO,
                        SH_ADDR_ALIGN_TEXT_INDEX,
                        SH_NO_FIXED_ENT_SIZE);
    // .rela.text
    ElfSectionHeaderSet(&elf_ctx->section_header_table[SH_RELA_TEXT_INDEX],
                        SH_STRTAB_RELA_TEXT_NAME_START,
                        SHT_RELA,
                        SHF_NOFLAGS,
                        (Elf64_Word) elf_ctx->offsets.reloc,
                        (Elf64_Word) elf_ctx->offsets.reloc_size,
                        SH_SYMTAB_INDEX,
                        SH_TEXT_INDEX,
                        SH_ADDR_ALIGN_RELA_TEXT_INDEX,
                        sizeof(RelocationTableElem_t));
    // .symtab
    ElfSectionHeaderSet(&elf_ctx->section_header_table[SH_SYMTAB_INDEX],
                        SH_STRTAB_SYMTAB_NAME_START,
                        SHT_SYMTAB,
                        SHF_NOFLAGS,
                        (Elf64_Word) elf_ctx->offsets.symtab,
                        (Elf64_Word) elf_ctx->offsets.symtab_size,
                        SH_STRTAB_INDEX,
                        (Elf64_Word) elf_ctx->sym_table.last_local_index_plus_one,
                        SH_ADDR_ALIGN_SYMTAB_INDEX,
                        sizeof(SymbolTableElem_t));
    // .strtab
    ElfSectionHeaderSet(&elf_ctx->section_header_table[SH_STRTAB_INDEX],
                        SH_STRTAB_STRTAB_NAME_START,
                        SHT_STRTAB,
                        SHF_NOFLAGS,
                        (Elf64_Word) elf_ctx->offsets.strtab,
                        (Elf64_Word) elf_ctx->offsets.strtab_size,
                        SH_NO_LINK,
                        SH_NO_INFO,
                        SH_ADDR_ALIGN_STRTAB_INDEX,
                        SH_NO_FIXED_ENT_SIZE);
    // .shstrtab
    ElfSectionHeaderSet(&elf_ctx->section_header_table[SH_SH_STRTAB_INDEX],
                        SH_STRTAB_SH_STRTAB_NAME_START,
                        SHT_STRTAB,
                        SHF_NOFLAGS,
                        (Elf64_Word) elf_ctx->offsets.shstrtab,
                        (Elf64_Word) elf_ctx->offsets.shstrtab_size,
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

    elf_ctx->text = backend_ctx->bin_code.buffer;

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
    
    if ((error = ElfBuildHeader(&elf_ctx->header, elf_ctx->offsets.section_header_table)))
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

BackendErr_t ElfWriteBuffer(FILE* elf_file, ElfCtx_t* elf_ctx)
{
    assert(elf_file);
    assert(elf_ctx);

    if (fwrite(elf_ctx->buffer, elf_ctx->offsets.total_size, 1, elf_file) != 1)
    {
        WPRINTERR(L"Failed fwrite buffer to elf file");
        return BACKEND_FAILED_FWRITE_TO_ELF;
    }

    return BACKEND_SUCCESS;
}

//==========================================================================================

BackendErr_t ElfCopyContextToBuffer(ElfCtx_t* elf_ctx)
{
    assert(elf_ctx);

    size_t buffer_pos = 0;

    elf_ctx->buffer = (uint8_t*) calloc(elf_ctx->offsets.total_size, 1);

    if (elf_ctx->buffer == NULL)
    {
        WPRINTERR(L"Failed memalloc for elf buffer");
        return BACKEND_MEMALLOC_ERROR;
    }

    memcpy(&elf_ctx->buffer[buffer_pos], &elf_ctx->header, sizeof(Elf64_Ehdr));
    buffer_pos += sizeof(Elf64_Ehdr);

    memcpy(&elf_ctx->buffer[buffer_pos], elf_ctx->text, elf_ctx->offsets.text_size);
    buffer_pos += elf_ctx->offsets.text_size + elf_ctx->offsets.before_reloc_padding;

    memcpy(&elf_ctx->buffer[buffer_pos], elf_ctx->reloc_table.data, elf_ctx->offsets.reloc_size);
    buffer_pos += elf_ctx->offsets.reloc_size + elf_ctx->offsets.before_symtab_padding;
    
    memcpy(&elf_ctx->buffer[buffer_pos], elf_ctx->sym_table.data, elf_ctx->offsets.symtab_size);
    buffer_pos += elf_ctx->offsets.symtab_size;
    
    memcpy(&elf_ctx->buffer[buffer_pos], elf_ctx->str_table.data, elf_ctx->offsets.strtab_size);
    buffer_pos += elf_ctx->offsets.strtab_size;

    memcpy(&elf_ctx->buffer[buffer_pos], SH_STR_TAB, elf_ctx->offsets.shstrtab_size);
    buffer_pos += elf_ctx->offsets.shstrtab_size;

    memcpy(&elf_ctx->buffer[buffer_pos], elf_ctx->section_header_table, 
            elf_ctx->offsets.section_header_table_size);

    buffer_pos += elf_ctx->offsets.section_header_table_size;

    return BACKEND_SUCCESS;
}

//==========================================================================================

#include "elf_build_string_table.h"

//==========================================================================================

BackendErr_t ElfCtxDump(BackendCtx_t*  backend_ctx, 
                        ElfCtx_t*      elf_ctx,
                        const char*    func,
                        const char*    file,
                        int            line,
                        const wchar_t* fmt,
                        ...)
{
    assert(backend_ctx);
    assert(elf_ctx);

    SYMBOL_TABLE_DUMP_(&elf_ctx->sym_table, L"general dump");
    STRING_POOL_TABLE_DUMP_(NULL, &elf_ctx->str_table, L"general dump");
    RELOCATION_TABLE_DUMP_(&elf_ctx->reloc_table, L"general dump");
    
    FILE* fp = backend_ctx->lang_ctx.tree.debug.fp;

    va_list args = {};

    va_start(args, fmt);

    fwprintf(fp, L"<h4><font color=blue>"
                 L"Dump ElfCtx_t %p called from %s at %s:%d"
                 L" Message: ",
                 elf_ctx,
                 func, 
                 file,
                 line);

    vfwprintf(fp, fmt, args);

    va_end(args);

    fwprintf(fp, L"</h4></font>\n\n");

    fwprintf(fp,
             L"elf_ctx->offsets:\n"
             L".text                      = %#x\n"
             L".text_size                 = %#x\n"
             L".before_reloc_padding      = %#x\n"
             L".reloc                     = %#x\n"
             L".reloc_size                = %#x\n"
             L".before_symtab_padding     = %#x\n"
             L".symtab                    = %#x\n"
             L".symtab_size               = %#x\n"
             L".strtab                    = %#x\n"
             L".strtab_size               = %#x\n"
             L".shstrtab                  = %#x\n"
             L".shstrtab_size             = %#x\n"
             L".section_header_table      = %#x\n"
             L".section_header_table_size = %#x\n"
             L".total_size                = %#x\n",
             elf_ctx->offsets.text,
             elf_ctx->offsets.text_size,
             elf_ctx->offsets.before_reloc_padding,
             elf_ctx->offsets.reloc,
             elf_ctx->offsets.reloc_size,
             elf_ctx->offsets.before_symtab_padding,
             elf_ctx->offsets.symtab,
             elf_ctx->offsets.symtab_size,
             elf_ctx->offsets.strtab,
             elf_ctx->offsets.strtab_size,
             elf_ctx->offsets.shstrtab,
             elf_ctx->offsets.shstrtab_size,
             elf_ctx->offsets.section_header_table,
             elf_ctx->offsets.section_header_table_size,
             elf_ctx->offsets.total_size);
    
    if (elf_ctx->buffer == NULL)
    {
        return BACKEND_SUCCESS;
    }

    fwprintf(fp, L"===================================== HEX DUMP =====================================\n");

    for (size_t i = 0; i < elf_ctx->offsets.total_size; i++)
    {
        if (i % 16 == 0)
        {
            fwprintf(fp, L"%#06x: ", i);
        }        

        if ((i == elf_ctx->offsets.text                ) ||
            (i == elf_ctx->offsets.reloc               ) ||
            (i == elf_ctx->offsets.symtab              ) ||
            (i == elf_ctx->offsets.strtab              ) ||
            (i == elf_ctx->offsets.shstrtab            ) ||
            (i == elf_ctx->offsets.section_header_table))
        {
            fwprintf(fp, L"<font color=red>%02x</font> ", elf_ctx->buffer[i]);
        }
        else
        {
            fwprintf(fp, L"%02x ", elf_ctx->buffer[i]);
        }

        if ((i+1) % 16 == 0)
        {
            fwprintf(fp, L"\n");
        }
    }

    fwprintf(fp, L"\n=========================================="
                 L"==========================================\n");


    fwprintf(fp, L"===================================== CHAR DUMP ====================================\n");

    for (size_t i = 0; i < elf_ctx->offsets.total_size; i++)
    {
        if (i % 16 == 0)
        {
            fwprintf(fp, L"%#06x: ", i);
        }        

        if ((i == elf_ctx->offsets.text                ) ||
            (i == elf_ctx->offsets.reloc               ) ||
            (i == elf_ctx->offsets.symtab              ) ||
            (i == elf_ctx->offsets.strtab              ) ||
            (i == elf_ctx->offsets.shstrtab            ) ||
            (i == elf_ctx->offsets.section_header_table))
        {
            if (elf_ctx->buffer[i] == '\0')
            {
                fwprintf(fp, L"<font color=red>00 </font>", elf_ctx->buffer[i]);
            }
            else
            {
                fwprintf(fp, L"<font color=red>%2c </font>", elf_ctx->buffer[i]);
            }
        }
        else
        {
            if (elf_ctx->buffer[i] == '\0')
            {
                fwprintf(fp, L"00 ", elf_ctx->buffer[i]);
            }
            else
            {
                fwprintf(fp, L"%2c ", elf_ctx->buffer[i]);
            }
        }

        if ((i+1) % 16 == 0)
        {
            fwprintf(fp, L"\n");
        }
    }

    fwprintf(fp, L"\n=========================================="
                 L"==========================================\n");

    fflush(fp);

    return BACKEND_SUCCESS;
}

//==========================================================================================
