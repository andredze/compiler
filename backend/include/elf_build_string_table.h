#ifndef ELF_BUILD_STRING_TABLE_H
#define ELF_BUILD_STRING_TABLE_H

//——————————————————————————————————————————————————————————————————————————————————————————

#include "backend.h"
#include <stdlib.h>
#include <elf.h>

//——————————————————————————————————————————————————————————————————————————————————————————
// stack for verifying if string was already put in table
typedef struct StringPoolElem
{
    size_t         w_string_hash;
    const wchar_t* w_string;
    size_t         strtab_index;
}
StringPoolElem_t;

//——————————————————————————————————————————————————————————————————————————————————————————

typedef struct StringPool
{
    size_t capacity;
    size_t size;

    StringPoolElem* data;
}
StringPool_t;

//——————————————————————————————————————————————————————————————————————————————————————————

size_t CountHash(const wchar_t* string);

//------------------------------------------------------------------//

#ifdef BACKEND_DEBUG
    #define STRING_POOL_TABLE_DUMP_(str_pool, fmt, ...)       \
            BEGIN                                             \
            WDPRINTF(fmt L"\n", ##__VA_ARGS__);               \
            if (StringPoolDump(backend_ctx,                   \
                               str_pool,                      \
                               __func__, __FILE__, __LINE__,  \
                               fmt, ##__VA_ARGS__))           \
            {                                                 \
                return BACKEND_LANG_ERROR;                    \
            }                                                 \
            if (StringTableDump(backend_ctx,                  \
                               __func__, __FILE__, __LINE__,  \
                               fmt, ##__VA_ARGS__))           \
            {                                                 \
                return BACKEND_LANG_ERROR;                    \
            }                                                 \
            END
#else
    #define STRING_POOL_TABLE_DUMP_(str_pool, fmt, ...)   ;
#endif /* BACKEND_DEBUG */

//------------------------------------------------------------------//

BackendErr_t StringPoolDump(BackendCtx_t*  backend_ctx, 
                            StringPool_t*  str_pool,
                            const char*    func,
                            const char*    file,
                            int            line,
                            const wchar_t* fmt,
                            ...);

BackendErr_t StringTableDump(BackendCtx_t*  backend_ctx, 
                             const char*    func,
                             const char*    file,
                             int            line,
                             const wchar_t* fmt,
                             ...);

//------------------------------------------------------------------//

BackendErr_t StringPoolCtor          (StringPool_t* str_pool, size_t init_cap);

BackendErr_t StringPoolGetStrTabIndex(StringPool_t*  str_pool, 
                                      const wchar_t* string, 
                                      size_t         hash,
                                      size_t*        str_tab_index_dst);

BackendErr_t StringPoolPushString(StringPool_t*  str_pool, 
                                  const wchar_t* string, 
                                  size_t         hash, 
                                  size_t         strtab_index);

void         StringPoolDtor          (StringPool_t* str_pool);

//——————————————————————————————————————————————————————————————————————————————————————————

const size_t MAX_CHAR_BUFFER_SIZE = 256;

//——————————————————————————————————————————————————————————————————————————————————————————

BackendErr_t StringTableCtor       (StringTable_t* string_table, size_t init_cap);

BackendErr_t StringTableWriteString(StringTable_t* str_table, 
                                    const char*    string, 
                                    size_t         length,
                                    size_t*        str_tab_index_dst);

BackendErr_t StringTableWriteSymbol(StringTable_t* string_table, char  symbol);
void         StringTableDtor       (StringTable_t* string_table);

//——————————————————————————————————————————————————————————————————————————————————————————

BackendErr_t ElfBuildStringTable(BackendCtx_t* backend_ctx);

//——————————————————————————————————————————————————————————————————————————————————————————

#endif /* ELF_BUILD_STRING_TABLE_H */