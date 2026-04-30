#ifndef LEXER_H
#define LEXER_H

//——————————————————————————————————————————————————————————————————————————————————————————

#include "lang_ctx.h"
#include "tree_commands.h"
#include "tree_types.h"
#include "operators.h"
#include "op_cases.h"

//——————————————————————————————————————————————————————————————————————————————————————————

LangErr_t Tokenize(LangCtx_t* lang_ctx);

//——————————————————————————————————————————————————————————————————————————————————————————

const size_t MAX_SYNTAX_ERR_MESSAGE_LEN = 128;

//——————————————————————————————————————————————————————————————————————————————————————————

#endif /* LEXER_H */
