#ifndef BIN_CODE_BUFFER
#define BIN_CODE_BUFFER

//——————————————————————————————————————————————————————————————————————————————————————————

#include <stdlib.h>
#include <stdint.h>
#include "common.h"

//——————————————————————————————————————————————————————————————————————————————————————————

const size_t BIN_CODE_INIT_CAPACITY = 4096;
const size_t DISP_SIZE              = 4;

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
    BIN_CODE_MEMALLOC_ERROR,
    BIN_CODE_CHANGE_EXCEEDS_SIZE
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

size_t       BinCodeGetCurrentPos(BinCode_t* bin_code);
BinCodeErr_t BinCodeCtor         (BinCode_t* bin_code, size_t init_capacity);
BinCodeErr_t BinCodeWrite        (BinCode_t* bin_code, const void* src, size_t size);
BinCodeErr_t BinCodeChange       (BinCode_t* bin_code, size_t pos, const void* src, size_t size);
BinCodeErr_t BinAddToDisplacement(BinCode_t* bin_code, size_t disp_pos, int disp_add);
void         BinCodeDtor         (BinCode_t* bin_code);
void         BinCodeDump         (BinCode_t* bin_code, const char* func, const char* file, int line);

//——————————————————————————————————————————————————————————————————————————————————————————

#endif /* BIN_CODE_BUFFER */