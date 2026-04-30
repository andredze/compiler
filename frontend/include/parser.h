#ifndef PARSER_H
#define PARSER_H

//——————————————————————————————————————————————————————————————————————————————————————————

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "tree_types.h"
#include "tree_commands.h"
#include "lang_ctx.h"
#include "lang_funcs.h"

//——————————————————————————————————————————————————————————————————————————————————————————

LangErr_t ParseTokens(LangCtx_t* lang_ctx);

//——————————————————————————————————————————————————————————————————————————————————————————

const int MAX_SYNTAX_PARSE_ERR_MESSAGE_LEN = 128;

//——————————————————————————————————————————————————————————————————————————————————————————

#endif /* PARSER_H */
