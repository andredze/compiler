#include "elf_build_symbol_table.h"
#include "elf_build.h"

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

    fwprintf(fp, L"--------------------------------------\n");
    fwprintf(fp, L"st_bind values:\n"
                 L"STB_LOCAL	0  Local symbol\n"
                 L"STB_GLOBAL	1  Global symbol\n\n");

    fwprintf(fp, L"st_type values:\n"
                 L"STT_FUNC	2  Symbol is a code object\n\n");
    fwprintf(fp, L"--------------------------------------\n");

    fwprintf(fp, L"index  {%19ls, %13ls, %13ls, %15ls, %27ls, %27ls}, %20ls\n",
                 L"st_name", L"st_info", L"st_other", L"st_shndx", L"st_value", L"st_size", L"for debug:");

    fwprintf(fp, L"index  {%19ls, %13ls, %13ls, %15ls, %27ls, %27ls}, %13ls %13ls\n",
                 L"strtab_ind", L"scope+type", L"visibility", L"section", L"func_label addr", L"func_size", L"bind(scope)", L"type");          

    for (size_t i = 0; i < sym_table->size; i++)
    {
        sym_elem = &sym_table->data[i];

        fwprintf(fp, L"[ %-2d]: {%8ul (%08x), %8hhu (%02x), %8hhu (%02x), %8ul (%04x), "
                     L"%8zu (%016x), %8zu (%016x)}, %8hhu (%02x), %8hhu (%02x)\n",
                     i,
                     sym_elem->st_name,  sym_elem->st_name,
                     sym_elem->st_info,  sym_elem->st_info,
                     sym_elem->st_other, sym_elem->st_other,
                     sym_elem->st_shndx, sym_elem->st_shndx,
                     sym_elem->st_value, sym_elem->st_value,
                     sym_elem->st_size,  sym_elem->st_size,
                     ELF64_ST_BIND(sym_elem->st_info), ELF64_ST_BIND(sym_elem->st_info), 
                     ELF64_ST_TYPE(sym_elem->st_info), ELF64_ST_TYPE(sym_elem->st_info));
    }

    fwprintf(fp, L"---------------------------------------"
                 L"---------------------------------------\n\n");
    
    fflush(fp);

    return BACKEND_SUCCESS;
}

//==========================================================================================

/* SymbolTableElem_t structure:
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
    
    unsigned char st_bind = 0;
    unsigned char st_type = STT_NOTYPE;

    switch (scope_type)
    {
        case REL_FUNC_EXTERN:
            st_bind = STB_GLOBAL;
            st_type = STT_NOTYPE;
            break;
    
        case REL_FUNC_GLOBAL:
            st_bind = STB_GLOBAL;
            st_type = STT_FUNC;
            break;

        case REL_FUNC_LOCAL:
            st_bind = STB_LOCAL;
            st_type = STT_FUNC;
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
        elem->st_shndx = SH_TEXT_INDEX;
        elem->st_value = func_start_bin_code_addr;
        elem->st_size  = func_size;
    }

    elem->st_info  = ELF64_ST_INFO(st_bind, st_type);
    elem->st_other = STV_DEFAULT; // default visibility

    return BACKEND_SUCCESS;
}

//==========================================================================================

static BackendErr_t ElfBuildSymbolTableElement(RelElem_t* rel_elem, SymbolTable_t* sym_table)
{
    assert(sym_table);
    assert(rel_elem);

    BackendErr_t      error      = BACKEND_SUCCESS;
    size_t            symtab_ind = 0;
    SymbolTableElem_t elem       = {};

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

    return BACKEND_SUCCESS;
}

//==========================================================================================

BackendErr_t ElfBuildSymbolTable(BackendCtx_t* backend_ctx, SymbolTable_t* sym_table)
{
    assert(backend_ctx);
    assert(sym_table);

    BackendErr_t error = BACKEND_SUCCESS;
    
    size_t            symtab_ind = 0;
    SymbolTableElem_t elem       = {};

    // first element should be NULL
    if ((error = SymbolTablePush(sym_table, &elem, &symtab_ind)))
    {
        return error;
    }

    RelTable_t* rel_table = &backend_ctx->rel_table;

    // firstly push only local funcs declarations
    for (size_t i = 0; i < rel_table->size; i++)
    {
        RelElem_t* rel_elem = &rel_table->data[i];

        if (!( (rel_elem->scope == REL_FUNC_LOCAL) &&
               (rel_elem->type  == REL_FUNC_DECL ) ) )
        {
            continue;
        }
        if ((error = ElfBuildSymbolTableElement(rel_elem, sym_table)))
        {
            return error;
        }

        SYMBOL_TABLE_DUMP_(sym_table, L"put elem %ls", rel_table->data[i].label);
    }

    sym_table->last_local_index_plus_one = sym_table->size;

    // now don't push locals
    // push only if call extern or if decl main
    for (size_t i = 0; i < rel_table->size; i++)
    {
        RelElem_t* rel_elem = &rel_table->data[i];

        if (!((rel_elem->scope == REL_FUNC_GLOBAL && rel_elem->type == REL_FUNC_DECL) ||
               rel_elem->scope == REL_FUNC_EXTERN))
        {
            continue;
        }
        if ((error = ElfBuildSymbolTableElement(rel_elem, sym_table)))
        {
            return error;
        }
        
        SYMBOL_TABLE_DUMP_(sym_table, L"put elem %ls", rel_table->data[i].label);
    }

    REL_TABLE_DUMP_(L"placed symtab_indexes");

    return BACKEND_SUCCESS;
}

//==========================================================================================
