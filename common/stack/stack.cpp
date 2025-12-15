#include "stack.h"

//------------------------------------------------------------------------------------------

StackErr_t StackCtor(Stack_t* stack, size_t capacity)
{
    if (stack == NULL)
    {
        WDPRINTF(L"<Stack is null pointer>");
        return STACK_IS_NULL;
    }

    if (capacity < STACK_MIN_CAPACITY)
    {
        capacity = STACK_MIN_CAPACITY;
    }

    stack->capacity = capacity;

    if (stack->capacity > STACK_SIZE_LIMIT)
    {
        return STACK_CAPACITY_EXCEEDS_LIMIT;
    }

#ifdef CANARY
    capacity += 2;
#endif

    item_t* data = (item_t*) calloc(capacity, sizeof(item_t));
    if (data == NULL)
    {
        WDPRINTF(L"Memory allocation failed");
        return STACK_CALLOC_ERROR;
    }
    stack->data = data;
    stack->size = 0;

    size_t start_poison = 0;
    size_t end_poison = capacity;

#ifdef CANARY
    data[0] = CANARY_VALUE;
    data[capacity - 1] = CANARY_VALUE;
    start_poison++;
    end_poison--;
#endif

    for (size_t i = start_poison; i < end_poison; i++)
    {
        data[i] = POISON;
    }

#ifdef HASH
    stack->hash = StackHash(stack);
#endif

    STACK_OK(stack, reason_end);

    return STACK_SUCCESS;
}

//------------------------------------------------------------------------------------------

StackErr_t StackReallocUp(Stack_t* stack)
{
    STACK_OK(stack, reason_start);

    size_t old_capacity = stack->capacity;
    size_t capacity = old_capacity * 2;
    stack->capacity = capacity;

#ifdef CANARY
    capacity += 2;
#endif

    item_t* data = (item_t*) realloc(stack->data,
                                     (capacity) * sizeof(item_t));
    if (data == NULL)
    {
        DPRINTF(L"Memory reallocation up failed");
        return STACK_REALLOC_ERROR;
    }
    stack->data = data;

    size_t start_poison = old_capacity;
    size_t end_poison = capacity;

#ifdef CANARY
    stack->data[0] = CANARY_VALUE;
    stack->data[capacity - 1] = CANARY_VALUE;
    start_poison++;
    end_poison--;
#endif

    for (size_t i = start_poison; i < end_poison; i++)
    {
        stack->data[i] = POISON;
    }

#ifdef HASH
    stack->hash = StackHash(stack);
#endif

    STACK_OK(stack, reason_end);

    return STACK_SUCCESS;
}

//------------------------------------------------------------------------------------------

StackErr_t StackReallocDown(Stack_t* stack)
{
    STACK_OK(stack, reason_start);

    size_t capacity = stack->capacity / 2;
    stack->capacity = capacity;

#ifdef CANARY
    capacity += 2;
#endif

    stack->data = (item_t*) realloc(stack->data,
                                    capacity * sizeof(item_t));
    if (stack->data == NULL)
    {
        DPRINTF(L"Memory reallocation down failed");
        return STACK_REALLOC_ERROR;
    }

#ifdef CANARY
    stack->data[capacity - 1] = CANARY_VALUE;
#endif

#ifdef HASH
    stack->hash = StackHash(stack);
#endif

    STACK_OK(stack, reason_end);

    return STACK_SUCCESS;
}

//------------------------------------------------------------------------------------------

StackErr_t StackPush(Stack_t* stack, item_t item)
{
    STACK_OK(stack, reason_start);

    if (stack->size == stack->capacity)
    {
        if (StackReallocUp(stack) != STACK_SUCCESS)
        {
            return STACK_REALLOC_ERROR;
        }
    }

#ifdef CANARY
    stack->data[1 + stack->size++] = item;
#else
    stack->data[stack->size++] = item;
#endif

    // DPRINTF("pushed = " SPEC "\n", item);

#ifdef HASH
    stack->hash = StackHash(stack);
#endif

    STACK_OK(stack, reason_end);

    return STACK_SUCCESS;
}

//------------------------------------------------------------------------------------------

