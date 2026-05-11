#include <wchar.h>
#include <locale.h>
#include "frontend.h"
#include "data_read.h"
#include "lexer.h"
#include "parser.h"
#include "AST_write.h"

//==========================================================================================

//TODO - в readme parser и lexer GRAMMAR

int main(int argc, char* argv[])
{
    setlocale(LC_ALL, "en_US.utf8");

    if (argc == 1)
    {
        WPRINTERR(L"Expected code input file through terminal args");
        return EXIT_FAILURE;
    }

    FrontendCtx_t frontend_ctx = {};

    if (FrontendCtxCtor(&frontend_ctx))
        return EXIT_FAILURE;

    do 
    {
        if (FrontendReadInputInTree(&frontend_ctx, argv[1]))
            break;

        wfcprintf(stderr, BLUE, L"%s.psy, я разделю тебя на токены...\n", argv[1]);

        if (Tokenize(&frontend_ctx))
            break;

        wfcprintf(stderr, PURPLE, L"и добью рекурсивно\n");

        if (ParseTokens(&frontend_ctx))
            break;

        if (ASTWriteData(&frontend_ctx.lang_ctx))
            break;
    } 
    while (0);

    // if (lang_ctx.error_info.error != LANG_SUCCESS)
    //     LangPrintError(&lang_ctx);

#ifdef TREE_DEBUG
    TreeShowLogs(&frontend_ctx.lang_ctx.tree);
#endif /* TREE_DEBUG */

    FrontendCtxDtor(&frontend_ctx);

    return EXIT_SUCCESS;
}

//==========================================================================================
