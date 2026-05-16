#ifndef PARSER_H
#define PARSER_H

//——————————————————————————————————————————————————————————————————————————————————————————

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "tree_types.h"
#include "tree_commands.h"
#include "lang_ctx.h"
#include "frontend.h"
#include "lang_funcs.h"

//——————————————————————————————————————————————————————————————————————————————————————————

#define SYNTAX_ERROR_(message, ...)                                 \
        BEGIN                                                       \
        TreeNode_t* _node = FrontendGetCurrentToken(frontend_ctx);  \
        WPRINTERR(L"At line %d: " message,                          \
                  _node->line, ##__VA_ARGS__);                      \
        END

//——————————————————————————————————————————————————————————————————————————————————————————

LangErr_t ParseTokens(FrontendCtx_t* frontend_ctx);

//——————————————————————————————————————————————————————————————————————————————————————————

const int MAX_SYNTAX_PARSE_ERR_MESSAGE_LEN = 128;

//——————————————————————————————————————————————————————————————————————————————————————————

#endif /* PARSER_H */
