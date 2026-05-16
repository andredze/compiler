#include "assembly_nasm.h"
#include "opcode_cases.h"

//==========================================================================================

#define _DSL_DEFINE_
#include "dsl.h"

//——————————————————————————————————————————————————————————————————————————————————————————

void PrintAsm(FILE* asm_file, const wchar_t* format_string, ...)
{
    assert(asm_file);

    va_list args = {};
    va_list args_copy = {};

    va_start(args, format_string);
    va_copy(args_copy, args);

    vfwprintf(asm_file, format_string, args);
    
    va_end(args);

#ifdef BACKEND_DEBUG
    wvfcprintf(stderr, GREEN, format_string, args_copy);
#endif /* BACKEND_DEBUG */

    va_end(args_copy);
}

//==========================================================================================

static BackendErr_t AssembleOpcode(BackendCtx_t* backend_ctx, OpcodeType_t opcode)
{
    assert(backend_ctx);

    if (!(OPCODE_UNKNOWN < opcode && opcode < OPCODE_COUNT))
    {
        WPRINTERR(L"Opcode type exceeds limit (%d)", opcode);
        return BACKEND_INVALID_OPCODE;
    }

    OpcodeCase_t op_case = OPCODE_CASES_TABLE[opcode]; 
    
    ASM_PRINT_(L"%s", op_case.asm_name);
    
    return BACKEND_SUCCESS;
}

//==========================================================================================

static BackendErr_t AssembleOperandReg(BackendCtx_t* backend_ctx, Register_t reg)
{
    assert(backend_ctx);

    if (!(REG_RAX <= reg && reg <= REG_R15))
    {
        WPRINTERR(L"Invalid register given (%d)", reg);
        return BACKEND_INVALID_REGISTER;
    }

    ASM_PRINT_(L"%s", REG_NAMES_TABLE[reg]);

    return BACKEND_SUCCESS;
}

//==========================================================================================

static void AssembleNumberAbs(BackendCtx_t* backend_ctx, int number)
{
    assert(backend_ctx);

    if (number < 0)
    {
        number = -number;
    }

    ASM_PRINT_(L"%#x", number);
}

//==========================================================================================

static void AssembleNumber(BackendCtx_t* backend_ctx, int number)
{
    assert(backend_ctx);

    if (number < 0)
    {
        ASM_PRINT_(L"-", number);
    }

    AssembleNumberAbs(backend_ctx, number);
}

//==========================================================================================

static BackendErr_t AssembleOperandMem(BackendCtx_t* backend_ctx, MemoryOperand_t mem)
{
    assert(backend_ctx);

    ASM_PRINT_(L"[");

    BackendErr_t error = BACKEND_SUCCESS;

    if (mem.base != REG_UNKNOWN)
    {
        if ((error = AssembleOperandReg(backend_ctx, mem.base)))
        {
            return error;
        }
        if (mem.disp == 0)
        {
            ASM_PRINT_(L"]");
            return BACKEND_SUCCESS;
        }
        if (mem.disp < 0)
        {
            ASM_PRINT_(L" - ");
        }
        else
        {
            ASM_PRINT_(L" + ");
        }
    }
    else if (mem.disp < 0)
    {
        ASM_PRINT_(L"-");
    }

    AssembleNumberAbs(backend_ctx, mem.disp);
    
    ASM_PRINT_(L"]");

    return BACKEND_SUCCESS;
}

//==========================================================================================

static BackendErr_t AssembleOperandRel(BackendCtx_t* backend_ctx, RelOperand_t rel)
{
    assert(backend_ctx);

    ASM_PRINT_(L"%ls", rel.label);
    ASM_PRINT_(L" ; %#x", rel.rel_addr);

    return BACKEND_SUCCESS;
}

//==========================================================================================

static BackendErr_t AssembleOperand(BackendCtx_t* backend_ctx, Operand_t operand)
{
    assert(backend_ctx);

    switch (operand.type)
    {
        case OPERAND_NONE:
            return BACKEND_SUCCESS;

        case OPERAND_REG_64:
            return AssembleOperandReg(backend_ctx, operand.value.reg);

        case OPERAND_MEM_64:
            return AssembleOperandMem(backend_ctx, operand.value.mem);

        case OPERAND_IMM_32:
            AssembleNumber(backend_ctx, operand.value.imm);
            return BACKEND_SUCCESS;

        case OPERAND_REL_32:
            return AssembleOperandRel(backend_ctx, operand.value.rel);

        case OPERAND_UNKNOWN:
        case OPERAND_TYPE_COUNT:
        default:
            WPRINTERR(L"Invalid operand type given (%d)", operand.type);
            return BACKEND_INVALID_OPERAND_TYPE;
    }

    return BACKEND_SUCCESS;
}

//==========================================================================================

BackendErr_t AssembleInstruction(BackendCtx_t* backend_ctx, Instruction_t* instr)
{
    assert(backend_ctx);
    assert(instr);

    ASM_PRINT_(L"\t");

    BackendErr_t error = BACKEND_SUCCESS;

    if ((error = AssembleOpcode(backend_ctx, instr->opcode_type)))
    {
        return error;
    }

    if (instr->operand_1.type == OPERAND_NONE)
    {
        ASM_PRINT_(L"\n");
        return BACKEND_SUCCESS;
    }

    ASM_PRINT_(L" ");
    
    if ((error = AssembleOperand(backend_ctx, instr->operand_1)))
    {
        return error;
    }

    if (instr->operand_2.type == OPERAND_NONE)
    {
        ASM_PRINT_(L"\n");
        return BACKEND_SUCCESS;
    }

    ASM_PRINT_(L", ");

    if ((error = AssembleOperand(backend_ctx, instr->operand_2)))
    {
        return error;
    }

    ASM_PRINT_(L"\n");

    return BACKEND_SUCCESS;
}

//——————————————————————————————————————————————————————————————————————————————————————————

#define _DSL_UNDEF_
#include "dsl.h"

//==========================================================================================
