#include "lang_ctx.h"
#include "lang_funcs.h"
#include "AST_read.h"
#include "op_cases.h"
#include <locale.h>

//==========================================================================================

int main(int argc, char* argv[])
{
    setlocale(LC_ALL, "en_US.utf8");

    if (argc == 1)
    {
        WPRINTERR(L"Expected AST input file through terminal args");
        return EXIT_FAILURE;
    }

    LangCtx_t lang_ctx = {};

    do
    {
        if (LangCtxCtor(&lang_ctx))
            break;

        if (ASTReadData(&lang_ctx, argv[1]))
            break;

        if (LangOpenReverseFile(&lang_ctx))
            break;

        if (SrcNode(&lang_ctx, lang_ctx.tree.dummy->right))
            break;
    }
    while (0);

    LangCtxDtor(&lang_ctx);

    return EXIT_SUCCESS;
}

//==========================================================================================
