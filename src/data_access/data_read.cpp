#include "data_read.h"

//------------------------------------------------------------------------------------------

static MathErr_t MathParseFuncParams(Expr_t* expr, FuncParams_t* params);

//------------------------------------------------------------------------------------------

MathErr_t TreeReadInputData(Tree_t* tree, FuncParams_t* params)
{
    char file_path[MAX_FILENAME_LEN] = {};

    printf("Enter database file path: ");

    if (scanf("%s", file_path) != 1)
    {
        PRINTERR("scanf failed");
        return MATH_INVALID_INPUT;
    }

    getchar();

    return TreeReadData(math_ctx, file_path, params);
}

//------------------------------------------------------------------------------------------

MathErr_t TreeReadData(Tree_t* tree, const char* data_file_path, FuncParams_t* params)
{
    DEBUG_TREE_CHECK(math_ctx, "ERROR BEFORE TREE READ DATA");

    assert(data_file_path != NULL);

    DPRINTF("Reading file %s\n", data_file_path);

    FILE* fp = fopen(data_file_path, "r");

    if (fp == NULL)
    {
        PRINTERR("Error with opening file: %s", data_file_path);
        return MATH_FILE_ERROR;
    }

    Expr_t expr = {};

    if (ReadFile(fp, &expr.buffer, data_file_path))
        return MATH_FILE_ERROR;

    expr.cur_p = expr.buffer;

    MathErr_t error = MATH_SUCCESS;

    if ((error = MathParseText(math_ctx, &expr)))
    {
        free(expr.buffer);
        return error;
    }

    if (MathParseFuncParams(&expr, params))
    {
        free(expr.buffer);
        return MATH_FILE_ERROR;
    }

    free(expr.buffer);

    return MATH_SUCCESS;
}

//------------------------------------------------------------------------------------------

static MathErr_t MathParseFuncParams(Expr_t* expr, FuncParams_t* params)
{
    assert(params != NULL);
    assert(expr   != NULL);

    DPRINTF("expr->buffer = %p;\nexpr->cur_p = %p;\n", expr->buffer, expr->cur_p);
    DPRINTF("expr->cur_p = %s;\n", expr->cur_p);
    int result = 0;

    if ((result = sscanf(expr->cur_p,
               " taylor around %lg to a %d power\n"
               "graphic %s from %lg to %lg\n"
               "graphic f from %lg to %lg ",
               &params->taylor_point,
               &params->taylor_degree,
               params->diff_var,
               &params->x_left,
               &params->x_right,
               &params->y_left,
               &params->y_right)) != 7)
    {
        PRINTERR("File doesn't match format:"
                 "taylor around <point> to a <degree> power\n"
                 "graphic <arg> from <left_boundary> to <right_boundary>\n"
                 "graphic f(<arg>) from <left_boundary> to <right_boundary>\n"
                 "read: %s\nresult = %d\n", expr->cur_p, result);

        return MATH_FILE_ERROR;
    }

    return MATH_SUCCESS;
}

//------------------------------------------------------------------------------------------

int ReadFile(FILE* fp, char** buffer_ptr, const char* file_path)
{
    assert(buffer_ptr != NULL);
    assert(file_path  != NULL);
    assert(fp         != NULL);

    size_t size = 0;

    if (CountSize(file_path, &size))
    {
        return 1;
    }

    char* buffer = (char*) calloc(size, 1);

    if (buffer == NULL)
    {
        PRINTERR("Memory allocation failed");
        return 1;
    }

    if (fread(buffer, size - 1, 1, fp) != 1)
    {
        PRINTERR("Reading file error");
        return 1;
    }

    buffer[size - 1] = '\0'; /* set null-term */

    DPRINTF("buffer = %s;\n", buffer);

    *buffer_ptr = buffer;

    return 0;
}

//------------------------------------------------------------------------------------------

int CountSize(const char* file_path, size_t* size)
{
    struct stat fileinfo = {};

    if (stat(file_path, &fileinfo) == -1)
    {
        PRINTERR("Error with stat()");
        return 1;
    }

    *size = (size_t) fileinfo.st_size + 1;

    return 0;
}

//------------------------------------------------------------------------------------------
