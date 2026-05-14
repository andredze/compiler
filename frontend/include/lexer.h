#ifndef LEXER_H
#define LEXER_H

//——————————————————————————————————————————————————————————————————————————————————————————

#include "lang_ctx.h"
#include "tree_commands.h"
#include "tree_types.h"
#include "keywords.h"
#include "frontend.h"
#include "keyword_cases.h"

//——————————————————————————————————————————————————————————————————————————————————————————

LangErr_t Tokenize(FrontendCtx_t* frontend_ctx);

//——————————————————————————————————————————————————————————————————————————————————————————

const size_t MAX_SYNTAX_ERR_MESSAGE_LEN = 128;

//——————————————————————————————————————————————————————————————————————————————————————————

#endif /* LEXER_H */
