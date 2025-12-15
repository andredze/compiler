#include "tree_commands.h"
#include "lang_funcs.h"
#include "lexer.h"
#include "data_read.h"
#include <wchar.h>
#include <locale.h>
#include "parser.h"

//------------------------------------------------------------------------------------------

//TODO - в readme parser и lexer GRAMMAR

int main()
{
    setlocale(LC_ALL, "en_US.utf8");

    fwprintf(stderr, L"code = \n");

    LangCtx_t lang_ctx = {};

    if (LangCtxCtor(&lang_ctx))
        return EXIT_FAILURE;

    fwprintf(stderr, L"code = \n");

    do {
        if (TreeReadInputData(&lang_ctx))
            break;

        if (Tokenize(&lang_ctx))
            break;

        if (ParseTokens(&lang_ctx))
            break;

    } while (0);

    LangCtxDtor(&lang_ctx);

    return EXIT_SUCCESS;
}

//------------------------------------------------------------------------------------------
