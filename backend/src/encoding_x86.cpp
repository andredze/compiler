#include "encoding_x86.h"
#include "bin_code_buffer.h"

//——————————————————————————————————————————————————————————————————————————————————————————

#define _DSL_DEFINE_
#include "dsl.h"

//==========================================================================================

static void BinInstrSetOpcode(BinInstruction_t* bin_instr, 
                              Opcode_t          opcode)
{
    assert(bin_instr);

    bin_instr->info.contains_opcode = 1;
    bin_instr->opcode               = opcode;
}

//==========================================================================================

static void BinInstrSetREXPrefix(BinInstruction_t*     bin_instr,
                                 OperandSize_t         w_bit,
                                 uint8_t               r_bit,
                                 SIBIndexExtension_t   x_bit,
                                 uint8_t               b_bit)
{
    assert(bin_instr);

    bin_instr->info.contains_rex     = 1;
    bin_instr->rex.fixed_bit_pattern = REX_FIRST_4_BITS;
    bin_instr->rex.w                 = w_bit & 1;
    bin_instr->rex.r                 = r_bit & 1;
    bin_instr->rex.x                 = x_bit & 1;
    bin_instr->rex.b                 = b_bit & 1;
}

//==========================================================================================

static void BinInstrSetModRM(BinInstruction_t* bin_instr,
                             ModRMMod_t        mod,
                             uint8_t           reg,
                             uint8_t           rm)
{
    assert(bin_instr);

    bin_instr->info.contains_modrm = 1;
    bin_instr->modrm.mod           = mod & 0b11;
    bin_instr->modrm.reg           = reg & 0b111;
    bin_instr->modrm.rm            = rm  & 0b111;
}

//==========================================================================================

static void BinInstrSetSIB(BinInstruction_t* bin_instr,
                           uint8_t           scale,
                           uint8_t           index,
                           uint8_t           base)
{
    assert(bin_instr);

    bin_instr->info.contains_sib = 1;
    bin_instr->sib.scale         = scale & 0b11;
    bin_instr->sib.index         = index & 0b111;
    bin_instr->sib.base          = base  & 0b111;
}

//==========================================================================================

static uint8_t GetRegCode(Register_t reg)
{
    return (uint8_t) reg & 0b111;
}

//==========================================================================================

static uint8_t GetModRMExtensionBit(Register_t reg)
{
    return (reg >> 3) & 1;   
}

//==========================================================================================

static uint8_t GetModRMExtensionBitForOperand(Operand_t operand)
{
    switch (operand.type)
    {
        case OPERAND_REG_64:
            return GetModRMExtensionBit(operand.value.reg);
        
        case OPERAND_MEM_64:
            return GetModRMExtensionBit(operand.value.mem.base);

        case OPERAND_IMM_32:
        case OPERAND_REL_32:
        case OPERAND_NONE:
        case OPERAND_TYPE_UNKNOWN:
        case OPERAND_TYPE_COUNT:
        default:
            return 0;
    }

    return 0;
}

//==========================================================================================

static void BinInstrSetREXPrefixAuto(BinInstruction_t*   bin_instr, 
                                     Instruction_t*      instr,
                                     ModRMType_t         op1_modrm_type, 
                                     ModRMType_t         op2_modrm_type,
                                     OperandSize_t       w_bit,
                                     SIBIndexExtension_t x_bit)
{
    assert(bin_instr);
    assert(instr);

    uint8_t r_bit = 0;
    uint8_t b_bit = 0;

    switch (op1_modrm_type)
    {
        case MODRM_TYPE_RM:
            b_bit = GetModRMExtensionBitForOperand(instr->operand_1);
            break;

        case MODRM_TYPE_REG:
            r_bit = GetModRMExtensionBitForOperand(instr->operand_1);
            break;

        case MODRM_TYPE_UNKNOWN:
        default:
            break;
    }

    switch (op2_modrm_type)
    {
        case MODRM_TYPE_RM:
            b_bit = GetModRMExtensionBitForOperand(instr->operand_2);
            break;

        case MODRM_TYPE_REG:
            r_bit = GetModRMExtensionBitForOperand(instr->operand_2);
            break;

        case MODRM_TYPE_UNKNOWN:
        default:
            break;
    }

    BinInstrSetREXPrefix(bin_instr, w_bit, r_bit, x_bit, b_bit);
}

//==========================================================================================

static void BinInstrSetREXPrefixDefault(BinInstruction_t* bin_instr, 
                                        Instruction_t*    instr, 
                                        ModRMType_t       op1_modrm_type,
                                        ModRMType_t       op2_modrm_type)
{
    return BinInstrSetREXPrefixAuto(bin_instr, instr, 
                                    op1_modrm_type, 
                                    op2_modrm_type, 
                                    OPERAND_SIZE_64, 
                                    SIB_INDEX_NO_EXTEND);
}

