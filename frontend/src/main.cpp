#include "tree_commands.h"
#include "lang_funcs.h"
#include "lexer.h"
// #include "parser.h"

//------------------------------------------------------------------------------------------

int main()
{
    LangCtx_t lang_ctx = {};

    if (LangCtxCtor(&lang_ctx))
        return EXIT_FAILURE;

    lang_ctx.code   = strdup("5+2; \n"
                             "если (2 + 3 - 5)\n"
                             "{ аборт }\n"
                             "иначе \n"
                             "{ x = 5 + 1; }");
    lang_ctx.buffer = lang_ctx.code;

    LexicallyAnalyze(&lang_ctx);

    LangCtxDtor(&lang_ctx);

    return EXIT_SUCCESS;
}

//------------------------------------------------------------------------------------------
