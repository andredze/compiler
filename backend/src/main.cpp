#include "backend.h"
#include "AST_read.h"
#include "lang_funcs.h"
#include "backend.h"
#include "elf_build.h"
#include "emission.h"
#include "encoding_x86.h"
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
    ElfCtx_t     elf_ctx     = {};

    do
    {
        if (BackendCtxCtor(&backend_ctx))
            break;

        if (ASTReadData(&backend_ctx.lang_ctx, argv[1], backend_ctx.ast_file_name))
            break;

        if (LangCountAddresses(&backend_ctx.lang_ctx))
            break;

        if (BackendOpenAsmFile(&backend_ctx))
            break;

        #ifdef TEST_DSL
            TestDSLEncoding(&backend_ctx);
            break;
        #endif

        wcprintf(BLUE, L"Скомпилить скомпилить скомпилить\n"
                       L"\t%s/%s.txt\n"
                       L"\t%s/%s.txt\n"
                       L"\t%s/%s.txt\n",
                 AST_DIR_NAME, backend_ctx.ast_file_name,
                 AST_DIR_NAME, backend_ctx.ast_file_name,
                 AST_DIR_NAME, backend_ctx.ast_file_name);

        if (EmitProgram(&backend_ctx))
            break;

        if (ElfCtxCtor(&elf_ctx, &backend_ctx))
            break;

        if (ElfBuild(&backend_ctx, &elf_ctx))
            break;

        if (BackendOpenElfFile(&backend_ctx))
            break;

        if (ElfCopyContextToBuffer(&elf_ctx))
            break;

        ELF_CTX_DUMP_(&backend_ctx, &elf_ctx, L"dump after copying to buffer");

        if (ElfWriteBuffer(backend_ctx.elf_file, &elf_ctx))
            break;
    }
    while (0);

    wcprintf(GREEN, L"ГОТОВО ГОТОВО ГО Т ,Юб ВО?./???\n"
                     "\t%s/%s\n"
                     "\t%s/%s\n"
                     "\t%s/%s\n",
             EXEC_DIR_NAME, backend_ctx.ast_file_name,
             EXEC_DIR_NAME, backend_ctx.ast_file_name,
             EXEC_DIR_NAME, backend_ctx.ast_file_name);

#if defined(TREE_DEBUG) && !(defined(TEST_DSL))
    TreeShowLogs(&backend_ctx.lang_ctx.tree);
#endif /* TREE_DEBUG */

    BackendCtxDtor(&backend_ctx);
    ElfCtxDtor    (&elf_ctx);

    DPRINT_FUNC_LEAVE_MSG();
    return EXIT_SUCCESS;
}

//==========================================================================================