//==========================================================================================

BackendErr_t EncodeMovRegReg(BinInstruction_t* bin_instr, Instruction_t* instr)
{
    assert(bin_instr);
    assert(instr);

    ENCODE_VERIFY_(instr->opcode_type    == OPCODE_MOV_REG_REG);
    ENCODE_VERIFY_(instr->operand_1.type == OPERAND_REG_64);
    ENCODE_VERIFY_(instr->operand_2.type == OPERAND_REG_64);

    // REX.W + 8B /r
    BinInstrSetREXPrefixDefault(bin_instr, instr, MODRM_TYPE_REG, MODRM_TYPE_RM);

    BinInstrSetModRM(bin_instr, MODRM_MOD_RM_ONLY,
                                GetRegCode(instr->operand_1.value.reg),
                                GetRegCode(instr->operand_2.value.reg));

    return BACKEND_SUCCESS;
}

//==========================================================================================

static BackendErr_t EncodeInstruction(BinInstruction_t* bin_instr, Instruction_t* instr)
{
    assert(bin_instr);
    assert(instr);

    BinInstrSetOpcode(bin_instr, OPCODE_CASES_TABLE[instr->opcode_type].opcode);
    
    EncodeFunction_t encode_function = OPCODE_CASES_TABLE[instr->opcode_type].encode_function;
    
    return encode_function(bin_instr, instr);
}

//==========================================================================================

static uint8_t ReverseREXPrefix(REXPrefix_t rex)
{
    int byte = (rex.fixed_bit_pattern << 4) ^
               (rex.w << 3) ^
               (rex.r << 2) ^
               (rex.x << 1) ^
                rex.b;
    
    return (uint8_t) byte;
}

//==========================================================================================

static uint8_t ReverseModRM(ModRM_t modrm)
{
    int byte = (modrm.mod << 6) ^
               (modrm.reg << 3) ^
                modrm.rm;

    return (uint8_t) byte;
}

//==========================================================================================

static uint8_t ReverseSIB(SIB_t sib)
{
    int byte = (sib.scale << 5) ^ 
               (sib.index << 2) ^
                sib.base;
    
    return (uint8_t) byte;
}

//==========================================================================================

static BackendErr_t GenerateCodeFromBinInstruction(BinCode_t*        bin_code, 
                                                   BinInstruction_t* bin_instr)
{
    assert(bin_instr);
    assert(bin_code);
    assert(bin_code->buffer);
    
    EncodeInfo_t* encode_info = &bin_instr->info;

    if (encode_info->contains_rex)
    {
        uint8_t reversed = ReverseREXPrefix(bin_instr->rex);

        if (BinCodeWrite(bin_code, &reversed, sizeof(reversed)))
            return BACKEND_BINCODE_BUFFER_ERROR;
    }
    if (encode_info->contains_opcode)
    {
        // argument src = &bin_instr->opcode.data.size_1_byte,
        // however the actual size written in bin_code is specified
        // by bin_instr->opcode.size field and union field doesn't change anything
        if (BinCodeWrite(bin_code,
                         &bin_instr->opcode.data.size_1_byte, 
                         (size_t) bin_instr->opcode.size))
            return BACKEND_BINCODE_BUFFER_ERROR;
    }
    if (encode_info->contains_modrm)
    {
        uint8_t reversed = ReverseModRM(bin_instr->modrm);

        if (BinCodeWrite(bin_code, &reversed, sizeof(reversed)))
            return BACKEND_BINCODE_BUFFER_ERROR;
    }
    if (encode_info->contains_sib)
    {
        uint8_t reversed = ReverseSIB(bin_instr->sib);

        if (BinCodeWrite(bin_code, &reversed, sizeof(reversed)))
            return BACKEND_BINCODE_BUFFER_ERROR;
    }
    if (encode_info->contains_disp)
    {
        // same logic as with opcode union
        if (BinCodeWrite(bin_code, 
                         &bin_instr->disp.data.size_1_byte, 
                         bin_instr->disp.size))
            return BACKEND_BINCODE_BUFFER_ERROR;
    }
    if (encode_info->contains_imm)
    {
        // same logic as with opcode union
        if (BinCodeWrite(bin_code, 
                         &bin_instr->imm.data.size_1_byte, 
                         bin_instr->imm.size))
            return BACKEND_BINCODE_BUFFER_ERROR;
    }

    return BACKEND_SUCCESS;
}

//==========================================================================================

