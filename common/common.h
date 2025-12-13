#ifndef COMMON_H
#define COMMON_H

//——————————————————————————————————————————————————————————————————————————————————————————

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cassert>
#include "colors.h"

//——————————————————————————————————————————————————————————————————————————————————————————

#define BEGIN do {

#define END   } while (0)

//——————————————————————————————————————————————————————————————————————————————————————————

#define PRINTERR(text, ...)                                                             \
        BEGIN                                                                           \
        fcprintf(stderr, RED, "In %s:%d from %s:\nERROR: " text "\n",                   \
                 __FILE__, __LINE__, __PRETTY_FUNCTION__, ##__VA_ARGS__);               \
        END

#define WPRINTERR(text, ...)                                                            \
        BEGIN                                                                           \
        wfcprintf(stderr, RED, L"In %s:%d from %s:\nERROR: " text "\n",                 \
                  __FILE__, __LINE__, __PRETTY_FUNCTION__, ##__VA_ARGS__);              \
        END

//——————————————————————————————————————————————————————————————————————————————————————————

#ifdef DEBUG
    #define DPRINTF(text, ...)                                 \
            BEGIN                                              \
            fcprintf(stderr, YELLOW, text, ##__VA_ARGS__);     \
            END
    #define WDPRINTF(text, ...)                                \
            BEGIN                                              \
            wfcprintf(stderr, YELLOW, text, ##__VA_ARGS__);    \
            END
#else
    #define DPRINTF(...)  ;
    #define WDPRINTF(...) ;
#endif /* DEBUG */

//——————————————————————————————————————————————————————————————————————————————————————————

const int MAX_FILENAME_LEN = 50;
const int MAX_COMMAND_LEN  = 1024;

//——————————————————————————————————————————————————————————————————————————————————————————

#endif /* COMMON_H */
