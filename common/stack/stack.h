#ifndef STACK_H
#define STACK_H

//——————————————————————————————————————————————————————————————————————————————————————————

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "tree_types.h"

#ifndef STACK_DEBUG
    #define NDEBUG

#endif /* STACK_DEBUG */

#include <assert.h>

//==========================================================================================

#ifdef CANARY
    #define CHECK_CANARY(stack)                                          \
        do {                                                             \
            StackErr_t error = STACK_SUCCESS;                            \
            if ((error = StackCheckCanaries(stack)) != STACK_SUCCESS)    \
            {                                                            \
                return error;                                            \
            }                                                            \
        } while (0)

#else
    #define CHECK_CANARY(stack) ;

#endif /* CANARY */

//——————————————————————————————————————————————————————————————————————————————————————————

#ifdef HASH
    #define CHECK_HASH(stack)                           \
        if (StackCheckHash(stack) != STACK_SUCCESS)     \
        {                                               \
            return STACK_HASH_CHANGED;                  \
        }

#else
    #define CHECK_HASH(stack) ;

#endif /* HASH */

//——————————————————————————————————————————————————————————————————————————————————————————

#ifdef STACK_DEBUG
    #define STACK_OK_DEBUG(stack, reason)                                           \
        do {                                                                        \
            StackErr_t error = STACK_SUCCESS;                                       \
            if ((error = StackIsOk(stack, __FILE__, __func__, __LINE__, reason))    \
                        != STACK_SUCCESS)                                           \
            {                                                                       \
                return error;                                                       \
            }                                                                       \
        } while (0)

#else
    #define STACK_OK_DEBUG(stack, reason) ;

#endif /* STACK_DEBUG */

//——————————————————————————————————————————————————————————————————————————————————————————

#define STACK_OK(stack, reason)         \
    STACK_OK_DEBUG(stack, reason);      \
    CHECK_CANARY(stack);                \
    CHECK_HASH(stack);

//——————————————————————————————————————————————————————————————————————————————————————————

#ifdef STACK_DEBUG
    #define STACK_INIT(name) Stack_t name = {.var_info = {#name, __FILE__, __func__, __LINE__}}

#else
    #define STACK_INIT(name) Stack_t name = {}

#endif /* STACK_DEBUG */

//——————————————————————————————————————————————————————————————————————————————————————————

#ifdef STACK_DEBUG
#ifndef DPRINTF
    #define DPRINTF(...) fprintf(stderr, __VA_ARGS__);
#endif
#else
#ifndef DPRINTF
    #define DPRINTF(...) ;
#endif
#endif

//==========================================================================================

const size_t STACK_SIZE_LIMIT   = SIZE_MAX / 32 * 30;
const size_t STACK_MIN_CAPACITY = 32;

const char * const STACK_LOGFILENAME = "stack.log";

//——————————————————————————————————————————————————————————————————————————————————————————

typedef TreeNode_t* item_t;

const item_t POISON = NULL;

#define SPEC "%p"

//——————————————————————————————————————————————————————————————————————————————————————————

#ifdef CANARY
    const int CANARY_VALUE_INT = 0xAB0BADED;
    const item_t CANARY_VALUE = CANARY_VALUE_INT;

#endif /* CANARY */

//——————————————————————————————————————————————————————————————————————————————————————————

const char* const reason_start = "Stack didn't pass verifier at the start of function";
const char* const reason_end   = "Stack didn't pass verifier at the end of function";

//——————————————————————————————————————————————————————————————————————————————————————————

typedef enum StackErr
{
    STACK_SUCCESS                = 0,
    STACK_IS_NULL                = 1,
    STACK_DATA_IS_NULL           = 2,
    STACK_SIZE_EXCEEDS_LIMIT     = 3,
    STACK_CAPACITY_EXCEEDS_LIMIT = 4,
    STACK_SIZE_EXCEEDS_CAPACITY  = 5,
    STACK_START_CANARY_RUINED    = 6,
    STACK_END_CANARY_RUINED      = 7,
    STACK_HASH_CHANGED           = 8,
    STACK_CALLOC_ERROR           = 9,
    STACK_REALLOC_ERROR          = 10,
    STACK_FILE_OPENING_ERROR     = 11,
    STACK_SIZE_IS_ZERO           = 12,
    STACK_INDEX_TOOBIG           = 13
} StackErr_t;

//——————————————————————————————————————————————————————————————————————————————————————————

typedef struct VarInfo
{
    const char* struct_name;
    const char* file_name;
    const char* function;
    int line;
} VarInfo_t;

//——————————————————————————————————————————————————————————————————————————————————————————

typedef struct Stack
{
    item_t* data;
    size_t size;
    size_t capacity;

#ifdef HASH
    size_t hash;
#endif /* HASH */

#ifdef STACK_DEBUG
    VarInfo_t var_info;
#endif /* STACK_DEBUG */

} Stack_t;

//——————————————————————————————————————————————————————————————————————————————————————————

StackErr_t StackGetElement    (Stack_t* stack, size_t index, item_t* element);
StackErr_t StackSetElement    (Stack_t* stack, size_t index, item_t element);
StackErr_t StackCtor          (Stack_t* stack, size_t  capacity);
StackErr_t StackPush          (Stack_t* stack, item_t  item);
StackErr_t StackPop           (Stack_t* stack, item_t* item);
StackErr_t StackReallocUp     (Stack_t* stack);
StackErr_t StackReallocDown   (Stack_t* stack);
StackErr_t StackDtor          (Stack_t* stack);
StackErr_t StackPrint         (Stack_t* stack);
StackErr_t StackCheckCanaries (Stack_t* stack);
StackErr_t StackCheckHash     (Stack_t* stack);
StackErr_t StackVerify        (Stack_t* stack);
size_t     StackHash          (Stack_t* stack);
size_t     StackSize          (Stack_t* stack);

int        StackErrToStr      (StackErr_t error, const char* line[]);
StackErr_t StackDump          (Stack_t* stack, StackErr_t error, const char* reason_of_calling);

StackErr_t StackIsOk          (Stack_t* stack,
                               const char* file_name,
                               const char* function,
                               int line,
                               const char* reason_of_calling);

//——————————————————————————————————————————————————————————————————————————————————————————

#endif /* STACK_H */
