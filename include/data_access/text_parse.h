#ifndef TEXT_PARSE_H
#define TEXT_PARSE_H

//——————————————————————————————————————————————————————————————————————————————————————————

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "math_types.h"
#include "funcs.h"
#include "tex.h"
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

MathErr_t MathParseText(MathCtx_t* math_ctx, Expr_t* expr);

//——————————————————————————————————————————————————————————————————————————————————————————

#endif /* TEXT_PARSE_H */
