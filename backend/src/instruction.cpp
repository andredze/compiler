#include "instruction.h"
#include "opcode_cases.h"

//==========================================================================================

static BackendErr_t InstructionSetOpcode(Instruction_t* instr, OpcodeType_t opcode)
{
    assert(instr);

    if (!(OPCODE_UNKNOWN < opcode && opcode < OPCODE_COUNT))
    {
        WPRINTERR(L"Invalid opcode");
        return BACKEND_INVALID_OPCODE;
    }

    instr->opcode_type = opcode;

    return BACKEND_SUCCESS;
}

//==========================================================================================

static BackendErr_t SetOperandType(OperandType_t* op_type_dest, OperandType_t op_type_src)
{
    assert(op_type_dest);

    if (!(OPERAND_UNKNOWN < op_type_src && op_type_src < OPERAND_TYPE_COUNT))
    {
        WPRINTERR(L"Invalid operand type");
        return BACKEND_INVALID_OPERAND_TYPE;
    }

    *op_type_dest = op_type_src;

    return BACKEND_SUCCESS;
}

//==========================================================================================

static BackendErr_t InstructionSetOperand1_Type(Instruction_t* instr, 
                                                OperandType_t  op1_type)
{
    assert(instr);

    return SetOperandType(&instr->operand_1.type, op1_type);
}

//==========================================================================================

static BackendErr_t InstructionSetOperand2_Type(Instruction_t* instr, 
                                                OperandType_t  op2_type)
{
    assert(instr);

    return SetOperandType(&instr->operand_2.type, op2_type);
}

//==========================================================================================

static void InstructionSetOperand1_Value(Instruction_t* instr, 
                                         OperandValue_t op1_value)
{
    assert(instr);

    instr->operand_1.value = op1_value;
}

//==========================================================================================

static void InstructionSetOperand2_Value(Instruction_t* instr, 
                                         OperandValue_t op2_value)
{
    assert(instr);

    instr->operand_2.value = op2_value;
}

//==========================================================================================

Instruction_t* InstructionCreateRelNone(OpcodeType_t      opcode,
                                        RelativeAddress_t rel_addr,
                                        const wchar_t*    label)
{
    return InstructionCreate(opcode, {.rel = {.rel_addr = rel_addr, .label = label}}, {.none = 1});
}

//==========================================================================================

Instruction_t* InstructionCreateNoneNone(OpcodeType_t opcode)
{
    return InstructionCreate(opcode, {.none = 1}, {.none = 1});
}

//==========================================================================================

Instruction_t* InstructionCreateMemNone(OpcodeType_t   opcode,
                                        Register_t     base,
                                        Disp_t         disp)
{
    return InstructionCreate(opcode, {.mem = {.base = base, .disp = disp}}, {.none = 1});
}

//==========================================================================================

Instruction_t* InstructionCreateRegImm(OpcodeType_t   opcode,
                                       Register_t     reg1,
                                       Imm_t          imm)
{
    return InstructionCreate(opcode, {.reg = reg1}, {.imm = imm});
}

//==========================================================================================

Instruction_t* InstructionCreateMemReg(OpcodeType_t   opcode,
                                       Register_t     base,
                                       Disp_t         disp,
                                       Register_t     reg2)
{
    return InstructionCreate(opcode, {.mem = {.base = base, .disp = disp}}, {.reg = reg2});
}

//==========================================================================================

Instruction_t* InstructionCreateRegMem(OpcodeType_t   opcode,
                                       Register_t     reg1,
                                       Register_t     base,
                                       Disp_t         disp)
{
    return InstructionCreate(opcode, {.reg = reg1}, {.mem = {base, disp}});
}

//==========================================================================================

Instruction_t* InstructionCreateRegNone(OpcodeType_t   opcode,
                                        Register_t     reg1)
{
    return InstructionCreate(opcode, {.reg = reg1}, {.none = 1});
}

//==========================================================================================

Instruction_t* InstructionCreateRegReg(OpcodeType_t   opcode,
                                       Register_t     reg1, 
                                       Register_t     reg2)
{
    return InstructionCreate(opcode, {.reg = reg1}, {.reg = reg2});
}

//==========================================================================================

static BackendErr_t InstructionSetOpcodeWithOperandTypes(Instruction_t* instr, 
                                                         OpcodeType_t   opcode)
{
    assert(instr);

    BackendErr_t error = BACKEND_SUCCESS;

    if ((error = InstructionSetOpcode(instr, opcode)))
    {
        return error;
    }
    if ((error = InstructionSetOperand1_Type(instr, OPCODE_CASES_TABLE[opcode].op1_type)))
    {
        return error;
    }
    if ((error = InstructionSetOperand2_Type(instr, OPCODE_CASES_TABLE[opcode].op2_type)))
    {
        return error;
    }

    return BACKEND_SUCCESS;
}

//==========================================================================================

static BackendErr_t InstructionBuild(Instruction_t* instr, 
                                     OpcodeType_t   opcode,
                                     OperandValue_t operand1_value,
                                     OperandValue_t operand2_value)
{
    assert(instr);

    BackendErr_t error = BACKEND_SUCCESS;

    if ((error = InstructionSetOpcodeWithOperandTypes(instr, opcode)))
    {
        return error;
    }
    
    InstructionSetOperand1_Value(instr, operand1_value);
    InstructionSetOperand2_Value(instr, operand2_value);

    return BACKEND_SUCCESS;
}