StackErr_t StackPop(Stack_t* stack, item_t* item)
{
    STACK_OK(stack, reason_start);

    if (stack->size == 0)
    {
        DPRINTF(L"<Pop is impossible with zero size>\n");
        return STACK_SIZE_IS_ZERO;
    }

#ifdef CANARY
    *item = stack->data[--stack->size + 1];
    stack->data[stack->size + 1] = POISON;
#else
    *item = stack->data[--stack->size];
    stack->data[stack->size] = POISON;
#endif

#ifdef HASH
    stack->hash = StackHash(stack);
#endif

    if (stack->size < stack->capacity / 4)
    {
        if (StackReallocDown(stack) != STACK_SUCCESS)
        {
            return STACK_REALLOC_ERROR;
        }
    }

    STACK_OK(stack, reason_end);

    return STACK_SUCCESS;
}

//------------------------------------------------------------------------------------------

StackErr_t StackGetElement(Stack_t* stack, size_t index, item_t* element)
{
    STACK_OK(stack, reason_start);

    if (element == NULL)
    {
        DPRINTF(L"Error: nullptr given");
        return STACK_IS_NULL;
    }

    if (index >= stack->capacity)
    {
        DPRINTF(L"Given index is bigger than stack size");
        return STACK_INDEX_TOOBIG;
    }

    DPRINTF(L"elem = %p\n", element);
    *element = stack->data[index];

    return STACK_SUCCESS;
}

//------------------------------------------------------------------------------------------

StackErr_t StackSetElement(Stack_t* stack, size_t index, item_t element)
{
    STACK_OK(stack, reason_start);

    if (index >= stack->capacity)
    {
        DPRINTF(L"Given index is bigger than stack size");
        return STACK_INDEX_TOOBIG;
    }

    stack->data[index] = element;

    STACK_OK(stack, reason_end);

    return STACK_SUCCESS;
}

//------------------------------------------------------------------------------------------

size_t StackSize(Stack_t* stack)
{
    return stack->size;
}

//------------------------------------------------------------------------------------------

StackErr_t StackDtor(Stack_t* stack)
{
    if (stack == NULL)
    {
        DPRINTF(L"Null stack pointer given to the stack destructor\n");
        return STACK_IS_NULL;
    }
    if (stack->data == NULL)
    {
        DPRINTF(L"Null stack.data pointer given to the stack destructor\n");
        return STACK_DATA_IS_NULL;
    }

    free(stack->data);
    stack->data = NULL;

    return STACK_SUCCESS;
}

//------------------------------------------------------------------------------------------

#ifdef STACK_DEBUG
StackErr_t StackIsOk(Stack_t* stack,
                     const char* file_name,
                     const char* function,
                     int line,
                     const char* reason_of_calling)
{
    StackErr_t error = STACK_SUCCESS;
    StackErr_t dump_return = STACK_SUCCESS;
    if ((error = StackVerify(stack)) != STACK_SUCCESS)
    {
        WDPRINTF(L"<ERROR OCCURRED, CHECK STACK.LOG>\n");
        if (error != STACK_IS_NULL)
        {
            stack->var_info.file_name = file_name;
            stack->var_info.function = function;
            stack->var_info.line = line;
        }
        if ((dump_return = StackDump(stack, error, reason_of_calling)) \
                         != STACK_SUCCESS)
        {
            return dump_return;
        }
    }

    return error;
}

//------------------------------------------------------------------------------------------

int StackErrToStr(StackErr_t error, const char** line)
{
    switch (error)
    {
        case STACK_SUCCESS:
            *line = "No errors occurred";
            return 0;
        case STACK_IS_NULL:
            *line = "Pointer to the stack structure is NULL";
            return 0;
        case STACK_DATA_IS_NULL:
            *line = "Pointer to the stack data is NULL";
            return 0;
        case STACK_SIZE_EXCEEDS_LIMIT:
            *line = "Size exceeded limit (possibly set to the negative value)";
            return 0;
        case STACK_CAPACITY_EXCEEDS_LIMIT:
            *line = "Capacity exceeded limit (possibly set to the negative value)";
            return 0;
        case STACK_SIZE_EXCEEDS_CAPACITY:
            *line = "Size exceeded capacity";
            return 0;
        case STACK_CALLOC_ERROR:
            *line = "Memory allocation with calloc failed";
            return 0;
        case STACK_REALLOC_ERROR:
            *line = "Memory reallocation failed";
            return 0;
        case STACK_START_CANARY_RUINED:
            *line = "Start boundary value was changed";
            return 0;
        case STACK_END_CANARY_RUINED:
            *line = "End boundary value was changed";
            return 0;
        case STACK_FILE_OPENING_ERROR:
            *line = "Opening the log file failed";
            return 0;
        case STACK_SIZE_IS_ZERO:
            *line = "Size equals zero";
            return 0;
        case STACK_HASH_CHANGED:
            *line = "Hash changed it's value";
            return 0;
        case STACK_INDEX_TOOBIG:
            *line = "Given index exceeds size";
            return 0;
        default:
            return 1;
    }

    return 1;
}

