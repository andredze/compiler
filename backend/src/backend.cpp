#include "backend.h"

//------------------------------------------------------------------------------------------

LangErr_t AssembleProgram(LangCtx_t* lang_ctx)
{
    assert(lang_ctx);

    LangErr_t error = LANG_SUCCESS;

    if ((error = AssembleNode(lang_ctx, lang_ctx->tree.dummy->right)))
        return error;

    return LANG_SUCCESS;
}

//------------------------------------------------------------------------------------------
