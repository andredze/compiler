#include "elf_build_relocation_table.h"

//==========================================================================================

BackendErr_t RelocationTableCtor(RelocationTable_t* reloc_table, size_t init_cap)
{
    assert(reloc_table);

    reloc_table->size     = 0;
    reloc_table->capacity = init_cap;

    reloc_table->data = (RelocationTableElem_t*) calloc (reloc_table->capacity, 
                                                         sizeof(RelocationTableElem_t));

    if (reloc_table->data == NULL)
    {
        WPRINTERR(L"Failed memory allocation for reloc_table");
        return BACKEND_MEMALLOC_ERROR;
    }

    return BACKEND_SUCCESS;
}

//==========================================================================================

void RelocationTableDtor(RelocationTable_t* reloc_table)
{
    assert(reloc_table);

    reloc_table->size     = 0;
    reloc_table->capacity = 0;

    free(reloc_table->data);
    reloc_table->data = NULL;
}

//==========================================================================================

BackendErr_t RelocationTablePush(RelocationTable_t*     reloc_table, 
                                 RelocationTableElem_t* elem)
{
    assert(reloc_table);
    assert(elem);

    if (reloc_table->size + 1 >= reloc_table->capacity)
    {
        WPRINTERR(L"Can not push to reloc_table, cap limit");
        return BACKEND_RELOC_TABLE_LIMIT;
    }

    reloc_table->data[reloc_table->size++] = *elem;

    return BACKEND_SUCCESS;
}

//==========================================================================================

BackendErr_t RelocationTableDump(BackendCtx_t*      backend_ctx, 
                                 RelocationTable_t* reloc_table,
                                 const char*        func,
                                 const char*        file,
                                 int                line,
                                 const wchar_t*     fmt,
                                 ...)
{
    assert(backend_ctx);
    assert(reloc_table);

    FILE* fp = backend_ctx->lang_ctx.tree.debug.fp;

    va_list args = {};

    va_start(args, fmt);

    fwprintf(fp, L"<h4><font color=blue>"
                 L"Dump RelocationTable_t %p called from %s at %s:%d"
                 L" Message: ",
                 reloc_table,
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
             reloc_table->size,
             reloc_table->capacity,
             reloc_table->data);

    RelocationTableElem_t* reloc_elem = NULL;

    fwprintf(fp, L"index  {%8ls, %8ls, %8ls}\n",
                 L"r_offset", L"r_info", L"r_addend");

    for (size_t i = 0; i < reloc_table->size; i++)
    {
        reloc_elem = &reloc_table->data[i];

        fwprintf(fp, L"[ %-2d]: {%8zu, %8zu, %8d}\n",
                     i,
                     reloc_elem->r_offset,
                     reloc_elem->r_info,
                     reloc_elem->r_addend);
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
  Elf64_Addr	r_offset;		// Address 
  Elf64_Xword	r_info;			// Relocation type and symbol index 
  Elf64_Sxword	r_addend;		// Addend 
} Elf64_Rela;
*/
BackendErr_t RelocationTableBuildElem(RelocationTableElem_t* elem,
                                      size_t                 disp_address,
                                      size_t                 symtab_index)
{
    assert(elem);

    elem->r_offset = disp_address;
    elem->r_info   = ELF64_R_INFO(symtab_index, R_X86_64_PC32); // PC relative 32 bit signed
    elem->r_addend = R_ADDEND;

    return BACKEND_SUCCESS;
}

//==========================================================================================

BackendErr_t ElfBuildRelocationTable(BackendCtx_t*      backend_ctx, 
                                     RelocationTable_t* reloc_table)
{
    assert(reloc_table);
    assert(backend_ctx);

    BackendErr_t error = BACKEND_SUCCESS;

    RelocationTableElem_t elem = {};

    RelTable_t* my_rel_table = &backend_ctx->rel_table;

    for (size_t i = 0; i < my_rel_table->size; i++)
    {
        RelElem_t* my_rel_elem = &my_rel_table->data[i];

        if (!(my_rel_elem->type == REL_FUNC_CALL))
        {
            continue;
        }
        if ((error = RelocationTableBuildElem(&elem,
                                              my_rel_elem->bin_code_pos + CALL_INSTR_SIZE,
                                              my_rel_elem->symtab_index)))
        {
            return error;
        }
        if ((error = RelocationTablePush(reloc_table, &elem)))
        {
            return error;
        }
        
        RELOCATION_TABLE_DUMP_(reloc_table, L"put elem %ls", my_rel_table->data[i].label);
    }

    return BACKEND_SUCCESS;
}

//==========================================================================================