//------------------------------------------------------------------------------------------

StackErr_t StackDump(Stack_t* stack, StackErr_t error,
                     const char* reason_of_calling)
{
    assert(reason_of_calling != NULL);

    DPRINTF(L"Dumping...\n");

    FILE* stream = fopen(STACK_LOGFILENAME, "a");
    if (stream == NULL)
    {
        DPRINTF(L"Can not open stream: stack.log");
        return STACK_FILE_OPENING_ERROR;
    }

    const char* error_str = NULL;
    StackErrToStr(error, &error_str);

    if (error == STACK_IS_NULL)
    {
        DPRINTF(L"Stack is null activated\n");
        fprintf(stream, "%s\n", error_str);
        fclose(stream);
        return STACK_SUCCESS;
    }

    fprintf(stream, "reason: %s\n"
                    "from %s at %s:%d\n"
                    "ERROR %d: %s\n"
                    "stack: %s [%p]:\n{\n"
                    "\tsize = %zu;\n"
                    "\tcapacity = %zu;\n"
#ifdef HASH
                    "\thash = %zu;\n"
#endif
                    "\tdata [%p]:\n\t{\n",
                    reason_of_calling,
                    stack->var_info.function,
                    stack->var_info.file_name,
                    stack->var_info.line,
                    error, error_str,
                    stack->var_info.struct_name, stack,
                    stack->size,
                    stack->capacity,
#ifdef HASH
                    stack->hash,
#endif
                    stack->data);

    if (error == STACK_DATA_IS_NULL)
    {
        DPRINTF(L"Stack.data is null activated\n");
        fprintf(stream, "\t\t----------------\n"
                        "\t}\n"
                        "}\n");
        fclose(stream);
        return STACK_SUCCESS;
    }

    size_t size = stack->size;
    size_t capacity = stack->capacity;
    item_t* data = stack->data;

    // if size/capacity is wrong, output only 16 first elements
    if (error == STACK_SIZE_EXCEEDS_LIMIT)
    {
        size = 16;
    }
    if (error == STACK_CAPACITY_EXCEEDS_LIMIT)
    {
        capacity = size;
    }
    if (error == STACK_SIZE_EXCEEDS_CAPACITY)
    {
        size = capacity - 1;
    }

    size_t start_index = 0;
    size_t end_index = capacity;

#ifdef CANARY
    fprintf(stream, "\t\t [0] = " SPEC " (CANARY);\n", data[0]);
    start_index++;
    end_index++;
#endif /* CANARY */

    for (size_t i = start_index; i < size + 1; i++)
    {
        if (data[i] == POISON)
        {
            size = i - 1;
            break;
        }
        fprintf(stream, "\t\t*[%zu] = " SPEC ";\n", i, data[i]);
    }
    for (size_t j = size + 1; j < end_index; j++)
    {
        fprintf(stream, "\t\t [%zu] = " SPEC " (POISON);\n",
                        j, stack->data[j]);
    }

#ifdef CANARY
    if (error != STACK_CAPACITY_EXCEEDS_LIMIT)
    {
        fprintf(stream, "\t\t [%zu] = " SPEC " (CANARY);\n",
                        capacity + 1, stack->data[capacity + 1]);
    }
#endif /* CANARY */

    fprintf(stream, "\t}\n"
                    "}");

    fclose(stream);
    return STACK_SUCCESS;
}

//------------------------------------------------------------------------------------------

