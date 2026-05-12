#include "encoding_x86.h"

//==========================================================================================

#define _DSL_DEFINE_
#include "dsl.h"

//——————————————————————————————————————————————————————————————————————————————————————————

BackendErr_t TestDSLEncoding(BackendCtx_t* backend_ctx)
{
    assert(backend_ctx);

    //------------------------------------------------------------------//

    MOV_REG_REG_(REG_RAX, REG_RBX);
    MOV_REG_REG_(REG_RBP, REG_R15);
    MOV_REG_REG_(REG_R10, REG_R10);

    //------------------------------------------------------------------//

    MOV_REG_MEM_(REG_RAX, REG_RSP);
    MOV_REG_MEM_(REG_R11, REG_RBP);

    MOV_REG_MEM_DISP_(REG_RAX, REG_RDX, 0xDED);
    MOV_REG_MEM_DISP_(REG_R11, REG_RBP, -1000);
    // MOV_REG_MEM_DISP_(REG_RAX, REG_RSP,  1000);

    //------------------------------------------------------------------//

    ADD_REG_REG_(REG_R13, REG_RAX);
    SUB_REG_REG_(REG_R13, REG_RAX);

    //------------------------------------------------------------------//

    return BACKEND_SUCCESS;
}

//——————————————————————————————————————————————————————————————————————————————————————————

#define _DSL_UNDEF_
#include "dsl.h"

//==========================================================================================
