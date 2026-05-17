#include "elf_build_string_table.h"

//==========================================================================================

size_t CountHash(const wchar_t* string)
{
    assert(string);

    size_t hash = 5381;

    uint32_t ch = 0;

    while ((ch = (uint32_t) *string++) != L'\0')
    {
        hash = (hash << 5) + hash + ch;
    }

    return hash;
}

//==========================================================================================

BackendErr_t StringPoolCtor(StringPool_t* str_pool, size_t init_cap)
{
    assert(str_pool);

    str_pool->size     = 0;
    str_pool->capacity = init_cap;

    str_pool->data = (StringPoolElem_t*) calloc (str_pool->capacity, 
                                                 sizeof(StringPoolElem_t));

    if (str_pool->data == NULL)
    {
        WPRINTERR(L"Failed memory allocation for string pool");
        return BACKEND_MEMALLOC_ERROR;
    }

    return BACKEND_SUCCESS;
}

//==========================================================================================

void StringPoolDtor(StringPool_t* str_pool)
{
    assert(str_pool);

    str_pool->size     = 0;
    str_pool->capacity = 0;

    free(str_pool->data);
    str_pool->data = NULL;
}

//==========================================================================================

BackendErr_t StringPoolPushString(StringPool_t*  str_pool, 
                                  const wchar_t* string, 
                                  size_t         hash, 
                                  size_t         strtab_index)
{
    assert(str_pool);
    assert(string);

    if (str_pool->size + 1 >= str_pool->capacity)
    {
        WPRINTERR(L"Can not push to string pool, cap limit");
        return BACKEND_STR_POOL_LIMIT;
    }

    str_pool->data[str_pool->size++] = {.w_string_hash = hash,
                                        .w_string      = string,
                                        .strtab_index  = strtab_index};

    return BACKEND_SUCCESS;
}

//==========================================================================================

BackendErr_t StringPoolGetStrTabIndex(StringPool_t*  str_pool, 
                                      const wchar_t* string, 
                                      size_t         hash,
                                      size_t*        str_tab_index_dst)
{
    assert(str_tab_index_dst);
    assert(str_pool);

    size_t str_pool_size = str_pool->size;

    for (size_t i = 0; i < str_pool_size; i++)
    {
        if (str_pool->data[i].w_string_hash == hash &&
            wcscmp(string, str_pool->data[i].w_string) == 0)
        {
            *str_tab_index_dst = str_pool->data[i].strtab_index;
            return BACKEND_SUCCESS;
        }
    }

    *str_tab_index_dst = (size_t)-1;

    return BACKEND_SUCCESS;
}

//==========================================================================================

BackendErr_t StringTableCtor(StringTable_t* str_table, size_t init_cap)
{
    assert(str_table);

    str_table->size     = 0;
    str_table->capacity = init_cap;

    str_table->data = (uint8_t*) calloc (str_table->capacity, sizeof(uint8_t));

    if (str_table->data == NULL)
    {
        WPRINTERR(L"Failed memory allocation for string table");
        return BACKEND_MEMALLOC_ERROR;
    }

    return BACKEND_SUCCESS;
}

//==========================================================================================

void StringTableDtor(StringTable_t* str_table)
{
    assert(str_table);

    str_table->size     = 0;
    str_table->capacity = 0;

    free(str_table->data);
    str_table->data = NULL;
}

//==========================================================================================

static BackendErr_t StringTableRealloc(StringTable_t* str_table)
{
    assert(str_table);

    size_t new_cap = str_table->capacity * 2 + 1;

    uint8_t* new_data = (uint8_t*) realloc(str_table->data, sizeof(uint8_t) * new_cap);

    if (new_data == NULL)
    {
        WPRINTERR(L"Failed mem realloc for string table");
        return BACKEND_MEMALLOC_ERROR;
    }

    str_table->data     = new_data;
    str_table->capacity = new_cap;

    return BACKEND_SUCCESS;
}

//==========================================================================================

BackendErr_t StringTableWriteSymbol(StringTable_t* str_table, char symbol)
{
    assert(str_table);

    BackendErr_t error = BACKEND_SUCCESS;

    if (str_table->size + 1 >= str_table->capacity)
    {
        if ((error = StringTableRealloc(str_table)))
        {
            return error;
        }
    }

    str_table->data[str_table->size++] = (uint8_t) symbol;

    return BACKEND_SUCCESS;
}


//==========================================================================================

BackendErr_t StringTableWriteString(StringTable_t* str_table, 
                                    const char*    string, 
                                    size_t         length,
                                    size_t*        str_tab_index_dst)
{
    assert(str_table);
    assert(string);

    BackendErr_t error = BACKEND_SUCCESS;

    if (str_table->size + length + 1 >= str_table->capacity)
    {
        if ((error = StringTableRealloc(str_table)))
        {
            return error;
        }
    }

    if (str_tab_index_dst)
    {
        *str_tab_index_dst = str_table->size;
    }

    // including NULL-term
    memcpy(&str_table->data[str_table->size], string, length + 1);

    WDPRINTF(L"StringTableWrite %s\n", &str_table->data[str_table->size]);

    str_table->size += (length + 1);

    return BACKEND_SUCCESS;
}

//==========================================================================================