StackErr_t StackVerify(Stack_t* stack)
{
    if (stack == NULL)
    {
        return STACK_IS_NULL;
    }
    if (stack->data == NULL)
    {
        return STACK_DATA_IS_NULL;
    }
    if (stack->capacity > STACK_SIZE_LIMIT)
    {
        return STACK_CAPACITY_EXCEEDS_LIMIT;
    }
    if (stack->size > STACK_SIZE_LIMIT)
    {
        return STACK_SIZE_EXCEEDS_LIMIT;
    }
    if (stack->size > stack->capacity)
    {
        return STACK_SIZE_EXCEEDS_CAPACITY;
    }

#ifdef CANARY
    if (stack->data[0] != CANARY_VALUE)
    {
        return STACK_START_CANARY_RUINED;
    }
    if (stack->data[stack->capacity + 1] != CANARY_VALUE)
    {
        return STACK_END_CANARY_RUINED;
    }
#endif /* CANARY */

#ifdef HASH
    if (StackHash(stack) != stack->hash)
    {
        return STACK_HASH_CHANGED;
    }
#endif /* HASH */

    return STACK_SUCCESS;
}
#endif /* STACK_DEBUG */

//------------------------------------------------------------------------------------------

#ifdef CANARY
StackErr_t StackCheckCanaries(Stack_t* stack)
{
    assert(stack != NULL);
    assert(stack->data != NULL);

    if (stack->data[0] != CANARY_VALUE)
    {
        fprintf(stderr, "Start canary has changed, stack is now ruined\n");
        return STACK_START_CANARY_RUINED;
    }
    if (stack->data[stack->capacity + 1] != CANARY_VALUE)
    {
        fprintf(stderr, "End canary has changed, stack is now ruined\n");
        return STACK_END_CANARY_RUINED;
    }

    return STACK_SUCCESS;
}
#endif /* CANARY */

//------------------------------------------------------------------------------------------

#ifdef HASH
StackErr_t StackCheckHash(Stack_t* stack)
{
    assert(stack != NULL);
    assert(stack->data != NULL);

    if (stack->hash != StackHash(stack))
    {
        fprintf(stderr, "Hash has changed, stack is now ruined\n");
        return STACK_HASH_CHANGED;
    }

    return STACK_SUCCESS;
}
#endif /* HASH */

//------------------------------------------------------------------------------------------

#ifdef HASH
// djb2
size_t StackHash(Stack_t* stack)
{
    assert(stack != NULL);
    assert(stack->data != NULL);

    char* str = (char*) stack->data;
    size_t hash = 5381;

    char ch = 0;
    size_t i = 0;
    size_t str_len = (stack->capacity + 2) * sizeof(item_t) / sizeof(char);

    while (i < str_len)
    {
        ch = str[i++];
        hash = ((hash << 5) + hash) + ch;
    }

    return hash;
}
#endif /* HASH */

//------------------------------------------------------------------------------------------

#ifdef STACK_DEBUG

StackErr_t StackPrint(Stack_t* stack)
{
    STACK_OK(stack, reason_start);

    size_t end_index = stack->capacity - 1;

#ifdef CANARY
    if (stack->data[0] == CANARY_VALUE)
        printf("[cnry, ");
    else
        printf("[" SPEC ", ", stack->data[0]);
    end_index += 2;
#else
    if (stack->data[0] == POISON)
    {
        printf("[*, ");
    }
    else
    {
        printf("[" SPEC ", ", stack->data[0]);
    }
#endif /* CANARY */

    for (size_t i = 1; i < end_index; i++)
    {
        if (stack->data[i] == POISON)
            printf("*, ");
        else
            printf(SPEC ", ", stack->data[i]);
    }

#ifdef CANARY
    if (stack->data[stack->capacity + 1] == CANARY_VALUE)
        printf("cnry]\n");
    else
        printf(SPEC "]\n", stack->data[end_index]);
#else
    if (stack->data[end_index] == POISON)
    {
        printf("*]\n");
    }
    else
    {
        printf(SPEC "]\n", stack->data[end_index]);
    }
#endif /* CANARY */

#ifdef HASH
    printf("hash = %zu\n", stack->hash);
#endif

    return STACK_SUCCESS;
}
#endif

//------------------------------------------------------------------------------------------
