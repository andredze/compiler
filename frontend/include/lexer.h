#ifndef LEXER_H
#define LEXER_H

//——————————————————————————————————————————————————————————————————————————————————————————

#include "lang_ctx.h"
#include "tree_commands.h"
#include "tree_types.h"
#include "operators.h"

//——————————————————————————————————————————————————————————————————————————————————————————

LangErr_t LexicallyAnalyze(LangCtx_t* lang_ctx);

//——————————————————————————————————————————————————————————————————————————————————————————

#endif /* LEXER_H */