BackendErr_t GenerateCodeFromInstruction(BinCode_t* bin_code, Instruction_t* instr)
{
    assert(instr);

    BinInstruction_t bin_instr = {};
    BackendErr_t     error     = BACKEND_SUCCESS;

    if ((error = EncodeInstruction(&bin_instr, instr)))
    {
        free(instr);
        return error;
    }

    BIN_INSTRUCTION_DUMP(&bin_instr);

    if ((error = GenerateCodeFromBinInstruction(bin_code, &bin_instr)))
    {
        free(instr);
        return error;
    }

    free(instr);

    return BACKEND_SUCCESS;
}

//==========================================================================================

static void BinInstructionRexDump(REXPrefix_t rex)
{
    wcprintf(CYAN, 
LR"(rex prefix:
    fixed = %04b;
    w     = %01b;
    r     = %01b;
    x     = %01b;
    b     = %01b;
)", 
    rex.fixed_bit_pattern,
    rex.w,
    rex.r,
    rex.x,
    rex.b);
}

//------------------------------------------------------------------//

#define SET_GET_STRING_CASE(enum_value)         \
        BEGIN                                   \
        case enum_value: return #enum_value;    \
        END

//------------------------------------------------------------------//

static const char* BinInstructionGetModRMModString(ModRMMod_t mod)
{
    switch (mod)
    {
        SET_GET_STRING_CASE(MODRM_MOD_RM_ONLY);
        SET_GET_STRING_CASE(MODRM_MOD_RM_DISP8);
        SET_GET_STRING_CASE(MODRM_MOD_RM_DISP32);
        SET_GET_STRING_CASE(MODRM_MOD_RM_MIXED);
        
        default:
            break;
    }

    return "MODRM_MOD_LIMIT_EXCEEDED";
}

//------------------------------------------------------------------//

#undef SET_OPCODE_STRING_CASE

//------------------------------------------------------------------//

static void BinInstructionOpcodeDump(Opcode_t opcode)
{
    wcprintf(CYAN, 
LR"(opcode:
    size = %d;
    data = %x;
)", 
    opcode.size,
    opcode.data.size_3_byte);
}

//------------------------------------------------------------------//

static void BinInstructionModRMDump(ModRM_t modrm)
{
    wcprintf(CYAN, 
LR"(modrm byte:
    mod = %02b (%s);
    reg = %03b;
    rm  = %03b;
)", 
    modrm.mod,
    BinInstructionGetModRMModString((ModRMMod_t) modrm.mod),
    modrm.reg,
    modrm.rm);
}

//------------------------------------------------------------------//

static void BinInstructionSIBDump(SIB_t sib)
{
    wcprintf(CYAN, 
LR"(sib byte:
    scale = %02b;
    index = %03b;
    base  = %03b;
)", 
    sib.scale,
    sib.index,
    sib.base);
}

//------------------------------------------------------------------//

static void BinInstructionBinValueDump(BinValue_t value)
{
    wcprintf(CYAN, L"    size = %d;\n"
                   L"    data = %x;\n",
                   value.size,
                   value.data.size_8_byte);
}

//==========================================================================================

void BinInstructionDump(BinInstruction_t* bin_instr, 
                        const char*       func,
                        const char*       file, 
                        int               line)
{
    assert(bin_instr);

    wcprintf(CYAN, LR"(
===========================================
Dump: BinInstruction_t from %s at %s:%d
-------------------------------------------
bin_instr = %p
{
)",
    func,
    file,
    line,
    bin_instr);

    wcprintf(CYAN, L"contains_rex = %d\n", bin_instr->info.contains_rex);

    BinInstructionRexDump(bin_instr->rex);

    wcprintf(CYAN, L"contains_opcode = %d\n", bin_instr->info.contains_opcode);

    BinInstructionOpcodeDump(bin_instr->opcode);

    wcprintf(CYAN, L"contains_modrm = %d\n", bin_instr->info.contains_modrm);

    BinInstructionModRMDump(bin_instr->modrm);

    wcprintf(CYAN, L"contains_sib = %d\n", bin_instr->info.contains_sib);

    BinInstructionSIBDump(bin_instr->sib);

    wcprintf(CYAN, L"contains_imm = %d\n", bin_instr->info.contains_imm);

    wcprintf(CYAN, L"imm:\n");
    BinInstructionBinValueDump(bin_instr->imm);

    wcprintf(CYAN, L"contains_disp = %d\n", bin_instr->info.contains_disp);
    
    wcprintf(CYAN, L"disp:\n");
    BinInstructionBinValueDump(bin_instr->disp);

    wcprintf(CYAN, LR"(}
===========================================
)");
}

//==========================================================================================

#define _DSL_UNDEF_
#include "dsl.h"

//——————————————————————————————————————————————————————————————————————————————————————————
