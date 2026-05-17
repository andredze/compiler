#include "elf_build_symbol_table.h"

//==========================================================================================

BackendErr_t SymbolTableCtor(SymbolTable_t* sym_table, size_t init_cap)
{
    assert(sym_table);

    sym_table->size     = 0;
    sym_table->capacity = init_cap;

    sym_table->data = (SymbolTableElem_t*) calloc (sym_table->capacity, 
                                                   sizeof(SymbolTableElem_t));

    if (sym_table->data == NULL)
    {
        WPRINTERR(L"Failed memory allocation for sym_table");
        return BACKEND_MEMALLOC_ERROR;
    }

    return BACKEND_SUCCESS;
}

//==========================================================================================

void SymbolTableDtor(SymbolTable_t* sym_table)
{
    assert(sym_table);

    sym_table->size     = 0;
    sym_table->capacity = 0;

    free(sym_table->data);
    sym_table->data = NULL;
}

//==========================================================================================

BackendErr_t SymbolTablePush(SymbolTable_t*     sym_table, 
                             SymbolTableElem_t* elem,
                             size_t*            symtab_index_dst)
{
    assert(symtab_index_dst);
    assert(sym_table);
    assert(elem);

    if (sym_table->size + 1 >= sym_table->capacity)
    {
        WPRINTERR(L"Can not push to sym_table, cap limit");
        return BACKEND_SYM_TABLE_LIMIT;
    }

    *symtab_index_dst = sym_table->size;

    sym_table->data[sym_table->size++] = *elem;

    return BACKEND_SUCCESS;
}

//==========================================================================================

BackendErr_t SymbolTableDump(BackendCtx_t*  backend_ctx, 
                             SymbolTable_t* sym_table,
                             const char*    func,
                             const char*    file,
                             int            line,
                             const wchar_t* fmt,
                             ...)
{
    assert(backend_ctx);
    assert(sym_table);

    FILE* fp = backend_ctx->lang_ctx.tree.debug.fp;

    va_list args = {};

    va_start(args, fmt);

    fwprintf(fp, L"<h4><font color=blue>"
                 L"Dump SymbolTable_t %p called from %s at %s:%d"
                 L" Message: ",
                 sym_table,
                 func, 
                 file,
                 line);

    vfwprintf(fp, fmt, args);

    va_end(args);

    fwprintf(fp, L"</h4></font>\n\n");

    fwprintf(fp,
             L".size     = %zu\n"
             L".cap      = %zu\n"
             L".data     = %p\n",
             sym_table->size,
             sym_table->capacity,
             sym_table->data);

    SymbolTableElem_t* sym_elem = NULL;

    fwprintf(fp, L"index  {%8ls, %8ls, %8ls, %8ls, %8ls, %8ls}\n",
                 L"st_name", L"st_info", L"st_other", L"st_shndx", L"st_value", L"st_size");

    for (size_t i = 0; i < sym_table->size; i++)
    {
        sym_elem = &sym_table->data[i];

        fwprintf(fp, L"[ %-2d]: {%8ul, %8hhu, %8hhu, %8ul, %8zu, %8zu}\n",
                     i,
                     sym_elem->st_name,
                     sym_elem->st_info,
                     sym_elem->st_other,
                     sym_elem->st_shndx,
                     sym_elem->st_value,
                     sym_elem->st_size);
    }

    fwprintf(fp, L"---------------------------------------"
                 L"---------------------------------------\n\n");
    
    fflush(fp);

    return BACKEND_SUCCESS;
}

//==========================================================================================

/*
typedef struct
{
  Elf64_Word	st_name;		// Symbol name (string tbl index)
  unsigned char	st_info;		// Symbol type and binding 
  unsigned char st_other;		// Symbol visibility 
  Elf64_Section	st_shndx;		// Section index 
  Elf64_Addr	st_value;		// Symbol value 
  Elf64_Xword	st_size;		// Symbol size 
} Elf64_Sym;
*/
BackendErr_t SymbolTableBuildElem(SymbolTableElem_t* elem,
                                  size_t             str_tab_index,
                                  RelScopeType_t     scope_type,
                                  uint64_t           func_start_bin_code_addr,
                                  uint64_t           func_size)
{
    assert(elem);

    elem->st_name = (Elf64_Word) str_tab_index; // shift of the str_name in str_tab
    
    int st_bind = 0;

    switch (scope_type)
    {
        case REL_FUNC_EXTERN:
            st_bind = STB_GLOBAL;
            break;
    
        case REL_FUNC_GLOBAL:
            st_bind = STB_GLOBAL;
            break;

        case REL_FUNC_LOCAL:
            st_bind = STB_LOCAL;
            break;

        default:
            WPRINTERR(L"Wrong scope_type %d", scope_type);
            return BACKEND_CORRUPTED_ENUM;
    }

    if (scope_type == REL_FUNC_EXTERN)
    {
        elem->st_shndx = STN_UNDEF; // 0
        elem->st_value = 0;
        elem->st_size  = 0;
    }
    else
    {
        elem->st_shndx = TEXT_SECTION_NUMBER;
        elem->st_value = func_start_bin_code_addr;
        elem->st_size  = func_size;
    }

    elem->st_info  = ELF64_ST_INFO(st_bind, STT_FUNC);
    elem->st_other = STV_DEFAULT; // default visibility

    return BACKEND_SUCCESS;
}

//==========================================================================================

BackendErr_t ElfBuildSymbolTable(BackendCtx_t* backend_ctx, SymbolTable_t* sym_table)
{
    assert(backend_ctx);
    assert(sym_table);

    BackendErr_t error = BACKEND_SUCCESS;

    SymbolTableElem_t elem = {};

    size_t symtab_ind = 0;
    
    // first element should be NULL
    if ((error = SymbolTablePush(sym_table, &elem, &symtab_ind)))
    {
        return error;
    }

    RelTable_t* rel_table = &backend_ctx->rel_table;

    for (size_t i = 0; i < rel_table->size; i++)
    {
        RelElem_t* rel_elem = &rel_table->data[i];

        if (!(rel_elem->type  == REL_FUNC_DECL ||
              rel_elem->scope == REL_FUNC_EXTERN))
        {
            continue;
        }
        if ((error = SymbolTableBuildElem(&elem,
                                          rel_elem->strtab_index,
                                          rel_elem->scope,
                                          rel_elem->bin_code_pos,
                                          rel_elem->func_size)))
        {
            return error;
        }
        if ((error = SymbolTablePush(sym_table, &elem, &symtab_ind)))
        {
            return error;
        }

        rel_elem->symtab_index = symtab_ind;

        SYMBOL_TABLE_DUMP_(sym_table, L"put elem %ls", rel_table->data[i].label);
    }

    REL_TABLE_DUMP_(L"placed symtab_indexes");

    return BACKEND_SUCCESS;
}

//==========================================================================================
