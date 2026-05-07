#ifndef ID_TYPES_H
#define ID_TYPES_H

//——————————————————————————————————————————————————————————————————————————————————————————

#include <stdlib.h>
#include <wchar.h>

//——————————————————————————————————————————————————————————————————————————————————————————

#define LANG_NUM_SPEC L"%d"

typedef int Number_t;

//——————————————————————————————————————————————————————————————————————————————————————————

typedef enum IdType
{
    ID_TYPE_UNKNOWN  = 0,
    ID_TYPE_VARIABLE = 1,
    ID_TYPE_FUNCTION = 2
} IdType_t;

//——————————————————————————————————————————————————————————————————————————————————————————

typedef struct IdData
{
    size_t    name_index;

    wchar_t*  name;

    IdType_t  type;

    size_t    memory_needed;
    size_t    n_params;

    int       addr;

} IdData_t;

//——————————————————————————————————————————————————————————————————————————————————————————

typedef struct NamesPool
{
    wchar_t** data;

    size_t    size;
    size_t    capacity;

} NamesPool_t;

//——————————————————————————————————————————————————————————————————————————————————————————

typedef struct IdTable
{
    IdData_t* data;

    size_t    size;
    size_t    capacity;

} IdTable_t;

//——————————————————————————————————————————————————————————————————————————————————————————

#endif /* ID_TYPES_H */