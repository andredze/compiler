#ifndef PARSER_H
#define PARSER_H

//——————————————————————————————————————————————————————————————————————————————————————————

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "tree_types.h"
#include "tree_commands.h"

//——————————————————————————————————————————————————————————————————————————————————————————

typedef struct Expr
{
    char* buffer;
    char* cur_p;
} Expr_t;

//——————————————————————————————————————————————————————————————————————————————————————————

const char END_SYMBOL                 = '$';
const int  MAX_SYNTAX_ERR_MESSAGE_LEN = 128;

//——————————————————————————————————————————————————————————————————————————————————————————

#endif /* PARSER_H */