//==========================================================================================

/* Note: returned instr has to be freed */
Instruction_t* InstructionCreate(OpcodeType_t   opcode,
                                 OperandValue_t operand1_value,
                                 OperandValue_t operand2_value)
{
    Instruction_t* instr = (Instruction_t*) calloc(sizeof(Instruction_t), 1); 

    if (instr == NULL)
    {
        WPRINTERR(L"Memalloc error");
        return NULL;
    }

    if (InstructionBuild(instr, opcode, operand1_value, operand2_value))
    {
        free(instr);
        return NULL;
    }

    return instr;
}

//==========================================================================================

const char* InstructionGetOpcodeTypeString(OpcodeType_t opcode_type)
{
    if (!(OPCODE_UNKNOWN <= opcode_type && opcode_type < OPCODE_COUNT))
    {
        return "OPCODE_LIMIT_EXCEEDED";
    }

    return OPCODE_CASES_TABLE[opcode_type].opcode_name;
}

//------------------------------------------------------------------//

#define SET_GET_STRING_CASE(enum_value)         \
        BEGIN                                   \
        case enum_value: return #enum_value;    \
        END

//------------------------------------------------------------------//

static const char* InstructionGetRegisterString(Register_t reg)
{
    switch (reg)
    {
        SET_GET_STRING_CASE(REG_RAX);
        SET_GET_STRING_CASE(REG_RCX);
        SET_GET_STRING_CASE(REG_RDX);
        SET_GET_STRING_CASE(REG_RBX);
        SET_GET_STRING_CASE(REG_RSP);
        SET_GET_STRING_CASE(REG_RBP);
        SET_GET_STRING_CASE(REG_RSI);
        SET_GET_STRING_CASE(REG_RDI);
        SET_GET_STRING_CASE(REG_R8);
        SET_GET_STRING_CASE(REG_R9);
        SET_GET_STRING_CASE(REG_R10);
        SET_GET_STRING_CASE(REG_R11);
        SET_GET_STRING_CASE(REG_R12);
        SET_GET_STRING_CASE(REG_R13);
        SET_GET_STRING_CASE(REG_R14);
        SET_GET_STRING_CASE(REG_R15);
        case REG_UNKNOWN:
        default:
            break;
    }

    return "REG_LIMIT_EXCEEDED";
}

//------------------------------------------------------------------//

static const char* InstructionGetOperandTypeString(OperandType_t operand_type)
{
    switch (operand_type)
    {
        SET_GET_STRING_CASE(OPERAND_REG_64);
        SET_GET_STRING_CASE(OPERAND_MEM_64);
        SET_GET_STRING_CASE(OPERAND_IMM_32);
        SET_GET_STRING_CASE(OPERAND_REL_32);
        SET_GET_STRING_CASE(OPERAND_NONE);
        SET_GET_STRING_CASE(OPERAND_UNKNOWN);
        case OPERAND_TYPE_COUNT:
        default:
            break;
    }

    return "OPERAND_TYPE_LIMIT_EXCEEDED";
}

//------------------------------------------------------------------//

#undef SET_OPCODE_STRING_CASE

//==========================================================================================

static void InstructionOperandValueDump(Operand_t operand)
{
    switch (operand.type)
    {
        case OPERAND_REG_64:
            wcprintf(MAGENTA, L"%s", InstructionGetRegisterString(operand.value.reg));
            break;

        case OPERAND_MEM_64:
            wcprintf(MAGENTA, L"{\n"
                              L"\t\tbase = %s;\n"
                              L"\t\tdisp = %d;\n"
                              L"\t\t};\n",
                              InstructionGetRegisterString(operand.value.mem.base),
                              operand.value.mem.disp);
            break;

        case OPERAND_IMM_32:
            wcprintf(MAGENTA, L"%d", operand.value.imm);
            break;

        case OPERAND_REL_32:
            wcprintf(MAGENTA, L"%ls (%d | HEX %#x)", 
                              operand.value.rel.label, 
                              operand.value.rel.rel_addr,
                              operand.value.rel.rel_addr);
            break;

        case OPERAND_NONE:
            wcprintf(MAGENTA, L"none");
            break;

        case OPERAND_UNKNOWN:
        case OPERAND_TYPE_COUNT:
        default:
            wcprintf(MAGENTA, L"error");
            break;
    }
}

//==========================================================================================

static void InstructionOperandDump(Operand_t operand)
{
    wcprintf(MAGENTA, L"\toperand.type  = %s;\n"
                      L"\toperand.value = ", 
                      InstructionGetOperandTypeString(operand.type));

    InstructionOperandValueDump(operand);

    wcprintf(MAGENTA, L";\n");
}

//==========================================================================================

void InstructionDump(Instruction_t* instr, const char* func, const char* file, int line)
{
    assert(instr);

    wcprintf(MAGENTA, LR"(
===========================================
Dump: Instruction_t from %s at %s:%d
-------------------------------------------
instr = %p
{
  opcode_type = %s;
)",
    func,
    file,
    line,
    instr,
    InstructionGetOpcodeTypeString(instr->opcode_type));

    wcprintf(MAGENTA, L"  operand1:\n");
    InstructionOperandDump(instr->operand_1);
    
    wcprintf(MAGENTA, L"  operand2:\n");
    InstructionOperandDump(instr->operand_2);
    
    wcprintf(MAGENTA, LR"(}
===========================================
)");
}

//==========================================================================================
