#include "colors.h"
#include <wchar.h>

//------------------------------------------------------------------------------------------

int cprintf(const char * const color, const char* fmt, ...)
{
    va_list args = {};

    va_start(args, fmt);

    int return_value = vcprintf(color, fmt, args);

    va_end(args);

    return return_value;
}

//------------------------------------------------------------------------------------------

int vcprintf(const char * const color, const char* fmt, va_list args)
{
    return vfcprintf(stdout, color, fmt, args);
}

//------------------------------------------------------------------------------------------

int fcprintf(FILE* stream, const char * const color, const char* fmt, ...)
{
    va_list args = {};

    va_start(args, fmt);

    int return_value = vfcprintf(stream, color, fmt, args);

    va_end(args);

    return return_value;
}

//------------------------------------------------------------------------------------------

int vfcprintf(FILE* stream, const char * const color, const char* fmt, va_list args)
{
    if (color == NULL)
    {
        return vfprintf(stream, fmt, args);
    }

    char print_buffer[FMT_BUFFER_SIZE] = {};

    vsprintf(print_buffer, fmt, args);

    return fprintf(stream, "%s%s%s", color, print_buffer, RESET_COLOR);
}

//------------------------------------------------------------------------------------------

int wcprintf(const char * const color, const wchar_t* fmt, ...)
{
    va_list args = {};

    va_start(args, fmt);

    int return_value = wvcprintf(color, fmt, args);

    va_end(args);

    return return_value;
}

//------------------------------------------------------------------------------------------

int wvcprintf(const char * const color, const wchar_t* fmt, va_list args)
{
    return wvfcprintf(stdout, color, fmt, args);
}

//------------------------------------------------------------------------------------------

int wfcprintf(FILE* stream, const char * const color, const wchar_t* fmt, ...)
{
    va_list args = {};

    va_start(args, fmt);

    int return_value = wvfcprintf(stream, color, fmt, args);

    va_end(args);

    return return_value;
}

//------------------------------------------------------------------------------------------

int wvfcprintf(FILE* stream, const char * const color, const wchar_t* fmt, va_list args)
{
    if (color == NULL)
    {
        return vfwprintf(stream, fmt, args);
    }

    wchar_t print_buffer[FMT_BUFFER_SIZE] = {};

    vswprintf(print_buffer, sizeof(print_buffer), fmt, args);

    return fwprintf(stream, L"%s%ls%s", color, print_buffer, RESET_COLOR);
}

//------------------------------------------------------------------------------------------
