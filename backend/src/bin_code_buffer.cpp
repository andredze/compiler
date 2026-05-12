#include "bin_code_buffer.h"

//==========================================================================================

BinCodeErr_t BinCodeCtor(BinCode_t* bin_code, size_t init_capacity)
{
    assert(bin_code);

    bin_code->size     = 0;
    bin_code->capacity = init_capacity;
    bin_code->buffer   = (uint8_t*) calloc(init_capacity, sizeof(uint8_t));

    if (bin_code->buffer == NULL)
    {
        WPRINTERR(L"Memalloc error");
        return BIN_CODE_MEMALLOC_ERROR;
    }

    return BIN_CODE_SUCCESS;
}

//==========================================================================================

void BinCodeDtor(BinCode_t* bin_code)
{
    assert(bin_code);

    bin_code->size     = 0;
    bin_code->capacity = 0;

    free(bin_code->buffer);
}

//==========================================================================================

static BinCodeErr_t BinCodeRealloc(BinCode_t* bin_code)
{
    assert(bin_code);

    size_t new_capacity = bin_code->capacity * 2 + 1;

    uint8_t* buffer = (uint8_t*) realloc(bin_code->buffer, new_capacity);

    if (buffer == NULL)
    {
        WPRINTERR(L"Mem realloc error for bin_code->buffer");
        return BIN_CODE_MEMALLOC_ERROR;
    }

    return BIN_CODE_SUCCESS;
}

//==========================================================================================

BinCodeErr_t BinCodeWrite(BinCode_t* bin_code, const void* src, size_t size)
{
    assert(bin_code);
    assert(src);

    if (size + bin_code->size >= bin_code->capacity)
    {
        BinCodeErr_t error = BIN_CODE_SUCCESS;

        if ((error = BinCodeRealloc(bin_code)))
        {
            return error;
        }
    }

    memcpy(bin_code->buffer + bin_code->size, src, size);
    bin_code->size += size;

    return BIN_CODE_SUCCESS;
}

//==========================================================================================

void BinCodeDump(BinCode_t* bin_code, const char* func, const char* file, int line)
{
    wcprintf(LIGHT_YELLOW, LR"(
===========================================
Dump: BinCode_t from %s at %s:%d
-------------------------------------------
bin_code = %p
{
  capacity = %zu;
  size     = %zu;
  buffer   = )",
    func,
    file,
    line,
    bin_code,
    bin_code->capacity,
    bin_code->size);

    for (size_t i = 0; i < bin_code->size; i++)
    {
        wcprintf(BLUE, L"%02x ", bin_code->buffer[i]);
    }
    
    wcprintf(LIGHT_YELLOW, LR"(
}
===========================================
)");
}

//==========================================================================================
