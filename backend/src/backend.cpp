#include "backend.h"

//——————————————————————————————————————————————————————————————————————————————————————————

#define _DSL_DEFINE_
#include "dsl.h"

//==========================================================================================

LangErr_t AssembleProgram(LangCtx_t* lang_ctx)
{
    assert(lang_ctx);

    LangErr_t error = LANG_SUCCESS;

    if ((error = AssembleNode(lang_ctx, lang_ctx->tree.dummy->right)))
        return error;

    ASM_PRINT_(L"; end program\n\n");

    ASM_PRINT_(L"HLT\n");

    return LANG_SUCCESS;
}

//——————————————————————————————————————————————————————————————————————————————————————————

#define _DSL_UNDEF_
#include "dsl.h"

//——————————————————————————————————————————————————————————————————————————————————————————
