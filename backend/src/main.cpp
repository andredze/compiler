#include "backend.h"
#include "AST_read.h"
#include <locale.h>

//------------------------------------------------------------------------------------------

int main(int argc, char* argv[])
{
    setlocale(LC_ALL, "en_US.utf8");

    if (argc == 1)
    {
        WPRINTERR("Expected ast input file through terminal args");
        return EXIT_FAILURE;
    }

    LangCtx_t lang_ctx = {};

    if (LangCtxCtor(&lang_ctx))
        return EXIT_FAILURE;

    do {
        if (ASTReadData(&lang_ctx, argv[1]))
            break;

    } while (0);

    LangCtxDtor(&lang_ctx);

    return EXIT_SUCCESS;
}

//------------------------------------------------------------------------------------------
