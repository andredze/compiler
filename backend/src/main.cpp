#include "backend.h"
#include "AST_read.h"
#include "lang_funcs.h"
#include "backend.h"
#include "emission.h"
#include <locale.h>

//==========================================================================================

int main(int argc, char* argv[])
{
    setlocale(LC_ALL, "en_US.utf8");

    DPRINT_FUNC_ENTER_MSG();

    if (argc == 1)
    {
        WPRINTERR("Expected AST input file path through terminal args");
        return EXIT_FAILURE;
    }

    BackendCtx_t backend_ctx = {};

    do
    {
        if (BackendCtxCtor(&backend_ctx))
            break;

        if (ASTReadData(&backend_ctx.lang_ctx, argv[1], backend_ctx.ast_file_name))
            break;

        LangIdTableDump(&backend_ctx.lang_ctx.main_id_table);

        if (BackendOpenAsmFile(&backend_ctx))
            break;

        if (EmitProgram(&backend_ctx))
            break;
    }
    while (0);

    BackendCtxDtor(&backend_ctx);

    DPRINT_FUNC_LEAVE_MSG();
    return EXIT_SUCCESS;
}

//==========================================================================================
