#ifndef BIN_CODE_BUFFER
#define BIN_CODE_BUFFER

//——————————————————————————————————————————————————————————————————————————————————————————

#include <stdlib.h>
#include <stdint.h>
#include "common.h"

//——————————————————————————————————————————————————————————————————————————————————————————

const size_t BIN_CODE_INIT_CAPACITY = 4096;

//——————————————————————————————————————————————————————————————————————————————————————————

typedef struct BinCode
{
    uint8_t* buffer;

    size_t   size;
    size_t   capacity;
}
BinCode_t;

//——————————————————————————————————————————————————————————————————————————————————————————

typedef enum BinCodeErr
{
    BIN_CODE_SUCCESS,
    BIN_CODE_MEMALLOC_ERROR
}
BinCodeErr_t;

//——————————————————————————————————————————————————————————————————————————————————————————

#ifdef ENCODING_DEBUG
    #define BIN_CODE_DUMP(bin_code_)                                    \
            BEGIN                                                       \
            BinCodeDump((bin_code_), __func__, __FILE__, __LINE__);     \
            END
#else
    #define BIN_CODE_DUMP(bin_code_) ;
#endif /* ENCODING_DEBUG */

//------------------------------------------------------------------//

BinCodeErr_t BinCodeCtor (BinCode_t* bin_code, size_t init_capacity);
BinCodeErr_t BinCodeWrite(BinCode_t* bin_code, const void* src, size_t size);
void         BinCodeDtor (BinCode_t* bin_code);
void         BinCodeDump (BinCode_t* bin_code, const char* func, const char* file, int line);

//——————————————————————————————————————————————————————————————————————————————————————————

#endif /* BIN_CODE_BUFFER */