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

    //------------------------------------------------------------------//

    MOV_MEM_REG_(REG_RSP, REG_RAX);
    MOV_MEM_REG_(REG_RBP, REG_R11);

    MOV_MEM_DISP_REG_(REG_RDX, 0xDED, REG_RAX);
    MOV_MEM_DISP_REG_(REG_RBP, -1000, REG_R11);

    //------------------------------------------------------------------//

    MOV_REG_IMM_(REG_RDX, 0xDED);
    MOV_REG_IMM_(REG_RBP, -0x1);

    //------------------------------------------------------------------//

    ADD_REG_REG_(REG_R13, REG_RAX);
    SUB_REG_REG_(REG_R13, REG_RAX);

    //------------------------------------------------------------------//

    IMUL_REG_(REG_R13);
    IMUL_REG_(REG_RAX);

    IDIV_REG_(REG_R14);
    IDIV_REG_(REG_RAX);

    //------------------------------------------------------------------//

    PUSH_REG_(REG_RBP);
    PUSH_REG_(REG_RAX);
    
    POP_REG_(REG_RBP);
    POP_REG_(REG_RAX);

    //------------------------------------------------------------------//
    
    RET_();
    SYSCALL_();

    //------------------------------------------------------------------//

    return BACKEND_SUCCESS;
}

//——————————————————————————————————————————————————————————————————————————————————————————

#define _DSL_UNDEF_
#include "dsl.h"

//==========================================================================================