BackendErr_t StringPoolDump(BackendCtx_t*  backend_ctx, 
                            StringPool_t*  str_pool,
                            const char*    func,
                            const char*    file,
                            int            line,
                            const wchar_t* fmt,
                            ...)
{
    assert(backend_ctx);
    assert(str_pool);

    FILE* fp = backend_ctx->lang_ctx.tree.debug.fp;

    va_list args = {};

    va_start(args, fmt);

    fwprintf(fp, L"<h4><font color=blue>"
                 L"Dump StringPool_t %p called from %s at %s:%d"
                 L" Message: ",
                 str_pool,
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
             str_pool->size,
             str_pool->capacity,
             str_pool->data);

    StringPoolElem_t* str_elem = NULL;

    fwprintf(fp, L"index  {%25ls, %20ls, %15ls}\n",
                 L"hash", L"string", L"strtab_index");

    for (size_t i = 0; i < str_pool->size; i++)
    {
        str_elem = &str_pool->data[i];

        fwprintf(fp, L"[ %-2d]: {%25zu, %20ls, %15zu}\n",
                     i,
                     str_elem->w_string_hash,
                     str_elem->w_string,
                     str_elem->strtab_index);
    }

    fwprintf(fp, L"---------------------------------------"
                 L"---------------------------------------\n\n");
    
    fflush(fp);

    return BACKEND_SUCCESS;
}

//==========================================================================================

BackendErr_t StringTableDump(BackendCtx_t*  backend_ctx, 
                             StringTable_t* str_table,
                             const char*    func,
                             const char*    file,
                             int            line,
                             const wchar_t* fmt,
                             ...)
{
    assert(backend_ctx);
    assert(str_table);

    FILE* fp = backend_ctx->lang_ctx.tree.debug.fp;

    va_list args = {};

    va_start(args, fmt);

    fwprintf(fp, L"<h4><font color=blue>"
                 L"Dump StringTable_t %p called from %s at %s:%d"
                 L" Message: ",
                 str_table,
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
             str_table->size,
             str_table->capacity,
             str_table->data);

    fwprintf(fp, L"hex buffer dump:\n");

    for (size_t i = 1; i < str_table->size + 1; i++)
    {
        fwprintf(fp, L"%02x ", str_table->data[i - 1]);
    
        if (i % 16 == 0)
        {
            fwprintf(fp, L"\n");
        }
    }

    fwprintf(fp, L"\n");
    fwprintf(fp, L"char buffer dump:\n");

    for (size_t i = 1; i < str_table->size + 1; i++)
    {
        wchar_t wch = 0;

        mbtowc(&wch, (char*) &str_table->data[i - 1], 1);

        if (str_table->data[i - 1] == 0)
        {
            fwprintf(fp, L"00 ");
        }
        else
        {
            fwprintf(fp, L"%lc ", wch);
        }

        if (i % 16 == 0)
        {
            fwprintf(fp, L"\n");
            WDPRINTF(L"\n");
        }
    }

    fwprintf(fp, L"\n");

    fwprintf(fp, L"---------------------------------------"
                 L"---------------------------------------\n\n");
    
    fflush(fp);

    return BACKEND_SUCCESS;
}

//==========================================================================================

static size_t ConvertWstringToString(char* buffer, const wchar_t* w_string, size_t buf_size)
{
    assert(buffer);
    assert(w_string);

    size_t i = 0;

    for (; (i < buf_size - 1) && (*w_string != L'\0'); i++)
    {
        *(buffer++) = (char) *(w_string++); 
    }

    buffer[i] = '\0';

    return i;
}

//==========================================================================================

static BackendErr_t StringTablePutRelElem(StringTable_t* str_table,
                                          StringPool_t*  str_pool,
                                          RelElem_t*     rel_elem)
{
    assert(str_table);
    assert(str_pool);
    assert(rel_elem);

    BackendErr_t error = BACKEND_SUCCESS;

    const wchar_t* w_string     = rel_elem->label;
    size_t         hash         = CountHash(w_string);
    size_t         strtab_index = (size_t) -1;

    if ((error = StringPoolGetStrTabIndex(str_pool, w_string, hash, &strtab_index)))
    {
        return error;
    }
    if (strtab_index != (size_t) -1)
    {
        rel_elem->strtab_index = strtab_index;
        return BACKEND_SUCCESS;
    }

    char buffer[MAX_CHAR_BUFFER_SIZE] = {}; 

    size_t buffer_length = wcstombs(buffer, w_string, sizeof(buffer));

    WDPRINTF(L"Converted wchar to char: buffer %s\n", buffer);

    if ((error = StringTableWriteString(str_table, buffer, buffer_length, &strtab_index)))
    {
        return error;
    }

    rel_elem->strtab_index = strtab_index;

    if ((error = StringPoolPushString(str_pool, w_string, hash, strtab_index)))
    {
        return error;
    }

    return BACKEND_SUCCESS;
}

//==========================================================================================

BackendErr_t ElfBuildStringTable(BackendCtx_t* backend_ctx, StringTable_t* str_table)
{
    assert(backend_ctx);

    BackendErr_t error = BACKEND_SUCCESS;

    // first should be a NULL-term
    if ((error = StringTableWriteSymbol(str_table, '\0')))
    {
        return error;
    }

    StringPool_t str_pool = {};

    if ((error = StringPoolCtor(&str_pool, backend_ctx->rel_table.size + 1)))
    {
        StringPoolDtor(&str_pool);
        return error;
    }

    STRING_POOL_TABLE_DUMP_(&str_pool, str_table, L"ctored");

    RelTable_t* rel_table = &backend_ctx->rel_table;

    for (size_t i = 0; i < rel_table->size; i++)
    {
        if ((error = StringTablePutRelElem(str_table,
                                           &str_pool,
                                           &rel_table->data[i])))
        {
            StringPoolDtor(&str_pool);
            return error;
        }

        STRING_POOL_TABLE_DUMP_(&str_pool, str_table, L"put elem %ls", rel_table->data[i].label);
    }

    REL_TABLE_DUMP_(L"put strtab_indexes");
    
    StringPoolDtor(&str_pool);

    return BACKEND_SUCCESS;
}

//==========================================================================================
