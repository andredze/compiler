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
    ID_TYPE_UNKNOWN   = 0,
    ID_TYPE_VARIABLE  = 1,
    ID_TYPE_PARAMETER = 2,
    ID_TYPE_FUNCTION  = 3
} IdType_t;

//------------------------------------------------------------------//

const char * const ID_TYPE_NAMES[] = {
    [ID_TYPE_UNKNOWN  ] = "ID_TYPE_UNKNOWN",
    [ID_TYPE_VARIABLE ] = "ID_TYPE_VARIABLE",
    [ID_TYPE_PARAMETER] = "ID_TYPE_PARAMETER",
    [ID_TYPE_FUNCTION ] = "ID_TYPE_FUNCTION"
};

//——————————————————————————————————————————————————————————————————————————————————————————

typedef struct IdData
{
    size_t    name_index;

    wchar_t*  name;

    IdType_t  type;

    // only for functions
    size_t    n_local_vars;
    size_t    n_params;

    // only for variables
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

    // used for detecting scope
    size_t    current_function;

} IdTable_t;

//——————————————————————————————————————————————————————————————————————————————————————————

#endif /* ID_TYPES_H */