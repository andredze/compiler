#ifndef COLORS_H
#define COLORS_H

//——————————————————————————————————————————————————————————————————————————————————————————

#include <stdio.h>
#include <stdarg.h>

//——————————————————————————————————————————————————————————————————————————————————————————

const char * const PURPLE       = "\x1b[35m";
const char * const RED          = "\033[1;31m";
const char * const BLUE         = "\033[0;34m";
const char * const GREEN        = "\x1b[32m";
const char * const YELLOW       = "\033[33m";
const char * const LIGHT_YELLOW = "\e[0;33m";
const char * const GRAY         = "\x1b[90m";
const char * const RESET_COLOR  = "\033[1;0m";

//——————————————————————————————————————————————————————————————————————————————————————————

const int FMT_BUFFER_SIZE = 8024;

//——————————————————————————————————————————————————————————————————————————————————————————

int cprintf   (const char * const color, const char* fmt, ...);
int vcprintf  (const char * const color, const char* fmt, va_list args);
int fcprintf  (FILE* stream, const char * const color, const char* fmt, ...);
int vfcprintf (FILE* stream, const char * const color, const char* fmt, va_list args);

int wcprintf  (const char * const color, const wchar_t* fmt, ...);
int wvcprintf (const char * const color, const wchar_t* fmt, va_list args);
int wfcprintf (FILE* stream, const char * const color, const wchar_t* fmt, ...);
int wvfcprintf(FILE* stream, const char * const color, const wchar_t* fmt, va_list args);

//——————————————————————————————————————————————————————————————————————————————————————————

#endif /* COLORS_H */
